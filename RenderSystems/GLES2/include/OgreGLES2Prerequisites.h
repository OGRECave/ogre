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

#ifndef __GLES2Prerequisites_H__
#define __GLES2Prerequisites_H__

#include "OgrePrerequisites.h"
#include "OgreLogManager.h"
#include "OgreMath.h"

#ifndef GL_GLEXT_PROTOTYPES
#  define  GL_GLEXT_PROTOTYPES
#endif

#if OGRE_NO_GLES3_SUPPORT == 0 && OGRE_PLATFORM != OGRE_PLATFORM_EMSCRIPTEN
#   include <GLES3/gles3w.h>
#else
#   include <GLES2/gles2w.h>
#endif

#if (OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS)
#   ifdef __OBJC__
#       include <OpenGLES/EAGL.h>
#       if OGRE_NO_GLES3_SUPPORT == 0
#           define __gl_es20_h_
#           define __gl_es20ext_h_
#           ifndef GL_GLEXT_PROTOTYPES
#               define GL_GLEXT_PROTOTYPES
#           endif
#       endif
#   endif
#elif (OGRE_PLATFORM == OGRE_PLATFORM_ANDROID) || (OGRE_PLATFORM == OGRE_PLATFORM_NACL) || (OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN)
#   ifndef GL_GLEXT_PROTOTYPES
#       define GL_GLEXT_PROTOTYPES
#   endif
#   if OGRE_NO_GLES3_SUPPORT == 0 && OGRE_PLATFORM != OGRE_PLATFORM_EMSCRIPTEN
#       include <GLES3/gl3platform.h>
#       include <GLES3/gl3.h>
#   else
#       include <GLES2/gl2platform.h>
#       include <GLES2/gl2.h>
#       include <GLES2/gl2ext.h>
#   endif
#   if OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
#       define gleswIsSupported(x,y) (false)
#   endif
#   if (OGRE_PLATFORM == OGRE_PLATFORM_NACL)
#       include "ppapi/cpp/completion_callback.h"
#       include "ppapi/cpp/instance.h"
#       include "ppapi/c/ppp_graphics_3d.h"
#       include "ppapi/cpp/graphics_3d.h"
#       include "ppapi/cpp/graphics_3d_client.h"
#       include "ppapi/gles2/gl2ext_ppapi.h"
#       undef GL_OES_get_program_binary
#       undef GL_OES_mapbuffer
#       undef GL_OES_vertex_array_object
#   endif
#else
#   if (OGRE_PLATFORM == OGRE_PLATFORM_WIN32)
#       if !defined( __MINGW32__ )
#           define __PRETTY_FUNCTION__ __FUNCTION__
#           ifndef WIN32_LEAN_AND_MEAN
#               define WIN32_LEAN_AND_MEAN 1
#           endif
#           ifndef NOMINMAX
#               define NOMINMAX // required to stop windows.h messing up std::min
#           endif
#       endif
#   endif
#   undef  GL_GLEXT_PROTOTYPES
#   if OGRE_NO_GLES3_SUPPORT == 0
#       include <GLES3/gl3platform.h>
#       include <GLES3/gl3.h>
#   else
#       include <GLES2/gl2.h>
#       include <GLES2/gl2ext.h>
#   endif
#   include <EGL/egl.h>
#endif

#if (OGRE_NO_ETC_CODEC == 0)
#   ifndef GL_OES_compressed_ETC1_RGB8_texture
#       define GL_OES_compressed_ETC1_RGB8_texture 1
#       define GL_ETC1_RGB8_OES                                         0x8D64
#   endif
#   define GL_COMPRESSED_RGB8_ETC2                                      0x9274
#   define GL_COMPRESSED_SRGB8_ETC2                                     0x9275
#   define GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2                  0x9276
#   define GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2                 0x9277
#   define GL_COMPRESSED_RGBA8_ETC2_EAC                                 0x9278
#   define GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC                          0x9279
#endif

