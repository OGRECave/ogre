/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

#include "OgreSTBICodec.h"
#include "OgreLogManager.h"
#include "OgreDataStream.h"
#include "OgreImage.h"

#include "OgrePlatformInformation.h"
#include "OgreString.h"
#include "OgreRoot.h"

#include <vector>

#if __OGRE_HAVE_NEON
#define STBI_NEON
#endif

#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
// guard against wrong image headers, limits to 16K x 16K
#define STBI_MAX_DIMENSIONS 16384
#include "stbi/stb_image.h"

#ifdef HAVE_ZLIB
#include <zlib.h>
static Ogre::uchar* custom_zlib_compress(Ogre::uchar* data, int data_len, int* out_len, int /*quality*/)
{
    unsigned long destLen = compressBound(data_len);
    Ogre::uchar* dest = (Ogre::uchar*)malloc(destLen);
    int ret = compress(dest, &destLen, data, data_len); // use default quality
    if (ret != Z_OK)
    {
        free(dest);
        OGRE_EXCEPT(Ogre::Exception::ERR_INTERNAL_ERROR, "compress failed");
    }

    *out_len = destLen;
    return dest;
}
#define STBIW_ZLIB_COMPRESS custom_zlib_compress
#endif

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_WRITE_NO_STDIO
#include "stbi/stb_image_write.h"

namespace Ogre {

    namespace
    {
        void appendToVector(void* context, void* data, int size)
        {
            auto* out = static_cast<std::vector<uchar>*>(context);
            auto* bytes = static_cast<uchar*>(data);
            out->insert(out->end(), bytes, bytes + size);
        }
    }

    STBIImageCodec::RegisteredCodecList STBIImageCodec::msCodecList;
    //---------------------------------------------------------------------
    void STBIImageCodec::startup(void)
    {
        stbi_convert_iphone_png_to_rgb(1);
        stbi_set_unpremultiply_on_load(1);

        LogManager::getSingleton().logMessage("stb_image - v2.30 - public domain image loader");
        
        // Register codecs
        String exts = "jpeg,jpg,png,bmp,psd,tga,gif,pic,ppm,pgm,hdr";
        StringVector extsVector = StringUtil::split(exts, ",");
        for (auto & v : extsVector)
        {
            ImageCodec* codec = OGRE_NEW STBIImageCodec(v);
            msCodecList.push_back(codec);
            Codec::registerCodec(codec);
        }

        LogManager::getSingleton().logMessage("Supported formats: " + exts);
    }
    //---------------------------------------------------------------------
    void STBIImageCodec::shutdown(void)
    {
        for (auto & i : msCodecList)
        {
            Codec::unregisterCodec(i);
            OGRE_DELETE i;
        }
        msCodecList.clear();
    }
    //---------------------------------------------------------------------
    STBIImageCodec::STBIImageCodec(const String &type):
        mType(type)
    { 
    }
    //---------------------------------------------------------------------
    DataStreamPtr STBIImageCodec::encode(const Any& input) const
    {
        if(mType != "png" && mType != "jpeg" && mType != "jpg") {
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
                        "currently only encoding to PNG and JPEG supported",
                        "STBIImageCodec::encode" ) ;
        }

        Image* image = any_cast<Image*>(input);
        PixelFormat format = image->getFormat();
        uchar* inputData = image->getData();

        // Convert image data to ABGR format for STBI (unless it's already compatible)
        uchar* tempData = 0;
        if(format != Ogre::PF_A8B8G8R8 && format != PF_B8G8R8 && format != PF_BYTE_LA && 
            format != PF_L8 && format != PF_R8)
        {   
            format = Ogre::PF_A8B8G8R8;
            size_t tempDataSize = PixelUtil::getMemorySize(image->getWidth(), image->getHeight(), 1, format);
            tempData = OGRE_ALLOC_T(unsigned char, tempDataSize, Ogre::MEMCATEGORY_GENERAL);
            Ogre::PixelBox pbOut(image->getPixelBox(), format, tempData);
            PixelUtil::bulkPixelConversion(image->getPixelBox(), pbOut);

            inputData = tempData;
        }

        const int channels = PixelUtil::getComponentCount(format);

