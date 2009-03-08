/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/

#include "OgreStableHeaders.h"

#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreDDSCodec.h"
#include "OgreImage.h"
#include "OgreException.h"

#include "OgreLogManager.h"
#include "OgreStringConverter.h"


namespace Ogre {
	// Internal DDS structure definitions
#define FOURCC(c0, c1, c2, c3) (c0 | (c1 << 8) | (c2 << 16) | (c3 << 24))
	
#if OGRE_COMPILER == OGRE_COMPILER_MSVC
#pragma pack (push, 1)
#else
#pragma pack (1)
#endif

	// Nested structure
	struct DDSPixelFormat
	{
		uint32 size;
		uint32 flags;
		uint32 fourCC;
		uint32 rgbBits;
		uint32 redMask;
		uint32 greenMask;
		uint32 blueMask;
		uint32 alphaMask;
	};
	
	// Nested structure
	struct DDSCaps
	{
		uint32 caps1;
		uint32 caps2;
		uint32 reserved[2];
	};
	// Main header, note preceded by 'DDS '
	struct DDSHeader
	{
		uint32 size;		
		uint32 flags;
		uint32 height;
		uint32 width;
		uint32 sizeOrPitch;
		uint32 depth;
		uint32 mipMapCount;
		uint32 reserved1[11];
		DDSPixelFormat pixelFormat;
		DDSCaps caps;
		uint32 reserved2;
	};

	// An 8-byte DXT colour block, represents a 4x4 texel area. Used by all DXT formats
	struct DXTColourBlock
	{
		// 2 colour ranges
		uint16 colour_0;
		uint16 colour_1;
		// 16 2-bit indexes, each byte here is one row
		uint8 indexRow[4];
	};
	// An 8-byte DXT explicit alpha block, represents a 4x4 texel area. Used by DXT2/3
	struct DXTExplicitAlphaBlock
	{
		// 16 4-bit values, each 16-bit value is one row
		uint16 alphaRow[4];
	};
	// An 8-byte DXT interpolated alpha block, represents a 4x4 texel area. Used by DXT4/5
	struct DXTInterpolatedAlphaBlock
	{
		// 2 alpha ranges
		uint8 alpha_0;
		uint8 alpha_1;
		// 16 3-bit indexes. Unfortunately 3 bits doesn't map too well to row bytes
		// so just stored raw
		uint8 indexes[6];
	};
	
#if OGRE_COMPILER == OGRE_COMPILER_MSVC
#pragma pack (pop)
#else
#pragma pack ()
#endif

	const uint32 DDS_MAGIC = FOURCC('D', 'D', 'S', ' ');
	const uint32 DDS_PIXELFORMAT_SIZE = 8 * sizeof(uint32);
	const uint32 DDS_CAPS_SIZE = 4 * sizeof(uint32);
	const uint32 DDS_HEADER_SIZE = 19 * sizeof(uint32) + DDS_PIXELFORMAT_SIZE + DDS_CAPS_SIZE;

	const uint32 DDSD_CAPS = 0x00000001;
	const uint32 DDSD_HEIGHT = 0x00000002;
	const uint32 DDSD_WIDTH = 0x00000004;
	const uint32 DDSD_PITCH = 0x00000008;
	const uint32 DDSD_PIXELFORMAT = 0x00001000;
	const uint32 DDSD_MIPMAPCOUNT = 0x00020000;
	const uint32 DDSD_LINEARSIZE = 0x00080000;
	const uint32 DDSD_DEPTH = 0x00800000;
	const uint32 DDPF_ALPHAPIXELS = 0x00000001;
	const uint32 DDPF_FOURCC = 0x00000004;
	const uint32 DDPF_RGB = 0x00000040;
	const uint32 DDSCAPS_COMPLEX = 0x00000008;
	const uint32 DDSCAPS_TEXTURE = 0x00001000;
	const uint32 DDSCAPS_MIPMAP = 0x00400000;
	const uint32 DDSCAPS2_CUBEMAP = 0x00000200;
	const uint32 DDSCAPS2_CUBEMAP_POSITIVEX = 0x00000400;
	const uint32 DDSCAPS2_CUBEMAP_NEGATIVEX = 0x00000800;
	const uint32 DDSCAPS2_CUBEMAP_POSITIVEY = 0x00001000;
	const uint32 DDSCAPS2_CUBEMAP_NEGATIVEY = 0x00002000;
	const uint32 DDSCAPS2_CUBEMAP_POSITIVEZ = 0x00004000;
	const uint32 DDSCAPS2_CUBEMAP_NEGATIVEZ = 0x00008000;
	const uint32 DDSCAPS2_VOLUME = 0x00200000;

