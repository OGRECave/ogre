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
#ifndef _Archive_H__
#define _Archive_H__

#include "OgrePrerequisites.h"
#include "OgreString.h"
#include "OgreDataStream.h"
#include "OgreSharedPtr.h"
#include "OgreStringVector.h"
#include <ctime>

namespace Ogre {

    /** Information about a file/directory within the archive will be
    returned using a FileInfo struct.
    @see
    Archive
    */
    struct FileInfo {
		/// The archive in which the file has been found (for info when performing
		/// multi-Archive searches, note you should still open through ResourceGroupManager)
		Archive* archive;
        /// The file's fully qualified name
        String filename;
        /// Path name; separated by '/' and ending with '/'
        String path;
        /// Base filename
        String basename;
        /// Compressed size
        size_t compressedSize;
        /// Uncompressed size
        size_t uncompressedSize;
    };

    typedef vector<FileInfo>::type FileInfoList;
    typedef SharedPtr<FileInfoList> FileInfoListPtr;

    /** Archive-handling class.
    @remarks
        An archive is a generic term for a container of files. This may be a
        filesystem folder, it may be a compressed archive, it may even be 
        a remote location shared on the web. This class is designed to be 
        subclassed to provide access to a range of file locations. 
    @par
        Instances of this class are never constructed or even handled by end-user
        applications. They are constructed by custom ArchiveFactory classes, 
        which plugins can register new instances of using ArchiveManager. 
        End-user applications will typically use ResourceManager or 
        ResourceGroupManager to manage resources at a higher level, rather than 
        reading files directly through this class. Doing it this way allows you
        to benefit from OGRE's automatic searching of multiple file locations 
        for the resources you are looking for.
    */
	class _OgreExport Archive : public ArchiveAlloc
    {
    protected:
        /// Archive name
        String mName; 
        /// Archive type code
        String mType;
		/// Read-only flag
		bool mReadOnly;
    public:


        /** Constructor - don't call direct, used by ArchiveFactory.
        */
        Archive( const String& name, const String& archType )
            : mName(name), mType(archType), mReadOnly(true) {}

        /** Default destructor.
        */
        virtual ~Archive() {}

		/// Get the name of this archive
		const String& getName(void) const { return mName; }

        /// Returns whether this archive is case sensitive in the way it matches files
        virtual bool isCaseSensitive(void) const = 0;

        /** Loads the archive.
        @remarks
            This initializes all the internal data of the class.
        @warning
            Do not call this function directly, it is meant to be used
            only by the ArchiveManager class.
        */
        virtual void load() = 0;

        /** Unloads the archive.
        @warning
            Do not call this function directly, it is meant to be used
            only by the ArchiveManager class.
        */
        virtual void unload() = 0;

		/** Reports whether this Archive is read-only, or whether the contents
			can be updated. 
		*/
		virtual bool isReadOnly() const { return mReadOnly; }

        /** Open a stream on a given file. 
        @note
            There is no equivalent 'close' method; the returned stream
            controls the lifecycle of this file operation.
        @param filename The fully qualified name of the file
		@param readOnly Whether to open the file in read-only mode or not (note, 
			if the archive is read-only then this cannot be set to false)
        @returns A shared pointer to a DataStream which can be used to 
            read / write the file. If the file is not present, returns a null
			shared pointer.
        */
        virtual DataStreamPtr open(const String& filename, bool readOnly = true) const = 0;

		/** Create a new file (or overwrite one already there). 
		@note If the archive is read-only then this method will fail.
		@param filename The fully qualified name of the file
		@returns A shared pointer to a DataStream which can be used to 
		read / write the file. 
		*/
		virtual DataStreamPtr create(const String& filename) const = 0;

		/** Delete a named file.
		@remarks Not possible on read-only archives
		@param filename The fully qualified name of the file
		*/
		virtual void remove(const String& filename) const = 0;


        /** List all file names in the archive.
        @note
            This method only returns filenames, you can also retrieve other
            information using listFileInfo.
        @param recursive Whether all paths of the archive are searched (if the 
            archive has a concept of that)
        @param dirs Set to true if you want the directories to be listed
            instead of files
        @returns A list of filenames matching the criteria, all are fully qualified
        */
        virtual StringVectorPtr list(bool recursive = true, bool dirs = false) = 0;
        
        /** List all files in the archive with accompanying information.
        @param recursive Whether all paths of the archive are searched (if the 
            archive has a concept of that)
        @param dirs Set to true if you want the directories to be listed
            instead of files
        @returns A list of structures detailing quite a lot of information about
            all the files in the archive.
        */
        virtual FileInfoListPtr listFileInfo(bool recursive = true, bool dirs = false) = 0;

        /** Find all file or directory names matching a given pattern
            in this archive.
        @note
            This method only returns filenames, you can also retrieve other
            information using findFileInfo.
        @param pattern The pattern to search for; wildcards (*) are allowed
        @param recursive Whether all paths of the archive are searched (if the 
            archive has a concept of that)
        @param dirs Set to true if you want the directories to be listed
            instead of files
        @returns A list of filenames matching the criteria, all are fully qualified
        */
        virtual StringVectorPtr find(const String& pattern, bool recursive = true,
            bool dirs = false) = 0;

        /** Find out if the named file exists (note: fully qualified filename required) */
        virtual bool exists(const String& filename) = 0; 

		/** Retrieve the modification time of a given file */
		virtual time_t getModifiedTime(const String& filename) = 0; 


        /** Find all files or directories matching a given pattern in this
            archive and get some detailed information about them.
        @param pattern The pattern to search for; wildcards (*) are allowed
        @param recursive Whether all paths of the archive are searched (if the 
        archive has a concept of that)
        @param dirs Set to true if you want the directories to be listed
            instead of files
        @returns A list of file information structures for all files matching 
            the criteria.
        */
        virtual FileInfoListPtr findFileInfo(const String& pattern, 
            bool recursive = true, bool dirs = false) = 0;

        /// Return the type code of this Archive
        const String& getType(void) const { return mType; }
        
    };
}

#endif
