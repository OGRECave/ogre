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

#include "OgreGLSLESLinkProgram.h"
#include "OgreGLSLESProgram.h"
#include "OgreGLSLESProgramManager.h"
#include "OgreLogManager.h"
#include "OgreGpuProgramManager.h"
#include "OgreStringConverter.h"
#include "OgreRoot.h"
#include "OgreGLUtil.h"
#include "OgreGLES2RenderSystem.h"
#include "OgreGLNativeSupport.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    GLSLESLinkProgram::GLSLESLinkProgram(const GLShaderList& shaders)
    : GLSLESProgramCommon(shaders)
    {
    }

    //-----------------------------------------------------------------------
    GLSLESLinkProgram::~GLSLESLinkProgram(void)
    {
        OGRE_CHECK_GL_ERROR(glDeleteProgram(mGLProgramHandle));
    }

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID || OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
    void GLSLESLinkProgram::notifyOnContextLost()
    {
        OGRE_CHECK_GL_ERROR(glDeleteProgram(mGLProgramHandle));
        mGLProgramHandle = 0;
        GLSLESProgramCommon::notifyOnContextLost();
    }
#endif
    //-----------------------------------------------------------------------
    void GLSLESLinkProgram::activate(void)
    {
        if (!mLinked)
        {
            glGetError(); // Clean up the error. Otherwise will flood log.

            OGRE_CHECK_GL_ERROR(mGLProgramHandle = glCreateProgram());

            uint32 hash = getCombinedHash();
            mLinked = getMicrocodeFromCache(hash, mGLProgramHandle);
            if (!mLinked)
            {
#if !OGRE_NO_GLES2_GLSL_OPTIMISER
                // Check CmdParams for each shader type to see if we should optimize
                if(mVertexProgram)
                {
                    String paramStr = mVertexProgram->getGLSLProgram()->getParameter("use_optimiser");
                    if((paramStr == "true") || paramStr.empty())
                    {
                        GLSLESLinkProgramManager::getSingleton().optimiseShaderSource(mVertexProgram);
                    }
                }

                if(mFragmentProgram)
                {
                    String paramStr = mFragmentProgram->getGLSLProgram()->getParameter("use_optimiser");
                    if((paramStr == "true") || paramStr.empty())
                    {
                        GLSLESLinkProgramManager::getSingleton().optimiseShaderSource(mFragmentProgram);
                    }
                }
#endif
                compileAndLink();

#if !OGRE_NO_GLES2_GLSL_OPTIMISER
                // TODO: we will never reach this - move to GLSLESProgram
                // Try it again when we used the optimised versions
                if(mVertexProgram->getGLSLProgram()->getOptimiserEnabled() && 
                    mFragmentProgram->getGLSLProgram()->getOptimiserEnabled())
                {
                    LogManager::getSingleton().stream() << "Try not optimised shader."; 
                    mVertexProgram->getGLSLProgram()->setOptimiserEnabled(false);
                    mFragmentProgram->getGLSLProgram()->setOptimiserEnabled(false);
                    compileAndLink();
                }
#endif
            }

            buildGLUniformReferences();
        }

        if (mLinked)
        {
            OGRE_CHECK_GL_ERROR(glUseProgram( mGLProgramHandle ));
        }
    }

    //-----------------------------------------------------------------------
    void GLSLESLinkProgram::compileAndLink()
    {
        uint32 hash = getCombinedHash();

        // attach Vertex Program
        mShaders[GPT_VERTEX_PROGRAM]->attachToProgramObject(mGLProgramHandle);
        // attach Fragment Program
        mShaders[GPT_FRAGMENT_PROGRAM]->attachToProgramObject(mGLProgramHandle);
        
        bindFixedAttributes( mGLProgramHandle );

        // The link
        OGRE_CHECK_GL_ERROR(glLinkProgram( mGLProgramHandle ));
        OGRE_CHECK_GL_ERROR(glGetProgramiv( mGLProgramHandle, GL_LINK_STATUS, &mLinked ));

        GLSLES::logObjectInfo( getCombinedName() + String("GLSL link result : "), mGLProgramHandle );

        const RenderSystemCapabilities* caps = Root::getSingleton().getRenderSystem()->getCapabilities();

        if(caps->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
        {
            if(glIsProgramPipelineEXT(mGLProgramHandle))
                glValidateProgramPipelineEXT(mGLProgramHandle);
        }
        else if(glIsProgram(mGLProgramHandle))
        {
            glValidateProgram(mGLProgramHandle);
        }

        GLSLES::logObjectInfo( getCombinedName() + String(" GLSL validation result : "), mGLProgramHandle );

        if(mLinked)
        {
            _writeToCache(hash, mGLProgramHandle);
        }
    }

    //-----------------------------------------------------------------------
    void GLSLESLinkProgram::buildGLUniformReferences(void)
    {
        if (!mUniformRefsBuilt)
        {
            const GpuConstantDefinitionMap* vertParams = 0;
            const GpuConstantDefinitionMap* fragParams = 0;
            if (mShaders[GPT_VERTEX_PROGRAM])
            {
                vertParams = &(mShaders[GPT_VERTEX_PROGRAM]->getConstantDefinitions().map);
            }
            if (mShaders[GPT_FRAGMENT_PROGRAM])
            {
                fragParams = &(mShaders[GPT_FRAGMENT_PROGRAM]->getConstantDefinitions().map);
            }

            GLSLESProgramManager::extractUniforms(mGLProgramHandle, vertParams, fragParams,
                                                  mGLUniformReferences);

            mUniformRefsBuilt = true;
        }
    }

    //-----------------------------------------------------------------------
    void GLSLESLinkProgram::updateUniforms(GpuProgramParametersSharedPtr params, 
        uint16 mask, GpuProgramType fromProgType)
    {
        // Iterate through uniform reference list and update uniform values
        GLUniformReferenceIterator currentUniform = mGLUniformReferences.begin();
        GLUniformReferenceIterator endUniform = mGLUniformReferences.end();

        for (;currentUniform != endUniform; ++currentUniform)
        {
            // Only pull values from buffer it's supposed to be in (vertex or fragment)
            // This method will be called twice, once for vertex program params, 
            // and once for fragment program params.
            if (fromProgType == currentUniform->mSourceProgType)
            {
                const GpuConstantDefinition* def = currentUniform->mConstantDef;
                if (def->variability & mask)
                {
                    GLsizei glArraySize = (GLsizei)def->arraySize;

                    // this is a monolitic program so we can use the cache of any attached shader
                    GLUniformCache* uniformCache =  mShaders[GPT_VERTEX_PROGRAM]->getUniformCache();
                    void* val = def->isSampler() ? (void*)params->getRegPointer(def->physicalIndex)
                                                 : (void*)params->getFloatPointer(def->physicalIndex);

                    bool shouldUpdate = uniformCache->updateUniform(currentUniform->mLocation, val,
                                                                    def->elementSize * glArraySize * 4);
                    if(!shouldUpdate)
                        continue;

                    // Get the index in the parameter real list
                    switch (def->constType)
                    {
                    case GCT_FLOAT1:
                        OGRE_CHECK_GL_ERROR(glUniform1fv(currentUniform->mLocation, glArraySize, 
                                                         params->getFloatPointer(def->physicalIndex)));
                        break;
                    case GCT_FLOAT2:
                        OGRE_CHECK_GL_ERROR(glUniform2fv(currentUniform->mLocation, glArraySize, 
                                                         params->getFloatPointer(def->physicalIndex)));
                        break;
                    case GCT_FLOAT3:
                        OGRE_CHECK_GL_ERROR(glUniform3fv(currentUniform->mLocation, glArraySize, 
                                                         params->getFloatPointer(def->physicalIndex)));
                        break;
                    case GCT_FLOAT4:
                        OGRE_CHECK_GL_ERROR(glUniform4fv(currentUniform->mLocation, glArraySize, 
                                                         params->getFloatPointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_2X2:
                        OGRE_CHECK_GL_ERROR(glUniformMatrix2fv(currentUniform->mLocation, glArraySize, 
                                                               GL_FALSE, params->getFloatPointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_3X3:
                        OGRE_CHECK_GL_ERROR(glUniformMatrix3fv(currentUniform->mLocation, glArraySize, 
                                                               GL_FALSE, params->getFloatPointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_4X4:
                        OGRE_CHECK_GL_ERROR(glUniformMatrix4fv(currentUniform->mLocation, glArraySize, 
                                                               GL_FALSE, params->getFloatPointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_2X3:
                        OGRE_CHECK_GL_ERROR(glUniformMatrix2x3fv(currentUniform->mLocation, glArraySize,
                                                                 GL_FALSE, params->getFloatPointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_2X4:
                        OGRE_CHECK_GL_ERROR(glUniformMatrix2x4fv(currentUniform->mLocation, glArraySize,
                                                                 GL_FALSE, params->getFloatPointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_3X2:
                        OGRE_CHECK_GL_ERROR(glUniformMatrix3x2fv(currentUniform->mLocation, glArraySize,
                                                                 GL_FALSE, params->getFloatPointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_3X4:
                        OGRE_CHECK_GL_ERROR(glUniformMatrix3x4fv(currentUniform->mLocation, glArraySize,
                                                                 GL_FALSE, params->getFloatPointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_4X2:
                        OGRE_CHECK_GL_ERROR(glUniformMatrix4x2fv(currentUniform->mLocation, glArraySize,
                                                                 GL_FALSE, params->getFloatPointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_4X3:
                        OGRE_CHECK_GL_ERROR(glUniformMatrix4x3fv(currentUniform->mLocation, glArraySize, 
                                                                 GL_FALSE, params->getFloatPointer(def->physicalIndex)));
                        break;
                    case GCT_SAMPLER1D:
                    case GCT_SAMPLER1DSHADOW:
                    case GCT_SAMPLER2D:
                    case GCT_SAMPLER2DSHADOW:
                    case GCT_SAMPLER3D:
                    case GCT_SAMPLERCUBE:
                    case GCT_SAMPLER2DARRAY:
                        // Samplers handled like 1-element ints
                    case GCT_INT1:
                        OGRE_CHECK_GL_ERROR(glUniform1iv(currentUniform->mLocation, glArraySize,
                                                         (GLint*)val));
                        break;
                    case GCT_INT2:
                        OGRE_CHECK_GL_ERROR(glUniform2iv(currentUniform->mLocation, glArraySize, 
                                                         (GLint*)params->getIntPointer(def->physicalIndex)));
                        break;
                    case GCT_INT3:
                        OGRE_CHECK_GL_ERROR(glUniform3iv(currentUniform->mLocation, glArraySize, 
                                                         (GLint*)params->getIntPointer(def->physicalIndex)));
                        break;
                    case GCT_INT4:
                        OGRE_CHECK_GL_ERROR(glUniform4iv(currentUniform->mLocation, glArraySize, 
                                                         (GLint*)params->getIntPointer(def->physicalIndex)));
                        break;
                    case GCT_UNKNOWN:
                    case GCT_SPECIALIZATION:
                    case GCT_DOUBLE1:
                    case GCT_DOUBLE2:
                    case GCT_DOUBLE3:
                    case GCT_DOUBLE4:
                    case GCT_MATRIX_DOUBLE_2X2:
                    case GCT_MATRIX_DOUBLE_2X3:
                    case GCT_MATRIX_DOUBLE_2X4:
                    case GCT_MATRIX_DOUBLE_3X2:
                    case GCT_MATRIX_DOUBLE_3X3:
                    case GCT_MATRIX_DOUBLE_3X4:
                    case GCT_MATRIX_DOUBLE_4X2:
                    case GCT_MATRIX_DOUBLE_4X3:
                    case GCT_MATRIX_DOUBLE_4X4:
                    default:
                        break;

                    } // End switch
                } // Variability & mask
            } // fromProgType == currentUniform->mSourceProgType
  
        } // End for
    }
} // namespace Ogre
