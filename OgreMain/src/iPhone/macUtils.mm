/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General  License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General  License for more details.

You should have received a copy of the GNU Lesser General  License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/

#include <CoreFoundation/CoreFoundation.h>
#include <Foundation/Foundation.h>

#include "OgreString.h"
#include "macUtils.h"

namespace Ogre {

    // Basically a dummy function.  Dynamic libraries aren't supported on iPhone
    void* mac_loadDylib(const char* name)
    {
        return NULL;
    }

    std::string macBundlePath()
    {
        char path[PATH_MAX];
        CFBundleRef mainBundle = CFBundleGetMainBundle();
        assert(mainBundle);
        
        CFURLRef mainBundleURL = CFBundleCopyBundleURL(mainBundle);
        assert(mainBundleURL);
        
        CFStringRef cfStringRef = CFURLCopyFileSystemPath( mainBundleURL, kCFURLPOSIXPathStyle);
        assert(cfStringRef);
        
        CFStringGetCString(cfStringRef, path, PATH_MAX, kCFStringEncodingASCII);
        
        CFRelease(mainBundleURL);
        CFRelease(cfStringRef);
        
        return std::string(path);
    }
    
    std::string iPhoneDocumentsDirectory()
    {
        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString *documentsDirectory = [paths objectAtIndex:0];
        
        return std::string([documentsDirectory cStringUsingEncoding:NSASCIIStringEncoding]);
    }

    void pointOrientedToScreen(const Vector2 &v, int orientationMode, Vector2 &outv)
    {
        pointOrientedToScreen(v.x, v.y, orientationMode, outv.x, outv.y);
    }

    void pointOrientedToScreen(Real orientedX, Real orientedY, int orientationMode, Real &screenX, Real &screenY)
    {
        Real orX = orientedX;
        Real orY = orientedY;
        switch (orientationMode)
        {
        case 1:
            screenX = orY;
            screenY = Real(1.0) - orX;
            break;
        case 2:
            screenX = Real(1.0) - orX;
            screenY = Real(1.0) - orY;
            break;
        case 3:
            screenX = Real(1.0) - orY;
            screenY = orX;
            break;
        default:
            screenX = orX;
            screenY = orY;
            break;
        }
    }
}
