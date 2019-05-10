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

#include "OgreGLSLLinkProgramManager.h"
#include "OgreGLSLGpuProgram.h"
#include "OgreGLSLProgram.h"
#include "OgreStringConverter.h"
#include "OgreLogManager.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    template<> GLSL::GLSLLinkProgramManager* Singleton<GLSL::GLSLLinkProgramManager>::msSingleton = 0;

    namespace GLSL {

    //-----------------------------------------------------------------------
    GLSLLinkProgramManager* GLSLLinkProgramManager::getSingletonPtr(void)
    {
        return msSingleton;
    }

    //-----------------------------------------------------------------------
    GLSLLinkProgramManager& GLSLLinkProgramManager::getSingleton(void)
    {  
        assert( msSingleton );  return ( *msSingleton );  
    }

    //-----------------------------------------------------------------------
    GLSLLinkProgramManager::GLSLLinkProgramManager(void) : mActiveVertexGpuProgram(NULL),
        mActiveGeometryGpuProgram(NULL), mActiveFragmentGpuProgram(NULL), mActiveLinkProgram(NULL)
    {
        // Fill in the relationship between type names and enums
        mTypeEnumMap.emplace("float", GL_FLOAT);
        mTypeEnumMap.emplace("vec2", GL_FLOAT_VEC2);
        mTypeEnumMap.emplace("vec3", GL_FLOAT_VEC3);
        mTypeEnumMap.emplace("vec4", GL_FLOAT_VEC4);
        mTypeEnumMap.emplace("sampler1D", GL_SAMPLER_1D);
        mTypeEnumMap.emplace("sampler2D", GL_SAMPLER_2D);
        mTypeEnumMap.emplace("sampler3D", GL_SAMPLER_3D);
        mTypeEnumMap.emplace("sampler2DArray", GL_SAMPLER_2D_ARRAY_EXT);
        mTypeEnumMap.emplace("samplerCube", GL_SAMPLER_CUBE);
        mTypeEnumMap.emplace("sampler1DShadow", GL_SAMPLER_1D_SHADOW);
        mTypeEnumMap.emplace("sampler2DShadow", GL_SAMPLER_2D_SHADOW);
        mTypeEnumMap.emplace("int", GL_INT);
        mTypeEnumMap.emplace("ivec2", GL_INT_VEC2);
        mTypeEnumMap.emplace("ivec3", GL_INT_VEC3);
        mTypeEnumMap.emplace("ivec4", GL_INT_VEC4);
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

    }

    //-----------------------------------------------------------------------
    GLSLLinkProgramManager::~GLSLLinkProgramManager(void) {}

    //-----------------------------------------------------------------------
    GLSLLinkProgram* GLSLLinkProgramManager::getActiveLinkProgram(void)
    {
        // if there is an active link program then return it
        if (mActiveLinkProgram)
            return mActiveLinkProgram;

        // no active link program so find one or make a new one
        // is there an active key?
        uint32 activeKey = 0;

        if (mActiveVertexGpuProgram)
        {
            activeKey = HashCombine(activeKey, mActiveVertexGpuProgram->getShaderID());
        }
        if (mActiveGeometryGpuProgram)
        {
            activeKey = HashCombine(activeKey, mActiveGeometryGpuProgram->getShaderID());
        }
        if (mActiveFragmentGpuProgram)
        {
            activeKey = HashCombine(activeKey, mActiveFragmentGpuProgram->getShaderID());
        }

        // only return a link program object if a vertex, geometry or fragment program exist
        if (activeKey > 0)
        {
            // find the key in the hash map
            ProgramIterator programFound = mPrograms.find(activeKey);
            // program object not found for key so need to create it
            if (programFound == mPrograms.end())
            {
                mActiveLinkProgram = new GLSLLinkProgram(mActiveVertexGpuProgram, mActiveGeometryGpuProgram,mActiveFragmentGpuProgram);
                mPrograms[activeKey] = mActiveLinkProgram;
            }
            else
            {
                // found a link program in map container so make it active
                mActiveLinkProgram = static_cast<GLSLLinkProgram*>(programFound->second);
            }

        }
        // make the program object active
        if (mActiveLinkProgram) mActiveLinkProgram->activate();

        return mActiveLinkProgram;

    }

    //-----------------------------------------------------------------------
    void GLSLLinkProgramManager::setActiveFragmentShader(GLSLProgram* fragmentGpuProgram)
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
    void GLSLLinkProgramManager::setActiveVertexShader(GLSLProgram* vertexGpuProgram)
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
    void GLSLLinkProgramManager::setActiveGeometryShader(GLSLProgram* geometryGpuProgram)
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
    void GLSLLinkProgramManager::convertGLUniformtoOgreType(GLenum gltype,
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
        case GL_SAMPLER_2D_ARRAY_EXT:
            defToUpdate.constType = GCT_SAMPLER2DARRAY;
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
        GLint uniformCount = 0;

        #define BUFFERSIZE 200
        char   uniformName[BUFFERSIZE] = "";
        //GLint location;
        GLUniformReference newGLUniformReference;

        // get the number of active uniforms
        glGetObjectParameterivARB(programObject, GL_OBJECT_ACTIVE_UNIFORMS_ARB,
            &uniformCount);

        // Loop over each of the active uniforms, and add them to the reference container
        // only do this for user defined uniforms, ignore built in gl state uniforms
        for (int index = 0; index < uniformCount; index++)
        {
            GLint arraySize = 0;
            GLenum glType;
            glGetActiveUniformARB(programObject, index, BUFFERSIZE, NULL, 
                &arraySize, &glType, uniformName);
            // don't add built in uniforms
            newGLUniformReference.mLocation = glGetUniformLocationARB(programObject, uniformName);
            if (newGLUniformReference.mLocation >= 0)
            {
                // user defined uniform found, add it to the reference list
                String paramName = String( uniformName );

                // Current ATI drivers (Catalyst 7.2 and earlier) and older NVidia drivers will include all array elements as uniforms but we only want the root array name and location
                // Also note that ATI Catalyst 6.8 to 7.2 there is a bug with glUniform that does not allow you to update a uniform array past the first uniform array element
                // ie you can't start updating an array starting at element 1, must always be element 0.

                // if the uniform name has a "[" in it then its an array element uniform.
                String::size_type arrayStart = paramName.find('[');
                if (arrayStart != String::npos)
                {
                    // if not the first array element then skip it and continue to the next uniform
                    if (paramName.compare(arrayStart, paramName.size() - 1, "[0]") != 0) continue;
                    paramName = paramName.substr(0, arrayStart);
                }

                // find out which params object this comes from
                bool foundSource = completeParamSource(paramName,
                        vertexConstantDefs, geometryConstantDefs, fragmentConstantDefs, newGLUniformReference);

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
}
}
