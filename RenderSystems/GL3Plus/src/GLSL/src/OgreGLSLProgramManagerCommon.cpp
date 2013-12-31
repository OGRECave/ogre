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

#include "OgreGLSLProgramManagerCommon.h"
#include "OgreGLSLGpuProgram.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreGLSLProgram.h"
#include "OgreGpuProgramManager.h"
#include "OgreHardwareBufferManager.h"
#include "OgreRoot.h"

namespace Ogre {

	//-----------------------------------------------------------------------
	GLSLProgramManagerCommon::GLSLProgramManagerCommon(void) :
        mActiveVertexGpuProgram(NULL),
        mActiveGeometryGpuProgram(NULL),
        mActiveFragmentGpuProgram(NULL),
        mActiveHullGpuProgram(NULL),
        mActiveDomainGpuProgram(NULL),
        mActiveComputeGpuProgram(NULL)
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
		mTypeEnumMap.insert(StringToEnumMap::value_type("bvec2", GL_BOOL_VEC2));
		mTypeEnumMap.insert(StringToEnumMap::value_type("bvec3", GL_BOOL_VEC3));
		mTypeEnumMap.insert(StringToEnumMap::value_type("bvec4", GL_BOOL_VEC4));
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

        // GL 3.0
        mTypeEnumMap.insert(StringToEnumMap::value_type("uint", GL_UNSIGNED_INT));
		mTypeEnumMap.insert(StringToEnumMap::value_type("uvec2", GL_UNSIGNED_INT_VEC2));
		mTypeEnumMap.insert(StringToEnumMap::value_type("uvec3", GL_UNSIGNED_INT_VEC3));
		mTypeEnumMap.insert(StringToEnumMap::value_type("uvec4", GL_UNSIGNED_INT_VEC4));
		mTypeEnumMap.insert(StringToEnumMap::value_type("samplerCubeShadow", GL_SAMPLER_CUBE_SHADOW));
		mTypeEnumMap.insert(StringToEnumMap::value_type("sampler1DArray", GL_SAMPLER_1D_ARRAY));
		mTypeEnumMap.insert(StringToEnumMap::value_type("sampler2DArray", GL_SAMPLER_2D_ARRAY));
		mTypeEnumMap.insert(StringToEnumMap::value_type("sampler1DArrayShadow", GL_SAMPLER_1D_ARRAY_SHADOW));
		mTypeEnumMap.insert(StringToEnumMap::value_type("sampler2DArrayShadow", GL_SAMPLER_2D_ARRAY_SHADOW));
		mTypeEnumMap.insert(StringToEnumMap::value_type("isampler1D", GL_INT_SAMPLER_1D));
		mTypeEnumMap.insert(StringToEnumMap::value_type("isampler2D", GL_INT_SAMPLER_2D));
		mTypeEnumMap.insert(StringToEnumMap::value_type("isampler3D", GL_INT_SAMPLER_3D));
		mTypeEnumMap.insert(StringToEnumMap::value_type("isamplerCube", GL_INT_SAMPLER_CUBE));
		mTypeEnumMap.insert(StringToEnumMap::value_type("isampler1DArray", GL_INT_SAMPLER_1D_ARRAY));
		mTypeEnumMap.insert(StringToEnumMap::value_type("isampler2DArray", GL_INT_SAMPLER_2D_ARRAY));
		mTypeEnumMap.insert(StringToEnumMap::value_type("usampler1D", GL_UNSIGNED_INT_SAMPLER_1D));
		mTypeEnumMap.insert(StringToEnumMap::value_type("usampler2D", GL_UNSIGNED_INT_SAMPLER_2D));
		mTypeEnumMap.insert(StringToEnumMap::value_type("usampler3D", GL_UNSIGNED_INT_SAMPLER_3D));
		mTypeEnumMap.insert(StringToEnumMap::value_type("usamplerCube", GL_UNSIGNED_INT_SAMPLER_CUBE));
		mTypeEnumMap.insert(StringToEnumMap::value_type("usampler1DArray", GL_UNSIGNED_INT_SAMPLER_1D_ARRAY));
		mTypeEnumMap.insert(StringToEnumMap::value_type("usampler2DArray", GL_UNSIGNED_INT_SAMPLER_2D_ARRAY));

