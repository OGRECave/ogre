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

#include "OgreGLSLESProgramCommon.h"
#include "OgreGLSLESProgram.h"
#include "OgreGpuProgramManager.h"
#include "OgreGLUtil.h"
#include "OgreGLES2RenderSystem.h"
#include "OgreRoot.h"
#include "OgreGLNativeSupport.h"

namespace Ogre {
    
    //-----------------------------------------------------------------------
    GLSLESProgramCommon::GLSLESProgramCommon(const GLShaderList& shaders)
    : GLSLProgramCommon(shaders)
    {
    }

    void GLSLESProgramCommon::bindFixedAttributes(GLuint program)
    {
        GLint maxAttribs = Root::getSingleton().getRenderSystem()->getCapabilities()->getNumVertexAttributes();

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        // must query active attributes on OSX to avoid warning spam
        OGRE_CHECK_GL_ERROR(glLinkProgram( program ));
#endif

        size_t numAttribs = sizeof(msCustomAttributes) / sizeof(CustomAttribute);
        for (size_t i = 0; i < numAttribs; ++i)
        {
            const CustomAttribute& a = msCustomAttributes[i];
            if (a.attrib < maxAttribs)
            {
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
                if(glGetAttribLocation(program, a.name) == -1) continue;
#endif
                OGRE_CHECK_GL_ERROR(glBindAttribLocation(program, a.attrib, a.name));
            }
        }
    }

    //-----------------------------------------------------------------------
    bool GLSLESProgramCommon::getMicrocodeFromCache(uint32 id, GLuint programHandle)
    {
        if (!GpuProgramManager::canGetCompiledShaderBuffer())
            return false;

        if (!GpuProgramManager::getSingleton().isMicrocodeAvailableInCache(id))
            return false;

        GpuProgramManager::Microcode cacheMicrocode = GpuProgramManager::getSingleton().getMicrocodeFromCache(id);

        // turns out we need this param when loading
        GLenum binaryFormat = 0;

        cacheMicrocode->seek(0);

        // get size of binary
        cacheMicrocode->read(&binaryFormat, sizeof(GLenum));

        if(!Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_CAN_GET_COMPILED_SHADER_BUFFER))
            return false;

        GLint binaryLength = static_cast<GLint>(cacheMicrocode->size() - sizeof(GLenum));

        // load binary
        OGRE_CHECK_GL_ERROR(glProgramBinaryOES(programHandle,
                           binaryFormat,
                           cacheMicrocode->getCurrentPtr(),
                           binaryLength));

        GLint success = 0;
        OGRE_CHECK_GL_ERROR(glGetProgramiv(programHandle, GL_LINK_STATUS, &success));

        return success;
    }
    void GLSLESProgramCommon::_writeToCache(uint32 id, GLuint programHandle)
    {
        if(!GpuProgramManager::canGetCompiledShaderBuffer())
            return;

        if(!GpuProgramManager::getSingleton().getSaveMicrocodesToCache())
            return;

        // Add to the microcode to the cache
        // Get buffer size
        GLint binaryLength = 0;
        OGRE_CHECK_GL_ERROR(glGetProgramiv(programHandle, GL_PROGRAM_BINARY_LENGTH_OES, &binaryLength));

        // Create microcode
        auto newMicrocode = GpuProgramManager::createMicrocode(static_cast<uint32>(binaryLength + sizeof(GLenum)));

        // Get binary
        OGRE_CHECK_GL_ERROR(glGetProgramBinaryOES(programHandle, binaryLength, NULL, (GLenum *)newMicrocode->getPtr(),
                                                  newMicrocode->getPtr() + sizeof(GLenum)));

        // Add to the microcode to the cache
        GpuProgramManager::getSingleton().addMicrocodeToCache(id, newMicrocode);
    }

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID || OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
    void GLSLESProgramCommon::notifyOnContextLost()
    {
        mLinked = false;
        mUniformRefsBuilt = false;
        for(auto s : mShaders)
        {
            if(s) s->getUniformCache()->clearCache();
        }
    }

    void GLSLESProgramCommon::notifyOnContextReset()
    {
        activate();
    }
#endif
} // namespace Ogre