#if (OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS)
#define OGRE_IF_IOS_VERSION_IS_GREATER_THAN(vers) \
    if(static_cast<EAGL2Support*>(dynamic_cast<GLES2RenderSystem*>(Root::getSingleton().getRenderSystem())->getGLSupportRef())->getCurrentOSVersion() >= vers)
#else
#define OGRE_IF_IOS_VERSION_IS_GREATER_THAN(vers)
#endif

#define getGLES2SupportRef() dynamic_cast<GLES2RenderSystem*>(Root::getSingleton().getRenderSystem())->getGLSupportRef()

// Copy this definition from desktop GL.  Used for polygon modes.
#ifndef GL_FILL
#   define GL_FILL    0x1B02
#endif

namespace Ogre {
    struct GLES2HlmsSamplerblock;
    class GLES2GpuProgram;
    class GLES2Texture;
    typedef SharedPtr<GLES2GpuProgram> GLES2GpuProgramPtr;
    typedef SharedPtr<GLES2Texture> GLES2TexturePtr;
};

// Apple doesn't define this in their extension.  We'll do it just for convenience.
// Using the value from desktop GL
#ifndef GL_SAMPLER_2D_SHADOW_EXT
#   define GL_SAMPLER_2D_SHADOW_EXT             0x8B62
#endif

#ifndef GL_EXT_texture_filter_anisotropic
#   define GL_TEXTURE_MAX_ANISOTROPY_EXT        0x84FE
#   define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT    0x84FF
#endif

// Defines for extensions that were made core in OpenGL ES 3
#if OGRE_NO_GLES3_SUPPORT == 0
#ifndef GL_OES_mapbuffer
#define GL_WRITE_ONLY_OES GL_MAP_WRITE_BIT
#define glUnmapBufferOES glUnmapBuffer
#endif

#ifndef GL_OES_texture_half_float
#define GL_HALF_FLOAT_OES GL_HALF_FLOAT
#endif

#ifndef GL_OES_rgb8_rgba8
#define GL_RGB8_OES GL_RGB8
#define GL_RGBA8_OES GL_RGBA8
#endif

#ifndef GL_EXT_texture_rg
#define GL_RG8_EXT GL_RG8
#define GL_RED_EXT GL_RED
#define GL_RG_EXT GL_RG
#define GL_R8_EXT GL_R8
#endif

#ifndef GL_EXT_texture_storage
#define GL_R16F_EXT GL_R16F
#define GL_R32F_EXT GL_R32F
#define GL_RG16F_EXT GL_RG16F
#define GL_RG32F_EXT GL_RG32F
#define GL_RGB16F_EXT GL_RGB16F
#define GL_RGB32F_EXT GL_RGB32F
#define GL_RGBA16F_EXT GL_RGBA16F
#define GL_RGBA32F_EXT GL_RGBA32F
#define GL_DEPTH_COMPONENT32_OES GL_DEPTH_COMPONENT32F
#endif

#ifndef GL_EXT_blend_minmax
#define GL_MIN_EXT GL_MIN
#define GL_MAX_EXT GL_MAX
#endif

#ifndef GL_OES_depth24
#define GL_DEPTH_COMPONENT24_OES GL_DEPTH_COMPONENT24
#endif

#ifndef GL_OES_packed_depth_stencil
#define GL_DEPTH24_STENCIL8_OES GL_DEPTH24_STENCIL8
#endif

#ifndef GL_APPLE_texture_max_level
#define GL_TEXTURE_MAX_LEVEL_APPLE GL_TEXTURE_MAX_LEVEL
#endif

#ifndef GL_APPLE_framebuffer_multisample
#define GL_MAX_SAMPLES_APPLE GL_MAX_SAMPLES
#define glRenderbufferStorageMultisampleAPPLE glRenderbufferStorageMultisample
#endif

