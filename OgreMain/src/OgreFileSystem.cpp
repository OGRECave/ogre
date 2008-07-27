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
#include "OgreFileSystem.h"
#include "OgreLogManager.h"
#include "OgreException.h"
#include "OgreStringVector.h"
#include "OgreRoot.h"

#include <sys/types.h>
#include <sys/stat.h>

#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX || OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#   include "OgreSearchOps.h"
#   include <sys/param.h>
#   define MAX_PATH MAXPATHLEN
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#   include <windows.h>
#   include <direct.h>
#   include <io.h>
#endif

namespace Ogre {

	bool FileSystemArchive::ms_IgnoreHidden = true;

    //-----------------------------------------------------------------------
    FileSystemArchive::FileSystemArchive(const String& name, const String& archType )
        : Archive(name, archType)
    {
    }
    //-----------------------------------------------------------------------
    bool FileSystemArchive::isCaseSensitive(void) const
    {
        #if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
            return false;
        #else
            return true;
        #endif

    }
    //-----------------------------------------------------------------------
    static bool is_reserved_dir (const char *fn)
    {
        return (fn [0] == '.' && (fn [1] == 0 || (fn [1] == '.' && fn [2] == 0)));
    }
    //-----------------------------------------------------------------------
    static bool is_absolute_path(const char* path)
    {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        if (isalpha(uchar(path[0])) && path[1] == ':')
            return true;
#endif
        return path[0] == '/' || path[0] == '\\';
    }
    //-----------------------------------------------------------------------
    static String concatenate_path(const String& base, const String& name)
    {
        if (base.empty() || is_absolute_path(name.c_str()))
            return name;
        else
            return base + '/' + name;
    }
    //-----------------------------------------------------------------------
    void FileSystemArchive::findFiles(const String& pattern, bool recursive, 
        bool dirs, StringVector* simpleList, FileInfoList* detailList)
    {
        long lHandle, res;
        struct _finddata_t tagData;

        // pattern can contain a directory name, separate it from mask
        size_t pos1 = pattern.rfind ('/');
        size_t pos2 = pattern.rfind ('\\');
        if (pos1 == pattern.npos || ((pos2 != pattern.npos) && (pos1 < pos2)))
            pos1 = pos2;
        String directory;
        if (pos1 != pattern.npos)
            directory = pattern.substr (0, pos1 + 1);

        String full_pattern = concatenate_path(mName, pattern);

        lHandle = _findfirst(full_pattern.c_str(), &tagData);
        res = 0;
        while (lHandle != -1 && res != -1)
        {
            if ((dirs == ((tagData.attrib & _A_SUBDIR) != 0)) &&
				( !ms_IgnoreHidden || (tagData.attrib & _A_HIDDEN) == 0 ) &&
                (!dirs || !is_reserved_dir (tagData.name)))
            {
                if (simpleList)
                {
                    simpleList->push_back(directory + tagData.name);
                }
                else if (detailList)
                {
                    FileInfo fi;
                    fi.archive = this;
                    fi.filename = directory + tagData.name;
                    fi.basename = tagData.name;
                    fi.path = directory;
                    fi.compressedSize = tagData.size;
                    fi.uncompressedSize = tagData.size;
                    detailList->push_back(fi);
                }
            }
            res = _findnext( lHandle, &tagData );
        }
        // Close if we found any files
        if(lHandle != -1)
            _findclose(lHandle);

        // Now find directories
        if (recursive)
        {
            String base_dir = mName;
            if (!directory.empty ())
            {
                base_dir = concatenate_path(mName, directory);
                // Remove the last '/'
                base_dir.erase (base_dir.length () - 1);
            }
            base_dir.append ("/*");

            // Remove directory name from pattern
            String mask ("/");
            if (pos1 != pattern.npos)
                mask.append (pattern.substr (pos1 + 1));
            else
                mask.append (pattern);

            lHandle = _findfirst(base_dir.c_str (), &tagData);
            res = 0;
            while (lHandle != -1 && res != -1)
            {
                if ((tagData.attrib & _A_SUBDIR) &&
					( !ms_IgnoreHidden || (tagData.attrib & _A_HIDDEN) == 0 ) &&
                    !is_reserved_dir (tagData.name))
                {
                    // recurse
                    base_dir = directory;
                    base_dir.append (tagData.name).append (mask);
                    findFiles(base_dir, recursive, dirs, simpleList, detailList);
                }
                res = _findnext( lHandle, &tagData );
            }
            // Close if we found any files
            if(lHandle != -1)
                _findclose(lHandle);
        }
    }
    //-----------------------------------------------------------------------
    FileSystemArchive::~FileSystemArchive()
    {
        unload();
    }
    //-----------------------------------------------------------------------
    void FileSystemArchive::load()
    {
        // do nothing here, what has to be said will be said later
    }
    //-----------------------------------------------------------------------
    void FileSystemArchive::unload()
    {
        // nothing to see here, move along
    }
    //-----------------------------------------------------------------------
    DataStreamPtr FileSystemArchive::open(const String& filename) const
    {
        String full_path = concatenate_path(mName, filename);

        // Use filesystem to determine size 
        // (quicker than streaming to the end and back)
        struct stat tagStat;
	int ret = stat(full_path.c_str(), &tagStat);
        assert(ret == 0 && "Problem getting file size" );

        // Always open in binary mode
        std::ifstream *origStream = OGRE_NEW_T(std::ifstream, MEMCATEGORY_GENERAL)();
        origStream->open(full_path.c_str(), std::ios::in | std::ios::binary);

        // Should check ensure open succeeded, in case fail for some reason.
        if (origStream->fail())
        {
            OGRE_DELETE_T(origStream, basic_ifstream, MEMCATEGORY_GENERAL);
            OGRE_EXCEPT(Exception::ERR_FILE_NOT_FOUND,
                "Cannot open file: " + filename,
                "FileSystemArchive::open");
        }

        /// Construct return stream, tell it to delete on destroy
        FileStreamDataStream* stream = OGRE_NEW FileStreamDataStream(filename,
            origStream, tagStat.st_size, true);
        return DataStreamPtr(stream);
    }
    //-----------------------------------------------------------------------
    StringVectorPtr FileSystemArchive::list(bool recursive, bool dirs)
    {
		// directory change requires locking due to saved returns
		// Note that we have to tell the SharedPtr to use OGRE_DELETE_T not OGRE_DELETE by passing category
		StringVectorPtr ret(OGRE_NEW_T(StringVector, MEMCATEGORY_GENERAL)(), SPFM_DELETE_T);

        findFiles("*", recursive, dirs, ret.getPointer(), 0);

        return ret;
    }
    //-----------------------------------------------------------------------
    FileInfoListPtr FileSystemArchive::listFileInfo(bool recursive, bool dirs)
    {
		// Note that we have to tell the SharedPtr to use OGRE_DELETE_T not OGRE_DELETE by passing category
        FileInfoListPtr ret(OGRE_NEW_T(FileInfoList, MEMCATEGORY_GENERAL)(), SPFM_DELETE_T);

        findFiles("*", recursive, dirs, 0, ret.getPointer());

        return ret;
    }
    //-----------------------------------------------------------------------
    StringVectorPtr FileSystemArchive::find(const String& pattern,
                                            bool recursive, bool dirs)
    {
		// Note that we have to tell the SharedPtr to use OGRE_DELETE_T not OGRE_DELETE by passing category
		StringVectorPtr ret(OGRE_NEW_T(StringVector, MEMCATEGORY_GENERAL)(), SPFM_DELETE_T);

        findFiles(pattern, recursive, dirs, ret.getPointer(), 0);

        return ret;

    }
    //-----------------------------------------------------------------------
    FileInfoListPtr FileSystemArchive::findFileInfo(const String& pattern, 
        bool recursive, bool dirs)
    {
		// Note that we have to tell the SharedPtr to use OGRE_DELETE_T not OGRE_DELETE by passing category
		FileInfoListPtr ret(OGRE_NEW_T(FileInfoList, MEMCATEGORY_GENERAL)(), SPFM_DELETE_T);

        findFiles(pattern, recursive, dirs, 0, ret.getPointer());

        return ret;
    }
    //-----------------------------------------------------------------------
	bool FileSystemArchive::exists(const String& filename)
	{
        String full_path = concatenate_path(mName, filename);

        struct stat tagStat;
        bool ret = (stat(full_path.c_str(), &tagStat) == 0);

		// stat will return true if the filename is absolute, but we need to check
		// the file is actually in this archive
        if (ret && is_absolute_path(filename.c_str()))
		{
			// only valid if full path starts with our base
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
			// case insensitive on windows
			String lowerCaseName = mName;
			StringUtil::toLowerCase(lowerCaseName);
			ret = Ogre::StringUtil::startsWith(full_path, lowerCaseName, true);
#else
			// case sensitive
			ret = Ogre::StringUtil::startsWith(full_path, mName, false);
#endif
		}

		return ret;
	}
	//---------------------------------------------------------------------
	time_t FileSystemArchive::getModifiedTime(const String& filename)
	{
		String full_path = concatenate_path(mName, filename);

		struct stat tagStat;
		bool ret = (stat(full_path.c_str(), &tagStat) == 0);

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
    const String& FileSystemArchiveFactory::getType(void) const
    {
        static String name = "FileSystem";
        return name;
    }

}
