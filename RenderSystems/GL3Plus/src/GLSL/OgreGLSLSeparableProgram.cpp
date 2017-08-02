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

#include "OgreGLSLSeparableProgram.h"
#include "OgreStringConverter.h"
#include "OgreGLSLShader.h"
#include "OgreGLSLSeparableProgramManager.h"
#include "OgreGpuProgramManager.h"
#include "OgreGL3PlusUtil.h"
#include "OgreLogManager.h"

namespace Ogre
{
    GLSLSeparableProgram::GLSLSeparableProgram(GLSLShader* vertexShader,
                                               GLSLShader* hullShader,
                                               GLSLShader* domainShader,
                                               GLSLShader* geometryShader,
                                               GLSLShader* fragmentShader,
                                               GLSLShader* computeShader) :
        GLSLProgram(vertexShader,
                    hullShader,
                    domainShader,
                    geometryShader,
                    fragmentShader,
                    computeShader)
    {
        mVertexArrayObject = new GL3PlusOldVertexArrayObject();
    }

    GLSLSeparableProgram::~GLSLSeparableProgram()
    {
        OGRE_CHECK_GL_ERROR(glDeleteProgramPipelines(1, &mGLProgramPipelineHandle));
    }

    void GLSLSeparableProgram::compileAndLink()
    {
        // Ensure no monolithic programs are in use.
        OGRE_CHECK_GL_ERROR(glUseProgram(0));

        OGRE_CHECK_GL_ERROR(glGenProgramPipelines(1, &mGLProgramPipelineHandle));
        //OGRE_CHECK_GL_ERROR(glBindProgramPipeline(mGLProgramPipelineHandle));

        mVertexArrayObject->bind();

        loadIndividualProgram(mVertexShader);
        loadIndividualProgram(mDomainShader);
        loadIndividualProgram(mHullShader);
        loadIndividualProgram(mGeometryShader);
        loadIndividualProgram(mFragmentShader);
        loadIndividualProgram(mComputeShader);

        if (mLinked)
        {
            // if (GpuProgramManager::getSingleton().getSaveMicrocodesToCache() )
            // {
            //     // Add to the microcode to the cache
            //     String name;
            //     name = getCombinedName();

            //     // Get buffer size
            //     GLint binaryLength = 0;

            //     //OGRE_CHECK_GL_ERROR(glGetProgramiv(mGLProgramPipelineHandle, GL_PROGRAM_BINARY_LENGTH, &binaryLength));

            //     OGRE_CHECK_GL_ERROR(glGetProgramiv(, GL_PROGRAM_BINARY_LENGTH, &binaryLength));

            //     // Create microcode
            //     GpuProgramManager::Microcode newMicrocode =
            //         GpuProgramManager::getSingleton().createMicrocode((unsigned long)binaryLength + sizeof(GLenum));

            //     // Get binary
            //     OGRE_CHECK_GL_ERROR(glGetProgramBinary(, binaryLength, NULL, (GLenum *)newMicrocode->getPtr(), newMicrocode->getPtr() + sizeof(GLenum)));

            //     // Add to the microcode to the cache
            //     GpuProgramManager::getSingleton().addMicrocodeToCache(name, newMicrocode);
            // }
            if (mVertexShader && mVertexShader->isLinked())
            {
                OGRE_CHECK_GL_ERROR(glUseProgramStages(mGLProgramPipelineHandle, GL_VERTEX_SHADER_BIT, mVertexShader->getGLProgramHandle()));
            }
            if (mDomainShader && mDomainShader->isLinked())
            {
                OGRE_CHECK_GL_ERROR(glUseProgramStages(mGLProgramPipelineHandle, GL_TESS_EVALUATION_SHADER_BIT, mDomainShader->getGLProgramHandle()));
            }
            if (mHullShader && mHullShader->isLinked())
            {
                OGRE_CHECK_GL_ERROR(glUseProgramStages(mGLProgramPipelineHandle, GL_TESS_CONTROL_SHADER_BIT, mHullShader->getGLProgramHandle()));
            }
            if (mGeometryShader && mGeometryShader->isLinked())
            {
                OGRE_CHECK_GL_ERROR(glUseProgramStages(mGLProgramPipelineHandle, GL_GEOMETRY_SHADER_BIT, mGeometryShader->getGLProgramHandle()));
            }
            if (mFragmentShader && mFragmentShader->isLinked())
            {
                OGRE_CHECK_GL_ERROR(glUseProgramStages(mGLProgramPipelineHandle, GL_FRAGMENT_SHADER_BIT, mFragmentShader->getGLProgramHandle()));
            }
            if (mComputeShader && mComputeShader->isLinked())
            {
                OGRE_CHECK_GL_ERROR(glUseProgramStages(mGLProgramPipelineHandle, GL_COMPUTE_SHADER_BIT, mComputeShader->getGLProgramHandle()));
            }

            // Validate pipeline
            OGRE_CHECK_GL_ERROR(glValidateProgramPipeline(mGLProgramPipelineHandle));
            logObjectInfo( getCombinedName() + String("GLSL program pipeline validation result: "), mGLProgramPipelineHandle );

            if( mVertexShader && mVertexShader->isLinked() )
                setupBaseInstance( mGLProgramPipelineHandle );

            //            if (getGLSupport()->checkExtension("GL_KHR_debug") || mHasGL43)
            //                glObjectLabel(GL_PROGRAM_PIPELINE, mGLProgramPipelineHandle, 0,
            //                                 (mVertexShader->getName() + "/" + mFragmentShader->getName()).c_str());
        }
    }

