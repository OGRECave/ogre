/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
#ifndef __D3D11HARDWAREVERTEXBUFFER_H__
#define __D3D11HARDWAREVERTEXBUFFER_H__

#include "OgreD3D11Prerequisites.h"
#include "OgreHardwareVertexBuffer.h"

namespace Ogre {

	/// Specialisation of HardwareVertexBuffer for D3D11
	class D3D11HardwareVertexBuffer : public HardwareVertexBuffer 
	{
	protected:
		D3D11HardwareBuffer* mBufferImpl;
		// have to implement these, but do nothing as overridden lock/unlock
		void* lockImpl(size_t offset, size_t length, LockOptions options) {return 0;}
		void unlockImpl(void) {}

	public:
		D3D11HardwareVertexBuffer(HardwareBufferManagerBase* mgr, size_t vertexSize, size_t numVertices, 
			HardwareBuffer::Usage usage, D3D11Device & device, bool useSystemMem, bool useShadowBuffer);
		~D3D11HardwareVertexBuffer();

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
		bool recreateIfDefaultPool(D3D11Device & device);

		/// Get the D3D-specific vertex buffer
		ID3D11Buffer * getD3DVertexBuffer(void) const;


	};

}
#endif

