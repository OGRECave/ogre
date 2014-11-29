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
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSFileManager.h>

namespace Ogre
{
	void FileSystemLayer::getConfigPaths()
	{
        mConfigPaths.push_back(Ogre::iOSDocumentsDirectory() + "/");
		mConfigPaths.push_back(Ogre::macBundlePath() + "/");
	}
    //---------------------------------------------------------------------
	void FileSystemLayer::prepareUserHome(const Ogre::String& subdir)
	{
        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);

        mHomePath = Ogre::String([[paths objectAtIndex:0] cStringUsingEncoding:NSASCIIStringEncoding]) + "/";
	}
    //---------------------------------------------------------------------
    bool FileSystemLayer::fileExists(const Ogre::String& path) const
	{
		return access(path.c_str(), R_OK) == 0;
	}
    //---------------------------------------------------------------------
	bool FileSystemLayer::createDirectory(const Ogre::String& path)
	{
        NSString *directoryPath = [NSString stringWithCString:path.c_str() encoding:NSUTF8StringEncoding];
        NSError *error;

        if (![[NSFileManager defaultManager] createDirectoryAtPath:directoryPath
                                       withIntermediateDirectories:NO
                                                        attributes:nil
                                                             error:&error])
        {
            return false;
        }
        return true;
	}
}
