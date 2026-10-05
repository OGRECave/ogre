/* Copyright 2026 Google LLC
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
      http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

/*
 * Fuzz the texture codecs built into OgreMain: DDS (OgreDDSCodec.cpp),
 * PVR (OgrePVRTCCodec.cpp), PKM/KTX (OgreETCCodec.cpp) and ASTC
 * (OgreASTCCodec.cpp).
 *
 * These files are untrusted assets: they arrive with downloaded models, mods
 * and level packs, and the codecs parse headers, mipmap chains and cubemap
 * faces, and DDSCodec decompresses DXT1/3/5 blocks by hand.
 *
 * The existing image_fuzz cannot reach any of it: it registers only
 * Ogre::STBIImageCodec and calls Image::load(stream, "png"), so the codec is
 * chosen by that fixed extension.
 *
 * The codecs are private classes (their headers are not installed), so they
 * are registered the supported way instead: Ogre::Root::Root() calls their
 * startup() unless OGRE_NO_<FORMAT>_CODEC is set. The Root has to outlive every
 * call, because ~Root() calls shutdown() and unregisters them again -- that is
 * precisely why image_fuzz, whose Root is a scoped local, leaves them
 * unreachable.
 *
 * The codec is picked from the file's magic number, so plain texture files are
 * valid seeds. The extension is still passed explicitly, because Ogre's own
 * detection reads only 32 bytes and misses the PVR v2 tag at offset 44.
 */

#include <stdint.h>
#include <stddef.h>

#include <algorithm>
#include <cstring>
#include <string>

#include "OgreDataStream.h"
#include "OgreException.h"
#include "OgreImage.h"
#include "OgreLogManager.h"
#include "OgreRoot.h"

static Ogre::Root* g_root = nullptr;

static uint32_t read_le32(const uint8_t* p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static uint32_t read_le24(const uint8_t* p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16);
}

static bool bounded_dimensions(uint32_t width, uint32_t height, uint32_t depth)
{
    return width > 0 && height > 0 && depth > 0 && width <= 4096 && height <= 4096 && depth <= 256 &&
           (uint64_t)width * height * depth <= (1u << 22);
}

static bool bounded_image_set(uint32_t width, uint32_t height, uint32_t depth, uint32_t faces,
                              uint32_t surfaces, uint32_t mips)
{
    // Ogre::Image supports only 1 or 6 faces; PVR decoding does not handle arrays.
    if ((faces != 1 && faces != 6) || surfaces != 1 || mips == 0 || mips > 16)
        return false;
    return (uint64_t)width * height * depth * faces * mips <= (1u << 22);
}

static bool plausible_dds(const uint8_t* data, size_t size)
{
    if (size < 4 + 7 * 4 || memcmp(data, "DDS ", 4) != 0)
        return false;

    auto field = [data](size_t index) -> uint64_t { return read_le32(data + 4 + index * 4); };

    const uint64_t height = field(2);
    const uint64_t width = field(3);
    const uint64_t depth = field(5) ? field(5) : 1;
    const uint64_t mips = field(6);

    return width <= 4096 && height <= 4096 && depth <= 1024 &&
           width * height * depth <= (1u << 22) && mips <= 16;
}

static bool plausible_astc(const uint8_t* data, size_t size)
{
    if (size < 16 || read_le32(data) != 0x5ca1ab13)
        return false;

    const uint8_t bx = data[4];
    const uint8_t by = data[5];
    const uint8_t bz = data[6];
    const bool valid_2d_block = (bx == 4 && by == 4) || (bx == 5 && (by == 4 || by == 5)) ||
                                (bx == 6 && (by == 5 || by == 6)) ||
                                (bx == 8 && (by == 5 || by == 6 || by == 8)) ||
                                (bx == 10 && (by == 5 || by == 6 || by == 8 || by == 10)) ||
                                (bx == 12 && (by == 10 || by == 12));

    return valid_2d_block && bz != 0 && bz <= 6 &&
           bounded_dimensions(read_le24(data + 7), read_le24(data + 10), read_le24(data + 13));
}

