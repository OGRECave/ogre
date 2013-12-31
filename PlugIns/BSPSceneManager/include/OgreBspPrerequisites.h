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
#ifndef __BspPrerequisites_H__
#define __BspPrerequisites_H__

#include "OgrePrerequisites.h"

namespace Ogre {

    // Predeclare classes

    class BspLevel;
    class BspNode;
    class BspResourceManager;
    class BspSceneManager;
    class Quake3Level;
    class Quake3ShaderManager;
    class Quake3Shader;

#if (OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WINRT) && !defined(OGRE_STATIC_LIB)
#	ifdef OGRE_BSPPLUGIN_EXPORTS
#		define _OgreBspPluginExport __declspec(dllexport)
#	else
#       if defined( __MINGW32__ )
#           define _OgreBspPluginExport
#       else
#    		define _OgreBspPluginExport __declspec(dllimport)
#       endif
#   endif
#elif defined ( OGRE_GCC_VISIBILITY )
#    define _OgreBspPluginExport  __attribute__ ((visibility("default")))
#else
#	define _OgreBspPluginExport
#endif	// OGRE_WIN32


}


#endif
