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
    GLSLLinkProgramManager::GLSLLinkProgramManager(void) {}

    //-----------------------------------------------------------------------
    GLSLLinkProgramManager::~GLSLLinkProgramManager(void) {}

    //-----------------------------------------------------------------------
    GLSLLinkProgram* GLSLLinkProgramManager::getActiveLinkProgram(void)
    {
        // if there is an active link program then return it
        if (mActiveProgram)
            return static_cast<GLSLLinkProgram*>(mActiveProgram);;

        // no active link program so find one or make a new one
        // is there an active key?
        uint32 activeKey = 0;
        for(auto shader : mActiveShader)
        {
            if(!shader) continue;
            activeKey = HashCombine(activeKey, shader->getShaderID());
        }

        // only return a link program object if a vertex, geometry or fragment program exist
        if (activeKey > 0)
        {
            // find the key in the hash map
            ProgramIterator programFound = mPrograms.find(activeKey);
            // program object not found for key so need to create it
            if (programFound == mPrograms.end())
            {
                mActiveProgram = new GLSLLinkProgram(mActiveShader);
                mPrograms[activeKey] = mActiveProgram;
            }
            else
            {
                // found a link program in map container so make it active
                mActiveProgram = static_cast<GLSLLinkProgram*>(programFound->second);
            }

        }
        // make the program object active
        if (mActiveProgram) mActiveProgram->activate();

        return static_cast<GLSLLinkProgram*>(mActiveProgram);;

    }
    //---------------------------------------------------------------------
    void GLSLLinkProgramManager::extractUniforms(uint programObject,
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
        glGetObjectParameterivARB((GLhandleARB)programObject, GL_OBJECT_ACTIVE_UNIFORMS_ARB,
            &uniformCount);

        const GpuConstantDefinitionMap* params[6] = { vertexConstantDefs, fragmentConstantDefs, geometryConstantDefs };

        // Loop over each of the active uniforms, and add them to the reference container
        // only do this for user defined uniforms, ignore built in gl state uniforms
        for (int index = 0; index < uniformCount; index++)
        {
            GLint numActiveArrayElements = 0;
            GLenum glType;
            glGetActiveUniformARB((GLhandleARB)programObject, index, BUFFERSIZE, NULL,
                &numActiveArrayElements, &glType, uniformName);
            newGLUniformReference.mLocation = glGetUniformLocationARB((GLhandleARB)programObject, uniformName);

            if(!validateParam(uniformName, numActiveArrayElements, params, newGLUniformReference))
                continue;
            list.push_back(newGLUniformReference);
        }

    }
}
}