static bool plausible_pkm(const uint8_t* data, size_t size)
{
    if (size < 16 || memcmp(data, "PKM ", 4) != 0)
        return false;

    const uint32_t width = ((uint32_t)data[12] << 8) | data[13];
    const uint32_t height = ((uint32_t)data[14] << 8) | data[15];
    return bounded_dimensions(width, height, 1);
}

static bool plausible_ktx(const uint8_t* data, size_t size)
{
    static const uint8_t ktx_id[12] = {0xab, 0x4b, 0x54, 0x58, 0x20, 0x31,
                                       0x31, 0xbb, 0x0d, 0x0a, 0x1a, 0x0a};
    if (size < 64 || memcmp(data, ktx_id, sizeof(ktx_id)) != 0)
        return false;

    const uint32_t width = read_le32(data + 36);
    const uint32_t height = read_le32(data + 40);
    const uint32_t depth = std::max(1u, read_le32(data + 44));
    const uint32_t faces = read_le32(data + 52);
    const uint32_t mips = std::max(1u, read_le32(data + 56));
    const uint32_t metadata_size = read_le32(data + 60);

    return bounded_dimensions(width, height, depth) &&
           bounded_image_set(width, height, depth, faces, 1, mips) && metadata_size <= size - 64;
}

static bool plausible_pvr(const uint8_t* data, size_t size)
{
    if (size < 52)
        return false;

    if (read_le32(data + 44) == 0x21525650) // PVR v2 "PVR!"
    {
        const uint32_t height = read_le32(data + 4);
        const uint32_t width = read_le32(data + 8);
        const uint32_t mip_count = read_le32(data + 12);
        return bounded_dimensions(width, height, 1) && mip_count < 16 &&
               bounded_image_set(width, height, 1, 1, 1, mip_count + 1);
    }

    if (read_le32(data) == 0x03525650) // PVR v3
    {
        const uint32_t height = read_le32(data + 24);
        const uint32_t width = read_le32(data + 28);
        const uint32_t depth = std::max(1u, read_le32(data + 32));
        const uint32_t surfaces = read_le32(data + 36);
        const uint32_t faces = read_le32(data + 40);
        const uint32_t mips = read_le32(data + 44);
        const uint32_t metadata_size = read_le32(data + 48);
        return bounded_dimensions(width, height, depth) &&
               bounded_image_set(width, height, depth, faces, surfaces, mips) &&
               metadata_size <= size - 52;
    }

    return false;
}

static const char* detect_extension(const uint8_t* data, size_t size)
{
    if (plausible_dds(data, size))
        return "dds";
    if (plausible_astc(data, size))
        return "astc";
    if (plausible_pkm(data, size))
        return "pkm";
    if (plausible_ktx(data, size))
        return "ktx";
    if (plausible_pvr(data, size))
        return "pvr";
    return nullptr;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (g_root == nullptr)
    {
        Ogre::LogManager* logMgr = new Ogre::LogManager();
        logMgr->createLog("dds_fuzz.log", true, false, true);
        logMgr->setMinLogLevel(Ogre::LML_CRITICAL);

        g_root = new Ogre::Root("", "", "");
    }

    if (size == 0 || size > 1024 * 1024)
        return 0;

    const char* extension = detect_extension(data, size);
    if (!extension)
        return 0;

    try
    {
        Ogre::DataStreamPtr stream(new Ogre::MemoryDataStream(
            const_cast<uint8_t*>(data), size, false, true));

        Ogre::Image img;
        img.load(stream, extension);

        if (img.getSize() != 0)
        {
            volatile uint8_t sink = img.getData()[0];
            (void)sink;
            (void)img.getWidth();
            (void)img.getHeight();
            (void)img.getNumMipmaps();
            (void)img.getFormat();
        }
    }
    catch (const Ogre::Exception&)
    {
    }

    return 0;
}
