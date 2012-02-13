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

#include "OgreGLSLESProgramManagerCommon.h"
#include "OgreGLSLESGpuProgram.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreGLSLESProgram.h"

// Apple doesn't define this in their extension.  We'll do it just for convenience.
// Using the value from desktop GL
#if GL_EXT_shadow_samplers
#define GL_SAMPLER_2D_SHADOW_EXT 0x8B62
#endif

namespace Ogre {

	//-----------------------------------------------------------------------
	GLSLESProgramManagerCommon::GLSLESProgramManagerCommon(void) : mActiveVertexGpuProgram(NULL),
		mActiveFragmentGpuProgram(NULL)
	{
		// Fill in the relationship between type names and enums
		mTypeEnumMap.insert(StringToEnumMap::value_type("float", GL_FLOAT));
		mTypeEnumMap.insert(StringToEnumMap::value_type("vec2", GL_FLOAT_VEC2));
		mTypeEnumMap.insert(StringToEnumMap::value_type("vec3", GL_FLOAT_VEC3));
		mTypeEnumMap.insert(StringToEnumMap::value_type("vec4", GL_FLOAT_VEC4));
		mTypeEnumMap.insert(StringToEnumMap::value_type("sampler2D", GL_SAMPLER_2D));
		mTypeEnumMap.insert(StringToEnumMap::value_type("samplerCube", GL_SAMPLER_CUBE));
#if GL_EXT_shadow_samplers
		mTypeEnumMap.insert(StringToEnumMap::value_type("sampler2DShadow", GL_SAMPLER_2D_SHADOW_EXT));
#endif
		mTypeEnumMap.insert(StringToEnumMap::value_type("int", GL_INT));
		mTypeEnumMap.insert(StringToEnumMap::value_type("ivec2", GL_INT_VEC2));
		mTypeEnumMap.insert(StringToEnumMap::value_type("ivec3", GL_INT_VEC3));
		mTypeEnumMap.insert(StringToEnumMap::value_type("ivec4", GL_INT_VEC4));
		mTypeEnumMap.insert(StringToEnumMap::value_type("mat2", GL_FLOAT_MAT2));
		mTypeEnumMap.insert(StringToEnumMap::value_type("mat3", GL_FLOAT_MAT3));
		mTypeEnumMap.insert(StringToEnumMap::value_type("mat4", GL_FLOAT_MAT4));
        
#if !OGRE_NO_GLES2_GLSL_OPTIMISER
        mGLSLOptimiserContext = glslopt_initialize(true);
#endif
	}

