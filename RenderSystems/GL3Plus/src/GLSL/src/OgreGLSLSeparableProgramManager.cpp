/*
  -----------------------------------------------------------------------------
  This source file is part of OGRE
  (Object-oriented Graphics Rendering Engine)
  For the latest info, see http://www.ogre3d.org/

  Copyright (c) 2000-2013 Torus Knot Software Ltd

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
#include "OgreGLSLGpuProgram.h"
#include "OgreGLSLShader.h"

namespace Ogre
{
    //-----------------------------------------------------------------------
    template<> GLSLSeparableProgramManager* Singleton<GLSLSeparableProgramManager>::msSingleton = 0;

    //-----------------------------------------------------------------------
    GLSLSeparableProgramManager* GLSLSeparableProgramManager::getSingletonPtr(void)
    {
        return msSingleton;
    }

    //-----------------------------------------------------------------------
    GLSLSeparableProgramManager& GLSLSeparableProgramManager::getSingleton(void)
    {
        assert( msSingleton );  return ( *msSingleton );
    }

    GLSLSeparableProgramManager::GLSLSeparableProgramManager(void) :
        GLSLProgramManager(), mActiveSeparableProgram(NULL) { }

    GLSLSeparableProgramManager::~GLSLSeparableProgramManager(void)
    {
        // Iterate through map container and delete program pipelines
        for (SeparableProgramIterator currentProgram = mSeparablePrograms.begin();
             currentProgram != mSeparablePrograms.end(); ++currentProgram)
        {
            delete currentProgram->second;
        }
    }

    //-----------------------------------------------------------------------
    void GLSLSeparableProgramManager::setActiveFragmentShader(GLSLGpuProgram* fragmentGpuProgram)
    {
        if (fragmentGpuProgram != mActiveFragmentGpuProgram)
        {
            mActiveFragmentGpuProgram = fragmentGpuProgram;
            // ActiveSeparableProgram is no longer valid
            mActiveSeparableProgram = NULL;
        }
    }

    //-----------------------------------------------------------------------
    void GLSLSeparableProgramManager::setActiveVertexShader(GLSLGpuProgram* vertexGpuProgram)
    {
        if (vertexGpuProgram != mActiveVertexGpuProgram)
        {
            mActiveVertexGpuProgram = vertexGpuProgram;
            // ActiveSeparableProgram is no longer valid
            mActiveSeparableProgram = NULL;
        }
    }

    void GLSLSeparableProgramManager::setActiveGeometryShader(GLSLGpuProgram* geometryGpuProgram)
    {
        if (geometryGpuProgram != mActiveGeometryGpuProgram)
        {
            mActiveGeometryGpuProgram = geometryGpuProgram;
            // ActiveSeparableProgram is no longer valid
            mActiveSeparableProgram = NULL;
        }
    }

    void GLSLSeparableProgramManager::setActiveTessDomainShader(GLSLGpuProgram* domainGpuProgram)
    {
        if (domainGpuProgram != mActiveDomainGpuProgram)
        {
            mActiveDomainGpuProgram = domainGpuProgram;
            // ActiveSeparableProgram is no longer valid
            mActiveSeparableProgram = NULL;
        }
    }

    void GLSLSeparableProgramManager::setActiveTessHullShader(GLSLGpuProgram* hullGpuProgram)
    {
        if (hullGpuProgram != mActiveHullGpuProgram)
        {
            mActiveHullGpuProgram = hullGpuProgram;
            // ActiveSeparableProgram is no longer valid
            mActiveSeparableProgram = NULL;
        }
    }

    void GLSLSeparableProgramManager::setActiveComputeShader(GLSLGpuProgram* computeGpuProgram)
    {
        if (computeGpuProgram != mActiveComputeGpuProgram)
        {
            mActiveComputeGpuProgram = computeGpuProgram;
            // ActiveSeparableProgram is no longer valid
            mActiveSeparableProgram = NULL;
        }
    }

    GLSLSeparableProgram* GLSLSeparableProgramManager::getCurrentSeparableProgram(void)
    {
        // If there is an active link program then return it
        if (mActiveSeparableProgram)
            return mActiveSeparableProgram;

        // No active link program so find one or make a new one
        // Is there an active key?
        uint32 activeKey = 0;
        GLuint progID = 0;
        if (mActiveVertexGpuProgram)
        {
            progID = mActiveVertexGpuProgram->getProgramID();
            activeKey = FastHash((const char *)(&progID), sizeof(GLuint), activeKey);
        }
        if (mActiveFragmentGpuProgram)
        {
            progID = mActiveFragmentGpuProgram->getProgramID();
            activeKey = FastHash((const char *)(&progID), sizeof(GLuint), activeKey);
        }
        if (mActiveGeometryGpuProgram)
        {
            progID = mActiveGeometryGpuProgram->getProgramID();
            activeKey = FastHash((const char *)(&progID), sizeof(GLuint), activeKey);
        }
        if (mActiveDomainGpuProgram)
        {
            progID = mActiveDomainGpuProgram->getProgramID();
            activeKey = FastHash((const char *)(&progID), sizeof(GLuint), activeKey);
        }
        if (mActiveHullGpuProgram)
        {
            progID = mActiveHullGpuProgram->getProgramID();
            activeKey = FastHash((const char *)(&progID), sizeof(GLuint), activeKey);
        }
        if (mActiveComputeGpuProgram)
        {
            progID = mActiveComputeGpuProgram->getProgramID();
            activeKey = FastHash((const char *)(&progID), sizeof(GLuint), activeKey);
        }

        // Only return a program pipeline object if a vertex or fragment stage exist
        if (activeKey > 0)
        {
            // Find the key in the hash map
            SeparableProgramIterator programFound = mSeparablePrograms.find(activeKey);
            // Program object not found for key so need to create it
            if (programFound == mSeparablePrograms.end())
            {
                mActiveSeparableProgram = new GLSLSeparableProgram(mActiveVertexGpuProgram, mActiveGeometryGpuProgram,
                                                                 mActiveFragmentGpuProgram, mActiveHullGpuProgram,
                                                                 mActiveDomainGpuProgram, mActiveComputeGpuProgram);
                mSeparablePrograms[activeKey] = mActiveSeparableProgram;
            }
            else
            {
                // Found a link program in map container so make it active
                mActiveSeparableProgram = programFound->second;
            }
        }

        return mActiveSeparableProgram;
    }
    
    GLSLSeparableProgram* GLSLSeparableProgramManager::getActiveSeparableProgram(void)
    {
        getCurrentSeparableProgram();

        // Make the program object active
        if (mActiveSeparableProgram)
            mActiveSeparableProgram->activate();

        return mActiveSeparableProgram;
    }

}
