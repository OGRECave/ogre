/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2011 Torus Knot Software Ltd

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
#include "OgreGLSLESExtSupport.h"
#include "OgreGLSLESGpuProgram.h"
#include "OgreGLSLESProgram.h"
#include "OgreGLSLESLinkProgramManager.h"
#include "OgreGLES2RenderSystem.h"
#include "OgreStringVector.h"
#include "OgreLogManager.h"
#include "OgreGpuProgramManager.h"
#include "OgreStringConverter.h"

namespace Ogre {

	//-----------------------------------------------------------------------
	GLSLESLinkProgram::GLSLESLinkProgram(GLSLESGpuProgram* vertexProgram, GLSLESGpuProgram* fragmentProgram)
        : mVertexProgram(vertexProgram)
		, mFragmentProgram(fragmentProgram)
		, mUniformRefsBuilt(false)
        , mLinked(false)
		, mTriedToLinkAndFailed(false)
	{
		// init CustomAttributesIndexs
		for(size_t i = 0 ; i < VES_COUNT; i++)
			for(size_t j = 0 ; j < OGRE_MAX_TEXTURE_COORD_SETS; j++)
		{
			mCustomAttributesIndexes[i][j] = NULL_CUSTOM_ATTRIBUTES_INDEX;
		}
        
        if (!mVertexProgram || !mFragmentProgram)
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        "Attempted to create a shader program without both a vertex and fragment program.",
                        "GLSLESLinkProgram::GLSLESLinkProgram");
        }
	}

	//-----------------------------------------------------------------------
	GLSLESLinkProgram::~GLSLESLinkProgram(void)
	{
		glDeleteProgram(mGLHandle);
        GL_CHECK_ERROR;
	}

	//-----------------------------------------------------------------------
	Ogre::String GLSLESLinkProgram::getCombinedName()
	{
		String name;
		if (mVertexProgram)
		{
			name += "Vertex Program:" ;
			name += mVertexProgram->getName();
		}
		if (mFragmentProgram)
		{
			name += " Fragment Program:" ;
			name += mFragmentProgram->getName();
		}
		return name;
	}
	//-----------------------------------------------------------------------
	void GLSLESLinkProgram::activate(void)
	{
		if (!mLinked && !mTriedToLinkAndFailed)
		{
			glGetError(); // Clean up the error. Otherwise will flood log.
			mGLHandle = glCreateProgram();
			GL_CHECK_ERROR

			if ( GpuProgramManager::getSingleton().canGetCompiledShaderBuffer() &&
				GpuProgramManager::getSingleton().isMicrocodeAvailableInCache(getCombinedName()) )
			{
				getMicrocodeFromCache();
			}
			else
			{
#ifdef OGRE_USE_GLES2_GLSL_OPTIMISER
                // check CmdParams for each shader type to see if we should optimize
                String paramStr = mVertexProgram->getGLSLProgram()->getParameter("use_optimiser");
                if((paramStr == "true") || paramStr.empty())
                {
                    GLSLESLinkProgramManager::getSingleton().optimiseShaderSource(mVertexProgram);
                }
                paramStr = mFragmentProgram->getGLSLProgram()->getParameter("use_optimiser");
                if((paramStr == "true") || paramStr.empty())
                {
                    GLSLESLinkProgramManager::getSingleton().optimiseShaderSource(mFragmentProgram);
                }
#endif
				compileAndLink();
			}

			buildGLUniformReferences();
		}

		if (mLinked)
		{
            GL_CHECK_ERROR
            glUseProgram( mGLHandle );
            GL_CHECK_ERROR
		}
	}
	//-----------------------------------------------------------------------
	void GLSLESLinkProgram::getMicrocodeFromCache(void)
	{
		GpuProgramManager::Microcode cacheMicrocode = 
			GpuProgramManager::getSingleton().getMicrocodeFromCache(getCombinedName());

		// add to the microcode to the cache
		String name;
		name = getCombinedName();

		// turns out we need this param when loading
		GLenum binaryFormat = 0;

		cacheMicrocode->seek(0);

		// get size of binary
		cacheMicrocode->read(&binaryFormat, sizeof(GLenum));

#if GL_OES_get_program_binary
        GLint binaryLength = cacheMicrocode->size() - sizeof(GLenum);

        // load binary
		glProgramBinaryOES( mGLHandle, 
							binaryFormat, 
							cacheMicrocode->getPtr(),
							binaryLength
			);
        GL_CHECK_ERROR;
#endif
		GLint   success = 0;
		glGetProgramiv(mGLHandle, GL_LINK_STATUS, &success);
    GL_CHECK_ERROR;
		if (!success)
		{
			//
			// Something must have changed since the program binaries
			// were cached away.  Fallback to source shader loading path,
			// and then retrieve and cache new program binaries once again.
			//
			compileAndLink();
		}

	}

	//-----------------------------------------------------------------------
	void GLSLESLinkProgram::compileAndLink()
	{
		// compile and attach Vertex Program
		if (!mVertexProgram->getGLSLProgram()->compile(true))
		{
			// todo error
            mTriedToLinkAndFailed = true;
			return;
		}
        mVertexProgram->getGLSLProgram()->attachToProgramObject(mGLHandle);
        setSkeletalAnimationIncluded(mVertexProgram->isSkeletalAnimationIncluded());
        
		// compile and attach Fragment Program
		if (!mFragmentProgram->getGLSLProgram()->compile(true))
		{
			// todo error
            mTriedToLinkAndFailed = true;
			return;
		}		
        mFragmentProgram->getGLSLProgram()->attachToProgramObject(mGLHandle);

		// the link
		glLinkProgram( mGLHandle );
		GL_CHECK_ERROR
        glGetProgramiv( mGLHandle, GL_LINK_STATUS, &mLinked );
		GL_CHECK_ERROR
	
		mTriedToLinkAndFailed = !mLinked;

		logObjectInfo( getCombinedName() + String(" GLSL link result : "), mGLHandle );
		if(mLinked)
		{
			if ( GpuProgramManager::getSingleton().getSaveMicrocodesToCache() )
			{
				// add to the microcode to the cache
				String name;
				name = getCombinedName();

				// get buffer size
				GLint binaryLength = 0;
#if GL_OES_get_program_binary
				glGetProgramiv(mGLHandle, GL_PROGRAM_BINARY_LENGTH_OES, &binaryLength);
                GL_CHECK_ERROR;
#endif

                // create microcode
                GpuProgramManager::Microcode newMicrocode = 
                    GpuProgramManager::getSingleton().createMicrocode(binaryLength + sizeof(GLenum));

#if GL_OES_get_program_binary
				// get binary
				glGetProgramBinaryOES(mGLHandle, binaryLength, NULL, (GLenum *)newMicrocode->getPtr(), newMicrocode->getPtr() + sizeof(GLenum));
                GL_CHECK_ERROR;
#endif

        		// add to the microcode to the cache
				GpuProgramManager::getSingleton().addMicrocodeToCache(name, newMicrocode);
			}
		}
	}
	//-----------------------------------------------------------------------
	const char * getAttributeSemanticString(VertexElementSemantic semantic)
	{
		switch(semantic)
		{
			case VES_POSITION:
				return "vertex";
			case VES_BLEND_WEIGHTS:
				return "blendWeights";
			case VES_NORMAL:
				return "normal";
			case VES_DIFFUSE:
				return "colour";
			case VES_SPECULAR:
				return "secondary_colour";
			case VES_BLEND_INDICES:
				return "blendIndices";
			case VES_TANGENT:
				return "tangent";
			case VES_BINORMAL:
				return "binormal";
			case VES_TEXTURE_COORDINATES:
				return "uv";
			default:
				assert(false && "Missing attribute!");
				return 0;
		};

	}
	//-----------------------------------------------------------------------
	GLuint GLSLESLinkProgram::getAttributeIndex(VertexElementSemantic semantic, uint index)
	{
		GLint res = mCustomAttributesIndexes[semantic-1][index];
		if (res == NULL_CUSTOM_ATTRIBUTES_INDEX)
		{
			const char * attString = getAttributeSemanticString(semantic);
			GLint attrib = glGetAttribLocation(mGLHandle, attString);
            GL_CHECK_ERROR;

			// sadly position is a special case 
			if (attrib == NOT_FOUND_CUSTOM_ATTRIBUTES_INDEX && semantic == VES_POSITION)
			{
				attrib = glGetAttribLocation(mGLHandle, "position");
            GL_CHECK_ERROR;
			}

			// for uv and other case the index is a part of the name
			if (attrib == NOT_FOUND_CUSTOM_ATTRIBUTES_INDEX)
			{
				String attStringWithSemantic = String(attString) + StringConverter::toString(index);
				attrib = glGetAttribLocation(mGLHandle, attStringWithSemantic.c_str());
            GL_CHECK_ERROR;
			}

			// update mCustomAttributesIndexes with the index we found (or didn't find) 
			mCustomAttributesIndexes[semantic-1][index] = attrib;
			res = attrib;
		}
		return (GLuint)res;
	}
	//-----------------------------------------------------------------------
	bool GLSLESLinkProgram::isAttributeValid(VertexElementSemantic semantic, uint index)
	{
		return (GLint)(getAttributeIndex(semantic, index)) != NOT_FOUND_CUSTOM_ATTRIBUTES_INDEX;
	}
	//-----------------------------------------------------------------------
	void GLSLESLinkProgram::buildGLUniformReferences(void)
	{
		if (!mUniformRefsBuilt)
		{
			const GpuConstantDefinitionMap* vertParams = 0;
			const GpuConstantDefinitionMap* fragParams = 0;
			if (mVertexProgram)
			{
				vertParams = &(mVertexProgram->getGLSLProgram()->getConstantDefinitions().map);
			}
			if (mFragmentProgram)
			{
				fragParams = &(mFragmentProgram->getGLSLProgram()->getConstantDefinitions().map);
			}

			GLSLESLinkProgramManager::getSingleton().extractUniforms(
				mGLHandle, vertParams, fragParams, mGLUniformReferences);

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

					// Get the index in the parameter real list
					switch (def->constType)
					{
					case GCT_FLOAT1:
						glUniform1fv(currentUniform->mLocation, glArraySize, 
							params->getFloatPointer(def->physicalIndex));
						break;
					case GCT_FLOAT2:
						glUniform2fv(currentUniform->mLocation, glArraySize, 
							params->getFloatPointer(def->physicalIndex));
						break;
					case GCT_FLOAT3:
						glUniform3fv(currentUniform->mLocation, glArraySize, 
							params->getFloatPointer(def->physicalIndex));
						break;
					case GCT_FLOAT4:
						glUniform4fv(currentUniform->mLocation, glArraySize, 
							params->getFloatPointer(def->physicalIndex));
						break;
					case GCT_MATRIX_2X2:
						glUniformMatrix2fv(currentUniform->mLocation, glArraySize, 
							GL_FALSE, params->getFloatPointer(def->physicalIndex));
						break;
					case GCT_MATRIX_3X3:
						glUniformMatrix3fv(currentUniform->mLocation, glArraySize, 
							GL_FALSE, params->getFloatPointer(def->physicalIndex));
						break;
					case GCT_MATRIX_4X4:
						glUniformMatrix4fv(currentUniform->mLocation, glArraySize, 
							GL_FALSE, params->getFloatPointer(def->physicalIndex));
						break;
					case GCT_INT1:
						glUniform1iv(currentUniform->mLocation, glArraySize, 
							(GLint*)params->getIntPointer(def->physicalIndex));
						break;
					case GCT_INT2:
						glUniform2iv(currentUniform->mLocation, glArraySize, 
							(GLint*)params->getIntPointer(def->physicalIndex));
						break;
					case GCT_INT3:
						glUniform3iv(currentUniform->mLocation, glArraySize, 
							(GLint*)params->getIntPointer(def->physicalIndex));
						break;
					case GCT_INT4:
						glUniform4iv(currentUniform->mLocation, glArraySize, 
							(GLint*)params->getIntPointer(def->physicalIndex));
						break;
					case GCT_SAMPLER1D:
					case GCT_SAMPLER1DSHADOW:
					case GCT_SAMPLER2D:
					case GCT_SAMPLER2DSHADOW:
					case GCT_SAMPLER3D:
					case GCT_SAMPLERCUBE:
						// Samplers handled like 1-element ints
						glUniform1iv(currentUniform->mLocation, 1, 
							(GLint*)params->getIntPointer(def->physicalIndex));
						break;
					case GCT_MATRIX_2X3:
					case GCT_MATRIX_2X4:
					case GCT_MATRIX_3X2:
					case GCT_MATRIX_3X4:
					case GCT_MATRIX_4X2:
					case GCT_MATRIX_4X3:
                    case GCT_UNKNOWN:
                        break;

					} // End switch
                    GL_CHECK_ERROR;
				} // Variability & mask
			} // fromProgType == currentUniform->mSourceProgType
  
  		} // End for
	}
	//-----------------------------------------------------------------------
	void GLSLESLinkProgram::updatePassIterationUniforms(GpuProgramParametersSharedPtr params)
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
					glUniform1fv(currentUniform->mLocation, 1, params->getFloatPointer(index));
                    GL_CHECK_ERROR;
					// There will only be one multipass entry
					return;
				}
			}
		}
    }
} // namespace Ogre
