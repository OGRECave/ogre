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
#ifndef __GL3PlusPrerequisites_H__
#define __GL3PlusPrerequisites_H__

#include "OgrePrerequisites.h"

#include "OgreLogManager.h"

namespace Ogre {
    // Forward declarations
    class GLContext;
    class GLRTTManager;
    typedef GLContext GL3PlusContext;
    typedef GLRTTManager GL3PlusRTTManager;
    class GL3PlusRenderSystem;
    class GL3PlusStateCacheManager;
    class GL3PlusSupport;
    class GL3PlusTexture;
    class GL3PlusTextureManager;
    class GL3PlusFBOManager;
    class GL3PlusHardwarePixelBuffer;
    class GL3PlusRenderBuffer;
    class GL3PlusDepthBuffer;
    
    class GLSLShader;

    typedef shared_ptr<GLSLShader> GLSLShaderPtr;
    typedef shared_ptr<GL3PlusTexture> GL3PlusTexturePtr;
}

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#if !defined( __MINGW32__ )
#   define WIN32_LEAN_AND_MEAN
#  ifndef NOMINMAX
#   define NOMINMAX // required to stop windows.h messing up std::min
#  endif
#endif
#   include <windows.h>
#   include <wingdi.h>
#   include <GL/gl3w.h>
#elif OGRE_PLATFORM == OGRE_PLATFORM_LINUX
#   include <GL/gl3w.h>
#elif OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#   include <GL/gl3w.h>
#endif

// Lots of generated code in here which triggers the new VC CRT security warnings
#if !defined( _CRT_SECURE_NO_DEPRECATE )
#define _CRT_SECURE_NO_DEPRECATE
#endif

#if (OGRE_PLATFORM == OGRE_PLATFORM_WIN32) && !defined(__MINGW32__) && !defined(OGRE_STATIC_LIB)
#   ifdef RenderSystem_GL3Plus_EXPORTS
#       define _OgreGL3PlusExport __declspec(dllexport)
#   else
#       if defined( __MINGW32__ )
#           define _OgreGL3PlusExport
#       else
#           define _OgreGL3PlusExport __declspec(dllimport)
#       endif
#   endif
#elif defined ( OGRE_GCC_VISIBILITY )
#    define _OgreGL3PlusExport  __attribute__ ((visibility("default")))
#else
#    define _OgreGL3PlusExport
#endif

// Convenience macro from ARB_vertex_buffer_object spec
#define GL_BUFFER_OFFSET(i) ((char *)(i))

#if OGRE_COMPILER == OGRE_COMPILER_MSVC
#   define __PRETTY_FUNCTION__ __FUNCTION__
#endif

#define ENABLE_GL_CHECK 0
#define ENABLE_GL_DEBUG_OUTPUT 0

#if ENABLE_GL_CHECK
#include "OgreStringVector.h"
#define OGRE_CHECK_GL_ERROR(glFunc) \
{ \
    glFunc; \
    int e = glGetError(); \
    if (e != 0) \
    { \
        const char * errorString = ""; \
        switch(e) \
        { \
            case GL_INVALID_ENUM:       errorString = "GL_INVALID_ENUM";        break; \
            case GL_INVALID_VALUE:      errorString = "GL_INVALID_VALUE";       break; \
            case GL_INVALID_OPERATION:  errorString = "GL_INVALID_OPERATION";   break; \
            case GL_INVALID_FRAMEBUFFER_OPERATION:  errorString = "GL_INVALID_FRAMEBUFFER_OPERATION";   break; \
            case GL_OUT_OF_MEMORY:      errorString = "GL_OUT_OF_MEMORY";       break; \
            default:                                                            break; \
        } \
        char msgBuf[4096]; \
        StringVector tokens = StringUtil::split(#glFunc, "("); \
        sprintf(msgBuf, "OpenGL error 0x%04X %s in %s at line %i for %s\n", e, errorString, __PRETTY_FUNCTION__, __LINE__, tokens[0].c_str()); \
        LogManager::getSingleton().logMessage(msgBuf, LML_CRITICAL); \
    } \
}
#else
#   define OGRE_CHECK_GL_ERROR(glFunc) { glFunc; }
#endif

#endif //#ifndef __GL3PlusPrerequisites_H__
