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
	//	8  gl_MultiTexCoord0	uv0
	//	9  gl_MultiTexCoord1	uv1
	//	10 gl_MultiTexCoord2	uv2
	//	11 gl_MultiTexCoord3	uv3
	//	12 gl_MultiTexCoord4	uv4
	//	13 gl_MultiTexCoord5	uv5
	//	14 gl_MultiTexCoord6	uv6, tangent
	//	15 gl_MultiTexCoord7	uv7, binormal
	GLSLESLinkProgram::CustomAttribute GLSLESLinkProgram::msCustomAttributes[] = {
		CustomAttribute("vertex", GLES2GpuProgram::getFixedAttributeIndex(VES_POSITION, 0)),
		CustomAttribute("blendWeights", GLES2GpuProgram::getFixedAttributeIndex(VES_BLEND_WEIGHTS, 0)),
		CustomAttribute("normal", GLES2GpuProgram::getFixedAttributeIndex(VES_NORMAL, 0)),
		CustomAttribute("colour", GLES2GpuProgram::getFixedAttributeIndex(VES_DIFFUSE, 0)),
		CustomAttribute("secondary_colour", GLES2GpuProgram::getFixedAttributeIndex(VES_SPECULAR, 0)),
		CustomAttribute("blendIndices", GLES2GpuProgram::getFixedAttributeIndex(VES_BLEND_INDICES, 0)),
		CustomAttribute("uv0", GLES2GpuProgram::getFixedAttributeIndex(VES_TEXTURE_COORDINATES, 0)),
		CustomAttribute("uv1", GLES2GpuProgram::getFixedAttributeIndex(VES_TEXTURE_COORDINATES, 1)),
		CustomAttribute("uv2", GLES2GpuProgram::getFixedAttributeIndex(VES_TEXTURE_COORDINATES, 2)),
		CustomAttribute("uv3", GLES2GpuProgram::getFixedAttributeIndex(VES_TEXTURE_COORDINATES, 3)),
		CustomAttribute("uv4", GLES2GpuProgram::getFixedAttributeIndex(VES_TEXTURE_COORDINATES, 4)),
		CustomAttribute("uv5", GLES2GpuProgram::getFixedAttributeIndex(VES_TEXTURE_COORDINATES, 5)),
		CustomAttribute("uv6", GLES2GpuProgram::getFixedAttributeIndex(VES_TEXTURE_COORDINATES, 6)),
		CustomAttribute("uv7", GLES2GpuProgram::getFixedAttributeIndex(VES_TEXTURE_COORDINATES, 7)),
		CustomAttribute("tangent", GLES2GpuProgram::getFixedAttributeIndex(VES_TANGENT, 0)),
		CustomAttribute("binormal", GLES2GpuProgram::getFixedAttributeIndex(VES_BINORMAL, 0)),
	};

	//-----------------------------------------------------------------------
	GLSLESLinkProgram::GLSLESLinkProgram(GLSLESGpuProgram* vertexProgram, GLSLESGpuProgram* fragmentProgram)
        : mVertexProgram(vertexProgram)
		, mFragmentProgram(fragmentProgram)
		, mUniformRefsBuilt(false)
        , mLinked(false)
		, mTriedToLinkAndFailed(false)
	{
		// init CustomAttributesIndexs
		mCustomAttributesIndexs.resize(GLES2GpuProgram::getFixedAttributeIndexCount());
		mCustomAttributesNames.resize(GLES2GpuProgram::getFixedAttributeIndexCount());

		for(size_t i = 0 ; i < GLES2GpuProgram::getFixedAttributeIndexCount(); i++)
		{
			mCustomAttributesIndexs[i] = -1;
		}

        glGetError(); // Clean up the error. Otherwise will flood log.
        mGLHandle = glCreateProgram();
        GL_CHECK_ERROR

        // Tell shaders to attach themselves to the LinkProgram
        // let the shaders do the attaching since they may have several children to attach
        if (mVertexProgram)
        {
            mVertexProgram->getGLSLProgram()->attachToProgramObject(mGLHandle);
            setSkeletalAnimationIncluded(mVertexProgram->isSkeletalAnimationIncluded());
        }
        
        if (mFragmentProgram)
        {
            mFragmentProgram->getGLSLProgram()->attachToProgramObject(mGLHandle);
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
				link();
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
		GLint binaryLength = 0;

		// turns out we need this param when loading
		GLenum binaryFormat = 0; 

		cacheMicrocode->seek(0);

		// get size of binary
		cacheMicrocode->read(&binaryLength, sizeof(GLint));
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
			link();
		}
		else
		{
			// jump binary
			cacheMicrocode->seek(sizeof(GLint) + sizeof(GLenum) + binaryLength);

			// load custom attributes
			for(size_t i = 0 ; i < GLES2GpuProgram::getFixedAttributeIndexCount(); i++)
			{
				String & name = mCustomAttributesNames[i];
				// read name length
				size_t lengthOfName = 0;
				cacheMicrocode->read(&lengthOfName, sizeof(size_t));
				name.resize(lengthOfName);

				// read name
				cacheMicrocode->read(&name[0], lengthOfName);

				// read index
				GLuint & index = mCustomAttributesIndexs[i];
				cacheMicrocode->read(&index, sizeof(GLuint));
			}

		}

	}

	//-----------------------------------------------------------------------
	void GLSLESLinkProgram::link()
	{

		size_t sizeOfCustomAttributesAsBuffer = bindAttribLocation();

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
				GpuProgramManager::Microcode newMicrocode(OGRE_NEW MemoryDataStream(name,  binaryLength + sizeof(GLenum) + sizeOfCustomAttributesAsBuffer));
				newMicrocode->seek(0);

				// write size of binary
				newMicrocode->write(&binaryLength, sizeof(GLint));

#if GL_OES_get_program_binary
				// get binary
				glGetProgramBinaryOES(mGLHandle, binaryLength, NULL, (GLenum *)newMicrocode->getPtr(), newMicrocode->getPtr() + sizeof(GLenum));
#endif
				newMicrocode->seek(sizeof(GLint) + sizeof(GLenum) +binaryLength);

				// save custom attributes
				for(size_t i = 0 ; i < GLES2GpuProgram::getFixedAttributeIndexCount(); i++)
				{
					const String & name = mCustomAttributesNames[i];
					// write name length
					size_t lengthOfName = name.size();
					newMicrocode->write(&lengthOfName, sizeof(size_t));

					// write name
					newMicrocode->write(&name[0], lengthOfName);

					// write index
					GLuint index = mCustomAttributesIndexs[i];
					newMicrocode->write(&index, sizeof(GLuint));
				}


				GpuProgramManager::getSingleton().addMicrocodeToCache(name, newMicrocode);
			}
		}
	}
	//-----------------------------------------------------------------------
	void GLSLESLinkProgram::extractAttributes(void)
	{
		size_t numAttribs = sizeof(msCustomAttributes)/sizeof(CustomAttribute);

		for (size_t i = 0; i < numAttribs; ++i)
		{
			const CustomAttribute& a = msCustomAttributes[i];
			GLint attrib = glGetAttribLocation(mGLHandle, a.name.c_str());
            GL_CHECK_ERROR;

			// seems that the word "position" is also used to define the position 
			if(i == 0 && attrib == -1)
			{
				attrib = glGetAttribLocation(mGLHandle, "position");
				GL_CHECK_ERROR;
			}
			
			if (attrib != -1)
			{
				mCustomAttributesIndexs[a.attrib] = attrib;
				mValidAttributes.insert(attrib);
			}
		}
	}
	//-----------------------------------------------------------------------
	GLuint GLSLESLinkProgram::getAttributeIndex(VertexElementSemantic semantic, uint index)
	{
		return mCustomAttributesIndexs[GLES2GpuProgram::getFixedAttributeIndex(semantic, index)];
	}
	//-----------------------------------------------------------------------
	bool GLSLESLinkProgram::isAttributeValid(VertexElementSemantic semantic, uint index)
	{
		return mValidAttributes.find(getAttributeIndex(semantic, index)) != mValidAttributes.end();
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
	//-----------------------------------------------------------------------
	size_t GLSLESLinkProgram::bindAttribLocation()
	{

		size_t sizeOfCustomAttributesAsBuffer = 0;
		size_t indexCount = 0;
		size_t numAttribs = sizeof(msCustomAttributes)/sizeof(CustomAttribute);
		const String& vpSource = mVertexProgram->getGLSLProgram()->getSource();

		map<String, const CustomAttribute *>::type customAttributesMap;
		for (size_t i = 0; i < numAttribs; ++i)
		{
			const CustomAttribute& a = msCustomAttributes[i];
			customAttributesMap[a.name] = &a;
		}

		// seems that the word "position" is also used to define the position
		customAttributesMap["position"] = &msCustomAttributes[0];

		// We're looking for either: 
		//   attribute vec<n> <semantic_name>
		// The latter is recommended in GLSL 1.3 onwards 
		// be slightly flexible about formatting

		// get the source lines
		StringVector sourceLines = StringUtil::split(vpSource, "\n");

		StringVector sourceWords;		
		// delete one line remarks
		for (size_t i = 0; i < sourceLines.size(); ++i)
		{
			String & curLine = sourceLines[i];
			String::size_type pos = vpSource.find("//");
			if (pos != String::npos)
			{
				curLine.erase(pos, curLine.size() - pos);
			}
			StringVector lineWords = StringUtil::split(curLine, " \r\t\n;");
			for (size_t j = 0; j < lineWords.size(); ++j)
			{
				sourceWords.push_back(lineWords[j]);
			}
		}

		bool insideMultilineRemark = false;
		for (size_t i = 0; i < sourceWords.size(); ++i)
		{
			String & curWord = sourceWords[i];
			// one char are too short for us here
			if (curWord.size() < 2)
				continue;
			if(curWord.substr(0, 2) == "*/")
				insideMultilineRemark = false;

			if(curWord.substr(0, 2) == "/*")
				insideMultilineRemark = true;

			if (insideMultilineRemark)
				continue;

			if (curWord == "attribute")
			{
				if( i + 2 < sourceWords.size())
				{
					String & attName = sourceWords[i+2];
					String lowercaseAttName = attName;
					StringUtil::toLowerCase(lowercaseAttName);
					if (customAttributesMap.find(lowercaseAttName) != customAttributesMap.end())
					{
						const CustomAttribute& a = *customAttributesMap.find(attName)->second;
						mCustomAttributesIndexs[a.attrib] = indexCount;
						mCustomAttributesNames[a.attrib] = attName;
						sizeOfCustomAttributesAsBuffer += sizeof(int) +  sizeof(size_t) + attName.length();

						glBindAttribLocation(mGLHandle, indexCount, attName.c_str());
						indexCount++;
						GL_CHECK_ERROR;
					}
				}
			}
		}
		return sizeOfCustomAttributesAsBuffer;
	}
} // namespace Ogre
