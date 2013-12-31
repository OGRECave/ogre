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
#ifndef __GLSLLinkProgramManager_H__
#define __GLSLLinkProgramManager_H__

#include "OgreGL3PlusPrerequisites.h"
#include "OgreSingleton.h"

#include "OgreGLSLExtSupport.h"
#include "OgreGLSLLinkProgram.h"
#include "OgreGLSLProgramManagerCommon.h"

namespace Ogre {


	/** Ogre assumes that there are separate vertex and fragment programs to deal with but
		GLSL has one program object that represents the active vertex and fragment shader objects
		during a rendering state.  GLSL Vertex and fragment 
		shader objects are compiled separately and then attached to a program object and then the
		program object is linked.  Since Ogre can only handle one vertex program and one fragment
		program being active in a pass, the GLSL Link Program Manager does the same.  The GLSL Link
		program manager acts as a state machine and activates a program object based on the active
		vertex and fragment program.  Previously created program objects are stored along with a unique
		key in a hash_map for quick retrieval the next time the program object is required.

	*/

	class _OgreGL3PlusExport GLSLLinkProgramManager : public GLSLProgramManagerCommon, public Singleton<GLSLLinkProgramManager>
	{

	private:
	
		typedef map<uint32, GLSLLinkProgram*>::type LinkProgramMap;
		typedef LinkProgramMap::iterator LinkProgramIterator;

		/// container holding previously created program objects 
		LinkProgramMap mLinkPrograms; 

		/// active objects defining the active rendering gpu state
		GLSLLinkProgram* mActiveLinkProgram;

		typedef map<String, GLenum>::type StringToEnumMap;
		StringToEnumMap mTypeEnumMap;

	public:

		GLSLLinkProgramManager(void);

		~GLSLLinkProgramManager(void);

		/**
			Get the program object that links the two active shader objects together
			if a program object was not already created and linked a new one is created and linked
		*/
		GLSLLinkProgram* getActiveLinkProgram(void);

		/** Set the active fragment shader for the next rendering state.
			The active program object will be cleared.
			Normally called from the GLSLGpuProgram::bindProgram and unbindProgram methods
		*/
		void setActiveFragmentShader(GLSLGpuProgram* fragmentGpuProgram);
		/** Set the active geometry shader for the next rendering state.
			The active program object will be cleared.
			Normally called from the GLSLGpuProgram::bindProgram and unbindProgram methods
		*/
		void setActiveGeometryShader(GLSLGpuProgram* geometryGpuProgram);
		/** Set the active vertex shader for the next rendering state.
			The active program object will be cleared.
			Normally called from the GLSLGpuProgram::bindProgram and unbindProgram methods
		*/
		void setActiveVertexShader(GLSLGpuProgram* vertexGpuProgram);
		/** Set the active hull(control) shader for the next rendering state.
			The active program object will be cleared.
			Normally called from the GLSLGpuProgram::bindProgram and unbindProgram methods
		*/
		void setActiveHullShader(GLSLGpuProgram* hullGpuProgram);
		/** Set the active domain(evaluation) shader for the next rendering state.
			The active program object will be cleared.
			Normally called from the GLSLGpuProgram::bindProgram and unbindProgram methods
		*/
		void setActiveDomainShader(GLSLGpuProgram* domainGpuProgram);
   		/** Set the active compute shader for the next rendering state.
			The active program object will be cleared.
			Normally called from the GLSLGpuProgram::bindProgram and unbindProgram methods
		*/
		void setActiveComputeShader(GLSLGpuProgram* computeGpuProgram);

		static GLSLLinkProgramManager& getSingleton(void);
        static GLSLLinkProgramManager* getSingletonPtr(void);
	};

}

#endif // __GLSLLinkProgramManager_H__
