// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#include "OgreRsImageCodec.h"
#include "OgreLogManager.h"
#include "OgreDataStream.h"
#include "OgreImage.h"

enum ColorType
{
    /// Pixel is 8-bit luminance
    L8,
    /// Pixel is 8-bit luminance with an alpha channel
    La8,
    /// Pixel contains 8-bit R, G and B channels
    Rgb8,
    /// Pixel is 8-bit RGB with an alpha channel
    Rgba8,
    /// Pixel is 16-bit luminance
    L16,
    /// Pixel is 16-bit luminance with an alpha channel
    La16,
    /// Pixel is 16-bit RGB
    Rgb16,
    /// Pixel is 16-bit RGBA
    Rgba16,
    /// Pixel is 32-bit float RGB
    Rgb32F,
    /// Pixel is 32-bit float RGBA
    Rgba32F
};

extern "C"
{
    void rs_image_decode(const char* buf, size_t len, const char* ext, uint32_t* w, uint32_t* h, ColorType* t,
                         uint8_t** out, size_t* out_len);
    void rs_image_encode(uint8_t* buf, size_t len, uint32_t w, uint32_t h, ColorType t, const char* ext, uint8_t** out,
                         size_t* out_len);
    void rs_vec_free(uint8_t* ptr, size_t len);
}

namespace Ogre
{

RsImageCodec::RegisteredCodecList RsImageCodec::msCodecList;
//---------------------------------------------------------------------
void RsImageCodec::startup(void)
{
    // Register codecs
    String exts = "jpeg,jpg,png,bmp,gif,tiff,tga,exr";
    StringVector extsVector = StringUtil::split(exts, ",");
    for (auto& v : extsVector)
    {
        ImageCodec* codec = OGRE_NEW RsImageCodec(v);
        msCodecList.push_back(codec);
        Codec::registerCodec(codec);
    }

    LogManager::getSingleton().logMessage("image-rs loader - supported formats: " + exts);
}
//---------------------------------------------------------------------
void RsImageCodec::shutdown(void)
{
    for (auto& i : msCodecList)
    {
        Codec::unregisterCodec(i);
        OGRE_DELETE i;
    }
    msCodecList.clear();
}
//---------------------------------------------------------------------
RsImageCodec::RsImageCodec(const String& type) : mType(type) {}
//---------------------------------------------------------------------
DataStreamPtr RsImageCodec::encode(const Any& input) const
{
    Image* image = any_cast<Image*>(input);
    PixelFormat format = image->getFormat();
    uchar* inputData = image->getData();

    ColorType t;

    switch (format)
    {
    case PF_BYTE_L:
        t = L8;
        break;
    case PF_BYTE_LA:
        t = La8;
        break;
    case PF_BYTE_RGB:
        t = Rgb8;
        break;
    case PF_BYTE_RGBA:
        t = Rgba8;
        break;
    default:
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Unsupported image format");
        break;
    }

    size_t len;
    uchar* data = NULL;

    rs_image_encode(inputData, image->getSize(), image->getWidth(), image->getHeight(), t, mType.c_str(), &data, &len);

    if (!data)
    {
        OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Error encoding image");
    }

    auto stream = std::make_shared<MemoryDataStream>(len);
    memcpy(stream->getPtr(), data, len); // avoid messing with rust memory allocator
    rs_vec_free(data, len);

    return stream;
}
//---------------------------------------------------------------------
void RsImageCodec::encodeToFile(const Any& input, const String& outFileName) const
{
#if OGRE_PLATFORM != OGRE_PLATFORM_EMSCRIPTEN
    MemoryDataStreamPtr data = static_pointer_cast<MemoryDataStream>(encode(input));
    std::ofstream f(outFileName.c_str(), std::ios::out | std::ios::binary);

    if (!f.is_open())
    {
        OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "could not open file " + outFileName);
    }

    f.write((char*)data->getPtr(), data->size());
#endif
}
//---------------------------------------------------------------------
void RsImageCodec::decode(const DataStreamPtr& input, const Any& output) const
{
    auto image = any_cast<Image*>(output);
    String contents = input->getAsString();

    uint width, height;
    ColorType t;
    uchar* pixelData = NULL;
    size_t dataLen;

    rs_image_decode(contents.data(), contents.size(), mType.c_str(), &width, &height, &t, &pixelData, &dataLen);

    if (!pixelData)
    {
        OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Error decoding image");
    }

    PixelFormat format = PF_UNKNOWN;
    switch (t)
    {
    case L8:
        format = PF_BYTE_L;
        break;
    case La8:
        format = PF_BYTE_LA;
        break;
    case Rgb8:
        format = PF_BYTE_RGB;
        break;
    case Rgba8:
        format = PF_BYTE_RGBA;
        break;
    case L16:
        format = PF_L16;
        break;
    case Rgb32F:
        format = PF_FLOAT32_RGB;
        break;
    case Rgba32F:
        format = PF_FLOAT32_RGBA;
        break;
    default:
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Unsupported image format");
        break;
    }

    image->create(format, width, height);
    memcpy(image->getData(), pixelData, dataLen); // avoid messing with rust memory allocator
    rs_vec_free(pixelData, dataLen);
}
//---------------------------------------------------------------------
String RsImageCodec::getType() const { return mType; }
//---------------------------------------------------------------------
String RsImageCodec::magicNumberToFileExt(const char* magicNumberPtr, size_t maxbytes) const { return BLANKSTRING; }

#ifndef OGRE_STATIC_LIB
extern "C" void _OgreRsImageCodecExport dllStartPlugin();
extern "C" void _OgreRsImageCodecExport dllStopPlugin();

extern "C" void _OgreRsImageCodecExport dllStartPlugin() { RsImageCodec::startup(); }
extern "C" void _OgreRsImageCodecExport dllStopPlugin() { RsImageCodec::shutdown(); }
#endif
} // namespace Ogre
