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
#include "OgreGLSLESGpuProgram.h"
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
	GLSLESLinkProgramManager::~GLSLESLinkProgramManager(void)
	{
		// iterate through map container and delete link programs
		for (LinkProgramIterator currentProgram = mLinkPrograms.begin();
			currentProgram != mLinkPrograms.end(); ++currentProgram)
		{
			OGRE_DELETE currentProgram->second;
		}
	}

	//-----------------------------------------------------------------------
	GLSLESLinkProgram* GLSLESLinkProgramManager::getActiveLinkProgram(void)
	{
		// If there is an active link program then return it
		if (mActiveLinkProgram)
			return mActiveLinkProgram;

		// No active link program so find one or make a new one
		// Is there an active key?
		uint64 activeKey = 0;

		if (mActiveVertexGpuProgram)
		{
			activeKey = static_cast<uint64>(mActiveVertexGpuProgram->getProgramID()) << 32;
		}
		if (mActiveFragmentGpuProgram)
		{
			activeKey += static_cast<uint64>(mActiveFragmentGpuProgram->getProgramID());
		}

		// Only return a link program object if a vertex or fragment program exist
		if (activeKey > 0)
		{
			// Find the key in the hash map
			LinkProgramIterator programFound = mLinkPrograms.find(activeKey);
			// Program object not found for key so need to create it
			if (programFound == mLinkPrograms.end())
			{
				mActiveLinkProgram = OGRE_NEW GLSLESLinkProgram(mActiveVertexGpuProgram,mActiveFragmentGpuProgram);
				mLinkPrograms[activeKey] = mActiveLinkProgram;
			}
			else
			{
				// Found a link program in map container so make it active
				mActiveLinkProgram = programFound->second;
			}

		}
		// Make the program object active
		if (mActiveLinkProgram) mActiveLinkProgram->activate();

		return mActiveLinkProgram;
	}

	//-----------------------------------------------------------------------
	GLSLESLinkProgram* GLSLESLinkProgramManager::getByProgram(GLSLESGpuProgram* gpuProgram)
	{
		for (LinkProgramIterator currentProgram = mLinkPrograms.begin();
			currentProgram != mLinkPrograms.end(); ++currentProgram)
		{
			GLSLESLinkProgram* prgm = currentProgram->second;
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
		for (LinkProgramIterator currentProgram = mLinkPrograms.begin();
			currentProgram != mLinkPrograms.end(); ++currentProgram)
		{
			GLSLESLinkProgram* prgm = currentProgram->second;
			if(prgm == linkProgram)
			{
				mLinkPrograms.erase(mLinkPrograms.find(currentProgram->first));
				OGRE_DELETE prgm;
				return true;				
			}
		}

		return false;
	}

	//-----------------------------------------------------------------------
	void GLSLESLinkProgramManager::destroyAllByProgram(GLSLESGpuProgram* gpuProgram)
	{
		std::vector<uint64> keysToErase;
		for (LinkProgramIterator currentProgram = mLinkPrograms.begin();
			currentProgram != mLinkPrograms.end(); ++currentProgram)
		{
			GLSLESLinkProgram* prgm = currentProgram->second;
			if(prgm->getVertexProgram() == gpuProgram || prgm->getFragmentProgram() == gpuProgram)
			{
				OGRE_DELETE prgm;
				keysToErase.push_back(currentProgram->first);
			}
		}
		
		for(size_t i = 0; i < keysToErase.size(); ++i)
		{
			mLinkPrograms.erase(mLinkPrograms.find(keysToErase[i]));
		}
	}

	//-----------------------------------------------------------------------
	void GLSLESLinkProgramManager::setActiveFragmentShader(GLSLESGpuProgram* fragmentGpuProgram)
	{
		if (fragmentGpuProgram != mActiveFragmentGpuProgram)
		{
			mActiveFragmentGpuProgram = fragmentGpuProgram;
			// ActiveLinkProgram is no longer valid
			mActiveLinkProgram = NULL;
		}
	}

	//-----------------------------------------------------------------------
	void GLSLESLinkProgramManager::setActiveVertexShader(GLSLESGpuProgram* vertexGpuProgram)
	{
		if (vertexGpuProgram != mActiveVertexGpuProgram)
		{
			mActiveVertexGpuProgram = vertexGpuProgram;
			// ActiveLinkProgram is no longer valid
			mActiveLinkProgram = NULL;
		}
	}
}
