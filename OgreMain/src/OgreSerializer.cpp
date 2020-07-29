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

namespace Ogre {

    const uint16 HEADER_STREAM_ID = 0x1000;
    const uint16 OTHER_ENDIAN_HEADER_STREAM_ID = 0x0010;
    //---------------------------------------------------------------------
    Serializer::Serializer() :
        mVersion("[Serializer_v1.00]"), // Version number
        mFlipEndian(false)
#if OGRE_SERIALIZER_VALIDATE_CHUNKSIZE
        , mReportChunkErrors(true)
#endif
    {
    }

    //---------------------------------------------------------------------
    Serializer::~Serializer()
    {
    }
    //---------------------------------------------------------------------
    void Serializer::determineEndianness(const DataStreamPtr& stream)
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
#if OGRE_SERIALIZER_VALIDATE_CHUNKSIZE
        if (!mChunkSizeStack.empty()){
            size_t pos = mStream->tell();
            if (pos != static_cast<size_t>(mChunkSizeStack.back()) && mReportChunkErrors){
                LogManager::getSingleton().logMessage("Corrupted chunk detected! Stream name: '" + mStream->getName()
                    + "' Chunk id: " + StringConverter::toString(id));
            }
            mChunkSizeStack.back() = pos + size;
        }
#endif
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
            uint32 * pIntToWrite = (uint32 *)malloc(sizeof(uint32) * count);
            memcpy(pIntToWrite, pInt, sizeof(uint32) * count);
            
            flipToLittleEndian(pIntToWrite, sizeof(uint32), count);
            writeData(pIntToWrite, sizeof(uint32), count);
            
            free(pIntToWrite);
        }
        else
        {
            writeData(pInt, sizeof(uint32), count);
        }
    }
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void Serializer::writeBools(const bool* const pBool, size_t count = 1)
    {
        //no endian flipping for 1-byte bools
        static_assert(sizeof(bool) == 1, "add conversion to char for your platform");
        writeData(pBool, sizeof(bool), count);
    }
    
    //---------------------------------------------------------------------
    void Serializer::writeData(const void* const buf, size_t size, size_t count)
    {
        mStream->write(buf, size * count);
    }
    //---------------------------------------------------------------------
    void Serializer::writeString(const String& string)
    {
        // Old, backwards compatible way - \n terminated
        mStream->write(string.c_str(), string.length());
        // Write terminating newline char
        char terminator = '\n';
        mStream->write(&terminator, 1);
    }
    //---------------------------------------------------------------------
    void Serializer::readFileHeader(const DataStreamPtr& stream)
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
    unsigned short Serializer::readChunk(const DataStreamPtr& stream)
    {
#if OGRE_SERIALIZER_VALIDATE_CHUNKSIZE
        size_t pos = stream->tell();
#endif
        unsigned short id;
        readShorts(stream, &id, 1);
        
        readInts(stream, &mCurrentstreamLen, 1);
#if OGRE_SERIALIZER_VALIDATE_CHUNKSIZE
        if (!mChunkSizeStack.empty() && !stream->eof()){
            if (pos != static_cast<size_t>(mChunkSizeStack.back()) && mReportChunkErrors){
                LogManager::getSingleton().logMessage("Corrupted chunk detected! Stream name: '" + stream->getName() + "' Chunk id: " + StringConverter::toString(id));
            }
            mChunkSizeStack.back() = pos + mCurrentstreamLen;
        }
#endif
        return id;
    }
    //---------------------------------------------------------------------
    void Serializer::readBools(const DataStreamPtr& stream, bool* pDest, size_t count)
    {
        static_assert(sizeof(bool) == 1, "add conversion to char for your platform");
        stream->read(pDest, sizeof(bool) * count);
    }
    //---------------------------------------------------------------------
    void Serializer::readFloats(const DataStreamPtr& stream, float* pDest, size_t count)
    {
        stream->read(pDest, sizeof(float) * count);
        flipFromLittleEndian(pDest, sizeof(float), count);
    }
    //---------------------------------------------------------------------
    void Serializer::readFloats(const DataStreamPtr& stream, double* pDest, size_t count)
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
    void Serializer::readShorts(const DataStreamPtr& stream, unsigned short* pDest, size_t count)
    {
        stream->read(pDest, sizeof(unsigned short) * count);
        flipFromLittleEndian(pDest, sizeof(unsigned short), count);
    }
    //---------------------------------------------------------------------
    void Serializer::readInts(const DataStreamPtr& stream, uint32* pDest, size_t count)
    {
        stream->read(pDest, sizeof(uint32) * count);
        flipFromLittleEndian(pDest, sizeof(uint32), count);
    }
    //---------------------------------------------------------------------
    String Serializer::readString(const DataStreamPtr& stream, size_t numChars)
    {
        OgreAssert(numChars <= 255, "");
        char str[255];
        stream->read(str, numChars);
        str[numChars] = '\0';
        return str;
    }
    //---------------------------------------------------------------------
    String Serializer::readString(const DataStreamPtr& stream)
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
        float tmp[4] = {
            static_cast<float>(q.x),
            static_cast<float>(q.y),
            static_cast<float>(q.z),
            static_cast<float>(q.w)
        };
        writeFloats(tmp, 4);
    }
    //---------------------------------------------------------------------
    void Serializer::readObject(const DataStreamPtr& stream, Vector3& pDest)
    {
        readFloats(stream, pDest.ptr(), 3);
    }
    //---------------------------------------------------------------------
    void Serializer::readObject(const DataStreamPtr& stream, Quaternion& pDest)
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
			Bitwise::bswapChunks(pData, size, count);
        }
    }
    
    void Serializer::flipFromLittleEndian(void* pData, size_t size, size_t count)
    {
        if(mFlipEndian)
        {
	        Bitwise::bswapChunks(pData, size, count);
        }
    }
    
    size_t Serializer::calcChunkHeaderSize()
    {
        return sizeof(uint16) + sizeof(uint32);
    }

    size_t Serializer::calcStringSize( const String& string )
    {
        // string + terminating \n character
        return string.length() + 1;
    }

    void Serializer::pushInnerChunk(const DataStreamPtr& stream)
    {
#if OGRE_SERIALIZER_VALIDATE_CHUNKSIZE
        mChunkSizeStack.push_back(stream->tell());
#endif
    }
    void Serializer::backpedalChunkHeader(const DataStreamPtr& stream)
    {
        if (!stream->eof()){
            stream->skip(-(int)calcChunkHeaderSize());
        }
#if OGRE_SERIALIZER_VALIDATE_CHUNKSIZE
        mChunkSizeStack.back() = stream->tell();
#endif
    }
    void Serializer::popInnerChunk(const DataStreamPtr& stream)
    {
#if OGRE_SERIALIZER_VALIDATE_CHUNKSIZE
        if (!mChunkSizeStack.empty()){
            size_t pos = stream->tell();
            if (pos != static_cast<size_t>(mChunkSizeStack.back()) && !stream->eof() && mReportChunkErrors){
                LogManager::getSingleton().logMessage("Corrupted chunk detected! Stream name: " + stream->getName());
            }

            mChunkSizeStack.pop_back();
        }
#endif
    }

}

