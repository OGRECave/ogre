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

#include "OgreVulkanHardwareBuffer.h"

#include "OgreVulkanDevice.h"
#include "OgreVulkanUtils.h"

namespace Ogre
{
    VulkanHardwareBuffer::VulkanHardwareBuffer(uint32 target,
        size_t sizeBytes, Usage usage, bool useShadowBuffer, VulkanDevice *device ) :
        HardwareBuffer(usage, false, useShadowBuffer),
        mBuffer(VK_NULL_HANDLE),
        mDevice( device ),
        mTarget( target ),
        mMappedPtr( nullptr )
    {
        mSizeInBytes = sizeBytes;

        discard();

        if(mUsage == HBU_GPU_ONLY)
        {
            mShadowBuffer.reset(new VulkanHardwareBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sizeBytes, HBU_CPU_ONLY, false, mDevice));
        }
    }

    void VulkanHardwareBuffer::discard()
    {
        if(mBuffer)
            mDevice->mGraphicsQueue.queueForDeletion(mBuffer, mMemory);

        VkBufferCreateInfo bufferCi = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bufferCi.size = mSizeInBytes;
        bufferCi.usage = mTarget;

        if((mUsage & HBU_CPU_ONLY) == 0)
            bufferCi.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        OGRE_VK_CHECK(vkCreateBuffer(mDevice->mDevice, &bufferCi, 0, &mBuffer));

        VkMemoryRequirements buffMemRequirements;
        vkGetBufferMemoryRequirements( mDevice->mDevice, mBuffer, &buffMemRequirements );

        VkMemoryAllocateInfo memAllocInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        memAllocInfo.allocationSize = buffMemRequirements.size;

        uint32 reqFlags = 0;
        if(mUsage != HBU_GPU_ONLY)
            reqFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        else
            reqFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        const auto& memProperties = mDevice->mDeviceMemoryProperties;
        for(; memAllocInfo.memoryTypeIndex < memProperties.memoryTypeCount; memAllocInfo.memoryTypeIndex++)
        {
            if ((memProperties.memoryTypes[memAllocInfo.memoryTypeIndex].propertyFlags & reqFlags) == reqFlags)
            {
                break;
            }
        }

        OGRE_VK_CHECK(vkAllocateMemory(mDevice->mDevice, &memAllocInfo, NULL, &mMemory));
        OGRE_VK_CHECK(vkBindBufferMemory(mDevice->mDevice, mBuffer, mMemory, 0));

        if (mUsage == HBU_CPU_TO_GPU)
        {
            OGRE_VK_CHECK(vkMapMemory(mDevice->mDevice, mMemory, 0, mSizeInBytes, 0, &mMappedPtr));
        }
    }

    VulkanHardwareBuffer::~VulkanHardwareBuffer()
    {
        if(mMappedPtr)
            vkUnmapMemory( mDevice->mDevice, mMemory );
        // delay until we are sure this buffer is no longer in use
        mDevice->mGraphicsQueue.queueForDeletion(mBuffer, mMemory);
    }

    void* VulkanHardwareBuffer::lockImpl(size_t offset, size_t length, LockOptions options)
    {
        if(mShadowBuffer)
            return mShadowBuffer->lock(offset, length, options);

        if (options == HBL_DISCARD && mUsage == HBU_CPU_TO_GPU)
            discard();

        if ((options == HBL_READ_ONLY || options == HBL_NORMAL) && mUsage == HBU_GPU_ONLY)
        {
            LogManager::getSingleton().logWarning("HardwareBuffer - UNIMPLEMENTED implicit GPU to HOST copy (slow)");
        }

        if (mMappedPtr) // persistent mapping
            return static_cast<uint8*>(mMappedPtr) + offset;

        void *retPtr = 0;
        OGRE_VK_CHECK(vkMapMemory(mDevice->mDevice, mMemory, 0, mSizeInBytes, 0, &retPtr));
        return static_cast<uint8*>(retPtr) + offset;
    }

    void VulkanHardwareBuffer::unlockImpl()
    {
        OgreAssert(!mShadowBuffer, "should be handled by _updateFromShadow");
        if(mMappedPtr) // persistent mapping
            return;
        vkUnmapMemory( mDevice->mDevice, mMemory );
    }

    void VulkanHardwareBuffer::readData( size_t offset, size_t length, void *pDest )
    {
        // just use memcpy
        HardwareBufferLockGuard thisLock(this, offset, length, HBL_READ_ONLY);
        memcpy(pDest, thisLock.pData, length);
    }

    void VulkanHardwareBuffer::writeData(size_t offset, size_t length, const void* pSource,
                                               bool discardWholeBuffer)
    {
        // just use memcpy
        HardwareBufferLockGuard thisLock(this, offset, length, discardWholeBuffer ? HBL_DISCARD : HBL_NORMAL);
        memcpy(thisLock.pData, pSource, length);
    }

    void VulkanHardwareBuffer::_updateFromShadow()
    {
        if(mShadowBuffer && mShadowUpdated && !mSuppressHardwareUpdate)
		{
			bool discardWholeBuffer = mLockStart == 0 && mLockSize == mSizeInBytes;
			copyData(*mShadowBuffer, mLockStart, mLockStart, mLockSize, discardWholeBuffer);
			mShadowUpdated = false;
        }
    }

    void VulkanHardwareBuffer::copyData( HardwareBuffer& _srcBuffer, size_t srcOffset,
                                               size_t dstOffset, size_t length, bool discardWholeBuffer )
    {
        auto srcBuffer = dynamic_cast<VulkanHardwareBuffer*>( &_srcBuffer );
        if( srcBuffer && (mUsage & HBU_CPU_ONLY) == 0)
        {
            mDevice->mGraphicsQueue.getCopyEncoderV1Buffer( false );

            VkBuffer srcBuf = srcBuffer->getVkBuffer();
            VkBufferCopy region = {srcOffset, dstOffset, length};
            vkCmdCopyBuffer( mDevice->mGraphicsQueue.mCurrentCmdBuffer, srcBuf, mBuffer, 1u, &region );
        }
        else
        {
            const void* srcData = _srcBuffer.lock(srcOffset, length, HBL_READ_ONLY);
            writeData(dstOffset, length, srcData, discardWholeBuffer);
            _srcBuffer.unlock();
        }
    }
}
