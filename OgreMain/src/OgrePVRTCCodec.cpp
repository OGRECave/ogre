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

#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgrePVRTCCodec.h"
#include "OgreImage.h"
#include "OgreException.h"

#include "OgreLogManager.h"
#include "OgreStringConverter.h"

#define FOURCC(c0, c1, c2, c3) (c0 | (c1 << 8) | (c2 << 16) | (c3 << 24))
#define PVR_TEXTURE_FLAG_TYPE_MASK	0xff

namespace Ogre {
	
#if OGRE_COMPILER == OGRE_COMPILER_MSVC
#pragma pack (push, 1)
#else
#pragma pack (1)
#endif

    const uint32 PVR_MAGIC = FOURCC('P', 'V', 'R', '!');

    enum
    {
        kPVRTextureFlagTypePVRTC_2 = 24,
        kPVRTextureFlagTypePVRTC_4
    };
    
    typedef struct _PVRTCTexHeader
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
    } PVRTCTexHeader;
	
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
			Codec::unRegisterCodec(msInstance);
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
    DataStreamPtr PVRTCCodec::code(MemoryDataStreamPtr& input, Codec::CodecDataPtr& pData) const
    {        
		OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
                    "PVRTC encoding not supported",
                    "PVRTCCodec::code" ) ;
    }
    //---------------------------------------------------------------------
    void PVRTCCodec::codeToFile(MemoryDataStreamPtr& input, 
        const String& outFileName, Codec::CodecDataPtr& pData) const
    {
		OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
                    "PVRTC encoding not supported",
                    "PVRTCCodec::codeToFile" ) ;
	}
    //---------------------------------------------------------------------
    Codec::DecodeResult PVRTCCodec::decode(DataStreamPtr& stream) const
    {
        PVRTCTexHeader header;
        uint32 flags = 0, pvrTag = 0, formatFlags = 0;
        size_t numFaces = 1; // Assume one face until we know otherwise

        ImageData *imgData = OGRE_NEW ImageData();
		MemoryDataStreamPtr output;

        // Read the PVRTC header
        stream->read(&header, sizeof(PVRTCTexHeader));

        // Get the file type identifier
        pvrTag = header.pvrTag;

        if (PVR_MAGIC != pvrTag)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                    "This is not a PVR file!", "PVRTCCodec::decode");
        }

        // Get format flags
        flags = header.flags;
        flipEndian((void *)flags, sizeof(uint32));
        formatFlags = flags & PVR_TEXTURE_FLAG_TYPE_MASK;

        uint32 bitmaskAlpha = header.bitmaskAlpha;
        flipEndian((void *)bitmaskAlpha, sizeof(uint32));

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
            imgData->num_mipmaps = header.numMipmaps;

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
    String PVRTCCodec::getType() const 
    {
        return mType;
    }
    //---------------------------------------------------------------------    
    void PVRTCCodec::flipEndian(void * pData, size_t size, size_t count) const
    {
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
		for(unsigned int index = 0; index < count; index++)
        {
            flipEndian((void *)((long)pData + (index * size)), size);
        }
#endif
    }
    //---------------------------------------------------------------------    
    void PVRTCCodec::flipEndian(void * pData, size_t size) const
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
	String PVRTCCodec::magicNumberToFileExt(const char *magicNumberPtr, size_t maxbytes) const
	{
		if (maxbytes >= sizeof(uint32))
		{
			uint32 fileType;
			memcpy(&fileType, magicNumberPtr, sizeof(uint32));
			flipEndian(&fileType, sizeof(uint32), 1);

			if (PVR_MAGIC == fileType)
			{
				return String("pvr");
			}
		}

		return StringUtil::BLANK;
	}
}
