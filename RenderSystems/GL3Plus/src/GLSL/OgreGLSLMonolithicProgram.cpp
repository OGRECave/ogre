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
#include "OgreGLSLMonolithicProgramManager.h"
#include "OgreGL3PlusRenderSystem.h"
#include "OgreGL3PlusVertexArrayObject.h"
#include "OgreStringVector.h"
#include "OgreLogManager.h"
#include "OgreGpuProgramManager.h"
#include "OgreStringConverter.h"
#include "OgreProfiler.h"

namespace Ogre {

    GLint getGLGeometryInputPrimitiveType(OperationType operationType, bool requiresAdjacency)
    {
        switch (operationType)
        {
        case OT_POINT_LIST:
            return GL_POINTS;
        case OT_LINE_LIST:
            return requiresAdjacency ? GL_LINES_ADJACENCY : GL_LINES;
        case OT_LINE_STRIP:
            return requiresAdjacency ? GL_LINE_STRIP_ADJACENCY : GL_LINES;
        default:
        case OT_TRIANGLE_LIST:
            return requiresAdjacency ? GL_TRIANGLES_ADJACENCY : GL_TRIANGLES;
        case OT_TRIANGLE_STRIP:
            return requiresAdjacency ? GL_TRIANGLE_STRIP_ADJACENCY : GL_TRIANGLES;
        case OT_TRIANGLE_FAN:
            return requiresAdjacency ? GL_TRIANGLES_ADJACENCY : GL_TRIANGLES;
        }
    }


