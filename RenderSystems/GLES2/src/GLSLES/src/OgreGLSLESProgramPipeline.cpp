/*
 -----------------------------------------------------------------------------
 This source file is part of OGRE
 (Object-oriented Graphics Rendering Engine)
 For the latest info, see http://www.ogre3d.org/
 
 Copyright (c) 2000-2012 Torus Knot Software Ltd
 
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

#include "OgreGLSLESProgramPipeline.h"
#include "OgreStringConverter.h"
#include "OgreGLSLESGpuProgram.h"
#include "OgreGLSLESProgram.h"
#include "OgreGLSLESProgramPipelineManager.h"
#include "OgreGpuProgramManager.h"

namespace Ogre
{
    GLSLESProgramPipeline::GLSLESProgramPipeline(GLSLESGpuProgram* vertexProgram, GLSLESGpuProgram* fragmentProgram) : 
    GLSLESProgramCommon(vertexProgram, fragmentProgram) { }

    GLSLESProgramPipeline::~GLSLESProgramPipeline()
    {
#if GL_EXT_separate_shader_objects
        glDeleteProgramPipelinesEXT(1, &mGLProgramPipelineHandle);
        GL_CHECK_ERROR;
#endif
    }

    void GLSLESProgramPipeline::compileAndLink()
	{
#if GL_EXT_separate_shader_objects
        GLint linkStatus = 0;
        
        glGenProgramPipelinesEXT(1, &mGLProgramPipelineHandle);
        GL_CHECK_ERROR
        glBindProgramPipelineEXT(mGLProgramPipelineHandle);
        GL_CHECK_ERROR

		// Compile and attach Vertex Program
        if(mVertexProgram && !mVertexProgram->isLinked())
        {
            if (!mVertexProgram->getGLSLProgram()->compile(true))
            {
                mTriedToLinkAndFailed = true;
                return;
            }
            GLuint programHandle = mVertexProgram->getGLSLProgram()->getGLProgramHandle();
            glProgramParameteriEXT(programHandle, GL_PROGRAM_SEPARABLE_EXT, GL_TRUE);
            GL_CHECK_ERROR
            mVertexProgram->getGLSLProgram()->attachToProgramObject(programHandle);
            glLinkProgram(programHandle);
            GL_CHECK_ERROR
            glGetProgramiv(programHandle, GL_LINK_STATUS, &linkStatus);
            GL_CHECK_ERROR
            
            if(linkStatus)
            {
                mVertexProgram->setLinked(linkStatus);
                mLinked |= VERTEX_PROGRAM_LINKED;
            }
            
            GL_CHECK_ERROR
            // TODO: Just fail out here if the program doesn't link?
            mTriedToLinkAndFailed = !linkStatus;
            
            logObjectInfo( getCombinedName() + String("GLSL vertex program result : "), programHandle );
            
            setSkeletalAnimationIncluded(mVertexProgram->isSkeletalAnimationIncluded());
        }
        
		// Compile and attach Fragment Program
        if(mFragmentProgram && !mFragmentProgram->isLinked())
        {
            if (!mFragmentProgram->getGLSLProgram()->compile(true))
            {
                mTriedToLinkAndFailed = true;
                return;
            }

            GLuint programHandle = mFragmentProgram->getGLSLProgram()->getGLProgramHandle();
            glProgramParameteriEXT(programHandle, GL_PROGRAM_SEPARABLE_EXT, GL_TRUE);
            GL_CHECK_ERROR
            mFragmentProgram->getGLSLProgram()->attachToProgramObject(programHandle);
            glLinkProgram(programHandle);
            GL_CHECK_ERROR
            glGetProgramiv(programHandle, GL_LINK_STATUS, &linkStatus);
            GL_CHECK_ERROR
            
            if(linkStatus)
            {
                mFragmentProgram->setLinked(linkStatus);
                mLinked |= FRAGMENT_PROGRAM_LINKED;
            }
            
            mTriedToLinkAndFailed = !linkStatus;
            
            logObjectInfo( getCombinedName() + String("GLSL fragment program result : "), programHandle );
        }
        
		if(mLinked)
		{
			if ( GpuProgramManager::getSingleton().getSaveMicrocodesToCache() )
			{
				// Add to the microcode to the cache
				String name;
				name = getCombinedName();

				// Get buffer size
				GLint binaryLength = 0;

#if GL_OES_get_program_binary
				glGetProgramiv(mGLHandle, GL_PROGRAM_BINARY_LENGTH_OES, &binaryLength);
                GL_CHECK_ERROR;
#endif

                // Create microcode
                GpuProgramManager::Microcode newMicrocode = 
                    GpuProgramManager::getSingleton().createMicrocode((unsigned long)binaryLength + sizeof(GLenum));

#if GL_OES_get_program_binary
				// Get binary
				glGetProgramBinaryOES(mGLHandle, binaryLength, NULL, (GLenum *)newMicrocode->getPtr(), newMicrocode->getPtr() + sizeof(GLenum));
                GL_CHECK_ERROR;
#endif

        		// Add to the microcode to the cache
				GpuProgramManager::getSingleton().addMicrocodeToCache(name, newMicrocode);
			}
            if(mVertexProgram && mVertexProgram->isLinked())
            {
                glUseProgramStagesEXT(mGLProgramPipelineHandle, GL_VERTEX_SHADER_BIT_EXT, mVertexProgram->getGLSLProgram()->getGLProgramHandle());
                GL_CHECK_ERROR
            }
            if(mFragmentProgram && mFragmentProgram->isLinked())
            {
                glUseProgramStagesEXT(mGLProgramPipelineHandle, GL_FRAGMENT_SHADER_BIT_EXT, mFragmentProgram->getGLSLProgram()->getGLProgramHandle());
                GL_CHECK_ERROR
            }

            // Validate pipeline
            logObjectInfo( getCombinedName() + String("GLSL program pipeline result : "), mGLProgramPipelineHandle );
		}
#endif
	}

    void GLSLESProgramPipeline::_useProgram(void)
    {
		if (mLinked)
		{
#if GL_EXT_separate_shader_objects
            GL_CHECK_ERROR
            glBindProgramPipelineEXT(mGLProgramPipelineHandle);
            GL_CHECK_ERROR
#endif
		}
    }

	//-----------------------------------------------------------------------
	GLint GLSLESProgramPipeline::getAttributeIndex(VertexElementSemantic semantic, uint index)
	{
		GLint res = mCustomAttributesIndexes[semantic-1][index];
		if (res == NULL_CUSTOM_ATTRIBUTES_INDEX)
		{
            GLuint handle = mVertexProgram->getGLSLProgram()->getGLProgramHandle();
			const char * attString = getAttributeSemanticString(semantic);
			GLint attrib = glGetAttribLocation(handle, attString);
            GL_CHECK_ERROR;

			// Sadly position is a special case 
			if (attrib == NOT_FOUND_CUSTOM_ATTRIBUTES_INDEX && semantic == VES_POSITION)
			{
				attrib = glGetAttribLocation(handle, "position");
                GL_CHECK_ERROR;
			}
            
			// For uv and other case the index is a part of the name
			if (attrib == NOT_FOUND_CUSTOM_ATTRIBUTES_INDEX)
			{
				String attStringWithSemantic = String(attString) + StringConverter::toString(index);
				attrib = glGetAttribLocation(handle, attStringWithSemantic.c_str());
                GL_CHECK_ERROR;
			}
            
			// Update mCustomAttributesIndexes with the index we found (or didn't find) 
			mCustomAttributesIndexes[semantic-1][index] = attrib;
			res = attrib;
		}
        
		return res;
	}

    //-----------------------------------------------------------------------
	void GLSLESProgramPipeline::activate(void)
	{
		if (!mLinked && !mTriedToLinkAndFailed)
		{
			glGetError(); // Clean up the error. Otherwise will flood log.
			GL_CHECK_ERROR
            
			if ( GpuProgramManager::getSingleton().canGetCompiledShaderBuffer() &&
				GpuProgramManager::getSingleton().isMicrocodeAvailableInCache(getCombinedName()) )
			{
				getMicrocodeFromCache();
			}
			else
			{
#if !OGRE_NO_GLES2_GLSL_OPTIMISER
                // Check CmdParams for each shader type to see if we should optimise
                if(mVertexProgram)
                {
                    String paramStr = mVertexProgram->getGLSLProgram()->getParameter("use_optimiser");
                    if((paramStr == "true") || paramStr.empty())
                    {
                        GLSLESProgramPipelineManager::getSingleton().optimiseShaderSource(mVertexProgram);
                    }
                }

                if(mFragmentProgram)
                {
                    String paramStr = mFragmentProgram->getGLSLProgram()->getParameter("use_optimiser");
                    if((paramStr == "true") || paramStr.empty())
                    {
                        GLSLESProgramPipelineManager::getSingleton().optimiseShaderSource(mFragmentProgram);
                    }
                }
#endif
				compileAndLink();
			}

            extractLayoutQualifiers();

			buildGLUniformReferences();
		}

        _useProgram();
	}

    //-----------------------------------------------------------------------
	void GLSLESProgramPipeline::buildGLUniformReferences(void)
	{
		if (!mUniformRefsBuilt)
		{
			const GpuConstantDefinitionMap* vertParams = 0;
			const GpuConstantDefinitionMap* fragParams = 0;
			if (mVertexProgram)
			{
				vertParams = &(mVertexProgram->getGLSLProgram()->getConstantDefinitions().map);
                GLSLESProgramPipelineManager::getSingleton().extractUniforms(mVertexProgram->getGLSLProgram()->getGLProgramHandle(),
                                                                         vertParams, NULL, mGLUniformReferences);
			}
			if (mFragmentProgram)
			{
				fragParams = &(mFragmentProgram->getGLSLProgram()->getConstantDefinitions().map);
                GLSLESProgramPipelineManager::getSingleton().extractUniforms(mFragmentProgram->getGLSLProgram()->getGLProgramHandle(),
                                                                         NULL, fragParams, mGLUniformReferences);
			}

			mUniformRefsBuilt = true;
		}
	}

	//-----------------------------------------------------------------------
	void GLSLESProgramPipeline::updateUniforms(GpuProgramParametersSharedPtr params, 
                                           uint16 mask, GpuProgramType fromProgType)
	{
		// Iterate through uniform reference list and update uniform values
		GLUniformReferenceIterator currentUniform = mGLUniformReferences.begin();
		GLUniformReferenceIterator endUniform = mGLUniformReferences.end();
#if GL_EXT_separate_shader_objects
        GLuint progID = 0;
        if(fromProgType == GPT_VERTEX_PROGRAM)
            progID = mVertexProgram->getGLSLProgram()->getGLProgramHandle();
        else if(fromProgType == GPT_FRAGMENT_PROGRAM)
            progID = mFragmentProgram->getGLSLProgram()->getGLProgramHandle();
        
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
                            glProgramUniform1fvEXT(progID, currentUniform->mLocation, glArraySize, 
                                                   params->getFloatPointer(def->physicalIndex));
                            break;
                        case GCT_FLOAT2:
                            glProgramUniform2fvEXT(progID, currentUniform->mLocation, glArraySize, 
                                                   params->getFloatPointer(def->physicalIndex));
                            break;
                        case GCT_FLOAT3:
                            glProgramUniform3fvEXT(progID, currentUniform->mLocation, glArraySize, 
                                                   params->getFloatPointer(def->physicalIndex));
                            break;
                        case GCT_FLOAT4:
                            glProgramUniform4fvEXT(progID, currentUniform->mLocation, glArraySize, 
                                                   params->getFloatPointer(def->physicalIndex));
                            break;
                        case GCT_MATRIX_2X2:
                            glProgramUniformMatrix2fvEXT(progID, currentUniform->mLocation, glArraySize, 
                                                         GL_FALSE, params->getFloatPointer(def->physicalIndex));
                            break;
                        case GCT_MATRIX_3X3:
                            glProgramUniformMatrix3fvEXT(progID, currentUniform->mLocation, glArraySize, 
                                                         GL_FALSE, params->getFloatPointer(def->physicalIndex));
                            break;
                        case GCT_MATRIX_4X4:
                            glProgramUniformMatrix4fvEXT(progID, currentUniform->mLocation, glArraySize, 
                                                         GL_FALSE, params->getFloatPointer(def->physicalIndex));
                            break;
                        case GCT_INT1:
                            glProgramUniform1ivEXT(progID, currentUniform->mLocation, glArraySize, 
                                                   params->getIntPointer(def->physicalIndex));
                            break;
                        case GCT_INT2:
                            glProgramUniform2ivEXT(progID, currentUniform->mLocation, glArraySize, 
                                                   params->getIntPointer(def->physicalIndex));
                            break;
                        case GCT_INT3:
                            glProgramUniform3ivEXT(progID, currentUniform->mLocation, glArraySize, 
                                                   params->getIntPointer(def->physicalIndex));
                            break;
                        case GCT_INT4:
                            glProgramUniform4ivEXT(progID, currentUniform->mLocation, glArraySize, 
                                                   params->getIntPointer(def->physicalIndex));
                            break;
                        case GCT_SAMPLER1D:
                        case GCT_SAMPLER1DSHADOW:
                        case GCT_SAMPLER2D:
                        case GCT_SAMPLER2DSHADOW:
                        case GCT_SAMPLER3D:
                        case GCT_SAMPLERCUBE:
                            // Samplers handled like 1-element ints
                            glProgramUniform1ivEXT(progID, currentUniform->mLocation, 1, 
                                                   params->getIntPointer(def->physicalIndex));
                            break;
                        case GCT_MATRIX_2X3:
                        case GCT_MATRIX_2X4:
                        case GCT_MATRIX_3X2:
                        case GCT_MATRIX_3X4:
                        case GCT_MATRIX_4X2:
                        case GCT_MATRIX_4X3:
                        case GCT_SAMPLER2DARRAY:
                        case GCT_UNKNOWN:
                            break;
                            
					} // End switch
                    GL_CHECK_ERROR;
				} // Variability & mask
			} // fromProgType == currentUniform->mSourceProgType
            
  		} // End for
#endif
	}
	//-----------------------------------------------------------------------
	void GLSLESProgramPipeline::updatePassIterationUniforms(GpuProgramParametersSharedPtr params)
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
#if GL_EXT_separate_shader_objects
                    GLuint progID = 0;
                    if (mVertexProgram && currentUniform->mSourceProgType == GPT_VERTEX_PROGRAM)
                    {
                        progID = mVertexProgram->getGLSLProgram()->getGLProgramHandle();
                        glProgramUniform1fvEXT(progID, currentUniform->mLocation, 1, params->getFloatPointer(index));
                        GL_CHECK_ERROR;
                    }
                    
                    if (mFragmentProgram && currentUniform->mSourceProgType == GPT_FRAGMENT_PROGRAM)
                    {
                        progID = mFragmentProgram->getGLSLProgram()->getGLProgramHandle();
                        glProgramUniform1fvEXT(progID, currentUniform->mLocation, 1, params->getFloatPointer(index));
                        GL_CHECK_ERROR;
                    }
#endif
					// There will only be one multipass entry
					return;
				}
			}
		}
    }
    //-----------------------------------------------------------------------
    void GLSLESProgramPipeline::extractLayoutQualifiers(void)
    {
        // Format is:
        //      layout(location = 0) attribute vec4 vertex;

        if(mVertexProgram)
        {
            String shaderSource = mVertexProgram->getGLSLProgram()->getSource();
            String::size_type currPos = shaderSource.find("layout");
            while (currPos != String::npos)
            {
                VertexElementSemantic semantic;
                GLint index = 0;
                
                String::size_type endPos = shaderSource.find(";", currPos);
				if (endPos == String::npos)
				{
					// Problem, missing semicolon, abort
					break;
				}
                
				String line = shaderSource.substr(currPos, endPos - currPos);
                
                // Skip over 'layout'
                currPos += 6;
                
                // Skip until '='
                String::size_type eqPos = line.find("=");
                String::size_type parenPos = line.find(")");
                
                // Skip past '=' up to a ')' which contains an integer(the position).  This could be a definition, does the preprocessor do replacement?
                String attrLocation = line.substr(eqPos + 1, parenPos - eqPos - 1);
                StringUtil::trim(attrLocation);
                GLint attrib = StringConverter::parseInt(attrLocation);
                
                // The rest of the line is a standard attribute definition.
                // Erase up to it then split the remainder by spaces.
                line.erase (0, parenPos + 1);
                StringUtil::trim(line);
				StringVector parts = StringUtil::split(line, " ");
                
                if(parts.size() < 3)
                {
                    // This is a malformed attribute
                    // It should contain 3 parts, i.e. "attribute vec4 vertex"
                    break;
                }
                
                String attrName = parts[2];
                
                // Special case for attribute named position
                if(attrName == "position")
                    semantic = getAttributeSemanticEnum("vertex");
                else
                    semantic = getAttributeSemanticEnum(attrName);
                
                // Find the texture unit index
                String::size_type uvPos = attrName.find("uv");
                if(uvPos != String::npos)
                {
                    String uvIndex = attrName.substr(uvPos + 2, attrName.length() - 2);
                    index = StringConverter::parseInt(uvIndex);
                }
                
                mCustomAttributesIndexes[semantic-1][index] = attrib;
                
                currPos = shaderSource.find("layout", currPos);
            }
        }
    }
}