        // GL 3.1
		mTypeEnumMap.insert(StringToEnumMap::value_type("sampler2DRect", GL_SAMPLER_2D_RECT));
		mTypeEnumMap.insert(StringToEnumMap::value_type("sampler2DRectShadow", GL_SAMPLER_2D_RECT_SHADOW));
		mTypeEnumMap.insert(StringToEnumMap::value_type("isampler2DRect", GL_INT_SAMPLER_2D_RECT));
		mTypeEnumMap.insert(StringToEnumMap::value_type("usampler2DRect", GL_UNSIGNED_INT_SAMPLER_2D_RECT));
		mTypeEnumMap.insert(StringToEnumMap::value_type("samplerBuffer", GL_SAMPLER_BUFFER));
		mTypeEnumMap.insert(StringToEnumMap::value_type("isamplerBuffer", GL_INT_SAMPLER_BUFFER));
		mTypeEnumMap.insert(StringToEnumMap::value_type("usamplerBuffer", GL_UNSIGNED_INT_SAMPLER_BUFFER));

        // GL 3.2
		mTypeEnumMap.insert(StringToEnumMap::value_type("sampler2DMS", GL_SAMPLER_2D_MULTISAMPLE));
		mTypeEnumMap.insert(StringToEnumMap::value_type("isampler2DMS", GL_INT_SAMPLER_2D_MULTISAMPLE));
		mTypeEnumMap.insert(StringToEnumMap::value_type("usampler2DMS", GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE));
		mTypeEnumMap.insert(StringToEnumMap::value_type("sampler2DMSArray", GL_SAMPLER_2D_MULTISAMPLE_ARRAY));
		mTypeEnumMap.insert(StringToEnumMap::value_type("isampler2DMSArray", GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY));
		mTypeEnumMap.insert(StringToEnumMap::value_type("usampler2DMSArray", GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY));

        // GL 4.0
		mTypeEnumMap.insert(StringToEnumMap::value_type("double", GL_DOUBLE));
		mTypeEnumMap.insert(StringToEnumMap::value_type("dmat2", GL_DOUBLE_MAT2));
		mTypeEnumMap.insert(StringToEnumMap::value_type("dmat3", GL_DOUBLE_MAT3));
		mTypeEnumMap.insert(StringToEnumMap::value_type("dmat4", GL_DOUBLE_MAT4));
		mTypeEnumMap.insert(StringToEnumMap::value_type("dmat2x2", GL_DOUBLE_MAT2));
		mTypeEnumMap.insert(StringToEnumMap::value_type("dmat3x3", GL_DOUBLE_MAT3));
		mTypeEnumMap.insert(StringToEnumMap::value_type("dmat4x4", GL_DOUBLE_MAT4));
		mTypeEnumMap.insert(StringToEnumMap::value_type("dmat2x3", GL_DOUBLE_MAT2x3));
		mTypeEnumMap.insert(StringToEnumMap::value_type("dmat3x2", GL_DOUBLE_MAT3x2));
		mTypeEnumMap.insert(StringToEnumMap::value_type("dmat3x4", GL_DOUBLE_MAT3x4));
		mTypeEnumMap.insert(StringToEnumMap::value_type("dmat4x3", GL_DOUBLE_MAT4x3));
		mTypeEnumMap.insert(StringToEnumMap::value_type("dmat2x4", GL_DOUBLE_MAT2x4));
		mTypeEnumMap.insert(StringToEnumMap::value_type("dmat4x2", GL_DOUBLE_MAT4x2));
		mTypeEnumMap.insert(StringToEnumMap::value_type("dvec2", GL_DOUBLE_VEC2));
		mTypeEnumMap.insert(StringToEnumMap::value_type("dvec3", GL_DOUBLE_VEC3));
		mTypeEnumMap.insert(StringToEnumMap::value_type("dvec4", GL_DOUBLE_VEC4));
		mTypeEnumMap.insert(StringToEnumMap::value_type("samplerCubeArray", GL_SAMPLER_CUBE_MAP_ARRAY));
		mTypeEnumMap.insert(StringToEnumMap::value_type("samplerCubeArrayShadow", GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW));
		mTypeEnumMap.insert(StringToEnumMap::value_type("isamplerCubeArray", GL_INT_SAMPLER_CUBE_MAP_ARRAY));
		mTypeEnumMap.insert(StringToEnumMap::value_type("usamplerCubeArray", GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY));

