/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
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

#ifndef __GLESPrerequisites_H__
#define __GLESPrerequisites_H__


#if (OGRE_PLATFORM == OGRE_PLATFORM_SYMBIAN)
#	undef _WIN32
#endif

#if (OGRE_PLATFORM == OGRE_PLATFORM_WIN32)
#	if !defined( __MINGW32__ )
#		ifndef WIN32_LEAN_AND_MEAN
#			define WIN32_LEAN_AND_MEAN 1
#		endif
#		ifndef NOMINMAX
#			define NOMINMAX // required to stop windows.h messing up std::min
#		endif
#	endif
#	define OGRE_NEW_FIX_FOR_WIN32 new 
#else
#	define OGRE_NEW_FIX_FOR_WIN32 OGRE_NEW
#endif

#include "OgrePrerequisites.h"
#include "OgreMath.h"

#if (OGRE_PLATFORM == OGRE_PLATFORM_IPHONE)
#   include <OpenGLES/ES1/gl.h>
#   include <OpenGLES/ES1/glext.h>
#   ifdef __OBJC__
#       include <OpenGLES/EAGL.h>
#   endif
#	ifndef GL_GLEXT_PROTOTYPES
#		define  GL_GLEXT_PROTOTYPES
#	endif
#else
#   include <GLES/gl.h>
#   include <GLES/glext.h>
#   include <GLES/egl.h>

// If we are going to use the PVRTC_CODEC make sure we
// setup the needed constants
#if (OGRE_NO_PVRTC_CODEC == 0)
#	ifndef GL_IMG_texture_compression_pvrtc
#		define GL_IMG_texture_compression_pvrtc 1
#		define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG                      0x8C00
#		define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG                      0x8C01
#		define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG                     0x8C02
#		define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG                     0x8C03
#	endif
#endif

// Function pointers for FBO extension methods
// Declare them here since we don't have GLEW to do it for us

#	ifndef GL_GLEXT_PROTOTYPES
extern PFNGLISRENDERBUFFEROESPROC glIsRenderbufferOES;
extern PFNGLBINDRENDERBUFFEROESPROC glBindRenderbufferOES;
extern PFNGLDELETERENDERBUFFERSOESPROC glDeleteRenderbuffersOES;
extern PFNGLGENRENDERBUFFERSOESPROC glGenRenderbuffersOES;
extern PFNGLRENDERBUFFERSTORAGEOESPROC glRenderbufferStorageOES;
extern PFNGLGETRENDERBUFFERPARAMETERIVOESPROC glGetRenderbufferParameterivOES;
extern PFNGLISFRAMEBUFFEROESPROC glIsFramebufferOES;
extern PFNGLBINDFRAMEBUFFEROESPROC glBindFramebufferOES;
extern PFNGLDELETEFRAMEBUFFERSOESPROC glDeleteFramebuffersOES;
extern PFNGLGENFRAMEBUFFERSOESPROC glGenFramebuffersOES;
extern PFNGLCHECKFRAMEBUFFERSTATUSOESPROC glCheckFramebufferStatusOES;
extern PFNGLFRAMEBUFFERRENDERBUFFEROESPROC glFramebufferRenderbufferOES;
extern PFNGLFRAMEBUFFERTEXTURE2DOESPROC glFramebufferTexture2DOES;
extern PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVOESPROC glGetFramebufferAttachmentParameterivOES;
extern PFNGLGENERATEMIPMAPOESPROC glGenerateMipmapOES;
extern PFNGLBLENDEQUATIONOESPROC glBlendEquationOES;
extern PFNGLBLENDFUNCSEPARATEOESPROC glBlendFuncSeparateOES;
extern PFNGLBLENDEQUATIONSEPARATEOESPROC glBlendEquationSeparateOES;
#	endif

#endif

// Define GL_NONE for convenience
#define GL_NONE 0

#ifndef GL_BGRA
#   define GL_BGRA  0x80E1
#endif

#if (OGRE_PLATFORM == OGRE_PLATFORM_WIN32) && !defined(__MINGW32__) && !defined(OGRE_STATIC_LIB)
#   ifdef OGRE_GLESPLUGIN_EXPORTS
#       define _OgreGLESExport __declspec(dllexport)
#   else
#       if defined( __MINGW32__ )
#           define _OgreGLESExport
#       else
#           define _OgreGLESExport __declspec(dllimport)
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

#define ENABLE_GL_CHECK 0
#if ENABLE_GL_CHECK
#define GL_CHECK_ERROR \
    { \
        int e = glGetError(); \
        if (e != 0) \
        { \
            fprintf(stderr, "OpenGL error 0x%04X in %s at line %i in %s\n", e, __PRETTY_FUNCTION__, __LINE__, __FILE__); \
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
            fprintf(stderr, "OpenGL error 0x%04X in %s at line %i in %s\n", e, __PRETTY_FUNCTION__, __LINE__, __FILE__); \
            assert(false); \
        } \
    }
#else
    #define EGL_CHECK_ERROR {}
#endif

#endif
