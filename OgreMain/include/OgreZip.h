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
#ifndef __Zip_H__
#define __Zip_H__

#include "OgrePrerequisites.h"

#include "OgreArchive.h"
#include "OgreArchiveFactory.h"
#include "OgreHeaderPrefix.h"
#include "Threading/OgreThreadHeaders.h"

// Forward declaration for zziplib to avoid header file dependency.
typedef struct zzip_dir     ZZIP_DIR;
typedef struct zzip_file    ZZIP_FILE;
typedef union _zzip_plugin_io zzip_plugin_io_handlers;

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
        /// A pointer to file io alternative implementation 
        zzip_plugin_io_handlers* mPluginIo;

        OGRE_AUTO_MUTEX;
    public:
        ZipArchive(const String& name, const String& archType, zzip_plugin_io_handlers* pluginIo = NULL);
        ~ZipArchive();
        /// @copydoc Archive::isCaseSensitive
        bool isCaseSensitive(void) const { return false; }

        /// @copydoc Archive::load
        void load();
        /// @copydoc Archive::unload
        void unload();

        /// @copydoc Archive::open
        DataStreamPtr open(const String& filename, bool readOnly = true);

        /// @copydoc Archive::create
        DataStreamPtr create(const String& filename);

        /// @copydoc Archive::remove
        void remove(const String& filename);

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
    class _OgreExport ZipArchiveFactory : public ArchiveFactory
    {
    public:
        virtual ~ZipArchiveFactory() {}
        /// @copydoc FactoryObj::getType
        const String& getType(void) const;
        /// @copydoc FactoryObj::createInstance
        Archive *createInstance( const String& name, bool readOnly ) 
        {
            if(!readOnly)
                return NULL;

            return OGRE_NEW ZipArchive(name, "Zip");
        }
        /// @copydoc FactoryObj::destroyInstance
        void destroyInstance( Archive* ptr) { OGRE_DELETE ptr; }
    };

    /** Specialisation of ZipArchiveFactory for embedded Zip files. */
    class _OgreExport EmbeddedZipArchiveFactory : public ZipArchiveFactory
    {
    protected:
        /// A static pointer to file io alternative implementation for the embedded files
        static zzip_plugin_io_handlers* mPluginIo; 
    public:
        EmbeddedZipArchiveFactory();
        virtual ~EmbeddedZipArchiveFactory();
        /// @copydoc FactoryObj::getType
        const String& getType(void) const;
        /// @copydoc FactoryObj::createInstance
        Archive *createInstance( const String& name, bool readOnly ) 
        {
            ZipArchive * resZipArchive = OGRE_NEW ZipArchive(name, "EmbeddedZip", mPluginIo);
            return resZipArchive;
        }
        
        /** a function type to decrypt embedded zip file
        @param pos pos in file
        @param buf current buffer to decrypt
        @param len - length of buffer
        @return success
        */  
        typedef bool (*DecryptEmbeddedZipFileFunc)(size_t pos, void* buf, size_t len);

        /// Add an embedded file to the embedded file list
        static void addEmbbeddedFile(const String& name, const uint8 * fileData, 
                        size_t fileSize, DecryptEmbeddedZipFileFunc decryptFunc);

        /// Remove an embedded file to the embedded file list
        static void removeEmbbeddedFile(const String& name);

    };

    /** Specialisation of DataStream to handle streaming data from zip archives. */
    class _OgreExport ZipDataStream : public DataStream
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

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