	//-----------------------------------------------------------------------
	GLSLESProgramManagerCommon::~GLSLESProgramManagerCommon(void)
	{
#if !OGRE_NO_GLES2_GLSL_OPTIMISER
        if(mGLSLOptimiserContext)
        {
            glslopt_cleanup(mGLSLOptimiserContext);
            mGLSLOptimiserContext = NULL;
        }
#endif
	}
	//---------------------------------------------------------------------
	void GLSLESProgramManagerCommon::completeDefInfo(GLenum gltype, 
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
#if GL_EXT_shadow_samplers
        case GL_SAMPLER_2D_SHADOW_EXT:
            defToUpdate.constType = GCT_SAMPLER2DSHADOW;
            break;
#endif
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
	bool GLSLESProgramManagerCommon::completeParamSource(
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

#if !OGRE_NO_GLES2_GLSL_OPTIMISER
    void GLSLESProgramManagerCommon::optimiseShaderSource(GLSLESGpuProgram* gpuProgram)
    {
        if(!gpuProgram->getGLSLProgram()->getIsOptimised())
        {
            GpuProgramType gpuType = gpuProgram->getType();
            const glslopt_shader_type shaderType = (gpuType == GPT_VERTEX_PROGRAM) ? kGlslOptShaderVertex : kGlslOptShaderFragment;
            String shaderSource = gpuProgram->getGLSLProgram()->getSource();
            glslopt_shader* shader = glslopt_optimize(mGLSLOptimiserContext, shaderType, shaderSource.c_str(), 0);

            std::stringstream os;
            if(glslopt_get_status(shader))
            {
                // Write the current version (this forces the driver to fulfill the glsl es standard)
                // TODO: Need to insert the current or compatibility version.  This is not future-proof
                os << "#version 100" << std::endl;
                
                // Default precision declaration is required in fragment and vertex shaders.
                os << "precision mediump float;" << std::endl;
                os << "precision highp int;" << std::endl;
                os << glslopt_get_output(shader);
                gpuProgram->getGLSLProgram()->setSource(os.str());
                gpuProgram->getGLSLProgram()->setIsOptimised(true);
            }
            else
            {
                LogManager::getSingleton().logMessage("Error from GLSL Optimiser, disabling optimisation for program: " + gpuProgram->getName());
                gpuProgram->getGLSLProgram()->setParameter("use_optimiser", "false");
//                LogManager::getSingleton().logMessage(String(glslopt_get_log(shader)));
//                LogManager::getSingleton().logMessage("Original Shader");
//                LogManager::getSingleton().logMessage(gpuProgram->getGLSLProgram()->getSource());
//                LogManager::getSingleton().logMessage("Optimized Shader");
//                LogManager::getSingleton().logMessage(os.str());
            }
            glslopt_shader_delete(shader);
        }
    }
#endif

	//---------------------------------------------------------------------
	void GLSLESProgramManagerCommon::extractUniforms(GLuint programObject, 
		const GpuConstantDefinitionMap* vertexConstantDefs, 
		const GpuConstantDefinitionMap* fragmentConstantDefs,
		GLUniformReferenceList& list)
	{
		// Scan through the active uniforms and add them to the reference list
		GLint uniformCount = 0;
        GLint maxLength = 0;
		char* uniformName = NULL;

		glGetProgramiv(programObject, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLength);
        GL_CHECK_ERROR;

        // If the max length of active uniforms is 0, then there are 0 active.
        // There won't be any to extract so we can return.
        if(maxLength == 0)
            return;

		uniformName = new char[maxLength + 1];
		GLUniformReference newGLUniformReference;

		// Get the number of active uniforms
		glGetProgramiv(programObject, GL_ACTIVE_UNIFORMS, &uniformCount);
        GL_CHECK_ERROR;

		// Loop over each of the active uniforms, and add them to the reference container
		// only do this for user defined uniforms, ignore built in gl state uniforms
		for (GLuint index = 0; index < (GLuint)uniformCount; index++)
		{
			GLint arraySize = 0;
			GLenum glType = GL_NONE;
			glGetActiveUniform(programObject, index, maxLength, NULL, 
				&arraySize, &glType, uniformName);
            GL_CHECK_ERROR;
			// Don't add built in uniforms
			newGLUniformReference.mLocation = glGetUniformLocation(programObject, uniformName);
			if (newGLUniformReference.mLocation >= 0)
			{
				// User defined uniform found, add it to the reference list
				String paramName = String( uniformName );

				// If the uniform name has a "[" in it then its an array element uniform.
				String::size_type arrayStart = paramName.find("[");
				if (arrayStart != String::npos)
				{
					// If not the first array element then skip it and continue to the next uniform
					if (paramName.compare(arrayStart, paramName.size() - 1, "[0]") != 0) continue;
					paramName = paramName.substr(0, arrayStart);
				}

				// Find out which params object this comes from
				bool foundSource = completeParamSource(paramName,
						vertexConstantDefs,	fragmentConstantDefs, newGLUniformReference);

				// Only add this parameter if we found the source
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
		
		if( uniformName != NULL ) 
		{
			delete uniformName;
		}
	}
	//---------------------------------------------------------------------
	void GLSLESProgramManagerCommon::extractConstantDefs(const String& src,
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
				// the following split() can split the line at inappropriate
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

                        // Skip over precision keywords
                        if(StringUtil::match((*i), "lowp") ||
                           StringUtil::match((*i), "mediump") ||
                           StringUtil::match((*i), "highp"))
                            continue;

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
							def.arraySize = StringConverter::parseUnsignedLong(arrayDimTerm);

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
