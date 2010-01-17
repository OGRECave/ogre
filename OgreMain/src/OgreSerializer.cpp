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

#include "OgreSerializer.h"
#include "OgreLogManager.h"
#include "OgreDataStream.h"
#include "OgreException.h"
#include "OgreVector3.h"
#include "OgreQuaternion.h"


namespace Ogre {

    /// stream overhead = ID + size
    const size_t STREAM_OVERHEAD_SIZE = sizeof(uint16) + sizeof(uint32);
    const uint16 HEADER_STREAM_ID = 0x1000;
    const uint16 OTHER_ENDIAN_HEADER_STREAM_ID = 0x0010;
    //---------------------------------------------------------------------
    Serializer::Serializer()
    {
        // Version number
        mVersion = "[Serializer_v1.00]";
		mFlipEndian = false;
    }
    //---------------------------------------------------------------------
    Serializer::~Serializer()
    {
    }
    //---------------------------------------------------------------------
	void Serializer::determineEndianness(DataStreamPtr& stream)
	{
		if (stream->tell() != 0)
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
				"Can only determine the endianness of the input stream if it "
				"is at the start", "Serializer::determineEndianness");
		}
				
		uint16 dest;
		// read header id manually (no conversion)
        size_t actually_read = stream->read(&dest, sizeof(uint16));
		// skip back
        stream->skip(0 - (long)actually_read);
        if (actually_read != sizeof(uint16))
        {
            // end of file?
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Couldn't read 16 bit header value from input stream.",
                        "Serializer::determineEndianness");
        }
		if (dest == HEADER_STREAM_ID)
		{
			mFlipEndian = false;
		}
		else if (dest == OTHER_ENDIAN_HEADER_STREAM_ID)
		{
			mFlipEndian = true;
		}
		else
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
				"Header chunk didn't match either endian: Corrupted stream?",
				"Serializer::determineEndianness");
		}
	}
    //---------------------------------------------------------------------
	void Serializer::determineEndianness(Endian requestedEndian)
	{
		switch(requestedEndian)
		{
		case ENDIAN_NATIVE:
			mFlipEndian = false;
			break;
		case ENDIAN_BIG:
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
			mFlipEndian = false;
#else
			mFlipEndian = true;
#endif
			break;
		case ENDIAN_LITTLE:
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
			mFlipEndian = true;
#else
			mFlipEndian = false;
#endif
			break;
		}
	}
    //---------------------------------------------------------------------
    void Serializer::writeFileHeader(void)
    {
        
        uint16 val = HEADER_STREAM_ID;
        writeShorts(&val, 1);

        writeString(mVersion);

    }
    //---------------------------------------------------------------------
    void Serializer::writeChunkHeader(uint16 id, size_t size)
    {
        writeShorts(&id, 1);
		uint32 uint32size = static_cast<uint32>(size);
        writeInts(&uint32size, 1);
    }
    //---------------------------------------------------------------------
    void Serializer::writeFloats(const float* const pFloat, size_t count)
    {
		if (mFlipEndian)
		{
            float * pFloatToWrite = (float *)malloc(sizeof(float) * count);
            memcpy(pFloatToWrite, pFloat, sizeof(float) * count);
            
            flipToLittleEndian(pFloatToWrite, sizeof(float), count);
            writeData(pFloatToWrite, sizeof(float), count);
            
            free(pFloatToWrite);
		}
		else
		{
            writeData(pFloat, sizeof(float), count);
		}
    }
    //---------------------------------------------------------------------
    void Serializer::writeFloats(const double* const pDouble, size_t count)
    {
		// Convert to float, then write
		float* tmp = OGRE_ALLOC_T(float, count, MEMCATEGORY_GENERAL);
		for (unsigned int i = 0; i < count; ++i)
		{
			tmp[i] = static_cast<float>(pDouble[i]);
		}
		if(mFlipEndian)
		{
            flipToLittleEndian(tmp, sizeof(float), count);
            writeData(tmp, sizeof(float), count);
		}
		else
		{
            writeData(tmp, sizeof(float), count);
		}
		OGRE_FREE(tmp, MEMCATEGORY_GENERAL);
    }
    //---------------------------------------------------------------------
    void Serializer::writeShorts(const uint16* const pShort, size_t count = 1)
    {
		if(mFlipEndian)
		{
            unsigned short * pShortToWrite = (unsigned short *)malloc(sizeof(unsigned short) * count);
            memcpy(pShortToWrite, pShort, sizeof(unsigned short) * count);
            
            flipToLittleEndian(pShortToWrite, sizeof(unsigned short), count);
            writeData(pShortToWrite, sizeof(unsigned short), count);
            
            free(pShortToWrite);
		}
		else
		{
            writeData(pShort, sizeof(unsigned short), count);
		}
    }
    //---------------------------------------------------------------------
    void Serializer::writeInts(const uint32* const pInt, size_t count = 1)
    {
		if(mFlipEndian)
		{
            unsigned int * pIntToWrite = (unsigned int *)malloc(sizeof(unsigned int) * count);
            memcpy(pIntToWrite, pInt, sizeof(unsigned int) * count);
            
            flipToLittleEndian(pIntToWrite, sizeof(unsigned int), count);
            writeData(pIntToWrite, sizeof(unsigned int), count);
            
            free(pIntToWrite);
		}
		else
		{
            writeData(pInt, sizeof(unsigned int), count);
		}
    }
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void Serializer::writeBools(const bool* const pBool, size_t count = 1)
    {
    //no endian flipping for 1-byte bools
    //XXX Nasty Hack to convert to 1-byte bools
#	if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
        char * pCharToWrite = (char *)malloc(sizeof(char) * count);
        for(unsigned int i = 0; i < count; i++)
        {
            *(char *)(pCharToWrite + i) = *(bool *)(pBool + i);
        }
        
        writeData(pCharToWrite, sizeof(char), count);
        
        free(pCharToWrite);
#	else
        writeData(pBool, sizeof(bool), count);
#	endif

    }
    
    //---------------------------------------------------------------------
    void Serializer::writeData(const void* const buf, size_t size, size_t count)
    {
        fwrite((void* const)buf, size, count, mpfFile);
    }
    //---------------------------------------------------------------------
    void Serializer::writeString(const String& string)
    {
        fputs(string.c_str(), mpfFile);
        // Write terminating newline char
        fputc('\n', mpfFile);
    }
    //---------------------------------------------------------------------
    void Serializer::readFileHeader(DataStreamPtr& stream)
    {
        unsigned short headerID;
        
        // Read header ID
        readShorts(stream, &headerID, 1);
        
        if (headerID == HEADER_STREAM_ID)
        {
            // Read version
            String ver = readString(stream);
            if (ver != mVersion)
            {
                OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
                    "Invalid file: version incompatible, file reports " + String(ver) +
                    " Serializer is version " + mVersion,
                    "Serializer::readFileHeader");
            }
        }
        else
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Invalid file: no header", 
                "Serializer::readFileHeader");
        }

    }
    //---------------------------------------------------------------------
    unsigned short Serializer::readChunk(DataStreamPtr& stream)
    {
        unsigned short id;
        readShorts(stream, &id, 1);
        
        readInts(stream, &mCurrentstreamLen, 1);
        return id;
    }
    //---------------------------------------------------------------------
    void Serializer::readBools(DataStreamPtr& stream, bool* pDest, size_t count)
    {
        //XXX Nasty Hack to convert 1 byte bools to 4 byte bools
#	if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
        char * pTemp = (char *)malloc(1*count); // to hold 1-byte bools
        stream->read(pTemp, 1 * count);
        for(unsigned int i = 0; i < count; i++)
            *(bool *)(pDest + i) = *(char *)(pTemp + i);
            
        free (pTemp);
#	else
        stream->read(pDest, sizeof(bool) * count);
#	endif
        //no flipping on 1-byte datatypes
    }
    //---------------------------------------------------------------------
    void Serializer::readFloats(DataStreamPtr& stream, float* pDest, size_t count)
    {
        stream->read(pDest, sizeof(float) * count);
        flipFromLittleEndian(pDest, sizeof(float), count);
    }
    //---------------------------------------------------------------------
    void Serializer::readFloats(DataStreamPtr& stream, double* pDest, size_t count)
    {
		// Read from float, convert to double
		float* tmp = OGRE_ALLOC_T(float, count, MEMCATEGORY_GENERAL);
		float* ptmp = tmp;
        stream->read(tmp, sizeof(float) * count);
        flipFromLittleEndian(tmp, sizeof(float), count);
		// Convert to doubles (no cast required)
		while(count--)
		{
			*pDest++ = *ptmp++;
		}
		OGRE_FREE(tmp, MEMCATEGORY_GENERAL);
    }
    //---------------------------------------------------------------------
    void Serializer::readShorts(DataStreamPtr& stream, unsigned short* pDest, size_t count)
    {
        stream->read(pDest, sizeof(unsigned short) * count);
        flipFromLittleEndian(pDest, sizeof(unsigned short), count);
    }
    //---------------------------------------------------------------------
    void Serializer::readInts(DataStreamPtr& stream, unsigned int* pDest, size_t count)
    {
        stream->read(pDest, sizeof(unsigned int) * count);
        flipFromLittleEndian(pDest, sizeof(unsigned int), count);
    }
    //---------------------------------------------------------------------
    String Serializer::readString(DataStreamPtr& stream, size_t numChars)
    {
        assert (numChars <= 255);
        char str[255];
        stream->read(str, numChars);
        str[numChars] = '\0';
        return str;
    }
    //---------------------------------------------------------------------
    String Serializer::readString(DataStreamPtr& stream)
    {
        return stream->getLine(false);
    }
    //---------------------------------------------------------------------
    void Serializer::writeObject(const Vector3& vec)
    {
        writeFloats(vec.ptr(), 3);
    }
    //---------------------------------------------------------------------
    void Serializer::writeObject(const Quaternion& q)
    {
        float tmp[4] = { q.x, q.y, q.z, q.w };
        writeFloats(tmp, 4);
    }
    //---------------------------------------------------------------------
    void Serializer::readObject(DataStreamPtr& stream, Vector3& pDest)
    {
        readFloats(stream, pDest.ptr(), 3);
    }
    //---------------------------------------------------------------------
    void Serializer::readObject(DataStreamPtr& stream, Quaternion& pDest)
    {
        float tmp[4];
        readFloats(stream, tmp, 4);
        pDest.x = tmp[0];
        pDest.y = tmp[1];
        pDest.z = tmp[2];
        pDest.w = tmp[3];
    }
    //---------------------------------------------------------------------


    void Serializer::flipToLittleEndian(void* pData, size_t size, size_t count)
    {
		if(mFlipEndian)
		{
	        flipEndian(pData, size, count);
		}
    }
    
    void Serializer::flipFromLittleEndian(void* pData, size_t size, size_t count)
    {
		if(mFlipEndian)
		{
	        flipEndian(pData, size, count);
		}
    }
    
    void Serializer::flipEndian(void * pData, size_t size, size_t count)
    {
        for(unsigned int index = 0; index < count; index++)
        {
            flipEndian((void *)((size_t)pData + (index * size)), size);
        }
    }
    
    void Serializer::flipEndian(void * pData, size_t size)
    {
        char swapByte;
        for(unsigned int byteIndex = 0; byteIndex < size/2; byteIndex++)
        {
            swapByte = *(char *)((size_t)pData + byteIndex);
            *(char *)((size_t)pData + byteIndex) = *(char *)((size_t)pData + size - byteIndex - 1);
            *(char *)((size_t)pData + size - byteIndex - 1) = swapByte;
        }
    }
    
}

