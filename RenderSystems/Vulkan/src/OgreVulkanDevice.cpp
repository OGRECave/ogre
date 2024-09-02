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

#include "OgreVulkanDevice.h"

#include "OgreVulkanRenderSystem.h"
#include "OgreVulkanWindow.h"

#include "OgreException.h"
#include "OgreStringConverter.h"

#include "OgreVulkanUtils.h"

#define TODO_findRealPresentQueue

namespace Ogre
{
    VulkanDevice::VulkanDevice( VkInstance instance, uint32 deviceIdx,
                                VulkanRenderSystem *renderSystem ) :
        mInstance( instance ),
        mPhysicalDevice( 0 ),
        mDevice( 0 ),
        mPresentQueue( 0 ),
        mRenderSystem( renderSystem ),
        mSupportedStages( 0xFFFFFFFF )
    {
        memset( &mDeviceMemoryProperties, 0, sizeof( mDeviceMemoryProperties ) );
        createPhysicalDevice( deviceIdx );
    }
    //-------------------------------------------------------------------------
    VulkanDevice::~VulkanDevice()
    {
        if( mDevice )
        {
            vkDeviceWaitIdle( mDevice );

            mGraphicsQueue.destroy();
            destroyQueues( mComputeQueues );
            destroyQueues( mTransferQueues );

            vmaDestroyAllocator(mVmaAllocator);

            vkDestroyDevice( mDevice, 0 );
            mDevice = 0;
            mPhysicalDevice = 0;
        }
    }
    //-------------------------------------------------------------------------
    void VulkanDevice::destroyQueues( FastArray<VulkanQueue> &queueArray )
    {
        for (auto& q : queueArray)
            q.destroy();
        queueArray.clear();
    }
    //-------------------------------------------------------------------------
    VkInstance VulkanDevice::createInstance( FastArray<const char *> &extensions,
                                             FastArray<const char *> &layers,
                                             PFN_vkDebugReportCallbackEXT debugCallback)
    {
        VkInstanceCreateInfo createInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
        VkApplicationInfo appInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO};

        appInfo.pEngineName = "Ogre3D Vulkan Engine";
        appInfo.engineVersion = OGRE_VERSION;
        appInfo.apiVersion = VK_API_VERSION_1_2;

        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledLayerCount = layers.size();
        createInfo.ppEnabledLayerNames = layers.data();

        createInfo.enabledExtensionCount = extensions.size();
        createInfo.ppEnabledExtensionNames = extensions.data();

#if 1 //OGRE_DEBUG_MODE
        VkDebugReportCallbackCreateInfoEXT debugCb = {VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT};
        debugCb.pfnCallback = debugCallback;
        debugCb.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
        createInfo.pNext = &debugCb;
#endif

