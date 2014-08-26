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
#include "OgreD3D11HardwareIndexBuffer.h"
#include "OgreD3D11HardwareBuffer.h"

namespace Ogre {

	//---------------------------------------------------------------------
	D3D11HardwareIndexBuffer::D3D11HardwareIndexBuffer(HardwareBufferManagerBase* mgr, HardwareIndexBuffer::IndexType idxType, 
		size_t numIndexes, HardwareBuffer::Usage usage, D3D11Device & device, 
		bool useSystemMemory, bool useShadowBuffer)
		: HardwareIndexBuffer(mgr, idxType, numIndexes, usage, useSystemMemory, useShadowBuffer)
	{
		// everything is done via internal generalisation
		mBufferImpl = new D3D11HardwareBuffer(D3D11HardwareBuffer::INDEX_BUFFER, 
			mSizeInBytes, mUsage, device, useSystemMemory, useShadowBuffer, false);

	}
	//---------------------------------------------------------------------
	D3D11HardwareIndexBuffer::~D3D11HardwareIndexBuffer()
	{
		delete mBufferImpl;
	}
	//---------------------------------------------------------------------
	void* D3D11HardwareIndexBuffer::lock(size_t offset, size_t length, LockOptions options)
	{
		return mBufferImpl->lock(offset, length, options);
	}
	//---------------------------------------------------------------------
	void D3D11HardwareIndexBuffer::unlock(void)
	{
		mBufferImpl->unlock();
	}
	//---------------------------------------------------------------------
	void D3D11HardwareIndexBuffer::readData(size_t offset, size_t length, void* pDest)
	{
		mBufferImpl->readData(offset, length, pDest);
	}
	//---------------------------------------------------------------------
	void D3D11HardwareIndexBuffer::writeData(size_t offset, size_t length, const void* pSource,
		bool discardWholeBuffer)
	{
		mBufferImpl->writeData(offset, length, pSource, discardWholeBuffer);
	}
	//---------------------------------------------------------------------
	void D3D11HardwareIndexBuffer::copyData(HardwareBuffer& srcBuffer, size_t srcOffset, 
		size_t dstOffset, size_t length, bool discardWholeBuffer)
	{
		D3D11HardwareIndexBuffer& d3dBuf = static_cast<D3D11HardwareIndexBuffer&>(srcBuffer);

		mBufferImpl->copyData(*(d3dBuf.mBufferImpl), srcOffset, dstOffset, length, discardWholeBuffer);
	}
	//---------------------------------------------------------------------
	bool D3D11HardwareIndexBuffer::isLocked(void) const
	{
		return mBufferImpl->isLocked();
	}
	//---------------------------------------------------------------------
	ID3D11Buffer * D3D11HardwareIndexBuffer::getD3DIndexBuffer( void ) const
	{
		return mBufferImpl->getD3DBuffer();
	}
	//---------------------------------------------------------------------

}
