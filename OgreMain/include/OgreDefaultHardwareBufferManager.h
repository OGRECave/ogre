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

#ifndef __DefaultHardwareBufferManager_H__
#define __DefaultHardwareBufferManager_H__

#include "OgrePrerequisites.h"
#include "OgreHardwareBufferManager.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgreHardwareIndexBuffer.h"

namespace Ogre {

    /// Specialisation of HardwareVertexBuffer for emulation
    class _OgreExport DefaultHardwareVertexBuffer : public HardwareVertexBuffer 
    {
	protected:
		unsigned char* mpData;
        /** See HardwareBuffer. */
        void* lockImpl(size_t offset, size_t length, LockOptions options);
        /** See HardwareBuffer. */
		void unlockImpl(void);
    public:
		DefaultHardwareVertexBuffer(size_t vertexSize, size_t numVertices, 
            HardwareBuffer::Usage usage);
        ~DefaultHardwareVertexBuffer();
        /** See HardwareBuffer. */
        void readData(size_t offset, size_t length, void* pDest);
        /** See HardwareBuffer. */
        void writeData(size_t offset, size_t length, const void* pSource,
				bool discardWholeBuffer = false);
        /** Override HardwareBuffer to turn off all shadowing. */
        void* lock(size_t offset, size_t length, LockOptions options);
        /** Override HardwareBuffer to turn off all shadowing. */
		void unlock(void);


    };

	/// Specialisation of HardwareIndexBuffer for emulation
    class _OgreExport DefaultHardwareIndexBuffer : public HardwareIndexBuffer
    {
	protected:
		unsigned char* mpData;
        /** See HardwareBuffer. */
        void* lockImpl(size_t offset, size_t length, LockOptions options);
        /** See HardwareBuffer. */
		void unlockImpl(void);
    public:
		DefaultHardwareIndexBuffer(IndexType idxType, size_t numIndexes, HardwareBuffer::Usage usage);
        ~DefaultHardwareIndexBuffer();
        /** See HardwareBuffer. */
        void readData(size_t offset, size_t length, void* pDest);
        /** See HardwareBuffer. */
        void writeData(size_t offset, size_t length, const void* pSource,
				bool discardWholeBuffer = false);
        /** Override HardwareBuffer to turn off all shadowing. */
        void* lock(size_t offset, size_t length, LockOptions options);
        /** Override HardwareBuffer to turn off all shadowing. */
		void unlock(void);

    };

	/** Specialisation of HardwareBufferManager to emulate hardware buffers.
	@remarks
		You might want to instantiate this class if you want to utilise
		classes like MeshSerializer without having initialised the 
		rendering system (which is required to create a 'real' hardware
		buffer manager.
	*/
	class _OgreExport DefaultHardwareBufferManager : public HardwareBufferManager
	{
    public:
        DefaultHardwareBufferManager();
        ~DefaultHardwareBufferManager();
        /// Creates a vertex buffer
		HardwareVertexBufferSharedPtr 
            createVertexBuffer(size_t vertexSize, size_t numVerts, 
				HardwareBuffer::Usage usage, bool useShadowBuffer = false);
		/// Create a hardware vertex buffer
		HardwareIndexBufferSharedPtr 
            createIndexBuffer(HardwareIndexBuffer::IndexType itype, size_t numIndexes, 
				HardwareBuffer::Usage usage, bool useShadowBuffer = false);
		/// Create a hardware vertex buffer
		RenderToVertexBufferSharedPtr createRenderToVertexBuffer();
    };


}

#endif
