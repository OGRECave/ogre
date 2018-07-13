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

#include "OgreStableHeaders.h"

#include "OgreSTBICodec.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreDataStream.h"

#if __OGRE_HAVE_NEON
#define STBI_NEON
#endif

#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include "stbi/stb_image.h"

namespace Ogre {

    STBIImageCodec::RegisteredCodecList STBIImageCodec::msCodecList;
    //---------------------------------------------------------------------
    void STBIImageCodec::startup(void)
    {
        stbi_convert_iphone_png_to_rgb(1);
        stbi_set_unpremultiply_on_load(1);

        LogManager::getSingleton().logMessage(LML_NORMAL, "stb_image - v2.02 - public domain JPEG/PNG reader");
        
        // Register codecs
        String exts = "jpeg,jpg,png,bmp,psd,tga,gif,pic,ppm,pgm";
        StringVector extsVector = StringUtil::split(exts, ",");
        for (StringVector::iterator v = extsVector.begin(); v != extsVector.end(); ++v)
        {
            ImageCodec* codec = OGRE_NEW STBIImageCodec(*v);
            msCodecList.push_back(codec);
            Codec::registerCodec(codec);
        }
        
        StringStream strExt;
        strExt << "Supported formats: " << exts;
        
        LogManager::getSingleton().logMessage(
            LML_NORMAL,
            strExt.str());
    }
    //---------------------------------------------------------------------
    void STBIImageCodec::shutdown(void)
    {
        for (RegisteredCodecList::iterator i = msCodecList.begin();
            i != msCodecList.end(); ++i)
        {
            Codec::unregisterCodec(*i);
            OGRE_DELETE *i;
        }
        msCodecList.clear();
    }
    //---------------------------------------------------------------------
    STBIImageCodec::STBIImageCodec(const String &type):
        mType(type)
    { 
    }
    //---------------------------------------------------------------------
    DataStreamPtr STBIImageCodec::encode(MemoryDataStreamPtr& input, Codec::CodecDataPtr& pData) const
    {        
        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
                    "STBI encoding not supported",
                    "STBIImageCodec::encode" ) ;

        return DataStreamPtr();
    }
    //---------------------------------------------------------------------
    void STBIImageCodec::encodeToFile(MemoryDataStreamPtr& input,
        const String& outFileName, Codec::CodecDataPtr& pData) const
    {
        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
                    "STBI encoding not supported",
                    "STBIImageCodec::encodeToFile" ) ;
    }
    //---------------------------------------------------------------------
    Codec::DecodeResult STBIImageCodec::decode(DataStreamPtr& input) const
    {
        // Buffer stream into memory (TODO: override IO functions instead?)
        MemoryDataStream memStream(input, true);

        int width, height, components;
        stbi_uc* pixelData = stbi_load_from_memory(memStream.getPtr(), static_cast<int>(memStream.size()), &width, &height, &components, 0);
        
        
        if (!pixelData)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
                "Error decoding image: " + String(stbi_failure_reason()),
                "STBIImageCodec::decode");
        }

        ImageData* imgData = OGRE_NEW ImageData();
        MemoryDataStreamPtr output;

        imgData->depth = 1; // only 2D formats handled by this codec
        imgData->width = width;
        imgData->height = height;
        imgData->num_mipmaps = 0; // no mipmaps in non-DDS 
        imgData->flags = 0;

        switch( components )
        {
            case 1:
                imgData->format = PF_BYTE_L;
                break;
            case 2:
                imgData->format = PF_BYTE_LA;
                break;
            case 3:
                imgData->format = PF_BYTE_RGB;
                break;
            case 4:
                imgData->format = PF_BYTE_RGBA;
                break;
            default:
                stbi_image_free(pixelData);
                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
                            "Unknown or unsupported image format",
                            "STBIImageCodec::decode");
                break;
        }
        
        size_t dstPitch = imgData->width * PixelUtil::getNumElemBytes(imgData->format);
        imgData->size = dstPitch * imgData->height;
        output.bind(OGRE_NEW MemoryDataStream(imgData->size));
        
        uchar* pDst = output->getPtr();
        memcpy(pDst, pixelData, imgData->size);
        
        stbi_image_free(pixelData);

        DecodeResult ret;
        ret.first = output;
        ret.second = CodecDataPtr(imgData);
        return ret;
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
}
