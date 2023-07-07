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
#include <zip.h>

namespace Ogre {
namespace {
    class ZipArchive : public Archive
    {
    protected:
        /// Handle to root zip file
        zip_t* mZipFile;
        MemoryDataStreamPtr mBuffer;
        /// File list (since zziplib seems to only allow scanning of dir tree once)
        FileInfoList mFileList;
        OGRE_AUTO_MUTEX;
    public:
        ZipArchive(const String& name, const String& archType, const uint8* externBuf = 0, size_t externBufSz = 0);
        ~ZipArchive();
        /// @copydoc Archive::isCaseSensitive
        bool isCaseSensitive(void) const override { return OGRE_RESOURCEMANAGER_STRICT != 0; }

        /// @copydoc Archive::load
        void load() override;
        /// @copydoc Archive::unload
        void unload() final override;

        /// @copydoc Archive::open
        DataStreamPtr open(const String& filename, bool readOnly = true) const override;

        /// @copydoc Archive::create
        DataStreamPtr create(const String& filename) override;

        /// @copydoc Archive::remove
        void remove(const String& filename) override;

        /// @copydoc Archive::list
        StringVectorPtr list(bool recursive = true, bool dirs = false) const override;

        /// @copydoc Archive::listFileInfo
        FileInfoListPtr listFileInfo(bool recursive = true, bool dirs = false) const override;

        /// @copydoc Archive::find
        StringVectorPtr find(const String& pattern, bool recursive = true,
            bool dirs = false) const override;

        /// @copydoc Archive::findFileInfo
        FileInfoListPtr findFileInfo(const String& pattern, bool recursive = true,
            bool dirs = false) const override;

        /// @copydoc Archive::exists
        bool exists(const String& filename) const override;

