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

#include "OgreGLSLESLinkProgramManager.h"
#include "OgreGLSLESGpuProgram.h"
#include "OgreLogManager.h"
#include "OgreStringVector.h"
#include "OgreStringConverter.h"

namespace Ogre {

	//-----------------------------------------------------------------------
	template<> GLSLESLinkProgramManager* Singleton<GLSLESLinkProgramManager>::ms_Singleton = 0;

	//-----------------------------------------------------------------------
    GLSLESLinkProgramManager* GLSLESLinkProgramManager::getSingletonPtr(void)
    {
        return ms_Singleton;
    }

	//-----------------------------------------------------------------------
    GLSLESLinkProgramManager& GLSLESLinkProgramManager::getSingleton(void)
    {  
        assert( ms_Singleton );  return ( *ms_Singleton );  
    }

	//-----------------------------------------------------------------------
	GLSLESLinkProgramManager::GLSLESLinkProgramManager(void) : mActiveVertexGpuProgram(NULL),
		mActiveFragmentGpuProgram(NULL), mActiveLinkProgram(NULL)
	{
		// Fill in the relationship between type names and enums
		mTypeEnumMap.insert(StringToEnumMap::value_type("float", GL_FLOAT));
		mTypeEnumMap.insert(StringToEnumMap::value_type("vec2", GL_FLOAT_VEC2));
		mTypeEnumMap.insert(StringToEnumMap::value_type("vec3", GL_FLOAT_VEC3));
		mTypeEnumMap.insert(StringToEnumMap::value_type("vec4", GL_FLOAT_VEC4));
		mTypeEnumMap.insert(StringToEnumMap::value_type("sampler2D", GL_SAMPLER_2D));
		mTypeEnumMap.insert(StringToEnumMap::value_type("samplerCube", GL_SAMPLER_CUBE));
		mTypeEnumMap.insert(StringToEnumMap::value_type("int", GL_INT));
		mTypeEnumMap.insert(StringToEnumMap::value_type("ivec2", GL_INT_VEC2));
		mTypeEnumMap.insert(StringToEnumMap::value_type("ivec3", GL_INT_VEC3));
		mTypeEnumMap.insert(StringToEnumMap::value_type("ivec4", GL_INT_VEC4));
		mTypeEnumMap.insert(StringToEnumMap::value_type("mat2", GL_FLOAT_MAT2));
		mTypeEnumMap.insert(StringToEnumMap::value_type("mat3", GL_FLOAT_MAT3));
		mTypeEnumMap.insert(StringToEnumMap::value_type("mat4", GL_FLOAT_MAT4));
	}

	//-----------------------------------------------------------------------
	GLSLESLinkProgramManager::~GLSLESLinkProgramManager(void)
	{
		// iterate through map container and delete link programs
		for (LinkProgramIterator currentProgram = mLinkPrograms.begin();
			currentProgram != mLinkPrograms.end(); ++currentProgram)
		{
			OGRE_DELETE currentProgram->second;
		}

	}

	//-----------------------------------------------------------------------
	GLSLESLinkProgram* GLSLESLinkProgramManager::getActiveLinkProgram(void)
	{
		// if there is an active link program then return it
		if (mActiveLinkProgram)
			return mActiveLinkProgram;

		// no active link program so find one or make a new one
		// is there an active key?
		uint64 activeKey = 0;

		if (mActiveVertexGpuProgram)
		{
			activeKey = static_cast<uint64>(mActiveVertexGpuProgram->getProgramID()) << 32;
		}
		if (mActiveFragmentGpuProgram)
		{
			activeKey += static_cast<uint64>(mActiveFragmentGpuProgram->getProgramID());
		}

		// only return a link program object if a vertex or fragment program exist
		if (activeKey > 0)
		{
			// find the key in the hash map
			LinkProgramIterator programFound = mLinkPrograms.find(activeKey);
			// program object not found for key so need to create it
			if (programFound == mLinkPrograms.end())
			{
				//mActiveLinkProgram = OGRE_NEW GLSLESLinkProgram(mActiveVertexGpuProgram,mActiveFragmentGpuProgram);
				mActiveLinkProgram = new GLSLESLinkProgram(mActiveVertexGpuProgram,mActiveFragmentGpuProgram);
				mLinkPrograms[activeKey] = mActiveLinkProgram;
			}
			else
			{
				// found a link program in map container so make it active
				mActiveLinkProgram = programFound->second;
			}

		}
		// make the program object active
		if (mActiveLinkProgram) mActiveLinkProgram->activate();

		return mActiveLinkProgram;

	}

