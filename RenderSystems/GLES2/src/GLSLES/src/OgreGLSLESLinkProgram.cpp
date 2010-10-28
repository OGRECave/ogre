/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
#include "OgreStringVector.h"
#include "OgreLogManager.h"
#include "OgreGpuProgramManager.h"

namespace Ogre {

	//  a  builtin				custom attrib name
	// ----------------------------------------------
	//	0  gl_Vertex			vertex
	//  1  n/a					blendWeights		
	//	2  gl_Normal			normal
	//	3  gl_Color				colour
	//	4  gl_SecondaryColor	secondary_colour
	//	5  gl_FogCoord			fog_coord
	//  7  n/a					blendIndices
	//  8 tangent
	//  9 binormal
	//	10  gl_MultiTexCoord0	uv0
	//	11  gl_MultiTexCoord1	uv1
	//	12 gl_MultiTexCoord2	uv2
	//	13 gl_MultiTexCoord3	uv3
	//	14 gl_MultiTexCoord4	uv4
	//	15 gl_MultiTexCoord5	uv5
	//	16 gl_MultiTexCoord6	uv6
	//	17 gl_MultiTexCoord7	uv7
#define NUMBER_OF_CUSTOM_ATTS 18

	//-----------------------------------------------------------------------
	GLSLESLinkProgram::GLSLESLinkProgram(GLSLESGpuProgram* vertexProgram, GLSLESGpuProgram* fragmentProgram)
        : mVertexProgram(vertexProgram)
		, mFragmentProgram(fragmentProgram)
		, mUniformRefsBuilt(false)
        , mLinked(false)
		, mTriedToLinkAndFailed(false)
	{
		// init CustomAttributesIndexs
		mCustomAttributesIndexs.resize(NUMBER_OF_CUSTOM_ATTS);

		for(size_t i = 0 ; i < NUMBER_OF_CUSTOM_ATTS; i++)
		{
			mCustomAttributesIndexs[i] = -1;
		}
        
        if (!mVertexProgram && !mFragmentProgram)
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
			if ( GpuProgramManager::getSingleton().canGetCompiledShaderBuffer() &&
				GpuProgramManager::getSingleton().isMicrocodeAvailableInCache(getCombinedName()) )
			{
				getMicrocodeFromCache();
			}
			else
			{
				compileAndLink();
			}

			buildGLUniformReferences();
			extractAttributes();
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

		// buffer size
		GLint binaryLength = cacheMicrocode->size() - sizeof(GLenum);

		// turns out we need this param when loading
		GLenum binaryFormat = 0; 

		cacheMicrocode->seek(0);

		// get size of binary
		cacheMicrocode->read(&binaryFormat, sizeof(GLenum));

#if GL_OES_get_program_binary
		// load binary
		glProgramBinaryOES( mGLHandle, 
							binaryFormat, 
							cacheMicrocode->getPtr(),
							binaryLength
			);
#endif
		GLint   success = 0;
		glGetProgramiv(mGLHandle, GL_LINK_STATUS, &success);
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

        glGetError(); // Clean up the error. Otherwise will flood log.
        mGLHandle = glCreateProgram();
        GL_CHECK_ERROR
	
		// compile and attach Vertex Program
		if (!mVertexProgram->getGLSLProgram()->compile(true))
		{
			// todo error
			return;
		}
        mVertexProgram->getGLSLProgram()->attachToProgramObject(mGLHandle);
        setSkeletalAnimationIncluded(mVertexProgram->isSkeletalAnimationIncluded());
        
		// compile and attach Fragment Program
		if (!mFragmentProgram->getGLSLProgram()->compile(true))
		{
			// todo error
			return;
		}		
        mFragmentProgram->getGLSLProgram()->attachToProgramObject(mGLHandle);

		// the link
		glLinkProgram( mGLHandle );
		GL_CHECK_ERROR
			glGetProgramiv( mGLHandle, GL_LINK_STATUS, &mLinked );
		GL_CHECK_ERROR
	
		mTriedToLinkAndFailed = !mLinked;

		logObjectInfo( getCombinedName() + String("GLSL link result : "), mGLHandle );
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
#endif

				// turns out we need this param when loading
				GLenum binaryFormat = 0; 

				// get the buffer
				GpuProgramManager::Microcode newMicrocode(OGRE_NEW MemoryDataStream(name,  binaryLength + sizeof(GLenum)));
				newMicrocode->seek(0);

#if GL_OES_get_program_binary
				// get binary
				glGetProgramBinaryOES(mGLHandle, binaryLength, NULL, (GLenum *)newMicrocode->getPtr(), newMicrocode->getPtr() + sizeof(GLenum));
#endif

				GpuProgramManager::getSingleton().addMicrocodeToCache(name, newMicrocode);
			}
		}
	}
	//-----------------------------------------------------------------------
	void GLSLESLinkProgram::extractAttributes(void)
	{
		size_t numAttribs = NUMBER_OF_CUSTOM_ATTS;
	String customAttributesNames[] = {
		String("vertex"),
		String("blendWeights"),
		String("normal"),
		String("colour"),
		String("secondary_colour"),
		String("blendIndices"),
		String("tangent"),
		String("binormal"),
		String("uv0"),
		String("uv1"),
		String("uv2"),
		String("uv3"),
		String("uv4"),
		String("uv5"),
		String("uv6"),
		String("uv7"),
	};

		for (size_t i = 0; i < numAttribs; ++i)
		{
			const String& attName = customAttributesNames[i];
			GLint attrib = glGetAttribLocation(mGLHandle, attName.c_str());
            GL_CHECK_ERROR;

			// seems that the word "position" is also used to define the position 
			if(i == 0 && attrib == -1)
			{
				attrib = glGetAttribLocation(mGLHandle, "position");
				GL_CHECK_ERROR;
			}
			
			if (attrib != -1)
			{
				mCustomAttributesIndexs[i] = attrib;
			}
		}
	}
	//-----------------------------------------------------------------------
	GLuint getFixedAttributeIndex(VertexElementSemantic semantic, uint index)
	{
		// Some drivers (e.g. OS X on nvidia) incorrectly determine the attribute binding automatically
		// and end up aliasing existing built-ins. So avoid! Fixed builtins are: 

		//  a  builtin				custom attrib name
		// ----------------------------------------------
		//	0  gl_Vertex			vertex
		//  1  n/a					blendWeights		
		//	2  gl_Normal			normal
		//	3  gl_Color				colour
		//	4  gl_SecondaryColor	secondary_colour
		//	5  gl_FogCoord			fog_coord
		//  7  n/a					blendIndices
		//  8 tangent
		//  9 binormal
		//	10  gl_MultiTexCoord0	uv0
		//	11  gl_MultiTexCoord1	uv1
		//	12 gl_MultiTexCoord2	uv2
		//	13 gl_MultiTexCoord3	uv3
		//	14 gl_MultiTexCoord4	uv4
		//	15 gl_MultiTexCoord5	uv5
		//	16 gl_MultiTexCoord6	uv6
		//	17 gl_MultiTexCoord7	uv7
		switch(semantic)
		{
			case VES_POSITION:
				return 0;
			case VES_BLEND_WEIGHTS:
				return 1;
			case VES_NORMAL:
				return 2;
			case VES_DIFFUSE:
				return 3;
			case VES_SPECULAR:
				return 4;
			case VES_BLEND_INDICES:
				return 5;
			case VES_TANGENT:
				return 6;
			case VES_BINORMAL:
				return 7;
			case VES_TEXTURE_COORDINATES:
				return 8 + index;
			default:
				assert(false && "Missing attribute!");
				return 0;
		};

	}
	//-----------------------------------------------------------------------
	GLuint GLSLESLinkProgram::getAttributeIndex(VertexElementSemantic semantic, uint index)
	{

		return mCustomAttributesIndexs[getFixedAttributeIndex(semantic, index)];
	}
	//-----------------------------------------------------------------------
	bool GLSLESLinkProgram::isAttributeValid(VertexElementSemantic semantic, uint index)
	{
		return getAttributeIndex(semantic, index) != -1;
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
