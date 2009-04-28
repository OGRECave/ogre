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
#ifndef __D3D10HARDWAREVERTEXBUFFER_H__
#define __D3D10HARDWAREVERTEXBUFFER_H__

#include "OgreD3D10Prerequisites.h"
#include "OgreHardwareVertexBuffer.h"

namespace Ogre {

	/// Specialisation of HardwareVertexBuffer for D3D10
	class D3D10HardwareVertexBuffer : public HardwareVertexBuffer 
	{
	protected:
		D3D10HardwareBuffer* mBufferImpl;
		// have to implement these, but do nothing as overridden lock/unlock
		void* lockImpl(size_t offset, size_t length, LockOptions options) {return 0;}
		void unlockImpl(void) {}

	public:
		D3D10HardwareVertexBuffer(HardwareBufferManagerBase* mgr, size_t vertexSize, size_t numVertices, 
			HardwareBuffer::Usage usage, D3D10Device & device, bool useSystemMem, bool useShadowBuffer);
		~D3D10HardwareVertexBuffer();

		// override all data-gathering methods
		void* lock(size_t offset, size_t length, LockOptions options);
		void unlock(void);
		void readData(size_t offset, size_t length, void* pDest);
		void writeData(size_t offset, size_t length, const void* pSource,
			bool discardWholeBuffer = false);

		void copyData(HardwareBuffer& srcBuffer, size_t srcOffset, 
			size_t dstOffset, size_t length, bool discardWholeBuffer = false);
		bool isLocked(void) const;

		/// For dealing with lost devices - release the resource if in the default pool
		bool releaseIfDefaultPool(void);
		/// For dealing with lost devices - recreate the resource if in the default pool
		bool recreateIfDefaultPool(D3D10Device & device);

		/// Get the D3D-specific vertex buffer
		ID3D10Buffer * getD3DVertexBuffer(void) const;


	};

}
#endif