    void GLSLSeparableProgram::loadIndividualProgram(GLSLShader *program)
    {
        if (program && !program->isLinked())
        {
            GLint linkStatus = 0;

            String programSource = program->getSource();

            GLuint programHandle = program->getGLProgramHandle();

            OGRE_CHECK_GL_ERROR(glProgramParameteri(programHandle, GL_PROGRAM_SEPARABLE, GL_TRUE));
            //if (GpuProgramManager::getSingleton().getSaveMicrocodesToCache())
            OGRE_CHECK_GL_ERROR(glProgramParameteri(programHandle, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE));

            // Use precompiled program if possible.
            bool microcodeAvailableInCache = GpuProgramManager::getSingleton().isMicrocodeAvailableInCache(programSource);
            if (microcodeAvailableInCache)
            {
                GpuProgramManager::Microcode cacheMicrocode =
                    GpuProgramManager::getSingleton().getMicrocodeFromCache(programSource);
                cacheMicrocode->seek(0);

                GLenum binaryFormat = 0;
                cacheMicrocode->read(&binaryFormat, sizeof(GLenum));

                GLint binaryLength = cacheMicrocode->size() - sizeof(GLenum);

                OGRE_CHECK_GL_ERROR(glProgramBinary(programHandle,
                                                    binaryFormat,
                                                    cacheMicrocode->getPtr() + sizeof(GLenum),
                                                    binaryLength));

                OGRE_CHECK_GL_ERROR(glGetProgramiv(programHandle, GL_LINK_STATUS, &linkStatus));
                if (!linkStatus)
                    logObjectInfo("Could not use cached binary " + program->getName(), programHandle);
            }

            // Compilation needed if precompiled program is
            // unavailable or failed to link.
            if (!linkStatus)
            {
                try
                {
                    program->compile(true);
                }
                catch (Exception& e)
                {
                    LogManager::getSingleton().stream() << e.getDescription();
                    mTriedToLinkAndFailed = true;
                    return;
                }

                if( program->getType() == GPT_VERTEX_PROGRAM )
                    bindFixedAttributes( programHandle );

                program->attachToProgramObject(programHandle);
                OGRE_CHECK_GL_ERROR(glLinkProgram(programHandle));
                OGRE_CHECK_GL_ERROR(glGetProgramiv(programHandle, GL_LINK_STATUS, &linkStatus));

                // Binary cache needs an update.
                microcodeAvailableInCache = false;
            }

            program->setLinked(linkStatus);
            mLinked = linkStatus;

            mTriedToLinkAndFailed = !linkStatus;

            logObjectInfo( getCombinedName() + String("GLSL program result : "), programHandle );

            if (program->getType() == GPT_VERTEX_PROGRAM)
                setSkeletalAnimationIncluded(program->isSkeletalAnimationIncluded());

            // Add the microcode to the cache.
            if (!microcodeAvailableInCache && mLinked &&
                GpuProgramManager::getSingleton().getSaveMicrocodesToCache() )
            {
                // Get buffer size.
                GLint binaryLength = 0;

                OGRE_CHECK_GL_ERROR(glGetProgramiv(programHandle, GL_PROGRAM_BINARY_LENGTH, &binaryLength));

                // Create microcode.
                GpuProgramManager::Microcode newMicrocode =
                    GpuProgramManager::getSingleton().createMicrocode((unsigned long)binaryLength + sizeof(GLenum));

                // Get binary.
                OGRE_CHECK_GL_ERROR(glGetProgramBinary(programHandle, binaryLength, NULL, (GLenum *)newMicrocode->getPtr(), newMicrocode->getPtr() + sizeof(GLenum)));

                // std::vector<uchar> buffer(binaryLength);
                // GLenum format(0);
                // OGRE_CHECK_GL_ERROR(glGetProgramBinary(programHandle, binaryLength, NULL, &format, &buffer[0]));

                // GLenum binaryFormat = 0;
                // std::vector<uchar> binaryData(binaryLength);
                // newMicrocode->read(&binaryFormat, sizeof(GLenum));
                // newMicrocode->read(&binaryData[0], binaryLength);

                GpuProgramManager::getSingleton().addMicrocodeToCache(programSource, newMicrocode);
            }
        }
    }

