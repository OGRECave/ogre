/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

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
----------------------------------------------------------------------------
*/
#include "OgreD3D9HardwareIndexBuffer.h"
#include "OgreD3D9Mappings.h"
#include "OgreException.h"
#include "OgreD3D9HardwareBufferManager.h"
#include "OgreD3D9RenderSystem.h"
#include "OgreRoot.h"
#include "OgreD3D9Device.h"
#include "OgreD3D9ResourceManager.h"

namespace Ogre {

	//---------------------------------------------------------------------
    D3D9HardwareIndexBuffer::D3D9HardwareIndexBuffer(HardwareBufferManagerBase* mgr, HardwareIndexBuffer::IndexType idxType, 
        size_t numIndexes, HardwareBuffer::Usage usage,
        bool useSystemMemory, bool useShadowBuffer)
        : HardwareIndexBuffer(mgr, idxType, numIndexes, usage, useSystemMemory, 
		useShadowBuffer || 
		// Allocate the system memory buffer for restoring after device lost.
		(((usage & HardwareBuffer::HBU_WRITE_ONLY) != 0) && 
			D3D9RenderSystem::getResourceManager()->getAutoHardwareBufferManagement()))
   {
		D3D9_DEVICE_ACCESS_CRITICAL_SECTION

		D3DPOOL eResourcePool;

#if OGRE_D3D_MANAGE_BUFFERS
		eResourcePool = useSystemMemory? D3DPOOL_SYSTEMMEM : 
			// If not system mem, use managed pool UNLESS buffer is discardable
			// if discardable, keeping the software backing is expensive
			((usage & HardwareBuffer::HBU_DISCARDABLE) || (D3D9RenderSystem::isDirectX9Ex())) ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED;
#else
		eResourcePool = useSystemMemory? D3DPOOL_SYSTEMMEM : D3DPOOL_DEFAULT;
#endif

		// Set the desired memory pool.
		mBufferDesc.Pool = eResourcePool;
				
		// Set source buffer to NULL.
		mSourceBuffer = NULL;
		mSourceLockedBytes  = NULL;

		// Create buffer resource(s).
		for (uint i = 0; i < D3D9RenderSystem::getResourceCreationDeviceCount(); ++i)
		{
			IDirect3DDevice9* d3d9Device = D3D9RenderSystem::getResourceCreationDevice(i);

			createBuffer(d3d9Device, mBufferDesc.Pool);
		}					
    }
	//---------------------------------------------------------------------
    D3D9HardwareIndexBuffer::~D3D9HardwareIndexBuffer()
    {
		D3D9_DEVICE_ACCESS_CRITICAL_SECTION

		DeviceToBufferResourcesIterator it = mMapDeviceToBufferResources.begin();

		while (it != mMapDeviceToBufferResources.end())
		{
			SAFE_RELEASE(it->second->mBuffer);
			if (it->second != NULL)
			{
				OGRE_FREE (it->second, MEMCATEGORY_RENDERSYS);
				it->second = NULL;
			}
			++it;
		}	
		mMapDeviceToBufferResources.clear();   
	}
	//---------------------------------------------------------------------
    void* D3D9HardwareIndexBuffer::lockImpl(size_t offset, 
        size_t length, LockOptions options)
    {		
		D3D9_DEVICE_ACCESS_CRITICAL_SECTION
		
		DeviceToBufferResourcesIterator it = mMapDeviceToBufferResources.begin();

		while (it != mMapDeviceToBufferResources.end())
		{
			BufferResources* bufferResources = it->second;

			if (options != HBL_READ_ONLY)
				bufferResources->mOutOfDate = true;

			// Case it is the first buffer lock in this frame.
			if (bufferResources->mLockLength == 0)
			{
				if (offset < bufferResources->mLockOffset)
					bufferResources->mLockOffset = offset;
				if (length > bufferResources->mLockLength)
					bufferResources->mLockLength = length;
			}

			// Case buffer already locked in this frame.
			else
			{
				size_t highPoint = std::max( offset + length, 
					bufferResources->mLockOffset + bufferResources->mLockLength );
				bufferResources->mLockOffset = std::min( bufferResources->mLockOffset, offset );
				bufferResources->mLockLength = highPoint - bufferResources->mLockOffset;
			}			
					
			bufferResources->mLockOptions = options;
		
			++it;
		}		

		// Lock the source buffer.
		mSourceLockedBytes = _lockBuffer(mSourceBuffer, mSourceBuffer->mLockOffset, mSourceBuffer->mLockLength);

		return mSourceLockedBytes;		
	}
	//---------------------------------------------------------------------
	void D3D9HardwareIndexBuffer::unlockImpl(void)
    {	
		D3D9_DEVICE_ACCESS_CRITICAL_SECTION

		DeviceToBufferResourcesIterator it = mMapDeviceToBufferResources.begin();
		uint nextFrameNumber = Root::getSingleton().getNextFrameNumber();

		while (it != mMapDeviceToBufferResources.end())
		{
			BufferResources* bufferResources = it->second;

			if (bufferResources->mOutOfDate && 
				bufferResources->mBuffer != NULL &&
				nextFrameNumber - bufferResources->mLastUsedFrame <= 1)
			{
				if (mSourceBuffer != bufferResources)
				{
					updateBufferResources(mSourceLockedBytes, bufferResources);
				}				
			}

			++it;
		}	

		// Unlock the source buffer.
		_unlockBuffer(mSourceBuffer);
		mSourceLockedBytes = NULL;
	 }
	//---------------------------------------------------------------------
    void D3D9HardwareIndexBuffer::readData(size_t offset, size_t length, 
        void* pDest)
    {
       // There is no functional interface in D3D, just do via manual 
        // lock, copy & unlock
        void* pSrc = this->lock(offset, length, HardwareBuffer::HBL_READ_ONLY);
        memcpy(pDest, pSrc, length);
        this->unlock();

    }
	//---------------------------------------------------------------------
    void D3D9HardwareIndexBuffer::writeData(size_t offset, size_t length, 
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
	void D3D9HardwareIndexBuffer::notifyOnDeviceCreate(IDirect3DDevice9* d3d9Device)
	{			
		D3D9_DEVICE_ACCESS_CRITICAL_SECTION

		if (D3D9RenderSystem::getResourceManager()->getCreationPolicy() == RCP_CREATE_ON_ALL_DEVICES)
			createBuffer(d3d9Device, mBufferDesc.Pool);	

	}
	//---------------------------------------------------------------------
	void D3D9HardwareIndexBuffer::notifyOnDeviceDestroy(IDirect3DDevice9* d3d9Device)
	{		
		D3D9_DEVICE_ACCESS_CRITICAL_SECTION

		DeviceToBufferResourcesIterator it = mMapDeviceToBufferResources.find(d3d9Device);

		if (it != mMapDeviceToBufferResources.end())	
		{									
			// Case this is the source buffer.
			if (it->second == mSourceBuffer)
			{
				mSourceBuffer = NULL;
			}

			SAFE_RELEASE(it->second->mBuffer);
			if (it->second != NULL)
			{
				OGRE_FREE (it->second, MEMCATEGORY_RENDERSYS);
				it->second = NULL;
			}
			mMapDeviceToBufferResources.erase(it);

			// Case source buffer just destroyed -> switch to another one if exits.
			if (mSourceBuffer == NULL && mMapDeviceToBufferResources.size() > 0)
			{				
				mSourceBuffer = mMapDeviceToBufferResources.begin()->second;				
			}
		}	
	}
	//---------------------------------------------------------------------
	void D3D9HardwareIndexBuffer::notifyOnDeviceLost(IDirect3DDevice9* d3d9Device)
	{		
		D3D9_DEVICE_ACCESS_CRITICAL_SECTION

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
	void D3D9HardwareIndexBuffer::notifyOnDeviceReset(IDirect3DDevice9* d3d9Device)
	{		
		D3D9_DEVICE_ACCESS_CRITICAL_SECTION

		if (mBufferDesc.Pool == D3DPOOL_DEFAULT)
		{
			createBuffer(d3d9Device, mBufferDesc.Pool);		
		}
	}
	//---------------------------------------------------------------------
	void D3D9HardwareIndexBuffer::createBuffer(IDirect3DDevice9* d3d9Device, D3DPOOL ePool)
	{
		D3D9_DEVICE_ACCESS_CRITICAL_SECTION

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
			bufferResources = OGRE_ALLOC_T(BufferResources, 1, MEMCATEGORY_RENDERSYS);			
			mMapDeviceToBufferResources[d3d9Device] = bufferResources;
		}

		bufferResources->mBuffer = NULL;
		bufferResources->mOutOfDate = true;
		bufferResources->mLockOffset = 0;
		bufferResources->mLockLength = getSizeInBytes();
		bufferResources->mLockOptions = HBL_NORMAL;
		bufferResources->mLastUsedFrame = Root::getSingleton().getNextFrameNumber();

		// Create the Index buffer				
		hr = d3d9Device->CreateIndexBuffer(
			static_cast<UINT>(mSizeInBytes),
			D3D9Mappings::get(mUsage),
			D3D9Mappings::get(mIndexType),
			ePool,
			&bufferResources->mBuffer,
			NULL
			);

		if (FAILED(hr))
		{
			String msg = DXGetErrorDescription(hr);
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"Cannot create D3D9 Index buffer: " + msg, 
				"D3D9HardwareIndexBuffer::createBuffer");
		}

		hr = bufferResources->mBuffer->GetDesc(&mBufferDesc);
		if (FAILED(hr))
		{
			String msg = DXGetErrorDescription(hr);
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"Cannot get D3D9 Index buffer desc: " + msg, 
				"D3D9HardwareIndexBuffer::createBuffer");
		}		
		// Update source buffer if need to.
		if (mSourceBuffer == NULL)
		{
			mSourceBuffer = bufferResources;
		}