	//-----------------------------------------------------------------------
	void GLSLESLinkProgramManager::setActiveFragmentShader(GLSLESGpuProgram* fragmentGpuProgram)
	{
		if (fragmentGpuProgram != mActiveFragmentGpuProgram)
		{
			mActiveFragmentGpuProgram = fragmentGpuProgram;
			// ActiveLinkProgram is no longer valid
			mActiveLinkProgram = NULL;
			// Change back to fixed pipeline
			glUseProgram(0);
            GL_CHECK_ERROR;
		}
	}

	//-----------------------------------------------------------------------
	void GLSLESLinkProgramManager::setActiveVertexShader(GLSLESGpuProgram* vertexGpuProgram)
	{
		if (vertexGpuProgram != mActiveVertexGpuProgram)
		{
			mActiveVertexGpuProgram = vertexGpuProgram;
			// ActiveLinkProgram is no longer valid
			mActiveLinkProgram = NULL;
			// Change back to fixed pipeline
			glUseProgram(0);
            GL_CHECK_ERROR;
		}
	}
	//---------------------------------------------------------------------
	void GLSLESLinkProgramManager::completeDefInfo(GLenum gltype, 
		GpuConstantDefinition& defToUpdate)
	{
		// Decode uniform size and type
		// Note GLSL ES never packs rows into float4's(from an API perspective anyway)
		// therefore all values are tight in the buffer
		switch (gltype)
		{
		case GL_FLOAT:
			defToUpdate.constType = GCT_FLOAT1;
			break;
		case GL_FLOAT_VEC2:
			defToUpdate.constType = GCT_FLOAT2;
			break;
		case GL_FLOAT_VEC3:
			defToUpdate.constType = GCT_FLOAT3;
			break;
		case GL_FLOAT_VEC4:
			defToUpdate.constType = GCT_FLOAT4;
			break;
		case GL_SAMPLER_2D:
			defToUpdate.constType = GCT_SAMPLER2D;
			break;
		case GL_SAMPLER_CUBE:
			defToUpdate.constType = GCT_SAMPLERCUBE;
			break;
		case GL_INT:
			defToUpdate.constType = GCT_INT1;
			break;
		case GL_INT_VEC2:
			defToUpdate.constType = GCT_INT2;
			break;
		case GL_INT_VEC3:
			defToUpdate.constType = GCT_INT3;
			break;
		case GL_INT_VEC4:
			defToUpdate.constType = GCT_INT4;
			break;
		case GL_FLOAT_MAT2:
			defToUpdate.constType = GCT_MATRIX_2X2;
			break;
		case GL_FLOAT_MAT3:
			defToUpdate.constType = GCT_MATRIX_3X3;
			break;
		case GL_FLOAT_MAT4:
			defToUpdate.constType = GCT_MATRIX_4X4;
			break;
		default:
			defToUpdate.constType = GCT_UNKNOWN;
			break;
		}

		// GL doesn't pad
		defToUpdate.elementSize = GpuConstantDefinition::getElementSize(defToUpdate.constType, false);
	}

