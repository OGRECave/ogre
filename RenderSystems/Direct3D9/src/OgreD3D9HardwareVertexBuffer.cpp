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
#include "OgreD3D9HardwareVertexBuffer.h"
#include "OgreD3D9Mappings.h"
#include "OgreException.h"
#include "OgreD3D9HardwareBufferManager.h"
#include "OgreD3D9RenderSystem.h"
#include "OgreRoot.h"
#include "OgreD3D9Device.h"
#include "OgreD3D9ResourceManager.h"

namespace Ogre {

	//---------------------------------------------------------------------
    D3D9HardwareVertexBuffer::D3D9HardwareVertexBuffer(HardwareBufferManagerBase* mgr, size_t vertexSize, 
        size_t numVertices, HardwareBuffer::Usage usage, 
        bool useSystemMemory, bool useShadowBuffer)
        : HardwareVertexBuffer(mgr, vertexSize, numVertices, usage, useSystemMemory, useShadowBuffer)
    {
		D3DPOOL eResourcePool;
		       
#if OGRE_D3D_MANAGE_BUFFERS
		eResourcePool = useSystemMemory? D3DPOOL_SYSTEMMEM : 
			// If not system mem, use managed pool UNLESS buffer is discardable
			// if discardable, keeping the software backing is expensive
			(usage & HardwareBuffer::HBU_DISCARDABLE)? D3DPOOL_DEFAULT : D3DPOOL_MANAGED;
#else
		eResourcePool = useSystemMemory? D3DPOOL_SYSTEMMEM : D3DPOOL_DEFAULT;
#endif       		

		// Set the desired memory pool.
		mBufferDesc.Pool = eResourcePool;

		// Allocate the system memory buffer.
		mSystemMemoryBuffer = new char [getSizeInBytes()];
		memset(mSystemMemoryBuffer, 0, getSizeInBytes());	

		// Case we have to create this buffer resource on loading.
		if (D3D9RenderSystem::getResourceManager()->getCreationPolicy() == RCP_CREATE_ON_ALL_DEVICES)
		{
			for (uint i = 0; i < D3D9RenderSystem::getResourceCreationDeviceCount(); ++i)
			{
				IDirect3DDevice9* d3d9Device = D3D9RenderSystem::getResourceCreationDevice(i);

				createBuffer(d3d9Device, mBufferDesc.Pool);
			}
		}				
    }
	//---------------------------------------------------------------------
    D3D9HardwareVertexBuffer::~D3D9HardwareVertexBuffer()
    {
		DeviceToBufferResourcesIterator it = mMapDeviceToBufferResources.begin();

		while (it != mMapDeviceToBufferResources.end())
		{
			SAFE_RELEASE(it->second->mBuffer);
			SAFE_DELETE(it->second);
			++it;
		}	
		mMapDeviceToBufferResources.clear();   
		SAFE_DELETE(mSystemMemoryBuffer);
    }
	//---------------------------------------------------------------------
    void* D3D9HardwareVertexBuffer::lockImpl(size_t offset, 
        size_t length, LockOptions options)
    {
		if (options != HBL_READ_ONLY)
		{
			DeviceToBufferResourcesIterator it = mMapDeviceToBufferResources.begin();

			while (it != mMapDeviceToBufferResources.end())
			{
				BufferResources* bufferResources = it->second;

				bufferResources->mOutOfDate = true;
				if (offset < bufferResources->mLockOffset)
					bufferResources->mLockOffset = offset;
				if (length > bufferResources->mLockLength)
					bufferResources->mLockLength = length;
				if (bufferResources->mLockOptions != HBL_DISCARD)
					bufferResources->mLockOptions = options;					

				++it;
			}
		}

		return mSystemMemoryBuffer + offset;		
    }
	//---------------------------------------------------------------------
	void D3D9HardwareVertexBuffer::unlockImpl(void)
    {
		DeviceToBufferResourcesIterator it = mMapDeviceToBufferResources.begin();
		uint nextFrameNumber = Root::getSingleton().getNextFrameNumber();

		while (it != mMapDeviceToBufferResources.end())
		{
			BufferResources* bufferResources = it->second;

			if (bufferResources->mOutOfDate && 
				bufferResources->mBuffer != NULL&&
				nextFrameNumber - bufferResources->mLastUsedFrame <= 1)
				updateBufferResources(mSystemMemoryBuffer, bufferResources);

			++it;
		}		
    }
	//---------------------------------------------------------------------
    void D3D9HardwareVertexBuffer::readData(size_t offset, size_t length, 
        void* pDest)
    {
        // There is no functional interface in D3D, just do via manual 
        // lock, copy & unlock
        void* pSrc = this->lock(offset, length, HardwareBuffer::HBL_READ_ONLY);
        memcpy(pDest, pSrc, length);
        this->unlock();

    }
	//---------------------------------------------------------------------
	void D3D9HardwareVertexBuffer::writeData(size_t offset, size_t length, 
		const void* pSource,
		bool discardWholeBuffer)
	{
		// There is no functional interface in D3D, just do via manual 
		// lock, copy & unlock
		void* pDst = this->lock(offset, length, 
			discardWholeBuffer ? HardwareBuffer::HBL_DISCARD : HardwareBuffer::HBL_NORMAL);
		memcpy(pDst, pSource, length);
		this->unlock();
	}
	//---------------------------------------------------------------------
	void D3D9HardwareVertexBuffer::notifyOnDeviceCreate(IDirect3DDevice9* d3d9Device)
	{		
		
	}
	//---------------------------------------------------------------------
	void D3D9HardwareVertexBuffer::notifyOnDeviceDestroy(IDirect3DDevice9* d3d9Device)
	{
		DeviceToBufferResourcesIterator it = mMapDeviceToBufferResources.find(d3d9Device);

		if (it != mMapDeviceToBufferResources.end())	
		{								
			SAFE_RELEASE(it->second->mBuffer);
			SAFE_DELETE(it->second);
			mMapDeviceToBufferResources.erase(it);
		}	
	}
	//---------------------------------------------------------------------
	void D3D9HardwareVertexBuffer::notifyOnDeviceLost(IDirect3DDevice9* d3d9Device)
	{
		if (mBufferDesc.Pool == D3DPOOL_DEFAULT)
		{
			DeviceToBufferResourcesIterator it = mMapDeviceToBufferResources.find(d3d9Device);

			if (it != mMapDeviceToBufferResources.end())
			{				
				SAFE_RELEASE(it->second->mBuffer);	
			}					
		}
	}
	//---------------------------------------------------------------------
	void D3D9HardwareVertexBuffer::notifyOnDeviceReset(IDirect3DDevice9* d3d9Device)
	{
		if (D3D9RenderSystem::getResourceManager()->getCreationPolicy() == RCP_CREATE_ON_ALL_DEVICES)
			createBuffer(d3d9Device, mBufferDesc.Pool);
	}
	//---------------------------------------------------------------------
	void D3D9HardwareVertexBuffer::createBuffer(IDirect3DDevice9* d3d9Device, D3DPOOL ePool)
	{
		BufferResources* bufferResources;		
		HRESULT hr;

		DeviceToBufferResourcesIterator it;

		// Find the vertex buffer of this device.
		it = mMapDeviceToBufferResources.find(d3d9Device);
		if (it != mMapDeviceToBufferResources.end())
		{
			bufferResources = it->second;
			SAFE_RELEASE(bufferResources->mBuffer);
		}
		else
		{
			bufferResources = new BufferResources;			
			mMapDeviceToBufferResources[d3d9Device] = bufferResources;
		}

		bufferResources->mBuffer = NULL;
		bufferResources->mOutOfDate = true;
		bufferResources->mLockOffset = 0;
		bufferResources->mLockLength = getSizeInBytes();
		bufferResources->mLockOptions = HBL_NORMAL;
		bufferResources->mLastUsedFrame = Root::getSingleton().getNextFrameNumber();
		
		// Create the vertex buffer
		hr = d3d9Device->CreateVertexBuffer(
			static_cast<UINT>(mSizeInBytes), 
			D3D9Mappings::get(mUsage), 
			0, // No FVF here, thank you.
			ePool,
			&bufferResources->mBuffer,
			NULL);

		if (FAILED(hr))
		{
			String msg = DXGetErrorDescription9(hr);
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"Cannot restore D3D9 vertex buffer: " + msg, 
				"D3D9HardwareVertexBuffer::createBuffer");
		}

