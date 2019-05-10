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
#include "OgreGL3PlusRenderSystem.h"
#include "OgreRoot.h"

#include "OgreGLSLMonolithicProgram.h"
#include "OgreGLSLSeparableProgram.h"

namespace Ogre {

    template<> GLSLProgramManager* Singleton<GLSLProgramManager>::msSingleton = 0;


    GLSLProgramManager* GLSLProgramManager::getSingletonPtr(void)
    {
        return msSingleton;
    }


    GLSLProgramManager& GLSLProgramManager::getSingleton(void)
    {
        assert(msSingleton);
        return (*msSingleton);
    }
    
    GLSLProgramManager::GLSLProgramManager(GL3PlusRenderSystem* renderSystem) :
        mActiveVertexShader(NULL),
        mActiveHullShader(NULL),
        mActiveDomainShader(NULL),
        mActiveGeometryShader(NULL),
        mActiveFragmentShader(NULL),
        mActiveComputeShader(NULL),
        mActiveProgram(NULL),
        mRenderSystem(renderSystem)
    {
        // Fill in the relationship between type names and enums
        mTypeEnumMap.emplace("float", GL_FLOAT);
        mTypeEnumMap.emplace("vec2", GL_FLOAT_VEC2);
        mTypeEnumMap.emplace("vec3", GL_FLOAT_VEC3);
        mTypeEnumMap.emplace("vec4", GL_FLOAT_VEC4);
        mTypeEnumMap.emplace("sampler1D", GL_SAMPLER_1D);
        mTypeEnumMap.emplace("sampler2D", GL_SAMPLER_2D);
        mTypeEnumMap.emplace("sampler3D", GL_SAMPLER_3D);
        mTypeEnumMap.emplace("samplerCube", GL_SAMPLER_CUBE);
        mTypeEnumMap.emplace("sampler1DShadow", GL_SAMPLER_1D_SHADOW);
        mTypeEnumMap.emplace("sampler2DShadow", GL_SAMPLER_2D_SHADOW);
        mTypeEnumMap.emplace("int", GL_INT);
        mTypeEnumMap.emplace("ivec2", GL_INT_VEC2);
        mTypeEnumMap.emplace("ivec3", GL_INT_VEC3);
        mTypeEnumMap.emplace("ivec4", GL_INT_VEC4);
        mTypeEnumMap.emplace("bool", GL_BOOL);
        mTypeEnumMap.emplace("bvec2", GL_BOOL_VEC2);
        mTypeEnumMap.emplace("bvec3", GL_BOOL_VEC3);
        mTypeEnumMap.emplace("bvec4", GL_BOOL_VEC4);
        mTypeEnumMap.emplace("mat2", GL_FLOAT_MAT2);
        mTypeEnumMap.emplace("mat3", GL_FLOAT_MAT3);
        mTypeEnumMap.emplace("mat4", GL_FLOAT_MAT4);

        // GL 2.1
        mTypeEnumMap.emplace("mat2x2", GL_FLOAT_MAT2);
        mTypeEnumMap.emplace("mat3x3", GL_FLOAT_MAT3);
        mTypeEnumMap.emplace("mat4x4", GL_FLOAT_MAT4);
        mTypeEnumMap.emplace("mat2x3", GL_FLOAT_MAT2x3);
        mTypeEnumMap.emplace("mat3x2", GL_FLOAT_MAT3x2);
        mTypeEnumMap.emplace("mat3x4", GL_FLOAT_MAT3x4);
        mTypeEnumMap.emplace("mat4x3", GL_FLOAT_MAT4x3);
        mTypeEnumMap.emplace("mat2x4", GL_FLOAT_MAT2x4);
        mTypeEnumMap.emplace("mat4x2", GL_FLOAT_MAT4x2);

        // GL 3.0
        mTypeEnumMap.emplace("uint", GL_UNSIGNED_INT);
        mTypeEnumMap.emplace("uvec2", GL_UNSIGNED_INT_VEC2);
        mTypeEnumMap.emplace("uvec3", GL_UNSIGNED_INT_VEC3);
        mTypeEnumMap.emplace("uvec4", GL_UNSIGNED_INT_VEC4);
        mTypeEnumMap.emplace("samplerCubeShadow", GL_SAMPLER_CUBE_SHADOW);
        mTypeEnumMap.emplace("sampler1DArray", GL_SAMPLER_1D_ARRAY);
        mTypeEnumMap.emplace("sampler2DArray", GL_SAMPLER_2D_ARRAY);
        mTypeEnumMap.emplace("sampler1DArrayShadow", GL_SAMPLER_1D_ARRAY_SHADOW);
        mTypeEnumMap.emplace("sampler2DArrayShadow", GL_SAMPLER_2D_ARRAY_SHADOW);
        mTypeEnumMap.emplace("isampler1D", GL_INT_SAMPLER_1D);
        mTypeEnumMap.emplace("isampler2D", GL_INT_SAMPLER_2D);
        mTypeEnumMap.emplace("isampler3D", GL_INT_SAMPLER_3D);
        mTypeEnumMap.emplace("isamplerCube", GL_INT_SAMPLER_CUBE);
        mTypeEnumMap.emplace("isampler1DArray", GL_INT_SAMPLER_1D_ARRAY);
        mTypeEnumMap.emplace("isampler2DArray", GL_INT_SAMPLER_2D_ARRAY);
        mTypeEnumMap.emplace("usampler1D", GL_UNSIGNED_INT_SAMPLER_1D);
        mTypeEnumMap.emplace("usampler2D", GL_UNSIGNED_INT_SAMPLER_2D);
        mTypeEnumMap.emplace("usampler3D", GL_UNSIGNED_INT_SAMPLER_3D);
        mTypeEnumMap.emplace("usamplerCube", GL_UNSIGNED_INT_SAMPLER_CUBE);
        mTypeEnumMap.emplace("usampler1DArray", GL_UNSIGNED_INT_SAMPLER_1D_ARRAY);
        mTypeEnumMap.emplace("usampler2DArray", GL_UNSIGNED_INT_SAMPLER_2D_ARRAY);

        // GL 3.1
        mTypeEnumMap.emplace("sampler2DRect", GL_SAMPLER_2D_RECT);
        mTypeEnumMap.emplace("sampler2DRectShadow", GL_SAMPLER_2D_RECT_SHADOW);
        mTypeEnumMap.emplace("isampler2DRect", GL_INT_SAMPLER_2D_RECT);
        mTypeEnumMap.emplace("usampler2DRect", GL_UNSIGNED_INT_SAMPLER_2D_RECT);
        mTypeEnumMap.emplace("samplerBuffer", GL_SAMPLER_BUFFER);
        mTypeEnumMap.emplace("isamplerBuffer", GL_INT_SAMPLER_BUFFER);
        mTypeEnumMap.emplace("usamplerBuffer", GL_UNSIGNED_INT_SAMPLER_BUFFER);

        // GL 3.2
        mTypeEnumMap.emplace("sampler2DMS", GL_SAMPLER_2D_MULTISAMPLE);
        mTypeEnumMap.emplace("isampler2DMS", GL_INT_SAMPLER_2D_MULTISAMPLE);
        mTypeEnumMap.emplace("usampler2DMS", GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE);
        mTypeEnumMap.emplace("sampler2DMSArray", GL_SAMPLER_2D_MULTISAMPLE_ARRAY);
        mTypeEnumMap.emplace("isampler2DMSArray", GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY);
        mTypeEnumMap.emplace("usampler2DMSArray", GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY);

        // GL 4.0
        mTypeEnumMap.emplace("double", GL_DOUBLE);
        mTypeEnumMap.emplace("dmat2", GL_DOUBLE_MAT2);
        mTypeEnumMap.emplace("dmat3", GL_DOUBLE_MAT3);
        mTypeEnumMap.emplace("dmat4", GL_DOUBLE_MAT4);
        mTypeEnumMap.emplace("dmat2x2", GL_DOUBLE_MAT2);
        mTypeEnumMap.emplace("dmat3x3", GL_DOUBLE_MAT3);
        mTypeEnumMap.emplace("dmat4x4", GL_DOUBLE_MAT4);
        mTypeEnumMap.emplace("dmat2x3", GL_DOUBLE_MAT2x3);
        mTypeEnumMap.emplace("dmat3x2", GL_DOUBLE_MAT3x2);
        mTypeEnumMap.emplace("dmat3x4", GL_DOUBLE_MAT3x4);
        mTypeEnumMap.emplace("dmat4x3", GL_DOUBLE_MAT4x3);
        mTypeEnumMap.emplace("dmat2x4", GL_DOUBLE_MAT2x4);
        mTypeEnumMap.emplace("dmat4x2", GL_DOUBLE_MAT4x2);
        mTypeEnumMap.emplace("dvec2", GL_DOUBLE_VEC2);
        mTypeEnumMap.emplace("dvec3", GL_DOUBLE_VEC3);
        mTypeEnumMap.emplace("dvec4", GL_DOUBLE_VEC4);
        mTypeEnumMap.emplace("samplerCubeArray", GL_SAMPLER_CUBE_MAP_ARRAY);
        mTypeEnumMap.emplace("samplerCubeArrayShadow", GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW);
        mTypeEnumMap.emplace("isamplerCubeArray", GL_INT_SAMPLER_CUBE_MAP_ARRAY);
        mTypeEnumMap.emplace("usamplerCubeArray", GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY);

        mTypeEnumMap.emplace("image1D", GL_IMAGE_1D);
        mTypeEnumMap.emplace("iimage1D", GL_INT_IMAGE_1D);
        mTypeEnumMap.emplace("uimage1D", GL_UNSIGNED_INT_IMAGE_1D);
        mTypeEnumMap.emplace("image2D", GL_IMAGE_2D);
        mTypeEnumMap.emplace("iimage2D", GL_INT_IMAGE_2D);
        mTypeEnumMap.emplace("uimage2D", GL_UNSIGNED_INT_IMAGE_2D);
        mTypeEnumMap.emplace("image3D", GL_IMAGE_3D);
        mTypeEnumMap.emplace("iimage3D", GL_INT_IMAGE_3D);
        mTypeEnumMap.emplace("uimage3D", GL_UNSIGNED_INT_IMAGE_3D);
        mTypeEnumMap.emplace("image2DRect", GL_IMAGE_2D_RECT);
        mTypeEnumMap.emplace("iimage2DRect", GL_INT_IMAGE_2D_RECT);
        mTypeEnumMap.emplace("uimage2DRect", GL_UNSIGNED_INT_IMAGE_2D_RECT);
        mTypeEnumMap.emplace("imageCube", GL_IMAGE_CUBE);
        mTypeEnumMap.emplace("iimageCube", GL_INT_IMAGE_CUBE);
        mTypeEnumMap.emplace("uimageCube", GL_UNSIGNED_INT_IMAGE_CUBE);
        mTypeEnumMap.emplace("imageBuffer", GL_IMAGE_BUFFER);
        mTypeEnumMap.emplace("iimageBuffer", GL_INT_IMAGE_BUFFER);
        mTypeEnumMap.emplace("uimageBuffer", GL_UNSIGNED_INT_IMAGE_BUFFER);
        mTypeEnumMap.emplace("image1DArray", GL_IMAGE_1D_ARRAY);
        mTypeEnumMap.emplace("iimage1DArray", GL_INT_IMAGE_1D_ARRAY);
        mTypeEnumMap.emplace("uimage1DArray", GL_UNSIGNED_INT_IMAGE_1D_ARRAY);
        mTypeEnumMap.emplace("image2DArray", GL_IMAGE_2D_ARRAY);
        mTypeEnumMap.emplace("iimage2DArray", GL_INT_IMAGE_2D_ARRAY);
        mTypeEnumMap.emplace("uimage2DArray", GL_UNSIGNED_INT_IMAGE_2D_ARRAY);
        mTypeEnumMap.emplace("imageCubeArray", GL_IMAGE_CUBE_MAP_ARRAY);
        mTypeEnumMap.emplace("iimageCubeArray", GL_INT_IMAGE_CUBE_MAP_ARRAY);
        mTypeEnumMap.emplace("uimageCubeArray", GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY);
        mTypeEnumMap.emplace("image2DMS", GL_IMAGE_2D_MULTISAMPLE);
        mTypeEnumMap.emplace("iimage2DMS", GL_INT_IMAGE_2D_MULTISAMPLE);
        mTypeEnumMap.emplace("uimage2DMS", GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE);
        mTypeEnumMap.emplace("image2DMSArray", GL_IMAGE_2D_MULTISAMPLE_ARRAY);
        mTypeEnumMap.emplace("iimage2DMSArray", GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY);
        mTypeEnumMap.emplace("uimage2DMSArray", GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY);

        // GL 4.2
        mTypeEnumMap.emplace("atomic_uint", GL_UNSIGNED_INT_ATOMIC_COUNTER);
    }

