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

#include "OgreGLES2Exports.h"
#include "OgreGLES2Config.h"

namespace Ogre {
    class GLContext;
    typedef GLContext GLES2Context;
}

#include <GLES3/glesw.h>

#if (OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS)
#   ifdef __OBJC__
#       include <OpenGLES/EAGL.h>
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
#endif

#if (OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS)
namespace Ogre {
extern float EAGLCurrentOSVersion;
}
#define OGRE_IF_IOS_VERSION_IS_GREATER_THAN(vers) \
    if(EAGLCurrentOSVersion >= vers)
#else
#define OGRE_IF_IOS_VERSION_IS_GREATER_THAN(vers)
#endif

#define getGLES2RenderSystem() dynamic_cast<GLES2RenderSystem*>(Root::getSingleton().getRenderSystem())

// Copy this definition from desktop GL.  Used for polygon modes.
#ifndef GL_FILL
#   define GL_FILL    0x1B02
#endif

namespace Ogre {
    class GLNativeSupport;
    class GLES2GpuProgram;
    class GLES2Texture;
    typedef shared_ptr<GLES2GpuProgram> GLES2GpuProgramPtr;
    typedef shared_ptr<GLES2Texture> GLES2TexturePtr;
};

#if OGRE_NO_GLES3_SUPPORT == 0
#undef GL_DEPTH_COMPONENT32_OES
#define GL_DEPTH_COMPONENT32_OES GL_DEPTH_COMPONENT32F
#endif

#if (OGRE_PLATFORM == OGRE_PLATFORM_WIN32)
// an error in all windows gles sdks...
#   undef GL_OES_get_program_binary
#endif

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

#endif
