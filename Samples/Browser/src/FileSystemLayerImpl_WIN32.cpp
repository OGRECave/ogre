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
#include "FileSystemLayerImpl.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#include <io.h>

namespace OgreBites
{
	void FileSystemLayerImpl::getConfigPaths()
	{
		// try to determine the application's path
		DWORD bufsize = 256;
		char* resolved = 0;
		do
		{
			char* buf = OGRE_ALLOC_T(char, bufsize, Ogre::MEMCATEGORY_GENERAL);
			DWORD retval = GetModuleFileName(NULL, buf, bufsize);
			if (retval == 0)
			{
				// failed
				OGRE_FREE(buf, Ogre::MEMCATEGORY_GENERAL);
				break;
			}

			if (retval < bufsize)
			{
				// operation was successful.
				resolved = buf;
			}
			else
			{
				// buffer was too small, grow buffer and try again
				OGRE_FREE(buf, Ogre::MEMCATEGORY_GENERAL);
				bufsize <<= 1;
			}
		} while (!resolved);

		Ogre::String appPath = resolved;
		if (resolved)
			OGRE_FREE(resolved, Ogre::MEMCATEGORY_GENERAL);
		if (!appPath.empty())
		{
			// need to strip the application filename from the path
			Ogre::String::size_type pos = appPath.rfind('\\');
			if (pos != Ogre::String::npos)
				appPath.erase(pos);
		}
		else
		{
			// fall back to current working dir
			appPath = ".";
		}

		// use application path as config search path
		mConfigPaths.push_back(appPath + '\\');
	}
    //---------------------------------------------------------------------
	void FileSystemLayerImpl::prepareUserHome(const Ogre::String& subdir)
	{
		TCHAR path[MAX_PATH];
		if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL|CSIDL_FLAG_CREATE, NULL, 0, path)))
		{
			mHomePath = path;
			// create Ogre subdir
			mHomePath += "\\Ogre\\";
			if (! CreateDirectory(mHomePath.c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
			{
				// couldn't create directory, fall back to current working dir
				mHomePath.clear();
			}
			else
			{
				mHomePath += subdir + '\\';
				// create release subdir
				if (! CreateDirectory(mHomePath.c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
				{
					// couldn't create directory, fall back to current working dir
					mHomePath.clear();
				}
			}
		}

		if (mHomePath.empty())
		{
			// couldn't create dir in home directory, fall back to cwd
			mHomePath = "";
		}
	}
    //---------------------------------------------------------------------
	const bool FileSystemLayerImpl::fileExists(const Ogre::String& path) const
	{
		return _access(path.c_str(), 00) == 0;
	}
}
