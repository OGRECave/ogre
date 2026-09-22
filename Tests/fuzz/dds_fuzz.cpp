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
 * Fuzz the DDS texture codec (OgreMain/src/OgreDDSCodec.cpp).
 *
 * A .dds file is an untrusted asset: it arrives with downloaded models, mods
 * and level packs, and OgreDDSCodec parses its header, pixel format masks,
 * mipmap chain and cubemap faces, then decompresses DXT1/3/5 blocks by hand.
 *
 * The existing image_fuzz cannot reach any of it: it registers only
 * Ogre::STBIImageCodec and calls Image::load(stream, "png"), so the codec is
 * chosen by that fixed extension.
 *
 * DDSCodec is a private class (OgreMain/src/OgreDDSCodec.h is not installed),
 * so it is registered the supported way instead: Ogre::Root::Root() calls
 * DDSCodec::startup() when OGRE_CONFIG_ENABLE_DDS is on, which is the default.
 * The Root has to outlive every call, because ~Root() calls DDSCodec::shutdown()
 * and unregisters it again -- that is precisely why image_fuzz, whose Root is a
 * scoped local, leaves the codec unreachable.
 */

#include <stdint.h>
#include <stddef.h>

#include <cstring>
#include <string>

#include "OgreDataStream.h"
#include "OgreException.h"
#include "OgreImage.h"
#include "OgreLogManager.h"
#include "OgreRoot.h"

static Ogre::Root* g_root = nullptr;

static bool plausible_surface(const uint8_t* data, size_t size)
{
    if (size < 4 + 7 * 4 || memcmp(data, "DDS ", 4) != 0)
        return false;

    auto field = [data](size_t index) -> uint64_t {
        const uint8_t* p = data + 4 + index * 4;
        return (uint64_t)p[0] | ((uint64_t)p[1] << 8) |
               ((uint64_t)p[2] << 16) | ((uint64_t)p[3] << 24);
    };

    const uint64_t height = field(2);
    const uint64_t width = field(3);
    const uint64_t depth = field(5) ? field(5) : 1;
    const uint64_t mips = field(6);

    return width <= 4096 && height <= 4096 && depth <= 1024 &&
           width * height * depth <= (1u << 22) && mips <= 16;
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

    if (size == 0 || size > 1024 * 1024 || !plausible_surface(data, size))
        return 0;

    try
    {
        Ogre::DataStreamPtr stream(new Ogre::MemoryDataStream(
            const_cast<uint8_t*>(data), size, false, true));

        Ogre::Image img;
        img.load(stream, "dds");

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
