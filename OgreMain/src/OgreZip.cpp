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
// workaround for Wundef in zzip/conf.h
#ifndef __GNUC_MINOR_
#define __GNUC_MINOR_ 0
#endif

#include <zzip/zzip.h>
#include <zzip/plugin.h>

#ifdef _OGRE_FILESYSTEM_ARCHIVE_UNICODE
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#   include <stringapiset.h>
#endif

namespace Ogre {
namespace {
    class ZipArchive : public Archive
    {
    protected:
        /// Handle to root zip file
        ZZIP_DIR* mZzipDir;
        /// File list (since zziplib seems to only allow scanning of dir tree once)
        FileInfoList mFileList;
        /// A pointer to file io alternative implementation
        const zzip_plugin_io_handlers* mPluginIo;

        OGRE_AUTO_MUTEX;
    public:
        ZipArchive(const String& name, const String& archType, const zzip_plugin_io_handlers* pluginIo);
        ~ZipArchive();
        /// @copydoc Archive::isCaseSensitive
        bool isCaseSensitive(void) const { return OGRE_RESOURCEMANAGER_STRICT != 0; }

        /// @copydoc Archive::load
        void load();
        /// @copydoc Archive::unload
        void unload();

        /// @copydoc Archive::open
        DataStreamPtr open(const String& filename, bool readOnly = true) const;

        /// @copydoc Archive::create
        DataStreamPtr create(const String& filename);

        /// @copydoc Archive::remove
        void remove(const String& filename);

        /// @copydoc Archive::list
        StringVectorPtr list(bool recursive = true, bool dirs = false) const;

        /// @copydoc Archive::listFileInfo
        FileInfoListPtr listFileInfo(bool recursive = true, bool dirs = false) const;

        /// @copydoc Archive::find
        StringVectorPtr find(const String& pattern, bool recursive = true,
            bool dirs = false) const;

        /// @copydoc Archive::findFileInfo
        FileInfoListPtr findFileInfo(const String& pattern, bool recursive = true,
            bool dirs = false) const;

        /// @copydoc Archive::exists
        bool exists(const String& filename) const;

        /// @copydoc Archive::getModifiedTime
        time_t getModifiedTime(const String& filename) const;
    };

    /** Specialisation of DataStream to handle streaming data from zip archives. */
    class ZipDataStream : public DataStream
    {
    protected:
        ZZIP_FILE* mZzipFile;
        /// We need caching because sometimes serializers step back in data stream and zziplib behaves slow
        StaticCache<2 * OGRE_STREAM_TEMP_SIZE> mCache;
    public:
        /// Constructor for creating named streams
        ZipDataStream(const String& name, ZZIP_FILE* zzipFile, size_t uncompressedSize);
        ~ZipDataStream();
        /// @copydoc DataStream::read
        size_t read(void* buf, size_t count);
        /// @copydoc DataStream::write
        size_t write(const void* buf, size_t count);
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

    /// Utility method to format out zzip errors
    static String getErrorDescription(zzip_error_t zzipError, const String& file)
    {
        const char* errorMsg = "";
        switch (zzipError)
        {
        case ZZIP_NO_ERROR:
            break;
        case ZZIP_OUTOFMEM:
            errorMsg = "Out of memory";
            break;            
        case ZZIP_DIR_OPEN:
            errorMsg = "Unable to open zip file";
            break;
        case ZZIP_DIR_STAT: 
        case ZZIP_DIR_SEEK:
        case ZZIP_DIR_READ:
            errorMsg = "Unable to read zip file";
            break;            
        case ZZIP_UNSUPP_COMPR:
            errorMsg = "Unsupported compression format";
            break;            
        case ZZIP_CORRUPTED:
            errorMsg = "Corrupted archive";
            break;
        case ZZIP_DIR_TOO_SHORT:
            errorMsg = "Zip file is too short";
            break;
        case ZZIP_DIR_EDH_MISSING:
            errorMsg = "Zip-file's central directory record missing. Is this a 7z file";
            break;
        case ZZIP_ENOENT:
            errorMsg = "File not in archive";
            break;
        default:
            errorMsg = "Unknown error";
            break;            
        };

        return StringUtil::format("%s '%s'", errorMsg, file.c_str());
    }

#ifdef _OGRE_FILESYSTEM_ARCHIVE_UNICODE
    int wopen_wrapper(const char* filename, int oflag, ...)
    {
        int utf16Length = ::MultiByteToWideChar(CP_UTF8, 0, filename, (int)strlen(filename), NULL, 0);
        if (utf16Length > 0)
        {
            std::wstring wt;
            wt.resize(utf16Length);
            if (0 != ::MultiByteToWideChar(CP_UTF8, 0, filename, (int)strlen(filename), &wt[0],
                                           (int)wt.size()))
                return _wopen(wt.c_str(), oflag);
        }

        return -1;
    }
#endif

