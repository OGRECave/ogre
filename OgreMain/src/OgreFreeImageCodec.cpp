/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

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

#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreFreeImageCodec.h"
#include "OgreImage.h"
#include "OgreException.h"

#include "OgreLogManager.h"
#include "OgreStringConverter.h"

#include <FreeImage.h>

// freeimage 3.9.1~3.11.0 interoperability fix
#ifndef FREEIMAGE_COLORORDER
// we have freeimage 3.9.1, define these symbols in such way as 3.9.1 really work (do not use 3.11.0 definition, as color order was changed between these two versions on Apple systems)
#define FREEIMAGE_COLORORDER_BGR	0
#define FREEIMAGE_COLORORDER_RGB	1
#if defined(FREEIMAGE_BIGENDIAN)
#define FREEIMAGE_COLORORDER FREEIMAGE_COLORORDER_RGB
#else
#define FREEIMAGE_COLORORDER FREEIMAGE_COLORORDER_BGR
#endif
#endif

namespace Ogre {

	FreeImageCodec::RegisteredCodecList FreeImageCodec::msCodecList;
	//---------------------------------------------------------------------
	void FreeImageLoadErrorHandler(FREE_IMAGE_FORMAT fif, const char *message) 
	{
		// Callback method as required by FreeImage to report problems
		const char* typeName = FreeImage_GetFormatFromFIF(fif);
		if (typeName)
		{
			LogManager::getSingleton().stream() 
				<< "FreeImage error: '" << message << "' when loading format "
				<< typeName;
		}
		else
		{
			LogManager::getSingleton().stream() 
				<< "FreeImage error: '" << message << "'";
		}

	}
	//---------------------------------------------------------------------
	void FreeImageSaveErrorHandler(FREE_IMAGE_FORMAT fif, const char *message) 
	{
		// Callback method as required by FreeImage to report problems
		OGRE_EXCEPT(Exception::ERR_CANNOT_WRITE_TO_FILE, 
					message, "FreeImageCodec::save")
	}
	//---------------------------------------------------------------------
	void FreeImageCodec::startup(void)
	{
		FreeImage_Initialise(false);

		LogManager::getSingleton().logMessage(
			LML_NORMAL,
			"FreeImage version: " + String(FreeImage_GetVersion()));
		LogManager::getSingleton().logMessage(
			LML_NORMAL,
			FreeImage_GetCopyrightMessage());

		// Register codecs
		StringUtil::StrStreamType strExt;
		strExt << "Supported formats: ";
		bool first = true;
		for (int i = 0; i < FreeImage_GetFIFCount(); ++i)
		{

			// Skip DDS codec since FreeImage does not have the option 
			// to keep DXT data compressed, we'll use our own codec
			if ((FREE_IMAGE_FORMAT)i == FIF_DDS)
				continue;
			
			String exts(FreeImage_GetFIFExtensionList((FREE_IMAGE_FORMAT)i));
			if (!first)
			{
				strExt << ",";
			}
			first = false;
			strExt << exts;
			
			// Pull off individual formats (separated by comma by FI)
			StringVector extsVector = StringUtil::split(exts, ",");
			for (StringVector::iterator v = extsVector.begin(); v != extsVector.end(); ++v)
			{
				// FreeImage 3.13 lists many formats twice: once under their own codec and
				// once under the "RAW" codec, which is listed last. Avoid letting the RAW override
				// the dedicated codec!
				if (!Codec::isCodecRegistered(*v))
				{
					ImageCodec* codec = OGRE_NEW FreeImageCodec(*v, i);
					msCodecList.push_back(codec);
					Codec::registerCodec(codec);
				}
			}
		}
		LogManager::getSingleton().logMessage(
			LML_NORMAL,
			strExt.str());

		// Set error handler
		FreeImage_SetOutputMessage(FreeImageLoadErrorHandler);




	}
	//---------------------------------------------------------------------
	void FreeImageCodec::shutdown(void)
	{
		FreeImage_DeInitialise();

		for (RegisteredCodecList::iterator i = msCodecList.begin();
			i != msCodecList.end(); ++i)
		{
			Codec::unregisterCodec(*i);
			OGRE_DELETE *i;
		}
		msCodecList.clear();

	}
	//---------------------------------------------------------------------
    FreeImageCodec::FreeImageCodec(const String &type, unsigned int fiType):
        mType(type),
        mFreeImageType(fiType)
    { 
    }
	//---------------------------------------------------------------------
	FIBITMAP* FreeImageCodec::encodeBitmap(MemoryDataStreamPtr& input, CodecDataPtr& pData) const
	{
		FIBITMAP* ret = 0;

		ImageData* pImgData = static_cast< ImageData * >( pData.getPointer() );
		PixelBox src(pImgData->width, pImgData->height, pImgData->depth, pImgData->format, input->getPtr());

		// The required format, which will adjust to the format
		// actually supported by FreeImage.
		PixelFormat requiredFormat = pImgData->format;

		// determine the settings
		FREE_IMAGE_TYPE imageType;
		PixelFormat determiningFormat = pImgData->format;

		switch(determiningFormat)
		{
		case PF_R5G6B5:
		case PF_B5G6R5:
		case PF_R8G8B8:
		case PF_B8G8R8:
		case PF_A8R8G8B8:
		case PF_X8R8G8B8:
		case PF_A8B8G8R8:
		case PF_X8B8G8R8:
		case PF_B8G8R8A8:
		case PF_R8G8B8A8:
		case PF_A4L4:
		case PF_BYTE_LA:
		case PF_R3G3B2:
		case PF_A4R4G4B4:
		case PF_A1R5G5B5:
		case PF_A2R10G10B10:
		case PF_A2B10G10R10:
			// I'd like to be able to use r/g/b masks to get FreeImage to load the data
			// in it's existing format, but that doesn't work, FreeImage needs to have
			// data in RGB[A] (big endian) and BGR[A] (little endian), always.
			if (PixelUtil::hasAlpha(determiningFormat))
			{
#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_RGB
				requiredFormat = PF_BYTE_RGBA;
#else
				requiredFormat = PF_BYTE_BGRA;
#endif
			}
			else
			{
#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_RGB
				requiredFormat = PF_BYTE_RGB;
#else
				requiredFormat = PF_BYTE_BGR;
#endif
			}
			// fall through
		case PF_L8:
		case PF_A8:
			imageType = FIT_BITMAP;
			break;

		case PF_L16:
			imageType = FIT_UINT16;
			break;

		case PF_SHORT_GR:
			requiredFormat = PF_SHORT_RGB;
			// fall through
		case PF_SHORT_RGB:
			imageType = FIT_RGB16;
			break;

		case PF_SHORT_RGBA:
			imageType = FIT_RGBA16;
			break;

		case PF_FLOAT16_R:
			requiredFormat = PF_FLOAT32_R;
			// fall through
		case PF_FLOAT32_R:
			imageType = FIT_FLOAT;
			break;

		case PF_FLOAT16_GR:
		case PF_FLOAT16_RGB:
		case PF_FLOAT32_GR:
			requiredFormat = PF_FLOAT32_RGB;
			// fall through
		case PF_FLOAT32_RGB:
			imageType = FIT_RGBF;
			break;

		case PF_FLOAT16_RGBA:
			requiredFormat = PF_FLOAT32_RGBA;
			// fall through
		case PF_FLOAT32_RGBA:
			imageType = FIT_RGBAF;
			break;

		default:
			OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Invalid image format", "FreeImageCodec::encode");
		};

		// Check support for this image type & bit depth
		if (!FreeImage_FIFSupportsExportType((FREE_IMAGE_FORMAT)mFreeImageType, imageType) ||
			!FreeImage_FIFSupportsExportBPP((FREE_IMAGE_FORMAT)mFreeImageType, (int)PixelUtil::getNumElemBits(requiredFormat)))
		{
			// Ok, need to allocate a fallback
			// Only deal with RGBA -> RGB for now
			switch (requiredFormat)
			{
			case PF_BYTE_RGBA:
				requiredFormat = PF_BYTE_RGB;
				break;
			case PF_BYTE_BGRA:
				requiredFormat = PF_BYTE_BGR;
				break;
			default:
				break;
			};

		}

		bool conversionRequired = false;

		unsigned char* srcData = input->getPtr();

		// Check BPP
		unsigned bpp = static_cast<unsigned>(PixelUtil::getNumElemBits(requiredFormat));
		if (!FreeImage_FIFSupportsExportBPP((FREE_IMAGE_FORMAT)mFreeImageType, (int)bpp))
		{
			if (bpp == 32 && PixelUtil::hasAlpha(pImgData->format) && FreeImage_FIFSupportsExportBPP((FREE_IMAGE_FORMAT)mFreeImageType, 24))
			{
				// drop to 24 bit (lose alpha)
#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_RGB
				requiredFormat = PF_BYTE_RGB;
#else
				requiredFormat = PF_BYTE_BGR;
#endif
				bpp = 24;
			}
			else if (bpp == 128 && PixelUtil::hasAlpha(pImgData->format) && FreeImage_FIFSupportsExportBPP((FREE_IMAGE_FORMAT)mFreeImageType, 96))
			{
				// drop to 96-bit floating point
				requiredFormat = PF_FLOAT32_RGB;
			}
		}

		PixelBox convBox(pImgData->width, pImgData->height, 1, requiredFormat);
		if (requiredFormat != pImgData->format)
		{
			conversionRequired = true;
			// Allocate memory
			convBox.data = OGRE_ALLOC_T(uchar, convBox.getConsecutiveSize(), MEMCATEGORY_GENERAL);
			// perform conversion and reassign source
			PixelBox newSrc(pImgData->width, pImgData->height, 1, pImgData->format, input->getPtr());
			PixelUtil::bulkPixelConversion(newSrc, convBox);
			srcData = static_cast<unsigned char*>(convBox.data);
		}


		ret = FreeImage_AllocateT(
			imageType, 
			static_cast<int>(pImgData->width), 
			static_cast<int>(pImgData->height), 
			bpp);

		if (!ret)
		{
			if (conversionRequired)
				OGRE_FREE(convBox.data, MEMCATEGORY_GENERAL);

			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
				"FreeImage_AllocateT failed - possibly out of memory. ", 
				__FUNCTION__);
		}