#ifndef GL_EXT_occlusion_query_boolean
#define GL_ANY_SAMPLES_PASSED_EXT GL_ANY_SAMPLES_PASSED
#define GL_QUERY_RESULT_EXT GL_QUERY_RESULT
#define GL_QUERY_RESULT_AVAILABLE_EXT GL_QUERY_RESULT_AVAILABLE
#define glGenQueriesEXT glGenQueries
#define glDeleteQueriesEXT glDeleteQueries
#define glBeginQueryEXT glBeginQuery
#define glEndQueryEXT glEndQuery
#define glGetQueryObjectuivEXT glGetQueryObjectuiv
#endif

#ifndef GL_EXT_map_buffer_range
#define GL_MAP_WRITE_BIT_EXT GL_MAP_WRITE_BIT
#define GL_MAP_FLUSH_EXPLICIT_BIT_EXT GL_MAP_FLUSH_EXPLICIT_BIT
#define GL_MAP_INVALIDATE_RANGE_BIT_EXT GL_MAP_INVALIDATE_RANGE_BIT
#define GL_MAP_UNSYNCHRONIZED_BIT_EXT GL_MAP_UNSYNCHRONIZED_BIT
#define GL_MAP_READ_BIT_EXT GL_MAP_READ_BIT
#define glMapBufferRangeEXT glMapBufferRange
#define glFlushMappedBufferRangeEXT glFlushMappedBufferRange
#endif

#ifndef GL_APPLE_sync
#define GL_SYNC_GPU_COMMANDS_COMPLETE_APPLE GL_SYNC_GPU_COMMANDS_COMPLETE
#define GL_SYNC_FLUSH_COMMANDS_BIT_APPLE GL_SYNC_FLUSH_COMMANDS_BIT
#define GL_TIMEOUT_IGNORED_APPLE GL_TIMEOUT_IGNORED
#define GL_WAIT_FAILED_APPLE GL_WAIT_FAILED
#define glFenceSyncAPPLE glFenceSync
#define glClientWaitSyncAPPLE glClientWaitSync
#define glDeleteSyncAPPLE glDeleteSync
#endif

#define GL_PROGRAM_BINARY_LENGTH_OES GL_PROGRAM_BINARY_LENGTH
#define glProgramBinaryOES glProgramBinary
#define glGetProgramBinaryOES glGetProgramBinary

#define glDrawElementsInstancedEXT glDrawElementsInstanced
#define glDrawArraysInstancedEXT glDrawArraysInstanced
#define glVertexAttribDivisorEXT glVertexAttribDivisor
#define glBindVertexArrayOES glBindVertexArray
#define glGenVertexArraysOES glGenVertexArrays
#define glDeleteVertexArraysOES glDeleteVertexArrays
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
            case GL_OUT_OF_MEMORY:      errorString = "GL_OUT_OF_MEMORY";       break; \
            default:                                                            break; \
            } \
            char msgBuf[4096]; \
            StringVector tokens = StringUtil::split(#glFunc, "("); \
            sprintf(msgBuf, "OpenGL error 0x%04X %s in %s at line %i for %s\n", e, errorString, __PRETTY_FUNCTION__, __LINE__, tokens[0].c_str()); \
            LogManager::getSingleton().logMessage(msgBuf); \
        } \
    }
#else
#   define OGRE_CHECK_GL_ERROR(glFunc) { glFunc; }
#endif

#define OCGE OGRE_CHECK_GL_ERROR

#if ENABLE_GL_CHECK
    #define EGL_CHECK_ERROR \
    { \
        int e = eglGetError(); \
        if ((e != 0) && (e != EGL_SUCCESS))\
        { \
            char msgBuf[4096]; \
            sprintf(msgBuf, "EGL error 0x%04X in %s at line %i\n", e, __PRETTY_FUNCTION__, __LINE__); \
            LogManager::getSingleton().logMessage(msgBuf); \
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, msgBuf, __PRETTY_FUNCTION__); \
        } \
    }
#else
    #define EGL_CHECK_ERROR {}
#endif

#endif
