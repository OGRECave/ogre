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
#include "OgreD3D10HardwareBuffer.h"
#include "OgreD3D10Mappings.h"
#include "OgreD3D10Device.h"
#include "OgreException.h"
namespace Ogre {

	//---------------------------------------------------------------------
	D3D10HardwareBuffer::D3D10HardwareBuffer(
		BufferType btype, size_t sizeBytes,
		HardwareBuffer::Usage usage, D3D10Device & device, 
		bool useSystemMemory, bool useShadowBuffer)
		: HardwareBuffer(usage, useSystemMemory,  false /* TODO: useShadowBuffer*/),
		mlpD3DBuffer(0),
		mpTempStagingBuffer(0),
		mUseTempStagingBuffer(false),
		mBufferType(btype),
		mDevice(device)
	{
		mSizeInBytes = sizeBytes;
		mDesc.ByteWidth = static_cast<UINT>(sizeBytes);
		mDesc.CPUAccessFlags = D3D10Mappings::_getAccessFlags(mUsage); 

		if (useSystemMemory)
		{
			mDesc.Usage = D3D10_USAGE_STAGING;
			//A D3D10_USAGE_STAGING Resource cannot be bound to any parts of the graphics pipeline, so therefore cannot have any BindFlags bits set.
			mDesc.BindFlags = 0;
			mDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE | D3D10_CPU_ACCESS_READ ;// A D3D10_USAGE_STAGING Resource must have at least one CPUAccessFlag bit set.

		}
		else
		{
			mDesc.Usage = D3D10Mappings::_getUsage(mUsage);
			mDesc.BindFlags = btype == VERTEX_BUFFER ? D3D10_BIND_VERTEX_BUFFER : D3D10_BIND_INDEX_BUFFER;

		}

		mDesc.MiscFlags = 0;
		if (!useSystemMemory && (usage | HardwareBuffer::HBU_DYNAMIC))
		{
			// We want to be able to map this buffer
			mDesc.CPUAccessFlags |= D3D10_CPU_ACCESS_WRITE;
			mDesc.Usage = D3D10_USAGE_DYNAMIC;
		}

		// Note that in D3D10 you can only directly lock (map) a dynamic resource
		// directly for writing. You can only map for read / write staging resources,
		// which themselves cannot be used for input / output to the GPU. Thus
		// for any locks except write locks on dynamic resources, we have to use
		// temporary staging resources instead and use async copies.
		// We use the 'useSystemMemory' option to indicate a staging resource


		// TODO: we can explicitly initialise the buffer contents here if we like
		// not doing this since OGRE doesn't support this model yet
		HRESULT hr = device->CreateBuffer( &mDesc, 0, &mlpD3DBuffer );
		if (FAILED(hr) || mDevice.isError())
		{
			String msg = device.getErrorDescription(hr);
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"Cannot create D3D10 buffer: " + msg, 
				"D3D10HardwareBuffer::D3D10HardwareBuffer");
		}


	}
	//---------------------------------------------------------------------
	D3D10HardwareBuffer::~D3D10HardwareBuffer()
	{
		SAFE_RELEASE(mlpD3DBuffer);
		SAFE_DELETE(mpTempStagingBuffer); // should never be nonzero unless destroyed while locked

	}
	//---------------------------------------------------------------------
	void* D3D10HardwareBuffer::lockImpl(size_t offset, 
		size_t length, LockOptions options)
	{
		if (length > mSizeInBytes)
		{
			// need to realloc
			mDesc.ByteWidth = static_cast<UINT>(mSizeInBytes);
			HRESULT hr = mDevice->CreateBuffer( &mDesc, 0, &mlpD3DBuffer );
			if (FAILED(hr) || mDevice.isError())
			{
				String msg = mDevice.getErrorDescription(hr);
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
					"Cannot create D3D10 buffer: " + msg, 
					"D3D10HardwareBuffer::D3D10HardwareBuffer");
			}
		}


		if (mSystemMemory ||
			(mUsage & HardwareBuffer::HBU_DYNAMIC && 
			(options == HardwareBuffer::HBL_DISCARD)))
		{
			// Staging (system memory) buffers or dynamic, write-only buffers 
			// are the only case where we can lock directly
			// We have no 'lock for writing' but discard + locking the entire
			// buffer essentially means the same thing, especially since it's
			// not possible to map part of a buffer in Dx10

			// map directly
			D3D10_MAP mapType;
			switch(options)
			{
			case HBL_DISCARD:
				if (!mSystemMemory && (mUsage | HardwareBuffer::HBU_DYNAMIC))
				{
					// Map cannot be called with MAP_WRITE access, 
					// because the Resource was created as D3D10_USAGE_DYNAMIC. 
					// D3D10_USAGE_DYNAMIC Resources must use either MAP_WRITE_DISCARD 
					// or MAP_WRITE_NO_OVERWRITE with Map.
					mapType = D3D10_MAP_WRITE_DISCARD;
				}
				else
				{
					// Map cannot be called with MAP_WRITE_DISCARD access, 
					// because the Resource was not created as D3D10_USAGE_DYNAMIC. 
					// D3D10_USAGE_DYNAMIC Resources must use either MAP_WRITE_DISCARD 
					// or MAP_WRITE_NO_OVERWRITE with Map.
					mapType = D3D10_MAP_WRITE;
				}
				break;
			case HBL_NO_OVERWRITE:
				if (mSystemMemory)
				{
					// map cannot be called with MAP_WRITE_NO_OVERWRITE access, because the Resource was not created as D3D10_USAGE_DYNAMIC. D3D10_USAGE_DYNAMIC Resources must use either MAP_WRITE_DISCARD or MAP_WRITE_NO_OVERWRITE with Map.
					mapType = D3D10_MAP_READ_WRITE;
				}
				else
				{
					mapType = D3D10_MAP_WRITE_NO_OVERWRITE;
				}
				break;
			case HBL_NORMAL:
				if (mDesc.CPUAccessFlags & D3D10_CPU_ACCESS_READ)
				{
					mapType = D3D10_MAP_READ_WRITE;
				}
				else
				{
					mapType = D3D10_MAP_WRITE_DISCARD;
				}
				break;
			case HBL_READ_ONLY:
				mapType = D3D10_MAP_READ;
				break;
			}

			void* pRet;
			mDevice.clearStoredErrorMessages();
			HRESULT hr = mlpD3DBuffer->Map(mapType, 0, &pRet);
			if (FAILED(hr) || mDevice.isError())
			{
				String msg = mDevice.getErrorDescription(hr);
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
					"Error calling Map: " + msg, 
					"D3D10HardwareBuffer::lockImpl");
			}

			// In Dx10 we can only map entire buffer, not subregions, 
			// but we can offset the pointer for compat
			if (offset)
			{
				pRet = static_cast<void*>(static_cast<char*>(pRet) + offset);
			}

			return pRet;

		}
		else
		{
			mUseTempStagingBuffer = true;
			if (!mpTempStagingBuffer)
			{
				// create another buffer instance but use system memory
				mpTempStagingBuffer = new D3D10HardwareBuffer(mBufferType, 
					mSizeInBytes, mUsage, mDevice, true, false);
			}

			// schedule a copy to the staging
			if (options != HBL_DISCARD)
				mpTempStagingBuffer->copyData(*this, 0, 0, mSizeInBytes, true);

			// register whether we'll need to upload on unlock
			mStagingUploadNeeded = (options != HBL_READ_ONLY);

			return mpTempStagingBuffer->lock(offset, length, options);


		}
	}
	//---------------------------------------------------------------------
	void D3D10HardwareBuffer::unlockImpl(void)
	{

		if (mUseTempStagingBuffer)
		{
			mUseTempStagingBuffer = false;

			// ok, we locked the staging buffer
			mpTempStagingBuffer->unlock();

			// copy data if needed
			// this is async but driver should keep reference
			if (mStagingUploadNeeded)
				copyData(*mpTempStagingBuffer, 0, 0, mSizeInBytes, true);

			// delete
			// not that efficient, but we should not be locking often
			SAFE_DELETE(mpTempStagingBuffer);
		}
		else
		{
			// unmap
			mlpD3DBuffer->Unmap();
		}
	}
	//---------------------------------------------------------------------
	void D3D10HardwareBuffer::copyData(HardwareBuffer& srcBuffer, size_t srcOffset, 
		size_t dstOffset, size_t length, bool discardWholeBuffer)
	{
		// If we're copying same-size buffers in their entirety...
		if (srcOffset == 0 && dstOffset == 0 &&
			length == mSizeInBytes && mSizeInBytes == srcBuffer.getSizeInBytes())
		{
			// schedule hardware buffer copy
			mDevice->CopyResource(mlpD3DBuffer, static_cast<D3D10HardwareBuffer&>(srcBuffer).getD3DBuffer());
			if (mDevice.isError())
			{
				String errorDescription = mDevice.getErrorDescription();
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
					"Cannot copy D3D10 resource\nError Description:" + errorDescription,
					"D3D10HardwareBuffer::copyData");
			}
		}
		else
		{
			// copy subregion
			D3D10_BOX srcBox;
			srcBox.left = (UINT)srcOffset;
			srcBox.right = (UINT)srcOffset + length;
			srcBox.top = 0;
			srcBox.bottom = 1;
			srcBox.front = 0;
			srcBox.back = 1;

			mDevice->CopySubresourceRegion(mlpD3DBuffer, 0, (UINT)dstOffset, 0, 0, 
				static_cast<D3D10HardwareBuffer&>(srcBuffer).getD3DBuffer(), 0, &srcBox);
			if (mDevice.isError())
			{
				String errorDescription = mDevice.getErrorDescription();
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
					"Cannot copy D3D10 subresource region\nError Description:" + errorDescription,
					"D3D10HardwareBuffer::copyData");
			}
		}
	}
	//---------------------------------------------------------------------
	void D3D10HardwareBuffer::readData(size_t offset, size_t length, 
		void* pDest)
	{
		// There is no functional interface in D3D, just do via manual 
		// lock, copy & unlock
		void* pSrc = this->lock(offset, length, HardwareBuffer::HBL_READ_ONLY);
		memcpy(pDest, pSrc, length);
		this->unlock();

	}
	//---------------------------------------------------------------------
	void D3D10HardwareBuffer::writeData(size_t offset, size_t length, 
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

}
