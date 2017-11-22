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

#include "OgreGLSLSeparableProgramManager.h"
#include "OgreGLSLShader.h"
#include "OgreGLSLShader.h"

namespace Ogre
{
    template<> GLSLSeparableProgramManager* Singleton<GLSLSeparableProgramManager>::msSingleton = 0;


    GLSLSeparableProgramManager* GLSLSeparableProgramManager::getSingletonPtr(void)
    {
        return msSingleton;
    }


    GLSLSeparableProgramManager& GLSLSeparableProgramManager::getSingleton(void)
    {
        assert( msSingleton );  return ( *msSingleton );
    }


    GLSLSeparableProgramManager::GLSLSeparableProgramManager(GL3PlusRenderSystem* renderSystem) :
        GLSLProgramManager(renderSystem), mActiveSeparableProgram(NULL) { }


    GLSLSeparableProgramManager::~GLSLSeparableProgramManager(void) {}


    void GLSLSeparableProgramManager::setActiveFragmentShader(GLSLShader* fragmentShader)
    {
        if (fragmentShader != mActiveFragmentShader)
        {
            mActiveFragmentShader = fragmentShader;
            // ActiveSeparableProgram is no longer valid.
            mActiveSeparableProgram = NULL;
        }
    }


    void GLSLSeparableProgramManager::setActiveVertexShader(GLSLShader* vertexShader)
    {
        if (vertexShader != mActiveVertexShader)
        {
            mActiveVertexShader = vertexShader;
            // ActiveSeparableProgram is no longer valid.
            mActiveSeparableProgram = NULL;
        }
    }


    void GLSLSeparableProgramManager::setActiveGeometryShader(GLSLShader* geometryShader)
    {
        if (geometryShader != mActiveGeometryShader)
        {
            mActiveGeometryShader = geometryShader;
            // ActiveSeparableProgram is no longer valid.
            mActiveSeparableProgram = NULL;
        }
    }


    void GLSLSeparableProgramManager::setActiveTessDomainShader(GLSLShader* domainShader)
    {
        if (domainShader != mActiveDomainShader)
        {
            mActiveDomainShader = domainShader;
            // ActiveSeparableProgram is no longer valid.
            mActiveSeparableProgram = NULL;
        }
    }


    void GLSLSeparableProgramManager::setActiveTessHullShader(GLSLShader* hullShader)
    {
        if (hullShader != mActiveHullShader)
        {
            mActiveHullShader = hullShader;
            // ActiveSeparableProgram is no longer valid.
            mActiveSeparableProgram = NULL;
        }
    }


    void GLSLSeparableProgramManager::setActiveComputeShader(GLSLShader* computeShader)
    {
        if (computeShader != mActiveComputeShader)
        {
            mActiveComputeShader = computeShader;
            // ActiveSeparableProgram is no longer valid.
            mActiveSeparableProgram = NULL;
        }
    }


    GLSLSeparableProgram* GLSLSeparableProgramManager::getCurrentSeparableProgram(void)
    {
        // If there is an active link program then return it.
        if (mActiveSeparableProgram)
            return mActiveSeparableProgram;

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

        // Only return a program pipeline object if a vertex or fragment stage exist.
        if (activeKey > 0)
        {
            // Find the key in the hash map.
            ProgramIterator programFound = mPrograms.find(activeKey);
            // Program object not found for key so need to create it.
            if (programFound == mPrograms.end())
            {
                mActiveSeparableProgram = new GLSLSeparableProgram(
                    mActiveVertexShader,
                    mActiveHullShader,
                    mActiveDomainShader,
                    mActiveGeometryShader,
                    mActiveFragmentShader,
                    mActiveComputeShader);
                mPrograms[activeKey] = mActiveSeparableProgram;
            }
            else
            {
                // Found a link program in map container so make it active.
                mActiveSeparableProgram = static_cast<GLSLSeparableProgram*>(programFound->second);
            }
        }

        // Make the program object active
        if (mActiveSeparableProgram)
            mActiveSeparableProgram->activate();

        return mActiveSeparableProgram;
    }
}
