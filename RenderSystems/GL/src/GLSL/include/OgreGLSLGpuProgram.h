/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/

#ifndef __GLSLGpuProgram_H__
#define __GLSLGpuProgram_H__

// Precompiler options
#include "OgreGLSLExtSupport.h"
#include "OgreGLGpuProgram.h"


namespace Ogre {

    /** GLSL low level compiled shader object - this class is used to get at the linked program object 
		and provide an interface for GLRenderSystem calls.  GLSL does not provide access to the
		low level code of the shader so this class is really just a dummy place holder.
		GLSL uses a program object to represent the active vertex and fragment programs used
		but Ogre materials maintain seperate instances of the active vertex and fragment programs
		which creates a small problem for GLSL integration.  The GLSLGpuProgram class provides the
		interface between the GLSLLinkProgramManager , GLRenderSystem, and the active GLSLProgram
		instances.
	*/
    class _OgrePrivate GLSLGpuProgram : public GLGpuProgram
    {
    private:
		/// GL Handle for the shader object
		GLSLProgram* mGLSLProgram;

		/// keep track of the number of vertex shaders created
		static GLuint mVertexShaderCount;
		/// keep track of the number of fragment shaders created
		static GLuint mFragmentShaderCount;
		/// keep track of the number of geometry shaders created
		static GLuint mGeometryShaderCount;

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

		/// Get the assigned GL program id
		const GLuint getProgramID(void) const
		{ return mProgramID; }

		/// get the GLSLProgram for the shader object
		GLSLProgram* getGLSLProgram(void) const { return mGLSLProgram; }

		/// @copydoc GLGpuProgram::getAttributeIndex
		GLuint getAttributeIndex(VertexElementSemantic semantic, uint index);
		
		/// @copydoc GLGpuProgram::isAttributeValid
		bool isAttributeValid(VertexElementSemantic semantic, uint index);
		

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
