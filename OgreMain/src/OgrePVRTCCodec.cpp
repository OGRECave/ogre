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

#include "OgrePVRTCCodec.h"
#include "OgreImage.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreBitwise.h"

#define FOURCC(c0, c1, c2, c3) (c0 | (c1 << 8) | (c2 << 16) | (c3 << 24))
#define PVR_TEXTURE_FLAG_TYPE_MASK  0xff

namespace Ogre {
    
#if OGRE_COMPILER == OGRE_COMPILER_MSVC
#pragma pack (push, 1)
#else
#pragma pack (1)
#endif

    const uint32 PVR2_MAGIC = FOURCC('P', 'V', 'R', '!'); 
    const uint32 PVR3_MAGIC = FOURCC('P', 'V', 'R', 3); 

    enum
    {
        kPVRTextureFlagTypePVRTC_2 = 24,
        kPVRTextureFlagTypePVRTC_4
    };

    enum
    {
        kPVRTC1_PF_2BPP_RGB,
        kPVRTC1_PF_2BPP_RGBA,
        kPVRTC1_PF_4BPP_RGB,
        kPVRTC1_PF_4BPP_RGBA,
        kPVRTC2_PF_2BPP,
        kPVRTC2_PF_4BPP
    };

    typedef struct _PVRTCTexHeaderV2
    {
        uint32 headerLength;
        uint32 height;
        uint32 width;
        uint32 numMipmaps;
        uint32 flags;
        uint32 dataLength;
        uint32 bpp;
        uint32 bitmaskRed;
        uint32 bitmaskGreen;
        uint32 bitmaskBlue;
        uint32 bitmaskAlpha;
        uint32 pvrTag;
        uint32 numSurfs;
    } PVRTCTexHeaderV2;

    typedef struct _PVRTCTexHeaderV3
    {
        uint32  version;         //Version of the file header, used to identify it.
        uint32  flags;           //Various format flags.
        uint64  pixelFormat;     //The pixel format, 8cc value storing the 4 channel identifiers and their respective sizes.
        uint32  colourSpace;     //The Colour Space of the texture, currently either linear RGB or sRGB.
        uint32  channelType;     //Variable type that the channel is stored in. Supports signed/unsigned int/short/byte or float for now.
        uint32  height;          //Height of the texture.
        uint32  width;           //Width of the texture.
        uint32  depth;           //Depth of the texture. (Z-slices)
        uint32  numSurfaces;     //Number of members in a Texture Array.
        uint32  numFaces;        //Number of faces in a Cube Map. Maybe be a value other than 6.
        uint32  mipMapCount;     //Number of MIP Maps in the texture - NB: Includes top level.
        uint32  metaDataSize;    //Size of the accompanying meta data.
    } PVRTCTexHeaderV3;

    typedef struct _PVRTCMetaData
    {
        uint32 DevFOURCC;
        uint32 u32Key;
        uint32 u32DataSize;
        uint8* Data;
    } PVRTCMetadata;
    
#if OGRE_COMPILER == OGRE_COMPILER_MSVC
#pragma pack (pop)
#else
#pragma pack ()
#endif

