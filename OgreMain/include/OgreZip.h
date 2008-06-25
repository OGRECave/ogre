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
#ifndef __Zip_H__
#define __Zip_H__

#include "OgrePrerequisites.h"

#include "OgreArchive.h"
#include "OgreArchiveFactory.h"

// Forward declaration for zziplib to avoid header file dependency.
typedef struct zzip_dir		ZZIP_DIR;
typedef struct zzip_file	ZZIP_FILE;

namespace Ogre {

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
        DataStreamPtr open(const String& filename) const;

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

    /** Specialisation of DataStream to handle streaming data from zip archives. */
    class _OgrePrivate ZipDataStream : public DataStream
    {
    protected:
        ZZIP_FILE* mZzipFile;
    public:
        /// Unnamed constructor
        ZipDataStream(ZZIP_FILE* zzipFile, size_t uncompressedSize);
        /// Constructor for creating named streams
        ZipDataStream(const String& name, ZZIP_FILE* zzipFile, size_t uncompressedSize);
		~ZipDataStream();
        /// @copydoc DataStream::read
        size_t read(void* buf, size_t count);
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


}

#endif
