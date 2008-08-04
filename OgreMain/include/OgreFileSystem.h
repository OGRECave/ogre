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
#ifndef __FileSystem_H__
#define __FileSystem_H__

#include "OgrePrerequisites.h"

#include "OgreArchive.h"
#include "OgreArchiveFactory.h"

namespace Ogre {

    /** Specialisation of the Archive class to allow reading of files from 
        filesystem folders / directories.
    */
    class _OgreExport FileSystemArchive : public Archive 
    {
    protected:
        /** Utility method to retrieve all files in a directory matching pattern.
        @param pattern File pattern
        @param recursive Whether to cascade down directories
        @param dirs Set to true if you want the directories to be listed
            instead of files
        @param simpleList Populated if retrieving a simple list
        @param detailList Populated if retrieving a detailed list
        @param currentDir The current directory relative to the base of the 
            archive, for file naming
        */
        void findFiles(const String& pattern, bool recursive, bool dirs,
            StringVector* simpleList, FileInfoList* detailList);

    public:
        FileSystemArchive(const String& name, const String& archType );
        ~FileSystemArchive();

        /// @copydoc Archive::isCaseSensitive
        bool isCaseSensitive(void) const;

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

		/// @copydoc Archive::getModifiedTime
		time_t getModifiedTime(const String& filename);

		/// Set whether filesystem enumeration will include hidden files or not.
		/// This should be called prior to declaring and/or initializing filesystem
		/// resource locations. The default is true (ignore hidden files).
		static void setIgnoreHidden(bool ignore)
		{
			ms_IgnoreHidden = ignore;
		}

		/// Get whether hidden files are ignored during filesystem enumeration.
		static bool getIgnoreHidden()
		{
			return ms_IgnoreHidden;
		}

		static bool ms_IgnoreHidden;
    };

    /** Specialisation of ArchiveFactory for FileSystem files. */
    //class _OgrePrivate FileSystemArchiveFactory : public ArchiveFactory
    class _OgreExport FileSystemArchiveFactory : public ArchiveFactory
    {
    public:
        virtual ~FileSystemArchiveFactory() {}
        /// @copydoc FactoryObj::getType
        const String& getType(void) const;
        /// @copydoc FactoryObj::createInstance
        Archive *createInstance( const String& name ) 
        {
            return OGRE_NEW FileSystemArchive(name, "FileSystem");
        }
        /// @copydoc FactoryObj::destroyInstance
        void destroyInstance( Archive* arch) { delete arch; }
    };


}

#endif