		if (requiredFormat == PF_L8 || requiredFormat == PF_A8)
		{
			// Must explicitly tell FreeImage that this is greyscale by setting
			// a "grey" palette (otherwise it will save as a normal RGB
			// palettized image).
			FIBITMAP *tmp = FreeImage_ConvertToGreyscale(ret);
			FreeImage_Unload(ret);
			ret = tmp;
		}
		
		size_t dstPitch = FreeImage_GetPitch(ret);
		size_t srcPitch = pImgData->width * PixelUtil::getNumElemBytes(requiredFormat);


		// Copy data, invert scanlines and respect FreeImage pitch
		uchar* pDst = FreeImage_GetBits(ret);
		for (size_t y = 0; y < pImgData->height; ++y)
		{
			uchar* pSrc = srcData + (pImgData->height - y - 1) * srcPitch;
			memcpy(pDst, pSrc, srcPitch);
			pDst += dstPitch;
		}

		if (conversionRequired)
		{
			// delete temporary conversion area
			OGRE_FREE(convBox.data, MEMCATEGORY_GENERAL);
		}

		return ret;
	}
    //---------------------------------------------------------------------
    DataStreamPtr FreeImageCodec::encode(MemoryDataStreamPtr& input, Codec::CodecDataPtr& pData) const
    {        
		FIBITMAP* fiBitmap = encodeBitmap(input, pData);

		// open memory chunk allocated by FreeImage
		FIMEMORY* mem = FreeImage_OpenMemory();
		// write data into memory
		FreeImage_SaveToMemory((FREE_IMAGE_FORMAT)mFreeImageType, fiBitmap, mem);
		// Grab data information
		BYTE* data;
		DWORD size;
		FreeImage_AcquireMemory(mem, &data, &size);
		// Copy data into our own buffer
		// Because we're asking MemoryDataStream to free this, must create in a compatible way
		BYTE* ourData = OGRE_ALLOC_T(BYTE, size, MEMCATEGORY_GENERAL);
		memcpy(ourData, data, size);
		// Wrap data in stream, tell it to free on close 
		DataStreamPtr outstream(OGRE_NEW MemoryDataStream(ourData, size, true));
		// Now free FreeImage memory buffers
		FreeImage_CloseMemory(mem);
		// Unload bitmap
		FreeImage_Unload(fiBitmap);

		return outstream;


    }
    //---------------------------------------------------------------------
    void FreeImageCodec::encodeToFile(MemoryDataStreamPtr& input,
        const String& outFileName, Codec::CodecDataPtr& pData) const
    {
		FIBITMAP* fiBitmap = encodeBitmap(input, pData);

		FreeImage_Save((FREE_IMAGE_FORMAT)mFreeImageType, fiBitmap, outFileName.c_str());
		FreeImage_Unload(fiBitmap);
    }
    //---------------------------------------------------------------------
    Codec::DecodeResult FreeImageCodec::decode(DataStreamPtr& input) const
    {
		// Buffer stream into memory (TODO: override IO functions instead?)
		MemoryDataStream memStream(input, true);

		FIMEMORY* fiMem = 
			FreeImage_OpenMemory(memStream.getPtr(), static_cast<DWORD>(memStream.size()));

		FIBITMAP* fiBitmap = FreeImage_LoadFromMemory(
			(FREE_IMAGE_FORMAT)mFreeImageType, fiMem);
		if (!fiBitmap)
		{
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
				"Error decoding image", 
				"FreeImageCodec::decode");
		}


		ImageData* imgData = OGRE_NEW ImageData();
		MemoryDataStreamPtr output;

		imgData->depth = 1; // only 2D formats handled by this codec
		imgData->width = FreeImage_GetWidth(fiBitmap);
		imgData->height = FreeImage_GetHeight(fiBitmap);
        imgData->num_mipmaps = 0; // no mipmaps in non-DDS 
        imgData->flags = 0;

		// Must derive format first, this may perform conversions
		
		FREE_IMAGE_TYPE imageType = FreeImage_GetImageType(fiBitmap);
		FREE_IMAGE_COLOR_TYPE colourType = FreeImage_GetColorType(fiBitmap);
		unsigned bpp = FreeImage_GetBPP(fiBitmap);

		switch(imageType)
		{
		case FIT_UNKNOWN:
		case FIT_COMPLEX:
		case FIT_UINT32:
		case FIT_INT32:
		case FIT_DOUBLE:
        default:
			OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
				"Unknown or unsupported image format", 
				"FreeImageCodec::decode");
				
			break;
		case FIT_BITMAP:
			// Standard image type
			// Perform any colour conversions for greyscale
			if (colourType == FIC_MINISWHITE || colourType == FIC_MINISBLACK)
			{
				FIBITMAP* newBitmap = FreeImage_ConvertToGreyscale(fiBitmap);
				// free old bitmap and replace
				FreeImage_Unload(fiBitmap);
				fiBitmap = newBitmap;
				// get new formats
				bpp = FreeImage_GetBPP(fiBitmap);
			}
			// Perform any colour conversions for RGB
			else if (bpp < 8 || colourType == FIC_PALETTE || colourType == FIC_CMYK)
			{
				FIBITMAP* newBitmap =  NULL;	
				if (FreeImage_IsTransparent(fiBitmap))
				{
					// convert to 32 bit to preserve the transparency 
					// (the alpha byte will be 0 if pixel is transparent)
					newBitmap = FreeImage_ConvertTo32Bits(fiBitmap);
				}
				else
				{
					// no transparency - only 3 bytes are needed
					newBitmap = FreeImage_ConvertTo24Bits(fiBitmap);
				}

				// free old bitmap and replace
				FreeImage_Unload(fiBitmap);
				fiBitmap = newBitmap;
				// get new formats
				bpp = FreeImage_GetBPP(fiBitmap);
			}

			// by this stage, 8-bit is greyscale, 16/24/32 bit are RGB[A]
			switch(bpp)
			{
			case 8:
				imgData->format = PF_L8;
				break;
			case 16:
				// Determine 555 or 565 from green mask
				// cannot be 16-bit greyscale since that's FIT_UINT16
				if(FreeImage_GetGreenMask(fiBitmap) == FI16_565_GREEN_MASK)
				{
					imgData->format = PF_R5G6B5;
				}
				else
				{
					// FreeImage doesn't support 4444 format so must be 1555
					imgData->format = PF_A1R5G5B5;
				}
				break;
			case 24:
				// FreeImage differs per platform
				//     PF_BYTE_BGR[A] for little endian (== PF_ARGB native)
				//     PF_BYTE_RGB[A] for big endian (== PF_RGBA native)
#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_RGB
				imgData->format = PF_BYTE_RGB;
#else
				imgData->format = PF_BYTE_BGR;
#endif
				break;
			case 32:
#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_RGB
				imgData->format = PF_BYTE_RGBA;
#else
				imgData->format = PF_BYTE_BGRA;
#endif
				break;
				
				
			};
			break;
		case FIT_UINT16:
		case FIT_INT16:
			// 16-bit greyscale
			imgData->format = PF_L16;
			break;
		case FIT_FLOAT:
			// Single-component floating point data
			imgData->format = PF_FLOAT32_R;
			break;
		case FIT_RGB16:
			imgData->format = PF_SHORT_RGB;
			break;
		case FIT_RGBA16:
			imgData->format = PF_SHORT_RGBA;
			break;
		case FIT_RGBF:
			imgData->format = PF_FLOAT32_RGB;
			break;
		case FIT_RGBAF:
			imgData->format = PF_FLOAT32_RGBA;
			break;
			
			
		};

		unsigned char* srcData = FreeImage_GetBits(fiBitmap);
		unsigned srcPitch = FreeImage_GetPitch(fiBitmap);

		// Final data - invert image and trim pitch at the same time
		size_t dstPitch = imgData->width * PixelUtil::getNumElemBytes(imgData->format);
		imgData->size = dstPitch * imgData->height;
        // Bind output buffer
        output.bind(OGRE_NEW MemoryDataStream(imgData->size));

		uchar* pDst = output->getPtr();
		for (size_t y = 0; y < imgData->height; ++y)
		{
			uchar* pSrc = srcData + (imgData->height - y - 1) * srcPitch;
			memcpy(pDst, pSrc, dstPitch);
			pDst += dstPitch;
		}

		
		FreeImage_Unload(fiBitmap);
		FreeImage_CloseMemory(fiMem);


        DecodeResult ret;
        ret.first = output;
        ret.second = CodecDataPtr(imgData);
		return ret;

    }
    //---------------------------------------------------------------------    
    String FreeImageCodec::getType() const 
    {
        return mType;
    }
	//---------------------------------------------------------------------
	String FreeImageCodec::magicNumberToFileExt(const char *magicNumberPtr, size_t maxbytes) const
	{
		FIMEMORY* fiMem = 
                    FreeImage_OpenMemory((BYTE*)const_cast<char*>(magicNumberPtr), static_cast<DWORD>(maxbytes));

		FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(fiMem, (int)maxbytes);
		FreeImage_CloseMemory(fiMem);

		if (fif != FIF_UNKNOWN)
		{
			String ext(FreeImage_GetFormatFromFIF(fif));
			StringUtil::toLowerCase(ext);
			return ext;
		}
		else
		{
			return StringUtil::BLANK;
		}
	}
}
