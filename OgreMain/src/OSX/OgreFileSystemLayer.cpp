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
#include "OgreFileSystemLayer.h"
#include "macUtils.h"
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>

namespace Ogre
{
	void FileSystemLayer::getConfigPaths()
	{
		mConfigPaths.push_back(Ogre::macBundlePath() + "/Contents/Resources/");
		mConfigPaths.push_back(Ogre::macBundlePath() + "/");
	}
    //---------------------------------------------------------------------
	void FileSystemLayer::prepareUserHome(const Ogre::String& subdir)
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
			// create an Ogre subdir in application support
			mHomePath.append("/Library/Application Support/Ogre/");
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
	bool FileSystemLayer::fileExists(const Ogre::String& path) const
	{
		return access(path.c_str(), R_OK) == 0;
	}
    //---------------------------------------------------------------------
	bool FileSystemLayer::createDirectory(const Ogre::String& path)
	{
		return !mkdir(path.c_str(), 0755) || errno == EEXIST;
	}
}
