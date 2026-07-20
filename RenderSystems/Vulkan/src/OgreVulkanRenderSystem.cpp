/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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

#include "OgreVulkanRenderSystem.h"

#include <algorithm>
#include <numeric>

#include "OgreGpuProgramManager.h"
#include "OgreViewport.h"

#include "OgreVulkanRenderPassDescriptor.h"
#include "OgreVulkanDevice.h"
#include "OgreVulkanMappings.h"
#include "OgreVulkanProgram.h"
#include "OgreVulkanTextureGpuManager.h"
#include "OgreVulkanUtils.h"
#include "OgreVulkanWindow.h"
#include "OgreVulkanHardwareBuffer.h"
#include "OgreVulkanHardwareBufferManager.h"
#include "OgreVulkanTextureGpu.h"
#include "OgreVulkanDescriptorPool.h"

#include "OgreDepthBuffer.h"
#include "OgreRoot.h"

#include "OgrePixelFormat.h"

namespace Ogre
{
    static const uint32 VERTEX_ATTRIBUTE_INDEX[VES_COUNT] =
    {
        0,  // VES_POSITION - 1
        1,  // VES_BLEND_WEIGHTS - 1
        7,  // VES_BLEND_INDICES - 1
        2,  // VES_NORMAL - 1
        3,  // VES_DIFFUSE - 1
        4,  // VES_SPECULAR - 1
        8,  // VES_TEXTURE_COORDINATES - 1
        //There are up to 8 VES_TEXTURE_COORDINATES. Occupy range [8; 16)
        //Range [14; 16) overlaps with VES_TANGENT & VES_BINORMAL
        //(slot 16 is where const buffers start)
        15,// VES_BINORMAL - 1
        14,  // VES_TANGENT - 1
    };

static VKAPI_ATTR VkBool32 VKAPI_CALL dbgUtilsCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData )
{
    const char *severity = "INFO";
    if( messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT )
        severity = "ERROR";
    else if( messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT )
        severity = "WARNING";
    else if( messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT )
        severity = "VERBOSE";

    const char *type = "";
    if( messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT )
        type = "VALIDATION";
    else if( messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT )
        type = "PERFORMANCE";
    else if( messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT )
        type = "GENERAL";

    LogManager::getSingleton().logMessage(
        StringUtil::format( "[Vulkan][%s][%s] %s (ID: %s #%d)",
                            severity, type,
                            pCallbackData->pMessage,
                            pCallbackData->pMessageIdName ? pCallbackData->pMessageIdName : "",
                            pCallbackData->messageIdNumber ) );

    return VK_FALSE;
}

    //-------------------------------------------------------------------------
    VulkanRenderSystem::VulkanRenderSystem() :
        RenderSystem(),
        mInitialized( false ),
        mHardwareBufferManager( 0 ),
        mIndirectBuffer( 0 ),
        mSPIRVProgramFactory( 0 ),
        mVkInstance( 0 ),
        mActiveDevice( 0 ),
        mDevice( 0 ),
        mCurrentRenderPassDescriptor( 0 ),
        mHasValidationLayers( false ),
        CreateDebugUtilsMessenger( 0 ),
        DestroyDebugUtilsMessenger( 0 ),
        mDebugMessenger( 0 ),
        pipelineCi{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO},
        pipelineLayoutCi{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO},
        vertexFormatCi{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO},
        inputAssemblyCi{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO},
        mssCi{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO},
        rasterState{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO},
        blendStateCi{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO},
        mUBOInfo{},
        mUBODynOffsets{},
        mImageInfos{},
        mComputeImageInfo{},
        mStorageImageInfos{},
        mStorageTextures{},
        mStorageImageViews{},
        mBoundComputeProfile( Compute ),
        depthStencilStateCi{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO},
        mVkViewport{},
        mScissorRect{},
        viewportStateCi{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO}
    {
        mAutoParamsBufferPos = 0;

        pipelineCi.pVertexInputState = &vertexFormatCi;
        pipelineCi.pInputAssemblyState = &inputAssemblyCi;
        pipelineCi.pRasterizationState = &rasterState;
        pipelineCi.pMultisampleState = &mssCi;
        pipelineCi.pDepthStencilState = &depthStencilStateCi;
        pipelineCi.pColorBlendState = &blendStateCi;
        pipelineCi.pViewportState = &viewportStateCi;
        pipelineCi.pStages = shaderStages.data();
        pipelineCi.stageCount = 2; // vertex+fragment

        for(auto& p : mProfiles)
        {
            p.layout = VK_NULL_HANDLE;
            p.pipelineLayout = VK_NULL_HANDLE;
        }

        inputAssemblyCi.primitiveRestartEnable = false;

        mssCi.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        rasterState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterState.lineWidth = 1.0f;

        blendStateCi.attachmentCount = 1;
        blendStateCi.pAttachments = blendStates.data();

        depthStencilStateCi.minDepthBounds = 0.0f;
        depthStencilStateCi.maxDepthBounds = 1.0f;
        mVkViewport.minDepth = 0.0f;
        mVkViewport.maxDepth = 1.0f;

        viewportStateCi.pViewports = &mVkViewport;
        viewportStateCi.viewportCount = 1u;
        viewportStateCi.scissorCount = 1u;

        // use a single descriptor set for all shaders
        auto& graphics = mProfiles[Graphics];
        graphics.bindings.push_back({0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL_GRAPHICS});
        graphics.bindings.push_back({1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL_GRAPHICS});
        for(uint32 i = 0; i < OGRE_MAX_TEXTURE_COORD_SETS; ++i)
            graphics.bindings.push_back({2 + i, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL_GRAPHICS});

        // one descriptor will have at most OGRE_MAX_TEXTURE_LAYERS and one UBO per shader type (for now)
        graphics.poolSizes.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, GPT_COUNT});
        graphics.poolSizes.push_back({VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, OGRE_MAX_TEXTURE_COORD_SETS});

        // silence validation layer, when unused
        mUBOInfo[0].range = 1;
        mUBOInfo[1].range = 1;

        graphics.writes.resize(OGRE_MAX_TEXTURE_COORD_SETS + 2, {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET});
        graphics.writes[0].dstBinding = 0;
        graphics.writes[0].descriptorCount = 1;
        graphics.writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        graphics.writes[0].pBufferInfo = mUBOInfo.data();

        graphics.writes[1].dstBinding = 1;
        graphics.writes[1].descriptorCount = 1;
        graphics.writes[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        graphics.writes[1].pBufferInfo = mUBOInfo.data() + 1;

        for(int i = 0; i < OGRE_MAX_TEXTURE_COORD_SETS; i++)
        {
            graphics.writes[i + 2].dstBinding = 2 + i;
            graphics.writes[i + 2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            graphics.writes[i + 2].pImageInfo = mImageInfos.data() + i;
            graphics.writes[i + 2].descriptorCount = 1;
        }

        // Minimal compute profile for image write + dynamic UBO.
        auto &compute = mProfiles[Compute];
        compute.bindings.push_back( {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT} );
        compute.bindings.push_back(
            {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_COMPUTE_BIT} );
        compute.poolSizes.push_back( {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1u} );
        compute.poolSizes.push_back( {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1u} );
        compute.writes.resize( 2u, {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET} );
        compute.writes[0].dstBinding = 0;
        compute.writes[0].descriptorCount = 1;
        compute.writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        compute.writes[0].pImageInfo = &mComputeImageInfo;
        compute.writes[1].dstBinding = 1;
        compute.writes[1].descriptorCount = 1;
        compute.writes[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        compute.writes[1].pBufferInfo = mUBOInfo.data() + 1;

        auto &allUnits = mProfiles[AllUnits];
        allUnits.bindings.push_back({0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL_GRAPHICS});
        allUnits.bindings.push_back({1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL_GRAPHICS});
        for(uint32 i = 0; i < OGRE_MAX_TEXTURE_LAYERS; ++i)
            allUnits.bindings.push_back({2 + i, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL_GRAPHICS});

        allUnits.poolSizes.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, GPT_COUNT});
        allUnits.poolSizes.push_back({VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, OGRE_MAX_TEXTURE_LAYERS});

        allUnits.writes.resize(OGRE_MAX_TEXTURE_LAYERS + 2, {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET});
        // same dstBinding/descriptorType/pBufferInfo/pImageInfo wiring as legacy's loop, just OGRE_MAX_TEXTURE_LAYERS iterations

        if(volkInitialize() != VK_SUCCESS)
        {
            LogManager::getSingleton().logWarning("Vulkan unavailable - loader not found");
            return;
        }

        try {
            initializeVkInstance();
        } catch(const std::exception& e) {
            LogManager::getSingleton().logWarning(e.what());
            return;
        }
        enumerateDevices();
        initConfigOptions();
    }
    void VulkanRenderSystem::enumerateDevices()
    {
        mDevices.clear();

        VkInstance instance = getVkInstance();

        uint32 numDevices = 0u;
        OGRE_VK_CHECK(vkEnumeratePhysicalDevices(instance, &numDevices, NULL));

        if( numDevices == 0u )
        {
            LogManager::getSingleton().logError( "[Vulkan] No Vulkan devices found." );
            return;
        }

        FastArray<VkPhysicalDevice> pd(numDevices);
        OGRE_VK_CHECK(vkEnumeratePhysicalDevices(instance, &numDevices, pd.data()));

        LogManager::getSingleton().logMessage( "[Vulkan] Found devices:" );

        mDevices.reserve( numDevices );
        for( uint32 i = 0u; i < numDevices; ++i )
        {
            VkPhysicalDeviceProperties deviceProps;
            vkGetPhysicalDeviceProperties( pd[i], &deviceProps );

            mDevices.push_back( deviceProps.deviceName );

            LogManager::getSingleton().stream() << " #" << i << " " << deviceProps.deviceName;
        }
    }
    uint32 VulkanRenderSystem::getSelectedDeviceIdx() const
    {
        uint32 deviceIdx = 0u;
        auto itDevice = std::find(mDevices.begin(), mDevices.end(), mOptions.at("Device").currentValue);
        if( itDevice != mDevices.end() )
            deviceIdx = itDevice - mDevices.begin();

        return deviceIdx;
    }
    //-------------------------------------------------------------------------
    VulkanRenderSystem::~VulkanRenderSystem()
    {
        shutdown();

        if( mDebugMessenger )
        {
            DestroyDebugUtilsMessenger( mVkInstance, mDebugMessenger, nullptr );
            mDebugMessenger = 0;
        }

        if( mVkInstance )
        {
            vkDestroyInstance( mVkInstance, 0 );
            mVkInstance = 0;
        }
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::shutdown( void )
    {
        if( !mDevice )
            return;

        mDevice->stall();

        // destroys the depth buffers and render targets (incl. windows) like
        // the other render systems do - their Vulkan resources (e.g. the
        // VMA-backed window depth texture) must be released while the device
        // and its allocator are still alive
        RenderSystem::shutdown();

        mAutoParamsBuffer.reset();

        OGRE_DELETE mHardwareBufferManager;
        mHardwareBufferManager = 0;

        OGRE_DELETE mTextureManager;
        mTextureManager = 0;

        OGRE_DELETE mSPIRVProgramFactory;
        mSPIRVProgramFactory = 0;

        // Flush deferred image view deletions
        for( uint32 i = 0; i < FRAMES_IN_FLIGHT; ++i )
        {
            for( VkImageView view : mDeferredViewDeletions[i] )
                vkDestroyImageView( mDevice->mDevice, view, nullptr );
            mDeferredViewDeletions[i].clear();
        }

        clearStorageTextureBindings();

        for(auto& p : mProfiles)
        {
            if(p.pipelineLayout)
                vkDestroyPipelineLayout(mDevice->mDevice, p.pipelineLayout, 0);
            if(p.layout)
                vkDestroyDescriptorSetLayout(mDevice->mDevice, p.layout, 0);
            p.pool.reset();
        }

        for(auto it : mRenderPassCache)
        {
            vkDestroyRenderPass(mDevice->mDevice, it.second, 0);
        }

        clearPipelineCache();

        delete mDevice;
        mDevice = 0;
    }
    void VulkanRenderSystem::clearPipelineCache()
    {
        for(auto it : mPipelineCache)
        {
            vkDestroyPipeline(mDevice->mDevice, it.second, 0);
        }

        mPipelineCache.clear();

        for( auto it : mComputePipelineCache )
        {
            vkDestroyPipeline( mDevice->mDevice, it.second, 0 );
        }

        mComputePipelineCache.clear();
    }
    //-------------------------------------------------------------------------
    const String &VulkanRenderSystem::getName( void ) const
    {
        static String strName( "Vulkan Rendering Subsystem" );
        return strName;
    }
    void VulkanRenderSystem::initConfigOptions( void )
    {
        RenderSystem::initConfigOptions();

        ConfigOption optFSAA;
        optFSAA.name = "FSAA";
        optFSAA.immutable = false;
        optFSAA.possibleValues.push_back( "1" );
        optFSAA.possibleValues.push_back( "2" );
        optFSAA.possibleValues.push_back( "4" );
        optFSAA.possibleValues.push_back( "8" );
        optFSAA.possibleValues.push_back( "16" );
        optFSAA.currentValue = optFSAA.possibleValues.front();

        ConfigOption optDevices;
        optDevices.name = "Device";

        for(const auto& d : mDevices)
            optDevices.possibleValues.push_back( d);

        optDevices.currentValue = mDevices.front();
        optDevices.immutable = false;

        mOptions[optDevices.name] = optDevices;
        mOptions[optFSAA.name] = optFSAA;

        ConfigOption opt;
        opt.name = "Reversed Z-Buffer";
        opt.possibleValues = {"No", "Yes"};
        opt.currentValue = opt.possibleValues[0];
        opt.immutable = false;

        mOptions[opt.name] = opt;
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::setConfigOption( const String &name, const String &value )
    {
        ConfigOptionMap::iterator it = mOptions.find( name );

        if( it == mOptions.end() )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "Option named " + name + " does not exist.",
                         "VulkanSupport::setConfigOption" );
        }

        it->second.currentValue = value;

        if(name == "Reversed Z-Buffer")
            mIsReverseDepthBufferEnabled = StringConverter::parseBool(value);
    }
    //-------------------------------------------------------------------------
