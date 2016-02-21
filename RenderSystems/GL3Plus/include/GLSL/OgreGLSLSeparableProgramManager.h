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
#ifndef __GLSLSeparableProgramManager_H__
#define __GLSLSeparableProgramManager_H__

#include "OgreGL3PlusPrerequisites.h"
#include "OgreSingleton.h"
#include "OgreGLSLSeparableProgram.h"
#include "OgreGLSLProgramManager.h"

namespace Ogre
{
    /** Ogre assumes that there are separate vertex and fragment
        programs to deal with but GLSL has one program pipeline object
        that represents the active vertex and fragment program objects
        during a rendering state.  GLSL vertex and fragment program
        objects are compiled separately and then attached to a program
        object and then the program pipeline object is linked.  Since
        Ogre can only handle one vertex program stage and one fragment
        program stage being active in a pass, the GLSL Program
        Pipeline Manager does the same.  The GLSL Program Pipeline
        Manager acts as a state machine and activates a pipeline
        object based on the active vertex and fragment program.
        Previously created pipeline objects are stored along with a
        unique key in a hash map for quick retrieval the next time the
        pipeline object is required.
    */
    class _OgreGL3PlusExport GLSLSeparableProgramManager : public GLSLProgramManager, public Singleton<GLSLSeparableProgramManager>
    {
    private:

        typedef map<uint32, GLSLSeparableProgram*>::type SeparableProgramMap;
        typedef SeparableProgramMap::iterator SeparableProgramIterator;

        /// Container holding previously created program pipeline objects
        SeparableProgramMap mSeparablePrograms;

        /// Active objects defining the active rendering gpu state
        GLSLSeparableProgram* mActiveSeparableProgram;

    public:

        GLSLSeparableProgramManager(const GL3PlusSupport& support);
        ~GLSLSeparableProgramManager(void);

        /** Get the program pipeline that combines the current program
            objects.  If the program pipeline object was not already
            created a new one is created.  Note that this method does
            NOT link the program.
        */
        GLSLSeparableProgram* getCurrentSeparableProgram(void);

        /** Set the active link programs for the next rendering state.
            The active program pipeline object will be cleared.
            Normally called from the GLSLShader::bindProgram and
            unbindProgram methods.
        */
        void setActiveVertexShader(GLSLShader* vertexShader);
        void setActiveTessDomainShader(GLSLShader* domainShader);
        void setActiveTessHullShader(GLSLShader* hullShader);
        void setActiveGeometryShader(GLSLShader* geometryShader);
        void setActiveFragmentShader(GLSLShader* fragmentShader);
        void setActiveComputeShader(GLSLShader* computShader);

        static GLSLSeparableProgramManager& getSingleton(void);
        static GLSLSeparableProgramManager* getSingletonPtr(void);
    };
}

#endif
