/*
 -----------------------------------------------------------------------------
 This source file is part of OGRE
 (Object-oriented Graphics Rendering Engine)
 For the latest info, see http://www.ogre3d.org/
 
 Copyright (c) 2000-2011 Torus Knot Software Ltd
 
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
#include "OgreDeflate.h"
#include "OgreException.h"

#include <zlib.h>

namespace Ogre
{
	// memory implementations
	void* OgreZalloc(void* opaque, unsigned int items, unsigned int size)
	{
		return OGRE_MALLOC(items * size, MEMCATEGORY_GENERAL);
	}
	void OgreZfree(void* opaque, void* address)
	{
		OGRE_FREE(address, MEMCATEGORY_GENERAL);
	}
	#define OGRE_DEFLATE_TMP_SIZE 16384
    //---------------------------------------------------------------------
	DeflateStream::DeflateStream(const DataStreamPtr& compressedStream)
	: DataStream(compressedStream->getAccessMode())
	, mCompressedStream(compressedStream)
	, mpZStream(0)
	, mCurrentPos(0)
	, mpTmp(0)
	, mIsCompressedValid(true)
	{
		init();
	}
    //---------------------------------------------------------------------
	DeflateStream::DeflateStream(const String& name, const DataStreamPtr& compressedStream)		
	: DataStream(name, compressedStream->getAccessMode())
	, mCompressedStream(compressedStream)
	, mpZStream(0)
	, mCurrentPos(0)
	, mpTmp(0)
	, mIsCompressedValid(true)
	{
		init();
	}
    //---------------------------------------------------------------------
	void DeflateStream::init()
	{
		mpZStream = OGRE_ALLOC_T(z_stream, 1, MEMCATEGORY_GENERAL);
		mpZStream->zalloc = OgreZalloc;
		mpZStream->zfree = OgreZfree;
		
		if (getAccessMode() == READ)
		{
			mpTmp = (unsigned char*)OGRE_MALLOC(OGRE_DEFLATE_TMP_SIZE, MEMCATEGORY_GENERAL);
			size_t restorePoint = mCompressedStream->tell();
			// read early chunk
			mpZStream->next_in = mpTmp;
			mpZStream->avail_in = mCompressedStream->read(mpTmp, OGRE_DEFLATE_TMP_SIZE);
			
			if (inflateInit(mpZStream) != Z_OK)
			{
				mIsCompressedValid = false;
			}
			else
				mIsCompressedValid = true;
			
			if (mIsCompressedValid)
			{
				// in fact, inflateInit on some implementations doesn't try to read
				// anything. We need to at least read something to test
				Bytef testOut[4];
				size_t savedIn = mpZStream->avail_in;
				mpZStream->avail_out = 4;
				mpZStream->next_out = testOut;
				if (inflate(mpZStream, Z_SYNC_FLUSH) != Z_OK)
					mIsCompressedValid = false;
				// restore for reading
				mpZStream->avail_in = savedIn;
				mpZStream->next_in = mpTmp;

				inflateReset(mpZStream);
			}

			if (!mIsCompressedValid)
			{
				// Not compressed data!
				// Fail gracefully, fall back on reading the underlying stream direct
				destroy();
				mCompressedStream->seek(restorePoint);
			}				
		}
		else 
		{
			// Write to temp file
			char tmpname[L_tmpnam];
			tmpnam(tmpname);
			mTempFileName = tmpname;
			std::fstream *f = OGRE_NEW_T(std::fstream, MEMCATEGORY_GENERAL)();
			f->open(tmpname, std::ios::binary | std::ios::out);
			mTmpWriteStream = DataStreamPtr(OGRE_NEW FileStreamDataStream(f));
			
		}

	}
    //---------------------------------------------------------------------
	void DeflateStream::destroy()
	{
		if (getAccessMode() == READ)
			inflateEnd(mpZStream);

		OGRE_FREE(mpZStream, MEMCATEGORY_GENERAL);
		mpZStream = 0;
		OGRE_FREE(mpTmp, MEMCATEGORY_GENERAL);
		mpTmp = 0;
	}
	//---------------------------------------------------------------------
	DeflateStream::~DeflateStream()
	{
		close();
		destroy();
	}
    //---------------------------------------------------------------------
	size_t DeflateStream::read(void* buf, size_t count)
	{
		if (!mIsCompressedValid)
		{
			return mCompressedStream->read(buf, count);
		}
		
		if (getAccessMode() & WRITE)
		{
			return mTmpWriteStream->read(buf, count);
		}
		else 
		{

			size_t restorePoint = mCompressedStream->tell();
			// read from cache first
			size_t cachereads = mReadCache.read(buf, count);
			
			size_t newReadUncompressed = 0;

			if (cachereads < count)
			{
				mpZStream->avail_out = count - cachereads;
				mpZStream->next_out = (Bytef*)buf + cachereads;
				
				while (mpZStream->avail_out)
				{
					// Pull next chunk of compressed data from the underlying stream
					if (!mpZStream->avail_in && !mCompressedStream->eof())
					{
						mpZStream->avail_in = mCompressedStream->read(mpTmp, OGRE_DEFLATE_TMP_SIZE);
						mpZStream->next_in = mpTmp;
					}
					
					if (mpZStream->avail_in)
					{
						int availpre = mpZStream->avail_out;
						int status = inflate(mpZStream, Z_SYNC_FLUSH);
						size_t readUncompressed = availpre - mpZStream->avail_out;
						newReadUncompressed += readUncompressed;
						if (status != Z_OK)
						{
							// End of data, or error
							if (status != Z_STREAM_END)
							{
								mCompressedStream->seek(restorePoint);
								OGRE_EXCEPT(Exception::ERR_INVALID_STATE, 
											"Error in compressed stream",
											"DeflateStrea::read");
							}
							else 
							{
								// back up the stream so that it can be used from the end onwards													
								long unusedCompressed = mpZStream->avail_in;
								mCompressedStream->skip(-unusedCompressed);
							}

							break;
						}
					}
				}
			}
			
			// Cache the last bytes read
			mReadCache.cacheData((char*)buf + cachereads, newReadUncompressed);
			
			mCurrentPos += newReadUncompressed + cachereads;
			
			return newReadUncompressed + cachereads;
		}
	}
    //---------------------------------------------------------------------
	size_t DeflateStream::write(const void* buf, size_t count)
	{
		if ((getAccessMode() & WRITE) == 0)
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
						"Not a writable stream", "DeflateStream::write");
		
		return mTmpWriteStream->write(buf, count);
	}
    //---------------------------------------------------------------------
	void DeflateStream::compressFinal()
	{
		// Close temp stream
		mTmpWriteStream->close();
		
		// Copy & compress
		// We do this rather than compress directly because some code seeks
		// around while writing (e.g. to update size blocks) which is not
		// possible when compressing on the fly
		
		int ret, flush;
		char in[OGRE_DEFLATE_TMP_SIZE];
		char out[OGRE_DEFLATE_TMP_SIZE];
		
		if (deflateInit(mpZStream, Z_DEFAULT_COMPRESSION) != Z_OK)
		{
			destroy();
			OGRE_EXCEPT(Exception::ERR_INVALID_STATE, 
						"Error initialising deflate compressed stream!",
						"DeflateStream::init");
		}
		
		std::ifstream inFile;
		inFile.open(mTempFileName.c_str(), std::ios::in | std::ios::binary);
		
		do 
		{
			inFile.read(in, OGRE_DEFLATE_TMP_SIZE);
			mpZStream->avail_in = inFile.gcount();
			if (inFile.bad()) 
			{
				deflateEnd(mpZStream);
				OGRE_EXCEPT(Exception::ERR_INVALID_STATE, 
							"Error reading temp uncompressed stream!",
							"DeflateStream::init");
			}
			flush = inFile.eof() ? Z_FINISH : Z_NO_FLUSH;
			mpZStream->next_in = (Bytef*)in;
			
			/* run deflate() on input until output buffer not full, finish
			 compression if all of source has been read in */
			do 
			{
				mpZStream->avail_out = OGRE_DEFLATE_TMP_SIZE;
				mpZStream->next_out = (Bytef*)out;
				ret = deflate(mpZStream, flush);    /* no bad return value */
				assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
				size_t compressed = OGRE_DEFLATE_TMP_SIZE - mpZStream->avail_out;
				mCompressedStream->write(out, compressed);
			} while (mpZStream->avail_out == 0);
			assert(mpZStream->avail_in == 0);     /* all input will be used */
			
			/* done when last data in file processed */
		} while (flush != Z_FINISH);
		assert(ret == Z_STREAM_END);        /* stream will be complete */
		
		deflateEnd(mpZStream);
				
		remove(mTempFileName.c_str());
						
	}
    //---------------------------------------------------------------------
	void DeflateStream::skip(long count)
	{
		if (!mIsCompressedValid)
		{
			mCompressedStream->skip(count);
			return;
		}
		
		if (getAccessMode() & WRITE)
		{
			mTmpWriteStream->skip(count);
		}
		else 
		{
			if (count > 0)
			{
				if (!mReadCache.ff(count))
				{
					OGRE_EXCEPT(Exception::ERR_INVALID_STATE, 
								"You can only skip within the cache range in a deflate stream.",
								"DeflateStream::skip");
				}
			}
			else if (count < 0)
			{
				if (!mReadCache.rewind((size_t)(-count)))
				{
					OGRE_EXCEPT(Exception::ERR_INVALID_STATE, 
								"You can only skip within the cache range in a deflate stream.",
								"DeflateStream::skip");
				}
			}
		}		
		mCurrentPos = static_cast<size_t>(static_cast<long>(mCurrentPos) + count);
		
		
	}
    //---------------------------------------------------------------------
	void DeflateStream::seek( size_t pos )
	{
		if (!mIsCompressedValid)
		{
			mCompressedStream->seek(pos);
			return;
		}
		if (getAccessMode() & WRITE)
		{
			mTmpWriteStream->seek(pos);
		}
		else
		{
			if (pos == 0)
			{
				mCurrentPos = 0;
				mpZStream->next_in = mpTmp;
				mCompressedStream->seek(0);
				mpZStream->avail_in = mCompressedStream->read(mpTmp, OGRE_DEFLATE_TMP_SIZE);			
				inflateReset(mpZStream);
			}
			else 
			{
				skip(pos - tell());
			}
		}		
	}
    //---------------------------------------------------------------------
	size_t DeflateStream::tell(void) const
	{
		if (!mIsCompressedValid)
		{
			return mCompressedStream->tell();
		}
		else if(getAccessMode() & WRITE) 
		{
			return mTmpWriteStream->tell();
		}
		else
		{
			return mCurrentPos;
		}

	}
    //---------------------------------------------------------------------
	bool DeflateStream::eof(void) const
	{
		if (getAccessMode() & WRITE)
			return mTmpWriteStream->eof();
		else 
		{
			if (!mIsCompressedValid)
				return mCompressedStream->eof();
			else
				return mCompressedStream->eof() && mpZStream->avail_in == 0;
		}
	}
    //---------------------------------------------------------------------
	void DeflateStream::close(void)
	{
		if (getAccessMode() & WRITE)
		{
			compressFinal();
		}
		
		// don't close underlying compressed stream in case used for something else
	}
    //---------------------------------------------------------------------
	
	
}