void VulkanRenderSystem::addInstanceDebugCallback( void )
{
    CreateDebugUtilsMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        mVkInstance, "vkCreateDebugUtilsMessengerEXT" );
    DestroyDebugUtilsMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        mVkInstance, "vkDestroyDebugUtilsMessengerEXT" );

    if( !CreateDebugUtilsMessenger || !DestroyDebugUtilsMessenger )
    {
        LogManager::getSingleton().logMessage(
            "[Vulkan] Unable to find vkCreateDebugUtilsMessengerEXT. "
            "Debug reporting won't be available" );
        return;
    }

    VkDebugUtilsMessengerCreateInfoEXT messengerCi = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    messengerCi.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    messengerCi.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    messengerCi.pfnUserCallback = dbgUtilsCallback;
    messengerCi.pUserData = nullptr;

    OGRE_VK_CHECK( CreateDebugUtilsMessenger( mVkInstance, &messengerCi, nullptr, &mDebugMessenger ) );
}
    //-------------------------------------------------------------------------
    HardwareOcclusionQuery *VulkanRenderSystem::createHardwareOcclusionQuery( void )
    {
        return 0;  // TODO
    }
    //-------------------------------------------------------------------------
    RenderSystemCapabilities *VulkanRenderSystem::createRenderSystemCapabilities( void ) const
    {
        RenderSystemCapabilities *rsc = new RenderSystemCapabilities();
        rsc->setRenderSystemName( getName() );

        // We would like to save the device properties for the device capabilities limits.
        // These limits are needed for buffers' binding alignments.
        VkPhysicalDeviceProperties *vkProperties =
            const_cast<VkPhysicalDeviceProperties *>( &mActiveDevice->mDeviceProperties );
        vkGetPhysicalDeviceProperties( mActiveDevice->mPhysicalDevice, vkProperties );

        VkPhysicalDeviceProperties &properties = mActiveDevice->mDeviceProperties;

        LogManager::getSingleton().logMessage(
            StringUtil::format("[Vulkan] API Version: %d.%d.%d", VK_VERSION_MAJOR(properties.apiVersion),
                               VK_VERSION_MINOR(properties.apiVersion), VK_VERSION_PATCH(properties.apiVersion)));
        LogManager::getSingleton().logMessage(StringUtil::format("[Vulkan] Vendor ID: %#x", properties.vendorID));
        LogManager::getSingleton().logMessage(StringUtil::format("[Vulkan] Device ID: %#x", properties.deviceID));

        rsc->setDeviceName( properties.deviceName );

        switch( properties.vendorID )
        {
        case 0x10DE:
        {
            rsc->setVendor( GPU_NVIDIA );
            // 10 bits = major version (up to r1023)
            // 8 bits = minor version (up to 255)
            // 8 bits = secondary branch version/build version (up to 255)
            // 6 bits = tertiary branch/build version (up to 63)

            DriverVersion driverVersion;
            driverVersion.major = ( properties.driverVersion >> 22u ) & 0x3ff;
            driverVersion.minor = ( properties.driverVersion >> 14u ) & 0x0ff;
            driverVersion.release = ( properties.driverVersion >> 6u ) & 0x0ff;
            driverVersion.build = ( properties.driverVersion ) & 0x003f;
            rsc->setDriverVersion( driverVersion );
        }
        break;
        case 0x1002:
            rsc->setVendor( GPU_AMD );
            break;
        case 0x8086:
            rsc->setVendor( GPU_INTEL );
            break;
        case 0x13B5:
            rsc->setVendor( GPU_ARM );  // Mali
            break;
        case 0x5143:
            rsc->setVendor( GPU_QUALCOMM );
            break;
        case 0x1010:
            rsc->setVendor( GPU_IMAGINATION_TECHNOLOGIES );  // PowerVR
            break;
        }

        if( rsc->getVendor() != GPU_NVIDIA )
        {
            // Generic version routine that matches SaschaWillems's VulkanCapsViewer
            DriverVersion driverVersion;
            driverVersion.major = ( properties.driverVersion >> 22u ) & 0x3ff;
            driverVersion.minor = ( properties.driverVersion >> 12u ) & 0x3ff;
            driverVersion.release = ( properties.driverVersion ) & 0xfff;
            driverVersion.build = 0;
            rsc->setDriverVersion( driverVersion );
        }

        //if( mActiveDevice->mDeviceFeatures.imageCubeArray )
        //    rsc->setCapability( RSC_TEXTURE_CUBE_MAP_ARRAY );

        if( mActiveDevice->mDeviceFeatures.depthClamp )
            rsc->setCapability( RSC_DEPTH_CLAMP );

        {
            VkFormatProperties props;

            vkGetPhysicalDeviceFormatProperties( mDevice->mPhysicalDevice,
                                                 VulkanMappings::get( PF_DXT1 ), &props );
            if( props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT )
                rsc->setCapability( RSC_TEXTURE_COMPRESSION_DXT );

            vkGetPhysicalDeviceFormatProperties( mDevice->mPhysicalDevice,
                                                 VulkanMappings::get( PF_BC4_UNORM ), &props );
            if( props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT )
                rsc->setCapability( RSC_TEXTURE_COMPRESSION_BC4_BC5 );

            vkGetPhysicalDeviceFormatProperties( mDevice->mPhysicalDevice,
                                                 VulkanMappings::get( PF_BC6H_UF16 ), &props );
            if( props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT )
                rsc->setCapability( RSC_TEXTURE_COMPRESSION_BC6H_BC7 );

            // Vulkan doesn't allow supporting ETC1 without ETC2
            vkGetPhysicalDeviceFormatProperties( mDevice->mPhysicalDevice,
                                                 VulkanMappings::get( PF_ETC2_RGB8 ), &props );
            if( props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT )
            {
                rsc->setCapability( RSC_TEXTURE_COMPRESSION_ETC1 );
                rsc->setCapability( RSC_TEXTURE_COMPRESSION_ETC2 );
            }

            vkGetPhysicalDeviceFormatProperties(
                mDevice->mPhysicalDevice, VulkanMappings::get( PF_ASTC_RGBA_4X4_LDR ), &props );
            if( props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT )
                rsc->setCapability( RSC_TEXTURE_COMPRESSION_ASTC );
        }
        rsc->setCapability( RSC_COMPUTE_PROGRAM );

        const VkPhysicalDeviceLimits &deviceLimits = mDevice->mDeviceProperties.limits;
        //rsc->setMaximumResolutions( deviceLimits.maxImageDimension2D, deviceLimits.maxImageDimension3D,
        //                            deviceLimits.maxImageDimensionCube );
        //rsc->setMaxThreadsPerThreadgroupAxis( deviceLimits.maxComputeWorkGroupSize );
        //rsc->setMaxThreadsPerThreadgroup( deviceLimits.maxComputeWorkGroupInvocations );

        if( mActiveDevice->mDeviceFeatures.samplerAnisotropy && deviceLimits.maxSamplerAnisotropy > 1u )
        {
            rsc->setCapability( RSC_ANISOTROPY );
            rsc->setMaxSupportedAnisotropy( deviceLimits.maxSamplerAnisotropy );
        }

        //rsc->setCapability( RSC_STORE_AND_MULTISAMPLE_RESOLVE );
        //rsc->setCapability( RSC_TEXTURE_GATHER );

        rsc->setCapability( RSC_VERTEX_PROGRAM );
        //rsc->setCapability( RSC_UAV );
        //rsc->setCapability( RSC_TYPED_UAV_LOADS );
        //rsc->setCapability( RSC_EXPLICIT_FSAA_RESOLVE );
        rsc->setCapability( RSC_TEXTURE_1D );

        //rsc->setCapability( RSC_HWSTENCIL );
        rsc->setNumTextureUnits( OGRE_MAX_TEXTURE_COORD_SETS );
        rsc->setCapability( RSC_TEXTURE_COMPRESSION );
        rsc->setCapability( RSC_32BIT_INDEX );
        rsc->setCapability( RSC_TWO_SIDED_STENCIL );
        rsc->setCapability( RSC_STENCIL_WRAP );
        if( mActiveDevice->mDeviceFeatures.shaderClipDistance )
            rsc->setCapability( RSC_USER_CLIP_PLANES );
        rsc->setCapability( RSC_TEXTURE_3D );
        rsc->setCapability( RSC_NON_POWER_OF_2_TEXTURES );
        rsc->setCapability(RSC_VERTEX_TEXTURE_FETCH);
        rsc->setNonPOW2TexturesLimited( false );
        rsc->setCapability( RSC_HWRENDER_TO_TEXTURE );
        rsc->setCapability( RSC_TEXTURE_FLOAT );
        if( mActiveDevice->mDeviceFeatures.largePoints )
        {
            rsc->setCapability( RSC_POINT_SPRITES );
        }
        rsc->setCapability( RSC_TEXTURE_2D_ARRAY );
        rsc->setCapability( RSC_ALPHA_TO_COVERAGE );
        rsc->setCapability( RSC_HW_GAMMA );
        rsc->setCapability( RSC_VERTEX_BUFFER_INSTANCE_DATA );
        rsc->setCapability(RSC_VERTEX_FORMAT_INT_10_10_10_2);
        rsc->setCapability(RSC_VERTEX_FORMAT_16X3);
        rsc->setMaxPointSize( deviceLimits.pointSizeRange[1] );

        //rsc->setMaximumResolutions( 16384, 4096, 16384 );
        auto maxFloatVectors = deviceLimits.maxUniformBufferRange / (4 * sizeof(float));
        rsc->setVertexProgramConstantFloatCount(maxFloatVectors);
        rsc->setGeometryProgramConstantFloatCount(maxFloatVectors);
        rsc->setFragmentProgramConstantFloatCount(maxFloatVectors);
        rsc->setTessellationHullProgramConstantFloatCount(maxFloatVectors);
        rsc->setTessellationDomainProgramConstantFloatCount(maxFloatVectors);
        rsc->setComputeProgramConstantFloatCount(maxFloatVectors);

        rsc->addShaderProfile( "spirv" );

        if( rsc->getVendor() == GPU_QUALCOMM )
        {
#ifdef OGRE_VK_WORKAROUND_ADRENO_D32_FLOAT
            Workarounds::mAdrenoD32FloatBug = false;
            if( !rsc->getDriverVersion().hasMinVersion( 512, 415 ) )
                Workarounds::mAdrenoD32FloatBug = true;
#endif
#ifdef OGRE_VK_WORKAROUND_ADRENO_5XX_6XX_MINCAPS
            Workarounds::mAdreno5xx6xxMinCaps = false;

            const uint32 c_adreno5xx6xxDeviceIds[] = {
                0x5000400,  // 504
                0x5000500,  // 505
                0x5000600,  // 506
                0x5000800,  // 508
                0x5000900,  // 509
                0x5010000,  // 510
                0x5010200,  // 512
                0x5030002,  // 530
                0x5040001,  // 540

                0x6010000,  // 610
                0x6010200,  // 612
                0x6010501,  // 615
                0x6010600,  // 616
                0x6010800,  // 618
                0x6020001,  // 620
                0x6030001,  // 630
                0x6040001,  // 640
                0x6050002,  // 650
            };

            const size_t numDevIds =
                sizeof( c_adreno5xx6xxDeviceIds ) / sizeof( c_adreno5xx6xxDeviceIds[0] );
            for( size_t i = 0u; i < numDevIds; ++i )
            {
                if( properties.deviceID == c_adreno5xx6xxDeviceIds[i] )
                {
                    Workarounds::mAdreno5xx6xxMinCaps = true;
                    break;
                }
            }
#endif
        }

        return rsc;
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::resetAllBindings( void )
    {
        for( auto &imgInfo : mImageInfos )
            imgInfo = {};

        for( auto &imgInfo : mStorageImageInfos )
            imgInfo = {};

        mStorageTextures.fill( nullptr );
        mStorageImageViews.fill( VK_NULL_HANDLE );
        mComputeImageInfo = {};
        mBoundComputeProfile = Compute;
        mUBODynOffsets = {0u, 0u};
    }

    void VulkanRenderSystem::setStorageTexture( size_t texUnit, VulkanTextureGpu *texture, int mipmapLevel )
    {
        if( texUnit >= OGRE_MAX_TEXTURE_LAYERS )
            return;

        if( texture == mStorageTextures[texUnit] && mStorageImageViews[texUnit] != VK_NULL_HANDLE )
            return;

        // NEVER destroy here — just orphan the old view for later cleanup
        if( mStorageImageViews[texUnit] != VK_NULL_HANDLE )
            mDeferredViewDeletions[mDevice->mGraphicsQueue.mCurrentFrameIdx].push_back( mStorageImageViews[texUnit] );

        mStorageImageViews[texUnit] = VK_NULL_HANDLE;
        mStorageTextures[texUnit] = texture;
        mStorageImageInfos[texUnit] = {};

        if( !texture )
            return;

        const int maxMipLevel = static_cast<int>( texture->getNumMipmaps() );
        const int clampedMip = std::max( 0, std::min( mipmapLevel, maxMipLevel ) );

        VkImageViewUsageCreateInfo viewUsageCi = {VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO};
        viewUsageCi.usage = VK_IMAGE_USAGE_STORAGE_BIT;

        VkImageViewCreateInfo viewCi = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        viewCi.pNext = &viewUsageCi;
        viewCi.image = texture->getFinalTextureName();
        viewCi.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewCi.format = VulkanMappings::get( texture->getFormat() );
        viewCi.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewCi.subresourceRange.baseMipLevel = static_cast<uint32>( clampedMip );
        viewCi.subresourceRange.levelCount = 1;
        viewCi.subresourceRange.baseArrayLayer = 0;
        viewCi.subresourceRange.layerCount = 1;

        VkImageView imageView;
        OGRE_VK_CHECK( vkCreateImageView( mDevice->mDevice, &viewCi, nullptr, &imageView ) );

        mStorageImageViews[texUnit] = imageView;

        mStorageImageInfos[texUnit].sampler = VK_NULL_HANDLE;
        mStorageImageInfos[texUnit].imageView = imageView;
        mStorageImageInfos[texUnit].imageLayout = VK_IMAGE_LAYOUT_GENERAL;

        // After creating the new view, clear stale cache entries
        mProfiles[Compute].cache.clear();
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::clearStorageTextureBindings()
    {
        for( size_t i = 0u; i < mStorageImageViews.size(); ++i )
        {
            if( mStorageImageViews[i] != VK_NULL_HANDLE )
                mDeferredViewDeletions[mDevice->mGraphicsQueue.mCurrentFrameIdx].push_back( mStorageImageViews[i] );

            mStorageImageViews[i] = VK_NULL_HANDLE;
            mStorageTextures[i] = nullptr;
            mStorageImageInfos[i] = {};
        }
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::initializeVkInstance( void )
    {
        if( mVkInstance )
            return;

        LogManager::getSingleton().logMessage( "[Vulkan] Initializing VkInstance" );

        uint32 numExtensions = 0u;
        OGRE_VK_CHECK(vkEnumerateInstanceExtensionProperties(0, &numExtensions, 0));

        std::vector<VkExtensionProperties> availableExtensions(numExtensions);
        OGRE_VK_CHECK(vkEnumerateInstanceExtensionProperties(0, &numExtensions, availableExtensions.data()));

        bool debugEnabled = USE_VALIDATION_LAYERS;
        //StringConverter::parse(mOptions.at("Debug Layer").currentValue, debugEnabled);

        // Check supported extensions we may want
        std::vector<const char *> reqInstanceExtensions;
        for( size_t i = 0u; i < numExtensions; ++i )
        {
            const String extensionName = availableExtensions[i].extensionName;
            LogManager::getSingleton().logMessage( "Found instance extension: " + extensionName );

            if (extensionName == VulkanWindow::getRequiredExtensionName())
            {
                reqInstanceExtensions.push_back(VulkanWindow::getRequiredExtensionName());
            }

            if (debugEnabled && extensionName == "VK_EXT_debug_utils")
                reqInstanceExtensions.push_back("VK_EXT_debug_utils");

#ifdef VK_KHR_portability_enumeration
            // portability (non fully conformant) implementations like MoltenVK are
            // only enumerated when VK_KHR_portability_enumeration is enabled
            if (extensionName == VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)
                reqInstanceExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

        }

        reqInstanceExtensions.push_back("VK_KHR_surface"); // required for window surface

        // Check supported layers we may want
        uint32 numInstanceLayers = 0u;
        OGRE_VK_CHECK(vkEnumerateInstanceLayerProperties(&numInstanceLayers, 0));

        FastArray<VkLayerProperties> instanceLayerProps(numInstanceLayers);
        OGRE_VK_CHECK(vkEnumerateInstanceLayerProperties(&numInstanceLayers, instanceLayerProps.data()));

        FastArray<const char *> instanceLayers;
        for( size_t i = 0u; i < numInstanceLayers; ++i )
        {
            const String layerName = instanceLayerProps[i].layerName;
            LogManager::getSingleton().logMessage( "Found instance layer: " + layerName );
            if( debugEnabled && layerName == "VK_LAYER_KHRONOS_validation" )
            {
                mHasValidationLayers = true;
                instanceLayers.push_back( "VK_LAYER_KHRONOS_validation" );
            }
        }

        if (debugEnabled && !mHasValidationLayers)
        {
            LogManager::getSingleton().logWarning(
                "Debug Layer requested, but VK_LAYER_KHRONOS_validation layer not present");
        }

        VkDebugUtilsMessengerCreateInfoEXT debugCi = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
        void *pNext = nullptr;

        if( mHasValidationLayers )
        {
            debugCi.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                                      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
            debugCi.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            debugCi.pfnUserCallback = dbgUtilsCallback;
            debugCi.pUserData = nullptr;
            pNext = &debugCi;
        }

        mVkInstance = VulkanDevice::createInstance(reqInstanceExtensions, instanceLayers, pNext);
        volkLoadInstanceOnly(mVkInstance);

        if(mHasValidationLayers)
        {
            LogManager::getSingleton().logMessage("[Vulkan] Setting up debug utils messenger...");
            addInstanceDebugCallback();
        }
        else
        {
            LogManager::getSingleton().logMessage("[Vulkan] No validation layers active");
        }
    }
    //-------------------------------------------------------------------------
    RenderWindow *VulkanRenderSystem::_createRenderWindow( const String &name, uint32 width, uint32 height,
                                                     bool fullScreen,
                                                     const NameValuePairList *miscParams )
    {
        RenderSystem::_createRenderWindow(name, width, height, fullScreen, miscParams);

        VulkanWindow *win = OGRE_NEW VulkanWindow( name, width, height, fullScreen );
        attachRenderTarget((Ogre::RenderTarget&) *win);

        if( !mInitialized )
        {
            initializeVkInstance();

            mDevice = new VulkanDevice( mVkInstance, getSelectedDeviceIdx(), this );
            mActiveDevice = mDevice;

            mRealCapabilities = createRenderSystemCapabilities();
            mCurrentCapabilities = mRealCapabilities;

            mSPIRVProgramFactory = OGRE_NEW VulkanProgramFactory( mActiveDevice );
            GpuProgramManager::getSingleton().addFactory( mSPIRVProgramFactory );

            mNativeShadingLanguageVersion = 100;

            bool bCanRestrictImageViewUsage = false;

            FastArray<const char *> deviceExtensions;
            {
                uint32 numExtensions = 0;
                vkEnumerateDeviceExtensionProperties( mDevice->mPhysicalDevice, 0, &numExtensions, 0 );

                std::vector<VkExtensionProperties> availableExtensions(numExtensions);
                vkEnumerateDeviceExtensionProperties( mDevice->mPhysicalDevice, 0, &numExtensions,
                                                      availableExtensions.data() );
                for( size_t i = 0u; i < numExtensions; ++i )
                {
                    const String extensionName = availableExtensions[i].extensionName;
                    LogManager::getSingleton().logMessage( "Found device extension: " + extensionName );

                    if( extensionName == VK_KHR_MAINTENANCE2_EXTENSION_NAME )
                    {
                        deviceExtensions.push_back( VK_KHR_MAINTENANCE2_EXTENSION_NAME );
                        bCanRestrictImageViewUsage = true;
                    }
                    else if( extensionName == VK_EXT_SHADER_SUBGROUP_VOTE_EXTENSION_NAME )
                        deviceExtensions.push_back( VK_EXT_SHADER_SUBGROUP_VOTE_EXTENSION_NAME );
                    else if( extensionName == "VK_KHR_portability_subset" )
                        // required by the spec whenever the implementation
                        // advertises it (e.g. MoltenVK)
                        deviceExtensions.push_back( "VK_KHR_portability_subset" );
                    else if( extensionName == VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME )
                    {
                        deviceExtensions.push_back( VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME );
                        mRealCapabilities->setCapability( RSC_VP_RT_INDEX_ANY_SHADER );
                    }
#ifdef VK_EXT_mesh_shader
                    else if( extensionName == VK_EXT_MESH_SHADER_EXTENSION_NAME)
                    {
                        deviceExtensions.push_back( VK_EXT_MESH_SHADER_EXTENSION_NAME );
                        deviceExtensions.push_back( VK_KHR_SPIRV_1_4_EXTENSION_NAME );
                        mRealCapabilities->setCapability(RSC_MESH_PROGRAM);

                        for( auto &binding : mProfiles[Graphics].bindings )
                            binding.stageFlags |= VK_SHADER_STAGE_MESH_BIT_NV;
                    }
#endif
                }
            }

            if( !bCanRestrictImageViewUsage )
            {
                LogManager::getSingleton().logMessage(
                    "WARNING: " VK_KHR_MAINTENANCE2_EXTENSION_NAME
                    " not present. We may have to force the driver to do UAV + SRGB operations "
                    "the GPU should support, but it's not guaranteed to work" );
            }

            if( mHasValidationLayers )
                deviceExtensions.push_back( VK_EXT_DEBUG_MARKER_EXTENSION_NAME );

            mDevice->createDevice( deviceExtensions, 0u, 0u );

            mHardwareBufferManager = OGRE_NEW VulkanHardwareBufferManager( mDevice );

            mActiveDevice->initQueues();
            //vaoManager->initDrawIdVertexBuffer();

            mTextureManager = new VulkanTextureGpuManager(this, mDevice, bCanRestrictImageViewUsage);
            mTextureManager->_getWarningTexture(); // preload warning texture, so does not interrupt render pass

            for( auto &profile : mProfiles )
            {
                if( profile.bindings.empty() )
                    continue;

                VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCi =
                    {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
                descriptorSetLayoutCi.bindingCount = profile.bindings.size();
                descriptorSetLayoutCi.pBindings = profile.bindings.data();
                OGRE_VK_CHECK( vkCreateDescriptorSetLayout( mActiveDevice->mDevice, &descriptorSetLayoutCi,
                                                            nullptr, &profile.layout ) );

                pipelineLayoutCi.pSetLayouts = &profile.layout;
                pipelineLayoutCi.setLayoutCount = 1;
                OGRE_VK_CHECK(
                    vkCreatePipelineLayout( mActiveDevice->mDevice, &pipelineLayoutCi, 0,
                                            &profile.pipelineLayout ) );
            }

            // allocate 1.5MB buffer. Holds e.g. 1024 batches of 512 bytes for 3 frames-in-flight
            resizeAutoParamsBuffer(1024 * 512 * 3);
            mAutoParamsBufferUsage.resize(mActiveDevice->mGraphicsQueue.mNumFramesInFlight);

            resetAllBindings();

            String workaroundsStr;
            //Workarounds::dump( (void *)&workaroundsStr );
            if( !workaroundsStr.empty() )
            {
                workaroundsStr = "Workarounds applied:" + workaroundsStr;
                LogManager::getSingleton().logMessage( workaroundsStr );
            }

            mInitialized = true;
        }

        win->_setDevice( mActiveDevice );
        win->create( name, width, height, fullScreen, miscParams );

        return win;
    }

    void VulkanRenderSystem::resizeAutoParamsBuffer(size_t size)
    {
        size = alignToNextMultiple(size, mDevice->mDeviceProperties.limits.minUniformBufferOffsetAlignment);
        mAutoParamsBuffer = mHardwareBufferManager->createUniformBuffer(size);
        mAutoParamsBufferPos = 0;

        mUBOInfo[0].buffer = static_cast<VulkanHardwareBuffer*>(mAutoParamsBuffer.get())->getVkBuffer();
        mUBOInfo[1].buffer = mUBOInfo[0].buffer;

        // descriptors referring to old buffer are invalidated
        for(auto& p : mProfiles)
        {
            if(!p.layout)
                continue;
            p.cache.clear();
            if(p.pool)
                mActiveDevice->mGraphicsQueue.queueForDeletion(p.pool);
            p.pool.reset( new VulkanDescriptorPool(p.poolSizes, p.layout, mDevice));
        }
    }

    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_notifyDeviceStalled()
    {
        VulkanHardwareBufferManager *hwBufferMgr =
            static_cast<VulkanHardwareBufferManager *>( mHardwareBufferManager );

        hwBufferMgr->_notifyDeviceStalled();
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_setTexture( size_t unit, bool enabled, const TexturePtr& texPtr )
    {
        setStorageTexture( unit, nullptr, 0 );

        if( texPtr && enabled)
        {
            VulkanTextureGpu *tex = static_cast<VulkanTextureGpu *>( texPtr.get() );
            mImageInfos[unit].imageView = tex->getDefaultDisplaySrv();
            mImageInfos[unit].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }
        else
        {
            mImageInfos[unit].imageView = NULL;
        }
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_beginFrame( void )
    {
        if( mCurrentRenderPassDescriptor )
            mCurrentRenderPassDescriptor->mResuming = false;

        uint32 frameIdx = mDevice->mGraphicsQueue.mCurrentFrameIdx;
        for( VkImageView view : mDeferredViewDeletions[frameIdx] )
            vkDestroyImageView( mDevice->mDevice, view, nullptr );
        mDeferredViewDeletions[frameIdx].clear();
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_notifyActiveEncoderEnded()
    {
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_notifyActiveComputeEnded( void )
    {

    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_endFrame( void )
    {
        endRenderPassDescriptor();
        //mActiveDevice->commitAndNextCommandBuffer( SubmissionType::EndFrameAndSwap );
    }

VkDescriptorSet VulkanRenderSystem::getDescriptorSet( DescriptorSetProfileId profile )
{
    auto &prf = mProfiles[profile];
    if( !prf.pool )
        return VK_NULL_HANDLE;

    if( profile == Graphics || profile == AllUnits )
    {
        const uint32 maxTextures = prf.numTextureUnits; // see note below
        uint32 hash = HashCombine( 0, mUBOInfo );

        int numTextures = 0;
        for( ; numTextures < static_cast<int>( maxTextures ); numTextures++ )
        {
            if( !mImageInfos[numTextures].imageView )
                break;
            hash = HashCombine( hash, mImageInfos[numTextures] );
        }

        VkDescriptorSet retVal = prf.cache[hash];
        if( retVal != VK_NULL_HANDLE )
            return retVal;

        retVal = prf.pool->allocate();

        prf.writes[0].dstSet = retVal;
        prf.writes[1].dstSet = retVal;
        for( int i = 0; i < numTextures; i++ )
            prf.writes[i + 2].dstSet = retVal;

        int bindCount = numTextures + 2;
        vkUpdateDescriptorSets( mActiveDevice->mDevice, bindCount, prf.writes.data(), 0, nullptr );

        prf.cache[hash] = retVal;
        return retVal;
    }

    if( profile != Compute )
        return VK_NULL_HANDLE;

    mComputeImageInfo = mStorageImageInfos[0];

    if( !mComputeImageInfo.imageView )
        return VK_NULL_HANDLE;

    uint32 hash = HashCombine( 0u, mUBOInfo[1] );
    hash = HashCombine( hash, mComputeImageInfo.imageView );

    VkDescriptorSet retVal = prf.cache[hash];
    if( retVal != VK_NULL_HANDLE )
        return retVal;

    retVal = prf.pool->allocate();
    prf.writes[0].dstSet = retVal;
    prf.writes[0].pImageInfo = &mComputeImageInfo;
    prf.writes[1].dstSet = retVal;
    vkUpdateDescriptorSets( mActiveDevice->mDevice, 2u, prf.writes.data(), 0, nullptr );
    prf.cache[hash] = retVal;
    return retVal;
}

    VkPipeline VulkanRenderSystem::getPipeline( DescriptorSetProfileId profile )
    {
        if( profile == Graphics )
        {
            auto &prf = mProfiles[Graphics];
            pipelineCi.renderPass = mCurrentRenderPassDescriptor->getRenderPass();
            pipelineCi.layout = prf.pipelineLayout;
            mssCi.rasterizationSamples =
                VkSampleCountFlagBits( std::max( mActiveRenderTarget->getFSAA(), 1u ) );

            auto hash = HashCombine( 0, pipelineCi.renderPass );
            hash = HashCombine( hash, blendStates[0] );
            hash = HashCombine( hash, rasterState );
            hash = HashCombine( hash, inputAssemblyCi );
            hash = HashCombine( hash, mssCi );
            hash = HashCombine( hash, depthStencilStateCi );

            for( uint32 i = 0; i < vertexFormatCi.vertexAttributeDescriptionCount; i++ )
            {
                hash = HashCombine( hash, vertexFormatCi.pVertexAttributeDescriptions[i] );
            }

            for( uint32 i = 0; i < vertexFormatCi.vertexBindingDescriptionCount; i++ )
            {
                hash = HashCombine( hash, vertexFormatCi.pVertexBindingDescriptions[i] );
            }

            for( uint32 i = 0; i < pipelineCi.stageCount; i++ )
            {
                hash = HashCombine( hash, mBoundGpuPrograms[i] );
            }

            VkPipeline retVal = mPipelineCache[hash];
            if( retVal != VK_NULL_HANDLE )
                return retVal;

            // if we resize the window, we create new FBOs which mean a new renderpass and invalid caches anyway..
            const VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_SCISSOR};
            VkPipelineDynamicStateCreateInfo dynamicStateCi =
                {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
            dynamicStateCi.dynamicStateCount = 1;
            dynamicStateCi.pDynamicStates = dynamicStates;
            pipelineCi.pDynamicState = &dynamicStateCi;

            OGRE_VK_CHECK(
                vkCreateGraphicsPipelines( mActiveDevice->mDevice, 0, 1, &pipelineCi, 0, &retVal ) );

            mPipelineCache[hash] = retVal;

            return retVal;
        }

        auto &prf = mProfiles[profile];
        if( prf.pipelineLayout == VK_NULL_HANDLE )
            return VK_NULL_HANDLE;

        auto hash = HashCombine( 0u, uint32( profile ) );
        hash = HashCombine( hash, mBoundGpuPrograms[GPT_COMPUTE_PROGRAM] );

        VkPipeline retVal = mComputePipelineCache[hash];
        if( retVal != VK_NULL_HANDLE )
            return retVal;

        if( !mProgramBound[GPT_COMPUTE_PROGRAM] )
            return VK_NULL_HANDLE;

        const auto &computeStage = mComputeShaderStage;
        if( computeStage.stage != VK_SHADER_STAGE_COMPUTE_BIT ||
            computeStage.module == VK_NULL_HANDLE )
        {
            return VK_NULL_HANDLE;
        }

        VkComputePipelineCreateInfo computeCi = {VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
        computeCi.layout = prf.pipelineLayout;
        computeCi.stage = computeStage;

        OGRE_VK_CHECK( vkCreateComputePipelines( mActiveDevice->mDevice, VK_NULL_HANDLE, 1u, &computeCi,
                                                 nullptr, &retVal ) );
        mComputePipelineCache[hash] = retVal;
        return retVal;
    }

    void VulkanRenderSystem::_render( const RenderOperation &op )
    {
        if ((op.useIndexes && op.indexData->indexCount == 0) || op.vertexData->vertexCount == 0)
            return;

        // Call super class.
        RenderSystem::_render( op );

        if(mActiveDevice->mGraphicsQueue.getEncoderState() != VulkanQueue::EncoderGraphicsOpen)
        {
            // ensure scissor is set
            vkCmdSetScissor(mActiveDevice->mGraphicsQueue.mCurrentCmdBuffer, 0u, 1, &mScissorRect);
            mAutoParamsBufferUsage[mActiveDevice->mGraphicsQueue.mCurrentFrameIdx] = 0;
            executeRenderPassDescriptorDelayedActions();
        }

        std::vector<VkVertexInputAttributeDescription> vertexInputs;
        uint32 uvCount = 0;
        for (auto elem : op.vertexData->vertexDeclaration->getElements())
        {
            VkVertexInputAttributeDescription inputDesc;
            inputDesc.location = VERTEX_ATTRIBUTE_INDEX[elem.getSemantic() - 1];
            if (elem.getSemantic() == VES_TEXTURE_COORDINATES)
                inputDesc.location += uvCount++;

            inputDesc.format = VulkanMappings::get( elem.getType() );
            inputDesc.binding = elem.getSource();
            inputDesc.offset = elem.getOffset();
            if (!op.vertexData->vertexBufferBinding->isBufferBound(inputDesc.binding))
                continue; // skip unbound elements
            vertexInputs.push_back(inputDesc);
        }

        OgreAssert(!op.vertexData->vertexBufferBinding->hasGaps(), "no gaps allowed");

        std::vector<VkBuffer> vertexBuffers;
        std::vector<VkVertexInputBindingDescription> bufferBindings;
        for(const auto& it : op.vertexData->vertexBufferBinding->getBindings())
        {
            auto inputRate = it.second->isInstanceData() ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;
            bufferBindings.push_back({it.first, uint32(it.second->getVertexSize()), inputRate});
            auto b = it.second->_getImpl<VulkanHardwareBuffer>()->getVkBuffer();
            vertexBuffers.push_back(b);
        }

        VkDeviceSize offsets[15] = {};

        vertexFormatCi.pVertexAttributeDescriptions = vertexInputs.data();
        vertexFormatCi.vertexAttributeDescriptionCount = vertexInputs.size();
        vertexFormatCi.pVertexBindingDescriptions = bufferBindings.data();
        vertexFormatCi.vertexBindingDescriptionCount = bufferBindings.size();

        inputAssemblyCi.topology = VulkanMappings::get( op.operationType );

        VkCommandBuffer cmdBuffer = mActiveDevice->mGraphicsQueue.mCurrentCmdBuffer;

        if(!vertexBuffers.empty())
            vkCmdBindVertexBuffers(cmdBuffer, 0, vertexBuffers.size(), vertexBuffers.data(), offsets);

        if(op.useIndexes)
        {
            auto itype = VkIndexType(op.indexData->indexBuffer->getType());
            auto b = op.indexData->indexBuffer->_getImpl<VulkanHardwareBuffer>()->getVkBuffer();
            vkCmdBindIndexBuffer(cmdBuffer, b, 0, itype);
        }

        auto pipeline = getPipeline( Graphics );
        auto descriptorSet = getDescriptorSet( Graphics );
        auto& prf = mProfiles[Graphics];
        vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, prf.pipelineLayout, 0, 1, &descriptorSet, 2,
                                mUBODynOffsets.data());

        vkCmdBindPipeline( cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline );

        // Render to screen!
        if(mProgramBound[GPT_MESH_PROGRAM])
        {
#ifdef VK_EXT_mesh_shader
            OgreAssert(op.indexData, "indexData required for mesh shader");
            vkCmdDrawMeshTasksEXT(cmdBuffer, op.indexData->indexCount, 1, 1);
#endif
        }
        else if( op.useIndexes )
        {
            do
            {
                // Update derived depth bias.
                if( mDerivedDepthBias && mCurrentPassIterationNum > 0 )
                {
                    const float biasSign = mIsReverseDepthBufferEnabled ? 1.0f : -1.0f;
                    vkCmdSetDepthBias( cmdBuffer,
                                       ( mDerivedDepthBiasBase +
                                         mDerivedDepthBiasMultiplier * mCurrentPassIterationNum ) *
                                           biasSign,
                                       0.f, mDerivedDepthBiasSlopeScale * biasSign );
                }

                vkCmdDrawIndexed(cmdBuffer, (uint32)op.indexData->indexCount, op.numberOfInstances,
                                 (uint32)op.indexData->indexStart, (int32)op.vertexData->vertexStart, 0u);
            } while( updatePassIterationRenderState() );
        }
        else
        {
            do
            {
                // Update derived depth bias.
                if( mDerivedDepthBias && mCurrentPassIterationNum > 0 )
                {
                    const float biasSign = mIsReverseDepthBufferEnabled ? 1.0f : -1.0f;
                    vkCmdSetDepthBias( cmdBuffer,
                                       ( mDerivedDepthBiasBase +
                                         mDerivedDepthBiasMultiplier * mCurrentPassIterationNum ) *
                                           biasSign,
                                       0.0f, mDerivedDepthBiasSlopeScale * biasSign );
                }
                const uint32 vertexStart = static_cast<uint32>( op.vertexData->vertexStart );
                vkCmdDraw(cmdBuffer, (uint32)op.vertexData->vertexCount, op.numberOfInstances, vertexStart, 0u);
            } while( updatePassIterationRenderState() );
        }
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_dispatchCompute( const Vector3i &workgroupDim )
    {
        if( !mActiveDevice )
            return;

        if( workgroupDim[0] <= 0 || workgroupDim[1] <= 0 || workgroupDim[2] <= 0 )
            return;

        if( !mProgramBound[GPT_COMPUTE_PROGRAM] )
        {
            LogManager::getSingleton().logWarning(
                "[Vulkan] _dispatchCompute called without a bound compute program" );
            return;
        }

        mActiveDevice->mGraphicsQueue.getComputeEncoder();
        VkCommandBuffer cmdBuffer = mActiveDevice->mGraphicsQueue.mCurrentCmdBuffer;

        // DIAGNOSTIC: verify the command buffer is actually recording
        if( cmdBuffer == VK_NULL_HANDLE )
        {
            LogManager::getSingleton().logError("[Vulkan][Compute] cmdBuffer is NULL after getComputeEncoder");
            return;
        }

        auto pipeline = getPipeline( mBoundComputeProfile );
        auto descriptorSet = getDescriptorSet( mBoundComputeProfile );
        if( pipeline == VK_NULL_HANDLE || descriptorSet == VK_NULL_HANDLE )
        {
            LogManager::getSingleton().logWarning(
                StringUtil::format("[Vulkan] Compute dispatch skipped: pipeline=%p descriptorSet=%p",
                                  (void*)pipeline, (void*)descriptorSet));
            return;
    }

        auto &prf = mProfiles[mBoundComputeProfile];
        const uint32 dynOffset = mUBODynOffsets[1];

        VulkanTextureGpu *computeTexture = mStorageTextures[0];
        if( computeTexture && computeTexture->mCurrLayout != VK_IMAGE_LAYOUT_GENERAL )
        {
            VkImageMemoryBarrier barrier = computeTexture->getImageMemoryBarrier();
            barrier.srcAccessMask = VulkanMappings::get( computeTexture ) & static_cast<VkAccessFlags>( ~VK_ACCESS_SHADER_READ_BIT );
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
            barrier.oldLayout = computeTexture->mCurrLayout;
            barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            if( barrier.srcAccessMask == 0u )
                barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier( cmdBuffer,
                                  VK_PIPELINE_STAGE_ALL_COMMANDS_BIT & mActiveDevice->mSupportedStages,
                                  VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0u, nullptr, 0u, nullptr,
                                  1u, &barrier );
            computeTexture->mCurrLayout = VK_IMAGE_LAYOUT_GENERAL;
            computeTexture->mNextLayout = VK_IMAGE_LAYOUT_GENERAL;
        }

        vkCmdBindPipeline( cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline );
        vkCmdBindDescriptorSets( cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, prf.pipelineLayout, 0, 1,
                                 &descriptorSet, 1, &dynOffset );
        vkCmdDispatch( cmdBuffer, static_cast<uint32>( workgroupDim[0] ),
                       static_cast<uint32>( workgroupDim[1] ),
                       static_cast<uint32>( workgroupDim[2] ) );

        if( computeTexture )
        {
            VkImageMemoryBarrier barrier = computeTexture->getImageMemoryBarrier();
            barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            vkCmdPipelineBarrier( cmdBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                                      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                  0, 0u, nullptr, 0u, nullptr, 1u, &barrier );
            computeTexture->mCurrLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            computeTexture->mNextLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::bindGpuProgram(GpuProgram* prg)
    {
        if(!prg)
        {
            LogManager::getSingleton().logError("[Vulkan] bindGpuProgram got null program");
            return;
        }

        auto shader = dynamic_cast<VulkanProgram*>(prg);
        if(!shader)
        {
            LogManager::getSingleton().logError(
                "[Vulkan] bindGpuProgram got non-Vulkan program: name='" + prg->getName() +
                "' syntax='" + prg->getSyntaxCode() + "'");
            return;
        }

        mBoundGpuPrograms[prg->getType()] = prg->_getHash();

        if (prg->getType() == GPT_COMPUTE_PROGRAM)
            mComputeShaderStage = shader->getPipelineShaderStageCi();
        else
            shaderStages[prg->getType() % GPT_PIPELINE_COUNT] = shader->getPipelineShaderStageCi();

        RenderSystem::bindGpuProgram(prg);
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::bindGpuProgramParameters( GpuProgramType gptype,
                                                   const GpuProgramParametersPtr& params,
                                                   uint16 variabilityMask )
    {
        mActiveParameters[gptype] = params;

        // Graphics: vertex → slot 0, fragment → slot 1
        // Compute:  always → slot 1  (matches Compute descriptor layout)
        int dstUBO;
        if( gptype == GPT_COMPUTE_PROGRAM )
            dstUBO = 1;
        else
            dstUBO = gptype % GPT_PIPELINE_COUNT;

        auto sizeBytes = params->getConstantList().size();
        if(sizeBytes && dstUBO < 2)
        {
            auto step =
                alignToNextMultiple(sizeBytes, mDevice->mDeviceProperties.limits.minUniformBufferOffsetAlignment);
            mUBOInfo[dstUBO].range = sizeBytes;

            if (std::accumulate(mAutoParamsBufferUsage.begin(), mAutoParamsBufferUsage.end(), 0) + step >=
                mAutoParamsBuffer->getSizeInBytes())
            {
                resizeAutoParamsBuffer(mAutoParamsBuffer->getSizeInBytes() * 2);
            }

            if((mAutoParamsBufferPos + sizeBytes) >= mAutoParamsBuffer->getSizeInBytes())
                mAutoParamsBufferPos = 0;

            mUBODynOffsets[dstUBO] = mAutoParamsBufferPos;

            mAutoParamsBuffer->writeData(mAutoParamsBufferPos, sizeBytes, params->getConstantList().data());
            mAutoParamsBufferPos += step;
            mAutoParamsBufferUsage[mActiveDevice->mGraphicsQueue.mCurrentFrameIdx] += step;

            if(mAutoParamsBufferPos >= mAutoParamsBuffer->getSizeInBytes())
                mAutoParamsBufferPos = 0;
        }
    }
    //-------------------------------------------------------------------------
    Real VulkanRenderSystem::getHorizontalTexelOffset( void ) { return 0.0f; }
    //-------------------------------------------------------------------------
    Real VulkanRenderSystem::getVerticalTexelOffset( void ) { return 0.0f; }
    //-------------------------------------------------------------------------
    Real VulkanRenderSystem::getMinimumDepthInputValue( void ) { return 0.0f; }
    //-------------------------------------------------------------------------
    Real VulkanRenderSystem::getMaximumDepthInputValue( void ) { return 1.0f; }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::beginProfileEvent(const String& eventName)
    {
        if(vkCmdDebugMarkerBeginEXT)
        {
            VkDebugMarkerMarkerInfoEXT markerInfo = {VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT};
            markerInfo.pMarkerName = eventName.c_str();
            vkCmdDebugMarkerBeginEXT(mActiveDevice->mGraphicsQueue.mCurrentCmdBuffer, &markerInfo);
        }
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::endProfileEvent(void) {
                if(vkCmdDebugMarkerEndEXT)
        {
            vkCmdDebugMarkerEndEXT(mActiveDevice->mGraphicsQueue.mCurrentCmdBuffer);
        }
 }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::markProfileEvent(const String& event) {}
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::initGPUProfiling( void ) {}
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::deinitGPUProfiling( void ) {}
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::beginGPUSampleProfile( const String &name, uint32 *hashCache ) {}
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::endGPUSampleProfile( const String &name ) {}
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::executeRenderPassDescriptorDelayedActions( bool officialCall )
    {
        mActiveDevice->mGraphicsQueue.endAllEncoders( false );
        mCurrentRenderPassDescriptor->performLoadActions();
        mActiveDevice->mGraphicsQueue.getGraphicsEncoder();
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::endRenderPassDescriptor()
    {
        if( mCurrentRenderPassDescriptor )
        {
            mCurrentRenderPassDescriptor->performStoreActions();
        }
    }
    //-------------------------------------------------------------------------

    DepthBuffer *VulkanRenderSystem::_createDepthBufferFor( RenderTarget* )
    {
        return NULL;
    }

    //-------------------------------------------------------------------------
    void VulkanRenderSystem::notifySwapchainDestroyed()
    {
        clearPipelineCache();
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::setColourBlendState(const ColourBlendState& state)
    {
        blendStates[0].blendEnable = state.blendingEnabled();
        blendStates[0].srcColorBlendFactor = VulkanMappings::get(state.sourceFactor);
        blendStates[0].dstColorBlendFactor = VulkanMappings::get(state.destFactor);
        blendStates[0].colorBlendOp = VulkanMappings::get(state.operation);
        blendStates[0].srcAlphaBlendFactor = VulkanMappings::get(state.sourceFactorAlpha);
        blendStates[0].dstAlphaBlendFactor = VulkanMappings::get(state.destFactorAlpha);
        blendStates[0].alphaBlendOp = VulkanMappings::get(state.alphaOperation);
        blendStates[0].colorWriteMask = state.writeA << 3 | state.writeB << 2 | state.writeG << 1 | int(state.writeR);

        for( uint i = 1; i < blendStateCi.attachmentCount; ++i )
            blendStates[i] = blendStates[0];
    }

    void VulkanRenderSystem::_setSampler(size_t texUnit, Sampler& sampler)
    {
        mImageInfos[texUnit].sampler = static_cast<VulkanSampler&>(sampler).bind();
    }

    void VulkanRenderSystem::_setPolygonMode(PolygonMode level)
    {
        rasterState.polygonMode = VulkanMappings::get(level);
    }

    void VulkanRenderSystem::_convertProjectionMatrix(const Matrix4& matrix, Matrix4& dest, bool)
    {
        dest = matrix;

        if (auto win = dynamic_cast<VulkanWindow*>(mActiveRenderTarget))
        {
            int transform = win->getSurfaceTransform();
            if (transform != VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
            {
                auto angle = Degree(90) * (transform - 1);
                dest = Matrix4(Quaternion(angle, Vector3::UNIT_Z)) * dest;
            }
        }

        if (mIsReverseDepthBufferEnabled)
        {
            // Convert depth range from [-1,+1] to [1,0]
            dest[2][0] = (dest[2][0] - dest[3][0]) * -0.5f;
            dest[2][1] = (dest[2][1] - dest[3][1]) * -0.5f;
            dest[2][2] = (dest[2][2] - dest[3][2]) * -0.5f;
            dest[2][3] = (dest[2][3] - dest[3][3]) * -0.5f;
        }
        else
        {
            // Convert depth range from [-1,+1] to [0,1]
            dest[2][0] = (dest[2][0] + dest[3][0]) / 2;
            dest[2][1] = (dest[2][1] + dest[3][1]) / 2;
            dest[2][2] = (dest[2][2] + dest[3][2]) / 2;
            dest[2][3] = (dest[2][3] + dest[3][3]) / 2;
        }
    }

    void VulkanRenderSystem::_setCullingMode(CullingMode mode)
    {
        rasterState.cullMode = VulkanMappings::get(mode);
        // flipFrontFace result is inverted due to inverted clip space Y
        rasterState.frontFace = !flipFrontFace() ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;
    }

    void VulkanRenderSystem::_setDepthBias(float constantBias, float slopeScaleBias)
    {
        rasterState.depthBiasEnable = (std::abs(constantBias) + std::abs(slopeScaleBias)) != 0.0f;
        rasterState.depthBiasConstantFactor = -constantBias;
        rasterState.depthBiasSlopeFactor = -slopeScaleBias;

        if(mIsReverseDepthBufferEnabled)
        {
            rasterState.depthBiasConstantFactor *= -1;
            rasterState.depthBiasSlopeFactor *= -1;
        }
    }

    void VulkanRenderSystem::_setDepthClamp(bool enable)
    {
        rasterState.depthClampEnable = enable;
    }

    void VulkanRenderSystem::_setAlphaRejectSettings(CompareFunction func, unsigned char value, bool alphaToCoverage)
    {
        mssCi.alphaToCoverageEnable = (func != CMPF_ALWAYS_PASS) && alphaToCoverage;
    }

    void VulkanRenderSystem::_setDepthBufferParams(bool depthTest, bool depthWrite, CompareFunction func)
    {
        depthStencilStateCi.depthTestEnable = depthTest;
        depthStencilStateCi.depthWriteEnable = depthWrite;

        if (isReverseDepthBufferEnabled())
            func = reverseCompareFunction(func);

        depthStencilStateCi.depthCompareOp = VulkanMappings::get(func);
    }

    void VulkanRenderSystem::setScissorTest(bool enabled, const Rect& rect)
    {
        if (!enabled)
        {
            mScissorRect = {{int32(mVkViewport.x), int32(mVkViewport.y)},
                            {uint32(mVkViewport.width), uint32(mVkViewport.height)}};
        }
        else
        {
            mScissorRect = {{int32(rect.left), int32(rect.top)}, {uint32(rect.width()), uint32(rect.height())}};
        }

        vkCmdSetScissor(mActiveDevice->mGraphicsQueue.mCurrentCmdBuffer, 0u, 1, &mScissorRect);
    }

    void VulkanRenderSystem::setStencilState(const StencilState& state)
    {
        depthStencilStateCi.stencilTestEnable = state.enabled;
        depthStencilStateCi.front.failOp = VulkanMappings::get(state.stencilFailOp);
        depthStencilStateCi.front.passOp = VulkanMappings::get(state.depthStencilPassOp);
        depthStencilStateCi.front.depthFailOp = VulkanMappings::get(state.depthFailOp);
        depthStencilStateCi.front.compareOp = VulkanMappings::get(state.compareOp);
        depthStencilStateCi.front.compareMask = state.compareMask;
        depthStencilStateCi.front.writeMask = state.writeMask;
        depthStencilStateCi.front.reference = state.referenceValue;

        depthStencilStateCi.back.failOp = VulkanMappings::get(state.stencilFailOp);
        depthStencilStateCi.back.passOp = VulkanMappings::get(state.depthStencilPassOp);
        depthStencilStateCi.back.depthFailOp = VulkanMappings::get(state.depthFailOp);
        depthStencilStateCi.back.compareOp = VulkanMappings::get(state.compareOp);
        depthStencilStateCi.back.compareMask = state.compareMask;
        depthStencilStateCi.back.writeMask = state.writeMask;
        depthStencilStateCi.back.reference = state.referenceValue;
    }

    void VulkanRenderSystem::_setViewport(Viewport *vp)
    {
        if (!vp)
        {
            mActiveViewport = NULL;
            _setRenderTarget(NULL);
        }
        else if (vp != mActiveViewport || vp->_isUpdated())
        {
            auto target = vp->getTarget();
            _setRenderTarget(target);
            mActiveViewport = vp;

            // Calculate the "lower-left" corner of the viewport
            Rect vpRect = vp->getActualDimensions();
            if (!target->requiresTextureFlipping())
            {
                // Convert "upper-left" corner to "lower-left"
                std::swap(vpRect.top, vpRect.bottom);
                vpRect.top = target->getHeight() - vpRect.top;
                vpRect.bottom = target->getHeight() - vpRect.bottom;
            }

            if (auto win = dynamic_cast<VulkanWindow*>(mActiveRenderTarget))
            {
                if (win->getSurfaceTransform() &
                    (VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR | VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR))
                {
                    std::swap(vpRect.bottom, vpRect.right);
                    std::swap(vpRect.top, vpRect.left);
                }
            }

            mVkViewport.x = vpRect.left;
            mVkViewport.y = vpRect.top;
            mVkViewport.width = vpRect.width();
            mVkViewport.height = vpRect.height();

            setScissorTest(false); // reset scissor

            vp->_clearUpdatedFlag();
        }
    }
    void VulkanRenderSystem::_setRenderTarget(RenderTarget *target)
    {
        mActiveRenderTarget = target;

        if (!target)
            return;

        if(auto win = dynamic_cast<VulkanWindow*>(target))
        {
            mCurrentRenderPassDescriptor = win->getRenderPassDescriptor();
        }
        if(auto rtt = dynamic_cast<VulkanRenderTexture*>(target))
        {
            mCurrentRenderPassDescriptor = rtt->getRenderPassDescriptor();
        }
    }
    void VulkanRenderSystem::clearFrameBuffer(unsigned int buffers, const ColourValue& colour, float depth,
                                              unsigned short stencil)
    {
        mCurrentRenderPassDescriptor->setClearColour(colour);
        mCurrentRenderPassDescriptor->setClearDepth(depth);
    }
}  // namespace Ogre
