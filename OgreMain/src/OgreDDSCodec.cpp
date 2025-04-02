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

#include "OgreDDSCodec.h"
#include "OgreImage.h"

namespace Ogre {
    // Internal DDS structure definitions
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
        uint32 caps3;
        uint32 caps4;
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

    // Extended header
    struct DDSExtendedHeader
    {
        uint32 dxgiFormat;
        uint32 resourceDimension;
        uint32 miscFlag; // see D3D11_RESOURCE_MISC_FLAG
        uint32 arraySize;
        uint32 reserved;
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

namespace {
    const uint32 DDS_MAGIC = FOURCC('D', 'D', 'S', ' ');
    const uint32 DDS_PIXELFORMAT_SIZE = 8 * sizeof(uint32);
    const uint32 DDS_CAPS_SIZE = 4 * sizeof(uint32);
    const uint32 DDS_HEADER_SIZE = 19 * sizeof(uint32) + DDS_PIXELFORMAT_SIZE + DDS_CAPS_SIZE;

    const uint32 DDSD_CAPS = 0x00000001;
    const uint32 DDSD_HEIGHT = 0x00000002;
    const uint32 DDSD_WIDTH = 0x00000004;
    const uint32 DDSD_PIXELFORMAT = 0x00001000;
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

    // Currently unused
//    const uint32 DDSD_PITCH = 0x00000008;
//    const uint32 DDSD_MIPMAPCOUNT = 0x00020000;
//    const uint32 DDSD_LINEARSIZE = 0x00080000;