        VkInstance instance;
        OGRE_VK_CHECK(vkCreateInstance(&createInfo, 0, &instance));
        return instance;
    }
    //-------------------------------------------------------------------------
    void VulkanDevice::createPhysicalDevice( uint32 deviceIdx )
    {
        uint32 numDevices = 0u;
        OGRE_VK_CHECK(vkEnumeratePhysicalDevices(mInstance, &numDevices, NULL));

        if( numDevices == 0u )
        {
            OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR, "No Vulkan devices found.",
                         "VulkanDevice::createPhysicalDevice" );
        }

        const String numDevicesStr = StringConverter::toString( numDevices );
        String deviceIdsStr = StringConverter::toString( deviceIdx );

        if( deviceIdx >= numDevices )
        {
            LogManager::getSingleton().logWarning( "[Vulkan] Requested device index " + deviceIdsStr +
                                                   " but there's only " +
                                                   StringConverter::toString( numDevices ) + "devices" );
            deviceIdx = 0u;
            deviceIdsStr = "0";
        }

        LogManager::getSingleton().logMessage( "[Vulkan] Selecting device #" + deviceIdsStr );

        FastArray<VkPhysicalDevice> pd(numDevices);
        OGRE_VK_CHECK(vkEnumeratePhysicalDevices(mInstance, &numDevices, pd.data()));
        mPhysicalDevice = pd[deviceIdx];

        vkGetPhysicalDeviceMemoryProperties( mPhysicalDevice, &mDeviceMemoryProperties );
        vkGetPhysicalDeviceFeatures( mPhysicalDevice, &mDeviceFeatures );

        mSupportedStages = 0xFFFFFFFF;
        if( !mDeviceFeatures.geometryShader )
            mSupportedStages ^= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
        if( !mDeviceFeatures.tessellationShader )
        {
            mSupportedStages ^= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
                                VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
        }
    }
    //-------------------------------------------------------------------------
    void VulkanDevice::findGraphicsQueue( FastArray<uint32> &inOutUsedQueueCount )
    {
        const size_t numQueues = mQueueProps.size();
        for( size_t i = 0u; i < numQueues; ++i )
        {
            if( mQueueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT &&
                inOutUsedQueueCount[i] < mQueueProps[i].queueCount )
            {
                mGraphicsQueue.setQueueData( this, VulkanQueue::Graphics, static_cast<uint32>( i ),
                                             inOutUsedQueueCount[i] );
                ++inOutUsedQueueCount[i];
                return;
            }
        }

        OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                     "GPU does not expose Graphics queue. Cannot be used for rendering",
                     "VulkanQueue::findGraphicsQueue" );
    }
    //-------------------------------------------------------------------------
    void VulkanDevice::findComputeQueue( FastArray<uint32> &inOutUsedQueueCount, uint32 maxNumQueues )
    {
        const size_t numQueues = mQueueProps.size();
        for( size_t i = 0u; i < numQueues && mComputeQueues.size() < maxNumQueues; ++i )
        {
            if( mQueueProps[i].queueFlags & VK_QUEUE_COMPUTE_BIT &&
                inOutUsedQueueCount[i] < mQueueProps[i].queueCount )
            {
                mComputeQueues.push_back( VulkanQueue() );
                mComputeQueues.back().setQueueData( this, VulkanQueue::Compute, static_cast<uint32>( i ),
                                                    inOutUsedQueueCount[i] );
                ++inOutUsedQueueCount[i];
            }
        }
    }
    //-------------------------------------------------------------------------
    void VulkanDevice::findTransferQueue( FastArray<uint32> &inOutUsedQueueCount, uint32 maxNumQueues )
    {
        const size_t numQueues = mQueueProps.size();
        for( size_t i = 0u; i < numQueues && mTransferQueues.size() < maxNumQueues; ++i )
        {
            if( mQueueProps[i].queueFlags & VK_QUEUE_TRANSFER_BIT &&
                !( mQueueProps[i].queueFlags & ( VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT ) ) &&
                inOutUsedQueueCount[i] < mQueueProps[i].queueCount )
            {
                mTransferQueues.push_back( VulkanQueue() );
                mTransferQueues.back().setQueueData( this, VulkanQueue::Transfer,
                                                     static_cast<uint32>( i ), inOutUsedQueueCount[i] );
                ++inOutUsedQueueCount[i];
            }
        }
    }
    //-------------------------------------------------------------------------
    void VulkanDevice::fillQueueCreationInfo( uint32 maxComputeQueues, uint32 maxTransferQueues,
                                              FastArray<VkDeviceQueueCreateInfo> &outQueueCiArray )
    {
        const size_t numQueueFamilies = mQueueProps.size();

        FastArray<uint32> usedQueueCount;
        usedQueueCount.resize( numQueueFamilies, 0u );

        findGraphicsQueue( usedQueueCount );
        findComputeQueue( usedQueueCount, maxComputeQueues );
        findTransferQueue( usedQueueCount, maxTransferQueues );

        VkDeviceQueueCreateInfo queueCi = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
        for( size_t i = 0u; i < numQueueFamilies; ++i )
        {
            queueCi.queueFamilyIndex = static_cast<uint32>( i );
            queueCi.queueCount = usedQueueCount[i];
            if( queueCi.queueCount > 0u )
                outQueueCiArray.push_back( queueCi );
        }
    }
    //-------------------------------------------------------------------------
    void VulkanDevice::createDevice( FastArray<const char *> &extensions, uint32 maxComputeQueues,
                                     uint32 maxTransferQueues )
    {
        uint32 numQueues;
        vkGetPhysicalDeviceQueueFamilyProperties( mPhysicalDevice, &numQueues, NULL );
        OgreAssert( numQueues > 0u, "No queues found" );
        mQueueProps.resize( numQueues );
        vkGetPhysicalDeviceQueueFamilyProperties( mPhysicalDevice, &numQueues, &mQueueProps[0] );

        // Setup queue creation
        FastArray<VkDeviceQueueCreateInfo> queueCreateInfo;
        fillQueueCreationInfo( maxComputeQueues, maxTransferQueues, queueCreateInfo );

        FastArray<FastArray<float> > queuePriorities;
        queuePriorities.resize( queueCreateInfo.size() );

        for( size_t i = 0u; i < queueCreateInfo.size(); ++i )
        {
            queuePriorities[i].resize( queueCreateInfo[i].queueCount, 1.0f );
            queueCreateInfo[i].pQueuePriorities = queuePriorities[i].data();
        }

        extensions.push_back( VK_KHR_SWAPCHAIN_EXTENSION_NAME );

        VkPhysicalDeviceFeatures2 features2 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
        features2.features = mDeviceFeatures;

#ifdef VK_EXT_mesh_shader
        VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT};
        if(mRenderSystem->getCapabilities()->hasCapability(RSC_MESH_PROGRAM))
        {
            meshShaderFeatures.taskShader = VK_TRUE;
            meshShaderFeatures.meshShader = VK_TRUE;
            features2.pNext = &meshShaderFeatures;
        }
