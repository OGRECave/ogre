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
#include "OgreDataStream.h"
#include "OgreLogManager.h"
#include "OgreException.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    template <typename T> DataStream& DataStream::operator >>(T& val)
    {
        read(static_cast<void*>(&val), sizeof(T));
        return *this;
    }
    //-----------------------------------------------------------------------
    String DataStream::getLine(bool trimAfter)
    {
        char tmpBuf[OGRE_STREAM_TEMP_SIZE];
        String retString;
        size_t readCount;
        // Keep looping while not hitting delimiter
        while ((readCount = read(tmpBuf, OGRE_STREAM_TEMP_SIZE-1)) != 0)
        {
            // Terminate string
            tmpBuf[readCount] = '\0';

            char* p = strchr(tmpBuf, '\n');
            if (p != 0)
            {
                // Reposition backwards
                skip((long)(p + 1 - tmpBuf - readCount));
                *p = '\0';
            }

            retString += tmpBuf;

            if (p != 0)
            {
                // Trim off trailing CR if this was a CR/LF entry
                if (retString.length() && retString[retString.length()-1] == '\r')
                {
                    retString.erase(retString.length()-1, 1);
                }

                // Found terminator, break out
                break;
            }
        }

        if (trimAfter)
        {
            StringUtil::trim(retString);
        }

        return retString;
    }
    //-----------------------------------------------------------------------
    size_t DataStream::readLine(char* buf, size_t maxCount, const String& delim)
    {
		// Deal with both Unix & Windows LFs
		bool trimCR = false;
		if (delim.find_first_of('\n') != String::npos)
		{
			trimCR = true;
		}

        char tmpBuf[OGRE_STREAM_TEMP_SIZE];
        size_t chunkSize = std::min(maxCount, (size_t)OGRE_STREAM_TEMP_SIZE-1);
        size_t totalCount = 0;
        size_t readCount; 
        while (chunkSize && (readCount = read(tmpBuf, chunkSize)))
        {
            // Terminate
            tmpBuf[readCount] = '\0';

            // Find first delimiter
            size_t pos = strcspn(tmpBuf, delim.c_str());

            if (pos < readCount)
            {
                // Found terminator, reposition backwards
                skip((long)(pos + 1 - readCount));
            }

            // Are we genuinely copying?
            if (buf)
            {
                memcpy(buf+totalCount, tmpBuf, pos);
            }
            totalCount += pos;

            if (pos < readCount)
            {
                // Trim off trailing CR if this was a CR/LF entry
                if (trimCR && totalCount && buf[totalCount-1] == '\r')
                {
                    --totalCount;
                }

                // Found terminator, break out
                break;
            }

            // Adjust chunkSize for next time
            chunkSize = std::min(maxCount-totalCount, (size_t)OGRE_STREAM_TEMP_SIZE-1);
        }

        // Terminate
        buf[totalCount] = '\0';

        return totalCount;
    }
    //-----------------------------------------------------------------------
    size_t DataStream::skipLine(const String& delim)
    {
        char tmpBuf[OGRE_STREAM_TEMP_SIZE];
        size_t total = 0;
        size_t readCount;
        // Keep looping while not hitting delimiter
        while ((readCount = read(tmpBuf, OGRE_STREAM_TEMP_SIZE-1)) != 0)
        {
            // Terminate string
            tmpBuf[readCount] = '\0';

            // Find first delimiter
            size_t pos = strcspn(tmpBuf, delim.c_str());

            if (pos < readCount)
            {
                // Found terminator, reposition backwards
                skip((long)(pos + 1 - readCount));

                total += pos + 1;

                // break out
                break;
            }

            total += readCount;
        }

        return total;
    }
    //-----------------------------------------------------------------------
    String DataStream::getAsString(void)
    {
        // Read the entire buffer
        char* pBuf = OGRE_ALLOC_T(char, mSize+1, MEMCATEGORY_GENERAL);
        // Ensure read from begin of stream
        seek(0);
        read(pBuf, mSize);
        pBuf[mSize] = '\0';
        String str;
        str.insert(0, pBuf, mSize);
        OGRE_FREE(pBuf, MEMCATEGORY_GENERAL);
        return str;
    }
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    MemoryDataStream::MemoryDataStream(void* pMem, size_t size, bool freeOnClose)
        : DataStream()
    {
        mData = mPos = static_cast<uchar*>(pMem);
        mSize = size;
        mEnd = mData + mSize;
        mFreeOnClose = freeOnClose;
        assert(mEnd >= mPos);
    }
    //-----------------------------------------------------------------------
    MemoryDataStream::MemoryDataStream(const String& name, void* pMem, size_t size, 
        bool freeOnClose)
        : DataStream(name)
    {
        mData = mPos = static_cast<uchar*>(pMem);
        mSize = size;
        mEnd = mData + mSize;
        mFreeOnClose = freeOnClose;
        assert(mEnd >= mPos);
    }
    //-----------------------------------------------------------------------
    MemoryDataStream::MemoryDataStream(DataStream& sourceStream, 
        bool freeOnClose)
        : DataStream()
    {
        // Copy data from incoming stream
        mSize = sourceStream.size();
        mData = OGRE_ALLOC_T(uchar, mSize, MEMCATEGORY_GENERAL);
        mPos = mData;
        mEnd = mData + sourceStream.read(mData, mSize);
        mFreeOnClose = freeOnClose;
        assert(mEnd >= mPos);
    }
    //-----------------------------------------------------------------------
    MemoryDataStream::MemoryDataStream(DataStreamPtr& sourceStream, 
        bool freeOnClose)
        : DataStream()
    {
        // Copy data from incoming stream
        mSize = sourceStream->size();
        mData = OGRE_ALLOC_T(uchar, mSize, MEMCATEGORY_GENERAL);
        mPos = mData;
        mEnd = mData + sourceStream->read(mData, mSize);
        mFreeOnClose = freeOnClose;
        assert(mEnd >= mPos);
    }
    //-----------------------------------------------------------------------
    MemoryDataStream::MemoryDataStream(const String& name, DataStream& sourceStream, 
        bool freeOnClose)
        : DataStream(name)
    {
        // Copy data from incoming stream
        mSize = sourceStream.size();
        mData = OGRE_ALLOC_T(uchar, mSize, MEMCATEGORY_GENERAL);
        mPos = mData;
        mEnd = mData + sourceStream.read(mData, mSize);
        mFreeOnClose = freeOnClose;
        assert(mEnd >= mPos);
    }
    //-----------------------------------------------------------------------
    MemoryDataStream::MemoryDataStream(const String& name, const DataStreamPtr& sourceStream, 
        bool freeOnClose)
        : DataStream(name)
    {
        // Copy data from incoming stream
        mSize = sourceStream->size();
        mData = OGRE_ALLOC_T(uchar, mSize, MEMCATEGORY_GENERAL);
        mPos = mData;
        mEnd = mData + sourceStream->read(mData, mSize);
        mFreeOnClose = freeOnClose;
        assert(mEnd >= mPos);
    }
    //-----------------------------------------------------------------------
    MemoryDataStream::MemoryDataStream(size_t size, bool freeOnClose)
        : DataStream()
    {
        mSize = size;
        mFreeOnClose = freeOnClose;
        mData = OGRE_ALLOC_T(uchar, mSize, MEMCATEGORY_GENERAL);
        mPos = mData;
        mEnd = mData + mSize;
        assert(mEnd >= mPos);
    }
    //-----------------------------------------------------------------------
    MemoryDataStream::MemoryDataStream(const String& name, size_t size, 
        bool freeOnClose)
        : DataStream(name)
    {
        mSize = size;
        mFreeOnClose = freeOnClose;
        mData = OGRE_ALLOC_T(uchar, mSize, MEMCATEGORY_GENERAL);
        mPos = mData;
        mEnd = mData + mSize;
        assert(mEnd >= mPos);
    }
    //-----------------------------------------------------------------------
    MemoryDataStream::~MemoryDataStream()
    {
        close();
    }
    //-----------------------------------------------------------------------
    size_t MemoryDataStream::read(void* buf, size_t count)
    {
        size_t cnt = count;
        // Read over end of memory?
        if (mPos + cnt > mEnd)
            cnt = mEnd - mPos;
        if (cnt == 0)
            return 0;

        assert (cnt<=count);

        memcpy(buf, mPos, cnt);
        mPos += cnt;
        return cnt;
    }
    //-----------------------------------------------------------------------
    size_t MemoryDataStream::readLine(char* buf, size_t maxCount, 
        const String& delim)
    {
        // Deal with both Unix & Windows LFs
		bool trimCR = false;
		if (delim.find_first_of('\n') != String::npos)
		{
			trimCR = true;
		}

        size_t pos = 0;

        // Make sure pos can never go past the end of the data 
        while (pos < maxCount && mPos < mEnd)
        {
            if (delim.find(*mPos) != String::npos)
            {
                // Trim off trailing CR if this was a CR/LF entry
                if (trimCR && pos && buf[pos-1] == '\r')
                {
                    // terminate 1 character early
                    --pos;
                }

                // Found terminator, skip and break out
                ++mPos;
                break;
            }

            buf[pos++] = *mPos++;
        }

        // terminate
        buf[pos] = '\0';

        return pos;
    }
    //-----------------------------------------------------------------------
    size_t MemoryDataStream::skipLine(const String& delim)
    {
        size_t pos = 0;

        // Make sure pos can never go past the end of the data 
        while (mPos < mEnd)
        {
            ++pos;
            if (delim.find(*mPos++) != String::npos)
            {
                // Found terminator, break out
                break;
            }
        }

        return pos;

    }
    //-----------------------------------------------------------------------
    void MemoryDataStream::skip(long count)
    {
        size_t newpos = (size_t)( ( mPos - mData ) + count );
        assert( mData + newpos <= mEnd );        

        mPos = mData + newpos;
    }
    //-----------------------------------------------------------------------
    void MemoryDataStream::seek( size_t pos )
    {
        assert( mData + pos <= mEnd );
        mPos = mData + pos;
    }
    //-----------------------------------------------------------------------
    size_t MemoryDataStream::tell(void) const
	{
		//mData is start, mPos is current location
		return mPos - mData;
	}
	//-----------------------------------------------------------------------
    bool MemoryDataStream::eof(void) const
    {
        return mPos >= mEnd;
    }
    //-----------------------------------------------------------------------
    void MemoryDataStream::close(void)    
    {
        if (mFreeOnClose && mData)
        {
            OGRE_FREE(mData, MEMCATEGORY_GENERAL);
            mData = 0;
        }

    }
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    FileStreamDataStream::FileStreamDataStream(std::ifstream* s, bool freeOnClose)
        : DataStream(), mpStream(s), mFreeOnClose(freeOnClose)
    {
        // calculate the size
        mpStream->seekg(0, std::ios_base::end);
        mSize = mpStream->tellg();
        mpStream->seekg(0, std::ios_base::beg);

    }
    //-----------------------------------------------------------------------
    FileStreamDataStream::FileStreamDataStream(const String& name, 
        std::ifstream* s, bool freeOnClose)
        : DataStream(name), mpStream(s), mFreeOnClose(freeOnClose)
    {
        // calculate the size
        mpStream->seekg(0, std::ios_base::end);
        mSize = mpStream->tellg();
        mpStream->seekg(0, std::ios_base::beg);
    }
    //-----------------------------------------------------------------------
    FileStreamDataStream::FileStreamDataStream(const String& name, 
        std::ifstream* s, size_t size, bool freeOnClose)
        : DataStream(name), mpStream(s), mFreeOnClose(freeOnClose)
    {
        // Size is passed in
        mSize = size;
    }
    //-----------------------------------------------------------------------
    FileStreamDataStream::~FileStreamDataStream()
    {
        close();
    }
    //-----------------------------------------------------------------------
    size_t FileStreamDataStream::read(void* buf, size_t count)
    {
		mpStream->read(static_cast<char*>(buf), static_cast<std::streamsize>(count));
        return mpStream->gcount();
    }
    //-----------------------------------------------------------------------
    size_t FileStreamDataStream::readLine(char* buf, size_t maxCount, 
        const String& delim)
    {
		if (delim.empty())
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "No delimiter provided",
				"FileStreamDataStream::readLine");
		}
		if (delim.size() > 1)
		{
			LogManager::getSingleton().logMessage(
				"WARNING: FileStreamDataStream::readLine - using only first delimeter");
		}
		// Deal with both Unix & Windows LFs
		bool trimCR = false;
		if (delim.at(0) == '\n') 
		{
			trimCR = true;
		}
		// maxCount + 1 since count excludes terminator in getline
		mpStream->getline(buf, static_cast<std::streamsize>(maxCount+1), delim.at(0));
		size_t ret = mpStream->gcount();
		// three options
		// 1) we had an eof before we read a whole line
		// 2) we ran out of buffer space
		// 3) we read a whole line - in this case the delim character is taken from the stream but not written in the buffer so the read data is of length ret-1 and thus ends at index ret-2
		// in all cases the buffer will be null terminated for us

		if (mpStream->eof()) 
		{
			// no problem
		}
		else if (mpStream->fail())
		{
			// Did we fail because of maxCount hit? No - no terminating character
			// in included in the count in this case
			if (ret == maxCount)
			{
				// clear failbit for next time 
				mpStream->clear();
			}
			else
			{
				OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
					"Streaming error occurred", 
					"FileStreamDataStream::readLine");
			}
		}
		else 
		{
			// we need to adjust ret because we want to use it as a
			// pointer to the terminating null character and it is
			// currently the length of the data read from the stream
			// i.e. 1 more than the length of the data in the buffer and
			// hence 1 more than the _index_ of the NULL character
			--ret;
		}

		// trim off CR if we found CR/LF
		if (trimCR && buf[ret-1] == '\r')
		{
			--ret;
			buf[ret] = '\0';
		}
		return ret;
	}
    //-----------------------------------------------------------------------
    void FileStreamDataStream::skip(long count)
    {
#if defined(STLPORT)
		// Workaround for STLport issues: After reached eof of file stream,
		// it's seems the stream was putted in intermediate state, and will be
		// fail if try to repositioning relative to current position.
		// Note: tellg() fail in this case too.
		if (mpStream->eof())
		{
			mpStream->clear();
			// Use seek relative to either begin or end to bring the stream
			// back to normal state.
			mpStream->seekg(0, std::ios::end);
		}
#endif 		
		mpStream->clear(); //Clear fail status in case eof was set
		mpStream->seekg(static_cast<std::ifstream::pos_type>(count), std::ios::cur);
    }
    //-----------------------------------------------------------------------
    void FileStreamDataStream::seek( size_t pos )
    {
		mpStream->clear(); //Clear fail status in case eof was set
        mpStream->seekg(static_cast<std::streamoff>(pos), std::ios::beg);
    }
	//-----------------------------------------------------------------------
    size_t FileStreamDataStream::tell(void) const
	{
		mpStream->clear(); //Clear fail status in case eof was set
		return mpStream->tellg();
	}
	//-----------------------------------------------------------------------
    bool FileStreamDataStream::eof(void) const
    {
        return mpStream->eof();
    }
    //-----------------------------------------------------------------------
    void FileStreamDataStream::close(void)
    {
        if (mpStream)
        {
            mpStream->close();
            if (mFreeOnClose)
            {
                // delete the stream too
                OGRE_DELETE_T(mpStream, basic_ifstream, MEMCATEGORY_GENERAL);
                mpStream = 0;
            }
        }
    }
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    FileHandleDataStream::FileHandleDataStream(FILE* handle)
        : DataStream(), mFileHandle(handle)
    {
		// Determine size
		fseek(mFileHandle, 0, SEEK_END);
		mSize = ftell(mFileHandle);
		fseek(mFileHandle, 0, SEEK_SET);
    }
    //-----------------------------------------------------------------------
    FileHandleDataStream::FileHandleDataStream(const String& name, FILE* handle)
        : DataStream(name), mFileHandle(handle)
    {
		// Determine size
		fseek(mFileHandle, 0, SEEK_END);
		mSize = ftell(mFileHandle);
		fseek(mFileHandle, 0, SEEK_SET);
    }
    //-----------------------------------------------------------------------
    FileHandleDataStream::~FileHandleDataStream()
    {
        close();
    }
    //-----------------------------------------------------------------------
    size_t FileHandleDataStream::read(void* buf, size_t count)
    {
        return fread(buf, 1, count, mFileHandle);
    }
    //-----------------------------------------------------------------------
    void FileHandleDataStream::skip(long count)
    {
        fseek(mFileHandle, count, SEEK_CUR);
    }
    //-----------------------------------------------------------------------
    void FileHandleDataStream::seek( size_t pos )
    {
        fseek(mFileHandle, static_cast<long>(pos), SEEK_SET);
    }
    //-----------------------------------------------------------------------
    size_t FileHandleDataStream::tell(void) const
	{
		return ftell( mFileHandle );
	}
	//-----------------------------------------------------------------------
    bool FileHandleDataStream::eof(void) const
    {
        return feof(mFileHandle) != 0;
    }
    //-----------------------------------------------------------------------
    void FileHandleDataStream::close(void)
    {
        fclose(mFileHandle);
        mFileHandle = 0;
    }
    //-----------------------------------------------------------------------

}
