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

// Emulate _findfirst, _findnext on non-Windows platforms


#include "OgreSearchOps.h"
#include <stdio.h>
#include <ctype.h>

#if OGRE_PLATFORM == OGRE_PLATFORM_SYMBIAN
#include "OgreString.h"

// SYMBIAN todo - possibly use - CDirScan from C:\Symbian\9.2\S60_3rd_FP1\Epoc32\include\f32file.h
// see this sample - http://wiki.forum.nokia.com/index.php/Find_Files

bool fnmatch (Ogre::String pattern, Ogre::String name, int dummy)
{
	if (pattern == "*")
	{
		return true;
	}
	if (pattern.substr(0,2) == "*.")
	{
		Ogre::StringUtil::toLowerCase(pattern);
		Ogre::StringUtil::toLowerCase(name);
		Ogre::String extToFind = pattern.substr(2, pattern.size() - 2);
		if ((name.size() > extToFind.size()) &&(extToFind == name.substr(name.size() - extToFind.size(), extToFind.size())))
		{
			return 0; // match
		}
		else
		{
			return 1; // don't match
		}
	}
	return false;
}
#endif

/* Win32 directory operations emulation */
#if OGRE_PLATFORM != OGRE_PLATFORM_WIN32


struct _find_search_t
{
    char *pattern;
    char *curfn;
    char *directory;
    int dirlen;
    DIR *dirfd;
};
        
long _findfirst(const char *pattern, struct _finddata_t *data)
{
    _find_search_t *fs = new _find_search_t;
    fs->curfn = NULL;
    fs->pattern = NULL;

    // Separate the mask from directory name
    const char *mask = strrchr (pattern, '/');
    if (mask)
    {
        fs->dirlen = mask - pattern;
        mask++;
        fs->directory = (char *)malloc (fs->dirlen + 1);
        memcpy (fs->directory, pattern, fs->dirlen);
        fs->directory [fs->dirlen] = 0;
    }
    else
    {
        mask = pattern;
        fs->directory = strdup (".");
        fs->dirlen = 1;
    }

    fs->dirfd = opendir (fs->directory);
    if (!fs->dirfd)
    {
        _findclose ((long)fs);
        return -1;
    }

    /* Hack for "*.*" -> "*' from DOS/Windows */
    if (strcmp (mask, "*.*") == 0)
        mask += 2;
    fs->pattern = strdup (mask);

    /* Get the first entry */
    if (_findnext ((long)fs, data) < 0)
    {
        _findclose ((long)fs);
        return -1;
    }

    return (long)fs;
}

int _findnext(long id, struct _finddata_t *data)
{
    _find_search_t *fs = (_find_search_t *)id;

    /* Loop until we run out of entries or find the next one */
    dirent *entry;
    for (;;)
    {
        if (!(entry = readdir (fs->dirfd)))
            return -1;

        /* See if the filename matches our pattern */
        if (fnmatch (fs->pattern, entry->d_name, 0) == 0)
            break;
    }

    if (fs->curfn)
        free (fs->curfn);
    data->name = fs->curfn = strdup (entry->d_name);

    size_t namelen = strlen (entry->d_name);
    char *xfn = new char [fs->dirlen + 1 + namelen + 1];
    sprintf (xfn, "%s/%s", fs->directory, entry->d_name);

    /* stat the file to get if it's a subdir and to find its length */
    struct stat stat_buf;
    if (stat (xfn, &stat_buf))
    {
        // Hmm strange, imitate a zero-length file then
        data->attrib = _A_NORMAL;
        data->size = 0;
    }
    else
    {
        if (S_ISDIR(stat_buf.st_mode))
            data->attrib = _A_SUBDIR;
        else
            /* Default type to a normal file */
            data->attrib = _A_NORMAL;

        data->size = stat_buf.st_size;
    }

    delete [] xfn;

    /* Files starting with a dot are hidden files in Unix */
    if (data->name [0] == '.')
        data->attrib |= _A_HIDDEN;

    return 0;
}

int _findclose(long id)
{
    int ret;
    _find_search_t *fs = (_find_search_t *)id;
    
    ret = fs->dirfd ? closedir (fs->dirfd) : 0;
    free (fs->pattern);
    free (fs->directory);
    if (fs->curfn)
        free (fs->curfn);
    delete fs;

    return ret;
}

#endif