    GLSLProgramManager::~GLSLProgramManager(void) {}

    GL3PlusStateCacheManager* GLSLProgramManager::getStateCacheManager()
    {
        return mRenderSystem->_getStateCacheManager();
    }

    GLSLProgram* GLSLProgramManager::getActiveProgram(void)
    {
        // If there is an active link program then return it.
        if (mActiveProgram)
            return mActiveProgram;

        // No active link program so find one or make a new one.
        // Is there an active key?
        uint32 activeKey = 0;
        if (mActiveVertexShader)
        {
            activeKey = HashCombine(activeKey, mActiveVertexShader->getShaderID());
        }
        if (mActiveDomainShader)
        {
            activeKey = HashCombine(activeKey, mActiveDomainShader->getShaderID());
        }
        if (mActiveHullShader)
        {
            activeKey = HashCombine(activeKey, mActiveHullShader->getShaderID());
        }
        if (mActiveGeometryShader)
        {
            activeKey = HashCombine(activeKey, mActiveGeometryShader->getShaderID());
        }
        if (mActiveFragmentShader)
        {
            activeKey = HashCombine(activeKey, mActiveFragmentShader->getShaderID());
        }
        if (mActiveComputeShader)
        {
            activeKey = HashCombine(activeKey, mActiveComputeShader->getShaderID());
        }

        // Only return a link program object if a program exists.
        if (activeKey > 0)
        {
            // Find the key in the hash map.
            ProgramIterator programFound = mPrograms.find(activeKey);
            // Program object not found for key so need to create it.
            if (programFound == mPrograms.end())
            {
                if (mRenderSystem->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
                {
                    mActiveProgram = new GLSLSeparableProgram(
                        mActiveVertexShader, mActiveHullShader, mActiveDomainShader,
                        mActiveGeometryShader, mActiveFragmentShader, mActiveComputeShader);
                }
                else
                {
                    mActiveProgram = new GLSLMonolithicProgram(
                        mActiveVertexShader, mActiveHullShader, mActiveDomainShader,
                        mActiveGeometryShader, mActiveFragmentShader, mActiveComputeShader);
                }

                mPrograms[activeKey] = mActiveProgram;
            }
            else
            {
                // Found a link program in map container so make it active.
                mActiveProgram = static_cast<GLSLProgram*>(programFound->second);
            }
        }

        // Make the program object active.
        if (mActiveProgram)
            mActiveProgram->activate();

        return mActiveProgram;
    }

    void GLSLProgramManager::setActiveFragmentShader(GLSLShader* fragmentShader)
    {
        if (fragmentShader != mActiveFragmentShader)
        {
            mActiveFragmentShader = fragmentShader;
            // ActiveMonolithicProgram is no longer valid
            mActiveProgram = NULL;
        }
    }


    void GLSLProgramManager::setActiveVertexShader(GLSLShader* vertexShader)
    {
        if (vertexShader != mActiveVertexShader)
        {
            mActiveVertexShader = vertexShader;
            // ActiveMonolithicProgram is no longer valid
            mActiveProgram = NULL;
        }
    }


    void GLSLProgramManager::setActiveGeometryShader(GLSLShader* geometryShader)
    {
        if (geometryShader != mActiveGeometryShader)
        {
            mActiveGeometryShader = geometryShader;
            // ActiveMonolithicProgram is no longer valid
            mActiveProgram = NULL;
        }
    }


    void GLSLProgramManager::setActiveHullShader(GLSLShader* hullShader)
    {
        if (hullShader != mActiveHullShader)
        {
            mActiveHullShader = hullShader;
            // ActiveMonolithicProgram is no longer valid
            mActiveProgram = NULL;
        }
    }


    void GLSLProgramManager::setActiveDomainShader(GLSLShader* domainShader)
    {
        if (domainShader != mActiveDomainShader)
        {
            mActiveDomainShader = domainShader;
            // ActiveMonolithicProgram is no longer valid
            mActiveProgram = NULL;
        }
    }


    void GLSLProgramManager::setActiveComputeShader(GLSLShader* computeShader)
    {
        if (computeShader != mActiveComputeShader)
        {
            mActiveComputeShader = computeShader;
            // ActiveMonolithicProgram is no longer valid
            mActiveProgram = NULL;
        }
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
        case GL_IMAGE_1D: //TODO should be its own type?
        case GL_SAMPLER_1D:
        case GL_SAMPLER_1D_ARRAY:
        case GL_INT_SAMPLER_1D:
        case GL_INT_SAMPLER_1D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_1D:
        case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
            // need to record samplers for GLSL
            defToUpdate.constType = GCT_SAMPLER1D;
            break;
        case GL_IMAGE_2D: //TODO should be its own type?
        case GL_IMAGE_2D_RECT:
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
        case GL_IMAGE_3D: //TODO should be its own type?
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
            defToUpdate.constType = GCT_UNKNOWN;
            break;
        }

        // GL doesn't pad
        defToUpdate.elementSize = GpuConstantDefinition::getElementSize(defToUpdate.constType, false);
    }

    
    bool GLSLProgramManager::findUniformDataSource(
        const String& paramName,
        const GpuConstantDefinitionMap* (&constantDefs)[6],
        GLUniformReference& refToUpdate)
    {
        for(int i = 0; i < 6; i++) {
            if (constantDefs[i])
            {
                GpuConstantDefinitionMap::const_iterator parami =
                        constantDefs[i]->find(paramName);
                if (parami != constantDefs[i]->end())
                {
                    refToUpdate.mSourceProgType = static_cast<GpuProgramType>(i);
                    refToUpdate.mConstantDef = &(parami->second);
                    return true;
                }
            }
        }
        return false;
    }

    
    //FIXME This is code bloat...either template or unify UniformReference
    // and AtomicCounterReference
    bool GLSLProgramManager::findAtomicCounterDataSource(
        const String& paramName,
        const GpuConstantDefinitionMap* (&constantDefs)[6],
        GLAtomicCounterReference& refToUpdate)
    {
        for(int i = 0; i < 6; i++) {
            if (constantDefs[i])
            {
                GpuConstantDefinitionMap::const_iterator parami =
                        constantDefs[i]->find(paramName);
                if (parami != constantDefs[i]->end())
                {
                    refToUpdate.mSourceProgType = static_cast<GpuProgramType>(i);
                    refToUpdate.mConstantDef = &(parami->second);
                    return true;
                }
            }
        }
        return false;
    }


    
    void GLSLProgramManager::extractUniformsFromProgram(
        GLuint programObject,
        const GpuConstantDefinitionMap* (&constantDefs)[6],
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
                String::size_type arrayStart = paramName.find('[');
                if (arrayStart != String::npos)
                {
                    // if not the first array element then skip it and continue to the next uniform
                    if (paramName.compare(arrayStart, paramName.size() - 1, "[0]") != 0) continue;
                    paramName = paramName.substr(0, arrayStart);
                }

                // Find out which params object this comes from
                bool foundSource = findUniformDataSource(paramName, constantDefs, newGLUniformReference);

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
                String::size_type arrayStart = paramName.find('[');
                if (arrayStart != String::npos)
                {
                    // if not the first array element then skip it and continue to the next uniform
                    if (paramName.compare(arrayStart, paramName.size() - 1, "[0]") != 0) continue;
                    paramName = paramName.substr(0, arrayStart);
                }

                printf("ATOMIC COUNTER FOUND: %s %d", paramName.c_str(), arraySize);

                // Find out which params object this comes from
                bool foundSource = findAtomicCounterDataSource(
                    paramName, constantDefs,newGLAtomicCounterReference);

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

        // Now deal with uniform blocks

        OGRE_CHECK_GL_ERROR(glGetProgramiv(programObject, GL_ACTIVE_UNIFORM_BLOCKS, &blockCount));

        for (int index = 0; index < blockCount; index++)
        {
            OGRE_CHECK_GL_ERROR(glGetActiveUniformBlockName(programObject, index, uniformLength, NULL, uniformName));

            // Map uniform block to binding point of GL buffer of
            // shared param bearing the same name.

            GpuSharedParametersPtr blockSharedParams = GpuProgramManager::getSingleton().getSharedParameters(uniformName);
            //TODO error handling for when buffer has no associated shared parameter?
            //if (bufferi == mSharedParamGLBufferMap.end()) continue;

            GL3PlusHardwareUniformBuffer* hwGlBuffer;
            SharedParamsBufferMap::const_iterator bufferMapi = sharedParamsBufferMap.find(blockSharedParams);
            if (bufferMapi != sharedParamsBufferMap.end())
            {
                hwGlBuffer = static_cast<GL3PlusHardwareUniformBuffer*>(bufferMapi->second.get());
            }
            else
            {
                // Create buffer and add entry to buffer map.
                GLint blockSize;
                OGRE_CHECK_GL_ERROR(glGetActiveUniformBlockiv(programObject, index, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize));
                HardwareUniformBufferSharedPtr newUniformBuffer = HardwareBufferManager::getSingleton().createUniformBuffer(blockSize, HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE, false, uniformName);
                // bufferMapi->second() = newUniformBuffer;
                hwGlBuffer = static_cast<GL3PlusHardwareUniformBuffer*>(newUniformBuffer.get());
                GLint bufferBinding = sharedParamsBufferMap.size();
                hwGlBuffer->setGLBufferBinding(bufferBinding);
                std::pair<GpuSharedParametersPtr, HardwareUniformBufferSharedPtr> newPair (blockSharedParams, newUniformBuffer);
                sharedParamsBufferMap.insert(newPair);

                // Get active block parameter properties.
                GpuConstantDefinitionIterator sharedParamDef = blockSharedParams->getConstantDefinitionIterator();
                std::vector<const char*> sharedParamNames;
                for (; sharedParamDef.current() != sharedParamDef.end(); sharedParamDef.moveNext())
                {
                    sharedParamNames.push_back(sharedParamDef.current()->first.c_str());
                }

                std::vector<GLuint> uniformParamIndices(sharedParamNames.size());
                std::vector<GLint> uniformParamOffsets(sharedParamNames.size());

                OGRE_CHECK_GL_ERROR(glGetUniformIndices(programObject, sharedParamNames.size(), &sharedParamNames[0], &uniformParamIndices[0]));
                //FIXME debug this (see stdout)
                OGRE_CHECK_GL_ERROR(glGetActiveUniformsiv(programObject, uniformParamIndices.size(), &uniformParamIndices[0], GL_UNIFORM_OFFSET, &uniformParamOffsets[0]));
                //TODO handle uniform arrays
                //GL_UNIFORM_ARRAY_STRIDE
                //TODO handle matrices
                //GL_UNIFORM_MATRIX_STRIDE

                GpuNamedConstants& consts = const_cast<GpuNamedConstants&>(blockSharedParams->getConstantDefinitions());
                MapIterator<GpuConstantDefinitionMap> sharedParamDefMut(consts.map);
                for (size_t i = 0; sharedParamDefMut.current() != sharedParamDefMut.end(); sharedParamDefMut.moveNext(), i++)
                {
                    // NOTE: the naming in GL3Plus is backward. logicalIndex is actually the physical index of the parameter
                    // while the physicalIndex is the logical array offset..
                    sharedParamDefMut.current()->second.logicalIndex = uniformParamOffsets[i];
                }
            }

            GLint bufferBinding = hwGlBuffer->getGLBufferBinding();

            //OGRE_CHECK_GL_ERROR(glGetActiveUniformBlockiv(programObject, index, GL_UNIFORM_BLOCK_BINDING, &blockBinding));

            OGRE_CHECK_GL_ERROR(glUniformBlockBinding(programObject, index, bufferBinding));
        }

        // Now deal with shader storage blocks

        //TODO Need easier, more robust feature checking.
        // if (mRenderSystem->checkExtension("GL_ARB_program_interface_query") || gl3wIsSupported(4, 3))
        if (mRenderSystem->hasMinGLVersion(4, 3))
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
                GL3PlusHardwareShaderStorageBuffer* hwGlBuffer;
                SharedParamsBufferMap::const_iterator bufferMapi = sharedParamsBufferMap.find(blockSharedParams);
                if (bufferMapi != sharedParamsBufferMap.end())
                {
                    hwGlBuffer = static_cast<GL3PlusHardwareShaderStorageBuffer*>(bufferMapi->second.get());
                }
                else
                {
                    // Create buffer and add entry to buffer map.
                    // bufferMapi->second() = newUniformBuffer;

                    GLint blockSize;
                    // const GLenum properties [2] = {GL_BUFFER_DATA_SIZE, GL_BUFFER_BINDING};
                    GLenum properties[] = {GL_BUFFER_DATA_SIZE};
                    OGRE_CHECK_GL_ERROR(glGetProgramResourceiv(programObject, GL_SHADER_STORAGE_BLOCK, index, 1, properties, 1, NULL, &blockSize));
                    //blockSize = properties[0];
                    //TODO Implement shared param access param in materials (R, W, R+W)
                    // HardwareUniformBufferSharedPtr newShaderStorageBuffer = static_cast<GL3PlusHardwareBufferManager*>(HardwareBufferManager::getSingletonPtr())->createShaderStorageBuffer(blockSize, HardwareBuffer::HBU_DYNAMIC, false, uniformName);
                    HardwareUniformBufferSharedPtr newShaderStorageBuffer = static_cast<GL3PlusHardwareBufferManager*>(HardwareBufferManager::getSingletonPtr())->createShaderStorageBuffer(blockSize, HardwareBuffer::HBU_DYNAMIC, false, uniformName);
                    hwGlBuffer = static_cast<GL3PlusHardwareShaderStorageBuffer*>(newShaderStorageBuffer.get());

                    //FIXME check parameters
                    GLint bufferBinding = sharedParamsBufferMap.size();
                    hwGlBuffer->setGLBufferBinding(bufferBinding);

                    std::pair<GpuSharedParametersPtr, HardwareUniformBufferSharedPtr> newPair (blockSharedParams, newShaderStorageBuffer);
                    sharedParamsBufferMap.insert(newPair);

                    // Get active block parameter properties.
                    properties[0] = GL_OFFSET;
                    GpuNamedConstants& consts = const_cast<GpuNamedConstants&>(blockSharedParams->getConstantDefinitions());
                    MapIterator<GpuConstantDefinitionMap> sharedParamDef(consts.map);
                    for (size_t i = 0; sharedParamDef.current() != sharedParamDef.end(); sharedParamDef.moveNext(), i++) {
                        GLuint varIndex = glGetProgramResourceIndex(programObject, GL_BUFFER_VARIABLE, sharedParamDef.current()->first.c_str());
                        GLint offset;
                        glGetProgramResourceiv(programObject, GL_BUFFER_VARIABLE, varIndex, 1, properties, 1, NULL, &offset);
                        sharedParamDef.current()->second.logicalIndex = offset;
                    }
                }

                GLint bufferBinding = hwGlBuffer->getGLBufferBinding();

                OGRE_CHECK_GL_ERROR(glShaderStorageBlockBinding(programObject, index, bufferBinding));
            }
        }
        // if (mRenderSystem->checkExtension("GL_ARB_shader_atomic_counters") || gl3wIsSupported(4, 2))
        if (mRenderSystem->hasMinGLVersion(4, 2))
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
                HardwareCounterBufferSharedPtr newCounterBuffer = HardwareBufferManager::getSingleton().createCounterBuffer(bufferSize, HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE, false);

                GL3PlusHardwareCounterBuffer* hwGlBuffer = static_cast<GL3PlusHardwareCounterBuffer*>(newCounterBuffer.get());
                hwGlBuffer->setGLBufferBinding(bufferBinding);
                counterBufferList.push_back(newCounterBuffer);
            }
        }
    }
}
