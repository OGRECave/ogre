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

#include "OgreGLSLProgramPipelineManager.h"
#include "OgreGLSLGpuProgram.h"
#include "OgreGLSLProgram.h"

namespace Ogre
{
    //-----------------------------------------------------------------------
	template<> GLSLProgramPipelineManager* Singleton<GLSLProgramPipelineManager>::msSingleton = 0;
    
	//-----------------------------------------------------------------------
    GLSLProgramPipelineManager* GLSLProgramPipelineManager::getSingletonPtr(void)
    {
        return msSingleton;
    }
    
	//-----------------------------------------------------------------------
    GLSLProgramPipelineManager& GLSLProgramPipelineManager::getSingleton(void)
    {  
        assert( msSingleton );  return ( *msSingleton );  
    }

    GLSLProgramPipelineManager::GLSLProgramPipelineManager(void) :
        GLSLProgramManagerCommon(), mActiveProgramPipeline(NULL) { }

    GLSLProgramPipelineManager::~GLSLProgramPipelineManager(void)
    {
		// Iterate through map container and delete program pipelines
		for (ProgramPipelineIterator currentProgram = mProgramPipelines.begin();
             currentProgram != mProgramPipelines.end(); ++currentProgram)
		{
			delete currentProgram->second;
		}
    }

    //-----------------------------------------------------------------------
	void GLSLProgramPipelineManager::setActiveFragmentLinkProgram(GLSLGpuProgram* fragmentGpuProgram)
	{
		if (fragmentGpuProgram != mActiveFragmentGpuProgram)
		{
			mActiveFragmentGpuProgram = fragmentGpuProgram;
			// ActiveProgramPipeline is no longer valid
			mActiveProgramPipeline = NULL;
		}
	}
    
	//-----------------------------------------------------------------------
	void GLSLProgramPipelineManager::setActiveVertexLinkProgram(GLSLGpuProgram* vertexGpuProgram)
	{
		if (vertexGpuProgram != mActiveVertexGpuProgram)
		{
			mActiveVertexGpuProgram = vertexGpuProgram;
			// ActiveProgramPipeline is no longer valid
			mActiveProgramPipeline = NULL;
		}
	}

    void GLSLProgramPipelineManager::setActiveGeometryLinkProgram(GLSLGpuProgram* geometryGpuProgram)
	{
		if (geometryGpuProgram != mActiveGeometryGpuProgram)
		{
			mActiveGeometryGpuProgram = geometryGpuProgram;
			// ActiveProgramPipeline is no longer valid
			mActiveProgramPipeline = NULL;
		}
	}

    void GLSLProgramPipelineManager::setActiveTessDomainLinkProgram(GLSLGpuProgram* domainGpuProgram)
	{
		if (domainGpuProgram != mActiveDomainGpuProgram)
		{
			mActiveDomainGpuProgram = domainGpuProgram;
			// ActiveProgramPipeline is no longer valid
			mActiveProgramPipeline = NULL;
		}
	}

    void GLSLProgramPipelineManager::setActiveTessHullLinkProgram(GLSLGpuProgram* hullGpuProgram)
	{
		if (hullGpuProgram != mActiveHullGpuProgram)
		{
			mActiveHullGpuProgram = hullGpuProgram;
			// ActiveProgramPipeline is no longer valid
			mActiveProgramPipeline = NULL;
		}
	}

    void GLSLProgramPipelineManager::setActiveComputeLinkProgram(GLSLGpuProgram* computeGpuProgram)
	{
		if (computeGpuProgram != mActiveComputeGpuProgram)
		{
			mActiveComputeGpuProgram = computeGpuProgram;
			// ActiveProgramPipeline is no longer valid
			mActiveProgramPipeline = NULL;
		}
	}

    //-----------------------------------------------------------------------
	GLSLProgramPipeline* GLSLProgramPipelineManager::getActiveProgramPipeline(void)
	{
		// If there is an active link program then return it
		if (mActiveProgramPipeline)
			return mActiveProgramPipeline;

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
			ProgramPipelineIterator programFound = mProgramPipelines.find(activeKey);
			// Program object not found for key so need to create it
			if (programFound == mProgramPipelines.end())
			{
				mActiveProgramPipeline = new GLSLProgramPipeline(mActiveVertexGpuProgram, mActiveGeometryGpuProgram,
                                                                 mActiveFragmentGpuProgram, mActiveHullGpuProgram,
                                                                 mActiveDomainGpuProgram, mActiveComputeGpuProgram);
				mProgramPipelines[activeKey] = mActiveProgramPipeline;
			}
			else
			{
				// Found a link program in map container so make it active
				mActiveProgramPipeline = programFound->second;
			}
		}
		// Make the program object active
		if (mActiveProgramPipeline)
            mActiveProgramPipeline->activate();
        
		return mActiveProgramPipeline;
	}

}
