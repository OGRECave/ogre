/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
#include "OgreArchiveManager.h"
#include "OgreException.h"
#include "OgreImageCodec.h"
#include "OgreColourValue.h"

#include "OgreImageResampler.h"

namespace Ogre {
	ImageCodec::~ImageCodec() {
	}

	//-----------------------------------------------------------------------------
	Image::Image()
		: m_uWidth(0),
		m_uHeight(0),
		m_uDepth(0),
		m_uSize(0),
		m_uNumMipmaps(0),
		m_uFlags(0),
		m_eFormat(PF_UNKNOWN),
		m_pBuffer( NULL ),
		m_bAutoDelete( true )
	{
	}

	//-----------------------------------------------------------------------------
	Image::Image( const Image &img )
		: m_pBuffer( NULL ),
		m_bAutoDelete( true )
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
		if( m_pBuffer && m_bAutoDelete )
		{
			OGRE_FREE(m_pBuffer, MEMCATEGORY_GENERAL);
			m_pBuffer = NULL;
		}

	}

	//-----------------------------------------------------------------------------
	Image & Image::operator = ( const Image &img )
	{
		freeMemory();
		m_uWidth = img.m_uWidth;
		m_uHeight = img.m_uHeight;
		m_uDepth = img.m_uDepth;
		m_eFormat = img.m_eFormat;
		m_uSize = img.m_uSize;
		m_uFlags = img.m_uFlags;
		m_ucPixelSize = img.m_ucPixelSize;
		m_uNumMipmaps = img.m_uNumMipmaps;
		m_bAutoDelete = img.m_bAutoDelete;
		//Only create/copy when previous data was not dynamic data
		if( m_bAutoDelete )
		{
			m_pBuffer = OGRE_ALLOC_T(uchar, m_uSize, MEMCATEGORY_GENERAL);
			memcpy( m_pBuffer, img.m_pBuffer, m_uSize );
		}
		else
		{
			m_pBuffer = img.m_pBuffer;
		}

		return *this;
	}

	//-----------------------------------------------------------------------------
	Image & Image::flipAroundY()
	{
		if( !m_pBuffer )
		{
			OGRE_EXCEPT( 
				Exception::ERR_INTERNAL_ERROR,
				"Can not flip an unitialized texture",
				"Image::flipAroundY" );
		}
        
         m_uNumMipmaps = 0; // Image operations lose precomputed mipmaps

		uchar	*pTempBuffer1 = NULL;
		ushort	*pTempBuffer2 = NULL;
		uchar	*pTempBuffer3 = NULL;
		uint	*pTempBuffer4 = NULL;

		uchar	*src1 = m_pBuffer, *dst1 = NULL;
		ushort	*src2 = (ushort *)m_pBuffer, *dst2 = NULL;
		uchar	*src3 = m_pBuffer, *dst3 = NULL;
		uint	*src4 = (uint *)m_pBuffer, *dst4 = NULL;

		ushort y;
		switch (m_ucPixelSize)
		{
		case 1:
			pTempBuffer1 = OGRE_ALLOC_T(uchar, m_uWidth * m_uHeight, MEMCATEGORY_GENERAL);
			for (y = 0; y < m_uHeight; y++)
			{
				dst1 = (pTempBuffer1 + ((y * m_uWidth) + m_uWidth - 1));
				for (ushort x = 0; x < m_uWidth; x++)
					memcpy(dst1--, src1++, sizeof(uchar));
			}

			memcpy(m_pBuffer, pTempBuffer1, m_uWidth * m_uHeight * sizeof(uchar));
			OGRE_FREE(pTempBuffer1, MEMCATEGORY_GENERAL);
			break;

		case 2:
			pTempBuffer2 = OGRE_ALLOC_T(ushort, m_uWidth * m_uHeight, MEMCATEGORY_GENERAL);
			for (y = 0; y < m_uHeight; y++)
			{
				dst2 = (pTempBuffer2 + ((y * m_uWidth) + m_uWidth - 1));
				for (ushort x = 0; x < m_uWidth; x++)
					memcpy(dst2--, src2++, sizeof(ushort));
			}

			memcpy(m_pBuffer, pTempBuffer2, m_uWidth * m_uHeight * sizeof(ushort));
			OGRE_FREE(pTempBuffer2, MEMCATEGORY_GENERAL);
			break;

		case 3:
			pTempBuffer3 = OGRE_ALLOC_T(uchar, m_uWidth * m_uHeight * 3, MEMCATEGORY_GENERAL);
			for (y = 0; y < m_uHeight; y++)
			{
				size_t offset = ((y * m_uWidth) + (m_uWidth - 1)) * 3;
				dst3 = pTempBuffer3;
				dst3 += offset;
				for (size_t x = 0; x < m_uWidth; x++)
				{
					memcpy(dst3, src3, sizeof(uchar) * 3);
					dst3 -= 3; src3 += 3;
				}
			}

			memcpy(m_pBuffer, pTempBuffer3, m_uWidth * m_uHeight * sizeof(uchar) * 3);
			OGRE_FREE(pTempBuffer3, MEMCATEGORY_GENERAL);
			break;

		case 4:
			pTempBuffer4 = OGRE_ALLOC_T(uint, m_uWidth * m_uHeight, MEMCATEGORY_GENERAL);
			for (y = 0; y < m_uHeight; y++)
			{
				dst4 = (pTempBuffer4 + ((y * m_uWidth) + m_uWidth - 1));
				for (ushort x = 0; x < m_uWidth; x++)
					memcpy(dst4--, src4++, sizeof(uint));
			}

			memcpy(m_pBuffer, pTempBuffer4, m_uWidth * m_uHeight * sizeof(uint));
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
		if( !m_pBuffer )
		{
			OGRE_EXCEPT( 
				Exception::ERR_INTERNAL_ERROR,
				"Can not flip an unitialized texture",
				"Image::flipAroundX" );
		}
        
        m_uNumMipmaps = 0; // Image operations lose precomputed mipmaps

		size_t rowSpan = m_uWidth * m_ucPixelSize;

		uchar *pTempBuffer = OGRE_ALLOC_T(uchar, rowSpan * m_uHeight, MEMCATEGORY_GENERAL);
		uchar *ptr1 = m_pBuffer, *ptr2 = pTempBuffer + ( ( m_uHeight - 1 ) * rowSpan );

		for( ushort i = 0; i < m_uHeight; i++ )
		{
			memcpy( ptr2, ptr1, rowSpan );
			ptr1 += rowSpan; ptr2 -= rowSpan;
		}

		memcpy( m_pBuffer, pTempBuffer, rowSpan * m_uHeight);

		OGRE_FREE(pTempBuffer, MEMCATEGORY_GENERAL);

		return *this;
	}

	//-----------------------------------------------------------------------------
	Image& Image::loadDynamicImage( uchar* pData, size_t uWidth, size_t uHeight, 
		size_t depth,
		PixelFormat eFormat, bool autoDelete, 
		size_t numFaces, size_t numMipMaps)
	{

		freeMemory();
		// Set image metadata
		m_uWidth = uWidth;
		m_uHeight = uHeight;
		m_uDepth = depth;
		m_eFormat = eFormat;
		m_ucPixelSize = static_cast<uchar>(PixelUtil::getNumElemBytes( m_eFormat ));
		m_uNumMipmaps = numMipMaps;
		m_uFlags = 0;
		// Set flags
		if (PixelUtil::isCompressed(eFormat))
			m_uFlags |= IF_COMPRESSED;
		if (m_uDepth != 1)
			m_uFlags |= IF_3D_TEXTURE;
		if(numFaces == 6)
			m_uFlags |= IF_CUBEMAP;
		if(numFaces != 6 && numFaces != 1)
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
			"Number of faces currently must be 6 or 1.", 
			"Image::loadDynamicImage");

		m_uSize = calculateSize(numMipMaps, numFaces, uWidth, uHeight, depth, eFormat);
		m_pBuffer = pData;
		m_bAutoDelete = autoDelete;

		return *this;

	}

	//-----------------------------------------------------------------------------
	Image & Image::loadRawData(
		DataStreamPtr& stream, 
		size_t uWidth, size_t uHeight, size_t uDepth,
		PixelFormat eFormat,
		size_t numFaces, size_t numMipMaps)
	{

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
		if( !m_pBuffer )
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
		imgData->format = m_eFormat;
		imgData->height = m_uHeight;
		imgData->width = m_uWidth;
		imgData->depth = m_uDepth;
		imgData->size = m_uSize;
		// Wrap in CodecDataPtr, this will delete
		Codec::CodecDataPtr codeDataPtr(imgData);
		// Wrap memory, be sure not to delete when stream destroyed
		MemoryDataStreamPtr wrapper(OGRE_NEW MemoryDataStream(m_pBuffer, m_uSize, false));

		pCodec->codeToFile(wrapper, filename, codeDataPtr);
	}
	//---------------------------------------------------------------------
	DataStreamPtr Image::encode(const String& formatextension)
	{
		if( !m_pBuffer )
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
		imgData->format = m_eFormat;
		imgData->height = m_uHeight;
		imgData->width = m_uWidth;
		imgData->depth = m_uDepth;
		// Wrap in CodecDataPtr, this will delete
		Codec::CodecDataPtr codeDataPtr(imgData);
		// Wrap memory, be sure not to delete when stream destroyed
		MemoryDataStreamPtr wrapper(OGRE_NEW MemoryDataStream(m_pBuffer, m_uSize, false));

		return pCodec->code(wrapper, codeDataPtr);
	}
	//-----------------------------------------------------------------------------
	Image & Image::load(DataStreamPtr& stream, const String& type )
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

		m_uWidth = pData->width;
		m_uHeight = pData->height;
		m_uDepth = pData->depth;
		m_uSize = pData->size;
		m_uNumMipmaps = pData->num_mipmaps;
		m_uFlags = pData->flags;

		// Get the format and compute the pixel size
		m_eFormat = pData->format;
		m_ucPixelSize = static_cast<uchar>(PixelUtil::getNumElemBytes( m_eFormat ));
		// Just use internal buffer of returned memory stream
		m_pBuffer = res.first->getPtr();
		// Make sure stream does not delete
		res.first->setFreeOnClose(false);
		// make sure we delete
		m_bAutoDelete = true;

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
			return StringUtil::BLANK;

	}
	//-----------------------------------------------------------------------------
	uchar* Image::getData()
	{
		return m_pBuffer;
	}

	//-----------------------------------------------------------------------------
	const uchar* Image::getData() const
	{
		assert( m_pBuffer );
		return m_pBuffer;
	}

	//-----------------------------------------------------------------------------
	size_t Image::getSize() const
	{
		return m_uSize;
	}

	//-----------------------------------------------------------------------------
	size_t Image::getNumMipmaps() const
	{
		return m_uNumMipmaps;
	}

	//-----------------------------------------------------------------------------
	bool Image::hasFlag(const ImageFlags imgFlag) const
	{
		if(m_uFlags & imgFlag)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	//-----------------------------------------------------------------------------
	size_t Image::getDepth() const
	{
		return m_uDepth;
	}
	//-----------------------------------------------------------------------------
	size_t Image::getWidth() const
	{
		return m_uWidth;
	}

	//-----------------------------------------------------------------------------
	size_t Image::getHeight() const
	{
		return m_uHeight;
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
		return m_uWidth * m_ucPixelSize;
	}

	//-----------------------------------------------------------------------------
	PixelFormat Image::getFormat() const
	{
		return m_eFormat;
	}

	//-----------------------------------------------------------------------------
	uchar Image::getBPP() const
	{
		return m_ucPixelSize * 8;
	}

	//-----------------------------------------------------------------------------
	bool Image::getHasAlpha(void) const
	{
		return PixelUtil::getFlags(m_eFormat) & PFF_HASALPHA;
	}
	//-----------------------------------------------------------------------------
	void Image::applyGamma( unsigned char *buffer, Real gamma, size_t size, uchar bpp )
	{
		if( gamma == 1.0f )
			return;

		//NB only 24/32-bit supported
		if( bpp != 24 && bpp != 32 ) return;

		uint stride = bpp >> 3;

		for( size_t i = 0, j = size / stride; i < j; i++, buffer += stride )
		{
			float r, g, b;

			r = (float)buffer[0];
			g = (float)buffer[1];
			b = (float)buffer[2];

			r = r * gamma;
			g = g * gamma;
			b = b * gamma;

			float scale = 1.0f, tmp;

			if( r > 255.0f && (tmp=(255.0f/r)) < scale )
				scale = tmp;
			if( g > 255.0f && (tmp=(255.0f/g)) < scale )
				scale = tmp;
			if( b > 255.0f && (tmp=(255.0f/b)) < scale )
				scale = tmp;

			r *= scale; g *= scale; b *= scale;

			buffer[0] = (uchar)r;
			buffer[1] = (uchar)g;
			buffer[2] = (uchar)b;
		}
	}
	//-----------------------------------------------------------------------------
	void Image::resize(ushort width, ushort height, Filter filter)
	{
		// resizing dynamic images is not supported
		assert(m_bAutoDelete);
		assert(m_uDepth == 1);

		// reassign buffer to temp image, make sure auto-delete is true
		Image temp;
		temp.loadDynamicImage(m_pBuffer, m_uWidth, m_uHeight, 1, m_eFormat, true);
		// do not delete[] m_pBuffer!  temp will destroy it

		// set new dimensions, allocate new buffer
		m_uWidth = width;
		m_uHeight = height;
		m_uSize = PixelUtil::getMemorySize(m_uWidth, m_uHeight, 1, m_eFormat);
		m_pBuffer = OGRE_ALLOC_T(uchar, m_uSize, MEMCATEGORY_GENERAL);
        m_uNumMipmaps = 0; // Loses precomputed mipmaps

		// scale the image from temp into our resized buffer
		Image::scale(temp.getPixelBox(), getPixelBox(), filter);
	}
	//-----------------------------------------------------------------------
	void Image::scale(const PixelBox &src, const PixelBox &scaled, Filter filter) 
	{
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

	ColourValue Image::getColourAt(int x, int y, int z) const
	{
		ColourValue rval;
		PixelUtil::unpackColour(&rval, m_eFormat, &m_pBuffer[m_ucPixelSize * (z * m_uWidth * m_uHeight + m_uWidth * y + x)]);
		return rval;
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
        size_t width = getWidth(), height=getHeight(), depth=getDepth();
		size_t numMips = getNumMipmaps();

		// Figure out the offsets 
		size_t fullFaceSize = 0;
		size_t finalFaceSize = 0;
		size_t finalWidth = 0, finalHeight = 0, finalDepth = 0;
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
    size_t Image::calculateSize(size_t mipmaps, size_t faces, size_t width, size_t height, size_t depth, 
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

		m_uWidth = rgb.getWidth();
		m_uHeight = rgb.getHeight();
		m_uDepth = rgb.getDepth();
		m_eFormat = fmt;
		m_uNumMipmaps = rgb.getNumMipmaps();
		size_t numFaces = rgb.getNumFaces();

		// Set flags
		m_uFlags = 0;
		if (m_uDepth != 1)
			m_uFlags |= IF_3D_TEXTURE;
		if(numFaces == 6)
			m_uFlags |= IF_CUBEMAP;

		m_uSize = calculateSize(m_uNumMipmaps, numFaces, m_uWidth, m_uHeight, m_uDepth, m_eFormat);

		m_ucPixelSize = static_cast<uchar>(PixelUtil::getNumElemBytes( m_eFormat ));

		m_pBuffer = static_cast<uchar*>(OGRE_MALLOC(m_uSize, MEMCATEGORY_GENERAL));

		// make sure we delete
		m_bAutoDelete = true;


		for (size_t face = 0; face < numFaces; ++face)
		{
			for (size_t mip = 0; mip <= m_uNumMipmaps; ++mip)
			{
				// convert the RGB first
				PixelBox srcRGB = rgb.getPixelBox(face, mip);
				PixelBox dst = getPixelBox(face, mip);
				PixelUtil::bulkPixelConversion(srcRGB, dst);

				// now selectively add the alpha
				PixelBox srcAlpha = alpha.getPixelBox(face, mip);
				uchar* psrcAlpha = static_cast<uchar*>(srcAlpha.data);
				uchar* pdst = static_cast<uchar*>(dst.data);
				for (size_t d = 0; d < m_uDepth; ++d)
				{
					for (size_t y = 0; y < m_uHeight; ++y)
					{
						for (size_t x = 0; x < m_uWidth; ++x)
						{
							ColourValue colRGBA, colA;
							// read RGB back from dest to save having another pointer
							PixelUtil::unpackColour(&colRGBA, m_eFormat, pdst);
							PixelUtil::unpackColour(&colA, alpha.getFormat(), psrcAlpha);

							// combine RGB from alpha source texture
							colRGBA.a = (colA.r + colA.g + colA.b) / 3.0;

							PixelUtil::packColour(colRGBA, m_eFormat, pdst);
							
							psrcAlpha += PixelUtil::getNumElemBytes(alpha.getFormat());
							pdst += PixelUtil::getNumElemBytes(m_eFormat);

						}
					}
				}
				

			}
		}

		return *this;

	}
	//---------------------------------------------------------------------

    
}
