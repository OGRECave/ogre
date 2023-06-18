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
#include "OgreD3D11HardwareBuffer.h"
#include "OgreD3D11Mappings.h"
#include "OgreD3D11Device.h"
#include "OgreException.h"
namespace Ogre {

    //---------------------------------------------------------------------
    D3D11HardwareBuffer::D3D11HardwareBuffer(
        BufferType btype, size_t sizeBytes,
        HardwareBuffer::Usage usage, D3D11Device & device, 
        bool useShadowBuffer, bool streamOut)
        : HardwareBuffer(usage, useShadowBuffer),
        mUseTempStagingBuffer(false),
        mBufferType(btype),
        mDevice(device)
    {
        mSizeInBytes = sizeBytes;
        mDesc.ByteWidth = static_cast<UINT>(sizeBytes);
        mDesc.CPUAccessFlags = 0;
        mDesc.MiscFlags = 0;

        if (usage == HBU_CPU_ONLY)
        {
            mDesc.Usage = D3D11_USAGE_STAGING;
            //A D3D11_USAGE_STAGING Resource cannot be bound to any parts of the graphics pipeline, so therefore cannot have any BindFlags bits set.
            mDesc.BindFlags = 0;
            mDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ ;// A D3D11_USAGE_STAGING Resource must have at least one CPUAccessFlag bit set.

        }
        else
        {
            mDesc.Usage = D3D11Mappings::_getUsage(mUsage);
            mDesc.BindFlags = btype == VERTEX_BUFFER ? D3D11_BIND_VERTEX_BUFFER : 
                              btype == INDEX_BUFFER  ? D3D11_BIND_INDEX_BUFFER  :
                                                       D3D11_BIND_CONSTANT_BUFFER;
        }
        // Better check of stream out flag
        if (streamOut && btype != CONSTANT_BUFFER)
        {
            mDesc.BindFlags |= D3D11_BIND_STREAM_OUTPUT;
        }

        if (usage == HBU_CPU_TO_GPU)
        {
            // We want to be able to map this buffer
            mDesc.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;
            mDesc.Usage = D3D11_USAGE_DYNAMIC;
        }

        // Note that in D3D11 you can only directly lock (map) a dynamic resource
        // directly for writing. You can only map for read / write staging resources,
        // which themselves cannot be used for input / output to the GPU. Thus
        // for any locks except write locks on dynamic resources, we have to use
        // temporary staging resources instead and use async copies.

        // TODO: we can explicitly initialise the buffer contents here if we like
        // not doing this since OGRE doesn't support this model yet
        OGRE_CHECK_DX_ERROR(device->CreateBuffer(&mDesc, NULL, mlpD3DBuffer.ReleaseAndGetAddressOf()));

        // Create shadow buffer
        if (useShadowBuffer)
        {
            mShadowBuffer.reset(new D3D11HardwareBuffer(mBufferType,
                    mSizeInBytes, HBU_CPU_ONLY, mDevice, false, false));
        }

    }
    //---------------------------------------------------------------------
    D3D11HardwareBuffer::~D3D11HardwareBuffer() {}
    //---------------------------------------------------------------------
    void* D3D11HardwareBuffer::lockImpl(size_t offset, 
        size_t length, LockOptions options)
    {
        if (mUsage == HBU_CPU_ONLY || (mUsage & HardwareBuffer::HBU_DYNAMIC &&
            (options == HardwareBuffer::HBL_DISCARD || options == HardwareBuffer::HBL_NO_OVERWRITE)))
        {
            // Staging (system memory) buffers or dynamic, write-only buffers 
            // are the only case where we can lock directly
            // We have no 'lock for writing' but discard + locking the entire
            // buffer essentially means the same thing, especially since it's
            // not possible to map part of a buffer in Dx11

            // map directly
            D3D11_MAP mapType;
            switch(options)
            {
            case HBL_DISCARD:
                // To use D3D11_MAP_WRITE_DISCARD resource must have been created with write access and dynamic usage.
                mapType = mUsage == HBU_CPU_ONLY ? D3D11_MAP_WRITE : D3D11_MAP_WRITE_DISCARD;
                break;
            case HBL_NO_OVERWRITE:
                // To use D3D11_MAP_WRITE_NO_OVERWRITE resource must have been created with write access.
                // TODO: check (D3D11_USAGE_STAGING => D3D11_MAP_WRITE_NO_OVERWRITE) combo - it`s not forbidden by MSDN
                mapType = mUsage == HBU_CPU_ONLY ? D3D11_MAP_WRITE : D3D11_MAP_WRITE_NO_OVERWRITE;
                break;
            case HBL_NORMAL:
                mapType = (mDesc.CPUAccessFlags & D3D11_CPU_ACCESS_READ) ? D3D11_MAP_READ_WRITE : D3D11_MAP_WRITE;
                break;
            case HBL_READ_ONLY:
                mapType = D3D11_MAP_READ;
                break;
            case HBL_WRITE_ONLY:
                mapType = D3D11_MAP_WRITE;
                break;
            }

            void * pRet = NULL;
            D3D11_MAPPED_SUBRESOURCE mappedSubResource;
            mappedSubResource.pData = NULL;
            OGRE_CHECK_DX_ERROR(mDevice.GetImmediateContext()->Map(mlpD3DBuffer.Get(), 0, mapType, 0, &mappedSubResource));

            pRet = static_cast<void*>(static_cast<char*>(mappedSubResource.pData) + offset);

            return pRet;

        }
        else
        {
            mIsLocked = false; // locking staging buffer instead
            mUseTempStagingBuffer = true;
            OgreAssertDbg(!mShadowBuffer,
                          "we should never arrive here, when already having a shadow buffer");
            // create temporary shadow buffer
            mShadowBuffer.reset(new D3D11HardwareBuffer(mBufferType,
                    mSizeInBytes, HBU_CPU_ONLY, mDevice, false, false));

            // schedule a copy to the staging
            if (options != HBL_DISCARD)
                mShadowBuffer->copyData(*this, offset, offset, length, true);

            // register whether we'll need to upload on unlock
            mShadowUpdated = (options != HBL_READ_ONLY);

            return mShadowBuffer->lock(offset, length, options);
        }
    }
    //---------------------------------------------------------------------
    void D3D11HardwareBuffer::unlockImpl(void)
    {
        OgreAssertDbg(!mUseTempStagingBuffer, "should be handled by _updateFromShadow");

        // unmap
        mDevice.GetImmediateContext()->Unmap(mlpD3DBuffer.Get(), 0);
    }
    //---------------------------------------------------------------------
    void D3D11HardwareBuffer::copyData(HardwareBuffer& srcBuffer, size_t srcOffset, 
        size_t dstOffset, size_t length, bool discardWholeBuffer)
    {
		if (mShadowBuffer)
		{
			mShadowBuffer->copyData(srcBuffer, srcOffset, dstOffset, length, discardWholeBuffer);
		}
		copyDataImpl(srcBuffer, srcOffset, dstOffset, length, discardWholeBuffer);
	}
	//---------------------------------------------------------------------
	void D3D11HardwareBuffer::copyDataImpl(HardwareBuffer& srcBuffer, size_t srcOffset,
		size_t dstOffset, size_t length, bool discardWholeBuffer)
	{
        // If we're copying same-size buffers in their entirety...
        if (srcOffset == 0 && dstOffset == 0 &&
            length == mSizeInBytes && mSizeInBytes == srcBuffer.getSizeInBytes())
        {
            // schedule hardware buffer copy
            mDevice.GetImmediateContext()->CopyResource(mlpD3DBuffer.Get(), static_cast<D3D11HardwareBuffer&>(srcBuffer).getD3DBuffer());
            if (mDevice.isError())
            {
                String errorDescription = mDevice.getErrorDescription();
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                    "Cannot copy D3D11 resource\nError Description:" + errorDescription,
                    "D3D11HardwareBuffer::copyData");
            }
        }
        else
        {
            // copy subregion
            D3D11_BOX srcBox;
            srcBox.left = (UINT)srcOffset;
            srcBox.right = (UINT)srcOffset + length;
            srcBox.top = 0;
            srcBox.bottom = 1;
            srcBox.front = 0;
            srcBox.back = 1;

            mDevice.GetImmediateContext()->CopySubresourceRegion(mlpD3DBuffer.Get(), 0, (UINT)dstOffset, 0, 0, 
                static_cast<D3D11HardwareBuffer&>(srcBuffer).getD3DBuffer(), 0, &srcBox);
            if (mDevice.isError())
            {
                String errorDescription = mDevice.getErrorDescription();
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                    "Cannot copy D3D11 subresource region\nError Description:" + errorDescription,
                    "D3D11HardwareBuffer::copyData");
            }
		}
	}
	//---------------------------------------------------------------------
	void D3D11HardwareBuffer::_updateFromShadow(void)
	{
		if(mShadowBuffer && mShadowUpdated && !mSuppressHardwareUpdate)
		{
			bool discardWholeBuffer = mLockStart == 0 && mLockSize == mSizeInBytes;
			copyDataImpl(*mShadowBuffer, mLockStart, mLockStart, mLockSize, discardWholeBuffer);
			mShadowUpdated = false;
        }

        if (mUseTempStagingBuffer)
        {
            // delete
            mShadowBuffer.reset();
            mUseTempStagingBuffer = false;
        }
    }
    //---------------------------------------------------------------------
    void D3D11HardwareBuffer::readData(size_t offset, size_t length, 
        void* pDest)
    {
        // There is no functional interface in D3D, just do via manual 
        // lock, copy & unlock
        HardwareBufferLockGuard thisLock(this, offset, length, HardwareBuffer::HBL_READ_ONLY);
        memcpy(pDest, thisLock.pData, length);
    }
    //---------------------------------------------------------------------
    void D3D11HardwareBuffer::writeData(size_t offset, size_t length, 
        const void* pSource,
        bool discardWholeBuffer)
    {
        // There is no functional interface in D3D, just do via manual 
        // lock, copy & unlock
        HardwareBufferLockGuard thisLock(this, offset, length,
            discardWholeBuffer ? HardwareBuffer::HBL_DISCARD : HardwareBuffer::HBL_NORMAL);
        memcpy(thisLock.pData, pSource, length);

        //What if we try UpdateSubresource
        //mDevice.GetImmediateContext()->UpdateSubresource(mlpD3DBuffer.Get(), 0, NULL, pSource, offset, length);
    }
    //---------------------------------------------------------------------

}
