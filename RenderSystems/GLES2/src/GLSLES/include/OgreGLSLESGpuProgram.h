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

#ifndef __GLSLESGpuProgram_H__
#define __GLSLESGpuProgram_H__

// Precompiler options
#include "OgreGLSLESExtSupport.h"
#include "OgreGLES2GpuProgram.h"

namespace Ogre {

    /** GLSL ES low level compiled shader object - this class is used to get at the linked program object 
        and provide an interface for GLES2RenderSystem calls.  GLSL ES does not provide access to the
        low level code of the shader so this class is really just a dummy place holder.
        GLSL ES uses a program object to represent the active vertex and fragment programs used
        but Ogre materials maintain separate instances of the active vertex and fragment programs
        which creates a small problem for GLSL integration.  The GLSLESGpuProgram class provides the
        interface between the GLSLESLinkProgramManager, GLSLESProgramPipelineManager , GLES2RenderSystem,
        and the active GLSLESProgram instances.
    */
    class _OgreGLES2Export GLSLESGpuProgram : public GLES2GpuProgram
    {
    private:
        /// GL Handle for the shader object
        GLSLESProgram* mGLSLProgram;

        /// Keep track of the number of vertex shaders created
        static GLuint mVertexShaderCount;
        /// Keep track of the number of fragment shaders created
        static GLuint mFragmentShaderCount;

        /** Flag indicating that the program object has been successfully linked.
            Only used when programs are linked separately with GL_EXT_separate_shader_objects.
         */
        GLint mLinked;

    public:
        GLSLESGpuProgram(GLSLESProgram* parent);
        ~GLSLESGpuProgram();

        /// Execute the binding functions for this program
        void bindProgram(void);
        /// Execute the unbinding functions for this program
        void unbindProgram(void);
        static void unbindAll(void);
        /// Execute the param binding functions for this program
        void bindProgramParameters(GpuProgramParametersSharedPtr params, uint16 mask);
        /// Execute the shared param binding functions for this program
        void bindProgramSharedParameters(GpuProgramParametersSharedPtr params, uint16 mask);
        /// Execute the pass iteration param binding functions for this program
        void bindProgramPassIterationParameters(GpuProgramParametersSharedPtr params);

        /// Get the GLSLESProgram for the shader object
        GLSLESProgram* getGLSLProgram(void) const { return mGLSLProgram; }      

        /** Return the programs link status
            Only used when programs are linked separately with GL_EXT_separate_shader_objects.
         */
        GLint isLinked(void) { return mLinked; }

        /** Set the programs link status
            Only used when programs are linked separately with GL_EXT_separate_shader_objects.
         */
        void setLinked(GLint flag) { mLinked = flag; }

    protected:
        /// Overridden from GpuProgram
        void loadFromSource(void);
        /// @copydoc Resource::unloadImpl
        void unloadImpl(void);
        /// @copydoc Resource::loadImpl
        void loadImpl(void);
    };
}

#endif // __GLSLESGpuProgram_H__