	//---------------------------------------------------------------------
	bool GLSLESLinkProgramManager::completeParamSource(
		const String& paramName,
		const GpuConstantDefinitionMap* vertexConstantDefs, 
		const GpuConstantDefinitionMap* fragmentConstantDefs,
		GLUniformReference& refToUpdate)
	{
		if (vertexConstantDefs)
		{
			GpuConstantDefinitionMap::const_iterator parami = 
				vertexConstantDefs->find(paramName);
			if (parami != vertexConstantDefs->end())
			{
				refToUpdate.mSourceProgType = GPT_VERTEX_PROGRAM;
				refToUpdate.mConstantDef = &(parami->second);
				return true;
			}

		}
		if (fragmentConstantDefs)
		{
			GpuConstantDefinitionMap::const_iterator parami = 
				fragmentConstantDefs->find(paramName);
			if (parami != fragmentConstantDefs->end())
			{
				refToUpdate.mSourceProgType = GPT_FRAGMENT_PROGRAM;
				refToUpdate.mConstantDef = &(parami->second);
				return true;
			}
		}
		return false;


	}
	//---------------------------------------------------------------------
	void GLSLESLinkProgramManager::extractUniforms(GLuint programObject, 
		const GpuConstantDefinitionMap* vertexConstantDefs, 
		const GpuConstantDefinitionMap* fragmentConstantDefs,
		GLUniformReferenceList& list)
	{
		// scan through the active uniforms and add them to the reference list
		GLint uniformCount;

		#define BUFFERSIZE 200
		char   uniformName[BUFFERSIZE];
		//GLint location;
		GLUniformReference newGLUniformReference;

		// get the number of active uniforms
		glGetProgramiv(programObject, GL_ACTIVE_UNIFORMS, &uniformCount);
        GL_CHECK_ERROR;

		// Loop over each of the active uniforms, and add them to the reference container
		// only do this for user defined uniforms, ignore built in gl state uniforms
		for (int index = 0; index < uniformCount; index++)
		{
			GLint arraySize;
			GLenum glType;
			glGetActiveUniform(programObject, index, BUFFERSIZE, NULL, 
				&arraySize, &glType, uniformName);
            GL_CHECK_ERROR;
			// Don't add built in uniforms
			newGLUniformReference.mLocation = glGetUniformLocation(programObject, uniformName);
			if (newGLUniformReference.mLocation >= 0)
			{
				// user defined uniform found, add it to the reference list
				String paramName = String( uniformName );

				// currant ATI drivers (Catalyst 7.2 and earlier) and older NVidia drivers will include all array elements as uniforms but we only want the root array name and location
				// Also note that ATI Catalyst 6.8 to 7.2 there is a bug with glUniform that does not allow you to update a uniform array past the first uniform array element
				// ie you can't start updating an array starting at element 1, must always be element 0.

				// if the uniform name has a "[" in it then its an array element uniform.
				String::size_type arrayStart = paramName.find("[");
				if (arrayStart != String::npos)
				{
					// if not the first array element then skip it and continue to the next uniform
					if (paramName.compare(arrayStart, paramName.size() - 1, "[0]") != 0) continue;
					paramName = paramName.substr(0, arrayStart);
				}

				// find out which params object this comes from
				bool foundSource = completeParamSource(paramName,
						vertexConstantDefs,	fragmentConstantDefs, newGLUniformReference);

				// only add this parameter if we found the source
				if (foundSource)
				{
					assert(size_t (arraySize) == newGLUniformReference.mConstantDef->arraySize
							&& "GL doesn't agree with our array size!");
					list.push_back(newGLUniformReference);
				}

				// Don't bother adding individual array params, they will be
				// picked up in the 'parent' parameter can copied all at once
				// anyway, individual indexes are only needed for lookup from
				// user params
			} // end if
		} // end for

	}
	//---------------------------------------------------------------------
	void GLSLESLinkProgramManager::extractConstantDefs(const String& src,
		GpuNamedConstants& defs, const String& filename)
	{
		// Parse the output string and collect all uniforms
		// NOTE this relies on the source already having been preprocessed
		// which is done in GLSLESProgram::loadFromSource
		String line;
		String::size_type currPos = src.find("uniform");
		while (currPos != String::npos)
		{
			GpuConstantDefinition def;
			String paramName;

			// Now check for using the word 'uniform' in a larger string & ignore
			bool inLargerString = false;
			if (currPos != 0)
			{
				char prev = src.at(currPos - 1);
				if (prev != ' ' && prev != '\t' && prev != '\r' && prev != '\n'
					&& prev != ';')
					inLargerString = true;
			}
			if (!inLargerString && currPos + 7 < src.size())
			{
				char next = src.at(currPos + 7);
				if (next != ' ' && next != '\t' && next != '\r' && next != '\n')
					inLargerString = true;
			}

			// skip 'uniform'
			currPos += 7;

			if (!inLargerString)
			{
				// find terminating semicolon
				String::size_type endPos = src.find(";", currPos);
				if (endPos == String::npos)
				{
					// problem, missing semicolon, abort
					break;
				}
				line = src.substr(currPos, endPos - currPos);

				// Remove spaces before opening square braces, otherwise
				// the following split() can split the line at inappropiate
				// places (e.g. "vec3 something [3]" won't work).
				for (String::size_type sqp = line.find (" ["); sqp != String::npos;
					 sqp = line.find (" ["))
					line.erase (sqp, 1);
				// Split into tokens
				StringVector parts = StringUtil::split(line, ", \t\r\n");

				for (StringVector::iterator i = parts.begin(); i != parts.end(); ++i)
				{
					// Is this a type?
					StringToEnumMap::iterator typei = mTypeEnumMap.find(*i);
					if (typei != mTypeEnumMap.end())
					{
						completeDefInfo(typei->second, def);
					}
					else
					{
						// if this is not a type, and not empty, it should be a name
						StringUtil::trim(*i);
						if (i->empty()) continue;

						String::size_type arrayStart = i->find("[", 0);
						if (arrayStart != String::npos)
						{
							// potential name (if butted up to array)
							String name = i->substr(0, arrayStart);
							StringUtil::trim(name);
							if (!name.empty())
								paramName = name;

							String::size_type arrayEnd = i->find("]", arrayStart);
							String arrayDimTerm = i->substr(arrayStart + 1, arrayEnd - arrayStart - 1);
							StringUtil::trim(arrayDimTerm);
							// the array term might be a simple number or it might be
							// an expression (e.g. 24*3) or refer to a constant expression
							// we'd have to evaluate the expression which could get nasty
							// TODO
							def.arraySize = StringConverter::parseInt(arrayDimTerm);

						}
						else
						{
							paramName = *i;
							def.arraySize = 1;
						}

						// Name should be after the type, so complete def and add
						// We do this now so that comma-separated params will do
						// this part once for each name mentioned 
						if (def.constType == GCT_UNKNOWN)
						{
							LogManager::getSingleton().logMessage(
								"Problem parsing the following GLSL Uniform: '"
								+ line + "' in file " + filename);
							// next uniform
							break;
						}

						// Complete def and add
						// increment physical buffer location
						def.logicalIndex = 0; // not valid in GLSL
						if (def.isFloat())
						{
							def.physicalIndex = defs.floatBufferSize;
							defs.floatBufferSize += def.arraySize * def.elementSize;
						}
						else
						{
							def.physicalIndex = defs.intBufferSize;
							defs.intBufferSize += def.arraySize * def.elementSize;
						}
						defs.map.insert(GpuConstantDefinitionMap::value_type(paramName, def));

						// Generate array accessors
						defs.generateConstantDefinitionArrayEntries(paramName, def);
					}

				}

			} // not commented or a larger symbol

			// Find next one
			currPos = src.find("uniform", currPos);

		}
		
	}

}
