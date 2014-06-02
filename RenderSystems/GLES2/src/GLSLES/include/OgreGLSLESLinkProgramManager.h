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
#ifndef __GLSLESLinkProgramManager_H__
#define __GLSLESLinkProgramManager_H__

#include "OgreGLES2Prerequisites.h"
#include "OgreSingleton.h"

#include "OgreGLSLESExtSupport.h"
#include "OgreGLSLESLinkProgram.h"
#include "OgreGLSLESProgramManagerCommon.h"

namespace Ogre {


    /** Ogre assumes that there are separate vertex and fragment programs to deal with but
        GLSL ES has one program object that represents the active vertex and fragment shader objects
        during a rendering state.  GLSL Vertex and fragment 
        shader objects are compiled separately and then attached to a program object and then the
        program object is linked.  Since Ogre can only handle one vertex program and one fragment
        program being active in a pass, the GLSL ES Link Program Manager does the same.  The GLSL ES Link
        program manager acts as a state machine and activates a program object based on the active
        vertex and fragment program.  Previously created program objects are stored along with a unique
        key in a hash_map for quick retrieval the next time the program object is required.
    */

    class _OgreGLES2Export GLSLESLinkProgramManager : public GLSLESProgramManagerCommon, public Singleton<GLSLESLinkProgramManager>
    {

    private:
    
        typedef map<uint64, GLSLESLinkProgram*>::type LinkProgramMap;
        typedef LinkProgramMap::iterator LinkProgramIterator;

        /// Container holding previously created program objects 
        LinkProgramMap mLinkPrograms; 

        /// Active objects defining the active rendering gpu state
        GLSLESLinkProgram* mActiveLinkProgram;

        typedef map<String, GLenum>::type StringToEnumMap;
        StringToEnumMap mTypeEnumMap;

    public:

        GLSLESLinkProgramManager(void);

        ~GLSLESLinkProgramManager(void);

        /**
            Get the program object that links the two active shader objects together
            if a program object was not already created and linked a new one is created and linked
        */
        GLSLESLinkProgram* getActiveLinkProgram(void);

		/**
			Get the linker program by a gpu program
		*/
		GLSLESLinkProgram* getByProgram(GLSLESGpuProgram* gpuProgram);

		/**
			Destroy and remove the linker program from the local cache
		*/
		bool destroyLinkProgram(GLSLESLinkProgram* linkProgram);

		/**
			Destroy all linker programs which referencing this gpu program
		*/
		void destroyAllByProgram(GLSLESGpuProgram* gpuProgram);

        /** Set the active fragment shader for the next rendering state.
            The active program object will be cleared.
            Normally called from the GLSLESGpuProgram::bindProgram and unbindProgram methods
        */
        void setActiveFragmentShader(GLSLESGpuProgram* fragmentGpuProgram);
        /** Set the active vertex shader for the next rendering state.
            The active program object will be cleared.
            Normally called from the GLSLESGpuProgram::bindProgram and unbindProgram methods
        */
        void setActiveVertexShader(GLSLESGpuProgram* vertexGpuProgram);

        static GLSLESLinkProgramManager& getSingleton(void);
        static GLSLESLinkProgramManager* getSingletonPtr(void);

    };

}

#endif // __GLSLESLinkProgramManager_H__
