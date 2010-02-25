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
#ifndef __FileSystemLayer_H__
#define __FileSystemLayer_H__

#include "OgreString.h"

namespace OgreBites
{
	/** Provides methods to find out where the Ogre config files are stored
	    and where logs and settings files should be written to.
		@remarks
			In modern multi-user OS, a standard user account will often not
			have write access to the path where the SampleBrowser is stored.
			In order to still be able to store graphics settings and log
			output and for the user to overwrite the default Ogre config files, 
			this class tries to create a folder inside the user's home directory. 
			Specialised implementations for each individual platform must
			subclass this abstract interface.
	  */
	class FileSystemLayer
	{
	public:
		virtual ~FileSystemLayer() {}

		/** Search for the given config file in the user's home path. If it can't
		    be found there, the function falls back to the system-wide install
			path for Ogre config files. (Usually the same place where the
			SampleBrowser resides, or a special config path above that path.)
			@param
				The config file name (without path)
			@returns
				The full path to the config file.
		 */
		virtual const Ogre::String getConfigFilePath(Ogre::String filename) const = 0;

		/** Find a path where the given filename can be written to. This path 
			will usually be in the user's home directory. This function should
			be used for any output like logs and graphics settings.
			@param
				Name of the file.
			@returns
				The full path to a writable location for the given filename.
		 */
		virtual const Ogre::String getWritablePath(const Ogre::String& filename) const = 0;
	};
}


#endif
