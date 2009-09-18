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
#include "OgreD3D10HardwareVertexBuffer.h"
#include "OgreD3D10HardwareBuffer.h"
#include "OgreD3D10Device.h"

namespace Ogre {

	//---------------------------------------------------------------------
	D3D10HardwareVertexBuffer::D3D10HardwareVertexBuffer(HardwareBufferManagerBase* mgr, size_t vertexSize, 
		size_t numVertices, HardwareBuffer::Usage usage, D3D10Device & device, 
		bool useSystemMemory, bool useShadowBuffer)
		: HardwareVertexBuffer(mgr, vertexSize, numVertices, usage, useSystemMemory, useShadowBuffer)
	{
		// everything is done via internal generalisation
		mBufferImpl = new D3D10HardwareBuffer(D3D10HardwareBuffer::VERTEX_BUFFER, 
			mSizeInBytes, mUsage, device, useSystemMemory, useShadowBuffer);

	}
	//---------------------------------------------------------------------
	D3D10HardwareVertexBuffer::~D3D10HardwareVertexBuffer()
	{
		delete mBufferImpl;
	}
	//---------------------------------------------------------------------
	void* D3D10HardwareVertexBuffer::lock(size_t offset, size_t length, LockOptions options)
	{
		return mBufferImpl->lock(offset, length, options);
	}
	//---------------------------------------------------------------------
	void D3D10HardwareVertexBuffer::unlock(void)
	{
		mBufferImpl->unlock();
	}
	//---------------------------------------------------------------------
	void D3D10HardwareVertexBuffer::readData(size_t offset, size_t length, void* pDest)
	{
		mBufferImpl->readData(offset, length, pDest);
	}
	//---------------------------------------------------------------------
	void D3D10HardwareVertexBuffer::writeData(size_t offset, size_t length, const void* pSource,
		bool discardWholeBuffer)
	{
		mBufferImpl->writeData(offset, length, pSource, discardWholeBuffer);
	}
	//---------------------------------------------------------------------
	void D3D10HardwareVertexBuffer::copyData(HardwareBuffer& srcBuffer, size_t srcOffset, 
		size_t dstOffset, size_t length, bool discardWholeBuffer)
	{
		D3D10HardwareVertexBuffer& d3dBuf = static_cast<D3D10HardwareVertexBuffer&>(srcBuffer);

		mBufferImpl->copyData(*(d3dBuf.mBufferImpl), srcOffset, dstOffset, length, discardWholeBuffer);
	}
	//---------------------------------------------------------------------
	bool D3D10HardwareVertexBuffer::isLocked(void) const
	{
		return mBufferImpl->isLocked();
	}
	//---------------------------------------------------------------------
	bool D3D10HardwareVertexBuffer::releaseIfDefaultPool(void)
	{
		/*		if (mD3DPool == D3DPOOL_DEFAULT)
		{
		SAFE_RELEASE(mlpD3DBuffer);
		return true;
		}
		return false;
		*/
		return true;
	}
	//---------------------------------------------------------------------
	bool D3D10HardwareVertexBuffer::recreateIfDefaultPool(D3D10Device & device)
	{
		/*	if (mD3DPool == D3DPOOL_DEFAULT)
		{
		// Create the Index buffer
		HRESULT hr = device->CreateIndexBuffer(
		static_cast<UINT>(mSizeInBytes),
		D3D10Mappings::get(mUsage),
		D3D10Mappings::get(mIndexType),
		mD3DPool,
		&mlpD3DBuffer,
		NULL
		);

		if (FAILED(hr))
		{
		String msg = DXGetErrorDescription(hr);
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
		"Cannot create D3D10 Index buffer: " + msg, 
		"D3D10HardwareVertexBuffer::D3D10HardwareVertexBuffer");
		}

		return true;
		}
		return false;
		*/
		return true;
	}
	//---------------------------------------------------------------------
	ID3D10Buffer * D3D10HardwareVertexBuffer::getD3DVertexBuffer( void ) const
	{
		return mBufferImpl->getD3DBuffer();
	}
	//---------------------------------------------------------------------

}

