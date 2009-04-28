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
		String msg = DXGetErrorDescription9(hr);
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