#endif

        VkDeviceCreateInfo createInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
        createInfo.pNext = &features2;

        createInfo.enabledExtensionCount = static_cast<uint32>( extensions.size() );
        createInfo.ppEnabledExtensionNames = extensions.data();

        createInfo.queueCreateInfoCount = static_cast<uint32>( queueCreateInfo.size() );
        createInfo.pQueueCreateInfos = &queueCreateInfo[0];

        //features2.features = mDeviceFeatures;

        OGRE_VK_CHECK(vkCreateDevice(mPhysicalDevice, &createInfo, NULL, &mDevice));

        volkLoadDevice(mDevice);

        VmaVulkanFunctions fns;
        fns.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
        fns.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
        fns.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
        fns.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
        fns.vkAllocateMemory = vkAllocateMemory;
        fns.vkFreeMemory = vkFreeMemory;
        fns.vkMapMemory = vkMapMemory;
        fns.vkUnmapMemory = vkUnmapMemory;
        fns.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
        fns.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
        fns.vkBindBufferMemory = vkBindBufferMemory;
        fns.vkBindImageMemory = vkBindImageMemory;
        fns.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
        fns.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
        fns.vkCreateBuffer = vkCreateBuffer;
        fns.vkDestroyBuffer = vkDestroyBuffer;
        fns.vkCreateImage = vkCreateImage;
        fns.vkDestroyImage = vkDestroyImage;
        fns.vkCmdCopyBuffer = vkCmdCopyBuffer;

        VmaAllocatorCreateInfo allocatorCreateInfo = {};
        allocatorCreateInfo.instance = mInstance;
        allocatorCreateInfo.device = mDevice;
        allocatorCreateInfo.physicalDevice = mPhysicalDevice;
        allocatorCreateInfo.pVulkanFunctions = &fns;
        vmaCreateAllocator(&allocatorCreateInfo, &mVmaAllocator);
    }
    //-------------------------------------------------------------------------
    void VulkanDevice::initQueues( void )
    {
        VkQueue queue = 0;
        vkGetDeviceQueue( mDevice, mGraphicsQueue.mFamilyIdx, mGraphicsQueue.mQueueIdx, &queue );
        mGraphicsQueue.init( mDevice, queue, mRenderSystem );

        for (auto& q : mComputeQueues)
        {
            vkGetDeviceQueue( mDevice, q.mFamilyIdx, q.mQueueIdx, &queue);
            q.init(mDevice, queue, mRenderSystem);
        }

        for (auto& q : mTransferQueues)
        {
            vkGetDeviceQueue(mDevice, q.mFamilyIdx, q.mQueueIdx, &queue);
            q.init(mDevice, queue, mRenderSystem);
        }

        TODO_findRealPresentQueue;
        mPresentQueue = mGraphicsQueue.mQueue;
    }
    //-------------------------------------------------------------------------
    void VulkanDevice::commitAndNextCommandBuffer( SubmissionType::SubmissionType submissionType )
    {
        mGraphicsQueue.endAllEncoders();
        mGraphicsQueue.commitAndNextCommandBuffer( submissionType );
    }
    //-------------------------------------------------------------------------
    void VulkanDevice::stall( void )
    {
        // We must flush the cmd buffer and our bindings because we take the
        // moment to delete all delayed buffers and API handles after a stall.
        //
        // We can't have potentially dangling API handles in a cmd buffer.
        // We must submit our current pending work so far and wait until that's done.
        commitAndNextCommandBuffer( SubmissionType::FlushOnly );
        mRenderSystem->resetAllBindings();

        vkDeviceWaitIdle( mDevice );

        mRenderSystem->_notifyDeviceStalled();
    }
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    VulkanDevice::SelectedQueue::SelectedQueue() :
        usage( VulkanQueue::NumQueueFamilies ),
        familyIdx( std::numeric_limits<uint32>::max() ),
        queueIdx( 0 )
    {
    }
}  // namespace Ogre
