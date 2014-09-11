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
#ifndef __GLSLESLinkProgram_H__
#define __GLSLESLinkProgram_H__

#include "OgreGLES2Prerequisites.h"
#include "OgreGpuProgram.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgreGLSLESProgramCommon.h"
#include "OgreGLES2ManagedResource.h"

namespace Ogre {
    
    class GLSLESGpuProgram;

	/** C++ encapsulation of GLSL ES Program Object

	*/

	class _OgreGLES2Export GLSLESLinkProgram : public GLSLESProgramCommon MANAGED_RESOURCE
	{
	protected:
        virtual void extractLayoutQualifiers(void) {}

		/// Compiles and links the vertex and fragment programs
		virtual void compileAndLink(void);
        /// Put a program in use
        virtual void _useProgram(void);

		void buildGLUniformReferences(void);

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
        /** See AndroidResource. */
        virtual void notifyOnContextLost();
        
        /** See AndroidResource. */
        virtual void notifyOnContextReset();
#endif
        
	public:
		/// Constructor should only be used by GLSLESLinkProgramManager
		GLSLESLinkProgram(GLSLESGpuProgram* vertexProgram, GLSLESGpuProgram* fragmentProgram);
		virtual ~GLSLESLinkProgram(void);

		/** Makes a program object active by making sure it is linked and then putting it in use.
		*/
		void activate(void);

		/** Updates program object uniforms using data from GpuProgramParameters.
		normally called by GLSLESGpuProgram::bindParameters() just before rendering occurs.
		*/
		virtual void updateUniforms(GpuProgramParametersSharedPtr params, uint16 mask, GpuProgramType fromProgType);
		/** Updates program object uniform blocks using data from GpuProgramParameters.
         normally called by GLSLGpuProgram::bindParameters() just before rendering occurs.
         */
		virtual void updateUniformBlocks(GpuProgramParametersSharedPtr params, uint16 mask, GpuProgramType fromProgType);
		/** Updates program object uniforms using data from pass iteration GpuProgramParameters.
		normally called by GLSLESGpuProgram::bindMultiPassParameters() just before multi pass rendering occurs.
		*/
		virtual void updatePassIterationUniforms(GpuProgramParametersSharedPtr params);
	};

}

#endif // __GLSLESLinkProgram_H__
