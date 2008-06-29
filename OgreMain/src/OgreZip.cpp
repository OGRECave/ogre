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
        if (mZzipDir)
        {
            zzip_dir_close(mZzipDir);
            mZzipDir = 0;
            mFileList.clear();
        }
    
    }
    //-----------------------------------------------------------------------
	DataStreamPtr ZipArchive::open(const String& filename) const
    {

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
    //-----------------------------------------------------------------------
    StringVectorPtr ZipArchive::list(bool recursive, bool dirs)
    {
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
		ZZIP_STAT zstat;
		int res = zzip_dir_stat(mZzipDir, filename.c_str(), &zstat, ZZIP_CASEINSENSITIVE);

		return (res == ZZIP_NO_ERROR);

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
        zzip_ssize_t r = zzip_file_read(mZzipFile, (char*)buf, count);
        if (r<0) {
            ZZIP_DIR *dir = zzip_dirhandle(mZzipFile);
            String msg = zzip_strerror_of(dir);
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        mName+" - error from zziplib: "+msg,
                        "ZipDataStream::read");
        }
        return (size_t) r;
    }
    //-----------------------------------------------------------------------
    void ZipDataStream::skip(long count)
    {
        zzip_seek(mZzipFile, static_cast<zzip_off_t>(count), SEEK_CUR);
    }
    //-----------------------------------------------------------------------
    void ZipDataStream::seek( size_t pos )
    {
		zzip_seek(mZzipFile, static_cast<zzip_off_t>(pos), SEEK_SET);
    }
    //-----------------------------------------------------------------------
    size_t ZipDataStream::tell(void) const
    {
		return zzip_tell(mZzipFile);
    }
    //-----------------------------------------------------------------------
    bool ZipDataStream::eof(void) const
    {
        return (zzip_tell(mZzipFile) >= static_cast<zzip_off_t>(mSize));
    }
    //-----------------------------------------------------------------------
    void ZipDataStream::close(void)
    {
        zzip_file_close(mZzipFile);
    }
    //-----------------------------------------------------------------------
    const String& ZipArchiveFactory::getType(void) const
    {
        static String name = "Zip";
        return name;
    }

}