    // Special FourCC codes
    const uint32 D3DFMT_R16F            = 111;
    const uint32 D3DFMT_G16R16F         = 112;
    const uint32 D3DFMT_A16B16G16R16F   = 113;
    const uint32 D3DFMT_R32F            = 114;
    const uint32 D3DFMT_G32R32F         = 115;
    const uint32 D3DFMT_A32B32G32R32F   = 116;
}

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
            Codec::unregisterCodec(msInstance);
            OGRE_DELETE msInstance;
            msInstance = 0;
        }

    }
    //---------------------------------------------------------------------
    DDSCodec::DDSCodec():
        mType("dds"),
        mDecodeEnforce(false)
    { 
    }
    //---------------------------------------------------------------------
    DataStreamPtr DDSCodec::encode(const Any& input) const
    {
        Image* image = any_cast<Image*>(input);

        bool isCubeMap = image->hasFlag(IF_CUBEMAP);

        // Establish texture attributes
        bool isVolume = (image->getDepth() > 1);
        bool isFloat32r = (image->getFormat() == PF_FLOAT32_R);
        bool isFloat16 = (image->getFormat() == PF_FLOAT16_RGBA);
        bool isFloat16r = (image->getFormat() == PF_FLOAT16_R);
        bool isFloat32 = (image->getFormat() == PF_FLOAT32_RGBA);
        bool notImplemented = false;
        String notImplementedString = "";

        // Check for all the 'not implemented' conditions
        if ((isVolume == true)&&(image->getWidth() != image->getHeight()))
        {
            // Square textures only
            notImplemented = true;
            notImplementedString += "non square textures";
        }

        uint32 size = 1;
        while (size < image->getWidth())
        {
            size <<= 1;
        }
        if (size != image->getWidth())
        {
            // Power two textures only
            notImplemented = true;
            notImplementedString += "non power two textures";
        }

        switch(image->getFormat())
        {
        case PF_A8R8G8B8:
        case PF_X8R8G8B8:
        case PF_R8G8B8:
        case PF_A8B8G8R8:
        case PF_X8B8G8R8:
        case PF_B8G8R8:
        case PF_FLOAT32_R:
        case PF_FLOAT16_R:
        case PF_FLOAT16_RGBA:
        case PF_FLOAT32_RGBA:
            break;
        default:
            // No crazy FOURCC or 565 et al. file formats at this stage
            notImplemented = true;
            notImplementedString = PixelUtil::getFormatName(image->getFormat());
            break;
        }       



        // Except if any 'not implemented' conditions were met
        if (notImplemented)
        {
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
                        "DDS encoding for " + notImplementedString + " not supported");
        }
        else
        {
            // Build header and write to disk

            // Variables for some DDS header flags
            bool hasAlpha = false;
            uint32 ddsHeaderFlags = 0;
            uint32 ddsHeaderRgbBits = 0;
            uint32 ddsHeaderSizeOrPitch = 0;
            uint32 ddsHeaderCaps1 = 0;
            uint32 ddsHeaderCaps2 = 0;
            uint32 ddsMagic = DDS_MAGIC;

            // Initalise the header flags
            ddsHeaderFlags = (isVolume) ? DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT|DDSD_DEPTH|DDSD_PIXELFORMAT :
                DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT|DDSD_PIXELFORMAT;  

            bool flipRgbMasks = false;

            // Initalise the rgbBits flags
            switch(image->getFormat())
            {
            case PF_A8B8G8R8:
                flipRgbMasks = true;
                OGRE_FALLTHROUGH;
            case PF_A8R8G8B8:
                ddsHeaderRgbBits = 8 * 4;
                hasAlpha = true;
                break;
            case PF_X8B8G8R8:
                flipRgbMasks = true;
                OGRE_FALLTHROUGH;
            case PF_X8R8G8B8:
                ddsHeaderRgbBits = 8 * 4;
                break;
            case PF_B8G8R8:
            case PF_R8G8B8:
                ddsHeaderRgbBits = 8 * 3;
                break;
            case PF_FLOAT32_R:
                ddsHeaderRgbBits = 32;
                break;
            case PF_FLOAT16_R:
                ddsHeaderRgbBits = 16;
                break;
            case PF_FLOAT16_RGBA:
                ddsHeaderRgbBits = 16 * 4;
                hasAlpha = true;
                break;
            case PF_FLOAT32_RGBA:
                ddsHeaderRgbBits = 32 * 4;
                hasAlpha = true;
                break;
            default:
                ddsHeaderRgbBits = 0;
                break;
            }

            // Initalise the SizeOrPitch flags (power two textures for now)
            ddsHeaderSizeOrPitch = static_cast<uint32>(ddsHeaderRgbBits * image->getWidth());

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

            if( image->getNumMipmaps() > 0 )
                ddsHeaderCaps1 |= DDSCAPS_MIPMAP;

            // Populate the DDS header information
            DDSHeader ddsHeader;
            ddsHeader.size = DDS_HEADER_SIZE;
            ddsHeader.flags = ddsHeaderFlags;       
            ddsHeader.width = image->getWidth();
            ddsHeader.height = image->getHeight();
            ddsHeader.depth = (uint32)(isVolume ? image->getDepth() : 0);
            ddsHeader.depth = (uint32)(isCubeMap ? 6 : ddsHeader.depth);
            ddsHeader.mipMapCount = image->getNumMipmaps() + 1;
            ddsHeader.sizeOrPitch = ddsHeaderSizeOrPitch;
            for (unsigned int & reserved1 : ddsHeader.reserved1) // XXX nasty constant 11
            {
                reserved1 = 0;
            }
            ddsHeader.reserved2 = 0;

            ddsHeader.pixelFormat.size = DDS_PIXELFORMAT_SIZE;
            ddsHeader.pixelFormat.flags = (hasAlpha) ? DDPF_RGB|DDPF_ALPHAPIXELS : DDPF_RGB;
            ddsHeader.pixelFormat.flags = (isFloat32r || isFloat16r || isFloat16 || isFloat32) ? DDPF_FOURCC : ddsHeader.pixelFormat.flags;
            if (isFloat32r) {
                ddsHeader.pixelFormat.fourCC = D3DFMT_R32F;
            }
            else if (isFloat16r) {
                ddsHeader.pixelFormat.fourCC = D3DFMT_R16F;
            }
            else if (isFloat16) {
                ddsHeader.pixelFormat.fourCC = D3DFMT_A16B16G16R16F;
            }
            else if (isFloat32) {
                ddsHeader.pixelFormat.fourCC = D3DFMT_A32B32G32R32F;
            }
            else {
                ddsHeader.pixelFormat.fourCC = 0;
            }
            ddsHeader.pixelFormat.rgbBits = ddsHeaderRgbBits;

            ddsHeader.pixelFormat.alphaMask = (hasAlpha)   ? 0xFF000000 : 0x00000000;
            ddsHeader.pixelFormat.alphaMask = (isFloat32r || isFloat16r) ? 0x00000000 : ddsHeader.pixelFormat.alphaMask;
            ddsHeader.pixelFormat.redMask   = (isFloat32r || isFloat16r) ? 0xFFFFFFFF :0x00FF0000;
            ddsHeader.pixelFormat.greenMask = (isFloat32r || isFloat16r) ? 0x00000000 :0x0000FF00;
            ddsHeader.pixelFormat.blueMask  = (isFloat32r || isFloat16r) ? 0x00000000 :0x000000FF;

            if( flipRgbMasks )
                std::swap( ddsHeader.pixelFormat.redMask, ddsHeader.pixelFormat.blueMask );

            ddsHeader.caps.caps1 = ddsHeaderCaps1;
            ddsHeader.caps.caps2 = ddsHeaderCaps2;
//          ddsHeader.caps.reserved[0] = 0;
//          ddsHeader.caps.reserved[1] = 0;

            // Swap endian
            flipEndian(&ddsMagic, sizeof(uint32));
            flipEndian(&ddsHeader, 4, sizeof(DDSHeader) / 4);

            char *tmpData = 0;
            char *dataPtr = (char*)image->getData();

            if( image->getFormat() == PF_B8G8R8 )
            {
                PixelBox src( image->getSize() / 3, 1, 1, PF_B8G8R8, image->getData() );
                tmpData = new char[image->getSize()];
                PixelBox dst( image->getSize() / 3, 1, 1, PF_R8G8B8, tmpData );

                PixelUtil::bulkPixelConversion( src, dst );

                dataPtr = tmpData;
            }

            size_t totalSize = sizeof(uint32) + DDS_HEADER_SIZE + image->getSize();
            auto pMemStream = OGRE_NEW Ogre::MemoryDataStream(totalSize);

            pMemStream->write(&ddsMagic, sizeof(uint32));
            pMemStream->write(&ddsHeader, DDS_HEADER_SIZE);
            pMemStream->write(dataPtr, image->getSize());
            pMemStream->seek(0);

            delete [] tmpData;

            return Ogre::DataStreamPtr(pMemStream);
        }
    }
    //---------------------------------------------------------------------
    void DDSCodec::encodeToFile(const Any& input, const String& outFileName) const
    {
        DataStreamPtr strm = encode(input);

        try
        {
            // Write the file
            std::ofstream of;
            of.open(outFileName.c_str(), std::ios_base::binary | std::ios_base::out);

            const size_t buffSize = 4096;
            char buffer[buffSize];

            while (!strm->eof()) {
                size_t bytesRead = strm->read(buffer, buffSize);
                of.write(buffer, bytesRead);
            }

            of.close();
        }
        catch(...)
        {
        }
    }
    //---------------------------------------------------------------------
    PixelFormat DDSCodec::convertDXToOgreFormat(uint32 dxfmt) const
    {
        switch (dxfmt) {
			case 2: // DXGI_FORMAT_R32G32B32A32_FLOAT
				return PF_FLOAT32_RGBA;
			case 3: // DXGI_FORMAT_R32G32B32A32_UINT
				return PF_R32G32B32A32_UINT;
			case 4: //DXGI_FORMAT_R32G32B32A32_SINT
				return PF_R32G32B32A32_SINT;
			case 6: // DXGI_FORMAT_R32G32B32_FLOAT
				return PF_FLOAT32_RGB;
			case 7: // DXGI_FORMAT_R32G32B32_UINT
				return PF_R32G32B32_UINT;
			case 8: // DXGI_FORMAT_R32G32B32_SINT
				return PF_R32G32B32_SINT;
			case 10: // DXGI_FORMAT_R16G16B16A16_FLOAT
				return PF_FLOAT16_RGBA;
			case 12: // DXGI_FORMAT_R16G16B16A16_UINT
				return PF_R16G16B16A16_UINT;
			case 13: // DXGI_FORMAT_R16G16B16A16_SNORM
				return PF_R16G16B16A16_SNORM;
			case 14: // DXGI_FORMAT_R16G16B16A16_SINT
				return PF_R16G16B16A16_SINT;
			case 16: // DXGI_FORMAT_R32G32_FLOAT
				return PF_FLOAT32_GR;
			case 17: // DXGI_FORMAT_R32G32_UINT
				return PF_R32G32_UINT;
			case 18: // DXGI_FORMAT_R32G32_SINT
				return PF_R32G32_SINT;
			case 24: // DXGI_FORMAT_R10G10B10A2_UNORM
			case 25: // DXGI_FORMAT_R10G10B10A2_UINT
				return PF_A2B10G10R10;
			case 26: // DXGI_FORMAT_R11G11B10_FLOAT
				return PF_R11G11B10_FLOAT;
			case 28: // DXGI_FORMAT_R8G8B8A8_UNORM
			case 29: // DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
				return PF_A8B8G8R8;
			case 30: // DXGI_FORMAT_R8G8B8A8_UINT
				return PF_R8G8B8A8_UINT;
			case 31: // DXGI_FORMAT_R8G8B8A8_SNORM
				return PF_R8G8B8A8_SNORM;
			case 32: // DXGI_FORMAT_R8G8B8A8_SINT
				return PF_R8G8B8A8_SINT;
			case 34: // DXGI_FORMAT_R16G16_FLOAT
				return PF_FLOAT16_GR;
			case 35: // DXGI_FORMAT_R16G16_UNORM
				return PF_SHORT_GR;
			case 36: // DXGI_FORMAT_R16G16_UINT
				return PF_R16G16_UINT;
			case 37: // DXGI_FORMAT_R16G16_SNORM
				return PF_R16G16_SNORM;
			case 38: // DXGI_FORMAT_R16G16_SINT
				return PF_R16G16_SINT;
			case 41: // DXGI_FORMAT_R32_FLOAT
				return PF_FLOAT32_R;
			case 42: // DXGI_FORMAT_R32_UINT
				return PF_R32_UINT;
			case 43: // DXGI_FORMAT_R32_SINT
				return PF_R32_SINT;
			case 49: // DXGI_FORMAT_R8G8_UNORM
			case 50: // DXGI_FORMAT_R8G8_UINT
				return PF_R8G8_UINT;
			case 52: // DXGI_FORMAT_R8G8_SINT
				return PF_R8G8_SINT;
			case 54: // DXGI_FORMAT_R16_FLOAT
				return PF_FLOAT16_R;
			case 56: // DXGI_FORMAT_R16_UNORM
				return PF_L16;
			case 57: // DXGI_FORMAT_R16_UINT
				return PF_R16_UINT;
			case 58: // DXGI_FORMAT_R16_SNORM
				return PF_R16_SNORM;
			case 59: // DXGI_FORMAT_R16_SINT
				return PF_R16_SINT;
			case 61: // DXGI_FORMAT_R8_UNORM
				return PF_R8;
			case 62: // DXGI_FORMAT_R8_UINT
				return PF_R8_UINT;
			case 63: // DXGI_FORMAT_R8_SNORM
				return PF_R8_SNORM;
			case 64: // DXGI_FORMAT_R8_SINT
				return PF_R8_SINT;
			case 65: // DXGI_FORMAT_A8_UNORM
				return PF_A8;
            case 80: // DXGI_FORMAT_BC4_UNORM
                return PF_BC4_UNORM;
            case 81: // DXGI_FORMAT_BC4_SNORM
                return PF_BC4_SNORM;
            case 83: // DXGI_FORMAT_BC5_UNORM
                return PF_BC5_UNORM;
            case 84: // DXGI_FORMAT_BC5_SNORM
                return PF_BC5_SNORM;
			case 85: // DXGI_FORMAT_B5G6R5_UNORM
				return PF_R5G6B5;
			case 86: // DXGI_FORMAT_B5G5R5A1_UNORM
				return PF_A1R5G5B5;
			case 87: // DXGI_FORMAT_B8G8R8A8_UNORM
				return PF_A8R8G8B8;
			case 88: // DXGI_FORMAT_B8G8R8X8_UNORM
				return PF_X8R8G8B8;
            case 95: // DXGI_FORMAT_BC6H_UF16
                return PF_BC6H_UF16;
            case 96: // DXGI_FORMAT_BC6H_SF16
                return PF_BC6H_SF16;
            case 98: // DXGI_FORMAT_BC7_UNORM
            case 99: // DXGI_FORMAT_BC7_UNORM_SRGB
                return PF_BC7_UNORM;
            case 20: // DXGI_FORMAT_D32_FLOAT_S8X24_UINT
            case 22: // DXGI_FORMAT_X32_TYPELESS_G8X24_UINT
            case 40: // DXGI_FORMAT_D32_FLOAT
            case 45: // DXGI_FORMAT_D24_UNORM_S8_UINT
            case 47: // DXGI_FORMAT_X24_TYPELESS_G8_UINT
            case 55: // DXGI_FORMAT_D16_UNORM
            default:
                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
                            "Unsupported DirectX format found in DDS file",
                            "DDSCodec::convertDXToOgreFormat");
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
        case FOURCC('A','T','I','1'):
        case FOURCC('B','C','4','U'):
            return PF_BC4_UNORM;
        case FOURCC('B','C','4','S'):
            return PF_BC4_SNORM;
        case FOURCC('A','T','I','2'):
        case FOURCC('B','C','5','U'):
            return PF_BC5_UNORM;
        case FOURCC('B','C','5','S'):
            return PF_BC5_SNORM;
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
                "DDSCodec::convertFourCCFormat");
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
                uint64 testMasks[4];
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
        for (unsigned short row : block.alphaRow)
        {
            for (size_t x = 0; x < 4; ++x)
            {
                // Shift and mask off to 4 bits
                uint8 val = static_cast<uint8>(row >> (x * 4) & 0xF);
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
        // Adaptive 3-bit alpha part
        float derivedAlphas[8];

        // Explicit extremes
        derivedAlphas[0] = ((float) block.alpha_0) * (1.0f / 255.0f);
        derivedAlphas[1] = ((float) block.alpha_1) * (1.0f / 255.0f);

        if(block.alpha_0 > block.alpha_1)
        {
            // 6 interpolated alpha values.
            // full range including extremes at [0] and [7]
            // we want to fill in [1] through [6] at weights ranging
            // from 1/7 to 6/7
            for(size_t i = 1; i < 7; ++i)
                derivedAlphas[i + 1] = (derivedAlphas[0] * (7 - i) + derivedAlphas[1] * i) * (1.0f / 7.0f);
        }
        else
        {
            // 4 interpolated alpha values.
            // full range including extremes at [0] and [5]
            // we want to fill in [1] through [4] at weights ranging
            // from 1/5 to 4/5
            for(size_t i = 1; i < 5; ++i)
                derivedAlphas[i + 1] = (derivedAlphas[0] * (5 - i) + derivedAlphas[1] * i) * (1.0f / 5.0f);

            derivedAlphas[6] = 0.0f;
            derivedAlphas[7] = 1.0f;
        }

        // Ok, now we've built the reference values, process the indexes
        uint32 dw = block.indexes[0] | (block.indexes[1] << 8) | (block.indexes[2] << 16);

        for(size_t i = 0; i < 8; ++i, dw >>= 3)
            pCol[i].a = derivedAlphas[dw & 0x7];

        dw = block.indexes[3] | (block.indexes[4] << 8) | (block.indexes[5] << 16);

        for(size_t i = 8; i < 16; ++i, dw >>= 3)
            pCol[i].a = derivedAlphas[dw & 0x7];
    }
    //---------------------------------------------------------------------
    void DDSCodec::decode(const DataStreamPtr& stream, const Any& output) const
    {
        Image* image = any_cast<Image*>(output);
        // Read 4 character code
        uint32 fileType;
        stream->read(&fileType, sizeof(uint32));
        flipEndian(&fileType, sizeof(uint32));
        
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

        uint32 imgDepth = 1; // (deal with volume later)
        uint32 numFaces = 1; // assume one face until we know otherwise
        uint32 num_mipmaps = 0;
        PixelFormat format = PF_UNKNOWN;

        if (header.caps.caps1 & DDSCAPS_MIPMAP)
        {
            num_mipmaps = static_cast<uint8>(header.mipMapCount - 1);
        }
        else
        {
            num_mipmaps = 0;
        }

        bool decompressDXT = false;
        // Figure out basic image type
        if (header.caps.caps2 & DDSCAPS2_CUBEMAP)
        {
            numFaces = 6;
        }
        else if (header.caps.caps2 & DDSCAPS2_VOLUME)
        {
            imgDepth = header.depth;
        }
        // Pixel format
        PixelFormat sourceFormat = PF_UNKNOWN;

        if (header.pixelFormat.flags & DDPF_FOURCC)
        {
            // Check if we have an DX10 style extended header and read it. This is necessary for B6H and B7 formats
            if(header.pixelFormat.fourCC == FOURCC('D', 'X', '1', '0'))
            {
                DDSExtendedHeader extHeader;
                stream->read(&extHeader, sizeof(DDSExtendedHeader));

                // Endian flip if required, all 32-bit values
                flipEndian(&header, sizeof(DDSExtendedHeader));
                sourceFormat = convertDXToOgreFormat(extHeader.dxgiFormat);
            }
            else
            {
                sourceFormat = convertFourCCFormat(header.pixelFormat.fourCC);
            }
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
            if (Root::getSingleton().getRenderSystem() == NULL ||
                !Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_TEXTURE_COMPRESSION_DXT) ||
                mDecodeEnforce)
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
                    flipEndian(&(block.colour_0), sizeof(uint16));
                    flipEndian(&(block.colour_1), sizeof(uint16));
                    // skip back since we'll need to read this again
                    stream->skip(0 - (long)sizeof(DXTColourBlock));
                    // colour_0 <= colour_1 means transparency in DXT1
                    if (block.colour_0 <= block.colour_1)
                    {
                        format = PF_BYTE_RGBA;
                    }
                    else
                    {
                        format = PF_BYTE_RGB;
                    }
                    break;
                case PF_DXT2:
                case PF_DXT3:
                case PF_DXT4:
                case PF_DXT5:
                    // full alpha present, formats vary only in encoding 
                    format = PF_BYTE_RGBA;
                    break;
                default:
                    // all other cases need no special format handling
                    break;
                }
            }
            else
            {
                // Use original format
                format = sourceFormat;
            }
        }
        else // not compressed
        {
            // Don't test against DDPF_RGB since greyscale DDS doesn't set this
            // just derive any other kind of format
            format = sourceFormat;
        }

        // Calculate total size from number of mipmaps, faces and size
        image->create(format, header.width, header.height, imgDepth, numFaces, num_mipmaps);

        // Now deal with the data
        void* destPtr = image->getData();

        // all mips for a face, then each face
        for(size_t i = 0; i < numFaces; ++i)
        {
            uint32 width = image->getWidth();
            uint32 height = image->getHeight();
            uint32 depth = image->getDepth();

            for(size_t mip = 0; mip <= num_mipmaps; ++mip)
            {
                size_t dstPitch = width * PixelUtil::getNumElemBytes(format);
                
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
                        size_t destBpp = PixelUtil::getNumElemBytes(format);

                        // slices are done individually
                        for(size_t z = 0; z < depth; ++z)
                        {
                            size_t remainingHeight = height;

                            // 4x4 blocks in x/y
                            for (size_t y = 0; y < height; y += 4)
                            {
                                size_t sy = std::min<size_t>( remainingHeight, 4u );
                                remainingHeight -= sy;

                                size_t remainingWidth = width;

                                for (size_t x = 0; x < width; x += 4)
                                {
                                    size_t sx = std::min<size_t>( remainingWidth, 4u );
                                    size_t destPitchMinus4 = dstPitch - destBpp * sx;

                                    remainingWidth -= sx;

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
                                        flipEndian(&(iAlpha.alpha_0), sizeof(uint16));
                                        flipEndian(&(iAlpha.alpha_1), sizeof(uint16));
                                        unpackDXTAlpha(iAlpha, tempColours) ;
                                    }
                                    // always read colour
                                    stream->read(&col, sizeof(DXTColourBlock));
                                    flipEndian(&(col.colour_0), sizeof(uint16));
                                    flipEndian(&(col.colour_1), sizeof(uint16));
                                    unpackDXTColour(sourceFormat, col, tempColours);

                                    // write 4x4 block to uncompressed version
                                    for (size_t by = 0; by < sy; ++by)
                                    {
                                        for (size_t bx = 0; bx < sx; ++bx)
                                        {
                                            PixelUtil::packColour(tempColours[by*4+bx],
                                                format, destPtr);
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
                        size_t dxtSize = PixelUtil::getMemorySize(width, height, depth, format);
                        stream->read(destPtr, dxtSize);
                        destPtr = static_cast<void*>(static_cast<uchar*>(destPtr) + dxtSize);
                    }

                }
                else
                {
                    // Note: We assume the source and destination have the same pitch
                    for (size_t z = 0; z < depth; ++z)
                    {
                        for (size_t y = 0; y < height; ++y)
                        {
                            stream->read(destPtr, dstPitch);
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
    }
    //---------------------------------------------------------------------    
    String DDSCodec::getType() const 
    {
        return mType;
    }
    //---------------------------------------------------------------------
    String DDSCodec::magicNumberToFileExt(const char *magicNumberPtr, size_t maxbytes) const
    {
        if (maxbytes >= sizeof(uint32))
        {
            uint32 fileType;
            memcpy(&fileType, magicNumberPtr, sizeof(uint32));
            flipEndian(&fileType, sizeof(uint32));

            if (DDS_MAGIC == fileType)
            {
                return String("dds");
            }
        }

        return BLANKSTRING;

    }
    //---------------------------------------------------------------------
    bool DDSCodec::setParameter(const String& name, const String& value)
    {
        if (name == "decode_enforce")
            return StringConverter::parse(value, mDecodeEnforce);

        return false;
    }
    
}