    // void GLSLSeparableProgram::_useProgram(void)
    // {
    //     if (mLinked)
    //     { 
    //        OGRE_CHECK_GL_ERROR(glBindProgramPipeline(mGLProgramPipelineHandle));
    //     }
    // }


    GLint GLSLSeparableProgram::getAttributeIndex(VertexElementSemantic semantic, uint index)
    {
        GLint res = mCustomAttributesIndexes[semantic-1][index];
        if (res == NULL_CUSTOM_ATTRIBUTES_INDEX)
        {
            GLuint handle = mVertexShader->getGLProgramHandle();
            const char * attString = getAttributeSemanticString(semantic);
            GLint attrib;
            OGRE_CHECK_GL_ERROR(attrib = glGetAttribLocation(handle, attString));

            // Sadly position is a special case.
            if (attrib == NOT_FOUND_CUSTOM_ATTRIBUTES_INDEX && semantic == VES_POSITION)
            {
                OGRE_CHECK_GL_ERROR(attrib = glGetAttribLocation(handle, "position"));
            }

            // For UV and other cases the index is a part of the name.
            if (attrib == NOT_FOUND_CUSTOM_ATTRIBUTES_INDEX)
            {
                String attStringWithSemantic = String(attString) + StringConverter::toString(index);
                OGRE_CHECK_GL_ERROR(attrib = glGetAttribLocation(handle, attStringWithSemantic.c_str()));
            }

            // Update mCustomAttributesIndexes with the index we found (or didn't find)
            mCustomAttributesIndexes[semantic - 1][index] = attrib;
            res = attrib;
        }

        return res;
    }


    void GLSLSeparableProgram::activate(void)
    {
        if (!mLinked && !mTriedToLinkAndFailed)
        {
            compileAndLink();

            extractLayoutQualifiers();

            buildGLUniformReferences();
        }

        // _useProgram();


        if (mLinked)
        { 
            OGRE_CHECK_GL_ERROR(glBindProgramPipeline(mGLProgramPipelineHandle));
        }
    }


