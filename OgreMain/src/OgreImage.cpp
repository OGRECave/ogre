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
#include "OgreImage.h"
#include "OgreException.h"
#include "OgreImageCodec.h"
#include "OgreColourValue.h"
#include "OgreMath.h"
#include "OgrePixelBox.h"
#include "OgreImageResampler.h"
#include "OgreImageDownsampler.h"
#include "OgreResourceGroupManager.h"
#include "OgreProfiler.h"

namespace Ogre {
    ImageCodec::~ImageCodec() {
    }

    //-----------------------------------------------------------------------------
    Image::Image()
        : mWidth(0),
        mHeight(0),
        mDepth(0),
        mBufSize(0),
        mNumMipmaps(0),
        mFlags(0),
        mFormat(PF_UNKNOWN),
        mBuffer( NULL ),
        mAutoDelete( true )
    {
    }

    //-----------------------------------------------------------------------------
    Image::Image( const Image &img )
        : mBuffer( NULL ),
        mAutoDelete( true )
    {
        // call assignment operator
        *this = img;
    }

    //-----------------------------------------------------------------------------
    Image::~Image()
    {
        freeMemory();
    }
    //---------------------------------------------------------------------
    void Image::freeMemory()
    {
        //Only delete if this was not a dynamic image (meaning app holds & destroys buffer)
        if( mBuffer && mAutoDelete )
        {
            OGRE_FREE(mBuffer, MEMCATEGORY_GENERAL);
            mBuffer = NULL;
        }

    }

    //-----------------------------------------------------------------------------
    Image & Image::operator = ( const Image &img )
    {
        freeMemory();
        mWidth = img.mWidth;
        mHeight = img.mHeight;
        mDepth = img.mDepth;
        mFormat = img.mFormat;
        mBufSize = img.mBufSize;
        mFlags = img.mFlags;
        mPixelSize = img.mPixelSize;
        mNumMipmaps = img.mNumMipmaps;
        mAutoDelete = img.mAutoDelete;
        //Only create/copy when previous data was not dynamic data
        if( mAutoDelete )
        {
            mBuffer = OGRE_ALLOC_T(uchar, mBufSize, MEMCATEGORY_GENERAL);
            memcpy( mBuffer, img.mBuffer, mBufSize );
        }
        else
        {
            mBuffer = img.mBuffer;
        }

        return *this;
    }

    //-----------------------------------------------------------------------------
    Image & Image::flipAroundY()
    {
        if( !mBuffer )
        {
            OGRE_EXCEPT( 
                Exception::ERR_INTERNAL_ERROR,
                "Can not flip an uninitialised texture",
                "Image::flipAroundY" );
        }
        
        mNumMipmaps = 0; // Image operations lose precomputed mipmaps

        uchar   *pTempBuffer1 = NULL;
        ushort  *pTempBuffer2 = NULL;
        uchar   *pTempBuffer3 = NULL;
        uint    *pTempBuffer4 = NULL;

        uchar   *src1 = mBuffer;
        ushort  *src2 = (ushort *)mBuffer;
        uchar   *src3 = mBuffer;
        uint    *src4 = (uint *)mBuffer;

        ushort y;
        switch (mPixelSize)
        {
        case 1:
            pTempBuffer1 = OGRE_ALLOC_T(uchar, mWidth * mHeight, MEMCATEGORY_GENERAL);
            for (y = 0; y < mHeight; y++)
            {
                uchar *dst1 = (pTempBuffer1 + ((y * mWidth) + mWidth - 1));
                for (ushort x = 0; x < mWidth; x++)
                    memcpy(dst1--, src1++, sizeof(uchar));
            }

            memcpy(mBuffer, pTempBuffer1, mWidth * mHeight * sizeof(uchar));
            OGRE_FREE(pTempBuffer1, MEMCATEGORY_GENERAL);
            break;

        case 2:
            pTempBuffer2 = OGRE_ALLOC_T(ushort, mWidth * mHeight, MEMCATEGORY_GENERAL);
            for (y = 0; y < mHeight; y++)
            {
                ushort *dst2 = (pTempBuffer2 + ((y * mWidth) + mWidth - 1));
                for (ushort x = 0; x < mWidth; x++)
                    memcpy(dst2--, src2++, sizeof(ushort));
            }

            memcpy(mBuffer, pTempBuffer2, mWidth * mHeight * sizeof(ushort));
            OGRE_FREE(pTempBuffer2, MEMCATEGORY_GENERAL);
            break;

        case 3:
            pTempBuffer3 = OGRE_ALLOC_T(uchar, mWidth * mHeight * 3, MEMCATEGORY_GENERAL);
            for (y = 0; y < mHeight; y++)
            {
                size_t offset = ((y * mWidth) + (mWidth - 1)) * 3;
                uchar *dst3 = pTempBuffer3;
                dst3 += offset;
                for (size_t x = 0; x < mWidth; x++)
                {
                    memcpy(dst3, src3, sizeof(uchar) * 3);
                    dst3 -= 3; src3 += 3;
                }
            }

            memcpy(mBuffer, pTempBuffer3, mWidth * mHeight * sizeof(uchar) * 3);
            OGRE_FREE(pTempBuffer3, MEMCATEGORY_GENERAL);
            break;

        case 4:
            pTempBuffer4 = OGRE_ALLOC_T(uint, mWidth * mHeight, MEMCATEGORY_GENERAL);
            for (y = 0; y < mHeight; y++)
            {
                uint *dst4 = (pTempBuffer4 + ((y * mWidth) + mWidth - 1));
                for (ushort x = 0; x < mWidth; x++)
                    memcpy(dst4--, src4++, sizeof(uint));
            }

            memcpy(mBuffer, pTempBuffer4, mWidth * mHeight * sizeof(uint));
            OGRE_FREE(pTempBuffer4, MEMCATEGORY_GENERAL);
            break;

        default:
            OGRE_EXCEPT( 
                Exception::ERR_INTERNAL_ERROR,
                "Unknown pixel depth",
                "Image::flipAroundY" );
            break;
        }

        return *this;

    }

