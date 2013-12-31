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

#ifndef __ATI_FS_GLGpuProgram_H__
#define __ATI_FS_GLGpuProgram_H__

#include "OgreGLPrerequisites.h"
#include "OgreGLGpuProgram.h"

namespace Ogre {

	/** Specialisation of the GL low-level program for ATI Fragment Shader programs. */
	class _OgreGLExport ATI_FS_GLGpuProgram : public GLGpuProgram
	{
	public:
        ATI_FS_GLGpuProgram(ResourceManager* creator, 
            const String& name, ResourceHandle handle, 
            const String& group, bool isManual, ManualResourceLoader* loader);
		virtual ~ATI_FS_GLGpuProgram();


		/// Execute the binding functions for this program
		void bindProgram(void);
		/// Execute the unbinding functions for this program
		void unbindProgram(void);
		/// Execute the param binding functions for this program
		void bindProgramParameters(GpuProgramParametersSharedPtr params, uint16 mask);
		/** Execute the pass iteration param binding functions for this program.
            Only binds those parameters used for multipass rendering
        */
        void bindProgramPassIterationParameters(GpuProgramParametersSharedPtr params);

		/// Get the assigned GL program id
		GLuint getProgramID(void) const
		{ return mProgramID; }

	protected:
		/// @copydoc Resource::unload
		void unloadImpl(void);
		void loadFromSource(void);

	}; // class ATI_FS_GLGpuProgram



} // namespace Ogre

#endif // __ATI_FS_GLGpuProgram_H__