    void GLSLSeparableProgram::buildGLUniformReferences(void)
    {
        if (!mUniformRefsBuilt)
        {
            const GpuConstantDefinitionMap* vertParams = 0;
            const GpuConstantDefinitionMap* hullParams = 0;
            const GpuConstantDefinitionMap* domainParams = 0;
            const GpuConstantDefinitionMap* geomParams = 0;
            const GpuConstantDefinitionMap* fragParams = 0;
            const GpuConstantDefinitionMap* computeParams = 0;
            if (mVertexShader)
            {
                vertParams = &(mVertexShader->getConstantDefinitions().map);
                GLSLSeparableProgramManager::getSingleton().extractUniformsFromProgram(mVertexShader->getGLProgramHandle(),
                                                                                       vertParams, NULL, NULL, NULL, NULL, NULL,
                                                                                       mGLUniformReferences, mGLAtomicCounterReferences, mGLUniformBufferReferences, mSharedParamsBufferMap, mGLCounterBufferReferences);
            }
            if (mHullShader)
            {
                hullParams = &(mHullShader->getConstantDefinitions().map);
                GLSLSeparableProgramManager::getSingleton().extractUniformsFromProgram(mHullShader->getGLProgramHandle(),
                                                                                       NULL, NULL, NULL, hullParams, NULL, NULL,
                                                                                       mGLUniformReferences, mGLAtomicCounterReferences, mGLUniformBufferReferences, mSharedParamsBufferMap, mGLCounterBufferReferences);
            }
            if (mDomainShader)
            {
                domainParams = &(mDomainShader->getConstantDefinitions().map);
                GLSLSeparableProgramManager::getSingleton().extractUniformsFromProgram(mDomainShader->getGLProgramHandle(),
                                                                                       NULL, NULL, NULL, NULL, domainParams, NULL,
                                                                                       mGLUniformReferences, mGLAtomicCounterReferences, mGLUniformBufferReferences, mSharedParamsBufferMap, mGLCounterBufferReferences);
            }
            if (mGeometryShader)
            {
                geomParams = &(mGeometryShader->getConstantDefinitions().map);
                GLSLSeparableProgramManager::getSingleton().extractUniformsFromProgram(mGeometryShader->getGLProgramHandle(),
                                                                                       NULL, geomParams, NULL, NULL, NULL, NULL,
                                                                                       mGLUniformReferences, mGLAtomicCounterReferences, mGLUniformBufferReferences, mSharedParamsBufferMap, mGLCounterBufferReferences);
            }
            if (mFragmentShader)
            {
                fragParams = &(mFragmentShader->getConstantDefinitions().map);
                GLSLSeparableProgramManager::getSingleton().extractUniformsFromProgram(mFragmentShader->getGLProgramHandle(),
                                                                                       NULL, NULL, fragParams, NULL, NULL, NULL,
                                                                                       mGLUniformReferences, mGLAtomicCounterReferences, mGLUniformBufferReferences, mSharedParamsBufferMap, mGLCounterBufferReferences);
            }
            if (mComputeShader)
            {
                computeParams = &(mComputeShader->getConstantDefinitions().map);
                GLSLSeparableProgramManager::getSingleton().extractUniformsFromProgram(mComputeShader->getGLProgramHandle(),
                                                                                       NULL, NULL, NULL, NULL, NULL, computeParams,
                                                                                       mGLUniformReferences, mGLAtomicCounterReferences, mGLUniformBufferReferences, mSharedParamsBufferMap, mGLCounterBufferReferences);
            }

            mUniformRefsBuilt = true;
        }
    }


