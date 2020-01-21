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

#include "OgreGLSLMonolithicProgram.h"
#include "OgreGLSLExtSupport.h"
#include "OgreGLSLShader.h"
#include "OgreGLSLProgramManager.h"
#include "OgreGL3PlusRenderSystem.h"
#include "OgreStringVector.h"
#include "OgreLogManager.h"
#include "OgreGpuProgramManager.h"
#include "OgreStringConverter.h"

namespace Ogre {

    GLSLMonolithicProgram::GLSLMonolithicProgram(const GLShaderList& shaders)
        : GLSLProgram(shaders)
    {
    }


    GLSLMonolithicProgram::~GLSLMonolithicProgram(void)
    {
        OGRE_CHECK_GL_ERROR(glDeleteProgram(mGLProgramHandle));
    }

    void GLSLMonolithicProgram::activate(void)
    {
        if (!mLinked)
        {
            uint32 hash = getCombinedHash();

            if(mGLProgramHandle == 0)
            {
                OGRE_CHECK_GL_ERROR(mGLProgramHandle = glCreateProgram());
            }

            mLinked = getMicrocodeFromCache(hash, mGLProgramHandle);
            if (!mLinked)
            {
                // Something must have changed since the program binaries
                // were cached away. Fallback to source shader loading path,
                // and then retrieve and cache new program binaries once again.
                compileAndLink();
            }

            extractLayoutQualifiers();
            buildGLUniformReferences();
        }

        if (mLinked)
        {
            OGRE_CHECK_GL_ERROR(glUseProgram(mGLProgramHandle));
        }
    }


    void GLSLMonolithicProgram::compileAndLink()
    {
        for (auto shader : mShaders)
        {
        	if(!shader) continue;

            shader->attachToProgramObject(mGLProgramHandle);
        }

        bindFixedAttributes(mGLProgramHandle);

        // the link
        OGRE_CHECK_GL_ERROR(glLinkProgram( mGLProgramHandle ));
        OGRE_CHECK_GL_ERROR(glGetProgramiv( mGLProgramHandle, GL_LINK_STATUS, &mLinked ));

        logObjectInfo( getCombinedName() + String(" GLSL link result : "), mGLProgramHandle );

        if(glIsProgram(mGLProgramHandle))
        {
            OGRE_CHECK_GL_ERROR(glValidateProgram(mGLProgramHandle));
        }
        logObjectInfo( getCombinedName() + String(" GLSL validation result : "), mGLProgramHandle );

        if(mLinked)
        {
            writeMicrocodeToCache(getCombinedHash(), mGLProgramHandle);
        }
    }


    void GLSLMonolithicProgram::buildGLUniformReferences(void)
    {
        if (mUniformRefsBuilt)
        {
            return;
        }

        // order must match GpuProgramType
        const GpuConstantDefinitionMap* params[6] = { NULL };

        for (int i = 0; i < 6; i++)
        {
            if (!mShaders[i])
                continue;

            params[i] = &(mShaders[i]->getConstantDefinitions().map);
        }

        // Do we know how many shared params there are yet? Or if there are any blocks defined?
        GLSLProgramManager::getSingleton().extractUniformsFromProgram(
            mGLProgramHandle, params, mGLUniformReferences, mGLAtomicCounterReferences,
            mGLCounterBufferReferences);

        mUniformRefsBuilt = true;
    }


