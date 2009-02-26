/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
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

#ifndef __GLESPrerequisites_H__
#define __GLESPrerequisites_H__

#include "OgrePrerequisites.h"
#include "OgreMath.h"

#include <GLES/gl.h>
#include <GLES/egl.h>

#ifndef None
#define None NULL
#endif

//PowerVR extension
#define GL_BGRA_PVR 0x80E1

#if (OGRE_PLATFORM == OGRE_PLATFORM_WIN32) && !defined(__MINGW32__) && !defined(OGRE_STATIC_LIB)
#   ifdef OGRE_GLESPLUGIN_EXPORTS
#       define _OgreGLESExport __declspec(dllexport)
#   else
#       if defined( __MINGW32__ )
#           define _OgreGLESExport
#       else
#           define _OgreGLExport __declspec(dllimport)
#       endif
#   endif
#elif defined ( OGRE_GCC_VISIBILITY )
#    define _OgreGLESExport  __attribute__ ((visibility("default")))
#else
#    define _OgreGLESExport
#endif

#define DEBUG_(text) \
    {\
        fprintf(stderr, "%s:%d: %s\n", __FUNCTION__, __LINE__, text); \
    }

#if ENABLE_GL_CHECK
#define GL_CHECK_ERROR \
    { \
        int e = glGetError(); \
        if (e != 0) \
        { \
            fprintf(stderr, "%s:%s:%d - GL ERROR: %x\n", __FILE__, __FUNCTION__, __LINE__, e); \
            assert(false); \
        } \
    }
#else
#define GL_CHECK_ERROR {}
#endif

#if ENABLE_GL_CHECK
    #define EGL_CHECK_ERROR \
    { \
        int e = eglGetError(); \
        if ((e != 0) && (e != EGL_SUCCESS))\
        { \
            fprintf(stderr, "%s:%s:%d - EGL ERROR: %x\n", __FILE__, __FUNCTION__, __LINE__, e); \
            assert(false); \
        } \
    }
#else
    #define EGL_CHECK_ERROR  {}
#endif

#endif
