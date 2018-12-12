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

#if OGRE_NO_ZIP_ARCHIVE == 0

#include "OgreDeflate.h"
#include "OgreException.h"
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS || OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#include "macUtils.h"
#endif

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
    DeflateStream::DeflateStream(const DataStreamPtr& compressedStream, const String& tmpFileName, size_t avail_in)
    : DataStream(compressedStream->getAccessMode())
    , mCompressedStream(compressedStream)
    , mTempFileName(tmpFileName)
    , mZStream(0)
    , mCurrentPos(0)
    , mAvailIn(avail_in)
    , mTmp(0)
    , mStreamType(ZLib)
    {
        init();
    }
    //---------------------------------------------------------------------
    DeflateStream::DeflateStream(const String& name, const DataStreamPtr& compressedStream, const String& tmpFileName, size_t avail_in)
    : DataStream(name, compressedStream->getAccessMode())
    , mCompressedStream(compressedStream)
    , mTempFileName(tmpFileName)
    , mZStream(0)
    , mCurrentPos(0)
    , mAvailIn(avail_in)
    , mTmp(0)
    , mStreamType(ZLib)
    {
        init();
    }
    //---------------------------------------------------------------------
    DeflateStream::DeflateStream(const String& name, const DataStreamPtr& compressedStream, StreamType streamType, const String& tmpFileName, size_t avail_in)
    : DataStream(name, compressedStream->getAccessMode())
    , mCompressedStream(compressedStream)
    , mTempFileName(tmpFileName)
    , mZStream(0)
    , mCurrentPos(0)
    , mAvailIn(avail_in)
    , mTmp(0)
    , mStreamType(streamType)
    {
        init();
    }
    //---------------------------------------------------------------------
    size_t DeflateStream::getAvailInForSinglePass()
    {
        size_t ret = OGRE_DEFLATE_TMP_SIZE;

        // if we are doing particial-uncompressing
        if(mAvailIn>0)
        {
            if(mAvailIn<ret)
                ret = mAvailIn;
            mAvailIn -= ret;
        }

        return ret;
    }
    //---------------------------------------------------------------------
    void DeflateStream::init()
    {
        mZStream = OGRE_ALLOC_T(z_stream, 1, MEMCATEGORY_GENERAL);
        mZStream->zalloc = OgreZalloc;
        mZStream->zfree = OgreZfree;
        
        if (getAccessMode() == READ)
        {
            mTmp = (unsigned char*)OGRE_MALLOC(OGRE_DEFLATE_TMP_SIZE, MEMCATEGORY_GENERAL);
            size_t restorePoint = mCompressedStream->tell();
            // read early chunk
            mZStream->next_in = mTmp;
            mZStream->avail_in = static_cast<uint>(mCompressedStream->read(mTmp, getAvailInForSinglePass()));
            
            int windowBits = (mStreamType == Deflate) ? -MAX_WBITS : (mStreamType == GZip) ? 16 + MAX_WBITS : MAX_WBITS;
            if (inflateInit2(mZStream, windowBits) != Z_OK)
            {
                mStreamType = Invalid;
            }
            
            if (mStreamType != Invalid)
            {
                // in fact, inflateInit on some implementations doesn't try to read
                // anything. We need to at least read something to test
                Bytef testOut[4];
                size_t savedIn = mZStream->avail_in;
                mZStream->avail_out = 4;
                mZStream->next_out = testOut;
                if (inflate(mZStream, Z_SYNC_FLUSH) != Z_OK)
                    mStreamType = Invalid;
                // restore for reading
                mZStream->avail_in = static_cast<uint>(savedIn);
                mZStream->next_in = mTmp;

                inflateReset(mZStream);
            }

            if (mStreamType == Invalid)
            {
                // Not compressed data!
                // Fail gracefully, fall back on reading the underlying stream direct
                destroy();
                mCompressedStream->seek(restorePoint);
            }               
        }
        else 
        {
            if(mTempFileName.empty())
            {
                // Write to temp file
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
                char* tmpname = _tempnam(".", "ogre");
                if (!tmpname)
                {
                    // Having no file name here will cause various problems later.
                    OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Temporary file name generation failed.", "DeflateStream::init");
                }
                else
                {
                    mTempFileName = tmpname;
                    free(tmpname);
                }
#elif OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS || OGRE_PLATFORM == OGRE_PLATFORM_APPLE
                mTempFileName = macTempFileName();
#else
                char tmpname[L_tmpnam];
                if (!tmpnam(tmpname))
                    OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Temporary file name generation failed.", "DeflateStream::init");

                mTempFileName = tmpname;
#endif
            }

            std::fstream *f = OGRE_NEW_T(std::fstream, MEMCATEGORY_GENERAL)();
            f->open(mTempFileName.c_str(), std::ios::binary | std::ios::out);
            mTmpWriteStream = DataStreamPtr(OGRE_NEW FileStreamDataStream(f));
            
        }

    }
    //---------------------------------------------------------------------
    void DeflateStream::destroy()
    {
        if (getAccessMode() == READ)
            inflateEnd(mZStream);

        OGRE_FREE(mZStream, MEMCATEGORY_GENERAL);
        mZStream = 0;
        OGRE_FREE(mTmp, MEMCATEGORY_GENERAL);
        mTmp = 0;
    }
    //---------------------------------------------------------------------
    DeflateStream::~DeflateStream()
    {
        close();
    }
    //---------------------------------------------------------------------
    size_t DeflateStream::read(void* buf, size_t count)
    {
        if (mStreamType == Invalid)
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
                mZStream->avail_out = static_cast<uint>(count - cachereads);
                mZStream->next_out = (Bytef*)buf + cachereads;
                
                while (mZStream->avail_out)
                {
                    // Pull next chunk of compressed data from the underlying stream
                    if (!mZStream->avail_in && !mCompressedStream->eof())
                    {
                        mZStream->avail_in = static_cast<uint>(mCompressedStream->read(mTmp, getAvailInForSinglePass()));
                        mZStream->next_in = mTmp;
                    }
                    
                    if (mZStream->avail_in)
                    {
                        int availpre = mZStream->avail_out;
                        int status = inflate(mZStream, Z_SYNC_FLUSH);
                        size_t readUncompressed = availpre - mZStream->avail_out;
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
                                long unusedCompressed = mZStream->avail_in;
                                mCompressedStream->skip(-unusedCompressed);
                            }

                            break;
                        }
                    }
                }
            
                // Cache the last bytes read not from cache
                mReadCache.cacheData((char*)buf + cachereads, newReadUncompressed);
            }
            
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
        mTmpWriteStream.setNull();
        
        // Copy & compress
        // We do this rather than compress directly because some code seeks
        // around while writing (e.g. to update size blocks) which is not
        // possible when compressing on the fly
        
        int ret, flush;
        char in[OGRE_DEFLATE_TMP_SIZE];
        char out[OGRE_DEFLATE_TMP_SIZE];
        
        int windowBits = (mStreamType == Deflate) ? -MAX_WBITS : (mStreamType == GZip) ? 16 + MAX_WBITS : MAX_WBITS;
        if (deflateInit2(mZStream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, windowBits, 8, Z_DEFAULT_STRATEGY) != Z_OK)
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
            mZStream->avail_in = (uInt)inFile.gcount();
            if (inFile.bad()) 
            {
                deflateEnd(mZStream);
                OGRE_EXCEPT(Exception::ERR_INVALID_STATE, 
                            "Error reading temp uncompressed stream!",
                            "DeflateStream::init");
            }
            flush = inFile.eof() ? Z_FINISH : Z_NO_FLUSH;
            mZStream->next_in = (Bytef*)in;
            
            /* run deflate() on input until output buffer not full, finish
             compression if all of source has been read in */
            do 
            {
                mZStream->avail_out = OGRE_DEFLATE_TMP_SIZE;
                mZStream->next_out = (Bytef*)out;
                ret = deflate(mZStream, flush);    /* no bad return value */
                assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
                size_t compressed = OGRE_DEFLATE_TMP_SIZE - mZStream->avail_out;
                mCompressedStream->write(out, compressed);
            } while (mZStream->avail_out == 0);
            assert(mZStream->avail_in == 0);     /* all input will be used */
            
            /* done when last data in file processed */
        } while (flush != Z_FINISH);
        assert(ret == Z_STREAM_END);        /* stream will be complete */
                (void)ret;
        deflateEnd(mZStream);

        inFile.close();
        remove(mTempFileName.c_str());
                        
    }
    //---------------------------------------------------------------------
    void DeflateStream::skip(long count)
    {
        if (mStreamType == Invalid)
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
        if (mStreamType == Invalid)
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
                mZStream->next_in = mTmp;
                mCompressedStream->seek(0);
                mZStream->avail_in = static_cast<uint>(mCompressedStream->read(mTmp, getAvailInForSinglePass()));
                inflateReset(mZStream);
                mReadCache.clear();
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
        if (mStreamType == Invalid)
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
            if (mStreamType == Invalid)
                return mCompressedStream->eof();
            else
                return mCompressedStream->eof() && mZStream->avail_in == 0;
        }
    }
    //---------------------------------------------------------------------
    void DeflateStream::close(void)
    {
        if (getAccessMode() & WRITE)
            compressFinal();

        destroy();

        mAccess = 0;

        // don't close underlying compressed stream in case used for something else
    }
    //---------------------------------------------------------------------
    
    
}

#endif
