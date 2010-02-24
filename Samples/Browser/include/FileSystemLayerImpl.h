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
#ifndef __FileSystemLayerImpl_H__
#define __FileSystemLayerImpl_H__

#include "FileSystemLayer.h"
#include "OgrePrerequisites.h"
#include "OgreStringVector.h"

namespace OgreBites
{
	/** Implementation for the FileSystemLayer interface. */
	class FileSystemLayerImpl : public FileSystemLayer
	{
	public:
		/** Creates a concrete platform-dependent implementation of FileSystemLayer.
		  	@param
				A subdirectory inside the user's path to distinguish between
				different Ogre releases.
		 */
		FileSystemLayerImpl(const Ogre::String& subdir)
		{
			// determine directories to search for config files
			getConfigPaths();
			// prepare write location in user directory
			prepareUserHome(subdir);
		}

		const Ogre::String getConfigFilePath(Ogre::String filename) const
		{
#if OGRE_DEBUG_MODE == 1
			// add _d suffix to config files
			Ogre::String::size_type pos = filename.rfind('.');
			if (pos != Ogre::String::npos)
				filename = filename.substr(0, pos) + "_d" + filename.substr(pos);
#endif
			// look for the requested file in several locations:

			// 1. in the writable path (so user can provide custom files)
			Ogre::String path = getWritablePath(filename);
			if (fileExists(path))
				return path;

			// 2. in the config file search paths
			for (size_t i = 0; i < mConfigPaths.size(); ++i)
			{
				path = mConfigPaths[i] + filename;
				if (fileExists(path))
					return path;
			}

			// 3. fallback to current working dir
			return filename;
		}

		const Ogre::String getWritablePath(const Ogre::String& filename) const
		{
			return mHomePath + filename;
		}

	private:
		Ogre::StringVector mConfigPaths;
		Ogre::String mHomePath;

		/** Determine config search paths. */
		void getConfigPaths();

		/** Create an Ogre directory and the given subdir in the user's home. */
		void prepareUserHome(const Ogre::String& subdir);

		/** Test if the given file exists. */
		const bool fileExists(const Ogre::String& path) const;

	};
}

#endif
