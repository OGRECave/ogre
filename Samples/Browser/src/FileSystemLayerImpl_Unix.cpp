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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>

namespace OgreBites
{
	namespace {
		/** Get actual file pointed to by symlink */
		const Ogre::String resolveSymlink(const Ogre::String& symlink)
		{
			ssize_t bufsize = 256;
			char* resolved = 0;
			do
			{
				char* buf = OGRE_ALLOC_T(char, bufsize, Ogre::MEMCATEGORY_GENERAL);
				ssize_t retval = readlink(symlink.c_str(), buf, bufsize);
				if (retval == -1)
				{
					OGRE_FREE(buf, Ogre::MEMCATEGORY_GENERAL);
					break;
				}

				if (retval < bufsize)
				{
					// operation was successful.
					// readlink does not guarantee to 0-terminate, so do this manually
					buf[retval] = '\0';
					resolved = buf;
				}
				else
				{
					// buffer was too small, grow buffer and try again
					OGRE_FREE(buf, Ogre::MEMCATEGORY_GENERAL);
					bufsize <<= 1;
				}
			} while (!resolved);

			Ogre::String result (resolved);
			if (resolved)
				OGRE_FREE(resolved, Ogre::MEMCATEGORY_GENERAL);
			return result;
		}
	}
    //---------------------------------------------------------------------
	void FileSystemLayerImpl::getConfigPaths()
	{
		// try to determine the application's path:
		// recent systems should provide the executable path via the /proc system
		Ogre::String appPath = resolveSymlink("/proc/self/exe");
		if (appPath.empty())
		{
			// if /proc/self/exe isn't available, try it via the program's pid
			pid_t pid = getpid();
			char proc[64];
			int retval = snprintf(proc, sizeof(proc), "/proc/%llu/exe", (unsigned long long) pid);
			if (retval > 0 && retval < sizeof(proc))
				appPath = resolveSymlink(proc);
		}

		if (!appPath.empty())
		{
			// we need to strip the executable name from the path
			Ogre::String::size_type pos = appPath.rfind('/');
			if (pos != Ogre::String::npos)
				appPath.erase(pos);
		}
		else
		{
			// couldn't find actual executable path, assume current working dir
			appPath = ".";
		}

		// use application path as first config search path
		mConfigPaths.push_back(appPath + '/');
		// then search inside ../share/OGRE
		mConfigPaths.push_back(appPath + "/../share/OGRE/");
		// then try system wide /etc
		mConfigPaths.push_back("/etc/OGRE/");
	}
    //---------------------------------------------------------------------
	void FileSystemLayerImpl::prepareUserHome(const Ogre::String& subdir)
	{
		struct passwd* pwd = getpwuid(getuid());
		if (pwd)
		{
			mHomePath = pwd->pw_dir;
		}
		else
		{
			// try the $HOME environment variable
			mHomePath = getenv("HOME");
		}

		if (!mHomePath.empty())
		{
			// create an .ogre subdir
			mHomePath.append("/.ogre/");
			if (mkdir(mHomePath.c_str(), 0755) != 0 && errno != EEXIST)
			{
				// can't create dir
				mHomePath.clear();
			}
			else
			{
				// now create the given subdir
				mHomePath.append(subdir + '/');
				if (mkdir(mHomePath.c_str(), 0755) != 0 && errno != EEXIST)
				{
					// can't create dir
					mHomePath.clear();
				}
			}
		}

		if (mHomePath.empty())
		{
			// couldn't create dir in home directory, fall back to cwd
			mHomePath = "./";
		}
	}
    //---------------------------------------------------------------------
	const bool FileSystemLayerImpl::fileExists(const Ogre::String& path) const
	{
		return access(path.c_str(), R_OK) == 0;
	}
}
