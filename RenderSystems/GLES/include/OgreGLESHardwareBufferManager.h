/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
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

#ifndef __GLESHardwareBufferManager_H__
#define __GLESHardwareBufferManager_H__

#include "OgreGLESPrerequisites.h"
#include "OgreHardwareBufferManager.h"

namespace Ogre {
    // Threshold at which glMapBuffer becomes more efficient than glBufferSubData (32k?)
    // non-Win32 machines are having issues with this, looks like buffer corruption
    // disable for now until we figure out where the problem lies
    #if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    #   define OGRE_GL_MAP_BUFFER_THRESHOLD (1024 * 32)
    #else
    #   define OGRE_GL_MAP_BUFFER_THRESHOLD 0
    #endif

    /** Implementation of HardwareBufferManager for OpenGLES. */
    class _OgrePrivate GLESHardwareBufferManager : public HardwareBufferManager
    {
        protected:
            char* mScratchBufferPool;
            OGRE_MUTEX(mScratchMutex)

        public:
            GLESHardwareBufferManager();
            virtual ~GLESHardwareBufferManager();
            /// Creates a vertex buffer
            HardwareVertexBufferSharedPtr createVertexBuffer(size_t vertexSize,
                size_t numVerts, HardwareBuffer::Usage usage, bool useShadowBuffer = false);
            /// Create a hardware vertex buffer
            HardwareIndexBufferSharedPtr createIndexBuffer(
                HardwareIndexBuffer::IndexType itype, size_t numIndexes,
                HardwareBuffer::Usage usage, bool useShadowBuffer = false);
	        /// Create a render to vertex buffer
    	    RenderToVertexBufferSharedPtr createRenderToVertexBuffer();

            /// Utility function to get the correct GL usage based on HBU's
            static GLenum getGLUsage(unsigned int usage);

            /// Utility function to get the correct GL type based on VET's
            static GLenum getGLType(unsigned int type);

            /** Allocator method to allow us to use a pool of memory as a scratch
                area for hardware buffers. This is because glMapBuffer is incredibly
                inefficient, seemingly no matter what options we give it. So for the
                period of lock/unlock, we will instead allocate a section of a local
                memory pool, and use glBufferSubDataARB / glGetBufferSubDataARB
                instead.
            */
            void* allocateScratch(uint32 size);

            /// @see allocateScratch
            void deallocateScratch(void* ptr);
    };
}

#endif
