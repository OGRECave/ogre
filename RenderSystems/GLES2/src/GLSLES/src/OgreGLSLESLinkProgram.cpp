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

#include "OgreGLSLESLinkProgram.h"
#include "OgreGLSLESGpuProgram.h"
#include "OgreGLSLESProgram.h"
#include "OgreGLSLESLinkProgramManager.h"
#include "OgreLogManager.h"
#include "OgreGpuProgramManager.h"
#include "OgreStringConverter.h"

namespace Ogre {

	//-----------------------------------------------------------------------
	GLSLESLinkProgram::GLSLESLinkProgram(GLSLESGpuProgram* vertexProgram, GLSLESGpuProgram* fragmentProgram)
    : GLSLESProgramCommon(vertexProgram, fragmentProgram)
	{
        if ((!mVertexProgram || !mFragmentProgram))
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        "Attempted to create a shader program without both a vertex and fragment program.",
                        "GLSLESLinkProgram::GLSLESLinkProgram");
        }
	}

	//-----------------------------------------------------------------------
	GLSLESLinkProgram::~GLSLESLinkProgram(void)
	{
		glDeleteProgram(mGLProgramHandle);
        GL_CHECK_ERROR;
	}

    void GLSLESLinkProgram::_useProgram(void)
    {
		if (mLinked)
		{
            GL_CHECK_ERROR
            glUseProgram( mGLProgramHandle );
            GL_CHECK_ERROR
		}
    }

	//-----------------------------------------------------------------------
	void GLSLESLinkProgram::activate(void)
	{
		if (!mLinked && !mTriedToLinkAndFailed)
		{
			glGetError(); // Clean up the error. Otherwise will flood log.

			mGLProgramHandle = glCreateProgram();
 			GL_CHECK_ERROR

			if ( GpuProgramManager::getSingleton().canGetCompiledShaderBuffer() &&
				GpuProgramManager::getSingleton().isMicrocodeAvailableInCache(getCombinedName()) )
			{
				getMicrocodeFromCache();
			}
			else
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
			}

            extractLayoutQualifiers();
			buildGLUniformReferences();
		}

        _useProgram();
	}

	//-----------------------------------------------------------------------
	void GLSLESLinkProgram::compileAndLink()
	{
		// Compile and attach Vertex Program
		if (!mVertexProgram->getGLSLProgram()->compile(true))
		{
			// TODO error
            mTriedToLinkAndFailed = true;
			return;
		}
        mVertexProgram->getGLSLProgram()->attachToProgramObject(mGLProgramHandle);
        setSkeletalAnimationIncluded(mVertexProgram->isSkeletalAnimationIncluded());
        
		// Compile and attach Fragment Program
		if (!mFragmentProgram->getGLSLProgram()->compile(true))
		{
			// TODO error
            mTriedToLinkAndFailed = true;
			return;
		}
        mFragmentProgram->getGLSLProgram()->attachToProgramObject(mGLProgramHandle);
        
        // The link
        glLinkProgram( mGLProgramHandle );
        GL_CHECK_ERROR
        glGetProgramiv( mGLProgramHandle, GL_LINK_STATUS, &mLinked );
        GL_CHECK_ERROR
        mTriedToLinkAndFailed = !mLinked;

        logObjectInfo( getCombinedName() + String("GLSL link result : "), mGLProgramHandle );

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
				glGetProgramiv(mGLProgramHandle, GL_PROGRAM_BINARY_LENGTH_OES, &binaryLength);
                GL_CHECK_ERROR;
#endif

                // Create microcode
                GpuProgramManager::Microcode newMicrocode = 
                    GpuProgramManager::getSingleton().createMicrocode((ulong)binaryLength + sizeof(GLenum));

#if GL_OES_get_program_binary
				// Get binary
				glGetProgramBinaryOES(mGLProgramHandle, binaryLength, NULL, (GLenum *)newMicrocode->getPtr(),
                                      newMicrocode->getPtr() + sizeof(GLenum));
                GL_CHECK_ERROR;
#endif

        		// Add to the microcode to the cache
				GpuProgramManager::getSingleton().addMicrocodeToCache(name, newMicrocode);
			}
		}
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
				mGLProgramHandle, vertParams, fragParams, mGLUniformReferences);

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
                    case GCT_SAMPLER2DARRAY:
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