    void GLSLMonolithicProgram::updateUniforms(GpuProgramParametersSharedPtr params,
                                               uint16 mask, GpuProgramType fromProgType)
    {
        // Iterate through uniform reference list and update uniform values
        GLUniformReferenceIterator currentUniform = mGLUniformReferences.begin();
        GLUniformReferenceIterator endUniform = mGLUniformReferences.end();

        // determine if we need to transpose matrices when binding
        bool transpose = !mShaders[fromProgType] || mShaders[fromProgType]->getColumnMajorMatrices();

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
                    case GCT_DOUBLE1:
                        OGRE_CHECK_GL_ERROR(glUniform1dv(currentUniform->mLocation, glArraySize,
                                                         params->getDoublePointer(def->physicalIndex)));
                        break;
                    case GCT_DOUBLE2:
                        OGRE_CHECK_GL_ERROR(glUniform2dv(currentUniform->mLocation, glArraySize,
                                                         params->getDoublePointer(def->physicalIndex)));
                        break;
                    case GCT_DOUBLE3:
                        OGRE_CHECK_GL_ERROR(glUniform3dv(currentUniform->mLocation, glArraySize,
                                                         params->getDoublePointer(def->physicalIndex)));
                        break;
                    case GCT_DOUBLE4:
                        OGRE_CHECK_GL_ERROR(glUniform4dv(currentUniform->mLocation, glArraySize,
                                                         params->getDoublePointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_DOUBLE_2X2:
                        OGRE_CHECK_GL_ERROR(glUniformMatrix2dv(currentUniform->mLocation, glArraySize,
                                                               transpose, params->getDoublePointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_DOUBLE_2X3:
                        OGRE_CHECK_GL_ERROR(glUniformMatrix2x3dv(currentUniform->mLocation, glArraySize,
                                                                 GL_FALSE, params->getDoublePointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_DOUBLE_2X4:
                        OGRE_CHECK_GL_ERROR(glUniformMatrix2x4dv(currentUniform->mLocation, glArraySize,
                                                                 GL_FALSE, params->getDoublePointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_DOUBLE_3X2:
                        OGRE_CHECK_GL_ERROR(glUniformMatrix3x2dv(currentUniform->mLocation, glArraySize,
                                                                 GL_FALSE, params->getDoublePointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_DOUBLE_3X3:
                        OGRE_CHECK_GL_ERROR(glUniformMatrix3dv(currentUniform->mLocation, glArraySize,
                                                               transpose, params->getDoublePointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_DOUBLE_3X4:
                        OGRE_CHECK_GL_ERROR(glUniformMatrix3x4dv(currentUniform->mLocation, glArraySize,
                                                                 GL_FALSE, params->getDoublePointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_DOUBLE_4X2:
                        OGRE_CHECK_GL_ERROR(glUniformMatrix4x2dv(currentUniform->mLocation, glArraySize,
                                                                 GL_FALSE, params->getDoublePointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_DOUBLE_4X3:
                        OGRE_CHECK_GL_ERROR(glUniformMatrix4x3dv(currentUniform->mLocation, glArraySize,
                                                                 GL_FALSE, params->getDoublePointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_DOUBLE_4X4:
                        OGRE_CHECK_GL_ERROR(glUniformMatrix4dv(currentUniform->mLocation, glArraySize,
                                                               transpose, params->getDoublePointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_2X2:
                        OGRE_CHECK_GL_ERROR(glUniformMatrix2fv(currentUniform->mLocation, glArraySize,
                                                               transpose, params->getFloatPointer(def->physicalIndex)));
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
                    case GCT_MATRIX_3X3:
                        OGRE_CHECK_GL_ERROR(glUniformMatrix3fv(currentUniform->mLocation, glArraySize,
                                                               transpose, params->getFloatPointer(def->physicalIndex)));
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
                    case GCT_MATRIX_4X4:
                        OGRE_CHECK_GL_ERROR(glUniformMatrix4fv(currentUniform->mLocation, glArraySize,
                                                               transpose, params->getFloatPointer(def->physicalIndex)));
                        break;
                    case GCT_SAMPLER1D:
                    case GCT_SAMPLER1DSHADOW:
                    case GCT_SAMPLER2D:
                    case GCT_SAMPLER2DSHADOW:
                    case GCT_SAMPLER2DARRAY:
                    case GCT_SAMPLER3D:
                    case GCT_SAMPLERCUBE:
                    case GCT_SAMPLERRECT:
                        // Samplers handled like 1-element ints
                    case GCT_INT1:
                        OGRE_CHECK_GL_ERROR(glUniform1iv(currentUniform->mLocation, glArraySize,
                                                         (GLint*)params->getIntPointer(def->physicalIndex)));
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
                    case GCT_UINT1:
                    case GCT_BOOL1:
                        OGRE_CHECK_GL_ERROR(glUniform1uiv(currentUniform->mLocation, glArraySize,
                                                          (GLuint*)params->getUnsignedIntPointer(def->physicalIndex)));
                        break;
                    case GCT_UINT2:
                    case GCT_BOOL2:
                        OGRE_CHECK_GL_ERROR(glUniform2uiv(currentUniform->mLocation, glArraySize,
                                                          (GLuint*)params->getUnsignedIntPointer(def->physicalIndex)));
                        break;
                    case GCT_UINT3:
                    case GCT_BOOL3:
                        OGRE_CHECK_GL_ERROR(glUniform3uiv(currentUniform->mLocation, glArraySize,
                                                          (GLuint*)params->getUnsignedIntPointer(def->physicalIndex)));
                        break;
                    case GCT_UINT4:
                    case GCT_BOOL4:
                        OGRE_CHECK_GL_ERROR(glUniform4uiv(currentUniform->mLocation, glArraySize,
                                                          (GLuint*)params->getUnsignedIntPointer(def->physicalIndex)));
                        break;
                    default:
                        break;

                    } // End switch

                } // Variability & mask
            } // fromProgType == currentUniform->mSourceProgType

        } // End for
    }

} // namespace Ogre