        if (mType == "png")
        {
            int len;
            uchar* data = stbi_write_png_to_mem(inputData, image->getRowSpan(), image->getWidth(),
                                                image->getHeight(), channels, &len);

            if(tempData)
            {
                OGRE_FREE(tempData, MEMCATEGORY_GENERAL);
            }

            if (!data) {
                OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                    "Error encoding image: " + String(stbi_failure_reason()),
                    "STBIImageCodec::encode");
            }

            return DataStreamPtr(new MemoryDataStream(data, len, true));
        }

        // Save to JPEG. JPEG has no alpha channel, so it is discarded by stb_image_write.
        static const int quality = 90;
        std::vector<uchar> data;
        const bool success = stbi_write_jpg_to_func(appendToVector, &data, image->getWidth(),
                                                    image->getHeight(), channels, inputData, quality);

        if(tempData)
        {
            OGRE_FREE(tempData, MEMCATEGORY_GENERAL);
        }

        if (!success)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Error encoding image as JPEG",
                        "STBIImageCodec::encode");
        }

        auto stream = std::make_shared<MemoryDataStream>(data.size());
        memcpy(stream->getPtr(), data.data(), data.size());
        return stream;
    }
    //---------------------------------------------------------------------
    void STBIImageCodec::encodeToFile(const Any& input, const String& outFileName) const
    {
#if OGRE_PLATFORM != OGRE_PLATFORM_EMSCRIPTEN
        MemoryDataStreamPtr data = static_pointer_cast<MemoryDataStream>(encode(input));
        DataStreamPtr f = Root::createFileStream(outFileName, "", true);

        f->write(data->getPtr(), data->size());
        f->close();
#endif
    }
    //---------------------------------------------------------------------
    void STBIImageCodec::decode(const DataStreamPtr& input, const Any& output) const
    {
        auto image = any_cast<Image*>(output);
        String contents = input->getAsString();

        int width, height, components;
        const int size = static_cast<int>(contents.size());
        const bool is16Bit = stbi_is_16_bit_from_memory((const uchar*)contents.data(), size) != 0;

        // stb_image always decodes to 8 bits unless explicitly asked for 16 bits
        void* pixelData = is16Bit
            ? static_cast<void*>(stbi_load_16_from_memory((const uchar*)contents.data(), size,
                                                          &width, &height, &components, 0))
            : static_cast<void*>(stbi_load_from_memory((const uchar*)contents.data(), size,
                                                       &width, &height, &components, 0));

        if (!pixelData)
        {
            const char* reason = stbi_failure_reason();
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Error decoding image: " + String(reason ? reason : "no reason"));
        }

        if (width <= 0 || height <= 0)
        {
            stbi_image_free(pixelData);
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        StringUtil::format("Image has invalid dimensions: %dx%d", width, height));
        }

        PixelFormat format = PF_UNKNOWN;
        switch( components )
        {
            case 1:
                format = is16Bit ? PF_L16 : PF_BYTE_L;
                break;
            case 2:
                // no dedicated 16-bit luminance-alpha format, use the 2-channel short format
                format = is16Bit ? PF_SHORT_GR : PF_BYTE_LA;
                break;
            case 3:
                format = is16Bit ? PF_SHORT_RGB : PF_BYTE_RGB;
                break;
            case 4:
                format = is16Bit ? PF_SHORT_RGBA : PF_BYTE_RGBA;
                break;
            default:
                stbi_image_free(pixelData);
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Only 1..4 channels supported");
                break;
        }
        
        image->loadDynamicImage(static_cast<uchar*>(pixelData), width, height, 1, format, true);
    }
    //---------------------------------------------------------------------    
    String STBIImageCodec::getType() const
    {
        return mType;
    }

#ifndef OGRE_STATIC_LIB
    extern "C" void _OgreSTBICodecExport dllStartPlugin();
    extern "C" void _OgreSTBICodecExport dllStopPlugin();

    extern "C" void _OgreSTBICodecExport dllStartPlugin()
    {
        STBIImageCodec::startup();
    }
    extern "C" void _OgreSTBICodecExport dllStopPlugin()
    {
        STBIImageCodec::shutdown();
    }
#endif
}
