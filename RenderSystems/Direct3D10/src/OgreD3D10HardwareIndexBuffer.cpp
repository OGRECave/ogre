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
#include "OgreD3D10HardwareIndexBuffer.h"
#include "OgreD3D10HardwareBuffer.h"

namespace Ogre {

	//---------------------------------------------------------------------
	D3D10HardwareIndexBuffer::D3D10HardwareIndexBuffer(HardwareBufferManagerBase* mgr, HardwareIndexBuffer::IndexType idxType, 
		size_t numIndexes, HardwareBuffer::Usage usage, D3D10Device & device, 
		bool useSystemMemory, bool useShadowBuffer)
		: HardwareIndexBuffer(mgr, idxType, numIndexes, usage, useSystemMemory, useShadowBuffer)
	{
		// everything is done via internal generalisation
		mBufferImpl = new D3D10HardwareBuffer(D3D10HardwareBuffer::INDEX_BUFFER, 
			mSizeInBytes, mUsage, device, useSystemMemory, useShadowBuffer);

	}
	//---------------------------------------------------------------------
	D3D10HardwareIndexBuffer::~D3D10HardwareIndexBuffer()
	{
		delete mBufferImpl;
	}
	//---------------------------------------------------------------------
	void* D3D10HardwareIndexBuffer::lock(size_t offset, size_t length, LockOptions options)
	{
		return mBufferImpl->lock(offset, length, options);
	}
	//---------------------------------------------------------------------
	void D3D10HardwareIndexBuffer::unlock(void)
	{
		mBufferImpl->unlock();
	}
	//---------------------------------------------------------------------
	void D3D10HardwareIndexBuffer::readData(size_t offset, size_t length, void* pDest)
	{
		mBufferImpl->readData(offset, length, pDest);
	}
	//---------------------------------------------------------------------
	void D3D10HardwareIndexBuffer::writeData(size_t offset, size_t length, const void* pSource,
		bool discardWholeBuffer)
	{
		mBufferImpl->writeData(offset, length, pSource, discardWholeBuffer);
	}
	//---------------------------------------------------------------------
	void D3D10HardwareIndexBuffer::copyData(HardwareBuffer& srcBuffer, size_t srcOffset, 
		size_t dstOffset, size_t length, bool discardWholeBuffer)
	{
		D3D10HardwareIndexBuffer& d3dBuf = static_cast<D3D10HardwareIndexBuffer&>(srcBuffer);

		mBufferImpl->copyData(*(d3dBuf.mBufferImpl), srcOffset, dstOffset, length, discardWholeBuffer);
	}
	//---------------------------------------------------------------------
	bool D3D10HardwareIndexBuffer::isLocked(void) const
	{
		return mBufferImpl->isLocked();
	}
	//---------------------------------------------------------------------
	bool D3D10HardwareIndexBuffer::releaseIfDefaultPool(void)
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
	bool D3D10HardwareIndexBuffer::recreateIfDefaultPool(D3D10Device & device)
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
		"D3D10HardwareIndexBuffer::D3D10HardwareIndexBuffer");
		}

		return true;
		}
		return false;
		*/
		return true;
	}
	//---------------------------------------------------------------------
	ID3D10Buffer * D3D10HardwareIndexBuffer::getD3DIndexBuffer( void ) const
	{
		return mBufferImpl->getD3DBuffer();
	}
	//---------------------------------------------------------------------

}
