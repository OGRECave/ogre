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

#include "OgreGL3PlusPrerequisites.h"

#include "OgreGLSLMonolithicProgramManager.h"
#include "OgreGLSLShader.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreGLSLProgram.h"
#include "OgreGpuProgramManager.h"
#include "OgreHardwareBufferManager.h"

namespace Ogre {


    template<> GLSLMonolithicProgramManager* Singleton<GLSLMonolithicProgramManager>::msSingleton = 0;


    GLSLMonolithicProgramManager* GLSLMonolithicProgramManager::getSingletonPtr(void)
    {
        return msSingleton;
    }


    GLSLMonolithicProgramManager& GLSLMonolithicProgramManager::getSingleton(void)
    {
        assert(msSingleton);  
        return (*msSingleton);
    }


    GLSLMonolithicProgramManager::GLSLMonolithicProgramManager(GL3PlusRenderSystem* renderSystem) :
        GLSLProgramManager(renderSystem),
        mActiveMonolithicProgram(NULL)
    {
    }


    GLSLMonolithicProgramManager::~GLSLMonolithicProgramManager(void) {}


    GLSLMonolithicProgram* GLSLMonolithicProgramManager::getActiveMonolithicProgram(void)
    {
        // If there is an active link program then return it.
        if (mActiveMonolithicProgram)
            return mActiveMonolithicProgram;

        // No active link program so find one or make a new one.
        // Is there an active key?
        uint32 activeKey = 0;
        if (mActiveVertexShader)
        {
            activeKey = HashCombine(mActiveVertexShader->getShaderID(), activeKey);
        }
        if (mActiveDomainShader)
        {
            activeKey = HashCombine(mActiveDomainShader->getShaderID(), activeKey);
        }
        if (mActiveHullShader)
        {
            activeKey = HashCombine(mActiveHullShader->getShaderID(), activeKey);
        }
        if (mActiveGeometryShader)
        {
            activeKey = HashCombine(mActiveGeometryShader->getShaderID(), activeKey);
        }
        if (mActiveFragmentShader)
        {
            activeKey = HashCombine(mActiveFragmentShader->getShaderID(), activeKey);
        }
        if (mActiveComputeShader)
        {
            activeKey = HashCombine(mActiveComputeShader->getShaderID(), activeKey);
        }

        // Only return a link program object if a program exists.
        if (activeKey > 0)
        {
            // Find the key in the hash map.
            ProgramIterator programFound = mPrograms.find(activeKey);
            // Program object not found for key so need to create it.
            if (programFound == mPrograms.end())
            {
                mActiveMonolithicProgram = new GLSLMonolithicProgram(
                    mActiveVertexShader,
                    mActiveHullShader,
                    mActiveDomainShader,
                    mActiveGeometryShader,
                    mActiveFragmentShader,
                    mActiveComputeShader);
                mPrograms[activeKey] = mActiveMonolithicProgram;
            }
            else
            {
                // Found a link program in map container so make it active.
                mActiveMonolithicProgram = static_cast<GLSLMonolithicProgram*>(programFound->second);
            }
        }
        // Make the program object active.
        if (mActiveMonolithicProgram)
            mActiveMonolithicProgram->activate();

        return mActiveMonolithicProgram;
    }


    void GLSLMonolithicProgramManager::setActiveFragmentShader(GLSLShader* fragmentShader)
    {
        if (fragmentShader != mActiveFragmentShader)
        {
            mActiveFragmentShader = fragmentShader;
            // ActiveMonolithicProgram is no longer valid
            mActiveMonolithicProgram = NULL;
        }
    }


    void GLSLMonolithicProgramManager::setActiveVertexShader(GLSLShader* vertexShader)
    {
        if (vertexShader != mActiveVertexShader)
        {
            mActiveVertexShader = vertexShader;
            // ActiveMonolithicProgram is no longer valid
            mActiveMonolithicProgram = NULL;
        }
    }


    void GLSLMonolithicProgramManager::setActiveGeometryShader(GLSLShader* geometryShader)
    {
        if (geometryShader != mActiveGeometryShader)
        {
            mActiveGeometryShader = geometryShader;
            // ActiveMonolithicProgram is no longer valid
            mActiveMonolithicProgram = NULL;
        }
    }


    void GLSLMonolithicProgramManager::setActiveHullShader(GLSLShader* hullShader)
    {
        if (hullShader != mActiveHullShader)
        {
            mActiveHullShader = hullShader;
            // ActiveMonolithicProgram is no longer valid
            mActiveMonolithicProgram = NULL;
        }
    }


    void GLSLMonolithicProgramManager::setActiveDomainShader(GLSLShader* domainShader)
    {
        if (domainShader != mActiveDomainShader)
        {
            mActiveDomainShader = domainShader;
            // ActiveMonolithicProgram is no longer valid
            mActiveMonolithicProgram = NULL;
        }
    }


    void GLSLMonolithicProgramManager::setActiveComputeShader(GLSLShader* computeShader)
    {
        if (computeShader != mActiveComputeShader)
        {
            mActiveComputeShader = computeShader;
            // ActiveMonolithicProgram is no longer valid
            mActiveMonolithicProgram = NULL;
        }
    }


}