    GLint getGLGeometryOutputPrimitiveType(OperationType operationType)
    {
        switch (operationType)
        {
        case OT_POINT_LIST:
            return GL_POINTS;
        case OT_LINE_STRIP:
            return GL_LINE_STRIP;
        case OT_TRIANGLE_STRIP:
            return GL_TRIANGLE_STRIP;
        default:
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        "Geometry shader output operation type can only be point list,"
                        "line strip or triangle strip",
                        "GLSLMonolithicProgram::getGLGeometryOutputPrimitiveType");
        }
    }


    GLSLMonolithicProgram::GLSLMonolithicProgram(GLSLShader* vertexProgram,
                                                 GLSLShader* hullProgram,
                                                 GLSLShader* domainProgram,
                                                 GLSLShader* geometryProgram,
                                                 GLSLShader* fragmentProgram,
                                                 GLSLShader* computeProgram)
        : GLSLProgram(vertexProgram,
                      hullProgram,
                      domainProgram,
                      geometryProgram,
                      fragmentProgram,
                      computeProgram)
    {
    }


    GLSLMonolithicProgram::~GLSLMonolithicProgram(void)
    {
        OGRE_CHECK_GL_ERROR(glDeleteProgram(mGLProgramHandle));
        mGLProgramHandle = 0;
    }


    void GLSLMonolithicProgram::_useProgram(void)
    {
        if (mLinked)
        {
            OGRE_CHECK_GL_ERROR(glUseProgram(mGLProgramHandle));
        }
    }


    void GLSLMonolithicProgram::activate(void)
    {
        OgreProfileExhaustiveAggr( "GLSLMonolithicProgram::activate" );

        if (!mLinked && !mTriedToLinkAndFailed)
        {
            OGRE_CHECK_GL_ERROR(mGLProgramHandle = glCreateProgram());

            if ( GpuProgramManager::getSingleton().canGetCompiledShaderBuffer() &&
                 GpuProgramManager::getSingleton().isMicrocodeAvailableInCache(getCombinedSource()) )
            {
                getMicrocodeFromCache();
            }
            else
            {
                compileAndLink();
            }

            extractLayoutQualifiers();
            buildGLUniformReferences();
        }

        _useProgram();
    }


    void GLSLMonolithicProgram::compileAndLink()
    {
        OgreProfileExhaustive( "GLSLMonolithicProgram::compileAndLink" );

        mVertexArrayObject = new GL3PlusOldVertexArrayObject();
        mVertexArrayObject->bind();

        // Compile and attach Vertex Program
        if (mVertexShader)
        {
            if (!mVertexShader->compile(true))
            {
                mTriedToLinkAndFailed = true;
                return;
            }
            mVertexShader->attachToProgramObject(mGLProgramHandle);
            setSkeletalAnimationIncluded(mVertexShader->isSkeletalAnimationIncluded());
        }

        // Compile and attach Fragment Program
        if (mFragmentShader)
        {
            if (!mFragmentShader->compile(true))
            {
                mTriedToLinkAndFailed = true;
                return;
            }
            mFragmentShader->attachToProgramObject(mGLProgramHandle);
        }

        // Compile and attach Geometry Program
        if (mGeometryShader)
        {
            if (!mGeometryShader->compile(true))
            {
                return;
            }

            mGeometryShader->attachToProgramObject(mGLProgramHandle);
        }

        // Compile and attach Tessellation Control Program
        if (mHullShader)
        {
            if (!mHullShader->compile(true))
            {
                return;
            }

            mHullShader->attachToProgramObject(mGLProgramHandle);
        }

        // Compile and attach Tessellation Evaluation Program
        if (mDomainShader)
        {
            if (!mDomainShader->compile(true))
            {
                return;
            }

            mDomainShader->attachToProgramObject(mGLProgramHandle);
        }

        // Compile and attach Compute Program
        if (mComputeShader)
        {
            if (!mComputeShader->compile(true))
            {
                return;
            }

            mComputeShader->attachToProgramObject(mGLProgramHandle);
        }

        bindFixedAttributes( mGLProgramHandle );

        // the link
        OGRE_CHECK_GL_ERROR(glLinkProgram( mGLProgramHandle ));
        OGRE_CHECK_GL_ERROR(glGetProgramiv( mGLProgramHandle, GL_LINK_STATUS, &mLinked ));

        mTriedToLinkAndFailed = !mLinked;

        logObjectInfo( getCombinedName() + String(" GLSL link result : "), mGLProgramHandle );

        if(glIsProgram(mGLProgramHandle))
        {
            OGRE_CHECK_GL_ERROR(glValidateProgram(mGLProgramHandle));
        }
        logObjectInfo( getCombinedName() + String(" GLSL validation result : "), mGLProgramHandle );

        if(mLinked)
        {
            setupBaseInstance( mGLProgramHandle );
            if ( GpuProgramManager::getSingleton().getSaveMicrocodesToCache() )
            {
                // add to the microcode to the cache
                String source;
                source = getCombinedSource();

                // get buffer size
                GLint binaryLength = 0;
                OGRE_CHECK_GL_ERROR(glGetProgramiv(mGLProgramHandle, GL_PROGRAM_BINARY_LENGTH, &binaryLength));

                // create microcode
                GpuProgramManager::Microcode newMicrocode =
                    GpuProgramManager::getSingleton().createMicrocode(binaryLength + sizeof(GLenum));

                // get binary
                OGRE_CHECK_GL_ERROR(glGetProgramBinary(mGLProgramHandle, binaryLength, NULL, (GLenum *)newMicrocode->getPtr(), newMicrocode->getPtr() + sizeof(GLenum)));

                // add to the microcode to the cache
                GpuProgramManager::getSingleton().addMicrocodeToCache(source, newMicrocode);
            }
        }
    }


    void GLSLMonolithicProgram::buildGLUniformReferences(void)
    {
        OgreProfileExhaustive( "GLSLMonolithicProgram::buildGLUniformReferences" );

        if (!mUniformRefsBuilt)
        {
            const GpuConstantDefinitionMap* vertParams = 0;
            const GpuConstantDefinitionMap* hullParams = 0;
            const GpuConstantDefinitionMap* domainParams = 0;
            const GpuConstantDefinitionMap* fragParams = 0;
            const GpuConstantDefinitionMap* geomParams = 0;
            const GpuConstantDefinitionMap* computeParams = 0;
            if (mVertexShader)
            {
                vertParams = &(mVertexShader->getConstantDefinitions().map);
            }
            if (mHullShader)
            {
                hullParams = &(mHullShader->getConstantDefinitions().map);
            }
            if (mDomainShader)
            {
                domainParams = &(mDomainShader->getConstantDefinitions().map);
            }
            if (mGeometryShader)
            {
                geomParams = &(mGeometryShader->getConstantDefinitions().map);
            }
            if (mFragmentShader)
            {
                fragParams = &(mFragmentShader->getConstantDefinitions().map);
            }
            if (mComputeShader)
            {
                computeParams = &(mComputeShader->getConstantDefinitions().map);
            }

            // Do we know how many shared params there are yet? Or if there are any blocks defined?
            GLSLMonolithicProgramManager::getSingleton().extractUniformsFromProgram(
                mGLProgramHandle, vertParams, geomParams, fragParams, hullParams, domainParams, computeParams,
                mGLUniformReferences, mGLAtomicCounterReferences, mGLUniformBufferReferences, mSharedParamsBufferMap, mGLCounterBufferReferences);

            mUniformRefsBuilt = true;
        }
    }


    void GLSLMonolithicProgram::updateUniforms(GpuProgramParametersSharedPtr params,
                                               uint16 mask, GpuProgramType fromProgType)
    {
        // Iterate through uniform reference list and update uniform values
        GLUniformReferenceIterator currentUniform = mGLUniformReferences.begin();
        GLUniformReferenceIterator endUniform = mGLUniformReferences.end();

        // determine if we need to transpose matrices when binding
        int transpose = GL_TRUE;
        if ((fromProgType == GPT_FRAGMENT_PROGRAM && mVertexShader && (!mVertexShader->getColumnMajorMatrices())) ||
            (fromProgType == GPT_VERTEX_PROGRAM && mFragmentShader && (!mFragmentShader->getColumnMajorMatrices())) ||
            (fromProgType == GPT_GEOMETRY_PROGRAM && mGeometryShader && (!mGeometryShader->getColumnMajorMatrices())) ||
            (fromProgType == GPT_HULL_PROGRAM && mHullShader && (!mHullShader->getColumnMajorMatrices())) ||
            (fromProgType == GPT_DOMAIN_PROGRAM && mDomainShader && (!mDomainShader->getColumnMajorMatrices())) ||
            (fromProgType == GPT_COMPUTE_PROGRAM && mComputeShader && (!mComputeShader->getColumnMajorMatrices())))
        {
            transpose = GL_FALSE;
        }

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
                                                                 transpose, params->getDoublePointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_DOUBLE_2X4:
                        OGRE_CHECK_GL_ERROR(glUniformMatrix2x4dv(currentUniform->mLocation, glArraySize,
                                                                 transpose, params->getDoublePointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_DOUBLE_3X2:
                        OGRE_CHECK_GL_ERROR(glUniformMatrix3x2dv(currentUniform->mLocation, glArraySize,
                                                                 transpose, params->getDoublePointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_DOUBLE_3X3:
                        OGRE_CHECK_GL_ERROR(glUniformMatrix3dv(currentUniform->mLocation, glArraySize,
                                                               transpose, params->getDoublePointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_DOUBLE_3X4:
                        OGRE_CHECK_GL_ERROR(glUniformMatrix3x4dv(currentUniform->mLocation, glArraySize,
                                                                 transpose, params->getDoublePointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_DOUBLE_4X2:
                        OGRE_CHECK_GL_ERROR(glUniformMatrix4x2dv(currentUniform->mLocation, glArraySize,
                                                                 transpose, params->getDoublePointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_DOUBLE_4X3:
                        OGRE_CHECK_GL_ERROR(glUniformMatrix4x3dv(currentUniform->mLocation, glArraySize,
                                                                 transpose, params->getDoublePointer(def->physicalIndex)));
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
                                                                 transpose, params->getFloatPointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_2X4:
                        OGRE_CHECK_GL_ERROR(glUniformMatrix2x4fv(currentUniform->mLocation, glArraySize,
                                                                 transpose, params->getFloatPointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_3X2:
                        OGRE_CHECK_GL_ERROR(glUniformMatrix3x2fv(currentUniform->mLocation, glArraySize,
                                                                 transpose, params->getFloatPointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_3X3:
                        OGRE_CHECK_GL_ERROR(glUniformMatrix3fv(currentUniform->mLocation, glArraySize,
                                                               transpose, params->getFloatPointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_3X4:
                        OGRE_CHECK_GL_ERROR(glUniformMatrix3x4fv(currentUniform->mLocation, glArraySize,
                                                                 transpose, params->getFloatPointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_4X2:
                        OGRE_CHECK_GL_ERROR(glUniformMatrix4x2fv(currentUniform->mLocation, glArraySize,
                                                                 transpose, params->getFloatPointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_4X3:
                        OGRE_CHECK_GL_ERROR(glUniformMatrix4x3fv(currentUniform->mLocation, glArraySize,
                                                                 transpose, params->getFloatPointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_4X4:
                        OGRE_CHECK_GL_ERROR(glUniformMatrix4fv(currentUniform->mLocation, glArraySize,
                                                               transpose, params->getFloatPointer(def->physicalIndex)));
                        break;
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

                    case GCT_SAMPLER1D:
                    case GCT_SAMPLER1DSHADOW:
                    case GCT_SAMPLER2D:
                    case GCT_SAMPLER2DSHADOW:
                    case GCT_SAMPLER2DARRAY:
                    case GCT_SAMPLER3D:
                    case GCT_SAMPLERCUBE:
                    case GCT_SAMPLERRECT:
                        // Samplers handled like 1-element ints
                        OGRE_CHECK_GL_ERROR(glUniform1iv(currentUniform->mLocation, glArraySize,
                                                         (GLint*)params->getIntPointer(def->physicalIndex)));
                        break;
                    default:
                        break;

                    } // End switch

                } // Variability & mask
            } // fromProgType == currentUniform->mSourceProgType

        } // End for
    }

    void GLSLMonolithicProgram::updateUniformBlocks(GpuProgramParametersSharedPtr params,
                                                    uint16 mask, GpuProgramType fromProgType)
    {
        // Iterate through the list of uniform buffers and update them as needed
        GLUniformBufferIterator currentBuffer = mGLUniformBufferReferences.begin();
        GLUniformBufferIterator endBuffer = mGLUniformBufferReferences.end();

        const GpuProgramParameters::GpuSharedParamUsageList& sharedParams = params->getSharedParameters();

        GpuProgramParameters::GpuSharedParamUsageList::const_iterator it, end = sharedParams.end();
        for (it = sharedParams.begin(); it != end; ++it)
        {
            for (;currentBuffer != endBuffer; ++currentBuffer)
            {
                v1::GL3PlusHardwareUniformBuffer* hwGlBuffer = static_cast<v1::GL3PlusHardwareUniformBuffer*>(currentBuffer->get());
                GpuSharedParametersPtr paramsPtr = it->getSharedParams();

                // Block name is stored in mSharedParams->mName of GpuSharedParamUsageList items
                GLint UniformTransform;
                OGRE_CHECK_GL_ERROR(UniformTransform = glGetUniformBlockIndex(mGLProgramHandle, it->getName().c_str()));
                OGRE_CHECK_GL_ERROR(glUniformBlockBinding(mGLProgramHandle, UniformTransform, hwGlBuffer->getGLBufferBinding()));

                hwGlBuffer->writeData(0, hwGlBuffer->getSizeInBytes(), &paramsPtr->getFloatConstantList().front());
            }
        }
    }


    void GLSLMonolithicProgram::updatePassIterationUniforms(GpuProgramParametersSharedPtr params)
    {
        if (params->hasPassIterationNumber())
        {
            size_t index = params->getPassIterationNumberIndex();

            GLUniformReferenceIterator currentUniform = mGLUniformReferences.begin();
            GLUniformReferenceIterator endUniform = mGLUniformReferences.end();

            // Need to find the uniform that matches the multi pass entry
            for (;currentUniform != endUniform; ++currentUniform)
            {
                // Get the index in the parameter real list
                if (index == currentUniform->mConstantDef->physicalIndex)
                {
                    OGRE_CHECK_GL_ERROR(glUniform1fv(currentUniform->mLocation, 1, params->getFloatPointer(index)));
                    // There will only be one multipass entry
                    return;
                }
            }
        }
    }

} // namespace Ogre
