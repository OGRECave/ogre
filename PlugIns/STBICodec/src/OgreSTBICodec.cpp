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

#if __OGRE_HAVE_NEON
#define STBI_NEON
#endif

#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
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

    STBIImageCodec::RegisteredCodecList STBIImageCodec::msCodecList;
    //---------------------------------------------------------------------
    void STBIImageCodec::startup(void)
    {
        stbi_convert_iphone_png_to_rgb(1);
        stbi_set_unpremultiply_on_load(1);

        LogManager::getSingleton().logMessage("stb_image - v2.27 - public domain image loader");
        
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
        if(mType != "png") {
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
                        "currently only encoding to PNG supported",
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
            size_t tempDataSize = image->getSize();
            tempData = OGRE_ALLOC_T(unsigned char, tempDataSize, Ogre::MEMCATEGORY_GENERAL);
            Ogre::PixelBox pbOut(image->getPixelBox(), format, tempData);
            PixelUtil::bulkPixelConversion(image->getPixelBox(), pbOut);

            inputData = tempData;
        }

        // Save to PNG
        int channels = PixelUtil::getComponentCount(format);
        int len;
        uchar* data = stbi_write_png_to_mem(inputData, image->getRowSpan(), image->getWidth(), image->getHeight(), channels, &len);

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
    //---------------------------------------------------------------------
    void STBIImageCodec::encodeToFile(const Any& input, const String& outFileName) const
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
    void STBIImageCodec::decode(const DataStreamPtr& input, const Any& output) const
    {
        auto image = any_cast<Image*>(output);
        String contents = input->getAsString();

        int width, height, components;
        stbi_uc* pixelData = stbi_load_from_memory((const uchar*)contents.data(),
                static_cast<int>(contents.size()), &width, &height, &components, 0);

        if (!pixelData)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
                "Error decoding image: " + String(stbi_failure_reason()),
                "STBIImageCodec::decode");
        }

        PixelFormat format = PF_UNKNOWN;
        switch( components )
        {
            case 1:
                format = PF_BYTE_L;
                break;
            case 2:
                format = PF_BYTE_LA;
                break;
            case 3:
                format = PF_BYTE_RGB;
                break;
            case 4:
                format = PF_BYTE_RGBA;
                break;
            default:
                stbi_image_free(pixelData);
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Only 1..4 channels supported");
                break;
        }
        
        image->loadDynamicImage(pixelData, width, height, 1, format, true);
    }
    //---------------------------------------------------------------------    
    String STBIImageCodec::getType() const
    {
        return mType;
    }
    //---------------------------------------------------------------------
    String STBIImageCodec::magicNumberToFileExt(const char *magicNumberPtr, size_t maxbytes) const
    {
        return BLANKSTRING;
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
