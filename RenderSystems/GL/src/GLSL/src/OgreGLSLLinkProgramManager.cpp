/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/

#include "OgreGLSLLinkProgramManager.h"
#include "OgreGLSLGpuProgram.h"
#include "OgreGLSLProgram.h"
#include "OgreStringConverter.h"
#include "OgreLogManager.h"

namespace Ogre {

	//-----------------------------------------------------------------------
	template<> GLSLLinkProgramManager* Singleton<GLSLLinkProgramManager>::ms_Singleton = 0;

	//-----------------------------------------------------------------------
    GLSLLinkProgramManager* GLSLLinkProgramManager::getSingletonPtr(void)
    {
        return ms_Singleton;
    }

	//-----------------------------------------------------------------------
    GLSLLinkProgramManager& GLSLLinkProgramManager::getSingleton(void)
    {  
        assert( ms_Singleton );  return ( *ms_Singleton );  
    }

	//-----------------------------------------------------------------------
	GLSLLinkProgramManager::GLSLLinkProgramManager(void) : mActiveVertexGpuProgram(NULL),
		mActiveGeometryGpuProgram(NULL), mActiveFragmentGpuProgram(NULL), mActiveLinkProgram(NULL)
	{
		// Fill in the relationship between type names and enums
		mTypeEnumMap.insert(StringToEnumMap::value_type("float", GL_FLOAT));
		mTypeEnumMap.insert(StringToEnumMap::value_type("vec2", GL_FLOAT_VEC2));
		mTypeEnumMap.insert(StringToEnumMap::value_type("vec3", GL_FLOAT_VEC3));
		mTypeEnumMap.insert(StringToEnumMap::value_type("vec4", GL_FLOAT_VEC4));
		mTypeEnumMap.insert(StringToEnumMap::value_type("sampler1D", GL_SAMPLER_1D));
		mTypeEnumMap.insert(StringToEnumMap::value_type("sampler2D", GL_SAMPLER_2D));
		mTypeEnumMap.insert(StringToEnumMap::value_type("sampler3D", GL_SAMPLER_3D));
		mTypeEnumMap.insert(StringToEnumMap::value_type("samplerCube", GL_SAMPLER_CUBE));
		mTypeEnumMap.insert(StringToEnumMap::value_type("sampler1DShadow", GL_SAMPLER_1D_SHADOW));
		mTypeEnumMap.insert(StringToEnumMap::value_type("sampler2DShadow", GL_SAMPLER_2D_SHADOW));
		mTypeEnumMap.insert(StringToEnumMap::value_type("int", GL_INT));
		mTypeEnumMap.insert(StringToEnumMap::value_type("ivec2", GL_INT_VEC2));
		mTypeEnumMap.insert(StringToEnumMap::value_type("ivec3", GL_INT_VEC3));
		mTypeEnumMap.insert(StringToEnumMap::value_type("ivec4", GL_INT_VEC4));
		mTypeEnumMap.insert(StringToEnumMap::value_type("mat2", GL_FLOAT_MAT2));
		mTypeEnumMap.insert(StringToEnumMap::value_type("mat3", GL_FLOAT_MAT3));
		mTypeEnumMap.insert(StringToEnumMap::value_type("mat4", GL_FLOAT_MAT4));
		// GL 2.1
		mTypeEnumMap.insert(StringToEnumMap::value_type("mat2x2", GL_FLOAT_MAT2));
		mTypeEnumMap.insert(StringToEnumMap::value_type("mat3x3", GL_FLOAT_MAT3));
		mTypeEnumMap.insert(StringToEnumMap::value_type("mat4x4", GL_FLOAT_MAT4));
		mTypeEnumMap.insert(StringToEnumMap::value_type("mat2x3", GL_FLOAT_MAT2x3));
		mTypeEnumMap.insert(StringToEnumMap::value_type("mat3x2", GL_FLOAT_MAT3x2));
		mTypeEnumMap.insert(StringToEnumMap::value_type("mat3x4", GL_FLOAT_MAT3x4));
		mTypeEnumMap.insert(StringToEnumMap::value_type("mat4x3", GL_FLOAT_MAT4x3));
		mTypeEnumMap.insert(StringToEnumMap::value_type("mat2x4", GL_FLOAT_MAT2x4));
		mTypeEnumMap.insert(StringToEnumMap::value_type("mat4x2", GL_FLOAT_MAT4x2));

	}

	//-----------------------------------------------------------------------
	GLSLLinkProgramManager::~GLSLLinkProgramManager(void)
	{
		// iterate through map container and delete link programs
		for (LinkProgramIterator currentProgram = mLinkPrograms.begin();
			currentProgram != mLinkPrograms.end(); ++currentProgram)
		{
			delete currentProgram->second;
		}

	}

	//-----------------------------------------------------------------------
	GLSLLinkProgram* GLSLLinkProgramManager::getActiveLinkProgram(void)
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
		if (mActiveGeometryGpuProgram)
		{
			activeKey += static_cast<uint64>(mActiveGeometryGpuProgram->getProgramID()) << 16;
		}
		if (mActiveFragmentGpuProgram)
		{
			activeKey += static_cast<uint64>(mActiveFragmentGpuProgram->getProgramID());
		}