		// This is a new buffer and source buffer exists we must update the content now 
		// to prevent situation where the source buffer will be destroyed and we won't be able to restore its content.
		else
		{			
			updateBufferContent(bufferResources);			
		}
	}
	//---------------------------------------------------------------------
	IDirect3DIndexBuffer9* D3D9HardwareIndexBuffer::getD3DIndexBuffer(void)
	{		
		IDirect3DDevice9* d3d9Device = D3D9RenderSystem::getActiveD3D9Device();
		DeviceToBufferResourcesIterator it;

		// Find the index buffer of this device.
		it = mMapDeviceToBufferResources.find(d3d9Device);

		// Case index buffer was not found for the current device -> create it.		
		if (it == mMapDeviceToBufferResources.end() || it->second->mBuffer == NULL)		
		{						
			createBuffer(d3d9Device, mBufferDesc.Pool);
			it = mMapDeviceToBufferResources.find(d3d9Device);						
		}

		// Make sure that the buffer content is updated.
		updateBufferContent(it->second);
		
		it->second->mLastUsedFrame = Root::getSingleton().getNextFrameNumber();

		return it->second->mBuffer;
	}

	//---------------------------------------------------------------------
	void D3D9HardwareIndexBuffer::updateBufferContent(BufferResources* bufferResources)
	{
		if (bufferResources->mOutOfDate)
		{
			if (mShadowBuffer != NULL)
			{
				const char* shadowData = (const char*)mShadowBuffer->lock(HBL_NORMAL);
				updateBufferResources(shadowData, bufferResources);
				mShadowBuffer->unlock();
			}
			else if (mSourceBuffer != bufferResources && (mUsage & HardwareBuffer::HBU_WRITE_ONLY) == 0)
			{				
				mSourceBuffer->mLockOptions = HBL_READ_ONLY;
				mSourceLockedBytes = _lockBuffer(mSourceBuffer, 0, mSizeInBytes);
				updateBufferResources(mSourceLockedBytes, bufferResources);
				_unlockBuffer(mSourceBuffer);
				mSourceLockedBytes = NULL;
			}			
		}
	}

	//---------------------------------------------------------------------
	bool D3D9HardwareIndexBuffer::updateBufferResources(const char* systemMemoryBuffer,
		BufferResources* bufferResources)
	{
		assert(bufferResources != NULL);
		assert(bufferResources->mBuffer != NULL);
		assert(bufferResources->mOutOfDate);
			
				
		char* dstBytes = _lockBuffer(bufferResources, bufferResources->mLockOffset, bufferResources->mLockLength);		
		memcpy(dstBytes, systemMemoryBuffer, bufferResources->mLockLength);		
		_unlockBuffer(bufferResources);
				
		return true;		
	}

	//---------------------------------------------------------------------
	char* D3D9HardwareIndexBuffer::_lockBuffer(BufferResources* bufferResources, size_t offset, size_t length)
	{
		HRESULT hr;
		char* pSourceBytes;


		// Lock the buffer.
		hr = bufferResources->mBuffer->Lock(
			static_cast<UINT>(offset), 
			static_cast<UINT>(length), 
			(void**)&pSourceBytes,
			D3D9Mappings::get(mSourceBuffer->mLockOptions, mUsage));

		if (FAILED(hr))
		{
			String msg = DXGetErrorDescription(hr);
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"Cannot lock D3D9 vertex buffer: " + msg, 
				"D3D9HardwareVertexBuffer::_lockBuffer");
		}

		return pSourceBytes;
	}

	//---------------------------------------------------------------------
	void D3D9HardwareIndexBuffer::_unlockBuffer( BufferResources* bufferResources )
	{
		HRESULT hr;

		// Unlock the buffer.
		hr = bufferResources->mBuffer->Unlock();
		if (FAILED(hr))
		{
			String msg = DXGetErrorDescription(hr);
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"Cannot unlock D3D9 vertex buffer: " + msg, 
				"D3D9HardwareVertexBuffer::_unlockBuffer");
		}

		// Reset attributes.
		bufferResources->mOutOfDate = false;
		bufferResources->mLockOffset = mSizeInBytes;
		bufferResources->mLockLength = 0;
		bufferResources->mLockOptions = HBL_NORMAL;

	}
}
