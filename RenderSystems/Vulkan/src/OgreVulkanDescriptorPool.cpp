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
#include "OgreVulkanUtils.h"
#include "OgreException.h"

namespace Ogre
{
    static const int MAX_POOL_CAPACITY = 50;
    //-------------------------------------------------------------------------
    VulkanDescriptorPool::VulkanDescriptorPool(const std::vector<VkDescriptorPoolSize>& poolSizes,
                                               VkDescriptorSetLayout layout, VulkanDevice* device)
        : mLayout(layout), mCurrentPoolIdx(0u), mDevice(device)
    {
        mPoolSizes = poolSizes;

        for(auto& p : mPoolSizes)
            p.descriptorCount *= MAX_POOL_CAPACITY;

        createNewPool();
    }
    //-------------------------------------------------------------------------
    VulkanDescriptorPool::~VulkanDescriptorPool()
    {
        for(auto p : mPools)
        {
            vkDestroyDescriptorPool(mDevice->mDevice, p, 0);
        }
        mPools.clear();
    }
    //-------------------------------------------------------------------------
    void VulkanDescriptorPool::createNewPool()
    {
        VkDescriptorPoolCreateInfo poolCi = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
        poolCi.flags = 0;
        poolCi.poolSizeCount = mPoolSizes.size();
        poolCi.pPoolSizes = mPoolSizes.data();
        poolCi.maxSets = MAX_POOL_CAPACITY;

        mCurrentPoolIdx = mPools.size();

        // Create the Vulkan descriptor pool
        VkDescriptorPool pool;
        OGRE_VK_CHECK(vkCreateDescriptorPool( mDevice->mDevice, &poolCi, 0, &pool ));
        mPools.push_back(pool);
        mDescriptorsUsedPerPool.push_back(0);
    }
    //-------------------------------------------------------------------------
    VkDescriptorSet VulkanDescriptorPool::allocate()
    {
        if(mDescriptorsUsedPerPool[mCurrentPoolIdx] == MAX_POOL_CAPACITY)
            createNewPool();

        VkDescriptorSetAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
        allocInfo.descriptorPool = mPools[mCurrentPoolIdx];
        allocInfo.descriptorSetCount = 1u;
        allocInfo.pSetLayouts = &mLayout;

        VkDescriptorSet handle = VK_NULL_HANDLE;

        // Allocate a new descriptor set from the current pool
        VkResult result = vkAllocateDescriptorSets( mDevice->mDevice, &allocInfo, &handle );
        if( result != VK_SUCCESS )
        {
            LogManager::getSingleton().logError("vkAllocateDescriptorSets failed! Out of Memory?");
            return VK_NULL_HANDLE;
        }

        mDescriptorsUsedPerPool[mCurrentPoolIdx]++;
        return handle;
    }
}  // namespace Ogre
