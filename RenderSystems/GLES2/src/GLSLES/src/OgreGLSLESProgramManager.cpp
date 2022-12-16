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

#include "OgreGLSLESProgramManager.h"
#include "OgreGLSLESProgram.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreGpuProgramManager.h"
#include "OgreHardwareBufferManager.h"
#include "OgreGLSLESProgram.h"

#include "OgreGLSLESLinkProgram.h"
#include "OgreGLSLESProgramPipeline.h"

#include "OgreRoot.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    template<> GLSLESProgramManager* Singleton<GLSLESProgramManager>::msSingleton = 0;

    //-----------------------------------------------------------------------
    GLSLESProgramManager* GLSLESProgramManager::getSingletonPtr(void)
    {
        return msSingleton;
    }

    //-----------------------------------------------------------------------
    GLSLESProgramManager& GLSLESProgramManager::getSingleton(void)
    {
        assert( msSingleton );  return ( *msSingleton );
    }

    //-----------------------------------------------------------------------
    GLSLESProgramManager::GLSLESProgramManager(void)
    {
        
#if !OGRE_NO_GLES2_GLSL_OPTIMISER
#if OGRE_NO_GLES3_SUPPORT == 0
        mGLSLOptimiserContext = glslopt_initialize(kGlslTargetOpenGLES30);
#else
        mGLSLOptimiserContext = glslopt_initialize(kGlslTargetOpenGLES20);
#endif
#endif
    }

    //-----------------------------------------------------------------------
    GLSLESProgramManager::~GLSLESProgramManager(void)
    {
#if !OGRE_NO_GLES2_GLSL_OPTIMISER
        if(mGLSLOptimiserContext)
        {
            glslopt_cleanup(mGLSLOptimiserContext);
            mGLSLOptimiserContext = NULL;
        }
#endif
    }

    //-----------------------------------------------------------------------
    GLSLESProgramCommon* GLSLESProgramManager::getActiveProgram(void)
    {
        // If there is an active link program then return it
        if (mActiveProgram)
            return static_cast<GLSLESProgramCommon*>(mActiveProgram);;

        // No active link program so find one or make a new one
        // Is there an active key?
        uint32 activeKey = 0;
        for(auto shader : mActiveShader)
        {
            if(!shader) continue;
            activeKey = HashCombine(activeKey, shader->getShaderID());
        }

        // Only return a link program object if a vertex or fragment program exist
        if (activeKey > 0)
        {
            // Find the key in the hash map
            ProgramIterator programFound = mPrograms.find(activeKey);
            // Program object not found for key so need to create it
            if (programFound == mPrograms.end())
            {
                if (Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(
                        RSC_SEPARATE_SHADER_OBJECTS))
                {
                    mActiveProgram = new GLSLESProgramPipeline(mActiveShader);
                }
                else
                {
                    mActiveProgram = new GLSLESLinkProgram(mActiveShader);
                }

                mPrograms[activeKey] = mActiveProgram;
            }
            else
            {
                // Found a link program in map container so make it active
                mActiveProgram = static_cast<GLSLESLinkProgram*>(programFound->second);
            }

        }
        // Make the program object active
        if (mActiveProgram) mActiveProgram->activate();

        return static_cast<GLSLESProgramCommon*>(mActiveProgram);
    }