		mTypeEnumMap.insert(StringToEnumMap::value_type("image1D", GL_IMAGE_1D));
		mTypeEnumMap.insert(StringToEnumMap::value_type("iimage1D", GL_INT_IMAGE_1D));
		mTypeEnumMap.insert(StringToEnumMap::value_type("uimage1D", GL_UNSIGNED_INT_IMAGE_1D));
		mTypeEnumMap.insert(StringToEnumMap::value_type("image2D", GL_IMAGE_2D));
		mTypeEnumMap.insert(StringToEnumMap::value_type("iimage2D", GL_INT_IMAGE_2D));
		mTypeEnumMap.insert(StringToEnumMap::value_type("uimage2D", GL_UNSIGNED_INT_IMAGE_2D));
		mTypeEnumMap.insert(StringToEnumMap::value_type("image3D", GL_IMAGE_3D));
		mTypeEnumMap.insert(StringToEnumMap::value_type("iimage3D", GL_INT_IMAGE_3D));
		mTypeEnumMap.insert(StringToEnumMap::value_type("uimage3D", GL_UNSIGNED_INT_IMAGE_3D));
		mTypeEnumMap.insert(StringToEnumMap::value_type("image2DRect", GL_IMAGE_2D_RECT));
		mTypeEnumMap.insert(StringToEnumMap::value_type("iimage2DRect", GL_INT_IMAGE_2D_RECT));
		mTypeEnumMap.insert(StringToEnumMap::value_type("uimage2DRect", GL_UNSIGNED_INT_IMAGE_2D_RECT));
		mTypeEnumMap.insert(StringToEnumMap::value_type("imageCube", GL_IMAGE_CUBE));
		mTypeEnumMap.insert(StringToEnumMap::value_type("iimageCube", GL_INT_IMAGE_CUBE));
		mTypeEnumMap.insert(StringToEnumMap::value_type("uimageCube", GL_UNSIGNED_INT_IMAGE_CUBE));
		mTypeEnumMap.insert(StringToEnumMap::value_type("imageBuffer", GL_IMAGE_BUFFER));
		mTypeEnumMap.insert(StringToEnumMap::value_type("iimageBuffer", GL_INT_IMAGE_BUFFER));
		mTypeEnumMap.insert(StringToEnumMap::value_type("uimageBuffer", GL_UNSIGNED_INT_IMAGE_BUFFER));
		mTypeEnumMap.insert(StringToEnumMap::value_type("image1DArray", GL_IMAGE_1D_ARRAY));
		mTypeEnumMap.insert(StringToEnumMap::value_type("iimage1DArray", GL_INT_IMAGE_1D_ARRAY));
		mTypeEnumMap.insert(StringToEnumMap::value_type("uimage1DArray", GL_UNSIGNED_INT_IMAGE_1D_ARRAY));
		mTypeEnumMap.insert(StringToEnumMap::value_type("image2DArray", GL_IMAGE_2D_ARRAY));
		mTypeEnumMap.insert(StringToEnumMap::value_type("iimage2DArray", GL_INT_IMAGE_2D_ARRAY));
		mTypeEnumMap.insert(StringToEnumMap::value_type("uimage2DArray", GL_UNSIGNED_INT_IMAGE_2D_ARRAY));
		mTypeEnumMap.insert(StringToEnumMap::value_type("imageCubeArray", GL_IMAGE_CUBE_MAP_ARRAY));
		mTypeEnumMap.insert(StringToEnumMap::value_type("iimageCubeArray", GL_INT_IMAGE_CUBE_MAP_ARRAY));
		mTypeEnumMap.insert(StringToEnumMap::value_type("uimageCubeArray", GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY));
		mTypeEnumMap.insert(StringToEnumMap::value_type("image2DMS", GL_IMAGE_2D_MULTISAMPLE));
		mTypeEnumMap.insert(StringToEnumMap::value_type("iimage2DMS", GL_INT_IMAGE_2D_MULTISAMPLE));
		mTypeEnumMap.insert(StringToEnumMap::value_type("uimage2DMS", GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE));
		mTypeEnumMap.insert(StringToEnumMap::value_type("image2DMSArray", GL_IMAGE_2D_MULTISAMPLE_ARRAY));
		mTypeEnumMap.insert(StringToEnumMap::value_type("iimage2DMSArray", GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY));
		mTypeEnumMap.insert(StringToEnumMap::value_type("uimage2DMSArray", GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY));
	}

	//-----------------------------------------------------------------------
	GLSLProgramManagerCommon::~GLSLProgramManagerCommon(void)
	{
	}
	//---------------------------------------------------------------------
	void GLSLProgramManagerCommon::completeDefInfo(GLenum gltype, 
		GpuConstantDefinition& defToUpdate)
	{
		// Decode uniform size and type
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
            case GL_SAMPLER_1D_ARRAY:
            case GL_INT_SAMPLER_1D:
            case GL_INT_SAMPLER_1D_ARRAY:
            case GL_UNSIGNED_INT_SAMPLER_1D:
            case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
                // need to record samplers for GLSL
                defToUpdate.constType = GCT_SAMPLER1D;
                break;
            case GL_SAMPLER_2D:
            case GL_SAMPLER_2D_RECT:    // TODO: Move these to a new type??
            case GL_INT_SAMPLER_2D_RECT:
            case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
            case GL_SAMPLER_2D_ARRAY:
            case GL_INT_SAMPLER_2D:
            case GL_INT_SAMPLER_2D_ARRAY:
            case GL_UNSIGNED_INT_SAMPLER_2D:
            case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
                defToUpdate.constType = GCT_SAMPLER2D;
                break;
            case GL_SAMPLER_3D:
            case GL_INT_SAMPLER_3D:
            case GL_UNSIGNED_INT_SAMPLER_3D:
                defToUpdate.constType = GCT_SAMPLER3D;
                break;
            case GL_SAMPLER_CUBE:
            case GL_SAMPLER_CUBE_SHADOW:
            case GL_INT_SAMPLER_CUBE:
            case GL_UNSIGNED_INT_SAMPLER_CUBE:
                defToUpdate.constType = GCT_SAMPLERCUBE;
                break;
            case GL_SAMPLER_1D_SHADOW:
            case GL_SAMPLER_1D_ARRAY_SHADOW:
                defToUpdate.constType = GCT_SAMPLER1DSHADOW;
                break;
            case GL_SAMPLER_2D_SHADOW:
            case GL_SAMPLER_2D_RECT_SHADOW:
            case GL_SAMPLER_2D_ARRAY_SHADOW:
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
            case GL_DOUBLE:
                defToUpdate.constType = GCT_DOUBLE1;
                break;
            case GL_DOUBLE_VEC2:
                defToUpdate.constType = GCT_DOUBLE2;
                break;
            case GL_DOUBLE_VEC3:
                defToUpdate.constType = GCT_DOUBLE3;
                break;
            case GL_DOUBLE_VEC4:
                defToUpdate.constType = GCT_DOUBLE4;
                break;
            case GL_DOUBLE_MAT2:
                defToUpdate.constType = GCT_MATRIX_DOUBLE_2X2;
                break;
            case GL_DOUBLE_MAT3:
                defToUpdate.constType = GCT_MATRIX_DOUBLE_3X3;
                break;
            case GL_DOUBLE_MAT4:
                defToUpdate.constType = GCT_MATRIX_DOUBLE_4X4;
                break;
            case GL_DOUBLE_MAT2x3:
                defToUpdate.constType = GCT_MATRIX_DOUBLE_2X3;
                break;
            case GL_DOUBLE_MAT3x2:
                defToUpdate.constType = GCT_MATRIX_DOUBLE_3X2;
                break;
            case GL_DOUBLE_MAT2x4:
                defToUpdate.constType = GCT_MATRIX_DOUBLE_2X4;
                break;
            case GL_DOUBLE_MAT4x2:
                defToUpdate.constType = GCT_MATRIX_DOUBLE_4X2;
                break;
            case GL_DOUBLE_MAT3x4:
                defToUpdate.constType = GCT_MATRIX_DOUBLE_3X4;
                break;
            case GL_DOUBLE_MAT4x3:
                defToUpdate.constType = GCT_MATRIX_DOUBLE_4X3;
                break;
            default:
                defToUpdate.constType = GCT_UNKNOWN;
                break;
		}

		// GL doesn't pad
		defToUpdate.elementSize = GpuConstantDefinition::getElementSize(defToUpdate.constType, false);
	}

	//---------------------------------------------------------------------
	bool GLSLProgramManagerCommon::completeParamSource(
		const String& paramName,
        const GpuConstantDefinitionMap* vertexConstantDefs,
        const GpuConstantDefinitionMap* geometryConstantDefs,
        const GpuConstantDefinitionMap* fragmentConstantDefs,
        const GpuConstantDefinitionMap* hullConstantDefs,
        const GpuConstantDefinitionMap* domainConstantDefs,
        const GpuConstantDefinitionMap* computeConstantDefs,
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
		if (hullConstantDefs)
		{
			GpuConstantDefinitionMap::const_iterator parami =
            hullConstantDefs->find(paramName);
			if (parami != hullConstantDefs->end())
			{
				refToUpdate.mSourceProgType = GPT_HULL_PROGRAM;
				refToUpdate.mConstantDef = &(parami->second);
				return true;
			}
		}
		if (domainConstantDefs)
		{
			GpuConstantDefinitionMap::const_iterator parami =
            domainConstantDefs->find(paramName);
			if (parami != domainConstantDefs->end())
			{
				refToUpdate.mSourceProgType = GPT_DOMAIN_PROGRAM;
				refToUpdate.mConstantDef = &(parami->second);
				return true;
			}
		}
		if (computeConstantDefs)
		{
			GpuConstantDefinitionMap::const_iterator parami =
            computeConstantDefs->find(paramName);
			if (parami != computeConstantDefs->end())
			{
				refToUpdate.mSourceProgType = GPT_COMPUTE_PROGRAM;
				refToUpdate.mConstantDef = &(parami->second);
				return true;
			}
		}
		return false;
	}

	//---------------------------------------------------------------------
	void GLSLProgramManagerCommon::extractUniforms(GLuint programObject, 
        const GpuConstantDefinitionMap* vertexConstantDefs,
        const GpuConstantDefinitionMap* geometryConstantDefs,
        const GpuConstantDefinitionMap* fragmentConstantDefs,
        const GpuConstantDefinitionMap* hullConstantDefs,
        const GpuConstantDefinitionMap* domainConstantDefs,
        const GpuConstantDefinitionMap* computeConstantDefs,
		GLUniformReferenceList& list, GLUniformBufferList& sharedList)
	{
		// Scan through the active uniforms and add them to the reference list
		GLint uniformCount = 0;
#define uniformLength 200
        //		GLint uniformLength = 0;
        //        glGetProgramiv(programObject, GL_ACTIVE_UNIFORM_MAX_LENGTH, &uniformLength);

		char uniformName[uniformLength];
		GLUniformReference newGLUniformReference;

		// Get the number of active uniforms
        OGRE_CHECK_GL_ERROR(glGetProgramiv(programObject, GL_ACTIVE_UNIFORMS, &uniformCount));

		// Loop over each of the active uniforms, and add them to the reference container
		// only do this for user defined uniforms, ignore built in gl state uniforms
		for (int index = 0; index < uniformCount; index++)
		{
			GLint arraySize;
			GLenum glType;
			OGRE_CHECK_GL_ERROR(glGetActiveUniform(programObject, index, uniformLength, NULL,
                                                   &arraySize, &glType, uniformName));

			// Don't add built in uniforms
			OGRE_CHECK_GL_ERROR(newGLUniformReference.mLocation = glGetUniformLocation(programObject, uniformName));
			if (newGLUniformReference.mLocation >= 0)
			{
				// User defined uniform found, add it to the reference list
				String paramName = String( uniformName );

				// Current ATI drivers (Catalyst 7.2 and earlier) and older NVidia drivers will include all array elements as uniforms but we only want the root array name and location
				// Also note that ATI Catalyst 6.8 to 7.2 there is a bug with glUniform that does not allow you to update a uniform array past the first uniform array element
				// ie you can't start updating an array starting at element 1, must always be element 0.

				// If the uniform name has a "[" in it then its an array element uniform.
				String::size_type arrayStart = paramName.find("[");
				if (arrayStart != String::npos)
				{
					// if not the first array element then skip it and continue to the next uniform
					if (paramName.compare(arrayStart, paramName.size() - 1, "[0]") != 0) continue;
					paramName = paramName.substr(0, arrayStart);
				}

				// Find out which params object this comes from
				bool foundSource = completeParamSource(paramName,
                                                       vertexConstantDefs, geometryConstantDefs,
                                                       fragmentConstantDefs, hullConstantDefs,
                                                       domainConstantDefs, computeConstantDefs, newGLUniformReference);

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

        // Now deal with uniform blocks

        GLint blockCount = 0;

        OGRE_CHECK_GL_ERROR(glGetProgramiv(programObject, GL_ACTIVE_UNIFORM_BLOCKS, &blockCount));

        for (int index = 0; index < blockCount; index++)
        {
            OGRE_CHECK_GL_ERROR(glGetActiveUniformBlockName(programObject, index, uniformLength, NULL, uniformName));

            GpuSharedParametersPtr blockSharedParams = GpuProgramManager::getSingleton().getSharedParameters(uniformName);

            GLint blockSize, blockBinding;
            OGRE_CHECK_GL_ERROR(glGetActiveUniformBlockiv(programObject, index, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize));
            OGRE_CHECK_GL_ERROR(glGetActiveUniformBlockiv(programObject, index, GL_UNIFORM_BLOCK_BINDING, &blockBinding));
            HardwareUniformBufferSharedPtr newUniformBuffer = HardwareBufferManager::getSingleton().createUniformBuffer(blockSize, HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE, false, uniformName);

            GL3PlusHardwareUniformBuffer* hwGlBuffer = static_cast<GL3PlusHardwareUniformBuffer*>(newUniformBuffer.get());
            hwGlBuffer->setGLBufferBinding(blockBinding);
            sharedList.push_back(newUniformBuffer);
        }
	}
	//---------------------------------------------------------------------
	void GLSLProgramManagerCommon::extractConstantDefs(const String& src,
		GpuNamedConstants& defs, const String& filename)
	{
		// Parse the output string and collect all uniforms
		// NOTE this relies on the source already having been preprocessed
		// which is done in GLSLProgram::loadFromSource
		String line;
		String::size_type currPos = src.find("uniform");
		while (currPos != String::npos)
		{
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
                String::size_type endPos;
                GpuSharedParametersPtr blockSharedParams;

                // Check for a type. If there is one, then the semicolon is missing
                // otherwise treat as if it is a uniform block
				String::size_type lineEndPos = src.find_first_of("\n\r", currPos);
				line = src.substr(currPos, lineEndPos - currPos);
                StringVector parts = StringUtil::split(line, " \t");
                StringToEnumMap::iterator typei = mTypeEnumMap.find(parts.front());
                if (typei == mTypeEnumMap.end())
                {
                    // Gobble up the external name
                    String externalName = parts.front();

                    // Now there should be an opening brace
                    String::size_type openBracePos = src.find("{", currPos);
                    if (openBracePos != String::npos)
                    {
                        currPos = openBracePos + 1;
                    }
                    else
                    {
                        LogManager::getSingleton().logMessage("Missing opening brace in GLSL Uniform Block in file "
                                                              + filename, LML_CRITICAL);
                        break;
                    }

                    // First we need to find the internal name for the uniform block
                    String::size_type endBracePos = src.find("}", currPos);

                    // Find terminating semicolon
                    currPos = endBracePos + 1;
                    endPos = src.find(";", currPos);
                    if (endPos == String::npos)
                    {
                        // problem, missing semicolon, abort
                        break;
                    }

                    // TODO: We don't need the internal name. Just skip over to the end of the block
                    // But we do need to know if this is an array of blocks. Is that legal?

// Find the internal name.
// This can be an array.
//                    line = src.substr(currPos, endPos - currPos);
//                    StringVector internalParts = StringUtil::split(line, ", \t\r\n");
//                    String internalName = "";
//                    uint16 arraySize = 0;
//                    for (StringVector::iterator i = internalParts.begin(); i != internalParts.end(); ++i)
//                    {
//                        StringUtil::trim(*i);
//                        String::size_type arrayStart = i->find("[", 0);
//                        if (arrayStart != String::npos)
//                        {
//                            // potential name (if butted up to array)
//                            String name = i->substr(0, arrayStart);
//                            StringUtil::trim(name);
//                            if (!name.empty())
//                                internalName = name;
//
//                            String::size_type arrayEnd = i->find("]", arrayStart);
//                            String arrayDimTerm = i->substr(arrayStart + 1, arrayEnd - arrayStart - 1);
//                            StringUtil::trim(arrayDimTerm);
//                            arraySize = StringConverter::parseUnsignedInt(arrayDimTerm);
//                        }
//                        else
//                        {
//                            internalName = *i;
//                        }
//                    }
//
//                    // Ok, now rewind and parse the individual uniforms in this block
//                    currPos = openBracePos + 1;
//                    blockSharedParams = GpuProgramManager::getSingleton().getSharedParameters(externalName);
//                    if(blockSharedParams.isNull())
//                        blockSharedParams = GpuProgramManager::getSingleton().createSharedParameters(externalName);
//                    do
//                    {
//                        lineEndPos = src.find_first_of("\n\r", currPos);
//                        endPos = src.find(";", currPos);
//                        line = src.substr(currPos, endPos - currPos);
//
//                        // TODO: Give some sort of block id
//                        // Parse the normally structured uniform
//                        parseIndividualConstant(src, defs, currPos, filename, blockSharedParams);
//                        currPos = lineEndPos + 1;
//                    } while (endBracePos > currPos);
                }
                else
                {
                    // find terminating semicolon
                    endPos = src.find(";", currPos);
                    if (endPos == String::npos)
                    {
                        // problem, missing semicolon, abort
                        break;
                    }
                    
                    parseIndividualConstant(src, defs, currPos, filename, blockSharedParams);
                }
                line = src.substr(currPos, endPos - currPos);
			} // not commented or a larger symbol
            
			// Find next one
			currPos = src.find("uniform", currPos);
		}
	}

    //---------------------------------------------------------------------
	void GLSLProgramManagerCommon::parseIndividualConstant(const String& src, GpuNamedConstants& defs,
                                                             String::size_type currPos,
                                                             const String& filename, GpuSharedParametersPtr sharedParams)
    {
        GpuConstantDefinition def;
        String paramName = "";
        String::size_type endPos = src.find(";", currPos);
        String line = src.substr(currPos, endPos - currPos);

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
                    LogManager::getSingleton().logMessage("Problem parsing the following GLSL Uniform: '"
                                                          + line + "' in file " + filename, LML_CRITICAL);
                    // next uniform
                    break;
                }

                // Special handling for shared parameters
                if(sharedParams.isNull())
                {
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
                else
                {
                    try
                    {
                        const GpuConstantDefinition &sharedDef = sharedParams->getConstantDefinition(paramName);
                        (void)sharedDef;    // Silence warning
                    }
                    catch (Exception& e)
                    {
                        // This constant doesn't exist so we'll create a new one
                        sharedParams->addConstantDefinition(paramName, def.constType);
                    }
                }
            }
        }
    }
}
