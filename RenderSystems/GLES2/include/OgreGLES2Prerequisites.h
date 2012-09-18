/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2012 Torus Knot Software Ltd

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

#ifndef __GLES2Prerequisites_H__
#define __GLES2Prerequisites_H__

#include "OgrePrerequisites.h"
#include "OgreLogManager.h"
#include "OgreMath.h"

#if (OGRE_PLATFORM == OGRE_PLATFORM_WIN32)
#	if !defined( __MINGW32__ )
#		define __PRETTY_FUNCTION__ __FUNCTION__
#		ifndef WIN32_LEAN_AND_MEAN
#			define WIN32_LEAN_AND_MEAN 1
#		endif
#		ifndef NOMINMAX
#			define NOMINMAX // required to stop windows.h messing up std::min
#		endif
#	endif
#endif

#ifndef GL_GLEXT_PROTOTYPES
#  define  GL_GLEXT_PROTOTYPES
#endif

#if (OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS)
#   include <OpenGLES/ES2/gl.h>
#   include <OpenGLES/ES2/glext.h>
#   ifdef __OBJC__
#       include <OpenGLES/EAGL.h>
#   endif
#elif (OGRE_PLATFORM == OGRE_PLATFORM_ANDROID) || (OGRE_PLATFORM == OGRE_PLATFORM_NACL)
#	ifndef GL_GLEXT_PROTOTYPES
#		define  GL_GLEXT_PROTOTYPES
#	endif
#	include <GLES2/gl2platform.h>
#	include <GLES2/gl2.h>
#	include <GLES2/gl2ext.h>
#	if (OGRE_PLATFORM == OGRE_PLATFORM_NACL)
#		include "ppapi/cpp/completion_callback.h"
#       include "ppapi/cpp/instance.h"
#       include "ppapi/c/ppp_graphics_3d.h"
#       include "ppapi/cpp/graphics_3d.h"
#       include "ppapi/cpp/graphics_3d_client.h"
#		include "ppapi/gles2/gl2ext_ppapi.h"
#       undef GL_OES_get_program_binary
#       undef GL_OES_mapbuffer
#	endif
#else
#   include <GLES2/gl2.h>
#   include <GLES2/gl2ext.h>
#   include <EGL/egl.h>

// Function pointers for FBO extension methods
// Declare them here since we don't have GLEW to do it for us

#	ifndef GL_GLEXT_PROTOTYPES
extern PFNGLMAPBUFFEROESPROC glMapBufferOES;
extern PFNGLUNMAPBUFFEROESPROC glUnmapBufferOES;
extern PFNGLDRAWBUFFERSARBPROC glDrawBuffersARB;
extern PFNGLREADBUFFERNVPROC glReadBufferNV;
extern PFNGLGETCOMPRESSEDTEXIMAGENVPROC glGetCompressedTexImageNV;
extern PFNGLGETTEXIMAGENVPROC glGetTexImageNV;
extern PFNGLGETTEXLEVELPARAMETERFVNVPROC glGetTexLevelParameterfvNV;
extern PFNGLGETTEXLEVELPARAMETERiVNVPROC glGetTexLevelParameterivNV;
#	endif

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

#endif


// Define GL_NONE for convenience
#define GL_NONE 0

#if !defined(GL_BGRA) && OGRE_PLATFORM != OGRE_PLATFORM_NACL
#   define GL_BGRA  0x80E1
#endif

#if (OGRE_PLATFORM == OGRE_PLATFORM_WIN32)
// an error in all windows gles sdks...
#   undef GL_OES_get_program_binary
#endif

#if (OGRE_PLATFORM == OGRE_PLATFORM_WIN32) && !defined(__MINGW32__) && !defined(OGRE_STATIC_LIB)
#   ifdef OGRE_GLES2PLUGIN_EXPORTS
#       define _OgreGLES2Export __declspec(dllexport)
#   else
#       if defined( __MINGW32__ )
#           define _OgreGLES2Export
#       else
#           define _OgreGLES2Export __declspec(dllimport)
#       endif
#   endif
#elif defined ( OGRE_GCC_VISIBILITY )
#    define _OgreGLES2Export  __attribute__ ((visibility("default")))
#else
#    define _OgreGLES2Export
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
            const char * errorString = ""; \
            switch(e) \
            { \
            case GL_INVALID_ENUM:       errorString = "GL_INVALID_ENUM";        break; \
            case GL_INVALID_VALUE:      errorString = "GL_INVALID_VALUE";       break; \
            case GL_INVALID_OPERATION:  errorString = "GL_INVALID_OPERATION";   break; \
            case GL_OUT_OF_MEMORY:      errorString = "GL_OUT_OF_MEMORY";       break; \
            default:                                                            break; \
            } \
            char msgBuf[10000]; \
            sprintf(msgBuf, "OpenGL ES2 error 0x%04X %s in %s at line %i\n", e, errorString, __PRETTY_FUNCTION__, __LINE__); \
            LogManager::getSingleton().logMessage(msgBuf); \
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
            char msgBuf[10000]; \
            sprintf(msgBuf, "EGL error 0x%04X in %s at line %i\n", e, __PRETTY_FUNCTION__, __LINE__); \
            LogManager::getSingleton().logMessage(msgBuf); \
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, msgBuf, __PRETTY_FUNCTION__); \
        } \
    }
#else
    #define EGL_CHECK_ERROR {}
#endif

#endif