    //-----------------------------------------------------------------------------
    Image & Image::flipAroundX()
    {
        if( !mBuffer )
        {
            OGRE_EXCEPT( 
                Exception::ERR_INTERNAL_ERROR,
                "Can not flip an uninitialised texture",
                "Image::flipAroundX" );
        }
        
        mNumMipmaps = 0; // Image operations lose precomputed mipmaps

        size_t rowSpan = mWidth * mPixelSize;

        uchar *pTempBuffer = OGRE_ALLOC_T(uchar, rowSpan * mHeight, MEMCATEGORY_GENERAL);
        uchar *ptr1 = mBuffer, *ptr2 = pTempBuffer + ( ( mHeight - 1 ) * rowSpan );

        for( ushort i = 0; i < mHeight; i++ )
        {
            memcpy( ptr2, ptr1, rowSpan );
            ptr1 += rowSpan; ptr2 -= rowSpan;
        }

        memcpy( mBuffer, pTempBuffer, rowSpan * mHeight);

        OGRE_FREE(pTempBuffer, MEMCATEGORY_GENERAL);

        return *this;
    }

    //-----------------------------------------------------------------------------
    Image& Image::loadDynamicImage( uchar* pData, uint32 uWidth, uint32 uHeight,
        uint32 depth,
        PixelFormat eFormat, bool autoDelete, 
        size_t numFaces, uint8 numMipMaps)
    {
        OgreProfileExhaustive( "Image::loadDynamicImage" );

        freeMemory();
        // Set image metadata
        mWidth = uWidth;
        mHeight = uHeight;
        mDepth = depth;
        mFormat = eFormat;
        mPixelSize = static_cast<uchar>(PixelUtil::getNumElemBytes( mFormat ));
        mNumMipmaps = numMipMaps;
        mFlags = 0;
        // Set flags
        if (PixelUtil::isCompressed(eFormat))
            mFlags |= IF_COMPRESSED;
        if (mDepth != 1)
            mFlags |= IF_3D_TEXTURE;
        if(numFaces == 6)
            mFlags |= IF_CUBEMAP;
        if(numFaces != 6 && numFaces != 1)
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
            "Number of faces currently must be 6 or 1.", 
            "Image::loadDynamicImage");

        mBufSize = calculateSize(numMipMaps, numFaces, uWidth, uHeight, depth, eFormat);
        mBuffer = pData;
        mAutoDelete = autoDelete;

