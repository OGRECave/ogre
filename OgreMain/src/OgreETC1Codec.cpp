/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2012 Torus Knot Software Ltd

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
#include "OgreETC1Codec.h"
#include "OgreImage.h"
#include "OgreException.h"

#include "OgreLogManager.h"
#include "OgreStringConverter.h"

#define FOURCC(c0, c1, c2, c3) (c0 | (c1 << 8) | (c2 << 16) | (c3 << 24))

namespace Ogre {

    const uint32 ETC1_MAGIC = FOURCC('P', 'K', 'M', ' ');

    typedef struct {
        int8   aName[6];
        uint16 iBlank;
        uint8  iPaddedWidthMSB;
        uint8  iPaddedWidthLSB;
        uint8  iPaddedHeightMSB;
        uint8  iPaddedHeightLSB;
        uint8  iWidthMSB;
        uint8  iWidthLSB;
        uint8  iHeightMSB;
        uint8  iHeightLSB;
    } ETCHeader;
	

	//---------------------------------------------------------------------
	ETC1Codec* ETC1Codec::msInstance = 0;
	//---------------------------------------------------------------------
	void ETC1Codec::startup(void)
	{
		if (!msInstance)
		{
			LogManager::getSingleton().logMessage(
				LML_NORMAL,
				"ETC1 codec registering");

			msInstance = OGRE_NEW ETC1Codec();
			Codec::registerCodec(msInstance);
		}
	}
	//---------------------------------------------------------------------
	void ETC1Codec::shutdown(void)
	{
		if(msInstance)
		{
			Codec::unRegisterCodec(msInstance);
			OGRE_DELETE msInstance;
			msInstance = 0;
		}
	}
	//---------------------------------------------------------------------
    ETC1Codec::ETC1Codec():
        mType("pkm")
    { 
    }
    //---------------------------------------------------------------------
    DataStreamPtr ETC1Codec::code(MemoryDataStreamPtr& input, Codec::CodecDataPtr& pData) const
    {        
		OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
                    "ETC1 encoding not supported",
                    "ETC1Codec::code" ) ;
    }
    //---------------------------------------------------------------------
    void ETC1Codec::codeToFile(MemoryDataStreamPtr& input, 
        const String& outFileName, Codec::CodecDataPtr& pData) const
    {
		OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
                    "ETC1 encoding not supported",
                    "ETC1Codec::codeToFile" ) ;
	}
    //---------------------------------------------------------------------
    Codec::DecodeResult ETC1Codec::decode(DataStreamPtr& stream) const
    {
        ETCHeader header;

        ImageData *imgData = OGRE_NEW ImageData();
		MemoryDataStreamPtr output;

        // Read the ETC1 header
        stream->read(&header, sizeof(ETCHeader));

        if (ETC1_MAGIC != FOURCC(header.aName[0], header.aName[1], header.aName[2], header.aName[3]) ) // "PKM 10"
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                    "This is not a ETC1 file!", "ETC1Codec::decode");
        }
        
        // TODO add endian awareness
        uint16 width = (header.iWidthMSB << 8) | header.iWidthLSB;
        uint16 height = (header.iHeightMSB << 8) | header.iHeightLSB;
        uint16 paddedWidth = (header.iPaddedWidthMSB << 8) | header.iPaddedWidthLSB;
        uint16 paddedHeight = (header.iPaddedHeightMSB << 8) | header.iPaddedHeightLSB;
    
        imgData->depth = 1;
        imgData->width = width;
        imgData->height = height;
        imgData->format = PF_ETC1_RGB8;
        
        // ETC1 has no support for mipmaps - malideveloper.com has a example 
        // where the load mipmap levels from different external files
        imgData->num_mipmaps = 0; 
        
        // ETC1 is a compressed format
        imgData->flags |= IF_COMPRESSED;
        

        // Calculate total size from number of mipmaps, faces and size
		imgData->size = (paddedWidth * paddedHeight) >> 1;

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
    String ETC1Codec::getType() const 
    {
        return mType;
    }
    //---------------------------------------------------------------------    
    void ETC1Codec::flipEndian(void * pData, size_t size, size_t count) const
    {
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
		for(unsigned int index = 0; index < count; index++)
        {
            flipEndian((void *)((long)pData + (index * size)), size);
        }
#endif
    }
    //---------------------------------------------------------------------    
    void ETC1Codec::flipEndian(void * pData, size_t size) const
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
	String ETC1Codec::magicNumberToFileExt(const char *magicNumberPtr, size_t maxbytes) const
	{
		if (maxbytes >= sizeof(uint32))
		{
			uint32 fileType;
			memcpy(&fileType, magicNumberPtr, sizeof(uint32));
			flipEndian(&fileType, sizeof(uint32), 1);

			if (ETC1_MAGIC == fileType)
			{
				return String("pkm");
			}
		}

		return StringUtil::BLANK;
	}
}
