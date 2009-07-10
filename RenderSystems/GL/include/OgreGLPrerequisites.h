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
#ifndef __GLPrerequisites_H__
#define __GLPrerequisites_H__

#include "OgrePrerequisites.h"

namespace Ogre {
    // Forward declarations
    class GLSupport;
    class GLRenderSystem;
    class GLTexture;
    class GLTextureManager;
    class GLGpuProgram;
    class GLContext;
    class GLRTTManager;
    class GLFBOManager;
    class GLHardwarePixelBuffer;
    class GLRenderBuffer;
}

#if OGRE_THREAD_SUPPORT == 1
#	define GLEW_MX
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#if !defined( __MINGW32__ )
#   define WIN32_LEAN_AND_MEAN
#   define NOMINMAX // required to stop windows.h messing up std::min
#endif
#   include <windows.h>
#   include <wingdi.h>
#   include <GL/glew.h>
#   include <GL/wglew.h>
#   include <GL/glu.h>
#elif OGRE_PLATFORM == OGRE_PLATFORM_LINUX
#   include <GL/glew.h>
#   include <GL/glu.h>
#   define GL_GLEXT_PROTOTYPES
#elif OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#   include <GL/glew.h>
#   include <OpenGL/glu.h>
#elif OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
#   include <OpenGLES/ES1/gl.h>
#   include <OpenGLES/ES1/glext.h>
#endif

#if OGRE_THREAD_SUPPORT == 1
	// implemented in OgreGLContext.cpp
	GLEWContext * glewGetContext();

#	if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	// implemented in OgreWin32Context.cpp
	WGLEWContext * wglewGetContext();
#	endif

#endif

/// Lots of generated code in here which triggers the new VC CRT security warnings
#if !defined( _CRT_SECURE_NO_DEPRECATE )
#define _CRT_SECURE_NO_DEPRECATE
#endif

#if (OGRE_PLATFORM == OGRE_PLATFORM_WIN32) && !defined(__MINGW32__) && !defined(OGRE_STATIC_LIB)
#	ifdef OGRE_GLPLUGIN_EXPORTS
#		define _OgreGLExport __declspec(dllexport)
#	else
#       if defined( __MINGW32__ )
#           define _OgreGLExport
#       else
#    		define _OgreGLExport __declspec(dllimport)
#       endif
#	endif
#elif defined ( OGRE_GCC_VISIBILITY )
#    define _OgreGLExport  __attribute__ ((visibility("default")))
#else
#    define _OgreGLExport
#endif

#endif //#ifndef __GLPrerequisites_H__