	// Special FourCC codes
	const uint32 D3DFMT_R16F			= 111;
	const uint32 D3DFMT_G16R16F			= 112;
	const uint32 D3DFMT_A16B16G16R16F	= 113;
	const uint32 D3DFMT_R32F            = 114;
	const uint32 D3DFMT_G32R32F         = 115;
	const uint32 D3DFMT_A32B32G32R32F   = 116;


	//---------------------------------------------------------------------
	DDSCodec* DDSCodec::msInstance = 0;
	//---------------------------------------------------------------------
	void DDSCodec::startup(void)
	{
		if (!msInstance)
		{

			LogManager::getSingleton().logMessage(
				LML_NORMAL,
				"DDS codec registering");

			msInstance = OGRE_NEW DDSCodec();
			Codec::registerCodec(msInstance);
		}

	}
	//---------------------------------------------------------------------
	void DDSCodec::shutdown(void)
	{
		if(msInstance)
		{
			Codec::unRegisterCodec(msInstance);
			OGRE_DELETE msInstance;
			msInstance = 0;
		}

	}
	//---------------------------------------------------------------------
    DDSCodec::DDSCodec():
        mType("dds")
    { 
    }
    //---------------------------------------------------------------------
    DataStreamPtr DDSCodec::code(MemoryDataStreamPtr& input, Codec::CodecDataPtr& pData) const
    {        
		OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
			"DDS encoding not supported",
			"DDSCodec::code" ) ;
    }
    //---------------------------------------------------------------------
    void DDSCodec::codeToFile(MemoryDataStreamPtr& input, 
        const String& outFileName, Codec::CodecDataPtr& pData) const
    {
		// Unwrap codecDataPtr - data is cleaned by calling function
		ImageData* imgData = static_cast<ImageData* >(pData.getPointer());  


		// Check size for cube map faces
		bool isCubeMap = (imgData->size == 
			Image::calculateSize(imgData->num_mipmaps, 6, imgData->width, 
			imgData->height, imgData->depth, imgData->format));

		// Establish texture attributes
		bool isVolume = (imgData->depth > 1);		
		bool isFloat32r = (imgData->format == PF_FLOAT32_R);
		bool hasAlpha = false;
		bool notImplemented = false;
		String notImplementedString = "";

		// Check for all the 'not implemented' conditions
		if (imgData->num_mipmaps != 0)
		{
			// No mip map functionality yet
			notImplemented = true;
			notImplementedString += " mipmaps";
		}

		if ((isVolume == true)&&(imgData->width != imgData->height))
		{
			// Square textures only
			notImplemented = true;
			notImplementedString += " non square textures";
		}

		uint32 size = 1;
		while (size < imgData->width)
		{
			size <<= 1;
		}
		if (size != imgData->width)
		{
			// Power two textures only
			notImplemented = true;
			notImplementedString += " non power two textures";
		}

		switch(imgData->format)
		{
		case PF_A8R8G8B8:
		case PF_X8R8G8B8:
		case PF_R8G8B8:
		case PF_FLOAT32_R:
			break;
		default:
			// No crazy FOURCC or 565 et al. file formats at this stage
			notImplemented = true;
			notImplementedString = " unsupported pixel format";
			break;
		}		



		// Except if any 'not implemented' conditions were met
		if (notImplemented)
		{
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
				"DDS encoding for" + notImplementedString + " not supported",
				"DDSCodec::codeToFile" ) ;
		}
		else
		{
			// Build header and write to disk

			// Variables for some DDS header flags
			uint32 ddsHeaderFlags = 0;			
			uint32 ddsHeaderRgbBits = 0;
			uint32 ddsHeaderSizeOrPitch = 0;
			uint32 ddsHeaderCaps1 = 0;
			uint32 ddsHeaderCaps2 = 0;
			uint32 ddsMagic = DDS_MAGIC;

			// Initalise the header flags
			ddsHeaderFlags = (isVolume) ? DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT|DDSD_DEPTH|DDSD_PIXELFORMAT :
				DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT|DDSD_PIXELFORMAT;	

			// Initalise the rgbBits flags
			switch(imgData->format)
			{
			case PF_A8R8G8B8:
				ddsHeaderRgbBits = 8 * 4;
				hasAlpha = true;
				break;
			case PF_X8R8G8B8:
				ddsHeaderRgbBits = 8 * 4;
				break;
			case PF_R8G8B8:
				ddsHeaderRgbBits = 8 * 3;
				break;
			case PF_FLOAT32_R:
				ddsHeaderRgbBits = 32;
				break;
			default:
				ddsHeaderRgbBits = 0;
				break;
			}

			// Initalise the SizeOrPitch flags (power two textures for now)
			ddsHeaderSizeOrPitch = ddsHeaderRgbBits * imgData->width;

			// Initalise the caps flags
			ddsHeaderCaps1 = (isVolume||isCubeMap) ? DDSCAPS_COMPLEX|DDSCAPS_TEXTURE : DDSCAPS_TEXTURE;
			if (isVolume)
			{
				ddsHeaderCaps2 = DDSCAPS2_VOLUME;
			}
			else if (isCubeMap)
			{
				ddsHeaderCaps2 = DDSCAPS2_CUBEMAP|
					DDSCAPS2_CUBEMAP_POSITIVEX|DDSCAPS2_CUBEMAP_NEGATIVEX|
					DDSCAPS2_CUBEMAP_POSITIVEY|DDSCAPS2_CUBEMAP_NEGATIVEY|
					DDSCAPS2_CUBEMAP_POSITIVEZ|DDSCAPS2_CUBEMAP_NEGATIVEZ;
			}

			// Populate the DDS header information
			DDSHeader ddsHeader;
			ddsHeader.size = DDS_HEADER_SIZE;
			ddsHeader.flags = ddsHeaderFlags;		
			ddsHeader.width = (uint32)imgData->width;
			ddsHeader.height = (uint32)imgData->height;
			ddsHeader.depth = (uint32)(isVolume ? imgData->depth : 0);
			ddsHeader.depth = (uint32)(isCubeMap ? 6 : ddsHeader.depth);
			ddsHeader.mipMapCount = 0;
			ddsHeader.sizeOrPitch = ddsHeaderSizeOrPitch;
			for (uint32 reserved1=0; reserved1<11; reserved1++) // XXX nasty constant 11
			{
				ddsHeader.reserved1[reserved1] = 0;
			}
			ddsHeader.reserved2 = 0;

			ddsHeader.pixelFormat.size = DDS_PIXELFORMAT_SIZE;
			ddsHeader.pixelFormat.flags = (hasAlpha) ? DDPF_RGB|DDPF_ALPHAPIXELS : DDPF_RGB;
			ddsHeader.pixelFormat.flags = (isFloat32r) ? DDPF_FOURCC : ddsHeader.pixelFormat.flags;
			ddsHeader.pixelFormat.fourCC = (isFloat32r) ? D3DFMT_R32F : 0;
			ddsHeader.pixelFormat.rgbBits = ddsHeaderRgbBits;

			ddsHeader.pixelFormat.alphaMask = (hasAlpha)   ? 0xFF000000 : 0x00000000;
			ddsHeader.pixelFormat.alphaMask = (isFloat32r) ? 0x00000000 : ddsHeader.pixelFormat.alphaMask;
			ddsHeader.pixelFormat.redMask   = (isFloat32r) ? 0xFFFFFFFF :0x00FF0000;
			ddsHeader.pixelFormat.greenMask = (isFloat32r) ? 0x00000000 :0x0000FF00;
			ddsHeader.pixelFormat.blueMask  = (isFloat32r) ? 0x00000000 :0x000000FF;

			ddsHeader.caps.caps1 = ddsHeaderCaps1;
			ddsHeader.caps.caps2 = ddsHeaderCaps2;
			ddsHeader.caps.reserved[0] = 0;
			ddsHeader.caps.reserved[1] = 0;

			// Swap endian
			flipEndian(&ddsMagic, sizeof(uint32), 1);
			flipEndian(&ddsHeader, 4, sizeof(DDSHeader) / 4);

			// Write the file 			
			std::ofstream of;
			of.open(outFileName.c_str(), std::ios_base::binary|std::ios_base::out);
			of.write((const char *)&ddsMagic, sizeof(uint32));
			of.write((const char *)&ddsHeader, DDS_HEADER_SIZE);
			// XXX flipEndian on each pixel chunk written unless isFloat32r ?
			of.write((const char *)input->getPtr(), (uint32)imgData->size);
			of.close();
		}
	}
	//---------------------------------------------------------------------
	PixelFormat DDSCodec::convertFourCCFormat(uint32 fourcc) const
	{
		// convert dxt pixel format
		switch(fourcc)
		{
		case FOURCC('D','X','T','1'):
			return PF_DXT1;
		case FOURCC('D','X','T','2'):
			return PF_DXT2;
		case FOURCC('D','X','T','3'):
			return PF_DXT3;
		case FOURCC('D','X','T','4'):
			return PF_DXT4;
		case FOURCC('D','X','T','5'):
			return PF_DXT5;
		case D3DFMT_R16F:
			return PF_FLOAT16_R;
		case D3DFMT_G16R16F:
			return PF_FLOAT16_GR;
		case D3DFMT_A16B16G16R16F:
			return PF_FLOAT16_RGBA;
		case D3DFMT_R32F:
			return PF_FLOAT32_R;
		case D3DFMT_G32R32F:
			return PF_FLOAT32_GR;
		case D3DFMT_A32B32G32R32F:
			return PF_FLOAT32_RGBA;
		// We could support 3Dc here, but only ATI cards support it, not nVidia
		default:
			OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
				"Unsupported FourCC format found in DDS file", 
				"DDSCodec::decode");
		};

	}
	//---------------------------------------------------------------------
	PixelFormat DDSCodec::convertPixelFormat(uint32 rgbBits, uint32 rMask, 
		uint32 gMask, uint32 bMask, uint32 aMask) const
	{
		// General search through pixel formats
		for (int i = PF_UNKNOWN + 1; i < PF_COUNT; ++i)
		{
			PixelFormat pf = static_cast<PixelFormat>(i);
			if (PixelUtil::getNumElemBits(pf) == rgbBits)
			{
				uint32 testMasks[4];
				PixelUtil::getBitMasks(pf, testMasks);
				int testBits[4];
				PixelUtil::getBitDepths(pf, testBits);
				if (testMasks[0] == rMask && testMasks[1] == gMask &&
					testMasks[2] == bMask && 
					// for alpha, deal with 'X8' formats by checking bit counts
					(testMasks[3] == aMask || (aMask == 0 && testBits[3] == 0)))
				{
					return pf;
				}
			}

		}

		OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Cannot determine pixel format",
			"DDSCodec::convertPixelFormat");

	}
	//---------------------------------------------------------------------
	void DDSCodec::unpackDXTColour(PixelFormat pf, const DXTColourBlock& block, 
		ColourValue* pCol) const
	{
		// Note - we assume all values have already been endian swapped

		// Colour lookup table
		ColourValue derivedColours[4];

		if (pf == PF_DXT1 && block.colour_0 <= block.colour_1)
		{
			// 1-bit alpha
			PixelUtil::unpackColour(&(derivedColours[0]), PF_R5G6B5, &(block.colour_0));
			PixelUtil::unpackColour(&(derivedColours[1]), PF_R5G6B5, &(block.colour_1));
			// one intermediate colour, half way between the other two
			derivedColours[2] = (derivedColours[0] + derivedColours[1]) / 2;
			// transparent colour
			derivedColours[3] = ColourValue::ZERO;
		}
		else
		{
			PixelUtil::unpackColour(&(derivedColours[0]), PF_R5G6B5, &(block.colour_0));
			PixelUtil::unpackColour(&(derivedColours[1]), PF_R5G6B5, &(block.colour_1));
			// first interpolated colour, 1/3 of the way along
			derivedColours[2] = (2 * derivedColours[0] + derivedColours[1]) / 3;
			// second interpolated colour, 2/3 of the way along
			derivedColours[3] = (derivedColours[0] + 2 * derivedColours[1]) / 3;
		}

		// Process 4x4 block of texels
		for (size_t row = 0; row < 4; ++row)
		{
			for (size_t x = 0; x < 4; ++x)
			{
				// LSB come first
				uint8 colIdx = static_cast<uint8>(block.indexRow[row] >> (x * 2) & 0x3);
				if (pf == PF_DXT1)
				{
					// Overwrite entire colour
					pCol[(row * 4) + x] = derivedColours[colIdx];
				}
				else
				{
					// alpha has already been read (alpha precedes colour)
					ColourValue& col = pCol[(row * 4) + x];
					col.r = derivedColours[colIdx].r;
					col.g = derivedColours[colIdx].g;
					col.b = derivedColours[colIdx].b;
				}
			}

		}


	}
	//---------------------------------------------------------------------
	void DDSCodec::unpackDXTAlpha(
		const DXTExplicitAlphaBlock& block, ColourValue* pCol) const
	{
		// Note - we assume all values have already been endian swapped
		
		// This is an explicit alpha block, 4 bits per pixel, LSB first
		for (size_t row = 0; row < 4; ++row)
		{
			for (size_t x = 0; x < 4; ++x)
			{
				// Shift and mask off to 4 bits
				uint8 val = static_cast<uint8>(block.alphaRow[row] >> (x * 4) & 0xF);
				// Convert to [0,1]
				pCol->a = (Real)val / (Real)0xF;
				pCol++;
				
			}
			
		}

	}
	//---------------------------------------------------------------------
	void DDSCodec::unpackDXTAlpha(
		const DXTInterpolatedAlphaBlock& block, ColourValue* pCol) const
	{
		// 8 derived alpha values to be indexed
		Real derivedAlphas[8];

		// Explicit extremes
		derivedAlphas[0] = block.alpha_0 / (Real)0xFF;
		derivedAlphas[1] = block.alpha_1 / (Real)0xFF;
		
		
		if (block.alpha_0 <= block.alpha_1)
		{
			// 4 interpolated alphas, plus zero and one			
			// full range including extremes at [0] and [5]
			// we want to fill in [1] through [4] at weights ranging
			// from 1/5 to 4/5
			Real denom = 1.0f / 5.0f;
			for (size_t i = 0; i < 4; ++i) 
			{
				Real factor0 = (4 - i) * denom;
				Real factor1 = (i + 1) * denom;
				derivedAlphas[i + 2] = 
					(factor0 * block.alpha_0) + (factor1 * block.alpha_1);
			}
			derivedAlphas[6] = 0.0f;
			derivedAlphas[7] = 1.0f;

		}
		else
		{
			// 6 interpolated alphas
			// full range including extremes at [0] and [7]
			// we want to fill in [1] through [6] at weights ranging
			// from 1/7 to 6/7
			Real denom = 1.0f / 7.0f;
			for (size_t i = 0; i < 6; ++i) 
			{
				Real factor0 = (6 - i) * denom;
				Real factor1 = (i + 1) * denom;
				derivedAlphas[i + 2] = 
					(factor0 * block.alpha_0) + (factor1 * block.alpha_1);
			}
			
		}

		// Ok, now we've built the reference values, process the indexes
		for (size_t i = 0; i < 16; ++i)
		{
			size_t baseByte = (i * 3) / 8;
			size_t baseBit = (i * 3) % 8;
			uint8 bits = static_cast<uint8>(block.indexes[baseByte] >> baseBit & 0x7);
			// do we need to stitch in next byte too?
			if (baseBit > 5)
			{
				uint8 extraBits = static_cast<uint8>(
					(block.indexes[baseByte+1] << (8 - baseBit)) & 0xFF);
				bits |= extraBits & 0x7;
			}
			pCol[i].a = derivedAlphas[bits];

		}

	}
    //---------------------------------------------------------------------
    Codec::DecodeResult DDSCodec::decode(DataStreamPtr& stream) const
    {
		// Read 4 character code
		uint32 fileType;
		stream->read(&fileType, sizeof(uint32));
		flipEndian(&fileType, sizeof(uint32), 1);
		
		if (FOURCC('D', 'D', 'S', ' ') != fileType)
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
				"This is not a DDS file!", "DDSCodec::decode");
		}
		
		// Read header in full
		DDSHeader header;
		stream->read(&header, sizeof(DDSHeader));

		// Endian flip if required, all 32-bit values
		flipEndian(&header, 4, sizeof(DDSHeader) / 4);

		// Check some sizes
		if (header.size != DDS_HEADER_SIZE)
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
				"DDS header size mismatch!", "DDSCodec::decode");
		}
		if (header.pixelFormat.size != DDS_PIXELFORMAT_SIZE)
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
				"DDS header size mismatch!", "DDSCodec::decode");
		}

		ImageData* imgData = OGRE_NEW ImageData();
		MemoryDataStreamPtr output;

		imgData->depth = 1; // (deal with volume later)
		imgData->width = header.width;
		imgData->height = header.height;
		size_t numFaces = 1; // assume one face until we know otherwise

		if (header.caps.caps1 & DDSCAPS_MIPMAP)
		{
	        imgData->num_mipmaps = header.mipMapCount - 1;
		}
		else
		{
			imgData->num_mipmaps = 0;
		}
		imgData->flags = 0;

		bool decompressDXT = false;
		// Figure out basic image type
		if (header.caps.caps2 & DDSCAPS2_CUBEMAP)
		{
			imgData->flags |= IF_CUBEMAP;
			numFaces = 6;
		}
		else if (header.caps.caps2 & DDSCAPS2_VOLUME)
		{
			imgData->flags |= IF_3D_TEXTURE;
			imgData->depth = header.depth;
		}
		// Pixel format
		PixelFormat sourceFormat = PF_UNKNOWN;

		if (header.pixelFormat.flags & DDPF_FOURCC)
		{
			sourceFormat = convertFourCCFormat(header.pixelFormat.fourCC);
		}
		else
		{
			sourceFormat = convertPixelFormat(header.pixelFormat.rgbBits, 
				header.pixelFormat.redMask, header.pixelFormat.greenMask, 
				header.pixelFormat.blueMask, 
				header.pixelFormat.flags & DDPF_ALPHAPIXELS ? 
				header.pixelFormat.alphaMask : 0);
		}

		if (PixelUtil::isCompressed(sourceFormat))
		{
			if (!Root::getSingleton().getRenderSystem()->getCapabilities()
				->hasCapability(RSC_TEXTURE_COMPRESSION_DXT))
			{
				// We'll need to decompress
				decompressDXT = true;
				// Convert format
				switch (sourceFormat)
				{
				case PF_DXT1:
					// source can be either 565 or 5551 depending on whether alpha present
					// unfortunately you have to read a block to figure out which
					// Note that we upgrade to 32-bit pixel formats here, even 
					// though the source is 16-bit; this is because the interpolated
					// values will benefit from the 32-bit results, and the source
					// from which the 16-bit samples are calculated may have been
					// 32-bit so can benefit from this.
					DXTColourBlock block;
					stream->read(&block, sizeof(DXTColourBlock));
					flipEndian(&(block.colour_0), sizeof(uint16), 1);
					flipEndian(&(block.colour_1), sizeof(uint16), 1);
					// skip back since we'll need to read this again
					stream->skip(0 - sizeof(DXTColourBlock));
					// colour_0 <= colour_1 means transparency in DXT1
					if (block.colour_0 <= block.colour_1)
					{
						imgData->format = PF_BYTE_RGBA;
					}
					else
					{
						imgData->format = PF_BYTE_RGB;
					}
					break;
				case PF_DXT2:
				case PF_DXT3:
				case PF_DXT4:
				case PF_DXT5:
					// full alpha present, formats vary only in encoding 
					imgData->format = PF_BYTE_RGBA;
					break;
                default:
                    // all other cases need no special format handling
                    break;
				}
			}
			else
			{
				// Use original format
				imgData->format = sourceFormat;
				// Keep DXT data compressed
				imgData->flags |= IF_COMPRESSED;
			}
		}
		else // not compressed
		{
			// Don't test against DDPF_RGB since greyscale DDS doesn't set this
			// just derive any other kind of format
			imgData->format = sourceFormat;
		}

		// Calculate total size from number of mipmaps, faces and size
		imgData->size = Image::calculateSize(imgData->num_mipmaps, numFaces, 
			imgData->width, imgData->height, imgData->depth, imgData->format);

		// Bind output buffer
		output.bind(OGRE_NEW MemoryDataStream(imgData->size));

		
		// Now deal with the data
		void* destPtr = output->getPtr();

		// all mips for a face, then each face
		for(size_t i = 0; i < numFaces; ++i)
		{   
			size_t width = imgData->width;
			size_t height = imgData->height;
			size_t depth = imgData->depth;

			for(size_t mip = 0; mip <= imgData->num_mipmaps; ++mip)
			{
				size_t dstPitch = width * PixelUtil::getNumElemBytes(imgData->format);

				if (PixelUtil::isCompressed(sourceFormat))
				{
					// Compressed data
					if (decompressDXT)
					{
						DXTColourBlock col;
						DXTInterpolatedAlphaBlock iAlpha;
						DXTExplicitAlphaBlock eAlpha;
						// 4x4 block of decompressed colour
						ColourValue tempColours[16];
						size_t destBpp = PixelUtil::getNumElemBytes(imgData->format);
						size_t sx = std::min(width, (size_t)4);
						size_t sy = std::min(height, (size_t)4);
						size_t destPitchMinus4 = dstPitch - destBpp * sx;
						// slices are done individually
						for(size_t z = 0; z < depth; ++z)
						{
							// 4x4 blocks in x/y
							for (size_t y = 0; y < height; y += 4)
							{
								for (size_t x = 0; x < width; x += 4)
								{
									if (sourceFormat == PF_DXT2 || 
										sourceFormat == PF_DXT3)
									{
										// explicit alpha
										stream->read(&eAlpha, sizeof(DXTExplicitAlphaBlock));
										flipEndian(eAlpha.alphaRow, sizeof(uint16), 4);
										unpackDXTAlpha(eAlpha, tempColours) ;
									}
									else if (sourceFormat == PF_DXT4 || 
										sourceFormat == PF_DXT5)
									{
										// interpolated alpha
										stream->read(&iAlpha, sizeof(DXTInterpolatedAlphaBlock));
										flipEndian(&(iAlpha.alpha_0), sizeof(uint16), 1);
										flipEndian(&(iAlpha.alpha_1), sizeof(uint16), 1);
										unpackDXTAlpha(iAlpha, tempColours) ;
									}
									// always read colour
									stream->read(&col, sizeof(DXTColourBlock));
									flipEndian(&(col.colour_0), sizeof(uint16), 1);
									flipEndian(&(col.colour_1), sizeof(uint16), 1);
									unpackDXTColour(sourceFormat, col, tempColours);

									// write 4x4 block to uncompressed version
									for (size_t by = 0; by < sy; ++by)
									{
										for (size_t bx = 0; bx < sx; ++bx)
										{
											PixelUtil::packColour(tempColours[by*4+bx],
												imgData->format, destPtr);
											destPtr = static_cast<void*>(
												static_cast<uchar*>(destPtr) + destBpp);
										}
										// advance to next row
										destPtr = static_cast<void*>(
											static_cast<uchar*>(destPtr) + destPitchMinus4);
									}
									// next block. Our dest pointer is 4 lines down
									// from where it started
									if (x + 4 >= width)
									{
										// Jump back to the start of the line
										destPtr = static_cast<void*>(
											static_cast<uchar*>(destPtr) - destPitchMinus4);
									}
									else
									{
										// Jump back up 4 rows and 4 pixels to the
										// right to be at the next block to the right
										destPtr = static_cast<void*>(
											static_cast<uchar*>(destPtr) - dstPitch * sy + destBpp * sx);

									}

								}

							}
						}

					}
					else
					{
						// load directly
						// DDS format lies! sizeOrPitch is not always set for DXT!!
						size_t dxtSize = PixelUtil::getMemorySize(width, height, depth, imgData->format);
						stream->read(destPtr, dxtSize);
						destPtr = static_cast<void*>(static_cast<uchar*>(destPtr) + dxtSize);
					}

				}
				else
				{
					// Final data - trim incoming pitch
					size_t srcPitch;
					if (header.flags & DDSD_PITCH)
					{
						srcPitch = header.sizeOrPitch / 
							std::max((size_t)1, mip * 2);
					}
					else
					{
						// assume same as final pitch
						srcPitch = dstPitch;
					}
					assert (dstPitch <= srcPitch);
					long srcAdvance = static_cast<long>(srcPitch) - static_cast<long>(dstPitch);

					for (size_t z = 0; z < imgData->depth; ++z)
					{
						for (size_t y = 0; y < imgData->height; ++y)
						{
							stream->read(destPtr, dstPitch);
							if (srcAdvance > 0)
								stream->skip(srcAdvance);

							destPtr = static_cast<void*>(static_cast<uchar*>(destPtr) + dstPitch);
						}
					}

				}

				
				/// Next mip
				if(width!=1) width /= 2;
				if(height!=1) height /= 2;
				if(depth!=1) depth /= 2;
			}

		}

		DecodeResult ret;
		ret.first = output;
		ret.second = CodecDataPtr(imgData);
		return ret;
		


    }
    //---------------------------------------------------------------------    
    String DDSCodec::getType() const 
    {
        return mType;
    }
    //---------------------------------------------------------------------    
    void DDSCodec::flipEndian(void * pData, size_t size, size_t count) const
    {
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
		for(unsigned int index = 0; index < count; index++)
        {
            flipEndian((void *)((long)pData + (index * size)), size);
        }
#endif
    }
    //---------------------------------------------------------------------    
    void DDSCodec::flipEndian(void * pData, size_t size) const
    {
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
        char swapByte;
        for(unsigned int byteIndex = 0; byteIndex < size/2; byteIndex++)
        {
            swapByte = *(char *)((long)pData + byteIndex);
            *(char *)((long)pData + byteIndex) = *(char *)((long)pData + size - byteIndex - 1);
            *(char *)((long)pData + size - byteIndex - 1) = swapByte;
        }
#endif
    }
	//---------------------------------------------------------------------
	String DDSCodec::magicNumberToFileExt(const char *magicNumberPtr, size_t maxbytes) const
	{
		if (maxbytes >= sizeof(uint32))
		{
			uint32 fileType;
			memcpy(&fileType, magicNumberPtr, sizeof(uint32));
			flipEndian(&fileType, sizeof(uint32), 1);

			if (DDS_MAGIC == fileType)
			{
				return String("dds");
			}
		}

		return StringUtil::BLANK;

	}
	
}

