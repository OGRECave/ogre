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
#ifndef __Zip_H__
#define __Zip_H__

#include "OgrePrerequisites.h"

#include "OgreArchive.h"
#include "OgreArchiveFactory.h"

// Forward declaration for zziplib to avoid header file dependency.
typedef struct zzip_dir		ZZIP_DIR;
typedef struct zzip_file	ZZIP_FILE;

namespace Ogre {

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Resources
	*  @{
	*/
	/** Specialisation of the Archive class to allow reading of files from a zip
        format source archive.
    @remarks
        This archive format supports all archives compressed in the standard
        zip format, including iD pk3 files.
    */
    class _OgreExport ZipArchive : public Archive 
    {
    protected:
        /// Handle to root zip file
        ZZIP_DIR* mZzipDir;
        /// Handle any errors from zzip
        void checkZzipError(int zzipError, const String& operation) const;
        /// File list (since zziplib seems to only allow scanning of dir tree once)
        FileInfoList mFileList;

		OGRE_AUTO_MUTEX
    public:
        ZipArchive(const String& name, const String& archType );
        ~ZipArchive();
        /// @copydoc Archive::isCaseSensitive
        bool isCaseSensitive(void) const { return false; }

        /// @copydoc Archive::load
        void load();
        /// @copydoc Archive::unload
        void unload();

        /// @copydoc Archive::open
        DataStreamPtr open(const String& filename, bool readOnly = true) const;

		/// @copydoc Archive::create
		DataStreamPtr create(const String& filename) const;

		/// @copydoc Archive::remove
		void remove(const String& filename) const;

        /// @copydoc Archive::list
        StringVectorPtr list(bool recursive = true, bool dirs = false);

        /// @copydoc Archive::listFileInfo
        FileInfoListPtr listFileInfo(bool recursive = true, bool dirs = false);

        /// @copydoc Archive::find
        StringVectorPtr find(const String& pattern, bool recursive = true,
            bool dirs = false);

        /// @copydoc Archive::findFileInfo
        FileInfoListPtr findFileInfo(const String& pattern, bool recursive = true,
            bool dirs = false);

        /// @copydoc Archive::exists
        bool exists(const String& filename);

		/// @copydoc Archive::getModifiedTime
		time_t getModifiedTime(const String& filename);
    };

    /** Specialisation of ArchiveFactory for Zip files. */
    class _OgrePrivate ZipArchiveFactory : public ArchiveFactory
    {
    public:
        virtual ~ZipArchiveFactory() {}
        /// @copydoc FactoryObj::getType
        const String& getType(void) const;
        /// @copydoc FactoryObj::createInstance
        Archive *createInstance( const String& name ) 
        {
            return OGRE_NEW ZipArchive(name, "Zip");
        }
        /// @copydoc FactoryObj::destroyInstance
        void destroyInstance( Archive* arch) { OGRE_DELETE arch; }
    };

	/** Template version of cache based on static array.
	'cacheSize' defines size of cache in bytes. */
	template <size_t cacheSize>
	class StaticCache
	{
	protected:
		/// Static buffer
		char mBuffer[cacheSize];

		/// Number of bytes valid in cache (written from the beginning of static buffer)
		size_t mValidBytes;
		/// Current read position
		size_t mPos;

	public:
		/// Constructor
		StaticCache()
		{
			mValidBytes = 0;
			mPos = 0;
		}

		/** Cache data pointed by 'buf'. If 'count' is greater than cache size, we cache only last bytes.
		Returns number of bytes written to cache. */
		size_t cacheData(const void* buf, size_t count)
		{
			assert(avail() == 0 && "It is assumed that you cache data only after you have read everything.");

			if (count < cacheSize)
			{
				// number of bytes written is less than total size of cache
				if (count + mValidBytes <= cacheSize)
				{
					// just append
					memcpy(mBuffer + mValidBytes, buf, count);
					mValidBytes += count;
				}
				else
				{
					size_t begOff = count - (cacheSize - mValidBytes);
					// override old cache content in the beginning
					memmove(mBuffer, mBuffer + begOff, mValidBytes - begOff);
					// append new data
					memcpy(mBuffer + cacheSize - count, buf, count);
					mValidBytes = cacheSize;
				}
				mPos = mValidBytes;
				return count;
			}
			else
			{
				// discard all
				memcpy(mBuffer, (const char*)buf + count - cacheSize, cacheSize);
				mValidBytes = mPos = cacheSize;
				return cacheSize;
			}
		}
		/** Read data from cache to 'buf' (maximum 'count' bytes). Returns number of bytes read from cache. */
		size_t read(void* buf, size_t count)
		{
			size_t rb = avail();
			rb = (rb < count) ? rb : count;
			memcpy(buf, mBuffer + mPos, rb);
			mPos += rb;
			return rb;
		}

		/** Step back in cached stream by 'count' bytes. Returns 'true' if cache contains resulting position. */
		bool rewind(size_t count)
		{
			if (mPos < count)
			{
				clear();
				return false;
			}
			else
			{
				mPos -= count;
				return true;
			}
		}
		/** Step forward in cached stream by 'count' bytes. Returns 'true' if cache contains resulting position. */
		bool ff(size_t count)
		{
			if (avail() < count)
			{
				clear();
				return false;
			}
			else
			{
				mPos += count;
				return true;
			}
		}

		/** Returns number of bytes available for reading in cache after rewinding. */
		size_t avail() const
		{
			return mValidBytes - mPos;
		}

		/** Clear the cache */
		void clear()
		{
			mValidBytes = 0;
			mPos = 0;
		}
	};

	/** Dummy version of cache to test no caching. 
	If you want to test, just uncomment it and add 'No' prefix
	to type in line 'StaticCache<2 * OGRE_STREAM_TEMP_SIZE> mCache;' of class ZipDataStream */
	/*template <size_t cacheSize>
	class NoStaticCache
	{
	public:
		NoStaticCache() { }

		size_t cacheData(const void* buf, size_t count) { return 0; }
		size_t read(void* buf, size_t count) { return 0; }

		bool rewind(size_t count) { return false; }
		bool ff(size_t count) { return false; }

		size_t avail() const { return 0; }

		void clear() { }
	};*/

    /** Specialisation of DataStream to handle streaming data from zip archives. */
    class _OgrePrivate ZipDataStream : public DataStream
    {
    protected:
        ZZIP_FILE* mZzipFile;
		/// We need caching because sometimes serializers step back in data stream and zziplib behaves slow
		StaticCache<2 * OGRE_STREAM_TEMP_SIZE> mCache;
    public:
        /// Unnamed constructor
        ZipDataStream(ZZIP_FILE* zzipFile, size_t uncompressedSize);
        /// Constructor for creating named streams
        ZipDataStream(const String& name, ZZIP_FILE* zzipFile, size_t uncompressedSize);
		~ZipDataStream();
        /// @copydoc DataStream::read
        size_t read(void* buf, size_t count);
		/// @copydoc DataStream::write
		size_t write(void* buf, size_t count);
        /// @copydoc DataStream::skip
        void skip(long count);
        /// @copydoc DataStream::seek
        void seek( size_t pos );
        /// @copydoc DataStream::seek
        size_t tell(void) const;
        /// @copydoc DataStream::eof
        bool eof(void) const;
        /// @copydoc DataStream::close
        void close(void);


    };

	/** @} */
	/** @} */

}

#endif
