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

#include "OgreGLSLESLinkProgramManager.h"
#include "OgreGLSLESProgram.h"
#include "OgreStringConverter.h"
#include "OgreGLSLESProgram.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    template<> GLSLESLinkProgramManager* Singleton<GLSLESLinkProgramManager>::msSingleton = 0;

    //-----------------------------------------------------------------------
    GLSLESLinkProgramManager* GLSLESLinkProgramManager::getSingletonPtr(void)
    {
        return msSingleton;
    }

    //-----------------------------------------------------------------------
    GLSLESLinkProgramManager& GLSLESLinkProgramManager::getSingleton(void)
    {  
        assert( msSingleton );  return ( *msSingleton );  
    }

    //-----------------------------------------------------------------------
    GLSLESLinkProgramManager::GLSLESLinkProgramManager(void) :
        GLSLESProgramManagerCommon(), mActiveLinkProgram(NULL) { }

    //-----------------------------------------------------------------------
    GLSLESLinkProgramManager::~GLSLESLinkProgramManager(void) {}

    //-----------------------------------------------------------------------
    GLSLESLinkProgram* GLSLESLinkProgramManager::getActiveLinkProgram(void)
    {
        // If there is an active link program then return it
        if (mActiveLinkProgram)
            return mActiveLinkProgram;

        // No active link program so find one or make a new one
        // Is there an active key?
        uint32 activeKey = 0;
        if (mActiveVertexGpuProgram)
        {
            activeKey = HashCombine(mActiveVertexGpuProgram->getShaderID(), activeKey);
        }
        if (mActiveFragmentGpuProgram)
        {
            activeKey = HashCombine(mActiveFragmentGpuProgram->getShaderID(), activeKey);
        }

        // Only return a link program object if a vertex or fragment program exist
        if (activeKey > 0)
        {
            // Find the key in the hash map
            ProgramIterator programFound = mPrograms.find(activeKey);
            // Program object not found for key so need to create it
            if (programFound == mPrograms.end())
            {
                mActiveLinkProgram = new GLSLESLinkProgram(mActiveVertexGpuProgram,mActiveFragmentGpuProgram);
                mPrograms[activeKey] = mActiveLinkProgram;
            }
            else
            {
                // Found a link program in map container so make it active
                mActiveLinkProgram = static_cast<GLSLESLinkProgram*>(programFound->second);
            }

        }
        // Make the program object active
        if (mActiveLinkProgram) mActiveLinkProgram->activate();

        return mActiveLinkProgram;
	}

	//-----------------------------------------------------------------------
	GLSLESLinkProgram* GLSLESLinkProgramManager::getByProgram(GLSLESProgram* gpuProgram)
	{
		for (ProgramIterator currentProgram = mPrograms.begin();
			currentProgram != mPrograms.end(); ++currentProgram)
		{
			GLSLESLinkProgram* prgm = static_cast<GLSLESLinkProgram*>(currentProgram->second);
			if(prgm->getVertexProgram() == gpuProgram || prgm->getFragmentProgram() == gpuProgram)
			{
				return prgm;
			}
		}

		return NULL;
	}

	//-----------------------------------------------------------------------
	bool GLSLESLinkProgramManager::destroyLinkProgram(GLSLESLinkProgram* linkProgram)
	{
		for (ProgramIterator currentProgram = mPrograms.begin();
			currentProgram != mPrograms.end(); ++currentProgram)
		{
			GLSLESLinkProgram* prgm = static_cast<GLSLESLinkProgram*>(currentProgram->second);
			if(prgm == linkProgram)
			{
				mPrograms.erase(mPrograms.find(currentProgram->first));
				OGRE_DELETE prgm;
				return true;				
			}
		}

		return false;
	}

	//-----------------------------------------------------------------------
	void GLSLESLinkProgramManager::destroyAllByProgram(GLSLESProgram* gpuProgram)
	{
		std::vector<uint32> keysToErase;
		for (ProgramIterator currentProgram = mPrograms.begin();
			currentProgram != mPrograms.end(); ++currentProgram)
		{
			GLSLESLinkProgram* prgm = static_cast<GLSLESLinkProgram*>(currentProgram->second);
			if(prgm->getVertexProgram() == gpuProgram || prgm->getFragmentProgram() == gpuProgram)
			{
				OGRE_DELETE prgm;
				keysToErase.push_back(currentProgram->first);
			}
		}
		
		for(size_t i = 0; i < keysToErase.size(); ++i)
		{
			mPrograms.erase(mPrograms.find(keysToErase[i]));
		}
    }

    //-----------------------------------------------------------------------
    void GLSLESLinkProgramManager::setActiveFragmentShader(GLSLESProgram* fragmentGpuProgram)
    {
        if (fragmentGpuProgram != mActiveFragmentGpuProgram)
        {
            mActiveFragmentGpuProgram = fragmentGpuProgram;
            // ActiveLinkProgram is no longer valid
            mActiveLinkProgram = NULL;
        }
    }

    //-----------------------------------------------------------------------
    void GLSLESLinkProgramManager::setActiveVertexShader(GLSLESProgram* vertexGpuProgram)
    {
        if (vertexGpuProgram != mActiveVertexGpuProgram)
        {
            mActiveVertexGpuProgram = vertexGpuProgram;
            // ActiveLinkProgram is no longer valid
            mActiveLinkProgram = NULL;
        }
    }
}
