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
#ifndef __GLSLProgramManager_H__
#define __GLSLProgramManager_H__

#include "OgreGL3PlusPrerequisites.h"
#include "OgreGLSLProgramManagerCommon.h"
#include "OgreGLSLShader.h"
#include "OgreGLSLProgram.h"
#include "OgreGLSLExtSupport.h"

namespace Ogre {

    /** Ogre assumes that there are separate vertex and fragment
        programs to deal with but GLSL has one program object that
        represents the active vertex and fragment shader objects
        during a rendering state.  GLSL Vertex and fragment shader
        objects are compiled separately and then attached to a program
        object and then the program object is linked.  Since Ogre can
        only handle one vertex program and one fragment program being
        active in a pass, the GLSL Link Program Manager does the same.
        The GLSL Link program manager acts as a state machine and
        activates a program object based on the active vertex and
        fragment program.  Previously created program objects are
        stored along with a unique key in a hash_map for quick
        retrieval the next time the program object is required.
    */
    class _OgreGL3PlusExport GLSLProgramManager : public GLSLProgramManagerCommon, public Singleton<GLSLProgramManager>
    {
    protected:
        /// active objects defining the active rendering gpu state
        GLSLProgram* mActiveProgram;

        GL3PlusRenderSystem* mRenderSystem;

        /** Find the data source definition for a given uniform name
            and reference. Return true if found and pair the reference
            with its data source. */
        static bool findUniformDataSource(
            const String& paramName,
            const GpuConstantDefinitionMap* (&constantDefs)[6],
            GLUniformReference& refToUpdate);
        /** Find the data source definition for a given atomic counter
            uniform name and reference. Return true if found and pair
            the reference with its data source. */
        static bool findAtomicCounterDataSource(
            const String& paramName,
            const GpuConstantDefinitionMap* (&constantDefs)[6],
            GLAtomicCounterReference& refToUpdate);
    public:

        GLSLProgramManager(GL3PlusRenderSystem* renderSystem);
        ~GLSLProgramManager();

        /** Get the program object that links the two active shader
            objects together if a program object was not already
            created and linked a new one is created and linked
            @note this method does NOT link seperable programs.
        */
        GLSLProgram* getActiveProgram(void);

        /** Set the shader for the next rendering state.
            The active program object will be cleared.  Normally
            called from the GLRenderSystem::bindGpuProgram and
            unbindProgram methods
        */
        void setActiveShader(GpuProgramType type, GLSLShader* shader);

        /** Populate a list of uniforms based on an OpenGL program object.
        */
        void extractUniformsFromProgram(
            GLuint programObject,
            const GpuConstantDefinitionMap* (&constantDefs)[6],
            GLUniformReferenceList& uniformList,
            GLAtomicCounterReferenceList& counterList,
            GLCounterBufferList& counterBufferList);

        GL3PlusStateCacheManager* getStateCacheManager();

        static GLSLProgramManager& getSingleton(void);
        static GLSLProgramManager* getSingletonPtr(void);
    };

}

#endif // __GLSLProgramManager_H__