        return *this;

    }

    //-----------------------------------------------------------------------------
    Image & Image::loadRawData(
        DataStreamPtr& stream, 
        uint32 uWidth, uint32 uHeight, uint32 uDepth,
        PixelFormat eFormat,
        size_t numFaces, size_t numMipMaps)
    {
        OgreProfileExhaustive( "Image::loadRawData" );

        size_t size = calculateSize(numMipMaps, numFaces, uWidth, uHeight, uDepth, eFormat);
        if (size != stream->size())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                "Stream size does not match calculated image size", 
                "Image::loadRawData");
        }

        uchar *buffer = OGRE_ALLOC_T(uchar, size, MEMCATEGORY_GENERAL);
        stream->read(buffer, size);

        return loadDynamicImage(buffer,
            uWidth, uHeight, uDepth,
            eFormat, true, numFaces, numMipMaps);

    }
    //-----------------------------------------------------------------------------
    Image & Image::load(const String& strFileName, const String& group)
    {
        OgreProfileExhaustive( "Image::load( string )" );

        String strExt;

        size_t pos = strFileName.find_last_of(".");
        if( pos != String::npos && pos < (strFileName.length() - 1))
        {
            strExt = strFileName.substr(pos+1);
        }

        DataStreamPtr encoded = ResourceGroupManager::getSingleton().openResource(strFileName, group);
        return load(encoded, strExt);

    }
    //-----------------------------------------------------------------------------
    void Image::save(const String& filename)
    {
        OgreProfileExhaustive( "Image::save" );

        if( !mBuffer )
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "No image data loaded", 
                "Image::save");
        }

        String strExt;
        size_t pos = filename.find_last_of(".");
        if( pos == String::npos )
            OGRE_EXCEPT(
            Exception::ERR_INVALIDPARAMS, 
            "Unable to save image file '" + filename + "' - invalid extension.",
            "Image::save" );

        while( pos != filename.length() - 1 )
            strExt += filename[++pos];

        Codec * pCodec = Codec::getCodec(strExt);
        if( !pCodec )
            OGRE_EXCEPT(
            Exception::ERR_INVALIDPARAMS, 
            "Unable to save image file '" + filename + "' - invalid extension.",
            "Image::save" );

        ImageCodec::ImageData* imgData = OGRE_NEW ImageCodec::ImageData();
        imgData->format = mFormat;
        imgData->height = mHeight;
        imgData->width = mWidth;
        imgData->depth = mDepth;
        imgData->size = mBufSize;
		imgData->num_mipmaps = mNumMipmaps;
        imgData->flags = mFlags;
        // Wrap in CodecDataPtr, this will delete
        Codec::CodecDataPtr codeDataPtr(imgData);
        // Wrap memory, be sure not to delete when stream destroyed
        MemoryDataStreamPtr wrapper(OGRE_NEW MemoryDataStream(mBuffer, mBufSize, false));

        pCodec->encodeToFile(wrapper, filename, codeDataPtr);
    }
    //---------------------------------------------------------------------
    DataStreamPtr Image::encode(const String& formatextension)
    {
        OgreProfileExhaustive( "Image::encode" );

        if( !mBuffer )
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "No image data loaded", 
                "Image::encode");
        }

        Codec * pCodec = Codec::getCodec(formatextension);
        if( !pCodec )
            OGRE_EXCEPT(
            Exception::ERR_INVALIDPARAMS, 
            "Unable to encode image data as '" + formatextension + "' - invalid extension.",
            "Image::encode" );

        ImageCodec::ImageData* imgData = OGRE_NEW ImageCodec::ImageData();
        imgData->format = mFormat;
        imgData->height = mHeight;
        imgData->width = mWidth;
        imgData->depth = mDepth;
        // Wrap in CodecDataPtr, this will delete
        Codec::CodecDataPtr codeDataPtr(imgData);
        // Wrap memory, be sure not to delete when stream destroyed
        MemoryDataStreamPtr wrapper(OGRE_NEW MemoryDataStream(mBuffer, mBufSize, false));

        return pCodec->encode(wrapper, codeDataPtr);
    }
    //-----------------------------------------------------------------------------
    Image & Image::load(DataStreamPtr& stream, const String& type )
    {
        OgreProfileExhaustive( "Image::load" );

        freeMemory();

        Codec * pCodec = 0;
        if (!type.empty())
        {
            // use named codec
            pCodec = Codec::getCodec(type);
        }
        else
        {
            // derive from magic number
            // read the first 32 bytes or file size, if less
            size_t magicLen = std::min(stream->size(), (size_t)32);
            char magicBuf[32];
            stream->read(magicBuf, magicLen);
            // return to start
            stream->seek(0);
            pCodec = Codec::getCodec(magicBuf, magicLen);

      if( !pCodec )
        OGRE_EXCEPT(
        Exception::ERR_INVALIDPARAMS, 
        "Unable to load image: Image format is unknown. Unable to identify codec. "
        "Check it or specify format explicitly.",
        "Image::load" );
        }

        Codec::DecodeResult res = pCodec->decode(stream);

        ImageCodec::ImageData* pData = 
            static_cast<ImageCodec::ImageData*>(res.second.getPointer());

        mWidth = pData->width;
        mHeight = pData->height;
        mDepth = pData->depth;
        mBufSize = pData->size;
        mNumMipmaps = pData->num_mipmaps;
        mFlags = pData->flags;

        // Get the format and compute the pixel size
        mFormat = pData->format;
        mPixelSize = static_cast<uchar>(PixelUtil::getNumElemBytes( mFormat ));
        // Just use internal buffer of returned memory stream
        mBuffer = res.first->getPtr();
        // Make sure stream does not delete
        res.first->setFreeOnClose(false);
        // make sure we delete
        mAutoDelete = true;

        return *this;
    }
    //---------------------------------------------------------------------
    String Image::getFileExtFromMagic(DataStreamPtr stream)
    {
        // read the first 32 bytes or file size, if less
        size_t magicLen = std::min(stream->size(), (size_t)32);
        char magicBuf[32];
        stream->read(magicBuf, magicLen);
        // return to start
        stream->seek(0);
        Codec* pCodec = Codec::getCodec(magicBuf, magicLen);

        if(pCodec)
            return pCodec->getType();
        else
            return BLANKSTRING;

    }
    //-----------------------------------------------------------------------------
    uchar* Image::getData()
    {
        return mBuffer;
    }

    //-----------------------------------------------------------------------------
    const uchar* Image::getData() const
    {
        assert( mBuffer );
        return mBuffer;
    }

    //-----------------------------------------------------------------------------
    size_t Image::getSize() const
    {
        return mBufSize;
    }

    //-----------------------------------------------------------------------------
    uint8 Image::getNumMipmaps() const
    {
        return mNumMipmaps;
    }

    //-----------------------------------------------------------------------------
    bool Image::hasFlag(const ImageFlags imgFlag) const
    {
        if(mFlags & imgFlag)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    //-----------------------------------------------------------------------------
    uint32 Image::getDepth() const
    {
        return mDepth;
    }
    //-----------------------------------------------------------------------------
    uint32 Image::getWidth() const
    {
        return mWidth;
    }

    //-----------------------------------------------------------------------------
    uint32 Image::getHeight() const
    {
        return mHeight;
    }
    //-----------------------------------------------------------------------------
    size_t Image::getNumFaces(void) const
    {
        if(hasFlag(IF_CUBEMAP))
            return 6;
        return 1;
    }
    //-----------------------------------------------------------------------------
    size_t Image::getRowSpan() const
    {
        return mWidth * mPixelSize;
    }

    //-----------------------------------------------------------------------------
    PixelFormat Image::getFormat() const
    {
        return mFormat;
    }

    //-----------------------------------------------------------------------------
    uchar Image::getBPP() const
    {
        return mPixelSize * 8;
    }

    //-----------------------------------------------------------------------------
    bool Image::getHasAlpha(void) const
    {
        return PixelUtil::getFlags(mFormat) & PFF_HASALPHA;
    }
    //-----------------------------------------------------------------------------
    void Image::applyGamma( uchar *buffer, Real gamma, size_t size, uchar bpp )
    {
        if( gamma == 1.0f )
            return;

        //NB only 24/32-bit supported
        if( bpp != 24 && bpp != 32 ) return;

        uint stride = bpp >> 3;
        
        uchar gammaramp[256];
        const Real exponent = 1.0f / gamma;
        for(int i = 0; i < 256; i++) {
            gammaramp[i] = static_cast<uchar>(Math::Pow(i/255.0f, exponent)*255+0.5f);
        }

        for( size_t i = 0, j = size / stride; i < j; i++, buffer += stride )
        {
            buffer[0] = gammaramp[buffer[0]];
            buffer[1] = gammaramp[buffer[1]];
            buffer[2] = gammaramp[buffer[2]];
        }
    }
    //-----------------------------------------------------------------------------
    void Image::resize(ushort width, ushort height, Filter filter)
    {
        OgreProfileExhaustive( "Image::resize" );

        // resizing dynamic images is not supported
        assert(mAutoDelete);
        assert(mDepth == 1);

        // reassign buffer to temp image, make sure auto-delete is true
        Image temp;
        temp.loadDynamicImage(mBuffer, mWidth, mHeight, 1, mFormat, true);
        // do not delete[] mBuffer!  temp will destroy it

        // set new dimensions, allocate new buffer
        mWidth = width;
        mHeight = height;
        mBufSize = PixelUtil::getMemorySize(mWidth, mHeight, 1, mFormat);
        mBuffer = OGRE_ALLOC_T(uchar, mBufSize, MEMCATEGORY_GENERAL);
        mNumMipmaps = 0; // Loses precomputed mipmaps

        // scale the image from temp into our resized buffer
        Image::scale(temp.getPixelBox(), getPixelBox(), filter);
    }
    //-----------------------------------------------------------------------------
    bool Image::generateMipmaps( bool gammaCorrected, Filter filter )
    {
        OgreProfileExhaustive( "Image::generateMipmaps" );

        // resizing dynamic images is not supported
        assert(mAutoDelete);
        assert(mDepth == 1);

        ImageDownsampler2D *downsampler2DFunc       = 0;
        ImageDownsamplerCube *downsamplerCubeFunc   = 0;
        ImageBlur2D *separableBlur2DFunc            = 0;

        switch( mFormat )
        {
        case PF_L8:
            if( !gammaCorrected )
            {
                downsampler2DFunc   = downscale2x_X8;
                downsamplerCubeFunc = downscale2x_X8_cube;
                separableBlur2DFunc = separableBlur_X8;
            }
            else
            {
                downsampler2DFunc   = downscale2x_sRGB_X8;
                downsamplerCubeFunc = downscale2x_sRGB_X8_cube;
                separableBlur2DFunc = separableBlur_sRGB_X8;
            }
            break;
        case PF_A8:
            if( !gammaCorrected )
            {
                downsampler2DFunc   = downscale2x_A8;
                downsamplerCubeFunc = downscale2x_A8_cube;
                separableBlur2DFunc = separableBlur_A8;
            }
            else
            {
                downsampler2DFunc   = downscale2x_sRGB_A8;
                downsamplerCubeFunc = downscale2x_sRGB_A8_cube;
                separableBlur2DFunc = separableBlur_sRGB_X8;
            }
            break;
        case PF_BYTE_LA:
            if( !gammaCorrected )
            {
                downsampler2DFunc   = downscale2x_XA88;
                downsamplerCubeFunc = downscale2x_XA88_cube;
                separableBlur2DFunc = separableBlur_XA88;
            }
            else
            {
                downsampler2DFunc   = downscale2x_sRGB_XA88;
                downsamplerCubeFunc = downscale2x_sRGB_XA88_cube;
                separableBlur2DFunc = separableBlur_sRGB_XA88;
            }
            break;
        case PF_RG8: case PF_R8G8_UINT:
            if( !gammaCorrected )
            {
                downsampler2DFunc   = downscale2x_XX88;
                downsamplerCubeFunc = downscale2x_XX88_cube;
                separableBlur2DFunc = separableBlur_XX88;
            }
            else
            {
                downsampler2DFunc   = downscale2x_sRGB_XX88;
                downsamplerCubeFunc = downscale2x_sRGB_XX88_cube;
                separableBlur2DFunc = separableBlur_sRGB_XX88;
            }
            break;
        case PF_R8G8B8: case PF_B8G8R8:
            if( !gammaCorrected )
            {
                downsampler2DFunc   = downscale2x_XXX888;
                downsamplerCubeFunc = downscale2x_XXX888_cube;
                separableBlur2DFunc = separableBlur_XXX888;
            }
            else
            {
                downsampler2DFunc   = downscale2x_sRGB_XXX888;
                downsamplerCubeFunc = downscale2x_sRGB_XXX888_cube;
                separableBlur2DFunc = separableBlur_sRGB_XXX888;
            }
            break;
        case PF_R8G8B8A8: case PF_B8G8R8A8:
            if( !gammaCorrected )
            {
                downsampler2DFunc   = downscale2x_XXXA8888;
                downsamplerCubeFunc = downscale2x_XXXA8888_cube;
                separableBlur2DFunc = separableBlur_XXXA8888;
            }
            else
            {
                downsampler2DFunc   = downscale2x_sRGB_XXXA8888;
                downsamplerCubeFunc = downscale2x_sRGB_XXXA8888_cube;
                separableBlur2DFunc = separableBlur_sRGB_XXXA8888;
            }
            break;
        case PF_A8B8G8R8: case PF_A8R8G8B8:
        case PF_X8B8G8R8: case PF_X8R8G8B8:
            if( !gammaCorrected )
            {
                downsampler2DFunc   = downscale2x_XXXA8888;
                downsamplerCubeFunc = downscale2x_XXXA8888_cube;
                separableBlur2DFunc = separableBlur_XXXA8888;
            }
            else
            {
                downsampler2DFunc   = downscale2x_sRGB_AXXX8888;
                downsamplerCubeFunc = downscale2x_sRGB_AXXX8888_cube;
                separableBlur2DFunc = separableBlur_sRGB_XXXA8888;
            }
            break;
        case PF_R8_SNORM: case PF_R8_SINT:
            downsampler2DFunc   = downscale2x_Signed_X8;
            downsamplerCubeFunc = downscale2x_Signed_X8_cube;
            separableBlur2DFunc = separableBlur_Signed_X8;
            break;
        case PF_R8G8_SNORM: case PF_R8G8_SINT:
            downsampler2DFunc   = downscale2x_Signed_XX88;
            downsamplerCubeFunc = downscale2x_Signed_XX88_cube;
            separableBlur2DFunc = separableBlur_Signed_XX88;
            break;
        case PF_R8G8B8_SNORM: case PF_R8G8B8_SINT:
            downsampler2DFunc   = downscale2x_Signed_XXX888;
            downsamplerCubeFunc = downscale2x_Signed_XXX888_cube;
            separableBlur2DFunc = separableBlur_Signed_XXX888;
            break;
        case PF_R8G8B8A8_SNORM: case PF_R8G8B8A8_SINT:
            downsampler2DFunc   = downscale2x_Signed_XXXA8888;
            downsamplerCubeFunc = downscale2x_Signed_XXXA8888_cube;
            separableBlur2DFunc = separableBlur_Signed_XXXA8888;
            break;
        case PF_FLOAT32_RGBA:
            downsampler2DFunc   = downscale2x_Float32_XXXA;
            downsamplerCubeFunc = downscale2x_Float32_XXXA_cube;
            separableBlur2DFunc = separableBlur_Float32_XXXA;
            break;
        case PF_FLOAT32_RGB:
            downsampler2DFunc   = downscale2x_Float32_XXX;
            downsamplerCubeFunc = downscale2x_Float32_XXX_cube;
            separableBlur2DFunc = separableBlur_Float32_XXX;
            break;
        case PF_FLOAT32_GR:
            downsampler2DFunc   = downscale2x_Float32_XX;
            downsamplerCubeFunc = downscale2x_Float32_XX_cube;
            separableBlur2DFunc = separableBlur_Float32_XX;
            break;
        case PF_FLOAT32_R:
            downsampler2DFunc   = downscale2x_Float32_X;
            downsamplerCubeFunc = downscale2x_Float32_X_cube;
            separableBlur2DFunc = separableBlur_Float32_X;
            break;
        default: //Keep compiler happy
            break;
        }

        if( (mDepth == 1 && getNumFaces() == 1 && !downsampler2DFunc) ||
            (getNumFaces() == 6 && (!downsamplerCubeFunc || filter == FILTER_GAUSSIAN_HIGH)) )
        {
            return false;
        }

        // reassign buffer to temp image, make sure auto-delete is true
        Image temp;
        temp.loadDynamicImage(mBuffer, mWidth, mHeight, mDepth, mFormat, true, getNumFaces());
        // do not delete[] mBuffer!  temp will destroy it

        // Allocate new buffer
        uint32 heighestRes = std::max( std::max( mWidth, mHeight ), mDepth );
#if (ANDROID || (OGRE_COMPILER == OGRE_COMPILER_MSVC && OGRE_COMP_VER < 1800))
        mNumMipmaps = static_cast<uint8>( floorf( logf( static_cast<float>(heighestRes) ) /
                                                  logf( 2.0f ) ) );
#else
        mNumMipmaps = static_cast<uint8>( floorf( log2f( static_cast<float>(heighestRes) ) ) );
#endif
        mBufSize    = calculateSize(mNumMipmaps, getNumFaces(), mWidth, mHeight, mDepth, mFormat);
        mBuffer     = OGRE_ALLOC_T(uchar, mBufSize, MEMCATEGORY_GENERAL);

        size_t numFaces = std::max<size_t>( mDepth, getNumFaces() );
        for( size_t i=0; i<numFaces; ++i )
        {
            PixelBox tempBox = temp.getPixelBox( i );
            memcpy( this->getPixelBox( i ).data, tempBox.data, tempBox.getConsecutiveSize() );
        }

        uint8 *tmpBuffer1 = 0;
        if( filter == FILTER_GAUSSIAN_HIGH )
        {
            const size_t bufSize = calculateSize( 1u, getNumFaces(), mWidth, mHeight, mDepth, mFormat );
            tmpBuffer1 = OGRE_ALLOC_T( uint8, bufSize, MEMCATEGORY_GENERAL );
        }

        uint32 dstWidth  = mWidth;
        uint32 dstHeight = mHeight;

        int filterIdx = 1;

        switch( filter )
        {
        case FILTER_NEAREST:
            filterIdx = 0; break;
        case FILTER_LINEAR:
        case FILTER_BILINEAR:
            filterIdx = 1; break;
        case FILTER_GAUSSIAN:
            filterIdx = 2; break;
        case FILTER_GAUSSIAN_HIGH:
            filterIdx = 1; break;
        default: // Keep compiler happy
            break;
        }

        const FilterKernel &chosenFilter = c_filterKernels[filterIdx];

        for( uint8 i=1; i<mNumMipmaps + 1; ++i )
        {
            uint32 srcWidth    = dstWidth;
            uint32 srcHeight   = dstHeight;
            dstWidth   = std::max<uint32>( 1, dstWidth >> 1 );
            dstHeight  = std::max<uint32>( 1, dstHeight >> 1 );

            if( hasFlag( IF_CUBEMAP ) )
            {
                uint8 const *upFaces[6];
                for( size_t j=0; j<6; ++j )
                    upFaces[j] = reinterpret_cast<uint8*>( this->getPixelBox( j, i - 1 ).data );

                for( size_t j=0; j<6; ++j )
                {
                    PixelBox downFace = this->getPixelBox( j, i );
                    (*downsamplerCubeFunc)( reinterpret_cast<uint8*>( downFace.data ), upFaces,
                                            dstWidth, dstHeight, srcWidth, srcHeight,
                                            chosenFilter.kernel,
                                            chosenFilter.kernelStartX, chosenFilter.kernelEndX,
                                            chosenFilter.kernelStartY, chosenFilter.kernelEndY,
                                            j );
                }
            }
            else
            {
                if( filter != FILTER_GAUSSIAN_HIGH )
                {
                    (*downsampler2DFunc)( reinterpret_cast<uint8*>( this->getPixelBox( 0, i ).data ),
                                          reinterpret_cast<uint8*>( this->getPixelBox( 0, i - 1 ).data ),
                                          dstWidth, dstHeight, srcWidth,
                                          chosenFilter.kernel,
                                          chosenFilter.kernelStartX, chosenFilter.kernelEndX,
                                          chosenFilter.kernelStartY, chosenFilter.kernelEndY );
                }
                else
                {
                    //Copy 'this' to temp
                    memcpy( temp.mBuffer, this->getPixelBox( 0, i - 1 ).data,
                            this->getPixelBox( 0, i - 1 ).getConsecutiveSize() );

                    //The image right now is in both 'this' and temp. We can't touch 'this',
                    //So we blur temp, and use tmpBuffer1 to store intermediate results
                    const FilterSeparableKernel &separableKernel = c_filterSeparableKernels[0];
                    (*separableBlur2DFunc)( tmpBuffer1,
                                            reinterpret_cast<uint8*>( temp.mBuffer ),
                                            srcWidth, srcHeight,
                                            separableKernel.kernel,
                                            separableKernel.kernelStart, separableKernel.kernelEnd );
                    //Filter again...
                    (*separableBlur2DFunc)( tmpBuffer1,
                                            reinterpret_cast<uint8*>( temp.mBuffer ),
                                            srcWidth, srcHeight,
                                            separableKernel.kernel,
                                            separableKernel.kernelStart, separableKernel.kernelEnd );

                    //Now that temp is blurred, bilinear downsample its contents into 'this'.
                    (*downsampler2DFunc)( reinterpret_cast<uint8*>( this->getPixelBox( 0, i ).data ),
                                          reinterpret_cast<uint8*>( temp.mBuffer ),
                                          dstWidth, dstHeight, srcWidth,
                                          chosenFilter.kernel,
                                          chosenFilter.kernelStartX, chosenFilter.kernelEndX,
                                          chosenFilter.kernelStartY, chosenFilter.kernelEndY );
                }
            }
        }

        if( tmpBuffer1 )
        {
            OGRE_FREE( tmpBuffer1, MEMCATEGORY_GENERAL );
            tmpBuffer1 = 0;
        }

        return true;
    }
    //-----------------------------------------------------------------------
    void Image::scale(const PixelBox &src, const PixelBox &scaled, Filter filter) 
    {
        OgreProfileExhaustive( "Image::scale" );

        assert(PixelUtil::isAccessible(src.format));
        assert(PixelUtil::isAccessible(scaled.format));
        MemoryDataStreamPtr buf; // For auto-delete
        PixelBox temp;
        switch (filter) 
        {
        default:
        case FILTER_NEAREST:
            if(src.format == scaled.format) 
            {
                // No intermediate buffer needed
                temp = scaled;
            }
            else
            {
                // Allocate temporary buffer of destination size in source format 
                temp = PixelBox(scaled.getWidth(), scaled.getHeight(), scaled.getDepth(), src.format);
                buf.bind(OGRE_NEW MemoryDataStream(temp.getConsecutiveSize()));
                temp.data = buf->getPtr();
            }
            // super-optimized: no conversion
            switch (PixelUtil::getNumElemBytes(src.format)) 
            {
            case 1: NearestResampler<1>::scale(src, temp); break;
            case 2: NearestResampler<2>::scale(src, temp); break;
            case 3: NearestResampler<3>::scale(src, temp); break;
            case 4: NearestResampler<4>::scale(src, temp); break;
            case 6: NearestResampler<6>::scale(src, temp); break;
            case 8: NearestResampler<8>::scale(src, temp); break;
            case 12: NearestResampler<12>::scale(src, temp); break;
            case 16: NearestResampler<16>::scale(src, temp); break;
            default:
                // never reached
                assert(false);
            }
            if(temp.data != scaled.data)
            {
                // Blit temp buffer
                PixelUtil::bulkPixelConversion(temp, scaled);
            }
            break;

        case FILTER_LINEAR:
        case FILTER_BILINEAR:
            switch (src.format) 
            {
            case PF_L8: case PF_A8: case PF_BYTE_LA:
            case PF_R8G8B8: case PF_B8G8R8:
            case PF_R8G8B8A8: case PF_B8G8R8A8:
            case PF_A8B8G8R8: case PF_A8R8G8B8:
            case PF_X8B8G8R8: case PF_X8R8G8B8:
                if(src.format == scaled.format) 
                {
                    // No intermediate buffer needed
                    temp = scaled;
                }
                else
                {
                    // Allocate temp buffer of destination size in source format 
                    temp = PixelBox(scaled.getWidth(), scaled.getHeight(), scaled.getDepth(), src.format);
                    buf.bind(OGRE_NEW MemoryDataStream(temp.getConsecutiveSize()));
                    temp.data = buf->getPtr();
                }
                // super-optimized: byte-oriented math, no conversion
                switch (PixelUtil::getNumElemBytes(src.format)) 
                {
                case 1: LinearResampler_Byte<1>::scale(src, temp); break;
                case 2: LinearResampler_Byte<2>::scale(src, temp); break;
                case 3: LinearResampler_Byte<3>::scale(src, temp); break;
                case 4: LinearResampler_Byte<4>::scale(src, temp); break;
                default:
                    // never reached
                    assert(false);
                }
                if(temp.data != scaled.data)
                {
                    // Blit temp buffer
                    PixelUtil::bulkPixelConversion(temp, scaled);
                }
                break;
            case PF_FLOAT32_RGB:
            case PF_FLOAT32_RGBA:
                if (scaled.format == PF_FLOAT32_RGB || scaled.format == PF_FLOAT32_RGBA)
                {
                    // float32 to float32, avoid unpack/repack overhead
                    LinearResampler_Float32::scale(src, scaled);
                    break;
                }
                // else, fall through
            default:
                // non-optimized: floating-point math, performs conversion but always works
                LinearResampler::scale(src, scaled);
            }
            break;
        }
    }

    //-----------------------------------------------------------------------------    

    ColourValue Image::getColourAt(size_t x, size_t y, size_t z) const
    {
        ColourValue rval;
        PixelUtil::unpackColour(&rval, mFormat, &mBuffer[mPixelSize * (z * mWidth * mHeight + mWidth * y + x)]);
        return rval;
    }

    //-----------------------------------------------------------------------------    
    
    void Image::setColourAt(ColourValue const &cv, size_t x, size_t y, size_t z)
    {
        unsigned char pixelSize = PixelUtil::getNumElemBytes(getFormat());
        PixelUtil::packColour(cv, getFormat(), &((unsigned char *)getData())[pixelSize * (z * getWidth() * getHeight() + y * getWidth() + x)]);
    }

    //-----------------------------------------------------------------------------    

    PixelBox Image::getPixelBox(size_t face, size_t mipmap) const
    {
        // Image data is arranged as:
        // face 0, top level (mip 0)
        // face 0, mip 1
        // face 0, mip 2
        // face 1, top level (mip 0)
        // face 1, mip 1
        // face 1, mip 2
        // etc
        if(mipmap > getNumMipmaps())
            OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
            "Mipmap index out of range",
            "Image::getPixelBox" ) ;
        if(face >= getNumFaces())
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Face index out of range",
            "Image::getPixelBox");
        // Calculate mipmap offset and size
        uint8 *offset = const_cast<uint8*>(getData());
        // Base offset is number of full faces
        uint32 width = getWidth(), height=getHeight(), depth=getDepth();
        size_t numMips = getNumMipmaps();

        // Figure out the offsets 
        size_t fullFaceSize = 0;
        size_t finalFaceSize = 0;
        uint32 finalWidth = 0, finalHeight = 0, finalDepth = 0;
        for(size_t mip=0; mip <= numMips; ++mip)
        {
            if (mip == mipmap)
            {
                finalFaceSize = fullFaceSize;
                finalWidth = width;
                finalHeight = height;
                finalDepth = depth;
            }
            fullFaceSize += PixelUtil::getMemorySize(width, height, depth, getFormat());

            /// Half size in each dimension
            if(width!=1) width /= 2;
            if(height!=1) height /= 2;
            if(depth!=1) depth /= 2;
        }
        // Advance pointer by number of full faces, plus mip offset into
        offset += face * fullFaceSize;
        offset += finalFaceSize;
        // Return subface as pixelbox
        PixelBox src(finalWidth, finalHeight, finalDepth, getFormat(), offset);
        return src;
    }
    //-----------------------------------------------------------------------------    
    size_t Image::calculateSize(size_t mipmaps, size_t faces, uint32 width, uint32 height, uint32 depth, 
        PixelFormat format)
    {
        size_t size = 0;
        for(size_t mip=0; mip<=mipmaps; ++mip)
        {
            size += PixelUtil::getMemorySize(width, height, depth, format)*faces; 
            if(width!=1) width /= 2;
            if(height!=1) height /= 2;
            if(depth!=1) depth /= 2;
        }
        return size;
    }
    //---------------------------------------------------------------------
    Image & Image::loadTwoImagesAsRGBA(const String& rgbFilename, const String& alphaFilename,
        const String& groupName, PixelFormat fmt)
    {
        Image rgb, alpha;

        rgb.load(rgbFilename, groupName);
        alpha.load(alphaFilename, groupName);

        return combineTwoImagesAsRGBA(rgb, alpha, fmt);

    }
    //---------------------------------------------------------------------
    Image & Image::loadTwoImagesAsRGBA(DataStreamPtr& rgbStream, DataStreamPtr& alphaStream,
        PixelFormat fmt, const String& rgbType, const String& alphaType)
    {
        Image rgb, alpha;

        rgb.load(rgbStream, rgbType);
        alpha.load(alphaStream, alphaType);

        return combineTwoImagesAsRGBA(rgb, alpha, fmt);

    }
    //---------------------------------------------------------------------
    Image & Image::combineTwoImagesAsRGBA(const Image& rgb, const Image& alpha, PixelFormat fmt)
    {
        OgreProfileExhaustive( "Image::combineTwoImagesAsRGBA" );

        // the images should be the same size, have the same number of mipmaps
        if (rgb.getWidth() != alpha.getWidth() ||
            rgb.getHeight() != alpha.getHeight() ||
            rgb.getDepth() != alpha.getDepth())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                "Images must be the same dimensions", "Image::combineTwoImagesAsRGBA");
        }
        if (rgb.getNumMipmaps() != alpha.getNumMipmaps() ||
            rgb.getNumFaces() != alpha.getNumFaces())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                "Images must have the same number of surfaces (faces & mipmaps)", 
                "Image::combineTwoImagesAsRGBA");
        }
        // Format check
        if (PixelUtil::getComponentCount(fmt) != 4)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                "Target format must have 4 components", 
                "Image::combineTwoImagesAsRGBA");
        }
        if (PixelUtil::isCompressed(fmt) || PixelUtil::isCompressed(rgb.getFormat()) 
            || PixelUtil::isCompressed(alpha.getFormat()))
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                "Compressed formats are not supported in this method", 
                "Image::combineTwoImagesAsRGBA");
        }

        freeMemory();

        mWidth = rgb.getWidth();
        mHeight = rgb.getHeight();
        mDepth = rgb.getDepth();
        mFormat = fmt;
        mNumMipmaps = rgb.getNumMipmaps();
        size_t numFaces = rgb.getNumFaces();

        // Set flags
        mFlags = 0;
        if (mDepth != 1)
            mFlags |= IF_3D_TEXTURE;
        if(numFaces == 6)
            mFlags |= IF_CUBEMAP;

        mBufSize = calculateSize(mNumMipmaps, numFaces, mWidth, mHeight, mDepth, mFormat);

        mPixelSize = static_cast<uchar>(PixelUtil::getNumElemBytes( mFormat ));

        mBuffer = static_cast<uchar*>(OGRE_MALLOC(mBufSize, MEMCATEGORY_GENERAL));

        // make sure we delete
        mAutoDelete = true;


        for (size_t face = 0; face < numFaces; ++face)
        {
            for (uint8 mip = 0; mip <= mNumMipmaps; ++mip)
            {
                // convert the RGB first
                PixelBox srcRGB = rgb.getPixelBox(face, mip);
                PixelBox dst = getPixelBox(face, mip);
                PixelUtil::bulkPixelConversion(srcRGB, dst);

                // now selectively add the alpha
                PixelBox srcAlpha = alpha.getPixelBox(face, mip);
                uchar* psrcAlpha = static_cast<uchar*>(srcAlpha.data);
                uchar* pdst = static_cast<uchar*>(dst.data);
                for (size_t d = 0; d < mDepth; ++d)
                {
                    for (size_t y = 0; y < mHeight; ++y)
                    {
                        for (size_t x = 0; x < mWidth; ++x)
                        {
                            ColourValue colRGBA, colA;
                            // read RGB back from dest to save having another pointer
                            PixelUtil::unpackColour(&colRGBA, mFormat, pdst);
                            PixelUtil::unpackColour(&colA, alpha.getFormat(), psrcAlpha);

                            // combine RGB from alpha source texture
                            colRGBA.a = (colA.r + colA.g + colA.b) / 3.0f;

                            PixelUtil::packColour(colRGBA, mFormat, pdst);
                            
                            psrcAlpha += PixelUtil::getNumElemBytes(alpha.getFormat());
                            pdst += PixelUtil::getNumElemBytes(mFormat);

                        }
                    }
                }
                

            }
        }

        return *this;

    }
    //---------------------------------------------------------------------

    
}