    const zzip_plugin_io_handlers* getDefaultIO()
    {
        static zzip_plugin_io_handlers defaultIO;
        static bool isInit = false;

        if (!isInit)
        {
            zzip_init_io(&defaultIO, 1);
#ifdef _OGRE_FILESYSTEM_ARCHIVE_UNICODE
            defaultIO.fd.open = wopen_wrapper;
#endif
            isInit = true;
        }

        return &defaultIO;
    }
}
    //-----------------------------------------------------------------------
    ZipArchive::ZipArchive(const String& name, const String& archType, const zzip_plugin_io_handlers* pluginIo)
        : Archive(name, archType), mZzipDir(0), mPluginIo(pluginIo)
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
        OGRE_LOCK_AUTO_MUTEX;
        if (!mZzipDir)
        {
            zzip_error_t zzipError;
            mZzipDir = zzip_dir_open_ext_io(mName.c_str(), &zzipError, 0, mPluginIo);
            if (zzipError)
                OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, getErrorDescription(zzipError, mName));

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
#if !OGRE_RESOURCEMANAGER_STRICT
                else
                {
                    info.filename = info.basename;
                }
#endif
                mFileList.push_back(info);

            }

        }
    }
    //-----------------------------------------------------------------------
    void ZipArchive::unload()
    {
        OGRE_LOCK_AUTO_MUTEX;
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
        OGRE_LOCK_AUTO_MUTEX;
        String lookUpFileName = filename;

#if OGRE_RESOURCEMANAGER_STRICT
        const int flags = 0;
#else
        const int flags = ZZIP_CASELESS;
#endif

        // Format not used here (always binary)
        ZZIP_FILE* zzipFile =
            zzip_file_open(mZzipDir, lookUpFileName.c_str(), ZZIP_ONLYZIP | flags);

#if !OGRE_RESOURCEMANAGER_STRICT
        if (!zzipFile) // Try if we find the file
        {
            String basename, path;
            StringUtil::splitFilename(lookUpFileName, basename, path);
            const FileInfoListPtr fileNfo = findFileInfo(basename, true);
            if (fileNfo->size() == 1) // If there are more files with the same do not open anyone
            {
                Ogre::FileInfo info = fileNfo->at(0);
                lookUpFileName = info.path + info.basename;
                zzipFile = zzip_file_open(mZzipDir, lookUpFileName.c_str(), ZZIP_ONLYZIP | flags); // When an error happens here we will catch it below
            }
        }
#endif

        if (!zzipFile)
        {
            OGRE_EXCEPT(Exception::ERR_FILE_NOT_FOUND, getErrorDescription((zzip_error_t)zzip_error(mZzipDir), mName));
        }

        // Get uncompressed size too
        ZZIP_STAT zstat;
        zzip_dir_stat(mZzipDir, lookUpFileName.c_str(), &zstat, flags);

        // Construct & return stream
        return DataStreamPtr(OGRE_NEW ZipDataStream(lookUpFileName, zzipFile, static_cast<size_t>(zstat.st_size)));

    }
    //---------------------------------------------------------------------
    DataStreamPtr ZipArchive::create(const String& filename)
    {
        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, 
            "Modification of zipped archives is not supported", 
            "ZipArchive::create");

    }
    //---------------------------------------------------------------------
    void ZipArchive::remove(const String& filename)
    {
        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, 
            "Modification of zipped archives is not supported", 
            "ZipArchive::remove");
    }
    //-----------------------------------------------------------------------
    StringVectorPtr ZipArchive::list(bool recursive, bool dirs) const
    {
        OGRE_LOCK_AUTO_MUTEX;
        StringVectorPtr ret = StringVectorPtr(OGRE_NEW_T(StringVector, MEMCATEGORY_GENERAL)(), SPFM_DELETE_T);

        FileInfoList::const_iterator i, iend;
        iend = mFileList.end();
        for (i = mFileList.begin(); i != iend; ++i)
            if ((dirs == (i->compressedSize == size_t (-1))) &&
                (recursive || i->path.empty()))
                ret->push_back(i->filename);

        return ret;
    }
    //-----------------------------------------------------------------------
    FileInfoListPtr ZipArchive::listFileInfo(bool recursive, bool dirs) const
    {
        OGRE_LOCK_AUTO_MUTEX;
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
    StringVectorPtr ZipArchive::find(const String& pattern, bool recursive, bool dirs) const
    {
        OGRE_LOCK_AUTO_MUTEX;
        StringVectorPtr ret = StringVectorPtr(OGRE_NEW_T(StringVector, MEMCATEGORY_GENERAL)(), SPFM_DELETE_T);
        // If pattern contains a directory name, do a full match
        bool full_match = (pattern.find ('/') != String::npos) ||
                          (pattern.find ('\\') != String::npos);
        bool wildCard = pattern.find('*') != String::npos;
            
        FileInfoList::const_iterator i, iend;
        iend = mFileList.end();
        for (i = mFileList.begin(); i != iend; ++i)
            if ((dirs == (i->compressedSize == size_t (-1))) &&
                (recursive || full_match || wildCard))
                // Check basename matches pattern (zip is case insensitive)
                if (StringUtil::match(full_match ? i->filename : i->basename, pattern, false))
                    ret->push_back(i->filename);

        return ret;
    }
    //-----------------------------------------------------------------------
    FileInfoListPtr ZipArchive::findFileInfo(const String& pattern, 
        bool recursive, bool dirs) const
    {
        OGRE_LOCK_AUTO_MUTEX;
        FileInfoListPtr ret = FileInfoListPtr(OGRE_NEW_T(FileInfoList, MEMCATEGORY_GENERAL)(), SPFM_DELETE_T);
        // If pattern contains a directory name, do a full match
        bool full_match = (pattern.find ('/') != String::npos) ||
                          (pattern.find ('\\') != String::npos);
        bool wildCard = pattern.find('*') != String::npos;

        FileInfoList::const_iterator i, iend;
        iend = mFileList.end();
        for (i = mFileList.begin(); i != iend; ++i)
            if ((dirs == (i->compressedSize == size_t (-1))) &&
                (recursive || full_match || wildCard))
                // Check name matches pattern (zip is case insensitive)
                if (StringUtil::match(full_match ? i->filename : i->basename, pattern, false))
                    ret->push_back(*i);

        return ret;
    }
    //-----------------------------------------------------------------------
    bool ZipArchive::exists(const String& filename) const
    {       
        OGRE_LOCK_AUTO_MUTEX;
        String cleanName = filename;
#if !OGRE_RESOURCEMANAGER_STRICT
        if(filename.rfind('/') != String::npos)
        {
            StringVector tokens = StringUtil::split(filename, "/");
            cleanName = tokens[tokens.size() - 1];
        }
#endif

        return std::find_if(mFileList.begin(), mFileList.end(), [&cleanName](const Ogre::FileInfo& fi) {
                   return fi.filename == cleanName;
               }) != mFileList.end();
    }
    //---------------------------------------------------------------------
    time_t ZipArchive::getModifiedTime(const String& filename) const
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
    size_t ZipDataStream::write(const void* buf, size_t count)
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
            skip((long)(newPos - prevPos));
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
        mAccess = 0;
        if (mZzipFile != 0)
        {
            zzip_file_close(mZzipFile);
            mZzipFile = 0;
        }
        mCache.clear();
    }
    //-----------------------------------------------------------------------
    //  ZipArchiveFactory
    //-----------------------------------------------------------------------
    Archive *ZipArchiveFactory::createInstance( const String& name, bool readOnly )
    {
        if(!readOnly)
            return NULL;

        return OGRE_NEW ZipArchive(name, getType(), getDefaultIO());
    }
    //-----------------------------------------------------------------------
    const String& ZipArchiveFactory::getType(void) const
    {
        static String name = "Zip";
        return name;
    }
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //  EmbeddedZipArchiveFactory
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    /// a struct to hold embedded file data
    struct EmbeddedFileData
    {
        const uint8 * fileData;
        zzip_size_t fileSize;
        zzip_size_t curPos;
        bool isFileOpened;
        EmbeddedZipArchiveFactory::DecryptEmbeddedZipFileFunc decryptFunc;
    };
    //-----------------------------------------------------------------------
    /// A type for a map between the file names to file index
    typedef std::map<String, int> FileNameToIndexMap;
    typedef FileNameToIndexMap::iterator FileNameToIndexMapIter;
    /// A type to store the embedded files data
    typedef std::vector<EmbeddedFileData> EmbbedFileDataList;

    namespace {
    /// A static map between the file names to file index
    FileNameToIndexMap * EmbeddedZipArchiveFactory_mFileNameToIndexMap;
    /// A static list to store the embedded files data
    EmbbedFileDataList * EmbeddedZipArchiveFactory_mEmbbedFileDataList;
    #define EMBED_IO_BAD_FILE_HANDLE (-1)
    #define EMBED_IO_SUCCESS (0)
    //-----------------------------------------------------------------------
    /// functions for embedded zzip_plugin_io_handlers implementation 
    /// The functions are here and not as static members because they 
    /// use types that I don't want to define in the header like zzip_char_t,
    //  zzip_ssize_t and such.
    //-----------------------------------------------------------------------
    // get file date by index
    EmbeddedFileData & getEmbeddedFileDataByIndex(int fd)
    {
        return (*EmbeddedZipArchiveFactory_mEmbbedFileDataList)[fd-1];
    }
    //-----------------------------------------------------------------------
    // opens the file
    int EmbeddedZipArchiveFactory_open(zzip_char_t* name, int flags, ...)
    {
        String nameAsString = name;
        FileNameToIndexMapIter foundIter = EmbeddedZipArchiveFactory_mFileNameToIndexMap->find(nameAsString);
        if (foundIter != EmbeddedZipArchiveFactory_mFileNameToIndexMap->end())
        {
            int fd = foundIter->second;
            EmbeddedFileData & curEmbeddedFileData = getEmbeddedFileDataByIndex(fd);
            if(curEmbeddedFileData.isFileOpened)
            {
               // file is opened - return an error handle
               return EMBED_IO_BAD_FILE_HANDLE;
            }
            
            curEmbeddedFileData.isFileOpened = true;
            return fd;
        }
        else
        {
           // not found - return an error handle
           return EMBED_IO_BAD_FILE_HANDLE;
        }
    }
    //-----------------------------------------------------------------------
    // Closes a file.
    // Return Value - On success, close returns 0. 
    int EmbeddedZipArchiveFactory_close(int fd)
    {
        if (fd == EMBED_IO_BAD_FILE_HANDLE)
        {
            // bad index - return an error
            return -1;
        }

        EmbeddedFileData & curEmbeddedFileData = getEmbeddedFileDataByIndex(fd);

        if(curEmbeddedFileData.isFileOpened == false)
        {
           // file is opened - return an error handle
           return -1;
        }
        else
        {
            // success
            curEmbeddedFileData.isFileOpened = false;
            curEmbeddedFileData.curPos = 0;
            return 0;
        }

    }
       
    //-----------------------------------------------------------------------
    // reads data from the file
    zzip_ssize_t EmbeddedZipArchiveFactory_read(int fd, void* buf, zzip_size_t len)
    {
        if (fd == EMBED_IO_BAD_FILE_HANDLE)
        {
            // bad index - return an error size - negative
            return -1;
        }
        // get the current buffer in file;
        EmbeddedFileData & curEmbeddedFileData = getEmbeddedFileDataByIndex(fd);
        const uint8 * curFileData = curEmbeddedFileData.fileData;
        if (len + curEmbeddedFileData.curPos > curEmbeddedFileData.fileSize)
        {
            len = curEmbeddedFileData.fileSize - curEmbeddedFileData.curPos;
        }
        curFileData += curEmbeddedFileData.curPos;
        
        // copy to out buffer
        memcpy(buf, curFileData, len);

        if( curEmbeddedFileData.decryptFunc != NULL )
        {
            if (!curEmbeddedFileData.decryptFunc(curEmbeddedFileData.curPos, buf, len))
            {
                // decrypt failed - return an error size - negative
                return -1;
            }
        }

        // move the cursor to the new pos
        curEmbeddedFileData.curPos += len;
        
        return len;
    }
    //-----------------------------------------------------------------------
    // Moves file pointer.
    zzip_off_t EmbeddedZipArchiveFactory_seeks(int fd, zzip_off_t offset, int whence)
    {
        if (fd == EMBED_IO_BAD_FILE_HANDLE)
        {
            // bad index - return an error - nonzero value.
            return -1;
        }
        
        zzip_size_t newPos = -1;
        // get the current buffer in file;
        EmbeddedFileData & curEmbeddedFileData = getEmbeddedFileDataByIndex(fd);
        switch(whence)
        {
            case SEEK_CUR:
                newPos = (zzip_size_t)(curEmbeddedFileData.curPos + offset);
                break;
            case SEEK_END:
                newPos = (zzip_size_t)(curEmbeddedFileData.fileSize - offset);
                break;
            case SEEK_SET:
                newPos = (zzip_size_t)offset;
                break;
            default:
                // bad whence - return an error - nonzero value.
                return -1;
                break;
        };
        if (newPos >= curEmbeddedFileData.fileSize)
        {
            // bad whence - return an error - nonzero value.
            return -1;
        }

        curEmbeddedFileData.curPos = newPos;
        return newPos;
    }
    //-----------------------------------------------------------------------
    // returns the file size
    zzip_off_t EmbeddedZipArchiveFactory_filesize(int fd)
    {
        if (fd == EMBED_IO_BAD_FILE_HANDLE)
        {
            // bad index - return an error - nonzero value.
            return -1;
        }
                // get the current buffer in file;
        EmbeddedFileData & curEmbeddedFileData = getEmbeddedFileDataByIndex(fd);
        return curEmbeddedFileData.fileSize;
    }
    //-----------------------------------------------------------------------
    // writes data to the file
    zzip_ssize_t EmbeddedZipArchiveFactory_write(int fd, _zzip_const void* buf, zzip_size_t len)
    {
        // the files in this case are read only - return an error  - nonzero value.
        return -1;
    }

    /// A static pointer to file io alternative implementation for the embedded files
    const zzip_plugin_io_handlers* getEmbeddedZipIO()
    {
        static zzip_plugin_io_handlers embeddedZipIO = {
            {EmbeddedZipArchiveFactory_open, EmbeddedZipArchiveFactory_close,
             EmbeddedZipArchiveFactory_read, EmbeddedZipArchiveFactory_seeks,
             EmbeddedZipArchiveFactory_filesize, 1, 1, EmbeddedZipArchiveFactory_write}};
        return &embeddedZipIO;
    }

    } // namespace {
    //-----------------------------------------------------------------------
    EmbeddedZipArchiveFactory::EmbeddedZipArchiveFactory() {}
    EmbeddedZipArchiveFactory::~EmbeddedZipArchiveFactory() {}
    //-----------------------------------------------------------------------
    Archive *EmbeddedZipArchiveFactory::createInstance( const String& name, bool readOnly )
    {
        ZipArchive * resZipArchive = OGRE_NEW ZipArchive(name, getType(), getEmbeddedZipIO());
        return resZipArchive;
    }
    void EmbeddedZipArchiveFactory::destroyInstance(Archive* ptr)
    {
        removeEmbbeddedFile(ptr->getName());
        ZipArchiveFactory::destroyInstance(ptr);
    }
    //-----------------------------------------------------------------------
    const String& EmbeddedZipArchiveFactory::getType(void) const
    {
        static String name = "EmbeddedZip";
        return name;
    }
    //-----------------------------------------------------------------------
    void EmbeddedZipArchiveFactory::addEmbbeddedFile(const String& name, const uint8 * fileData, 
                                        size_t fileSize, DecryptEmbeddedZipFileFunc decryptFunc)
    {
        static bool needToInit = true;
        if(needToInit)
        {
            needToInit = false;

            // we can't be sure when global variables get initialized
            // meaning it is possible our list has not been init when this
            // function is being called. The solution is to use local
            // static members in this function an init the pointers for the
            // global here. We know for use that the local static variables
            // are create in this stage.
            static FileNameToIndexMap sFileNameToIndexMap;
            static EmbbedFileDataList sEmbbedFileDataList;
            EmbeddedZipArchiveFactory_mFileNameToIndexMap = &sFileNameToIndexMap;
            EmbeddedZipArchiveFactory_mEmbbedFileDataList = &sEmbbedFileDataList;
        }

        EmbeddedFileData newEmbeddedFileData;
        newEmbeddedFileData.curPos = 0;
        newEmbeddedFileData.isFileOpened = false;
        newEmbeddedFileData.fileData = fileData;
        newEmbeddedFileData.fileSize = fileSize;
        newEmbeddedFileData.decryptFunc = decryptFunc;
        EmbeddedZipArchiveFactory_mEmbbedFileDataList->push_back(newEmbeddedFileData);
        (*EmbeddedZipArchiveFactory_mFileNameToIndexMap)[name] = static_cast<int>(EmbeddedZipArchiveFactory_mEmbbedFileDataList->size());
    }
    //-----------------------------------------------------------------------
    void EmbeddedZipArchiveFactory::removeEmbbeddedFile( const String& name )
    {
        EmbeddedZipArchiveFactory_mFileNameToIndexMap->erase(name);
    }
}

#endif
