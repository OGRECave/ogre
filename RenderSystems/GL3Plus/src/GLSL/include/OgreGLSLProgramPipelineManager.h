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
#ifndef __GLSLProgramPipelineManager_H__
#define __GLSLProgramPipelineManager_H__

#include "OgreGL3PlusPrerequisites.h"
#include "OgreSingleton.h"
#include "OgreGLSLProgramPipeline.h"
#include "OgreGLSLProgramManagerCommon.h"

namespace Ogre
{
	/** Ogre assumes that there are separate vertex and fragment programs to deal with but
     GLSL ES has one program pipeline object that represents the active vertex and fragment program objects
     during a rendering state.  GLSL vertex and fragment program objects are compiled separately
     and then attached to a program object and then the program pipeline object is linked.
     Since Ogre can only handle one vertex program stage and one fragment program stage being active
     in a pass, the GLSL ES Program Pipeline Manager does the same.  The GLSL ES Program Pipeline
     Manager acts as a state machine and activates a pipeline object based on the active
     vertex and fragment program.  Previously created pipeline objects are stored along with a unique
     key in a hash_map for quick retrieval the next time the pipeline object is required.
     */
    class _OgreGL3PlusExport GLSLProgramPipelineManager : public GLSLProgramManagerCommon, public Singleton<GLSLProgramPipelineManager>
    {
	private:

		typedef map<uint32, GLSLProgramPipeline*>::type ProgramPipelineMap;
		typedef ProgramPipelineMap::iterator ProgramPipelineIterator;

		/// Container holding previously created program pipeline objects 
		ProgramPipelineMap mProgramPipelines;

		/// Active objects defining the active rendering gpu state
		GLSLProgramPipeline* mActiveProgramPipeline;

	public:

		GLSLProgramPipelineManager(void);
		~GLSLProgramPipelineManager(void);

		/**
         Get the program object that links the two active program objects together.
         If a program pipeline object was not already created and linked a new one is created and linked.
         */
		GLSLProgramPipeline* getActiveProgramPipeline(void);

		/** Set the active vertex and fragment link programs for the next rendering state.
         The active program pipeline object will be cleared.
         Normally called from the GLSLGpuProgram::bindProgram and unbindProgram methods
         */
        void setActiveVertexLinkProgram(GLSLGpuProgram* vertexGpuProgram);
        void setActiveFragmentLinkProgram(GLSLGpuProgram* fragmentGpuProgram);
        void setActiveGeometryLinkProgram(GLSLGpuProgram* geometryGpuProgram);
        void setActiveTessDomainLinkProgram(GLSLGpuProgram* domainGpuProgram);
        void setActiveTessHullLinkProgram(GLSLGpuProgram* hullGpuProgram);
        void setActiveComputeLinkProgram(GLSLGpuProgram* computGpuProgram);

		static GLSLProgramPipelineManager& getSingleton(void);
        static GLSLProgramPipelineManager* getSingletonPtr(void);
    };
}

#endif