        /// @copydoc Archive::getModifiedTime
        time_t getModifiedTime(const String& filename) const override;
    };
}
    //-----------------------------------------------------------------------
    ZipArchive::ZipArchive(const String& name, const String& archType, const uint8* externBuf, size_t externBufSz)
        : Archive(name, archType), mZipFile(0)
    {
        if(externBuf)
            mBuffer.reset(new MemoryDataStream(const_cast<uint8*>(externBuf), externBufSz));
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
        if (!mZipFile)
        {
            if(!mBuffer)
                mBuffer.reset(new MemoryDataStream(_openFileStream(mName, std::ios::binary)));

            mZipFile = zip_stream_open((const char*)mBuffer->getPtr(), mBuffer->size(), 0, 'r');

            // Cache names
            int n = zip_entries_total(mZipFile);
            for (int i = 0; i < n; ++i) {
                FileInfo info;
                info.archive = this;

                zip_entry_openbyindex(mZipFile, i);

                info.filename = zip_entry_name(mZipFile);
                // Get basename / path
                StringUtil::splitFilename(info.filename, info.basename, info.path);

                // Get sizes
                info.uncompressedSize = zip_entry_size(mZipFile);
                info.compressedSize = zip_entry_comp_size(mZipFile);

                if (zip_entry_isdir(mZipFile))
                {
                    info.filename = info.filename.substr(0, info.filename.length() - 1);
                    StringUtil::splitFilename(info.filename, info.basename, info.path);
                    // Set compressed size to -1 for folders; anyway nobody will check
                    // the compressed size of a folder, and if he does, its useless anyway
                    info.compressedSize = size_t(-1);
                }
#if !OGRE_RESOURCEMANAGER_STRICT
                else
                {
                    info.filename = info.basename;
                }
#endif
                zip_entry_close(mZipFile);
                mFileList.push_back(info);
            }
        }
    }
    //-----------------------------------------------------------------------
    void ZipArchive::unload()
    {
        OGRE_LOCK_AUTO_MUTEX;
        if (mZipFile)
        {
            zip_close(mZipFile);
            mZipFile = 0;
            mFileList.clear();
            mBuffer.reset();
        }
    
    }
    //-----------------------------------------------------------------------
    DataStreamPtr ZipArchive::open(const String& filename, bool readOnly) const
    {
        // zip is not threadsafe
        OGRE_LOCK_AUTO_MUTEX;
        String lookUpFileName = filename;

        bool open = zip_entry_open(mZipFile, lookUpFileName.c_str(), OGRE_RESOURCEMANAGER_STRICT) == 0;
#if !OGRE_RESOURCEMANAGER_STRICT
        if (!open) // Try if we find the file
        {
            String basename, path;
            StringUtil::splitFilename(lookUpFileName, basename, path);
            const FileInfoListPtr fileNfo = findFileInfo(basename, true);
            if (fileNfo->size() == 1) // If there are more files with the same do not open anyone
            {
                Ogre::FileInfo info = fileNfo->at(0);
                lookUpFileName = info.path + info.basename;
                open = zip_entry_open(mZipFile, lookUpFileName.c_str(), OGRE_RESOURCEMANAGER_STRICT) == 0;
            }
        }
#endif

        if (!open)
        {
            OGRE_EXCEPT(Exception::ERR_FILE_NOT_FOUND, "could not open "+lookUpFileName);
        }

        // Construct & return stream
        auto ret = std::make_shared<MemoryDataStream>(lookUpFileName, zip_entry_size(mZipFile));

        if(zip_entry_noallocread(mZipFile, ret->getPtr(), ret->size()) < 0)
            OGRE_EXCEPT(Exception::ERR_FILE_NOT_FOUND, "could not read "+lookUpFileName);
        zip_entry_close(mZipFile);

        return ret;
    }
    //---------------------------------------------------------------------
    DataStreamPtr ZipArchive::create(const String& filename)
    {
        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Modification of zipped archives is not implemented");
    }
    //---------------------------------------------------------------------
    void ZipArchive::remove(const String& filename)
    {
        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Modification of zipped archives is not implemented");
    }
    //-----------------------------------------------------------------------
    StringVectorPtr ZipArchive::list(bool recursive, bool dirs) const
    {
        OGRE_LOCK_AUTO_MUTEX;
        auto ret = std::make_shared<StringVector>();

        for (auto& f : mFileList)
            if ((dirs == (f.compressedSize == size_t (-1))) &&
                (recursive || f.path.empty()))
                ret->push_back(f.filename);

        return ret;
    }
    //-----------------------------------------------------------------------
    FileInfoListPtr ZipArchive::listFileInfo(bool recursive, bool dirs) const
    {
        OGRE_LOCK_AUTO_MUTEX;
        auto ret = std::make_shared<FileInfoList>();
        for (auto& f : mFileList)
            if ((dirs == (f.compressedSize == size_t (-1))) &&
                (recursive || f.path.empty()))
                ret->push_back(f);

        return ret;
    }
    //-----------------------------------------------------------------------
    StringVectorPtr ZipArchive::find(const String& pattern, bool recursive, bool dirs) const
    {
        OGRE_LOCK_AUTO_MUTEX;
        auto ret = std::make_shared<StringVector>();
        // If pattern contains a directory name, do a full match
        bool full_match = (pattern.find ('/') != String::npos) ||
                          (pattern.find ('\\') != String::npos);
        bool wildCard = pattern.find('*') != String::npos;
            
        for (auto& f : mFileList)
            if ((dirs == (f.compressedSize == size_t (-1))) &&
                (recursive || full_match || wildCard))
                // Check basename matches pattern (zip is case insensitive)
                if (StringUtil::match(full_match ? f.filename : f.basename, pattern, false))
                    ret->push_back(f.filename);

        return ret;
    }
    //-----------------------------------------------------------------------
    FileInfoListPtr ZipArchive::findFileInfo(const String& pattern, 
        bool recursive, bool dirs) const
    {
        OGRE_LOCK_AUTO_MUTEX;
        auto ret = std::make_shared<FileInfoList>();
        // If pattern contains a directory name, do a full match
        bool full_match = (pattern.find ('/') != String::npos) ||
                          (pattern.find ('\\') != String::npos);
        bool wildCard = pattern.find('*') != String::npos;

        for (auto& f : mFileList)
            if ((dirs == (f.compressedSize == size_t (-1))) &&
                (recursive || full_match || wildCard))
                // Check name matches pattern (zip is case insensitive)
                if (StringUtil::match(full_match ? f.filename : f.basename, pattern, false))
                    ret->push_back(f);

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
    //  ZipArchiveFactory
    //-----------------------------------------------------------------------
    Archive *ZipArchiveFactory::createInstance( const String& name, bool readOnly )
    {
        if(!readOnly)
            return NULL;

        return OGRE_NEW ZipArchive(name, getType());
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
        size_t fileSize;
        size_t curPos;
        bool isFileOpened;
        EmbeddedZipArchiveFactory::DecryptEmbeddedZipFileFunc decryptFunc;
    };
    //-----------------------------------------------------------------------
    /// A type for a map between the file names to file index
    typedef std::map<String, EmbeddedFileData> EmbbedFileDataList;

    namespace {
    /// A static list to store the embedded files data
    EmbbedFileDataList *gEmbeddedFileDataList;
    } // namespace {
    //-----------------------------------------------------------------------
    EmbeddedZipArchiveFactory::EmbeddedZipArchiveFactory() {}
    EmbeddedZipArchiveFactory::~EmbeddedZipArchiveFactory() {}
    //-----------------------------------------------------------------------
    Archive *EmbeddedZipArchiveFactory::createInstance( const String& name, bool readOnly )
    {
        auto it = gEmbeddedFileDataList->find(name);
        if(it == gEmbeddedFileDataList->end())
            return NULL;

        // TODO: decryptFunc

        return new ZipArchive(name, getType(), it->second.fileData, it->second.fileSize);
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
            static EmbbedFileDataList sEmbbedFileDataList;
            gEmbeddedFileDataList = &sEmbbedFileDataList;
        }

        EmbeddedFileData newEmbeddedFileData;
        newEmbeddedFileData.curPos = 0;
        newEmbeddedFileData.isFileOpened = false;
        newEmbeddedFileData.fileData = fileData;
        newEmbeddedFileData.fileSize = fileSize;
        newEmbeddedFileData.decryptFunc = decryptFunc;
        gEmbeddedFileDataList->emplace(name, newEmbeddedFileData);
    }
    //-----------------------------------------------------------------------
    void EmbeddedZipArchiveFactory::removeEmbbeddedFile( const String& name )
    {
        gEmbeddedFileDataList->erase(name);
    }
}

#endif
