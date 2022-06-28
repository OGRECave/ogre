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

#ifndef _OgreVulkanDevice_H_
#define _OgreVulkanDevice_H_

#include "OgreVulkanPrerequisites.h"

#include "OgreVulkanQueue.h"

#include "OgreHeaderPrefix.h"

namespace Ogre
{
    struct _OgreVulkanExport VulkanDevice
    {
        struct SelectedQueue
        {
            VulkanQueue::QueueFamily usage;
            uint32 familyIdx;
            uint32 queueIdx;
            SelectedQueue();
        };

        // clang-format off
        VkInstance          mInstance;
        VkPhysicalDevice    mPhysicalDevice;
        VkDevice            mDevice;

        VkQueue             mPresentQueue;
        /// Graphics queue is *guaranteed by spec* to also be able to run compute and transfer
        /// A GPU may not have a graphics queue though (Ogre can't run there)
        VulkanQueue             mGraphicsQueue;
        /// Additional compute queues to run async compute (besides the main graphics one)
        FastArray<VulkanQueue>  mComputeQueues;
        /// Additional transfer queues to run async transfers (besides the main graphics one)
        FastArray<VulkanQueue>  mTransferQueues;
        // clang-format on

        VkPhysicalDeviceProperties mDeviceProperties;
        VkPhysicalDeviceMemoryProperties mDeviceMemoryProperties;
        VkPhysicalDeviceFeatures mDeviceFeatures;
        FastArray<VkQueueFamilyProperties> mQueueProps;

        VulkanRenderSystem *mRenderSystem;

        uint32 mSupportedStages;

        static void destroyQueues( FastArray<VulkanQueue> &queueArray );

        void findGraphicsQueue( FastArray<uint32> &inOutUsedQueueCount );
        void findComputeQueue( FastArray<uint32> &inOutUsedQueueCount, uint32 maxNumQueues );
        void findTransferQueue( FastArray<uint32> &inOutUsedQueueCount, uint32 maxNumQueues );

        void fillQueueCreationInfo( uint32 maxComputeQueues, uint32 maxTransferQueues,
                                    FastArray<VkDeviceQueueCreateInfo> &outQueueCiArray );

    public:
        VulkanDevice( VkInstance instance, uint32 deviceIdx, VulkanRenderSystem *renderSystem );
        ~VulkanDevice();

        static VkInstance createInstance( FastArray<const char *> &extensions,
                                          FastArray<const char *> &layers,
                                          PFN_vkDebugReportCallbackEXT debugCallback );

        void createPhysicalDevice( uint32 deviceIdx );

        void createDevice( FastArray<const char *> &extensions, uint32 maxComputeQueues,
                           uint32 maxTransferQueues );

        void initQueues( void );

        void commitAndNextCommandBuffer(
            SubmissionType::SubmissionType submissionType = SubmissionType::FlushOnly );

        /// Waits for the GPU to finish all pending commands.
        void stall( void );
    };
}  // namespace Ogre

#include "OgreHeaderSuffix.h"

#endif
