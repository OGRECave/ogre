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

#include "OgreGLSLProgram.h"
#include "OgreGLSLShader.h"
#include "OgreGpuProgramManager.h"
#include "OgreGLSLShader.h"
#include "OgreRoot.h"
#include "OgreGLSLExtSupport.h"
#include "OgreLogManager.h"

namespace Ogre {

    GLSLProgram::GLSLProgram(const GLShaderList& shaders)
        : GLSLProgramCommon(shaders)
    {
    }

    void GLSLProgram::bindFixedAttributes(GLuint program)
    {
        GLint maxAttribs = Root::getSingleton().getRenderSystem()->getCapabilities()->getNumVertexAttributes();

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
        // must query active attributes on OSX to avoid warning spam
        OGRE_CHECK_GL_ERROR(glLinkProgram( program ));
#endif

        size_t numAttribs = sizeof(msCustomAttributes) / sizeof(CustomAttribute);
        for (size_t i = 0; i < numAttribs; ++i)
        {
            const CustomAttribute& a = msCustomAttributes[i];
            if (a.attrib < maxAttribs)
            {
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
                if(glGetAttribLocation(program, a.name) == -1) continue;
#endif
                OGRE_CHECK_GL_ERROR(glBindAttribLocation(program, a.attrib, a.name));
            }
        }
    }

    void GLSLProgram::setTransformFeedbackVaryings(const std::vector<String>& nameStrings)
    {
        // Get program object ID.
        GLuint programId;
        if (Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
        {
            //TODO include tessellation stages
            auto glslGpuProgram = mShaders[GPT_GEOMETRY_PROGRAM];
            if (!glslGpuProgram)
                glslGpuProgram = mShaders[GPT_VERTEX_PROGRAM];

            programId = glslGpuProgram->getGLProgramHandle();

            // force re-link
            GpuProgramManager::getSingleton().removeMicrocodeFromCache(glslGpuProgram->_getHash());
            glslGpuProgram->resetLinked();
        }
        else
        {
            programId = getGLProgramHandle();

            // force re-link
            GpuProgramManager::getSingleton().removeMicrocodeFromCache(getCombinedHash());
        }
        mLinked = false;

        // Convert to const char * for GL
        std::vector<const char*> names;
        for (uint e = 0; e < nameStrings.size(); e++)
        {
            names.push_back(nameStrings[e].c_str());
        }

        // TODO replace glTransformFeedbackVaryings with in-shader specification (GL 4.4)
        OGRE_CHECK_GL_ERROR(glTransformFeedbackVaryings(programId, nameStrings.size(), &names[0],
                                                        GL_INTERLEAVED_ATTRIBS));

#if OGRE_DEBUG_MODE
        activate();
        // Check if varyings were successfully set.
        GLchar Name[64];
        GLsizei Length(0);
        GLsizei Size(0);
        GLenum Type(0);
        // bool Validated = false;
        for (size_t i = 0; i < nameStrings.size(); i++)
        {
            OGRE_CHECK_GL_ERROR(
                glGetTransformFeedbackVarying(programId, i, 64, &Length, &Size, &Type, Name));
            LogManager::getSingleton().stream() << "Varying " << i << ": " << Name << " " << Length
                                                << " " << Size << " " << Type;
            // Validated = (Size == 1) && (Type == GL_FLOAT_VEC3);
            // std::cout << Validated << " " << GL_FLOAT_VEC3 << std::endl;
        }
#endif
    }

    bool GLSLProgram::getMicrocodeFromCache(uint32 id, GLuint programHandle)
    {
        if (!GpuProgramManager::canGetCompiledShaderBuffer())
            return false;

        if (!GpuProgramManager::getSingleton().isMicrocodeAvailableInCache(id))
            return false;

        GpuProgramManager::Microcode cacheMicrocode =
            GpuProgramManager::getSingleton().getMicrocodeFromCache(id);

        cacheMicrocode->seek(0);

        // Turns out we need this param when loading.
        GLenum binaryFormat = 0;
        cacheMicrocode->read(&binaryFormat, sizeof(GLenum));

        // Get size of binary.
        GLint binaryLength = static_cast<GLint>(cacheMicrocode->size() - sizeof(GLenum));

        // Load binary.
        OGRE_CHECK_GL_ERROR(glProgramBinary(programHandle,
                                            binaryFormat,
                                            cacheMicrocode->getCurrentPtr(),
                                            binaryLength));

        GLint success = 0;
        OGRE_CHECK_GL_ERROR(glGetProgramiv(programHandle, GL_LINK_STATUS, &success));

        if(!success)
            logObjectInfo("could not load from cache", programHandle);

        return success;
    }

    void GLSLProgram::writeMicrocodeToCache(uint32 id, GLuint programHandle)
    {
        if (!GpuProgramManager::getSingleton().getSaveMicrocodesToCache())
            return;

        // get buffer size
        GLint binaryLength = 0;
        OGRE_CHECK_GL_ERROR(glGetProgramiv(programHandle, GL_PROGRAM_BINARY_LENGTH, &binaryLength));

        // create microcode
        GpuProgramManager::Microcode newMicrocode =
            GpuProgramManager::getSingleton().createMicrocode(binaryLength + sizeof(GLenum));

        // get binary
        OGRE_CHECK_GL_ERROR(glGetProgramBinary(programHandle, binaryLength, NULL,
                                               (GLenum*)newMicrocode->getPtr(),
                                               newMicrocode->getPtr() + sizeof(GLenum)));

        // add to the microcode to the cache
        GpuProgramManager::getSingleton().addMicrocodeToCache(id, newMicrocode);
    }

} // namespace Ogre
