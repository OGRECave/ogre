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
#include "OgreImageCodec.h"
#include "OgreImageResampler.h"

namespace Ogre {
    //-----------------------------------------------------------------------------
    Image::Image(PixelFormat format, uint32 width, uint32 height, uint32 depth, uchar* buffer, bool autoDelete)
        : mWidth(0),
        mHeight(0),
        mDepth(0),
        mNumMipmaps(0),
        mBufSize(0),
        mFlags(0),
        mFormat(format),
        mBuffer( NULL ),
        mAutoDelete( true )
    {
        if (format == PF_UNKNOWN)
            return;

        size_t size = calculateSize(0, 1,  width, height, depth, mFormat);

        if (size == 0)
            return;

        if (!buffer)
            buffer = OGRE_ALLOC_T(uchar, size, MEMCATEGORY_GENERAL);
        loadDynamicImage(buffer, width, height, depth, format, autoDelete);
    }

    void Image::create(PixelFormat format, uint32 width, uint32 height, uint32 depth, uint32 numFaces,
                       uint32 numMipMaps)
    {
        size_t size = calculateSize(numMipMaps, numFaces, width, height, depth, format);
        if (!mAutoDelete || !mBuffer || mBufSize != size)
        {
            freeMemory();
            mBuffer = new uchar[size]; // allocate
        }

        // make sure freeMemory() does nothing, we set this true immediately after
        mAutoDelete = false;
        loadDynamicImage(mBuffer, width, height, depth, format, true, numFaces, numMipMaps);
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
    Image& Image::operator=(const Image& img)
    {
        if (this == &img)
            return *this;

        // Only create & copy when other data was owning
        if (img.mBuffer && img.mAutoDelete)
        {
            create(img.mFormat, img.mWidth, img.mHeight, img.mDepth, img.getNumFaces(), img.mNumMipmaps);
            memcpy(mBuffer, img.mBuffer, mBufSize);
        }
        else
        {
            loadDynamicImage(img.mBuffer, img.mWidth, img.mHeight, img.mDepth, img.mFormat, false,
                             img.getNumFaces(), img.mNumMipmaps);
        }

        return *this;
    }

    void Image::setTo(const ColourValue& col)
    {
        OgreAssert(mBuffer, "No image data loaded");
        if(col == ColourValue::ZERO)
        {
            memset(mBuffer, 0, getSize());
            return;
        }

        uchar rawCol[4 * sizeof(float)]; // max packed size currently is 4*float
        PixelUtil::packColour(col, mFormat, rawCol);
        for(size_t p = 0; p < mBufSize; p += mPixelSize)
        {
            memcpy(mBuffer + p, rawCol, mPixelSize);
        }
    }

    //-----------------------------------------------------------------------------
    Image & Image::flipAroundY()
    {
        OgreAssert(mBuffer, "No image data loaded");
        mNumMipmaps = 0; // Image operations lose precomputed mipmaps

        uint32 y;
        switch (mPixelSize)
        {
        case 1:
            for (y = 0; y < mHeight; y++)
            {
                std::reverse(mBuffer + mWidth * y, mBuffer + mWidth * (y + 1));
            }
            break;

        case 2:
            for (y = 0; y < mHeight; y++)
            {
                std::reverse((ushort*)mBuffer + mWidth * y, (ushort*)mBuffer + mWidth * (y + 1));
            }
            break;

        case 3:
            typedef uchar uchar3[3];
            for (y = 0; y < mHeight; y++)
            {
                std::reverse((uchar3*)mBuffer + mWidth * y, (uchar3*)mBuffer + mWidth * (y + 1));
            }
            break;

        case 4:
            for (y = 0; y < mHeight; y++)
            {
                std::reverse((uint*)mBuffer + mWidth * y, (uint*)mBuffer + mWidth * (y + 1));
            }
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
        OgreAssert(mBuffer, "No image data loaded");
        
        mNumMipmaps = 0; // Image operations lose precomputed mipmaps
        PixelUtil::bulkPixelVerticalFlip(getPixelBox());

        return *this;
    }

    //-----------------------------------------------------------------------------
    Image& Image::loadDynamicImage(uchar* pData, uint32 uWidth, uint32 uHeight, uint32 depth,
                                   PixelFormat eFormat, bool autoDelete, uint32 numFaces, uint32 numMipMaps)
    {

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
        OgreAssert(numFaces == 6 || numFaces == 1, "Invalid number of faces");

        mBufSize = calculateSize(numMipMaps, numFaces, uWidth, uHeight, depth, eFormat);
        mBuffer = pData;
        mAutoDelete = autoDelete;

        return *this;
    }

    //-----------------------------------------------------------------------------
    Image& Image::loadRawData(const DataStreamPtr& stream, uint32 uWidth, uint32 uHeight, uint32 uDepth,
                              PixelFormat eFormat, uint32 numFaces, uint32 numMipMaps)
    {

        size_t size = calculateSize(numMipMaps, numFaces, uWidth, uHeight, uDepth, eFormat);
        OgreAssert(size == stream->size(), "Wrong stream size");

        uchar *buffer = OGRE_ALLOC_T(uchar, size, MEMCATEGORY_GENERAL);
        stream->read(buffer, size);

        return loadDynamicImage(buffer,
            uWidth, uHeight, uDepth,
            eFormat, true, numFaces, numMipMaps);

    }
    //-----------------------------------------------------------------------------
    Image & Image::load(const String& strFileName, const String& group)
    {

        String strExt;

        size_t pos = strFileName.find_last_of('.');
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
        OgreAssert(mBuffer, "No image data loaded");

        String base, ext;
        StringUtil::splitBaseFilename(filename, base, ext);

        // getCodec throws when no codec is found
        Codec::getCodec(ext)->encodeToFile(this, filename);
    }
    //---------------------------------------------------------------------
    DataStreamPtr Image::encode(const String& formatextension)
    {
        OgreAssert(mBuffer, "No image data loaded");
        // getCodec throws when no codec is found
        return Codec::getCodec(formatextension)->encode(this);
    }
    //-----------------------------------------------------------------------------
    Image & Image::load(const DataStreamPtr& stream, const String& type )
    {
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

            if (!pCodec)
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                            "Unable to load image: Image format is unknown. Unable to identify codec. "
                            "Check it or specify format explicitly.");
        }

        pCodec->decode(stream, this);

        // compute the pixel size
        mPixelSize = static_cast<uchar>(PixelUtil::getNumElemBytes( mFormat ));
        // make sure we delete
        mAutoDelete = true;

        return *this;
    }
    //---------------------------------------------------------------------
    String Image::getFileExtFromMagic(const DataStreamPtr stream)
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
    bool Image::getHasAlpha(void) const
    {
        return PixelUtil::getFlags(mFormat) & PFF_HASALPHA;
    }
    //-----------------------------------------------------------------------------
    void Image::applyGamma( uchar *buffer, Real gamma, size_t size, uchar bpp )
    {
        if( gamma == 1.0f )
            return;

        OgreAssert( bpp == 24 || bpp == 32, "");

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
        OgreAssert(mAutoDelete, "resizing dynamic images is not supported");
        OgreAssert(mDepth == 1, "only 2D formats supported");

        // reassign buffer to temp image, make sure auto-delete is true
        Image temp(mFormat, mWidth, mHeight, 1, mBuffer, true);

        // do not delete[] mBuffer!  temp will destroy it
        mBuffer = 0;

        // set new dimensions, allocate new buffer
        create(mFormat, width, height); // Loses precomputed mipmaps

        // scale the image from temp into our resized buffer
        Image::scale(temp.getPixelBox(), getPixelBox(), filter);
    }
    //-----------------------------------------------------------------------
    void Image::scale(const PixelBox &src, const PixelBox &scaled, Filter filter) 
    {
        assert(PixelUtil::isAccessible(src.format));
        assert(PixelUtil::isAccessible(scaled.format));
        Image buf; // For auto-delete
        // Assume no intermediate buffer needed
        PixelBox temp = scaled;
        switch (filter) 
        {
        default:
        case FILTER_NEAREST:
            if(src.format != scaled.format)
            {
                // Allocate temporary buffer of destination size in source format 
                buf.create(src.format, scaled.getWidth(), scaled.getHeight(), scaled.getDepth());
                temp = buf.getPixelBox();
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

        case FILTER_BILINEAR:
            switch (src.format) 
            {
            case PF_L8: case PF_R8: case PF_A8: case PF_BYTE_LA:
            case PF_R8G8B8: case PF_B8G8R8:
            case PF_R8G8B8A8: case PF_B8G8R8A8:
            case PF_A8B8G8R8: case PF_A8R8G8B8:
            case PF_X8B8G8R8: case PF_X8R8G8B8:
                if(src.format != scaled.format)
                {
                    // Allocate temp buffer of destination size in source format 
                    buf.create(src.format, scaled.getWidth(), scaled.getHeight(), scaled.getDepth());
                    temp = buf.getPixelBox();
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

    ColourValue Image::getColourAt(uint32 x, uint32 y, uint32 z) const
    {
        ColourValue rval;
        PixelUtil::unpackColour(&rval, mFormat, getData(x, y, z));
        return rval;
    }

    //-----------------------------------------------------------------------------    
    
    void Image::setColourAt(ColourValue const &cv, uint32 x, uint32 y, uint32 z)
    {
        PixelUtil::packColour(cv, mFormat, getData(x, y, z));
    }

    //-----------------------------------------------------------------------------    

    PixelBox Image::getPixelBox(uint32 face, uint32 mipmap) const
    {
        // Image data is arranged as:
        // face 0, top level (mip 0)
        // face 0, mip 1
        // face 0, mip 2
        // face 1, top level (mip 0)
        // face 1, mip 1
        // face 1, mip 2
        // etc
        OgreAssert(mipmap <= getNumMipmaps(), "out of range");
        OgreAssert(face < getNumFaces(), "out of range");
        // Calculate mipmap offset and size
        uint8 *offset = mBuffer;
        // Base offset is number of full faces
        uint32 width = getWidth(), height=getHeight(), depth=getDepth();
        uint32 numMips = getNumMipmaps();

        // Figure out the offsets 
        size_t fullFaceSize = 0;
        size_t finalFaceSize = 0;
        uint32 finalWidth = 0, finalHeight = 0, finalDepth = 0;
        for(uint32 mip=0; mip <= numMips; ++mip)
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
    size_t Image::calculateSize(uint32 mipmaps, uint32 faces, uint32 width, uint32 height, uint32 depth,
                                PixelFormat format)
    {
        size_t size = 0;
        for(uint32 mip=0; mip<=mipmaps; ++mip)
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
    Image& Image::loadTwoImagesAsRGBA(const DataStreamPtr& rgbStream,
                                      const DataStreamPtr& alphaStream, PixelFormat fmt,
                                      const String& rgbType, const String& alphaType)
    {
        Image rgb, alpha;

        rgb.load(rgbStream, rgbType);
        alpha.load(alphaStream, alphaType);

        return combineTwoImagesAsRGBA(rgb, alpha, fmt);

    }
    //---------------------------------------------------------------------
    Image & Image::combineTwoImagesAsRGBA(const Image& rgb, const Image& alpha, PixelFormat fmt)
    {
        // the images should be the same size, have the same number of mipmaps
        OgreAssert(rgb.getWidth() == alpha.getWidth() && rgb.getHeight() == alpha.getHeight() &&
                       rgb.getDepth() == alpha.getDepth(),
                   "Images must be the same dimensions");
        OgreAssert(rgb.getNumMipmaps() == alpha.getNumMipmaps() && rgb.getNumFaces() == alpha.getNumFaces(),
                   "Images must have the same number of surfaces");

        // Format check
        OgreAssert(PixelUtil::getComponentCount(fmt) == 4, "Target format must have 4 components");

        OgreAssert(!(PixelUtil::isCompressed(fmt) || PixelUtil::isCompressed(rgb.getFormat()) ||
                     PixelUtil::isCompressed(alpha.getFormat())),
                   "Compressed formats are not supported in this method");

        uint32 numFaces = rgb.getNumFaces();
        create(fmt, rgb.getWidth(), rgb.getHeight(), rgb.getDepth(), numFaces, rgb.getNumMipmaps());

        for (uint32 face = 0; face < numFaces; ++face)
        {
            for (uint8 mip = 0; mip <= mNumMipmaps; ++mip)
            {
                // convert the RGB first
                PixelBox srcRGB = rgb.getPixelBox(face, mip);
                PixelBox dst = getPixelBox(face, mip);
                PixelUtil::bulkPixelConversion(srcRGB, dst);

                // now selectively add the alpha
                PixelBox srcAlpha = alpha.getPixelBox(face, mip);
                uchar* psrcAlpha = srcAlpha.data;
                uchar* pdst = dst.data;
                for (uint32 d = 0; d < mDepth; ++d)
                {
                    for (uint32 y = 0; y < mHeight; ++y)
                    {
                        for (uint32 x = 0; x < mWidth; ++x)
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
