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


    GLSLSeparableProgramManager::GLSLSeparableProgramManager(const GL3PlusSupport& support) :
        GLSLProgramManager(support), mActiveSeparableProgram(NULL) { }


    GLSLSeparableProgramManager::~GLSLSeparableProgramManager(void)
    {
        // Iterate through map container and delete program pipelines.
        for (SeparableProgramIterator currentProgram = mSeparablePrograms.begin();
             currentProgram != mSeparablePrograms.end(); ++currentProgram)
        {
            delete currentProgram->second;
        }
    }


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
        GLuint shaderID = 0;
        if (mActiveVertexShader)
        {
            shaderID = mActiveVertexShader->getShaderID();
            activeKey = FastHash((const char *)(&shaderID), sizeof(GLuint), activeKey);
        }
        if (mActiveDomainShader)
        {
            shaderID = mActiveDomainShader->getShaderID();
            activeKey = FastHash((const char *)(&shaderID), sizeof(GLuint), activeKey);
        }
        if (mActiveHullShader)
        {
            shaderID = mActiveHullShader->getShaderID();
            activeKey = FastHash((const char *)(&shaderID), sizeof(GLuint), activeKey);
        }
        if (mActiveGeometryShader)
        {
            shaderID = mActiveGeometryShader->getShaderID();
            activeKey = FastHash((const char *)(&shaderID), sizeof(GLuint), activeKey);
        }
        else if (mActiveFragmentShader)
        {
            shaderID = mActiveFragmentShader->getShaderID();
            activeKey = FastHash((const char *)(&shaderID), sizeof(GLuint), activeKey);
        }
        if (mActiveComputeShader)
        {
            shaderID = mActiveComputeShader->getShaderID();
            activeKey = FastHash((const char *)(&shaderID), sizeof(GLuint), activeKey);
        }

        // Only return a program pipeline object if a vertex or fragment stage exist.
        if (activeKey > 0)
        {
            // Find the key in the hash map.
            SeparableProgramIterator programFound = mSeparablePrograms.find(activeKey);
            // Program object not found for key so need to create it.
            if (programFound == mSeparablePrograms.end())
            {
                mActiveSeparableProgram = new GLSLSeparableProgram(
                    mActiveVertexShader,
                    mActiveHullShader,
                    mActiveDomainShader,
                    mActiveGeometryShader,
                    mActiveFragmentShader,
                    mActiveComputeShader);
                mSeparablePrograms[activeKey] = mActiveSeparableProgram;
            }
            else
            {
                // Found a link program in map container so make it active.
                mActiveSeparableProgram = programFound->second;
            }
        }

        return mActiveSeparableProgram;
    }
}