		hr = bufferResources->mBuffer->GetDesc(&mBufferDesc);
		if (FAILED(hr))
		{
			String msg = DXGetErrorDescription9(hr);
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"Cannot get D3D9 Vertex buffer desc: " + msg, 
				"D3D9HardwareVertexBuffer::createBuffer");
		}		
	}
	//---------------------------------------------------------------------
	IDirect3DVertexBuffer9* D3D9HardwareVertexBuffer::getD3D9VertexBuffer(void)
	{
		IDirect3DDevice9* d3d9Device = D3D9RenderSystem::getActiveD3D9Device();
		DeviceToBufferResourcesIterator it;

		// Find the vertex buffer of this device.
		it = mMapDeviceToBufferResources.find(d3d9Device);

		// Case vertex buffer was not found for the current device -> create it.		
		if (it == mMapDeviceToBufferResources.end() || it->second->mBuffer == NULL)		
		{						
			createBuffer(d3d9Device, mBufferDesc.Pool);
			it = mMapDeviceToBufferResources.find(d3d9Device);			
		}

		if (it->second->mOutOfDate)
			updateBufferResources(mSystemMemoryBuffer, it->second);

		it->second->mLastUsedFrame = Root::getSingleton().getNextFrameNumber();

		return it->second->mBuffer;
	}	
	//---------------------------------------------------------------------
	bool D3D9HardwareVertexBuffer::updateBufferResources(const char* systemMemoryBuffer,
		BufferResources* bufferResources)
	{		
		assert(bufferResources != NULL);
		assert(bufferResources->mBuffer != NULL);
		assert(bufferResources->mOutOfDate);
				
		void* dstBytes;
		HRESULT hr;
		

		// Lock the buffer.
		hr = bufferResources->mBuffer->Lock(
			static_cast<UINT>(bufferResources->mLockOffset), 
			static_cast<UINT>(bufferResources->mLockLength), 
			&dstBytes,
			D3D9Mappings::get(bufferResources->mLockOptions, mUsage));

		if (FAILED(hr))
		{
			String msg = DXGetErrorDescription9(hr);
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"Cannot lock D3D9 vertex buffer: " + msg, 
				"D3D9HardwareVertexBuffer::updateBuffer");
		}

		memcpy(dstBytes, systemMemoryBuffer + bufferResources->mLockOffset, bufferResources->mLockLength);

		// Unlock the buffer.
		hr = bufferResources->mBuffer->Unlock();
		if (FAILED(hr))
		{
			String msg = DXGetErrorDescription9(hr);
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"Cannot unlock D3D9 vertex buffer: " + msg, 
				"D3D9HardwareVertexBuffer::updateBuffer");
		}

		bufferResources->mOutOfDate = false;
		bufferResources->mLockOffset = mSizeInBytes;
		bufferResources->mLockLength = 0;
		bufferResources->mLockOptions = HBL_NORMAL;

		return true;		
	}
}
