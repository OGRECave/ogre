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

#include "OgreGLSLProgramManager.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreGLSLShader.h"
#include "OgreGpuProgramManager.h"
#include "OgreGL3PlusHardwareBufferManager.h"
#include "OgreRoot.h"
#include "OgreGL3PlusUtil.h"

#include <iostream>

namespace Ogre {

    const size_t c_numTypeQualifiers = 8;

    const String c_typeQualifiers[c_numTypeQualifiers] =
    {
        "lowp",
        "mediump",
        "highp",
        "restrict",
        "coherent",
        "volatile",
        "readonly",
        "writeonly"
    };

    
    GLSLProgramManager::GLSLProgramManager( const GL3PlusSupport& support ) :
        mActiveVertexShader(NULL),
        mActiveHullShader(NULL),
        mActiveDomainShader(NULL),
        mActiveGeometryShader(NULL),
        mActiveFragmentShader(NULL),
        mActiveComputeShader(NULL),
        mGLSupport(support)
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
        mTypeEnumMap.insert(StringToEnumMap::value_type("bool", GL_BOOL));
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

        // GL 4.2
        mTypeEnumMap.insert(StringToEnumMap::value_type("atomic_uint", GL_UNSIGNED_INT_ATOMIC_COUNTER));
    }
    
    void GLSLProgramManager::convertGLUniformtoOgreType(GLenum gltype,
                                                        GpuConstantDefinition& defToUpdate)
    {
        // Note GLSL never packs rows into float4's (from an API perspective anyway)
        // therefore all values are tight in the buffer.
        //TODO Should the rest of the above enum types be included here?
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
        case GL_SAMPLER_2D_MULTISAMPLE:
        case GL_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
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
        case GL_SAMPLER_BUFFER:
        case GL_INT_SAMPLER_BUFFER:
        case GL_UNSIGNED_INT_SAMPLER_BUFFER:
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
        case GL_UNSIGNED_INT:
        case GL_UNSIGNED_INT_ATOMIC_COUNTER: //TODO should be its own type?
            defToUpdate.constType = GCT_UINT1;
            break;
        case GL_UNSIGNED_INT_VEC2:
            defToUpdate.constType = GCT_UINT2;
            break;
        case GL_UNSIGNED_INT_VEC3:
            defToUpdate.constType = GCT_UINT3;
            break;
        case GL_UNSIGNED_INT_VEC4:
            defToUpdate.constType = GCT_UINT4;
            break;
        case GL_BOOL:
            defToUpdate.constType = GCT_BOOL1;
            break;
        case GL_BOOL_VEC2:
            defToUpdate.constType = GCT_BOOL2;
            break;
        case GL_BOOL_VEC3:
            defToUpdate.constType = GCT_BOOL3;
            break;
        case GL_BOOL_VEC4:
            defToUpdate.constType = GCT_BOOL4;
            break;
        default:
            if( gltype >= GL_IMAGE_1D && gltype <= GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY )
                defToUpdate.constType = GCT_INT1;
            else
                defToUpdate.constType = GCT_UNKNOWN;
            break;
        }

        // GL doesn't pad
        defToUpdate.elementSize = GpuConstantDefinition::getElementSize(defToUpdate.constType, false);
    }

    
    bool GLSLProgramManager::findUniformDataSource(
        const String& paramName,
        const GpuConstantDefinitionMap* vertexConstantDefs,
        const GpuConstantDefinitionMap* hullConstantDefs,
        const GpuConstantDefinitionMap* domainConstantDefs,
        const GpuConstantDefinitionMap* geometryConstantDefs,
        const GpuConstantDefinitionMap* fragmentConstantDefs,
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

    
    //FIXME This is code bloat...either template or unify UniformReference
    // and AtomicCounterReference
    bool GLSLProgramManager::findAtomicCounterDataSource(
        const String& paramName,
        const GpuConstantDefinitionMap* vertexConstantDefs,
        const GpuConstantDefinitionMap* hullConstantDefs,
        const GpuConstantDefinitionMap* domainConstantDefs,
        const GpuConstantDefinitionMap* geometryConstantDefs,
        const GpuConstantDefinitionMap* fragmentConstantDefs,
        const GpuConstantDefinitionMap* computeConstantDefs,
        GLAtomicCounterReference& refToUpdate)
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


    
    void GLSLProgramManager::extractUniformsFromProgram(
        GLuint programObject,
        const GpuConstantDefinitionMap* vertexConstantDefs,
        const GpuConstantDefinitionMap* hullConstantDefs,
        const GpuConstantDefinitionMap* domainConstantDefs,
        const GpuConstantDefinitionMap* geometryConstantDefs,
        const GpuConstantDefinitionMap* fragmentConstantDefs,
        const GpuConstantDefinitionMap* computeConstantDefs,
        GLUniformReferenceList& uniformList,
        GLAtomicCounterReferenceList& counterList,
        GLUniformBufferList& uniformBufferList,
        SharedParamsBufferMap& sharedParamsBufferMap,
        // GLShaderStorageBufferList& shaderStorageBufferList,
        GLCounterBufferList& counterBufferList)
    {
        // Scan through the active uniforms and add them to the reference list.
        GLint uniformCount = 0;
#define uniformLength 200
        //              GLint uniformLength = 0;
        //        glGetProgramiv(programObject, GL_ACTIVE_UNIFORM_MAX_LENGTH, &uniformLength);

        char uniformName[uniformLength];
        GLUniformReference newGLUniformReference;
        GLAtomicCounterReference newGLAtomicCounterReference;

        // Get the number of active uniforms, including atomic
        // counters and uniforms contained in uniform blocks.
        OGRE_CHECK_GL_ERROR(glGetProgramiv(programObject, GL_ACTIVE_UNIFORMS, &uniformCount));

        // Loop over each active uniform and add it to the reference
        // container.
        for (GLuint index = 0; index < (GLuint)uniformCount; index++)
        {
            GLint arraySize;
            GLenum glType;
            OGRE_CHECK_GL_ERROR(glGetActiveUniform(programObject, index, uniformLength, NULL,
                                                   &arraySize, &glType, uniformName));

            // Don't add built in uniforms, atomic counters, or uniform block parameters.
            OGRE_CHECK_GL_ERROR(newGLUniformReference.mLocation = glGetUniformLocation(programObject, uniformName));
            if (newGLUniformReference.mLocation >= 0)
            {
                // User defined uniform found, add it to the reference list.
                String paramName = String(uniformName);

                // Current ATI drivers (Catalyst 7.2 and earlier) and
                // older NVidia drivers will include all array
                // elements as uniforms but we only want the root
                // array name and location. Also note that ATI Catalyst
                // 6.8 to 7.2 there is a bug with glUniform that does
                // not allow you to update a uniform array past the
                // first uniform array element ie you can't start
                // updating an array starting at element 1, must
                // always be element 0.

                // If the uniform name has a "[" in it then its an array element uniform.
                String::size_type arrayStart = paramName.find("[");
                if (arrayStart != String::npos)
                {
                    // if not the first array element then skip it and continue to the next uniform
                    if (paramName.compare(arrayStart, paramName.size() - 1, "[0]") != 0) continue;
                    paramName = paramName.substr(0, arrayStart);
                }

                // Find out which params object this comes from
                bool foundSource = findUniformDataSource(
                    paramName,
                    vertexConstantDefs, geometryConstantDefs,
                    fragmentConstantDefs, hullConstantDefs,
                    domainConstantDefs, computeConstantDefs,
                    newGLUniformReference);

                // Only add this parameter if we found the source
                if (foundSource)
                {
                    assert(size_t (arraySize) == newGLUniformReference.mConstantDef->arraySize
                           && "GL doesn't agree with our array size!");
                    uniformList.push_back(newGLUniformReference);
                }

                // Don't bother adding individual array params, they will be
                // picked up in the 'parent' parameter can copied all at once
                // anyway, individual indexes are only needed for lookup from
                // user params
            } // end if
            // Handle atomic counters. Currently atomic counters
            // cannot be in uniform blocks and are always unsigned
            // integers.
            else if (glType == GL_UNSIGNED_INT_ATOMIC_COUNTER)
            {
                String paramName = String(uniformName);

                GLint binding, offset;
                //GLuint indices [] = {index};
                OGRE_CHECK_GL_ERROR(glGetActiveUniformsiv(programObject, 1, &index, GL_UNIFORM_ATOMIC_COUNTER_BUFFER_INDEX, &binding));
                OGRE_CHECK_GL_ERROR(glGetActiveUniformsiv(programObject, 1, &index, GL_UNIFORM_OFFSET, &offset));

                newGLAtomicCounterReference.mBinding = binding;
                newGLAtomicCounterReference.mOffset = offset;

                // increment the total number of atomic counters
                // including size of array if applicable
                //atomicCounterCount += arraySize;
                // actually, this should not be necessary since
                // parameters are processed one by one

                // If the uniform name has a "[" in it then its an array element uniform.
                String::size_type arrayStart = paramName.find("[");
                if (arrayStart != String::npos)
                {
                    // if not the first array element then skip it and continue to the next uniform
                    if (paramName.compare(arrayStart, paramName.size() - 1, "[0]") != 0) continue;
                    paramName = paramName.substr(0, arrayStart);
                }

                std::cout << "ATOMIC COUNTER FOUND: " << paramName  << " " << arraySize << std::endl;

                // Find out which params object this comes from
                bool foundSource = findAtomicCounterDataSource(
                    paramName,
                    vertexConstantDefs, geometryConstantDefs,
                    fragmentConstantDefs, hullConstantDefs,
                    domainConstantDefs, computeConstantDefs,
                    newGLAtomicCounterReference);

                // Only add this parameter if we found the source
                if (foundSource)
                {
                    // size_t adjustedArraySize = 0;
                    // if (arraySize == 2 && newGLAtomicCounterReference.mConstantDef->arraySize == 1) {
                    //     adjustedArraySize = 1;
                    // }
                    // else {
                    //     adjustedArraySize = (size_t) arraySize;
                    // }

                    //FIXME On Linux AMD Catalyst 13.4, OpenGL reports
                    // a single atomic counter as having size 2.  Bug
                    // or feature?
                    // assert((size_t)arraySize == newGLAtomicCounterReference.mConstantDef->arraySize
                    //        && "GL doesn't agree with our array size!");

                    counterList.push_back(newGLAtomicCounterReference);
                    printf("ATOMIC COUNTER SOURCE FOUND\n");
                }
            }
        } // end for


        //FIXME uniform buffers need to be created during material script parsing of shared params
        // HardwareUniformBufferSharedPtr newUniformBuffer = HardwareBufferManager::getSingleton().createUniformBuffer(blockSize, HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE, false, uniformName);
        // GL3PlusHardwareUniformBuffer* hwGlBuffer = static_cast<GL3PlusHardwareUniformBuffer*>(newUniformBuffer.get());
        // hwGlBuffer->setGLBufferBinding(blockBinding);
        // GpuSharedParametersPtr blockSharedParams = GpuProgramManager::getSingleton().getSharedParameters(uniformName);
        // uniformBufferList.push_back(uniformBuffer);

        //FIXME Ogre materials need a new shared param that is associated with an entity.
        // This could be impemented as a switch-like statement inside shared_params:

        GLint blockCount = 0;

#if 0
        // Now deal with uniform blocks

        OGRE_CHECK_GL_ERROR(glGetProgramiv(programObject, GL_ACTIVE_UNIFORM_BLOCKS, &blockCount));

        for (int index = 0; index < blockCount; index++)
        {
            OGRE_CHECK_GL_ERROR(glGetActiveUniformBlockName(programObject, index, uniformLength, NULL, uniformName));

            // Map uniform block to binding point of GL buffer of
            // shared param bearing the same name.

            GpuSharedParametersPtr blockSharedParams;
            try {
                blockSharedParams = GpuProgramManager::getSingleton().createSharedParameters(uniformName);
            } catch (Exception& e) {
                blockSharedParams = GpuProgramManager::getSingleton().getSharedParameters(uniformName);
            }
            //TODO error handling for when buffer has no associated shared parameter?
            //if (bufferi == mSharedParamGLBufferMap.end()) continue;

            v1::GL3PlusHardwareUniformBuffer* hwGlBuffer;
            SharedParamsBufferMap::const_iterator bufferMapi = sharedParamsBufferMap.find(blockSharedParams);
            if (bufferMapi != sharedParamsBufferMap.end())
            {
                hwGlBuffer = static_cast<v1::GL3PlusHardwareUniformBuffer*>(bufferMapi->second.get());
            }
            else
            {
                // Create buffer and add entry to buffer map.
                GLint blockSize;
                OGRE_CHECK_GL_ERROR(glGetActiveUniformBlockiv(programObject, index, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize));
                v1::HardwareUniformBufferSharedPtr newUniformBuffer = v1::HardwareBufferManager::getSingleton().
                        createUniformBuffer(blockSize, v1::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE, false, uniformName);
                // bufferMapi->second() = newUniformBuffer;
                hwGlBuffer = static_cast<v1::GL3PlusHardwareUniformBuffer*>(newUniformBuffer.get());
                GLint bufferBinding = sharedParamsBufferMap.size();
                hwGlBuffer->setGLBufferBinding(bufferBinding);
                std::pair<GpuSharedParametersPtr, v1::HardwareUniformBufferSharedPtr> newPair (blockSharedParams, newUniformBuffer);
                sharedParamsBufferMap.insert(newPair);

                // Get active block parameter properties.
                //GpuNamedConstants namedParams = blockSharedParams->getNamed;
                //GpuConstantDefinitionIterator sharedParam = namedParams.map.begin();
                //GpuConstantDefinitionIterator endParam = namedParams.map.end();
                GpuConstantDefinitionIterator sharedParamDef = blockSharedParams->getConstantDefinitionIterator();
                std::vector<const char*> sharedParamNames;
                for (; sharedParamDef.current() != sharedParamDef.end(); sharedParamDef.moveNext())
                {
                    sharedParamNames.push_back(sharedParamDef.current()->first.c_str());
                }

                std::vector<GLuint> uniformParamIndices (sharedParamNames.size(), 0);
                std::vector<GLint> uniformParamOffsets (sharedParamNames.size(), -2);
                OGRE_CHECK_GL_ERROR(glGetUniformIndices(programObject, sharedParamNames.size(), &sharedParamNames[0], &uniformParamIndices[0]));
                //FIXME debug this (see stdout)
                OGRE_CHECK_GL_ERROR(glGetActiveUniformsiv(programObject, uniformParamIndices.size(), &uniformParamIndices[0], GL_UNIFORM_OFFSET, &uniformParamOffsets[0]));
                //TODO handle uniform arrays
                //GL_UNIFORM_ARRAY_STRIDE
                //TODO handle matrices
                //GL_UNIFORM_MATRIX_STRIDE

                //const GpuConstantDefinitionMap& sharedParamDefs = blockSharedParams->getConstantDefinitions().map;
                // sharedParamDef.begin();
                // for (int i = 0; sharedParamDef.current() != sharedParamDef.end(); sharedParamDef.moveNext(), ++i)
                // {
                //     sharedParamDef.current()->second.logicalIndex = uniformParamOffsets[i];
                // }
                //sharedParam->logicalIndex = ;

                hwGlBuffer->mBufferParamsLayout.indices = uniformParamIndices;
                hwGlBuffer->mBufferParamsLayout.offsets = uniformParamOffsets;
            }

            GLint bufferBinding = hwGlBuffer->getGLBufferBinding();

            //OGRE_CHECK_GL_ERROR(glGetActiveUniformBlockiv(programObject, index, GL_UNIFORM_BLOCK_BINDING, &blockBinding));

            OGRE_CHECK_GL_ERROR(glUniformBlockBinding(programObject, index, bufferBinding));
        }

        // Now deal with shader storage blocks

        //TODO Need easier, more robust feature checking.
        // if (mGLSupport.checkExtension("GL_ARB_program_interface_query") || mHasGL43)
        if (mGLSupport.hasMinGLVersion(4, 3))
        {
            OGRE_CHECK_GL_ERROR(glGetProgramInterfaceiv(programObject, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &blockCount));

            //TODO error if GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS > # shader_storage_blocks
            // do same for other buffers

            for (int index = 0; index < blockCount; index++)
            {
                //OGRE_CHECK_GL_ERROR(glGetProgramResourceiv(programObject, GL_SHADER_STORAGE_BLOCK, index, uniformName, ));
                // OGRE_CHECK_GL_ERROR(glGetIntegeri_v(GL_SHADER_STORAGE_BUFFER_BINDING, index, uniformName));
                OGRE_CHECK_GL_ERROR(glGetProgramResourceName(programObject, GL_SHADER_STORAGE_BLOCK, index, uniformLength, NULL, uniformName));

                // Map uniform block to binding point of GL buffer of
                // shared param bearing the same name.

                GpuSharedParametersPtr blockSharedParams = GpuProgramManager::getSingleton().getSharedParameters(uniformName);
                //TODO error handling for when buffer has no associated shared parameter?
                //if (bufferi == mSharedParamGLBufferMap.end()) continue;

                // No more shader storage blocks.
                // if (uniformName == 0) break;
                v1::GL3PlusHardwareShaderStorageBuffer* hwGlBuffer;
                SharedParamsBufferMap::const_iterator bufferMapi = sharedParamsBufferMap.find(blockSharedParams);
                if (bufferMapi != sharedParamsBufferMap.end())
                {
                    hwGlBuffer = static_cast<v1::GL3PlusHardwareShaderStorageBuffer*>(bufferMapi->second.get());
                }
                else
                {
                    // Create buffer and add entry to buffer map.
                    // OGRE_CHECK_GL_ERROR(glGetActiveUniformBlockiv(programObject, index, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize));
                    // HardwareUniformBufferSharedPtr newUniformBuffer = HardwareBufferManager::getSingleton().createUniformBuffer(blockSize, HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE, false, uniformName);
                    // bufferMapi->second() = newUniformBuffer;
                    // hwGlBuffer = static_cast<GL3PlusHardwareUniformBuffer*>(newUniformBuffer.get());
                    // GLint bufferBinding = sharedParamsBufferMap.size();
                    // hwGlBuffer->setGLBufferBinding(bufferBinding);
                    // std::pair<GpuSharedParametersPtr, HardwareUniformBufferSharedPtr> newPair (blockSharedParams, newUniformBuffer);
                    // sharedParamsBufferMap.insert(newPair);

                    GLint blockSize;
                    //GLint parameters [2];
                    // const GLenum properties [2] = {GL_BUFFER_DATA_SIZE, GL_BUFFER_BINDING};
                    const GLenum properties [1] = {GL_BUFFER_DATA_SIZE};
                    OGRE_CHECK_GL_ERROR(glGetProgramResourceiv(programObject, GL_SHADER_STORAGE_BLOCK, index, 1, properties, 1, NULL, &blockSize));
                    //blockSize = properties[0];
                    //TODO Implement shared param access param in materials (R, W, R+W)
                    // HardwareUniformBufferSharedPtr newShaderStorageBuffer = static_cast<GL3PlusHardwareBufferManager*>(HardwareBufferManager::getSingletonPtr())->createShaderStorageBuffer(blockSize, HardwareBuffer::HBU_DYNAMIC, false, uniformName);
                    v1::HardwareUniformBufferSharedPtr newShaderStorageBuffer = static_cast<v1::GL3PlusHardwareBufferManager*>(v1::HardwareBufferManager::getSingletonPtr())->createShaderStorageBuffer(blockSize, v1::HardwareBuffer::HBU_DYNAMIC, false, uniformName);
                    hwGlBuffer = static_cast<v1::GL3PlusHardwareShaderStorageBuffer*>(newShaderStorageBuffer.get());

                    // OGRE_CHECK_GL_ERROR(glGetActiveUniformBlockiv(programObject, index, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize));
                    // OGRE_CHECK_GL_ERROR(glGetActiveUniformBlockiv(programObject, index, GL_UNIFORM_BLOCK_BINDING, &blockBinding));
                    //FIXME check parameters

                    GLint bufferBinding = sharedParamsBufferMap.size();
                    hwGlBuffer->setGLBufferBinding(bufferBinding);

                    std::pair<GpuSharedParametersPtr, v1::HardwareUniformBufferSharedPtr> newPair (blockSharedParams, newShaderStorageBuffer);
                    //sharedParamsBufferMap.insert(newPair);

                    // Get active block parameter properties.
                }

                GLint bufferBinding = hwGlBuffer->getGLBufferBinding();

                OGRE_CHECK_GL_ERROR(glShaderStorageBlockBinding(programObject, index, bufferBinding));
            }
        }
#endif
        // if (mGLSupport.checkExtension("GL_ARB_shader_atomic_counters") || gl3wIsSupported(4, 2))
        if (mGLSupport.hasMinGLVersion(4, 2))
        {
            // Now deal with atomic counters buffers
            OGRE_CHECK_GL_ERROR(glGetProgramiv(programObject, GL_ACTIVE_ATOMIC_COUNTER_BUFFERS, &blockCount));

            for (int index = 0; index < blockCount; index++)
            {
                //TODO Is this necessary?
                //GpuSharedParametersPtr blockSharedParams = GpuProgramManager::getSingleton().getSharedParameters(uniformName);

                //TODO We could build list of atomic counters here or above,
                // whichever is most efficient.
                // GLint * active_indices;
                // OGRE_CHECK_GL_ERROR(glGetActiveAtomicCounterBufferiv(programObject, index, GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTER_INDICES, active_indices));

                GLint bufferSize, bufferBinding;
                OGRE_CHECK_GL_ERROR(glGetActiveAtomicCounterBufferiv(programObject, index, GL_ATOMIC_COUNTER_BUFFER_DATA_SIZE, &bufferSize));
                OGRE_CHECK_GL_ERROR(glGetActiveAtomicCounterBufferiv(programObject, index, GL_ATOMIC_COUNTER_BUFFER_BINDING, &bufferBinding));
                //TODO check parameters of this GL call
                v1::HardwareCounterBufferSharedPtr newCounterBuffer = v1::HardwareBufferManager::getSingleton().createCounterBuffer(bufferSize, v1::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE, false);

                v1::GL3PlusHardwareCounterBuffer* hwGlBuffer = static_cast<v1::GL3PlusHardwareCounterBuffer*>(newCounterBuffer.get());
                hwGlBuffer->setGLBufferBinding(bufferBinding);
                counterBufferList.push_back(newCounterBuffer);
            }
        }
    }
    
    void GLSLProgramManager::extractUniformsFromGLSL(
        const String& src, GpuNamedConstants& defs, const String& filename)
    {
        // Parse the output string and collect all uniforms
        // NOTE this relies on the source already having been preprocessed
        // which is done in GLSLShader::loadFromSource
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
                String typeString;
                GpuSharedParametersPtr blockSharedParams;

                // Check for a type. If there is one, then the
                // semicolon is missing.  Otherwise treat as if it is
                // a uniform block.
                String::size_type lineEndPos = src.find_first_of("\n\r", currPos);
                line = src.substr(currPos, lineEndPos - currPos);
                StringVector parts = StringUtil::split(line, " \t");

                size_t partStart = 0;
                for( size_t i=0; i<c_numTypeQualifiers && partStart<parts.size(); ++i )
                {
                    if( StringUtil::match( parts[partStart], c_typeQualifiers[i] ) )
                    {
                        i = 0;
                        ++partStart;
                    }
                }

                if( partStart < parts.size() )
                    typeString = parts[partStart];
                else
                    typeString = parts[0];

                StringToEnumMap::iterator typei = mTypeEnumMap.find(typeString);
                if (typei == mTypeEnumMap.end())
                {
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

                    // // Find the internal name.
                    // // This can be an array.
                    // line = src.substr(currPos, endPos - currPos);
                    // StringVector internalParts = StringUtil::split(line, ", \t\r\n");
                    // String internalName = "";
                    // uint16 arraySize = 0;
                    // for (StringVector::iterator i = internalParts.begin(); i != internalParts.end(); ++i)
                    // {
                    //     StringUtil::trim(*i);
                    //     String::size_type arrayStart = i->find("[", 0);
                    //     if (arrayStart != String::npos)
                    //     {
                    //         // potential name (if butted up to array)
                    //         String name = i->substr(0, arrayStart);
                    //         StringUtil::trim(name);
                    //         if (!name.empty())
                    //             internalName = name;

                    //         String::size_type arrayEnd = i->find("]", arrayStart);
                    //         String arrayDimTerm = i->substr(arrayStart + 1, arrayEnd - arrayStart - 1);
                    //         StringUtil::trim(arrayDimTerm);
                    //         arraySize = StringConverter::parseUnsignedInt(arrayDimTerm);
                    //     }
                    //     else
                    //     {
                    //         internalName = *i;
                    //     }
                    // }

                    // // Ok, now rewind and parse the individual uniforms in this block
                    // currPos = openBracePos + 1;
                    // blockSharedParams = GpuProgramManager::getSingleton().getSharedParameters(externalName);
                    // if(blockSharedParams.isNull())
                    //     blockSharedParams = GpuProgramManager::getSingleton().createSharedParameters(externalName);
                    // do
                    // {
                    //     lineEndPos = src.find_first_of("\n\r", currPos);
                    //     endPos = src.find(";", currPos);
                    //     line = src.substr(currPos, endPos - currPos);

                    //     // TODO: Give some sort of block id
                    //     // Parse the normally structured uniform
                    //     parseIndividualConstant(src, defs, currPos, filename, blockSharedParams);
                    //     currPos = lineEndPos + 1;
                    // } while (endBracePos > currPos);
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

                    parseGLSLUniform(src, defs, currPos, filename, blockSharedParams);
                }
                line = src.substr(currPos, endPos - currPos);
            } // not commented or a larger symbol

            // Find next one
            currPos = src.find("uniform", currPos);
        }
    }

    
    void GLSLProgramManager::parseGLSLUniform(
        const String& src, GpuNamedConstants& defs,
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
        //FIXME What are valid ways of including spaces in GLSL
        // variable declarations?  May need regex.
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
                convertGLUniformtoOgreType(typei->second, def);
            }
            else
            {
                // if this is not a type, and not empty, it should be a name
                StringUtil::trim(*i);
                if (i->empty()) continue;

                {
                    // Skip over precision keywords & other similar type qualifiers
                    bool skipElement = false;
                    for( size_t j=0; j<c_numTypeQualifiers && !skipElement; ++j )
                        skipElement = StringUtil::match( *i, c_typeQualifiers[j] );

                    if( skipElement )
                        continue;
                }

                String::size_type arrayStart = i->find("[", 0);
                if (arrayStart != String::npos)
                {
                    // potential name (if butted up to array)
                    String name = i->substr(0, arrayStart);
                    StringUtil::trim(name);
                    if (!name.empty())
                        paramName = name;

                    def.arraySize = 1;

                    // N-dimensional arrays
                    while (arrayStart != String::npos) {
                        String::size_type arrayEnd = i->find("]", arrayStart);
                        String arrayDimTerm = i->substr(arrayStart + 1, arrayEnd - arrayStart - 1);
                        StringUtil::trim(arrayDimTerm);
                        //TODO
                        // the array term might be a simple number or it might be
                        // an expression (e.g. 24*3) or refer to a constant expression
                        // we'd have to evaluate the expression which could get nasty
                        def.arraySize *= StringConverter::parseInt(arrayDimTerm);
                        arrayStart = i->find("[", arrayEnd);
                    }
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
                    else if (def.isDouble())
                    {
                        def.physicalIndex = defs.doubleBufferSize;
                        defs.doubleBufferSize += def.arraySize * def.elementSize;
                    }
                    else if (def.isInt() || def.isSampler())
                    {
                        def.physicalIndex = defs.intBufferSize;
                        defs.intBufferSize += def.arraySize * def.elementSize;
                    }
                    else if (def.isUnsignedInt() || def.isBool())
                    {
                        def.physicalIndex = defs.uintBufferSize;
                        defs.uintBufferSize += def.arraySize * def.elementSize;
                    }
                    // else if (def.isBool())
                    // {
                    //     def.physicalIndex = defs.boolBufferSize;
                    //     defs.boolBufferSize += def.arraySize * def.elementSize;
                    // }
                    else
                    {
                        LogManager::getSingleton().logMessage("Could not parse type of GLSL Uniform: '"
                                                              + line + "' in file " + filename);
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
                    catch (Exception&)
                    {
                        // This constant doesn't exist so we'll create a new one
                        sharedParams->addConstantDefinition(paramName, def.constType);
                    }
                }
            }
        }
    }
}
