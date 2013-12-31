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

#ifndef __GLSLGpuProgram_H__
#define __GLSLGpuProgram_H__

// Precompiler options
#include "OgreGLSLExtSupport.h"
#include "OgreGL3PlusGpuProgram.h"


namespace Ogre {

    /** GLSL low level compiled shader object - this class is used to get at the linked program object 
		and provide an interface for GL3PlusRenderSystem calls.  GLSL does not provide access to the
		low level code of the shader so this class is really just a dummy place holder.
		GLSL uses a program object to represent the active vertex and fragment programs used
		but Ogre materials maintain separate instances of the active vertex and fragment programs
		which creates a small problem for GLSL integration.  The GLSLGpuProgram class provides the
		interface between the GLSLLinkProgramManager , GL3PlusRenderSystem, and the active GLSLProgram
		instances.
	*/
    class _OgreGL3PlusExport GLSLGpuProgram : public GL3PlusGpuProgram
    {
    private:
		/// GL Handle for the shader object
		GLSLProgram* mGLSLProgram;

		/// Keep track of the number of vertex shaders created
		static GLuint mVertexShaderCount;
		/// Keep track of the number of fragment shaders created
		static GLuint mFragmentShaderCount;
		/// Keep track of the number of geometry shaders created
		static GLuint mGeometryShaderCount;
		/// Keep track of the number of tesselation hull(control) shaders created
		static GLuint mHullShaderCount;
		/// Keep track of the number of tesselation domain(evaluation) shaders created
		static GLuint mDomainShaderCount;
		/// Keep track of the number of compute shaders created
		static GLuint mComputeShaderCount;

        /** Flag indicating that the program object has been successfully linked.
         Only used when programs are linked separately with GL_ARB_separate_shader_objects.
         */
		GLint mLinked;

	public:
        GLSLGpuProgram(GLSLProgram* parent);
		~GLSLGpuProgram();


		/// Execute the binding functions for this program
		void bindProgram(void);
		/// Execute the unbinding functions for this program
		void unbindProgram(void);
		/// Execute the param binding functions for this program
		void bindProgramParameters(GpuProgramParametersSharedPtr params, uint16 mask);
		/// Execute the pass iteration param binding functions for this program
		void bindProgramPassIterationParameters(GpuProgramParametersSharedPtr params);
		/// Execute the shared param binding functions for this program
		void bindProgramSharedParameters(GpuProgramParametersSharedPtr params, uint16 mask);

		/// Get the GLSLProgram for the shader object
		GLSLProgram* getGLSLProgram(void) const { return mGLSLProgram; }		

        /** Return the programs link status
         Only used when programs are linked separately with GL_ARB_separate_shader_objects.
         */
        GLint isLinked(void) { return mLinked; }

        /** Set the programs link status
         Only used when programs are linked separately with GL_ARB_separate_shader_objects.
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

#endif // __GLSLGpuProgram_H__
