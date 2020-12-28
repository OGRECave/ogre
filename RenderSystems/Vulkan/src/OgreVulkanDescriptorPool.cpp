/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-present Torus Knot Software Ltd

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

#include "OgreVulkanDescriptorPool.h"

#include "OgreVulkanDevice.h"
#include "OgreVulkanRootLayout.h"
#include "OgreVulkanUtils.h"
#include "Vao/OgreVulkanVaoManager.h"

#include "OgreException.h"

#include "vulkan/vulkan_core.h"

namespace Ogre
{
    VulkanDescriptorPool::VulkanDescriptorPool::Pool::Pool( size_t _capacity ) :
        pool( VK_NULL_HANDLE ),
        size( 0u ),
        capacity( _capacity )
    {
    }
    //-------------------------------------------------------------------------
    VulkanDescriptorPool::VulkanDescriptorPool( VulkanVaoManager *vaoManager,
                                                const VulkanRootLayout *rootLayout, size_t setIdx,
                                                const size_t capacity ) :
        mCurrentCapacity( 0u ),
        mCurrentPoolIdx( 0u ),
        mLastFrameUsed( vaoManager->getFrameCount() - vaoManager->getDynamicBufferMultiplier() ),
        mAdvanceFrameScheduled( false ),
        mVaoManager( vaoManager )
    {
        const DescBindingRange *descBindingRanges = rootLayout->getDescBindingRanges( setIdx );

        size_t numPoolSizes = 0u;
        for( size_t i = 0u; i < DescBindingTypes::NumDescBindingTypes; ++i )
        {
            if( descBindingRanges[i].isInUse() )
                ++numPoolSizes;
        }

        mPoolSizes.resize( numPoolSizes );

        size_t poolIdx = 0u;
        for( size_t i = 0u; i < DescBindingTypes::NumDescBindingTypes; ++i )
        {
            if( descBindingRanges[i].isInUse() )
            {
                mPoolSizes[poolIdx].type = static_cast<VkDescriptorType>(
                    toVkDescriptorType( static_cast<DescBindingTypes::DescBindingTypes>( i ) ) );
                mPoolSizes[poolIdx].descriptorCount =
                    static_cast<uint32>( descBindingRanges[i].getNumUsedSlots() );
                ++poolIdx;
            }
        }

        createNewPool( vaoManager->getDevice(), capacity );
    }
    //-------------------------------------------------------------------------
    VulkanDescriptorPool::~VulkanDescriptorPool()
    {
        OGRE_ASSERT_LOW( mPools.empty() && "Call deinitialize first!" );
    }
    //-------------------------------------------------------------------------
    void VulkanDescriptorPool::deinitialize( VulkanDevice *device )
    {
        FastArray<Pool>::const_iterator itor = mPools.begin();
        FastArray<Pool>::const_iterator endt = mPools.end();

        while( itor != endt )
        {
            vkDestroyDescriptorPool( device->mDevice, itor->pool, 0 );
            ++itor;
        }
        mPools.clear();
    }
    //-------------------------------------------------------------------------
    void VulkanDescriptorPool::createNewPool( VulkanDevice *device )
    {
        const size_t newCapacity = mCurrentCapacity + ( mCurrentCapacity >> 1u ) + 1u;
        createNewPool( device, newCapacity );
    }
    //-------------------------------------------------------------------------
    void VulkanDescriptorPool::createNewPool( VulkanDevice *device, const size_t newCapacity )
    {
        VkDescriptorPoolCreateInfo poolCi;
        makeVkStruct( poolCi, VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO );

        const size_t maxNumDescTypes = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT + 1u;
        VkDescriptorPoolSize poolSizes[maxNumDescTypes];
        OGRE_ASSERT_HIGH( mPoolSizes.size() < maxNumDescTypes );

        const size_t numPoolSizes = mPoolSizes.size();
        for( size_t i = 0u; i < numPoolSizes; ++i )
        {
            poolSizes[i] = mPoolSizes[i];
            poolSizes[i].descriptorCount *= newCapacity;
        }

        // We do not set FREE_DESCRIPTOR_SET_BIT as we do not need to free individual descriptor sets
        poolCi.flags = 0;
        poolCi.poolSizeCount = static_cast<uint32>( numPoolSizes );
        poolCi.pPoolSizes = poolSizes;
        poolCi.maxSets = static_cast<uint32_t>( newCapacity - mCurrentCapacity );
        mCurrentCapacity = newCapacity;

        Pool pool( poolCi.maxSets );

        // Create the Vulkan descriptor pool
        VkResult result = vkCreateDescriptorPool( device->mDevice, &poolCi, 0, &pool.pool );
        checkVkResult( result, "vkCreateDescriptorPool" );

        mPools.push_back( pool );
        mCurrentPoolIdx = mPools.size() - 1u;
    }
    //-------------------------------------------------------------------------
    VkDescriptorSet VulkanDescriptorPool::allocate( VulkanDevice *device,
                                                    VkDescriptorSetLayout setLayout )
    {
        OGRE_ASSERT_HIGH( isAvailableInCurrentFrame() );

        if( !mAdvanceFrameScheduled )
            reset( device );

        while( mCurrentPoolIdx < mPools.size() && mPools[mCurrentPoolIdx].isFull() )
            ++mCurrentPoolIdx;

        if( mCurrentPoolIdx >= mPools.size() )
            createNewPool( device );

        Pool &pool = mPools[mCurrentPoolIdx];

        VkDescriptorSetAllocateInfo allocInfo;
        makeVkStruct( allocInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO );

        allocInfo.descriptorPool = pool.pool;
        allocInfo.descriptorSetCount = 1u;
        allocInfo.pSetLayouts = &setLayout;

        VkDescriptorSet handle = VK_NULL_HANDLE;

        // Allocate a new descriptor set from the current pool
        VkResult result = vkAllocateDescriptorSets( device->mDevice, &allocInfo, &handle );
        if( result != VK_SUCCESS )
        {
            LogManager::getSingleton().logMessage(
                "ERROR: vkAllocateDescriptorSets failed! Out of Memory?", LML_CRITICAL );
            return VK_NULL_HANDLE;
        }

        if( !mAdvanceFrameScheduled )
        {
            mVaoManager->_schedulePoolAdvanceFrame( this );
            mAdvanceFrameScheduled = true;
        }

        ++pool.size;

        return handle;
    }
    //-------------------------------------------------------------------------
    void VulkanDescriptorPool::reset( VulkanDevice *device )
    {
        FastArray<Pool>::iterator itor = mPools.begin();
        FastArray<Pool>::iterator endt = mPools.end();

        while( itor != endt )
        {
            vkResetDescriptorPool( device->mDevice, itor->pool, 0 );
            itor->size = 0u;
            ++itor;
        }

        mCurrentPoolIdx = 0u;
    }
    //-------------------------------------------------------------------------
    void VulkanDescriptorPool::_advanceFrame( void )
    {
        mLastFrameUsed = mVaoManager->getFrameCount();
        mAdvanceFrameScheduled = false;
    }
    //-------------------------------------------------------------------------
    bool VulkanDescriptorPool::isAvailableInCurrentFrame( void ) const
    {
        return mVaoManager->isFrameFinished( mLastFrameUsed );
    }
}  // namespace Ogre