    //---------------------------------------------------------------------
    PVRTCCodec* PVRTCCodec::msInstance = 0;
    //---------------------------------------------------------------------
    void PVRTCCodec::startup(void)
    {
        if (!msInstance)
        {
            LogManager::getSingleton().logMessage(
                LML_NORMAL,
                "PVRTC codec registering");

            msInstance = OGRE_NEW PVRTCCodec();
            Codec::registerCodec(msInstance);
        }
    }
    //---------------------------------------------------------------------
    void PVRTCCodec::shutdown(void)
    {
        if(msInstance)
        {
            Codec::unregisterCodec(msInstance);
            OGRE_DELETE msInstance;
            msInstance = 0;
        }
    }
    //---------------------------------------------------------------------
    PVRTCCodec::PVRTCCodec():
        mType("pvr")
    { 
    }
    //---------------------------------------------------------------------
    DataStreamPtr PVRTCCodec::encode(MemoryDataStreamPtr& input, Codec::CodecDataPtr& pData) const
    {        
        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
                    "PVRTC encoding not supported",
                    "PVRTCCodec::encode" ) ;
    }
    //---------------------------------------------------------------------
    void PVRTCCodec::encodeToFile(MemoryDataStreamPtr& input,
        const String& outFileName, Codec::CodecDataPtr& pData) const
    {
        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
                    "PVRTC encoding not supported",
                    "PVRTCCodec::encodeToFile" ) ;
    }
    //---------------------------------------------------------------------
    Codec::DecodeResult PVRTCCodec::decode(DataStreamPtr& stream) const
    {
        // Assume its a pvr 2 header
        PVRTCTexHeaderV2 headerV2;
        stream->read(&headerV2, sizeof(PVRTCTexHeaderV2));
        stream->seek(0);

        if (PVR2_MAGIC == headerV2.pvrTag)
        {           
            return decodeV2(stream);
        }

        // Try it as pvr 3 header
        PVRTCTexHeaderV3 headerV3;
        stream->read(&headerV3, sizeof(PVRTCTexHeaderV3));
        stream->seek(0);

        if (PVR3_MAGIC == headerV3.version)
        {
            return decodeV3(stream);
        }

        
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "This is not a PVR2 / PVR3 file!", "PVRTCCodec::decode");
    }
    //---------------------------------------------------------------------    
    Codec::DecodeResult PVRTCCodec::decodeV2(DataStreamPtr& stream) const
    {
        PVRTCTexHeaderV2 header;
        uint32 flags = 0, formatFlags = 0;
        size_t numFaces = 1; // Assume one face until we know otherwise

        ImageData *imgData = OGRE_NEW ImageData();
        MemoryDataStreamPtr output;

        // Read the PVRTC header
        stream->read(&header, sizeof(PVRTCTexHeaderV2));

        // Get format flags
        flags = header.flags;
        flipEndian(&flags, sizeof(uint32));
        formatFlags = flags & PVR_TEXTURE_FLAG_TYPE_MASK;

        uint32 bitmaskAlpha = header.bitmaskAlpha;
        flipEndian(&bitmaskAlpha, sizeof(uint32));

        if (formatFlags == kPVRTextureFlagTypePVRTC_4 || formatFlags == kPVRTextureFlagTypePVRTC_2)
        {
            if (formatFlags == kPVRTextureFlagTypePVRTC_4)
            {
                imgData->format = bitmaskAlpha ? PF_PVRTC_RGBA4 : PF_PVRTC_RGB4;
            }
            else if (formatFlags == kPVRTextureFlagTypePVRTC_2)
            {
                imgData->format = bitmaskAlpha ? PF_PVRTC_RGBA2 : PF_PVRTC_RGB2;
            }

            imgData->depth = 1;
            imgData->width = header.width;
            imgData->height = header.height;
            imgData->num_mipmaps = static_cast<uint8>(header.numMipmaps);

            // PVRTC is a compressed format
            imgData->flags |= IF_COMPRESSED;
        }

        // Calculate total size from number of mipmaps, faces and size
        imgData->size = Image::calculateSize(imgData->num_mipmaps, numFaces, 
                                             imgData->width, imgData->height, imgData->depth, imgData->format);

        // Bind output buffer
        output.bind(OGRE_NEW MemoryDataStream(imgData->size));

        // Now deal with the data
        void *destPtr = output->getPtr();
        stream->read(destPtr, imgData->size);
        destPtr = static_cast<void*>(static_cast<uchar*>(destPtr));

        DecodeResult ret;
        ret.first = output;
        ret.second = CodecDataPtr(imgData);

        return ret;
    }
    //---------------------------------------------------------------------    
    Codec::DecodeResult PVRTCCodec::decodeV3(DataStreamPtr& stream) const
    {
        PVRTCTexHeaderV3 header;
        PVRTCMetadata metadata;
        uint32 flags = 0;
        size_t numFaces = 1; // Assume one face until we know otherwise

        ImageData *imgData = OGRE_NEW ImageData();
        MemoryDataStreamPtr output;

        // Read the PVRTC header
        stream->read(&header, sizeof(PVRTCTexHeaderV3));

        // Read the PVRTC metadata
        if(header.metaDataSize)
        {
            stream->read(&metadata, sizeof(PVRTCMetadata));
        }

        // Identify the pixel format
        switch (header.pixelFormat)
        {
            case kPVRTC1_PF_2BPP_RGB:
                imgData->format = PF_PVRTC_RGB2;
                break;
            case kPVRTC1_PF_2BPP_RGBA:
                imgData->format = PF_PVRTC_RGBA2;
                break;
            case kPVRTC1_PF_4BPP_RGB:
                imgData->format = PF_PVRTC_RGB4;
                break;
            case kPVRTC1_PF_4BPP_RGBA:
                imgData->format = PF_PVRTC_RGBA4;
                break;
            case kPVRTC2_PF_2BPP:
                imgData->format = PF_PVRTC2_2BPP;
                break;
            case kPVRTC2_PF_4BPP:
                imgData->format = PF_PVRTC2_4BPP;
                break;
        }

        // Get format flags
        flags = header.flags;
        flipEndian(&flags, sizeof(uint32));

        imgData->depth = header.depth;
        imgData->width = header.width;
        imgData->height = header.height;
        imgData->num_mipmaps = static_cast<uint8>(header.mipMapCount);

        // PVRTC is a compressed format
        imgData->flags |= IF_COMPRESSED;

        if(header.numFaces == 6)
            imgData->flags |= IF_CUBEMAP;

        if(header.depth > 1)
            imgData->flags |= IF_3D_TEXTURE;

        // Calculate total size from number of mipmaps, faces and size
        imgData->size = Image::calculateSize(imgData->num_mipmaps, numFaces, 
                                             imgData->width, imgData->height, imgData->depth, imgData->format);

        // Bind output buffer
        output.bind(OGRE_NEW MemoryDataStream(imgData->size));

        // Now deal with the data
        void *destPtr = output->getPtr();
        
        uint width = imgData->width;
        uint height = imgData->height;
        uint depth = imgData->depth;

        // All mips for a surface, then each face
        for(size_t mip = 0; mip <= imgData->num_mipmaps; ++mip)
        {
            for(size_t surface = 0; surface < header.numSurfaces; ++surface)
            {
                for(size_t i = 0; i < numFaces; ++i)
                {
                    // Load directly
                    size_t pvrSize = PixelUtil::getMemorySize(width, height, depth, imgData->format);
                    stream->read(destPtr, pvrSize);
                    destPtr = static_cast<void*>(static_cast<uchar*>(destPtr) + pvrSize);
                }
            }

            // Next mip
            if(width!=1) width /= 2;
            if(height!=1) height /= 2;
            if(depth!=1) depth /= 2;
        }

        DecodeResult ret;
        ret.first = output;
        ret.second = CodecDataPtr(imgData);

        return ret;
    }
    //---------------------------------------------------------------------    
    String PVRTCCodec::getType() const 
    {
        return mType;
    }
    //---------------------------------------------------------------------    
    void PVRTCCodec::flipEndian(void * pData, size_t size, size_t count)
    {
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
		Bitwise::bswapChunks(pData, size, count);
#endif
    }
    //---------------------------------------------------------------------    
    void PVRTCCodec::flipEndian(void * pData, size_t size)
    {
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
        Bitwise::bswapBuffer(pData, size);
#endif
    }
    //---------------------------------------------------------------------
    String PVRTCCodec::magicNumberToFileExt(const char *magicNumberPtr, size_t maxbytes) const
    {
        if (maxbytes >= sizeof(uint32))
        {
            uint32 fileType;
            memcpy(&fileType, magicNumberPtr, sizeof(uint32));
			flipEndian(&fileType, sizeof(uint32));

            if (PVR3_MAGIC == fileType || PVR2_MAGIC == fileType)
            {
                return String("pvr");
            }
        }

        return BLANKSTRING;
    }
}