#if !OGRE_NO_GLES2_GLSL_OPTIMISER
    void GLSLESProgramManager::optimiseShaderSource(GLSLESGpuProgram* gpuProgram)
    {
        if(!gpuProgram->getGLSLProgram()->getIsOptimised())
        {
            GpuProgramType gpuType = gpuProgram->getType();
            const glslopt_shader_type shaderType = (gpuType == GPT_VERTEX_PROGRAM) ? kGlslOptShaderVertex : kGlslOptShaderFragment;
            String shaderSource = gpuProgram->getGLSLProgram()->getSource();
            glslopt_shader* shader = glslopt_optimize(mGLSLOptimiserContext, shaderType, shaderSource.c_str(), 0);

            StringStream os;
            if(glslopt_get_status(shader))
            {
                const String source = glslopt_get_output(shader);
                gpuProgram->getGLSLProgram()->setOptimisedSource(source);
                gpuProgram->getGLSLProgram()->setIsOptimised(true);
            }
            else
            {
                LogManager::getSingleton().logMessage("Error from GLSL Optimiser, disabling optimisation for program: " + gpuProgram->getName());
                gpuProgram->getGLSLProgram()->setParameter("use_optimiser", "false");
                //LogManager::getSingleton().logMessage(String(glslopt_get_log(shader)));
                //LogManager::getSingleton().logMessage("Original Shader");
                //LogManager::getSingleton().logMessage(gpuProgram->getGLSLProgram()->getSource());
                //LogManager::getSingleton().logMessage("Optimized Shader");
                //LogManager::getSingleton().logMessage(os.str());
            }
            glslopt_shader_delete(shader);
        }
    }
#endif

    //---------------------------------------------------------------------
    void GLSLESProgramManager::extractUniforms(GLuint programObject,
                                               const GpuConstantDefinitionMap* vertexConstantDefs,
                                               const GpuConstantDefinitionMap* fragmentConstantDefs,
                                               GLUniformReferenceList& list)
    {
        // Scan through the active uniforms and add them to the reference list
        GLint uniformCount = 0;
        #define uniformLength 200
        char uniformName[uniformLength] = {0};

        GLUniformReference newGLUniformReference;

        // Get the number of active uniforms
        OGRE_CHECK_GL_ERROR(glGetProgramiv(programObject, GL_ACTIVE_UNIFORMS, &uniformCount));

        const GpuConstantDefinitionMap* params[6] = { vertexConstantDefs, fragmentConstantDefs };

        // Loop over each of the active uniforms, and add them to the reference container
        // only do this for user defined uniforms, ignore built in gl state uniforms
        for (GLuint index = 0; index < (GLuint)uniformCount; index++)
        {
            GLint arraySize = 0;
            GLenum glType = GL_NONE;
            OGRE_CHECK_GL_ERROR(glGetActiveUniform(programObject, index, uniformLength, NULL,
                &arraySize, &glType, uniformName));

            // Don't add built in uniforms
            newGLUniformReference.mLocation = glGetUniformLocation(programObject, uniformName);

            if(!validateParam(uniformName, arraySize, params, newGLUniformReference))
                continue;
            list.push_back(newGLUniformReference);
        }

#if 0 // needs updating to GL3Plus code
        // Now deal with uniform blocks

        GLint blockCount = 0;

        OGRE_CHECK_GL_ERROR(glGetProgramiv(programObject, GL_ACTIVE_UNIFORM_BLOCKS, &blockCount));

        for (int index = 0; index < blockCount; index++)
        {
            OGRE_CHECK_GL_ERROR(glGetActiveUniformBlockName(programObject, index, uniformLength, NULL, uniformName));

            GpuSharedParametersPtr blockSharedParams = GpuProgramManager::getSingleton().getSharedParameters(uniformName);

            GLint blockSize, blockBinding;
            OGRE_CHECK_GL_ERROR(glGetActiveUniformBlockiv(programObject, index, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize));
            OGRE_CHECK_GL_ERROR(glGetActiveUniformBlockiv(programObject, index, GL_UNIFORM_BLOCK_BINDING, &blockBinding));
            HardwareUniformBufferSharedPtr newUniformBuffer = HardwareBufferManager::getSingleton().createUniformBuffer(blockSize, HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE, false, uniformName);

            GLES2HardwareUniformBuffer* hwGlBuffer = static_cast<GLES2HardwareUniformBuffer*>(newUniformBuffer.get());
            hwGlBuffer->setGLBufferBinding(blockBinding);
            sharedList.push_back(newUniformBuffer);
        }
#endif
    }
}
