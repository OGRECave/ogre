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

#include "OgreZip.h"

#include "OgreLogManager.h"
#include "OgreException.h"
#include "OgreStringVector.h"
#include "OgreRoot.h"

#include <zzip/zzip.h>


namespace Ogre {

    /// Utility method to format out zzip errors
    String getZzipErrorDescription(zzip_error_t zzipError) 
    {
        String errorMsg;
        switch (zzipError)
        {
        case ZZIP_NO_ERROR:
            break;
        case ZZIP_OUTOFMEM:
            errorMsg = "Out of memory.";
            break;            
        case ZZIP_DIR_OPEN:
        case ZZIP_DIR_STAT: 
        case ZZIP_DIR_SEEK:
        case ZZIP_DIR_READ:
            errorMsg = "Unable to read zip file.";
            break;            
        case ZZIP_UNSUPP_COMPR:
            errorMsg = "Unsupported compression format.";
            break;            
        case ZZIP_CORRUPTED:
            errorMsg = "Corrupted archive.";
            break;            
        default:
            errorMsg = "Unknown error.";
            break;            
        };

        return errorMsg;
    }
    //-----------------------------------------------------------------------
    ZipArchive::ZipArchive(const String& name, const String& archType )
        : Archive(name, archType), mZzipDir(0)
    {
    }
    //-----------------------------------------------------------------------
    ZipArchive::~ZipArchive()
    {
        unload();
    }
    //-----------------------------------------------------------------------
    void ZipArchive::load()
    {
		OGRE_LOCK_AUTO_MUTEX
        if (!mZzipDir)
        {
            zzip_error_t zzipError;
            mZzipDir = zzip_dir_open(mName.c_str(), &zzipError);
            checkZzipError(zzipError, "opening archive");

            // Cache names
            ZZIP_DIRENT zzipEntry;
            while (zzip_dir_read(mZzipDir, &zzipEntry))
            {
                FileInfo info;
				info.archive = this;
                // Get basename / path
                StringUtil::splitFilename(zzipEntry.d_name, info.basename, info.path);
                info.filename = zzipEntry.d_name;
                // Get sizes
                info.compressedSize = static_cast<size_t>(zzipEntry.d_csize);
                info.uncompressedSize = static_cast<size_t>(zzipEntry.st_size);
                // folder entries
                if (info.basename.empty())
                {
                    info.filename = info.filename.substr (0, info.filename.length () - 1);
                    StringUtil::splitFilename(info.filename, info.basename, info.path);
                    // Set compressed size to -1 for folders; anyway nobody will check
                    // the compressed size of a folder, and if he does, its useless anyway
                    info.compressedSize = size_t (-1);
                }

                mFileList.push_back(info);

            }

        }
    }
    //-----------------------------------------------------------------------
    void ZipArchive::unload()
    {
		OGRE_LOCK_AUTO_MUTEX
        if (mZzipDir)
        {
            zzip_dir_close(mZzipDir);
            mZzipDir = 0;
            mFileList.clear();
        }
    
    }
    //-----------------------------------------------------------------------
	DataStreamPtr ZipArchive::open(const String& filename, bool readOnly) const
    {
		// zziplib is not threadsafe
		OGRE_LOCK_AUTO_MUTEX

        // Format not used here (always binary)
        ZZIP_FILE* zzipFile = 
            zzip_file_open(mZzipDir, filename.c_str(), ZZIP_ONLYZIP | ZZIP_CASELESS);
        if (!zzipFile)
		{
            int zerr = zzip_error(mZzipDir);
            String zzDesc = getZzipErrorDescription((zzip_error_t)zerr);
            LogManager::getSingleton().logMessage(
                mName + " - Unable to open file " + filename + ", error was '" + zzDesc + "'");
                
			// return null pointer
			return DataStreamPtr();
		}

		// Get uncompressed size too
		ZZIP_STAT zstat;
		zzip_dir_stat(mZzipDir, filename.c_str(), &zstat, ZZIP_CASEINSENSITIVE);

        // Construct & return stream
        return DataStreamPtr(OGRE_NEW ZipDataStream(filename, zzipFile, static_cast<size_t>(zstat.st_size)));

    }
	//---------------------------------------------------------------------
	DataStreamPtr ZipArchive::create(const String& filename) const
	{
		OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, 
			"Modification of zipped archives is not supported", 
			"ZipArchive::create");

	}
	//---------------------------------------------------------------------
	void ZipArchive::remove(const String& filename) const
	{
	}
    //-----------------------------------------------------------------------
    StringVectorPtr ZipArchive::list(bool recursive, bool dirs)
    {
		OGRE_LOCK_AUTO_MUTEX
        StringVectorPtr ret = StringVectorPtr(OGRE_NEW_T(StringVector, MEMCATEGORY_GENERAL)(), SPFM_DELETE_T);

        FileInfoList::iterator i, iend;
        iend = mFileList.end();
        for (i = mFileList.begin(); i != iend; ++i)
            if ((dirs == (i->compressedSize == size_t (-1))) &&
                (recursive || i->path.empty()))
                ret->push_back(i->filename);

        return ret;
    }
    //-----------------------------------------------------------------------
    FileInfoListPtr ZipArchive::listFileInfo(bool recursive, bool dirs)
    {
		OGRE_LOCK_AUTO_MUTEX
        FileInfoList* fil = OGRE_NEW_T(FileInfoList, MEMCATEGORY_GENERAL)();
        FileInfoList::const_iterator i, iend;
        iend = mFileList.end();
        for (i = mFileList.begin(); i != iend; ++i)
            if ((dirs == (i->compressedSize == size_t (-1))) &&
                (recursive || i->path.empty()))
                fil->push_back(*i);

        return FileInfoListPtr(fil, SPFM_DELETE_T);
    }
    //-----------------------------------------------------------------------
    StringVectorPtr ZipArchive::find(const String& pattern, bool recursive, bool dirs)
    {
		OGRE_LOCK_AUTO_MUTEX
        StringVectorPtr ret = StringVectorPtr(OGRE_NEW_T(StringVector, MEMCATEGORY_GENERAL)(), SPFM_DELETE_T);
        // If pattern contains a directory name, do a full match
        bool full_match = (pattern.find ('/') != String::npos) ||
                          (pattern.find ('\\') != String::npos);

        FileInfoList::iterator i, iend;
        iend = mFileList.end();
        for (i = mFileList.begin(); i != iend; ++i)
            if ((dirs == (i->compressedSize == size_t (-1))) &&
                (recursive || full_match || i->path.empty()))
                // Check basename matches pattern (zip is case insensitive)
                if (StringUtil::match(full_match ? i->filename : i->basename, pattern, false))
                    ret->push_back(i->filename);

        return ret;
    }
    //-----------------------------------------------------------------------
	FileInfoListPtr ZipArchive::findFileInfo(const String& pattern, 
        bool recursive, bool dirs)
    {
		OGRE_LOCK_AUTO_MUTEX
        FileInfoListPtr ret = FileInfoListPtr(OGRE_NEW_T(FileInfoList, MEMCATEGORY_GENERAL)(), SPFM_DELETE_T);
        // If pattern contains a directory name, do a full match
        bool full_match = (pattern.find ('/') != String::npos) ||
                          (pattern.find ('\\') != String::npos);

        FileInfoList::iterator i, iend;
        iend = mFileList.end();
        for (i = mFileList.begin(); i != iend; ++i)
            if ((dirs == (i->compressedSize == size_t (-1))) &&
                (recursive || full_match || i->path.empty()))
                // Check name matches pattern (zip is case insensitive)
                if (StringUtil::match(full_match ? i->filename : i->basename, pattern, false))
                    ret->push_back(*i);

        return ret;
    }
    //-----------------------------------------------------------------------
	bool ZipArchive::exists(const String& filename)
	{
		// zziplib is not threadsafe
		OGRE_LOCK_AUTO_MUTEX
		ZZIP_STAT zstat;
		int res = zzip_dir_stat(mZzipDir, filename.c_str(), &zstat, ZZIP_CASEINSENSITIVE);

		return (res == ZZIP_NO_ERROR);

	}
	//---------------------------------------------------------------------
	time_t ZipArchive::getModifiedTime(const String& filename)
	{
		// Zziplib doesn't yet support getting the modification time of individual files
		// so just check the mod time of the zip itself
		struct stat tagStat;
		bool ret = (stat(mName.c_str(), &tagStat) == 0);

		if (ret)
		{
			return tagStat.st_mtime;
		}
		else
		{
			return 0;
		}

	}
	//-----------------------------------------------------------------------
    void ZipArchive::checkZzipError(int zzipError, const String& operation) const
    {
        if (zzipError != ZZIP_NO_ERROR)
        {
            String errorMsg = getZzipErrorDescription(static_cast<zzip_error_t>(zzipError));

            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
                mName + " - error whilst " + operation + ": " + errorMsg,
                "ZipArchive::checkZzipError");
        }
    }
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    ZipDataStream::ZipDataStream(ZZIP_FILE* zzipFile, size_t uncompressedSize)
        : mZzipFile(zzipFile)
    {
		mSize = uncompressedSize;
    }
    //-----------------------------------------------------------------------
    ZipDataStream::ZipDataStream(const String& name, ZZIP_FILE* zzipFile, size_t uncompressedSize)
        :DataStream(name), mZzipFile(zzipFile)
    {
		mSize = uncompressedSize;
    }
    //-----------------------------------------------------------------------
	ZipDataStream::~ZipDataStream()
	{
		close();
	}
    //-----------------------------------------------------------------------
    size_t ZipDataStream::read(void* buf, size_t count)
    {
		size_t was_avail = mCache.read(buf, count);
		zzip_ssize_t r = 0;
		if (was_avail < count)
		{
			r = zzip_file_read(mZzipFile, (char*)buf + was_avail, count - was_avail);
			if (r<0) {
				ZZIP_DIR *dir = zzip_dirhandle(mZzipFile);
				String msg = zzip_strerror_of(dir);
				OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
					mName+" - error from zziplib: "+msg,
					"ZipDataStream::read");
			}
			mCache.cacheData((char*)buf + was_avail, (size_t)r);
		}
		return was_avail + (size_t)r;
    }
	//---------------------------------------------------------------------
	size_t ZipDataStream::write(void* buf, size_t count)
	{
		// not supported
		return 0;
	}
    //-----------------------------------------------------------------------
    void ZipDataStream::skip(long count)
    {
        long was_avail = static_cast<long>(mCache.avail());
		if (count > 0)
		{
			if (!mCache.ff(count))
				zzip_seek(mZzipFile, static_cast<zzip_off_t>(count - was_avail), SEEK_CUR);
		}
		else if (count < 0)
		{
			if (!mCache.rewind((size_t)(-count)))
				zzip_seek(mZzipFile, static_cast<zzip_off_t>(count + was_avail), SEEK_CUR);
		}
    }
    //-----------------------------------------------------------------------
    void ZipDataStream::seek( size_t pos )
    {
		zzip_off_t newPos = static_cast<zzip_off_t>(pos);
		zzip_off_t prevPos = static_cast<zzip_off_t>(tell());
		if (prevPos < 0)
		{
			// seek set after invalid pos
			mCache.clear();
			zzip_seek(mZzipFile, newPos, SEEK_SET);
		}
		else
		{
			// everything is going all right, relative seek
			skip(newPos - prevPos);
		}
    }
    //-----------------------------------------------------------------------
    size_t ZipDataStream::tell(void) const
    {
		zzip_off_t pos = zzip_tell(mZzipFile);
		if (pos<0)
			return (size_t)(-1);
		return static_cast<size_t>(pos) - mCache.avail();
    }
    //-----------------------------------------------------------------------
    bool ZipDataStream::eof(void) const
    {
        return (tell() >= mSize);
    }
    //-----------------------------------------------------------------------
    void ZipDataStream::close(void)
    {
		if (mZzipFile != 0)
		{
			zzip_file_close(mZzipFile);
			mZzipFile = 0;
		}
		mCache.clear();
    }
    //-----------------------------------------------------------------------
    const String& ZipArchiveFactory::getType(void) const
    {
        static String name = "Zip";
        return name;
    }

}
