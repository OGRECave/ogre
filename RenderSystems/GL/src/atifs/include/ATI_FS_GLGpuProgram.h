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

#ifndef __ATI_FS_GLGpuProgram_H__
#define __ATI_FS_GLGpuProgram_H__

#include "OgreGLPrerequisites.h"
#include "OgreGLGpuProgram.h"

namespace Ogre {

	/** Specialisation of the GL low-level program for ATI Fragment Shader programs. */
	class _OgrePrivate ATI_FS_GLGpuProgram : public GLGpuProgram
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
		const GLuint getProgramID(void) const
		{ return mProgramID; }

	protected:
		/// @copydoc Resource::unload
		void unloadImpl(void);
		void loadFromSource(void);

	}; // class ATI_FS_GLGpuProgram



}; // namespace Ogre

#endif // __ATI_FS_GLGpuProgram_H__