    void GLSLSeparableProgram::updateUniforms(GpuProgramParametersSharedPtr params,
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

        GLuint progID = 0;
        if (fromProgType == GPT_VERTEX_PROGRAM)
        {
            progID = mVertexShader->getGLProgramHandle();
        }
        else if (fromProgType == GPT_FRAGMENT_PROGRAM)
        {
            progID = mFragmentShader->getGLProgramHandle();
        }
        else if (fromProgType == GPT_GEOMETRY_PROGRAM)
        {
            progID = mGeometryShader->getGLProgramHandle();
        }
        else if (fromProgType == GPT_HULL_PROGRAM)
        {
            progID = mHullShader->getGLProgramHandle();
        }
        else if (fromProgType == GPT_DOMAIN_PROGRAM)
        {
            progID = mDomainShader->getGLProgramHandle();
        }
        else if (fromProgType == GPT_COMPUTE_PROGRAM)
        {
            progID = mComputeShader->getGLProgramHandle();
        }

        for (; currentUniform != endUniform; ++currentUniform)
        {
            // Only pull values from buffer it's supposed to be in (vertex or fragment)
            // This method will be called once per shader stage.
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
                        OGRE_CHECK_GL_ERROR(glProgramUniform1fv(progID, currentUniform->mLocation, glArraySize,
                                                                params->getFloatPointer(def->physicalIndex)));
                        break;
                    case GCT_FLOAT2:
                        OGRE_CHECK_GL_ERROR(glProgramUniform2fv(progID, currentUniform->mLocation, glArraySize,
                                                                params->getFloatPointer(def->physicalIndex)));
                        break;
                    case GCT_FLOAT3:
                        OGRE_CHECK_GL_ERROR(glProgramUniform3fv(progID, currentUniform->mLocation, glArraySize,
                                                                params->getFloatPointer(def->physicalIndex)));
                        break;
                    case GCT_FLOAT4:
                        OGRE_CHECK_GL_ERROR(glProgramUniform4fv(progID, currentUniform->mLocation, glArraySize,
                                                                params->getFloatPointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_2X2:
                        OGRE_CHECK_GL_ERROR(glProgramUniformMatrix2fv(progID, currentUniform->mLocation, glArraySize,
                                                                      transpose, params->getFloatPointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_3X3:
                        OGRE_CHECK_GL_ERROR(glProgramUniformMatrix3fv(progID, currentUniform->mLocation, glArraySize,
                                                                      transpose, params->getFloatPointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_4X4:
                        OGRE_CHECK_GL_ERROR(glProgramUniformMatrix4fv(progID, currentUniform->mLocation, glArraySize,
                                                                      transpose, params->getFloatPointer(def->physicalIndex)));
                        break;
                    case GCT_INT1:
                        OGRE_CHECK_GL_ERROR(glProgramUniform1iv(progID, currentUniform->mLocation, glArraySize,
                                                                params->getIntPointer(def->physicalIndex)));
                        break;
                    case GCT_INT2:
                        OGRE_CHECK_GL_ERROR(glProgramUniform2iv(progID, currentUniform->mLocation, glArraySize,
                                                                params->getIntPointer(def->physicalIndex)));
                        break;
                    case GCT_INT3:
                        OGRE_CHECK_GL_ERROR(glProgramUniform3iv(progID, currentUniform->mLocation, glArraySize,
                                                                params->getIntPointer(def->physicalIndex)));
                        break;
                    case GCT_INT4:
                        OGRE_CHECK_GL_ERROR(glProgramUniform4iv(progID, currentUniform->mLocation, glArraySize,
                                                                params->getIntPointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_2X3:
                        OGRE_CHECK_GL_ERROR(glProgramUniformMatrix2x3fv(progID, currentUniform->mLocation, glArraySize,
                                                                        transpose, params->getFloatPointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_2X4:
                        OGRE_CHECK_GL_ERROR(glProgramUniformMatrix2x4fv(progID, currentUniform->mLocation, glArraySize,
                                                                        transpose, params->getFloatPointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_3X2:
                        OGRE_CHECK_GL_ERROR(glProgramUniformMatrix3x2fv(progID, currentUniform->mLocation, glArraySize,
                                                                        transpose, params->getFloatPointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_3X4:
                        OGRE_CHECK_GL_ERROR(glProgramUniformMatrix3x4fv(progID, currentUniform->mLocation, glArraySize,
                                                                        transpose, params->getFloatPointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_4X2:
                        OGRE_CHECK_GL_ERROR(glProgramUniformMatrix4x2fv(progID, currentUniform->mLocation, glArraySize,
                                                                        transpose, params->getFloatPointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_4X3:
                        OGRE_CHECK_GL_ERROR(glProgramUniformMatrix4x3fv(progID, currentUniform->mLocation, glArraySize,
                                                                        transpose, params->getFloatPointer(def->physicalIndex)));
                        break;
                    case GCT_DOUBLE1:
                        OGRE_CHECK_GL_ERROR(glProgramUniform1dv(progID, currentUniform->mLocation, glArraySize,
                                                                params->getDoublePointer(def->physicalIndex)));
                        break;
                    case GCT_DOUBLE2:
                        OGRE_CHECK_GL_ERROR(glProgramUniform2dv(progID, currentUniform->mLocation, glArraySize,
                                                                params->getDoublePointer(def->physicalIndex)));
                        break;
                    case GCT_DOUBLE3:
                        OGRE_CHECK_GL_ERROR(glProgramUniform3dv(progID, currentUniform->mLocation, glArraySize,
                                                                params->getDoublePointer(def->physicalIndex)));
                        break;
                    case GCT_DOUBLE4:
                        OGRE_CHECK_GL_ERROR(glProgramUniform4dv(progID, currentUniform->mLocation, glArraySize,
                                                                params->getDoublePointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_DOUBLE_2X2:
                        OGRE_CHECK_GL_ERROR(glProgramUniformMatrix2dv(progID, currentUniform->mLocation, glArraySize,
                                                                      transpose, params->getDoublePointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_DOUBLE_3X3:
                        OGRE_CHECK_GL_ERROR(glProgramUniformMatrix3dv(progID, currentUniform->mLocation, glArraySize,
                                                                      transpose, params->getDoublePointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_DOUBLE_4X4:
                        OGRE_CHECK_GL_ERROR(glProgramUniformMatrix4dv(progID, currentUniform->mLocation, glArraySize,
                                                                      transpose, params->getDoublePointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_DOUBLE_2X3:
                        OGRE_CHECK_GL_ERROR(glProgramUniformMatrix2x3dv(progID, currentUniform->mLocation, glArraySize,
                                                                        transpose, params->getDoublePointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_DOUBLE_2X4:
                        OGRE_CHECK_GL_ERROR(glProgramUniformMatrix2x4dv(progID, currentUniform->mLocation, glArraySize,
                                                                        transpose, params->getDoublePointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_DOUBLE_3X2:
                        OGRE_CHECK_GL_ERROR(glProgramUniformMatrix3x2dv(progID, currentUniform->mLocation, glArraySize,
                                                                        transpose, params->getDoublePointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_DOUBLE_3X4:
                        OGRE_CHECK_GL_ERROR(glProgramUniformMatrix3x4dv(progID, currentUniform->mLocation, glArraySize,
                                                                        transpose, params->getDoublePointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_DOUBLE_4X2:
                        OGRE_CHECK_GL_ERROR(glProgramUniformMatrix4x2dv(progID, currentUniform->mLocation, glArraySize,
                                                                        transpose, params->getDoublePointer(def->physicalIndex)));
                        break;
                    case GCT_MATRIX_DOUBLE_4X3:
                        OGRE_CHECK_GL_ERROR(glProgramUniformMatrix4x3dv(progID, currentUniform->mLocation, glArraySize,
                                                                        transpose, params->getDoublePointer(def->physicalIndex)));
                        break;
                    case GCT_UINT1:
                    case GCT_BOOL1:
                        OGRE_CHECK_GL_ERROR(glProgramUniform1uiv(progID, currentUniform->mLocation, glArraySize,
                                                                 params->getUnsignedIntPointer(def->physicalIndex)));
                        break;
                    case GCT_UINT2:
                    case GCT_BOOL2:
                        OGRE_CHECK_GL_ERROR(glProgramUniform2uiv(progID, currentUniform->mLocation, glArraySize,
                                                                 params->getUnsignedIntPointer(def->physicalIndex)));
                        break;
                    case GCT_UINT3:
                    case GCT_BOOL3:
                        OGRE_CHECK_GL_ERROR(glProgramUniform3uiv(progID, currentUniform->mLocation, glArraySize,
                                                                 params->getUnsignedIntPointer(def->physicalIndex)));
                        break;
                    case GCT_UINT4:
                    case GCT_BOOL4:
                        OGRE_CHECK_GL_ERROR(glProgramUniform4uiv(progID, currentUniform->mLocation, glArraySize,
                                                                 params->getUnsignedIntPointer(def->physicalIndex)));
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
                        OGRE_CHECK_GL_ERROR(glProgramUniform1iv(progID, currentUniform->mLocation, glArraySize,
                                                                params->getIntPointer(def->physicalIndex)));
                        break;
                    default:
                        break;

                    } // End switch
                } // Variability & mask
            } // fromProgType == currentUniform->mSourceProgType

        } // End for
    }


    void GLSLSeparableProgram::updateAtomicCounters(GpuProgramParametersSharedPtr params,
                                                    uint16 mask, GpuProgramType fromProgType)
    {
        // Iterate through the list of atomic counter buffers and update them as needed
        // GLAtomicBufferIterator currentBuffer = mGLAtomicBufferReferences.begin();
        // GLAtomicBufferIterator endBuffer = mGLAtomicBufferReferences.end();

        GLAtomicCounterReferenceIterator currentAtomicCounter = mGLAtomicCounterReferences.begin();
        GLAtomicCounterReferenceIterator endAtomicCounter = mGLAtomicCounterReferences.end();

        for (; currentAtomicCounter != endAtomicCounter; ++currentAtomicCounter)
        {
            if (fromProgType == currentAtomicCounter->mSourceProgType)
            {
                const GpuConstantDefinition* def = currentAtomicCounter->mConstantDef;
                if (def->variability & mask)
                {
                    GLsizei glArraySize = (GLsizei)def->arraySize;

                    // Get the index in the parameter real list
                    //switch (def->constType)

                    GLuint glBinding = currentAtomicCounter->mBinding;
                    GLuint glOffset = currentAtomicCounter->mOffset;

                    // Get the buffer this atomic counter belongs to.
                    //TODO exception handling
                    v1::HardwareCounterBufferSharedPtr atomic_buffer = mGLCounterBufferReferences[glBinding];

                    // Update the value.
                    atomic_buffer->writeData(glOffset, sizeof(GLuint) * glArraySize, params->getUnsignedIntPointer(def->physicalIndex));
                }
            }
        }

        // GpuProgramParameters::GpuSharedParamUsageList::const_iterator it, end = sharedParams.end();
        // for (it = sharedParams.begin(); it != end; ++it)
        // {
        //     for (;currentBuffer != endBuffer; ++currentBuffer)
        //     {
        //         GL3PlusHardwareUniformBuffer* hwGlBuffer = static_cast<GL3PlusHardwareUniformBuffer*>(currentBuffer->get());
        //         GpuSharedParametersPtr paramsPtr = it->getSharedParams();

        //         // Block name is stored in mSharedParams->mName of GpuSharedParamUsageList items
        //         GLint UniformTransform;
        //         OGRE_CHECK_GL_ERROR(UniformTransform = glGetUniformBlockIndex(mGLProgramHandle, it->getName().c_str()));
        //         OGRE_CHECK_GL_ERROR(glUniformBlockBinding(mGLProgramHandle, UniformTransform, hwGlBuffer->getGLBufferBinding()));

        //         hwGlBuffer->writeData(0, hwGlBuffer->getSizeInBytes(), &paramsPtr->getFloatConstantList().front());
        //     }
        // }
    }


    // void GLSLSeparableProgram::updateShaderStorageBlocks(GpuProgramParametersSharedPtr params,
    //                                                     uint16 mask, GpuProgramType fromProgType)
    // {
    //     // Iterate through the list of uniform buffers and update them as needed
    //     GLShaderStorageBufferIterator currentBuffer = mGLShaderStorageBufferReferences.begin();
    //     GLShaderStorageBufferIterator endBuffer = mGLShaderStorageBufferReferences.end();

    //     const GpuProgramParameters::GpuSharedParamUsageList& sharedParams = params->getSharedParameters();

    //     GpuProgramParameters::GpuSharedParamUsageList::const_iterator it, end = sharedParams.end();
    //     for (it = sharedParams.begin(); it != end; ++it)
    //     {
    //         for (;currentBuffer != endBuffer; ++currentBuffer)
    //         {
    //             GL3PlusHardwareShaderStorageBuffer* hwGlBuffer = static_cast<GL3PlusHardwareShaderStorageBuffer*>(currentBuffer->get());
    //             GpuSharedParametersPtr paramsPtr = it->getSharedParams();

    //             // Block name is stored in mSharedParams->mName of GpuSharedParamUsageList items
    //             GLint blockIndex;
    //             OGRE_CHECK_GL_ERROR(blockIndex = glGetProgramResourceIndex(mGLProgramHandle, GL_SHADER_STORAGE_BLOCK, it->getName().c_str()));
    //             OGRE_CHECK_GL_ERROR(glShaderStorageBlockBinding(mGLProgramHandle, blockIndex, hwGlBuffer->getGLBufferBinding()));

    //             //FIXME does not check if current progrtype, or if shared param is active

    //             //FIXME This seems to only support copying float data.
    //             // What about int, uint, double, etc?
    //             hwGlBuffer->writeData(0, hwGlBuffer->getSizeInBytes(), &paramsPtr->getFloatConstantList().front());
    //         }
    //     }
    // }


    void GLSLSeparableProgram::updateUniformBlocks(GpuProgramParametersSharedPtr params,
                                                   uint16 mask, GpuProgramType fromProgType)
    {
        //TODO Support uniform block arrays - need to figure how to do this via material.

        // Iterate through the list of uniform blocks and update them as needed.
        SharedParamsBufferMap::const_iterator currentPair = mSharedParamsBufferMap.begin();
        SharedParamsBufferMap::const_iterator endPair = mSharedParamsBufferMap.end();

        // const GpuProgramParameters::GpuSharedParamUsageList& sharedParams = params->getSharedParameters();

        // const GpuProgramParameters::GpuSharedParamUsageList& sharedParams = params->getSharedParameters();
        // GpuProgramParameters::GpuSharedParamUsageList::const_iterator it, end = sharedParams.end();

        for (; currentPair != endPair; ++currentPair)
        {
            GpuSharedParametersPtr paramsPtr = currentPair->first;

            //FIXME Possible buffer does not exist if no associated uniform block.
            v1::GL3PlusHardwareUniformBuffer* hwGlBuffer = static_cast<v1::GL3PlusHardwareUniformBuffer*>(currentPair->second.get());

            if (!paramsPtr->isDirty()) continue;

            //FIXME does not check if current progrtype, or if shared param is active

            GpuConstantDefinitionIterator parami = paramsPtr->getConstantDefinitionIterator();

            for (int i = 0; parami.current() != parami.end(); parami.moveNext(), i++)
            {
                //String name = parami->;
                //GpuConstantDefinition * param = GpuConstantConstantDefinition(name);

                //const String* name = &parami.current()->first;
                const GpuConstantDefinition* param = &parami.current()->second;

                BaseConstantType baseType = GpuConstantDefinition::getBaseType(param->constType);

                void* dataPtr;

                size_t index =  param->physicalIndex;

                //TODO Maybe move to GpuSharedParams?  Otherwise create bool buffer.
                switch (baseType)
                {
                case BCT_FLOAT:
                    dataPtr = paramsPtr->getFloatPointer(index);
                    break;
                case BCT_INT:
                    dataPtr = paramsPtr->getIntPointer(index);
                    break;
                case BCT_DOUBLE:
                    dataPtr = paramsPtr->getDoublePointer(index);
                    break;
                case BCT_UINT:
                case BCT_BOOL:
                    dataPtr = paramsPtr->getUnsignedIntPointer(index);
                    break;
                case BCT_SAMPLER:
                case BCT_SUBROUTINE:
                    //TODO implement me!
                default:
                    //TODO error handling
                    continue;
                }

                // in bytes
                size_t length = param->arraySize * param->elementSize * 4;
                size_t offset = hwGlBuffer->mBufferParamsLayout.offsets[i];
                hwGlBuffer->writeData(offset, length, dataPtr);
            }

            paramsPtr->_markClean();
        }
    }


    void GLSLSeparableProgram::updatePassIterationUniforms(GpuProgramParametersSharedPtr params)
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
                    GLuint progID = 0;
                    if (mVertexShader && currentUniform->mSourceProgType == GPT_VERTEX_PROGRAM)
                    {
                        progID = mVertexShader->getGLProgramHandle();
                    }

                    if (mFragmentShader && currentUniform->mSourceProgType == GPT_FRAGMENT_PROGRAM)
                    {
                        progID = mFragmentShader->getGLProgramHandle();
                    }

                    if (mGeometryShader && currentUniform->mSourceProgType == GPT_GEOMETRY_PROGRAM)
                    {
                        progID = mGeometryShader->getGLProgramHandle();
                    }

                    if (mDomainShader && currentUniform->mSourceProgType == GPT_DOMAIN_PROGRAM)
                    {
                        progID = mDomainShader->getGLProgramHandle();
                    }

                    if (mHullShader && currentUniform->mSourceProgType == GPT_HULL_PROGRAM)
                    {
                        progID = mHullShader->getGLProgramHandle();
                    }

                    if (mComputeShader && currentUniform->mSourceProgType == GPT_COMPUTE_PROGRAM)
                    {
                        progID = mComputeShader->getGLProgramHandle();
                    }

                    OGRE_CHECK_GL_ERROR(glProgramUniform1fv(progID, currentUniform->mLocation, 1, params->getFloatPointer(index)));

                    // There will only be one multipass entry
                    return;
                }
            }
        }
    }
}