		// only return a link program object if a vertex, geometry or fragment program exist
		if (activeKey > 0)
		{
			// find the key in the hash map
			LinkProgramIterator programFound = mLinkPrograms.find(activeKey);
			// program object not found for key so need to create it
			if (programFound == mLinkPrograms.end())
			{
				mActiveLinkProgram = new GLSLLinkProgram(mActiveVertexGpuProgram, mActiveGeometryGpuProgram,mActiveFragmentGpuProgram);
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
	void GLSLLinkProgramManager::setActiveFragmentShader(GLSLGpuProgram* fragmentGpuProgram)
	{
		if (fragmentGpuProgram != mActiveFragmentGpuProgram)
		{
			mActiveFragmentGpuProgram = fragmentGpuProgram;
			// ActiveLinkProgram is no longer valid
			mActiveLinkProgram = NULL;
			// change back to fixed pipeline
			glUseProgramObjectARB(0);
		}
	}

	//-----------------------------------------------------------------------
	void GLSLLinkProgramManager::setActiveVertexShader(GLSLGpuProgram* vertexGpuProgram)
	{
		if (vertexGpuProgram != mActiveVertexGpuProgram)
		{
			mActiveVertexGpuProgram = vertexGpuProgram;
			// ActiveLinkProgram is no longer valid
			mActiveLinkProgram = NULL;
			// change back to fixed pipeline
			glUseProgramObjectARB(0);
		}
	}
	//-----------------------------------------------------------------------
	void GLSLLinkProgramManager::setActiveGeometryShader(GLSLGpuProgram* geometryGpuProgram)
	{
		if (geometryGpuProgram != mActiveGeometryGpuProgram)
		{
			mActiveGeometryGpuProgram = geometryGpuProgram;
			// ActiveLinkProgram is no longer valid
			mActiveLinkProgram = NULL;
			// change back to fixed pipeline
			glUseProgramObjectARB(0);
		}
	}
	//---------------------------------------------------------------------
	void GLSLLinkProgramManager::completeDefInfo(GLenum gltype, 
		GpuConstantDefinition& defToUpdate)
	{
		// decode uniform size and type
		// Note GLSL never packs rows into float4's(from an API perspective anyway)
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
		case GL_SAMPLER_1D:
			// need to record samplers for GLSL
			defToUpdate.constType = GCT_SAMPLER1D;
			break;
		case GL_SAMPLER_2D:
		case GL_SAMPLER_2D_RECT_ARB:
			defToUpdate.constType = GCT_SAMPLER2D;
			break;
		case GL_SAMPLER_3D:
			defToUpdate.constType = GCT_SAMPLER3D;
			break;
		case GL_SAMPLER_CUBE:
			defToUpdate.constType = GCT_SAMPLERCUBE;
			break;
		case GL_SAMPLER_1D_SHADOW:
			defToUpdate.constType = GCT_SAMPLER1DSHADOW;
			break;
		case GL_SAMPLER_2D_SHADOW:
		case GL_SAMPLER_2D_RECT_SHADOW_ARB:
			defToUpdate.constType = GCT_SAMPLER2DSHADOW;
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
		case GL_FLOAT_MAT2x3:
			defToUpdate.constType = GCT_MATRIX_2X3;
			break;
		case GL_FLOAT_MAT3x2:
			defToUpdate.constType = GCT_MATRIX_3X2;
			break;
		case GL_FLOAT_MAT2x4:
			defToUpdate.constType = GCT_MATRIX_2X4;
			break;
		case GL_FLOAT_MAT4x2:
			defToUpdate.constType = GCT_MATRIX_4X2;
			break;
		case GL_FLOAT_MAT3x4:
			defToUpdate.constType = GCT_MATRIX_3X4;
			break;
		case GL_FLOAT_MAT4x3:
			defToUpdate.constType = GCT_MATRIX_4X3;
			break;
		default:
			defToUpdate.constType = GCT_UNKNOWN;
			break;

		}

		// GL doesn't pad
		defToUpdate.elementSize = GpuConstantDefinition::getElementSize(defToUpdate.constType, false);


	}
	//---------------------------------------------------------------------
	bool GLSLLinkProgramManager::completeParamSource(
		const String& paramName,
		const GpuConstantDefinitionMap* vertexConstantDefs, 
		const GpuConstantDefinitionMap* geometryConstantDefs,
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
		if (geometryConstantDefs)
		{
			GpuConstantDefinitionMap::const_iterator parami = 
				geometryConstantDefs->find(paramName);
			if (parami != geometryConstantDefs->end())
			{
				refToUpdate.mSourceProgType = GPT_GEOMETRY_PROGRAM;
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
	void GLSLLinkProgramManager::extractUniforms(GLhandleARB programObject, 
		const GpuConstantDefinitionMap* vertexConstantDefs, 
		const GpuConstantDefinitionMap* geometryConstantDefs,
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
		glGetObjectParameterivARB(programObject, GL_OBJECT_ACTIVE_UNIFORMS_ARB,
			&uniformCount);

		// Loop over each of the active uniforms, and add them to the reference container
		// only do this for user defined uniforms, ignore built in gl state uniforms
		for (int index = 0; index < uniformCount; index++)
		{
			GLint arraySize;
			GLenum glType;
			glGetActiveUniformARB(programObject, index, BUFFERSIZE, NULL, 
				&arraySize, &glType, uniformName);
			// don't add built in uniforms
			newGLUniformReference.mLocation = glGetUniformLocationARB(programObject, uniformName);
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
						vertexConstantDefs,	geometryConstantDefs, fragmentConstantDefs, newGLUniformReference);

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
	void GLSLLinkProgramManager::extractConstantDefs(const String& src,
		GpuNamedConstants& defs, const String& filename)
	{
		// Parse the output string and collect all uniforms
		// NOTE this relies on the source already having been preprocessed
		// which is done in GLSLProgram::loadFromSource
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
