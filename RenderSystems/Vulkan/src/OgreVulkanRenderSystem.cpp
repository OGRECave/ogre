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

#include "OgreRenderPassDescriptor.h"
#include "OgreVulkanCache.h"
#include "OgreVulkanDelayedFuncs.h"
#include "OgreVulkanDevice.h"
#include "OgreVulkanGpuProgramManager.h"
#include "OgreVulkanMappings.h"
#include "OgreVulkanProgramFactory.h"
#include "OgreVulkanRenderPassDescriptor.h"
#include "OgreVulkanResourceTransition.h"
#include "OgreVulkanRootLayout.h"
#include "OgreVulkanSupport.h"
#include "OgreVulkanTextureGpuManager.h"
#include "OgreVulkanUtils.h"
#include "OgreVulkanWindow.h"
#include "Vao/OgreVulkanVaoManager.h"

#include "OgreDefaultHardwareBufferManager.h"
#include "Vao/OgreVertexArrayObject.h"

#include "CommandBuffer/OgreCbDrawCall.h"

#include "OgreVulkanHlmsPso.h"
#include "Vao/OgreIndirectBufferPacked.h"
#include "Vao/OgreVulkanBufferInterface.h"
#include "Vao/OgreVulkanConstBufferPacked.h"
#include "Vao/OgreVulkanTexBufferPacked.h"

#include "OgreVulkanHardwareBufferManager.h"
#include "OgreVulkanHardwareIndexBuffer.h"
#include "OgreVulkanHardwareVertexBuffer.h"

#include "OgreVulkanDescriptorSets.h"

#include "OgreVulkanHardwareIndexBuffer.h"
#include "OgreVulkanHardwareVertexBuffer.h"
#include "OgreVulkanTextureGpu.h"
#include "Vao/OgreVulkanUavBufferPacked.h"

#include "OgreDepthBuffer.h"
#include "OgreRoot.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#    include "Windowing/win32/OgreVulkanWin32Window.h"
#elif OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
#    include "Windowing/Android/OgreVulkanAndroidWindow.h"
#else
#    include "Windowing/X11/OgreVulkanXcbWindow.h"
#endif

#include "OgrePixelFormatGpuUtils.h"

#define TODO_addVpCount_to_passpso

namespace Ogre
{
    /*static bool checkLayers( const FastArray<layer_properties> &layer_props,
                             const FastArray<const char *> &layer_names )
    {
        uint32_t check_count = layer_names.size();
        uint32_t layer_count = layer_props.size();
        for( uint32_t i = 0; i < check_count; i++ )
        {
            VkBool32 found = 0;
            for( uint32_t j = 0; j < layer_count; j++ )
            {
                if( !strcmp( layer_names[i], layer_props[j].properties.layerName ) )
                {
                    found = 1;
                }
            }
            if( !found )
            {
                std::cout << "Cannot find layer: " << layer_names[i] << std::endl;
                return 0;
            }
        }
        return 1;
    }*/
    VKAPI_ATTR VkBool32 VKAPI_CALL dbgFunc( VkFlags msgFlags, VkDebugReportObjectTypeEXT objType,
                                            uint64_t srcObject, size_t location, int32_t msgCode,
                                            const char *pLayerPrefix, const char *pMsg, void *pUserData )
    {
        // clang-format off
        char *message = (char *)malloc(strlen(pMsg) + 100);

        assert(message);

        if (msgFlags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
            sprintf(message, "INFORMATION: [%s] Code %d : %s", pLayerPrefix, msgCode, pMsg);
        } else if (msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
            sprintf(message, "WARNING: [%s] Code %d : %s", pLayerPrefix, msgCode, pMsg);
        } else if (msgFlags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
            sprintf(message, "PERFORMANCE WARNING: [%s] Code %d : %s", pLayerPrefix, msgCode, pMsg);
        } else if (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
            sprintf(message, "ERROR: [%s] Code %d : %s", pLayerPrefix, msgCode, pMsg);
            static_cast<VulkanRenderSystem*>( pUserData )->debugCallback();
        } else if (msgFlags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
            sprintf(message, "DEBUG: [%s] Code %d : %s", pLayerPrefix, msgCode, pMsg);
        } else {
            sprintf(message, "INFORMATION: [%s] Code %d : %s", pLayerPrefix, msgCode, pMsg);
        }

        LogManager::getSingleton().logMessage( message );

        free(message);

        // clang-format on

        /*
         * false indicates that layer should not bail-out of an
         * API call that had validation failures. This may mean that the
         * app dies inside the driver due to invalid parameter(s).
         * That's what would happen without validation layers, so we'll
         * keep that behavior here.
         */
        return false;
        // return true;
    }

    //-------------------------------------------------------------------------
    VulkanRenderSystem::VulkanRenderSystem() :
        RenderSystem(),
        mInitialized( false ),
        mHardwareBufferManager( 0 ),
        mIndirectBuffer( 0 ),
        mShaderManager( 0 ),
        mVulkanProgramFactory0( 0 ),
        mVulkanProgramFactory1( 0 ),
        mVulkanProgramFactory2( 0 ),
        mVulkanProgramFactory3( 0 ),
        mVkInstance( 0 ),
        mAutoParamsBufferIdx( 0 ),
        mCurrentAutoParamsBufferPtr( 0 ),
        mCurrentAutoParamsBufferSpaceLeft( 0 ),
        mActiveDevice( 0 ),
        mDevice( 0 ),
        mCache( 0 ),
        mPso( 0 ),
        mComputePso( 0 ),
        mStencilRefValue( 0u ),
        mStencilEnabled( false ),
        mTableDirty( false ),
        mComputeTableDirty( false ),
        mDummyBuffer( 0 ),
        mDummyTexBuffer( 0 ),
        mDummyTextureView( 0 ),
        mDummySampler( 0 ),
        mEntriesToFlush( 0u ),
        mVpChanged( false ),
        mInterruptedRenderCommandEncoder( false ),
        mValidationError( false ),
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        mHasWin32Support( false ),
#elif OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
        mHasAndroidSupport( false ),
#else
        mHasXcbSupport( false ),
#endif
#if OGRE_DEBUG_MODE >= OGRE_DEBUG_HIGH
        mHasValidationLayers( false ),
#endif
        CreateDebugReportCallback( 0 ),
        DestroyDebugReportCallback( 0 ),
        mDebugReportCallback( 0 )
    {
        memset( &mGlobalTable, 0, sizeof( mGlobalTable ) );
        mGlobalTable.reset();

        memset( &mComputeTable, 0, sizeof( mComputeTable ) );
        mComputeTable.reset();

        for( size_t i = 0u; i < NUM_BIND_TEXTURES; ++i )
        {
            mGlobalTable.textures[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            mComputeTable.textures[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }

        mInvertedClipSpaceY = true;

        mVulkanSupport = Ogre::getVulkanSupport();

        initConfigOptions();
    }
    //-------------------------------------------------------------------------
    VulkanRenderSystem::~VulkanRenderSystem()
    {
        shutdown();

        if( mVulkanSupport )
        {
            OGRE_DELETE mVulkanSupport;
            mVulkanSupport = 0;
        }

        if( mDebugReportCallback )
        {
            DestroyDebugReportCallback( mVkInstance, mDebugReportCallback, 0 );
            mDebugReportCallback = 0;
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

        {
            // Remove all windows.
            // (destroy primary window last since others may depend on it)
            Window *primary = 0;
            WindowSet::const_iterator itor = mWindows.begin();
            WindowSet::const_iterator endt = mWindows.end();

            while( itor != endt )
            {
                if( !primary && ( *itor )->isPrimary() )
                    primary = *itor;
                else
                    OGRE_DELETE *itor;

                ++itor;
            }

            OGRE_DELETE primary;
            mWindows.clear();
        }

        destroyAllRenderPassDescriptors();
        _cleanupDepthBuffers();
        OGRE_ASSERT_LOW( mSharedDepthBufferRefs.empty() &&
                         "destroyAllRenderPassDescriptors followed by _cleanupDepthBuffers should've "
                         "emptied mSharedDepthBufferRefs. Please report this bug to "
                         "https://github.com/OGRECave/ogre-next/issues/" );

        if( mDummySampler )
        {
            vkDestroySampler( mActiveDevice->mDevice, mDummySampler, 0 );
            mDummySampler = 0;
        }

        if( mDummyTextureView )
        {
            vkDestroyImageView( mActiveDevice->mDevice, mDummyTextureView, 0 );
            mDummyTextureView = 0;
        }

        if( mDummyTexBuffer )
        {
            mVaoManager->destroyTexBuffer( mDummyTexBuffer );
            mDummyTexBuffer = 0;
        }

        if( mDummyBuffer )
        {
            mVaoManager->destroyConstBuffer( mDummyBuffer );
            mDummyBuffer = 0;
        }

        OGRE_DELETE mHardwareBufferManager;
        mHardwareBufferManager = 0;

        OGRE_DELETE mCache;
        mCache = 0;

        OGRE_DELETE mShaderManager;
        mShaderManager = 0;

        OGRE_DELETE mTextureGpuManager;
        mTextureGpuManager = 0;
        OGRE_DELETE mVaoManager;
        mVaoManager = 0;

        OGRE_DELETE mVulkanProgramFactory3;  // LIFO destruction order
        mVulkanProgramFactory3 = 0;
        OGRE_DELETE mVulkanProgramFactory2;
        mVulkanProgramFactory2 = 0;
        OGRE_DELETE mVulkanProgramFactory1;
        mVulkanProgramFactory1 = 0;
        OGRE_DELETE mVulkanProgramFactory0;
        mVulkanProgramFactory0 = 0;

        VkDevice vkDevice = mDevice->mDevice;
        delete mDevice;
        mDevice = 0;
        vkDestroyDevice( vkDevice, 0 );
    }
    //-------------------------------------------------------------------------
    const String &VulkanRenderSystem::getName( void ) const
    {
        static String strName( "Vulkan Rendering Subsystem" );
        return strName;
    }
    //-------------------------------------------------------------------------
    const String &VulkanRenderSystem::getFriendlyName( void ) const
    {
        static String strName( "Vulkan_RS" );
        return strName;
    }
    void VulkanRenderSystem::initConfigOptions( void ) { mVulkanSupport->addConfig( this ); }
    //-------------------------------------------------------------------------
    ConfigOptionMap &VulkanRenderSystem::getConfigOptions( void )
    {
        return mVulkanSupport->getConfigOptions( this );
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::setConfigOption( const String &name, const String &value )
    {
        mVulkanSupport->setConfigOption( name, value );
    }
    //-------------------------------------------------------------------------
    String VulkanRenderSystem::validateConfigOptions( void )
    {
        return mVulkanSupport->validateConfigOptions();
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::debugCallback( void ) { mValidationError = true; }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::addInstanceDebugCallback( void )
    {
        CreateDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
            mVkInstance, "vkCreateDebugReportCallbackEXT" );
        DestroyDebugReportCallback = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
            mVkInstance, "vkDestroyDebugReportCallbackEXT" );
        if( !CreateDebugReportCallback )
        {
            LogManager::getSingleton().logMessage(
                "[Vulkan] GetProcAddr: Unable to find vkCreateDebugReportCallbackEXT. "
                "Debug reporting won't be available" );
            return;
        }
        if( !DestroyDebugReportCallback )
        {
            LogManager::getSingleton().logMessage(
                "[Vulkan] GetProcAddr: Unable to find vkDestroyDebugReportCallbackEXT. "
                "Debug reporting won't be available" );
            return;
        }
        // DebugReportMessage =
        //    (PFN_vkDebugReportMessageEXT)vkGetInstanceProcAddr( mVkInstance, "vkDebugReportMessageEXT"
        //    );
        // if( !DebugReportMessage )
        //{
        //    LogManager::getSingleton().logMessage(
        //        "[Vulkan] GetProcAddr: Unable to find DebugReportMessage. "
        //        "Debug reporting won't be available" );
        //}

        VkDebugReportCallbackCreateInfoEXT dbgCreateInfo;
        PFN_vkDebugReportCallbackEXT callback;
        makeVkStruct( dbgCreateInfo, VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT );
        callback = dbgFunc;
        dbgCreateInfo.pfnCallback = callback;
        dbgCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
                              VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
        dbgCreateInfo.pUserData = this;
        VkResult result =
            CreateDebugReportCallback( mVkInstance, &dbgCreateInfo, 0, &mDebugReportCallback );
        switch( result )
        {
        case VK_SUCCESS:
            break;
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            OGRE_VK_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR, result,
                            "CreateDebugReportCallback: out of host memory",
                            "VulkanDevice::addInstanceDebugCallback" );
        default:
            OGRE_VK_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR, result, "vkCreateDebugReportCallbackEXT",
                            "VulkanDevice::addInstanceDebugCallback" );
        }
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
            "[Vulkan] API Version: " +
            StringConverter::toString( VK_VERSION_MAJOR( properties.apiVersion ) ) + "." +
            StringConverter::toString( VK_VERSION_MINOR( properties.apiVersion ) ) + "." +
            StringConverter::toString( VK_VERSION_PATCH( properties.apiVersion ) ) + " (" +
            StringConverter::toString( properties.apiVersion, 0, ' ', std::ios::hex ) + ")" );
        LogManager::getSingleton().logMessage(
            "[Vulkan] Driver Version (raw): " +
            StringConverter::toString( properties.driverVersion, 0, ' ', std::ios::hex ) );
        LogManager::getSingleton().logMessage(
            "[Vulkan] Vendor ID: " +
            StringConverter::toString( properties.vendorID, 0, ' ', std::ios::hex ) );
        LogManager::getSingleton().logMessage(
            "[Vulkan] Device ID: " +
            StringConverter::toString( properties.deviceID, 0, ' ', std::ios::hex ) );

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
            rsc->setVendor( GPU_IMGTEC );  // PowerVR
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

        if( mActiveDevice->mDeviceFeatures.imageCubeArray )
            rsc->setCapability( RSC_TEXTURE_CUBE_MAP_ARRAY );

        if( mActiveDevice->mDeviceFeatures.depthClamp )
            rsc->setCapability( RSC_DEPTH_CLAMP );

        {
            VkFormatProperties props;

            vkGetPhysicalDeviceFormatProperties( mDevice->mPhysicalDevice,
                                                 VulkanMappings::get( PFG_BC1_UNORM ), &props );
            if( props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT )
                rsc->setCapability( RSC_TEXTURE_COMPRESSION_DXT );

            vkGetPhysicalDeviceFormatProperties( mDevice->mPhysicalDevice,
                                                 VulkanMappings::get( PFG_BC4_UNORM ), &props );
            if( props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT )
                rsc->setCapability( RSC_TEXTURE_COMPRESSION_BC4_BC5 );

            vkGetPhysicalDeviceFormatProperties( mDevice->mPhysicalDevice,
                                                 VulkanMappings::get( PFG_BC6H_UF16 ), &props );
            if( props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT )
                rsc->setCapability( RSC_TEXTURE_COMPRESSION_BC6H_BC7 );

            // Vulkan doesn't allow supporting ETC1 without ETC2
            vkGetPhysicalDeviceFormatProperties( mDevice->mPhysicalDevice,
                                                 VulkanMappings::get( PFG_ETC2_RGB8_UNORM ), &props );
            if( props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT )
            {
                rsc->setCapability( RSC_TEXTURE_COMPRESSION_ETC1 );
                rsc->setCapability( RSC_TEXTURE_COMPRESSION_ETC2 );
            }

            vkGetPhysicalDeviceFormatProperties( mDevice->mPhysicalDevice,
                                                 VulkanMappings::get( PFG_PVRTC_RGB2 ), &props );
            if( props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT )
                rsc->setCapability( RSC_TEXTURE_COMPRESSION_PVRTC );

            vkGetPhysicalDeviceFormatProperties(
                mDevice->mPhysicalDevice, VulkanMappings::get( PFG_ASTC_RGBA_UNORM_4X4_LDR ), &props );
            if( props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT )
                rsc->setCapability( RSC_TEXTURE_COMPRESSION_ASTC );
        }

        const VkPhysicalDeviceLimits &deviceLimits = mDevice->mDeviceProperties.limits;
        rsc->setMaximumResolutions( deviceLimits.maxImageDimension2D, deviceLimits.maxImageDimension3D,
                                    deviceLimits.maxImageDimensionCube );
        rsc->setMaxThreadsPerThreadgroupAxis( deviceLimits.maxComputeWorkGroupSize );
        rsc->setMaxThreadsPerThreadgroup( deviceLimits.maxComputeWorkGroupInvocations );

        if( mActiveDevice->mDeviceFeatures.samplerAnisotropy && deviceLimits.maxSamplerAnisotropy > 1u )
        {
            rsc->setCapability( RSC_ANISOTROPY );
            rsc->setMaxSupportedAnisotropy( deviceLimits.maxSamplerAnisotropy );
        }

        rsc->setCapability( RSC_STORE_AND_MULTISAMPLE_RESOLVE );
        rsc->setCapability( RSC_TEXTURE_GATHER );

        rsc->setCapability( RSC_COMPUTE_PROGRAM );
        rsc->setCapability( RSC_UAV );
        rsc->setCapability( RSC_TYPED_UAV_LOADS );
        rsc->setCapability( RSC_EXPLICIT_FSAA_RESOLVE );
        rsc->setCapability( RSC_TEXTURE_1D );

        rsc->setCapability( RSC_HWSTENCIL );
        rsc->setStencilBufferBitDepth( 8 );
        rsc->setNumTextureUnits( NUM_BIND_TEXTURES );
        rsc->setCapability( RSC_AUTOMIPMAP );
        rsc->setCapability( RSC_BLENDING );
        rsc->setCapability( RSC_DOT3 );
        rsc->setCapability( RSC_CUBEMAPPING );
        rsc->setCapability( RSC_TEXTURE_COMPRESSION );
        rsc->setCapability( RSC_VBO );
        rsc->setCapability( RSC_TWO_SIDED_STENCIL );
        rsc->setCapability( RSC_STENCIL_WRAP );
        rsc->setCapability( RSC_USER_CLIP_PLANES );
        rsc->setCapability( RSC_VERTEX_FORMAT_UBYTE4 );
        rsc->setCapability( RSC_INFINITE_FAR_PLANE );
        rsc->setCapability( RSC_TEXTURE_3D );
        rsc->setCapability( RSC_NON_POWER_OF_2_TEXTURES );
        rsc->setNonPOW2TexturesLimited( false );
        rsc->setCapability( RSC_HWRENDER_TO_TEXTURE );
        rsc->setCapability( RSC_TEXTURE_FLOAT );
        rsc->setCapability( RSC_POINT_SPRITES );
        rsc->setCapability( RSC_POINT_EXTENDED_PARAMETERS );
        rsc->setCapability( RSC_TEXTURE_2D_ARRAY );
        rsc->setCapability( RSC_CONST_BUFFER_SLOTS_IN_SHADER );
        rsc->setCapability( RSC_SEPARATE_SAMPLERS_FROM_TEXTURES );
        rsc->setCapability( RSC_ALPHA_TO_COVERAGE );
        rsc->setCapability( RSC_HW_GAMMA );
        rsc->setCapability( RSC_VERTEX_BUFFER_INSTANCE_DATA );
        rsc->setCapability( RSC_EXPLICIT_API );
        rsc->setMaxPointSize( 256 );

        rsc->setMaximumResolutions( 16384, 4096, 16384 );

        rsc->setVertexProgramConstantFloatCount( 256u );
        rsc->setVertexProgramConstantIntCount( 256u );
        rsc->setVertexProgramConstantBoolCount( 256u );
        rsc->setGeometryProgramConstantFloatCount( 256u );
        rsc->setGeometryProgramConstantIntCount( 256u );
        rsc->setGeometryProgramConstantBoolCount( 256u );
        rsc->setFragmentProgramConstantFloatCount( 256u );
        rsc->setFragmentProgramConstantIntCount( 256u );
        rsc->setFragmentProgramConstantBoolCount( 256u );
        rsc->setTessellationHullProgramConstantFloatCount( 256u );
        rsc->setTessellationHullProgramConstantIntCount( 256u );
        rsc->setTessellationHullProgramConstantBoolCount( 256u );
        rsc->setTessellationDomainProgramConstantFloatCount( 256u );
        rsc->setTessellationDomainProgramConstantIntCount( 256u );
        rsc->setTessellationDomainProgramConstantBoolCount( 256u );
        rsc->setComputeProgramConstantFloatCount( 256u );
        rsc->setComputeProgramConstantIntCount( 256u );
        rsc->setComputeProgramConstantBoolCount( 256u );

        rsc->addShaderProfile( "hlslvk" );
        rsc->addShaderProfile( "hlsl" );
        rsc->addShaderProfile( "glslvk" );
        rsc->addShaderProfile( "glsl" );

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
        OGRE_ASSERT_HIGH( dynamic_cast<VulkanConstBufferPacked *>( mDummyBuffer ) );
        VulkanConstBufferPacked *constBuffer = static_cast<VulkanConstBufferPacked *>( mDummyBuffer );

        VkDescriptorBufferInfo dummyBufferInfo;
        constBuffer->getBufferInfo( dummyBufferInfo );

        for( size_t i = 0u; i < NumShaderTypes + 1u; ++i )
        {
            mGlobalTable.paramsBuffer[i] = dummyBufferInfo;
            mComputeTable.paramsBuffer[i] = dummyBufferInfo;
        }

        for( size_t i = 0u; i < NUM_BIND_CONST_BUFFERS; ++i )
        {
            mGlobalTable.constBuffers[i] = dummyBufferInfo;
            mComputeTable.constBuffers[i] = dummyBufferInfo;
        }

        for( size_t i = 0u; i < NUM_BIND_READONLY_BUFFERS; ++i )
            mGlobalTable.readOnlyBuffers[i] = dummyBufferInfo;

        // Compute (mComputeTable) only uses baked descriptors for Textures and TexBuffers
        // hence no need to clean the emulated bindings
        OGRE_ASSERT_HIGH( dynamic_cast<VulkanTexBufferPacked *>( mDummyTexBuffer ) );
        VulkanTexBufferPacked *texBuffer = static_cast<VulkanTexBufferPacked *>( mDummyTexBuffer );

        VkBufferView texBufferView = texBuffer->_bindBufferCommon( 0u, 0u );
        for( size_t i = 0u; i < NUM_BIND_TEX_BUFFERS; ++i )
            mGlobalTable.texBuffers[i] = texBufferView;

        for( size_t i = 0u; i < NUM_BIND_TEXTURES; ++i )
        {
            mGlobalTable.textures[i].imageView = mDummyTextureView;
            mGlobalTable.textures[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }

        for( size_t i = 0u; i < NUM_BIND_SAMPLERS; ++i )
            mGlobalTable.samplers[i].sampler = mDummySampler;

        for( size_t i = 0u; i < BakedDescriptorSets::NumBakedDescriptorSets; ++i )
        {
            mGlobalTable.bakedDescriptorSets[i] = 0;
            mComputeTable.bakedDescriptorSets[i] = 0;
        }

        mGlobalTable.setAllDirty();
        mComputeTable.setAllDirty();
        mTableDirty = true;
        mComputeTableDirty = true;
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::reinitialise( void )
    {
        this->shutdown();
        this->_initialise( true );
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::initializeVkInstance( void )
    {
        if( mVkInstance )
            return;

        LogManager::getSingleton().logMessage( "[Vulkan] Initializing VkInstance" );

        uint32 numExtensions = 0u;
        VkResult result = vkEnumerateInstanceExtensionProperties( 0, &numExtensions, 0 );
        checkVkResult( result, "vkEnumerateInstanceExtensionProperties" );

        FastArray<VkExtensionProperties> availableExtensions;
        availableExtensions.resize( numExtensions );
        result =
            vkEnumerateInstanceExtensionProperties( 0, &numExtensions, availableExtensions.begin() );
        checkVkResult( result, "vkEnumerateInstanceExtensionProperties" );

        // Check supported extensions we may want
        FastArray<const char *> reqInstanceExtensions;
        for( size_t i = 0u; i < numExtensions; ++i )
        {
            const String extensionName = availableExtensions[i].extensionName;
            LogManager::getSingleton().logMessage( "Found instance extension: " + extensionName );

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
            if( extensionName == VulkanWin32Window::getRequiredExtensionName() )
            {
                mHasWin32Support = true;
                reqInstanceExtensions.push_back( VulkanWin32Window::getRequiredExtensionName() );
            }
#elif OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
            if( extensionName == VulkanAndroidWindow::getRequiredExtensionName() )
            {
                mHasAndroidSupport = true;
                reqInstanceExtensions.push_back( VulkanAndroidWindow::getRequiredExtensionName() );
            }
#else
            if( extensionName == VulkanXcbWindow::getRequiredExtensionName() )
            {
                mHasXcbSupport = true;
                reqInstanceExtensions.push_back( VulkanXcbWindow::getRequiredExtensionName() );
            }
#endif
#if OGRE_DEBUG_MODE >= OGRE_DEBUG_HIGH
            if( extensionName == VK_EXT_DEBUG_REPORT_EXTENSION_NAME )
                reqInstanceExtensions.push_back( VK_EXT_DEBUG_REPORT_EXTENSION_NAME );
#endif
        }

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        if( !mHasWin32Support )
#elif OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
        if( !mHasAndroidSupport )
#else
        if( !mHasXcbSupport )
#endif
        {
            LogManager::getSingleton().logMessage(
                "Vulkan support found but instance is uncapable of "
                "drawing to the screen! Cannot continue",
                LML_CRITICAL );
            return;
        }

        // Check supported layers we may want
        uint32 numInstanceLayers = 0u;
        result = vkEnumerateInstanceLayerProperties( &numInstanceLayers, 0 );
        checkVkResult( result, "vkEnumerateInstanceLayerProperties" );

        FastArray<VkLayerProperties> instanceLayerProps;
        instanceLayerProps.resize( numInstanceLayers );
        result = vkEnumerateInstanceLayerProperties( &numInstanceLayers, instanceLayerProps.begin() );
        checkVkResult( result, "vkEnumerateInstanceLayerProperties" );

        FastArray<const char *> instanceLayers;
        for( size_t i = 0u; i < numInstanceLayers; ++i )
        {
            const String layerName = instanceLayerProps[i].layerName;
            LogManager::getSingleton().logMessage( "Found instance layer: " + layerName );
#if OGRE_DEBUG_MODE >= OGRE_DEBUG_HIGH
            if( layerName == "VK_LAYER_KHRONOS_validation" )
            {
                mHasValidationLayers = true;
                instanceLayers.push_back( "VK_LAYER_KHRONOS_validation" );
            }
#endif
        }

#if OGRE_DEBUG_MODE >= OGRE_DEBUG_HIGH
        if( !mHasValidationLayers )
        {
            LogManager::getSingleton().logMessage(
                "WARNING: VK_LAYER_KHRONOS_validation layer not present. "
                "Extension " VK_EXT_DEBUG_MARKER_EXTENSION_NAME " not found",
                LML_CRITICAL );
        }
#endif

        mVkInstance = VulkanDevice::createInstance(
            Root::getSingleton().getAppName(), reqInstanceExtensions, instanceLayers, dbgFunc, this );

#if OGRE_DEBUG_MODE >= OGRE_DEBUG_HIGH
        addInstanceDebugCallback();
#endif
    }
    //-------------------------------------------------------------------------
    Window *VulkanRenderSystem::_initialise( bool autoCreateWindow, const String &windowTitle )
    {
        Window *autoWindow = 0;
        if( autoCreateWindow )
        {
            NameValuePairList miscParams;

            bool bFullscreen = false;
            uint32 w = 800, h = 600;

            const ConfigOptionMap &options = mVulkanSupport->getConfigOptions( this );
            ConfigOptionMap::const_iterator opt;
            ConfigOptionMap::const_iterator end = options.end();

            if( ( opt = options.find( "Full Screen" ) ) != end )
                bFullscreen = ( opt->second.currentValue == "Yes" );
            if( ( opt = options.find( "Video Mode" ) ) != end )
            {
                String val = opt->second.currentValue;
                String::size_type pos = val.find( 'x' );

                if( pos != String::npos )
                {
                    w = StringConverter::parseUnsignedInt( val.substr( 0, pos ) );
                    h = StringConverter::parseUnsignedInt( val.substr( pos + 1 ) );
                }
            }
            if( ( opt = options.find( "FSAA" ) ) != end )
                miscParams["FSAA"] = opt->second.currentValue;
            if( ( opt = options.find( "VSync" ) ) != end )
                miscParams["vsync"] = opt->second.currentValue;
            if( ( opt = options.find( "sRGB Gamma Conversion" ) ) != end )
                miscParams["gamma"] = opt->second.currentValue;
            if( ( opt = options.find( "VSync Method" ) ) != end )
                miscParams["vsync_method"] = opt->second.currentValue;

            autoWindow = _createRenderWindow( windowTitle, w, h, bFullscreen, &miscParams );
        }
        RenderSystem::_initialise( autoCreateWindow, windowTitle );

        return autoWindow;
    }
    //-------------------------------------------------------------------------
    Window *VulkanRenderSystem::_createRenderWindow( const String &name, uint32 width, uint32 height,
                                                     bool fullScreen,
                                                     const NameValuePairList *miscParams )
    {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        VulkanWindow *win = OGRE_NEW VulkanWin32Window( name, width, height, fullScreen );
#elif OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
        VulkanWindow *win = OGRE_NEW VulkanAndroidWindow( name, width, height, fullScreen );
#else
        VulkanWindow *win = OGRE_NEW VulkanXcbWindow( name, width, height, fullScreen );
#endif
        mWindows.insert( win );

        if( !mInitialized )
        {
            if( miscParams )
            {
                NameValuePairList::const_iterator itOption = miscParams->find( "reverse_depth" );
                if( itOption != miscParams->end() )
                    mReverseDepth = StringConverter::parseBool( itOption->second, true );
            }

            initializeVkInstance();

            mDevice = new VulkanDevice( mVkInstance, mVulkanSupport->getSelectedDeviceIdx(), this );
            mActiveDevice = mDevice;

            mRealCapabilities = createRenderSystemCapabilities();
            mCurrentCapabilities = mRealCapabilities;

            initialiseFromRenderSystemCapabilities( mCurrentCapabilities, 0 );

            mNativeShadingLanguageVersion = 450;

            bool bCanRestrictImageViewUsage = false;

            FastArray<const char *> deviceExtensions;
            {
                uint32 numExtensions = 0;
                vkEnumerateDeviceExtensionProperties( mDevice->mPhysicalDevice, 0, &numExtensions, 0 );

                FastArray<VkExtensionProperties> availableExtensions;
                availableExtensions.resize( numExtensions );
                vkEnumerateDeviceExtensionProperties( mDevice->mPhysicalDevice, 0, &numExtensions,
                                                      availableExtensions.begin() );
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
                }
            }

            if( !bCanRestrictImageViewUsage )
            {
                LogManager::getSingleton().logMessage(
                    "WARNING: " VK_KHR_MAINTENANCE2_EXTENSION_NAME
                    " not present. We may have to force the driver to do UAV + SRGB operations "
                    "the GPU should support, but it's not guaranteed to work" );
            }

#if OGRE_DEBUG_MODE >= OGRE_DEBUG_HIGH
            if( mHasValidationLayers )
                deviceExtensions.push_back( VK_EXT_DEBUG_MARKER_EXTENSION_NAME );
#endif

            mDevice->createDevice( deviceExtensions, 0u, 0u );

            VulkanVaoManager *vaoManager = OGRE_NEW VulkanVaoManager( mDevice, this, miscParams );
            mVaoManager = vaoManager;
            mHardwareBufferManager = OGRE_NEW v1::VulkanHardwareBufferManager( mDevice, mVaoManager );

            mActiveDevice->mVaoManager = vaoManager;
            mActiveDevice->initQueues();
            vaoManager->initDrawIdVertexBuffer();

            FastArray<PixelFormatGpu> depthFormatCandidates( 5u );
            if( PixelFormatGpuUtils::isStencil( DepthBuffer::DefaultDepthBufferFormat ) )
            {
                depthFormatCandidates.push_back( PFG_D32_FLOAT_S8X24_UINT );
                depthFormatCandidates.push_back( PFG_D24_UNORM_S8_UINT );
                depthFormatCandidates.push_back( PFG_D32_FLOAT );
                depthFormatCandidates.push_back( PFG_D24_UNORM );
                depthFormatCandidates.push_back( PFG_D16_UNORM );
            }
            else
            {
                depthFormatCandidates.push_back( PFG_D32_FLOAT );
                depthFormatCandidates.push_back( PFG_D24_UNORM );
                depthFormatCandidates.push_back( PFG_D32_FLOAT_S8X24_UINT );
                depthFormatCandidates.push_back( PFG_D24_UNORM_S8_UINT );
                depthFormatCandidates.push_back( PFG_D16_UNORM );
            }

            DepthBuffer::DefaultDepthBufferFormat = findSupportedFormat(
                mDevice->mPhysicalDevice, depthFormatCandidates, VK_IMAGE_TILING_OPTIMAL,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT );

            VulkanTextureGpuManager *textureGpuManager = OGRE_NEW VulkanTextureGpuManager(
                vaoManager, this, mDevice, bCanRestrictImageViewUsage );
            mTextureGpuManager = textureGpuManager;

            uint32 dummyData = 0u;
            mDummyBuffer = vaoManager->createConstBuffer( 4u, BT_IMMUTABLE, &dummyData, false );
            mDummyTexBuffer =
                vaoManager->createTexBuffer( PFG_RGBA8_UNORM, 4u, BT_IMMUTABLE, &dummyData, false );

            {
                VkImage dummyImage =
                    textureGpuManager->getBlankTextureVulkanName( TextureTypes::Type2D );

                VkImageViewCreateInfo imageViewCi;
                makeVkStruct( imageViewCi, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO );
                imageViewCi.image = dummyImage;
                imageViewCi.viewType = VK_IMAGE_VIEW_TYPE_2D;
                imageViewCi.format = VK_FORMAT_R8G8B8A8_UNORM;
                imageViewCi.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                imageViewCi.subresourceRange.levelCount = 1u;
                imageViewCi.subresourceRange.layerCount = 1u;

                VkResult result =
                    vkCreateImageView( mActiveDevice->mDevice, &imageViewCi, 0, &mDummyTextureView );
                checkVkResult( result, "vkCreateImageView" );
            }

            {
                VkSamplerCreateInfo samplerDescriptor;
                makeVkStruct( samplerDescriptor, VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO );
                float maxAllowedAnisotropy =
                    mActiveDevice->mDeviceProperties.limits.maxSamplerAnisotropy;
                samplerDescriptor.maxAnisotropy = maxAllowedAnisotropy;
                samplerDescriptor.anisotropyEnable = VK_FALSE;
                samplerDescriptor.minLod = -std::numeric_limits<float>::max();
                samplerDescriptor.maxLod = std::numeric_limits<float>::max();
                VkResult result =
                    vkCreateSampler( mActiveDevice->mDevice, &samplerDescriptor, 0, &mDummySampler );
                checkVkResult( result, "vkCreateSampler" );
            }

            resetAllBindings();

            String workaroundsStr;
            Workarounds::dump( (void *)&workaroundsStr );
            if( !workaroundsStr.empty() )
            {
                workaroundsStr = "Workarounds applied:" + workaroundsStr;
                LogManager::getSingleton().logMessage( workaroundsStr );
            }

            mInitialized = true;
        }

        win->_setDevice( mActiveDevice );
        win->_initialize( mTextureGpuManager, miscParams );

        return win;
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_notifyDeviceStalled()
    {
        v1::VulkanHardwareBufferManager *hwBufferMgr =
            static_cast<v1::VulkanHardwareBufferManager *>( mHardwareBufferManager );
        VulkanVaoManager *vaoManager = static_cast<VulkanVaoManager *>( mVaoManager );

        hwBufferMgr->_notifyDeviceStalled();
        vaoManager->_notifyDeviceStalled();
    }
    //-------------------------------------------------------------------------
    String VulkanRenderSystem::getErrorDescription( long errorNumber ) const { return BLANKSTRING; }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_useLights( const LightList &lights, unsigned short limit ) {}
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_setWorldMatrix( const Matrix4 &m ) {}
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_setViewMatrix( const Matrix4 &m ) {}
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_setProjectionMatrix( const Matrix4 &m ) {}
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_setSurfaceParams( const ColourValue &ambient, const ColourValue &diffuse,
                                                const ColourValue &specular, const ColourValue &emissive,
                                                Real shininess, TrackVertexColourType tracking )
    {
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_setPointSpritesEnabled( bool enabled ) {}
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_setPointParameters( Real size, bool attenuationEnabled, Real constant,
                                                  Real linear, Real quadratic, Real minSize,
                                                  Real maxSize )
    {
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::flushUAVs( void )
    {
        if( mUavRenderingDirty )
        {
            if( !mUavRenderingDescSet )
            {
                if( mGlobalTable.bakedDescriptorSets[BakedDescriptorSets::UavBuffers] )
                {
                    mGlobalTable.bakedDescriptorSets[BakedDescriptorSets::UavBuffers] = 0;
                    mGlobalTable.bakedDescriptorSets[BakedDescriptorSets::UavTextures] = 0;
                    mGlobalTable.dirtyBakedUavs = true;
                    mTableDirty = true;
                }
            }
            else
            {
                VulkanDescriptorSetUav *vulkanSet =
                    reinterpret_cast<VulkanDescriptorSetUav *>( mUavRenderingDescSet->mRsData );
                if( mGlobalTable.bakedDescriptorSets[BakedDescriptorSets::UavBuffers] !=
                    &vulkanSet->mWriteDescSets[0] )
                {
                    mGlobalTable.bakedDescriptorSets[BakedDescriptorSets::UavBuffers] =
                        &vulkanSet->mWriteDescSets[0];
                    mGlobalTable.bakedDescriptorSets[BakedDescriptorSets::UavTextures] =
                        &vulkanSet->mWriteDescSets[1];
                    mGlobalTable.dirtyBakedUavs = true;
                    mTableDirty = true;
                }
            }

            mUavRenderingDirty = false;
        }
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_setParamBuffer( GpuProgramType shaderStage,
                                              const VkDescriptorBufferInfo &bufferInfo )
    {
        if( shaderStage != GPT_COMPUTE_PROGRAM )
        {
            if( mGlobalTable.paramsBuffer[shaderStage].buffer != bufferInfo.buffer ||
                mGlobalTable.paramsBuffer[shaderStage].offset != bufferInfo.offset ||
                mGlobalTable.paramsBuffer[shaderStage].range != bufferInfo.range )
            {
                mGlobalTable.paramsBuffer[shaderStage] = bufferInfo;
                mGlobalTable.dirtyParamsBuffer = true;
                mTableDirty = true;
            }
        }
        else
        {
            if( mComputeTable.paramsBuffer[shaderStage].buffer != bufferInfo.buffer ||
                mComputeTable.paramsBuffer[shaderStage].offset != bufferInfo.offset ||
                mComputeTable.paramsBuffer[shaderStage].range != bufferInfo.range )
            {
                mComputeTable.paramsBuffer[shaderStage] = bufferInfo;
                mComputeTable.dirtyParamsBuffer = true;
                mComputeTableDirty = true;
            }
        }
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_setConstBuffer( size_t slot, const VkDescriptorBufferInfo &bufferInfo )
    {
        OGRE_ASSERT_MEDIUM( slot < NUM_BIND_CONST_BUFFERS );
        if( mGlobalTable.constBuffers[slot].buffer != bufferInfo.buffer ||
            mGlobalTable.constBuffers[slot].offset != bufferInfo.offset ||
            mGlobalTable.constBuffers[slot].range != bufferInfo.range )
        {
            mGlobalTable.constBuffers[slot] = bufferInfo;
            mGlobalTable.minDirtySlotConst = std::min( mGlobalTable.minDirtySlotConst, (uint8)slot );
            mTableDirty = true;
        }
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_setConstBufferCS( size_t slot, const VkDescriptorBufferInfo &bufferInfo )
    {
        OGRE_ASSERT_MEDIUM( slot < NUM_BIND_CONST_BUFFERS );
        if( mComputeTable.constBuffers[slot].buffer != bufferInfo.buffer ||
            mComputeTable.constBuffers[slot].offset != bufferInfo.offset ||
            mComputeTable.constBuffers[slot].range != bufferInfo.range )
        {
            mComputeTable.constBuffers[slot] = bufferInfo;
            mComputeTable.minDirtySlotConst = std::min( mComputeTable.minDirtySlotConst, (uint8)slot );
            mComputeTableDirty = true;
        }
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_setTexBuffer( size_t slot, VkBufferView bufferView )
    {
        OGRE_ASSERT_MEDIUM( slot < NUM_BIND_TEX_BUFFERS );
        if( mGlobalTable.texBuffers[slot] != bufferView )
        {
            mGlobalTable.texBuffers[slot] = bufferView;
            mGlobalTable.minDirtySlotTexBuffer =
                std::min( mGlobalTable.minDirtySlotTexBuffer, (uint8)slot );
            mTableDirty = true;
        }
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_setTexBufferCS( size_t slot, VkBufferView bufferView )
    {
        OGRE_ASSERT_MEDIUM( slot < NUM_BIND_TEX_BUFFERS );
        if( mComputeTable.texBuffers[slot] != bufferView )
        {
            mComputeTable.texBuffers[slot] = bufferView;
            mComputeTable.minDirtySlotTexBuffer =
                std::min( mComputeTable.minDirtySlotTexBuffer, (uint8)slot );
            mComputeTableDirty = true;
        }
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_setReadOnlyBuffer( size_t slot, const VkDescriptorBufferInfo &bufferInfo )
    {
        OGRE_ASSERT_MEDIUM( slot < NUM_BIND_READONLY_BUFFERS );
        if( mGlobalTable.readOnlyBuffers[slot].buffer != bufferInfo.buffer ||
            mGlobalTable.readOnlyBuffers[slot].offset != bufferInfo.offset ||
            mGlobalTable.readOnlyBuffers[slot].range != bufferInfo.range )
        {
            mGlobalTable.readOnlyBuffers[slot] = bufferInfo;
            mGlobalTable.minDirtySlotReadOnlyBuffer =
                std::min( mGlobalTable.minDirtySlotReadOnlyBuffer, (uint8)slot );
            mTableDirty = true;
        }
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_setCurrentDeviceFromTexture( TextureGpu *texture ) {}
    //-------------------------------------------------------------------------
#if OGRE_DEBUG_MODE >= OGRE_DEBUG_HIGH
    static void checkTextureLayout( const TextureGpu *texture,
                                    RenderPassDescriptor *currentRenderPassDescriptor )
    {
        OGRE_ASSERT_HIGH( dynamic_cast<const VulkanTextureGpu *>( texture ) );
        const VulkanTextureGpu *tex = static_cast<const VulkanTextureGpu *>( texture );

        if( tex->isDataReady() && tex->mCurrLayout != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL &&
            tex->mCurrLayout != VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL )
        {
            TextureGpu *targetTex;
            uint8 targetMip;
            currentRenderPassDescriptor->findAnyTexture( &targetTex, targetMip );
            String texName = targetTex ? targetTex->getNameStr() : "";
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                         "Texture " + tex->getNameStr() +
                             " is not in ResourceLayout::Texture nor RenderTargetReadOnly."
                             " Did you forget to expose it to  compositor? "
                             "Currently rendering to target: " +
                             texName,
                         "VulkanRenderSystem::checkTextureLayout" );
        }
    }
#endif
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_setTexture( size_t unit, TextureGpu *texPtr, bool bDepthReadOnly )
    {
        OGRE_ASSERT_MEDIUM( unit < NUM_BIND_TEXTURES );
        if( texPtr )
        {
            VulkanTextureGpu *tex = static_cast<VulkanTextureGpu *>( texPtr );

#if OGRE_DEBUG_MODE >= OGRE_DEBUG_HIGH
            checkTextureLayout( tex, mCurrentRenderPassDescriptor );
#endif
            const VkImageLayout targetLayout = bDepthReadOnly
                                                   ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
                                                   : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            if( mGlobalTable.textures[unit].imageView != tex->getDefaultDisplaySrv() ||
                mGlobalTable.textures[unit].imageLayout != targetLayout )
            {
                mGlobalTable.textures[unit].imageView = tex->getDefaultDisplaySrv();
                mGlobalTable.textures[unit].imageLayout = targetLayout;

                mGlobalTable.minDirtySlotTextures =
                    std::min( mGlobalTable.minDirtySlotTextures, (uint8)unit );
                mTableDirty = true;
            }
        }
        else
        {
            if( mGlobalTable.textures[unit].imageView != mDummyTextureView )
            {
                mGlobalTable.textures[unit].imageView = mDummyTextureView;
                mGlobalTable.textures[unit].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                mGlobalTable.minDirtySlotTextures =
                    std::min( mGlobalTable.minDirtySlotTextures, (uint8)unit );
                mTableDirty = true;
            }
        }
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_setTextures( uint32 slotStart, const DescriptorSetTexture *set,
                                           uint32 hazardousTexIdx )
    {
#if OGRE_DEBUG_MODE >= OGRE_DEBUG_HIGH
        {
            FastArray<const TextureGpu *>::const_iterator itor = set->mTextures.begin();
            FastArray<const TextureGpu *>::const_iterator endt = set->mTextures.end();

            while( itor != endt )
            {
                checkTextureLayout( *itor, mCurrentRenderPassDescriptor );
                ++itor;
            }
        }
#endif
        VulkanDescriptorSetTexture *vulkanSet =
            reinterpret_cast<VulkanDescriptorSetTexture *>( set->mRsData );

        VkWriteDescriptorSet *writeDescSet = &vulkanSet->mWriteDescSet;
        if( hazardousTexIdx < set->mTextures.size() &&
            mCurrentRenderPassDescriptor->hasAttachment( set->mTextures[hazardousTexIdx] ) )
        {
            vulkanSet->setHazardousTex( *set, hazardousTexIdx,
                                        static_cast<VulkanTextureGpuManager *>( mTextureGpuManager ) );
            writeDescSet = &vulkanSet->mWriteDescSetHazardous;
        }

        if( mGlobalTable.bakedDescriptorSets[BakedDescriptorSets::Textures] != writeDescSet )
        {
            mGlobalTable.bakedDescriptorSets[BakedDescriptorSets::TexBuffers] = 0;
            mGlobalTable.bakedDescriptorSets[BakedDescriptorSets::Textures] = writeDescSet;
            mGlobalTable.bakedDescriptorSets[BakedDescriptorSets::UavBuffers] = 0;
            mGlobalTable.dirtyBakedTextures = true;
            mTableDirty = true;
        }
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_setTextures( uint32 slotStart, const DescriptorSetTexture2 *set )
    {
#if OGRE_DEBUG_MODE >= OGRE_DEBUG_HIGH
        {
            FastArray<DescriptorSetTexture2::Slot>::const_iterator itor = set->mTextures.begin();
            FastArray<DescriptorSetTexture2::Slot>::const_iterator endt = set->mTextures.end();

            while( itor != endt )
            {
                if( itor->isTexture() )
                    checkTextureLayout( itor->getTexture().texture, mCurrentRenderPassDescriptor );
                ++itor;
            }
        }
#endif
        VulkanDescriptorSetTexture2 *vulkanSet =
            reinterpret_cast<VulkanDescriptorSetTexture2 *>( set->mRsData );

        if( mGlobalTable.bakedDescriptorSets[BakedDescriptorSets::ReadOnlyBuffers] !=
            &vulkanSet->mWriteDescSets[0] )
        {
            mGlobalTable.bakedDescriptorSets[BakedDescriptorSets::ReadOnlyBuffers] =
                &vulkanSet->mWriteDescSets[0];
            mGlobalTable.bakedDescriptorSets[BakedDescriptorSets::TexBuffers] =
                &vulkanSet->mWriteDescSets[1];
            mGlobalTable.bakedDescriptorSets[BakedDescriptorSets::Textures] =
                &vulkanSet->mWriteDescSets[2];
            mGlobalTable.dirtyBakedTextures = true;
            mTableDirty = true;
        }
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_setSamplers( uint32 slotStart, const DescriptorSetSampler *set )
    {
        VulkanDescriptorSetSampler *vulkanSet =
            reinterpret_cast<VulkanDescriptorSetSampler *>( set->mRsData );

        if( mGlobalTable.bakedDescriptorSets[BakedDescriptorSets::Samplers] !=
            &vulkanSet->mWriteDescSet )
        {
            mGlobalTable.bakedDescriptorSets[BakedDescriptorSets::Samplers] = &vulkanSet->mWriteDescSet;
            mGlobalTable.dirtyBakedSamplers = true;
            mTableDirty = true;
        }
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_setTexturesCS( uint32 slotStart, const DescriptorSetTexture *set )
    {
        VulkanDescriptorSetTexture *vulkanSet =
            reinterpret_cast<VulkanDescriptorSetTexture *>( set->mRsData );

        if( mComputeTable.bakedDescriptorSets[BakedDescriptorSets::Textures] !=
            &vulkanSet->mWriteDescSet )
        {
            mComputeTable.bakedDescriptorSets[BakedDescriptorSets::ReadOnlyBuffers] = 0;
            mComputeTable.bakedDescriptorSets[BakedDescriptorSets::TexBuffers] = 0;
            mComputeTable.bakedDescriptorSets[BakedDescriptorSets::Textures] = &vulkanSet->mWriteDescSet;
            mComputeTable.dirtyBakedSamplers = true;
            mComputeTableDirty = true;
        }
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_setTexturesCS( uint32 slotStart, const DescriptorSetTexture2 *set )
    {
        VulkanDescriptorSetTexture2 *vulkanSet =
            reinterpret_cast<VulkanDescriptorSetTexture2 *>( set->mRsData );

        if( mComputeTable.bakedDescriptorSets[BakedDescriptorSets::ReadOnlyBuffers] !=
            &vulkanSet->mWriteDescSets[0] )
        {
            mComputeTable.bakedDescriptorSets[BakedDescriptorSets::ReadOnlyBuffers] =
                &vulkanSet->mWriteDescSets[0];
            mComputeTable.bakedDescriptorSets[BakedDescriptorSets::TexBuffers] =
                &vulkanSet->mWriteDescSets[1];
            mComputeTable.bakedDescriptorSets[BakedDescriptorSets::Textures] =
                &vulkanSet->mWriteDescSets[2];
            mComputeTable.dirtyBakedSamplers = true;
            mComputeTableDirty = true;
        }
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_setSamplersCS( uint32 slotStart, const DescriptorSetSampler *set )
    {
        VulkanDescriptorSetSampler *vulkanSet =
            reinterpret_cast<VulkanDescriptorSetSampler *>( set->mRsData );

        if( mComputeTable.bakedDescriptorSets[BakedDescriptorSets::Samplers] !=
            &vulkanSet->mWriteDescSet )
        {
            mComputeTable.bakedDescriptorSets[BakedDescriptorSets::Samplers] = &vulkanSet->mWriteDescSet;
            mComputeTable.dirtyBakedSamplers = true;
            mComputeTableDirty = true;
        }
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_setUavCS( uint32 slotStart, const DescriptorSetUav *set )
    {
        VulkanDescriptorSetUav *vulkanSet = reinterpret_cast<VulkanDescriptorSetUav *>( set->mRsData );

        if( mComputeTable.bakedDescriptorSets[BakedDescriptorSets::UavBuffers] !=
            &vulkanSet->mWriteDescSets[0] )
        {
            mComputeTable.bakedDescriptorSets[BakedDescriptorSets::UavBuffers] =
                &vulkanSet->mWriteDescSets[0];
            mComputeTable.bakedDescriptorSets[BakedDescriptorSets::UavTextures] =
                &vulkanSet->mWriteDescSets[1];
            mComputeTable.dirtyBakedUavs = true;
            mComputeTableDirty = true;
        }
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_setTextureCoordCalculation( size_t unit, TexCoordCalcMethod m,
                                                          const Frustum *frustum )
    {
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_setTextureBlendMode( size_t unit, const LayerBlendModeEx &bm ) {}
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_setTextureMatrix( size_t unit, const Matrix4 &xform ) {}
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_setIndirectBuffer( IndirectBufferPacked *indirectBuffer )
    {
        if( mVaoManager->supportsIndirectBuffers() )
        {
            if( indirectBuffer )
            {
                VulkanBufferInterface *bufferInterface =
                    static_cast<VulkanBufferInterface *>( indirectBuffer->getBufferInterface() );
                mIndirectBuffer = bufferInterface->getVboName();
            }
            else
            {
                mIndirectBuffer = 0;
            }
        }
        else
        {
            if( indirectBuffer )
                mSwIndirectBufferPtr = indirectBuffer->getSwBufferPtr();
            else
                mSwIndirectBufferPtr = 0;
        }
    }
    //-------------------------------------------------------------------------
    RenderPassDescriptor *VulkanRenderSystem::createRenderPassDescriptor( void )
    {
        VulkanRenderPassDescriptor *retVal =
            OGRE_NEW VulkanRenderPassDescriptor( &mActiveDevice->mGraphicsQueue, this );
        mRenderPassDescs.insert( retVal );
        return retVal;
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_hlmsComputePipelineStateObjectCreated( HlmsComputePso *newPso )
    {
        VkComputePipelineCreateInfo computeInfo;
        makeVkStruct( computeInfo, VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO );

        VulkanProgram *computeShader =
            static_cast<VulkanProgram *>( newPso->computeShader->_getBindingDelegate() );
        computeShader->fillPipelineShaderStageCi( computeInfo.stage );

        VulkanRootLayout *rootLayout = computeShader->getRootLayout();
        computeInfo.layout = rootLayout->createVulkanHandles();

        VkPipeline vulkanPso = 0u;
        VkResult result = vkCreateComputePipelines( mActiveDevice->mDevice, VK_NULL_HANDLE, 1u,
                                                    &computeInfo, 0, &vulkanPso );
        checkVkResult( result, "vkCreateComputePipelines" );

        VulkanHlmsPso *pso = new VulkanHlmsPso( vulkanPso, rootLayout );
        newPso->rsData = pso;
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_hlmsComputePipelineStateObjectDestroyed( HlmsComputePso *pso )
    {
        if( pso == mComputePso )
        {
            mComputePso = 0;
            mComputeTable.setAllDirty();
            mComputeTableDirty = true;
        }

        OGRE_ASSERT_LOW( pso->rsData );
        VulkanHlmsPso *vulkanPso = static_cast<VulkanHlmsPso *>( pso->rsData );
        delayed_vkDestroyPipeline( mVaoManager, mActiveDevice->mDevice, vulkanPso->pso, 0 );
        delete vulkanPso;
        pso->rsData = 0;
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_beginFrame( void ) {}
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_endFrame( void ) {}
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_notifyActiveEncoderEnded( bool callEndRenderPassDesc )
    {
        // VulkanQueue::endRenderEncoder gets called either because
        //  * Another encoder was required. Thus we interrupted and callEndRenderPassDesc = true
        //  * endRenderPassDescriptor called us. Thus callEndRenderPassDesc = false
        //  * executeRenderPassDescriptorDelayedActions called us. Thus callEndRenderPassDesc = false
        // In all cases, when callEndRenderPassDesc = true, it also implies rendering was interrupted.
        if( callEndRenderPassDesc )
            endRenderPassDescriptor( true );

        mUavRenderingDirty = true;
        mGlobalTable.setAllDirty();
        mTableDirty = true;
        mPso = 0;
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_notifyActiveComputeEnded( void )
    {
        mComputePso = 0;
        mComputeTable.setAllDirty();
        mComputeTableDirty = true;
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_endFrameOnce( void )
    {
        RenderSystem::_endFrameOnce();
        endRenderPassDescriptor( false );
        mActiveDevice->commitAndNextCommandBuffer( SubmissionType::EndFrameAndSwap );
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_setHlmsSamplerblock( uint8 texUnit, const HlmsSamplerblock *samplerblock )
    {
        OGRE_ASSERT_MEDIUM( texUnit < NUM_BIND_SAMPLERS );
        if( !samplerblock )
        {
            if( mGlobalTable.samplers[texUnit].sampler != mDummySampler )
            {
                mGlobalTable.samplers[texUnit].sampler = mDummySampler;
                mGlobalTable.minDirtySlotSamplers =
                    std::min( mGlobalTable.minDirtySlotSamplers, texUnit );
                mTableDirty = true;
            }
        }
        else
        {
            VkSampler textureSampler = reinterpret_cast<VkSampler>( samplerblock->mRsData );
            if( mGlobalTable.samplers[texUnit].sampler != textureSampler )
            {
                mGlobalTable.samplers[texUnit].sampler = textureSampler;
                mGlobalTable.minDirtySlotSamplers =
                    std::min( mGlobalTable.minDirtySlotSamplers, texUnit );
                mTableDirty = true;
            }
        }
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_setPipelineStateObject( const HlmsPso *pso )
    {
        if( pso && mActiveDevice->mGraphicsQueue.getEncoderState() != VulkanQueue::EncoderGraphicsOpen )
        {
            OGRE_ASSERT_LOW(
                mInterruptedRenderCommandEncoder &&
                "Encoder can't be in EncoderGraphicsOpen at this stage if rendering was interrupted."
                " Did you call executeRenderPassDescriptorDelayedActions?" );
            executeRenderPassDescriptorDelayedActions( false );
        }

        if( mPso != pso )
        {
            VulkanRootLayout *oldRootLayout = 0;
            if( mPso )
                oldRootLayout = reinterpret_cast<VulkanHlmsPso *>( mPso->rsData )->rootLayout;

            VkCommandBuffer cmdBuffer = mActiveDevice->mGraphicsQueue.mCurrentCmdBuffer;
            OGRE_ASSERT_LOW( pso->rsData );
            VulkanHlmsPso *vulkanPso = reinterpret_cast<VulkanHlmsPso *>( pso->rsData );
            vkCmdBindPipeline( cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPso->pso );
            mPso = pso;

            if( vulkanPso && vulkanPso->rootLayout != oldRootLayout )
            {
                mGlobalTable.setAllDirty();
                mTableDirty = true;
            }
        }
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_setComputePso( const HlmsComputePso *pso )
    {
        mActiveDevice->mGraphicsQueue.getComputeEncoder();

        if( mComputePso != pso )
        {
            VulkanRootLayout *oldRootLayout = 0;
            if( mComputePso )
                oldRootLayout = reinterpret_cast<VulkanHlmsPso *>( mComputePso->rsData )->rootLayout;

            VulkanHlmsPso *vulkanPso = 0;

            if( pso )
            {
                OGRE_ASSERT_LOW( pso->rsData );
                vulkanPso = reinterpret_cast<VulkanHlmsPso *>( pso->rsData );
                VkCommandBuffer cmdBuffer = mActiveDevice->mGraphicsQueue.mCurrentCmdBuffer;
                vkCmdBindPipeline( cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, vulkanPso->pso );

                if( vulkanPso->rootLayout != oldRootLayout )
                {
                    mComputeTable.setAllDirty();
                    mComputeTableDirty = true;
                }
            }

            mComputePso = pso;
        }
    }
    //-------------------------------------------------------------------------
    VertexElementType VulkanRenderSystem::getColourVertexElementType( void ) const
    {
        return VET_COLOUR_ARGB;
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_dispatch( const HlmsComputePso &pso )
    {
        flushRootLayoutCS();

        vkCmdDispatch( mActiveDevice->mGraphicsQueue.mCurrentCmdBuffer, pso.mNumThreadGroups[0],
                       pso.mNumThreadGroups[1], pso.mNumThreadGroups[2] );
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_setVertexArrayObject( const VertexArrayObject *vao )
    {
        VkBuffer vulkanVertexBuffers[15];
        VkDeviceSize offsets[15];
        memset( offsets, 0, sizeof( offsets ) );

        const VertexBufferPackedVec &vertexBuffers = vao->getVertexBuffers();

        size_t numVertexBuffers = 0;
        VertexBufferPackedVec::const_iterator itor = vertexBuffers.begin();
        VertexBufferPackedVec::const_iterator endt = vertexBuffers.end();

        while( itor != endt )
        {
            VulkanBufferInterface *bufferInterface =
                static_cast<VulkanBufferInterface *>( ( *itor )->getBufferInterface() );
            vulkanVertexBuffers[numVertexBuffers++] = bufferInterface->getVboName();
            ++itor;
        }

        OGRE_ASSERT_LOW( numVertexBuffers < 15u );

        VkCommandBuffer cmdBuffer = mActiveDevice->mGraphicsQueue.mCurrentCmdBuffer;
        if( numVertexBuffers > 0u )
        {
            vkCmdBindVertexBuffers( cmdBuffer, 0, static_cast<uint32>( numVertexBuffers ),
                                    vulkanVertexBuffers, offsets );
        }

        IndexBufferPacked *indexBuffer = vao->getIndexBuffer();
        if( indexBuffer )
        {
            VulkanBufferInterface *bufferInterface =
                static_cast<VulkanBufferInterface *>( indexBuffer->getBufferInterface() );
            vkCmdBindIndexBuffer( cmdBuffer, bufferInterface->getVboName(), 0,
                                  static_cast<VkIndexType>( indexBuffer->getIndexType() ) );
        }
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::flushRootLayout( void )
    {
        if( !mTableDirty )
            return;

        VulkanVaoManager *vaoManager = static_cast<VulkanVaoManager *>( mVaoManager );
        VulkanRootLayout *rootLayout = reinterpret_cast<VulkanHlmsPso *>( mPso->rsData )->rootLayout;
        rootLayout->bind( mDevice, vaoManager, mGlobalTable );
        mGlobalTable.reset();
        mTableDirty = false;
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::flushRootLayoutCS( void )
    {
        if( !mComputeTableDirty )
            return;

        VulkanVaoManager *vaoManager = static_cast<VulkanVaoManager *>( mVaoManager );
        VulkanRootLayout *rootLayout =
            reinterpret_cast<VulkanHlmsPso *>( mComputePso->rsData )->rootLayout;
        rootLayout->bind( mDevice, vaoManager, mComputeTable );
        mComputeTable.reset();
        mComputeTableDirty = false;
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_render( const CbDrawCallIndexed *cmd )
    {
        flushRootLayout();

        VkCommandBuffer cmdBuffer = mActiveDevice->mGraphicsQueue.mCurrentCmdBuffer;
        vkCmdDrawIndexedIndirect( cmdBuffer, mIndirectBuffer,
                                  reinterpret_cast<VkDeviceSize>( cmd->indirectBufferOffset ),
                                  cmd->numDraws, sizeof( CbDrawIndexed ) );
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_render( const CbDrawCallStrip *cmd )
    {
        flushRootLayout();

        VkCommandBuffer cmdBuffer = mActiveDevice->mGraphicsQueue.mCurrentCmdBuffer;
        vkCmdDrawIndirect( cmdBuffer, mIndirectBuffer,
                           reinterpret_cast<VkDeviceSize>( cmd->indirectBufferOffset ), cmd->numDraws,
                           sizeof( CbDrawStrip ) );
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_renderEmulated( const CbDrawCallIndexed *cmd )
    {
        flushRootLayout();

        CbDrawIndexed *drawCmd = reinterpret_cast<CbDrawIndexed *>( mSwIndirectBufferPtr +
                                                                    (size_t)cmd->indirectBufferOffset );

        VkCommandBuffer cmdBuffer = mActiveDevice->mGraphicsQueue.mCurrentCmdBuffer;

        for( uint32 i = cmd->numDraws; i--; )
        {
            vkCmdDrawIndexed( cmdBuffer, drawCmd->primCount, drawCmd->instanceCount,
                              drawCmd->firstVertexIndex, static_cast<int32>( drawCmd->baseVertex ),
                              drawCmd->baseInstance );
            ++drawCmd;
        }
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_renderEmulated( const CbDrawCallStrip *cmd )
    {
        flushRootLayout();

        CbDrawStrip *drawCmd =
            reinterpret_cast<CbDrawStrip *>( mSwIndirectBufferPtr + (size_t)cmd->indirectBufferOffset );

        VkCommandBuffer cmdBuffer = mActiveDevice->mGraphicsQueue.mCurrentCmdBuffer;

        for( uint32 i = cmd->numDraws; i--; )
        {
            vkCmdDraw( cmdBuffer, drawCmd->primCount, drawCmd->instanceCount, drawCmd->firstVertexIndex,
                       drawCmd->baseInstance );
            ++drawCmd;
        }
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_setRenderOperation( const v1::CbRenderOp *cmd )
    {
        VulkanVaoManager *vaoManager = static_cast<VulkanVaoManager *>( mVaoManager );

        VkCommandBuffer cmdBuffer = mActiveDevice->mGraphicsQueue.mCurrentCmdBuffer;

        VkBuffer vulkanVertexBuffers[16];
        VkDeviceSize offsets[16];
        memset( vulkanVertexBuffers, 0, sizeof( vulkanVertexBuffers ) );
        memset( offsets, 0, sizeof( offsets ) );

        size_t maxUsedSlot = 0;
        const v1::VertexBufferBinding::VertexBufferBindingMap &binds =
            cmd->vertexData->vertexBufferBinding->getBindings();
        v1::VertexBufferBinding::VertexBufferBindingMap::const_iterator itor = binds.begin();
        v1::VertexBufferBinding::VertexBufferBindingMap::const_iterator end = binds.end();

        while( itor != end )
        {
            v1::VulkanHardwareVertexBuffer *vulkanBuffer =
                reinterpret_cast<v1::VulkanHardwareVertexBuffer *>( itor->second.get() );

            const size_t slot = itor->first;

            OGRE_ASSERT_LOW( slot < 15u );

            size_t offsetStart;
            vulkanVertexBuffers[slot] = vulkanBuffer->getBufferName( offsetStart );
            offsets[slot] = offsetStart;
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
            offsets[slot] += cmd->vertexData->vertexStart * vulkanBuffer->getVertexSize();
#endif
            ++itor;
            maxUsedSlot = std::max( maxUsedSlot, slot + 1u );
        }

        VulkanBufferInterface *bufIntf =
            static_cast<VulkanBufferInterface *>( vaoManager->getDrawId()->getBufferInterface() );
        vulkanVertexBuffers[maxUsedSlot] = bufIntf->getVboName();
        offsets[maxUsedSlot] = 0;

        vkCmdBindVertexBuffers( cmdBuffer, 0u, (uint32)maxUsedSlot + 1u, vulkanVertexBuffers, offsets );

        if( cmd->indexData )
        {
            size_t offsetStart = 0u;

            v1::VulkanHardwareIndexBuffer *vulkanBuffer =
                static_cast<v1::VulkanHardwareIndexBuffer *>( cmd->indexData->indexBuffer.get() );
            VkIndexType indexType = static_cast<VkIndexType>( vulkanBuffer->getType() );
            VkBuffer indexBuffer = vulkanBuffer->getBufferName( offsetStart );

            vkCmdBindIndexBuffer( cmdBuffer, indexBuffer, offsetStart, indexType );
        }

        mCurrentIndexBuffer = cmd->indexData;
        mCurrentVertexBuffer = cmd->vertexData;
        mCurrentPrimType = std::min( VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
                                     static_cast<VkPrimitiveTopology>( cmd->operationType - 1u ) );
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_render( const v1::CbDrawCallIndexed *cmd )
    {
        flushRootLayout();

        VkCommandBuffer cmdBuffer = mActiveDevice->mGraphicsQueue.mCurrentCmdBuffer;
        vkCmdDrawIndexed( cmdBuffer, cmd->primCount, cmd->instanceCount, cmd->firstVertexIndex,
                          (int32_t)mCurrentVertexBuffer->vertexStart, cmd->baseInstance );
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_render( const v1::CbDrawCallStrip *cmd )
    {
        flushRootLayout();

        VkCommandBuffer cmdBuffer = mActiveDevice->mGraphicsQueue.mCurrentCmdBuffer;
        vkCmdDraw( cmdBuffer, cmd->primCount, cmd->instanceCount, cmd->firstVertexIndex,
                   cmd->baseInstance );
    }

    void VulkanRenderSystem::_render( const v1::RenderOperation &op )
    {
        flushRootLayout();

        // Call super class.
        RenderSystem::_render( op );

        const size_t numberOfInstances = op.numberOfInstances;
        const bool hasInstanceData = mCurrentVertexBuffer->vertexBufferBinding->getHasInstanceData();

        VkCommandBuffer cmdBuffer = mActiveDevice->mGraphicsQueue.mCurrentCmdBuffer;

        // Render to screen!
        if( op.useIndexes )
        {
            do
            {
                // Update derived depth bias.
                if( mDerivedDepthBias && mCurrentPassIterationNum > 0 )
                {
                    const float biasSign = mReverseDepth ? 1.0f : -1.0f;
                    vkCmdSetDepthBias( cmdBuffer,
                                       ( mDerivedDepthBiasBase +
                                         mDerivedDepthBiasMultiplier * mCurrentPassIterationNum ) *
                                           biasSign,
                                       0.f, mDerivedDepthBiasSlopeScale * biasSign );
                }

                vkCmdDrawIndexed( cmdBuffer, (uint32)mCurrentIndexBuffer->indexCount,
                                  (uint32)numberOfInstances, (uint32)mCurrentIndexBuffer->indexStart,
                                  (int32)mCurrentVertexBuffer->vertexStart, 0u );
            } while( updatePassIterationRenderState() );
        }
        else
        {
            do
            {
                // Update derived depth bias.
                if( mDerivedDepthBias && mCurrentPassIterationNum > 0 )
                {
                    const float biasSign = mReverseDepth ? 1.0f : -1.0f;
                    vkCmdSetDepthBias( cmdBuffer,
                                       ( mDerivedDepthBiasBase +
                                         mDerivedDepthBiasMultiplier * mCurrentPassIterationNum ) *
                                           biasSign,
                                       0.0f, mDerivedDepthBiasSlopeScale * biasSign );
                }

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
                const uint32 vertexStart = 0;
#else
                const uint32 vertexStart = static_cast<uint32>( mCurrentVertexBuffer->vertexStart );
#endif

                vkCmdDraw( cmdBuffer, (uint32)mCurrentVertexBuffer->vertexCount,
                           (uint32)numberOfInstances, vertexStart, 0u );
            } while( updatePassIterationRenderState() );
        }
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::bindGpuProgramParameters( GpuProgramType gptype,
                                                       GpuProgramParametersSharedPtr params,
                                                       uint16 variabilityMask )
    {
        VulkanProgram *shader = 0;
        switch( gptype )
        {
        case GPT_VERTEX_PROGRAM:
            mActiveVertexGpuProgramParameters = params;
            shader = static_cast<VulkanProgram *>( mPso->vertexShader->_getBindingDelegate() );
            break;
        case GPT_FRAGMENT_PROGRAM:
            mActiveFragmentGpuProgramParameters = params;
            shader = static_cast<VulkanProgram *>( mPso->pixelShader->_getBindingDelegate() );
            break;
        case GPT_GEOMETRY_PROGRAM:
            mActiveGeometryGpuProgramParameters = params;
            shader = static_cast<VulkanProgram *>( mPso->geometryShader->_getBindingDelegate() );
            break;
        case GPT_HULL_PROGRAM:
            mActiveTessellationHullGpuProgramParameters = params;
            shader = static_cast<VulkanProgram *>( mPso->tesselationHullShader->_getBindingDelegate() );
            break;
        case GPT_DOMAIN_PROGRAM:
            mActiveTessellationDomainGpuProgramParameters = params;
            shader =
                static_cast<VulkanProgram *>( mPso->tesselationDomainShader->_getBindingDelegate() );
            break;
        case GPT_COMPUTE_PROGRAM:
            mActiveComputeGpuProgramParameters = params;
            shader = static_cast<VulkanProgram *>( mComputePso->computeShader->_getBindingDelegate() );
            break;
        }

        size_t bytesToWrite = shader->getBufferRequiredSize();
        if( shader && bytesToWrite > 0 )
        {
            if( mCurrentAutoParamsBufferSpaceLeft < bytesToWrite )
            {
                if( mAutoParamsBufferIdx >= mAutoParamsBuffer.size() )
                {
                    ConstBufferPacked *constBuffer =
                        mVaoManager->createConstBuffer( std::max<size_t>( 512u * 1024u, bytesToWrite ),
                                                        BT_DYNAMIC_PERSISTENT, 0, false );
                    mAutoParamsBuffer.push_back( constBuffer );
                }

                ConstBufferPacked *constBuffer = mAutoParamsBuffer[mAutoParamsBufferIdx];
                mCurrentAutoParamsBufferPtr =
                    reinterpret_cast<uint8 *>( constBuffer->map( 0, constBuffer->getNumElements() ) );
                mCurrentAutoParamsBufferSpaceLeft = constBuffer->getTotalSizeBytes();

                ++mAutoParamsBufferIdx;
            }

            shader->updateBuffers( params, mCurrentAutoParamsBufferPtr );

            assert( dynamic_cast<VulkanConstBufferPacked *>(
                mAutoParamsBuffer[mAutoParamsBufferIdx - 1u] ) );

            VulkanConstBufferPacked *constBuffer =
                static_cast<VulkanConstBufferPacked *>( mAutoParamsBuffer[mAutoParamsBufferIdx - 1u] );
            const size_t bindOffset =
                constBuffer->getTotalSizeBytes() - mCurrentAutoParamsBufferSpaceLeft;

            constBuffer->bindAsParamBuffer( gptype, bindOffset, bytesToWrite );

            mCurrentAutoParamsBufferPtr += bytesToWrite;

            const uint8 *oldBufferPos = mCurrentAutoParamsBufferPtr;
            mCurrentAutoParamsBufferPtr = reinterpret_cast<uint8 *>(
                alignToNextMultiple( reinterpret_cast<uintptr_t>( mCurrentAutoParamsBufferPtr ),
                                     mVaoManager->getConstBufferAlignment() ) );
            bytesToWrite += ( size_t )( mCurrentAutoParamsBufferPtr - oldBufferPos );

            // We know that bytesToWrite <= mCurrentAutoParamsBufferSpaceLeft, but that was
            // before padding. After padding this may no longer hold true.
            mCurrentAutoParamsBufferSpaceLeft -=
                std::min( mCurrentAutoParamsBufferSpaceLeft, bytesToWrite );
        }
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::bindGpuProgramPassIterationParameters( GpuProgramType gptype ) {}
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::clearFrameBuffer( RenderPassDescriptor *renderPassDesc,
                                               TextureGpu *anyTarget, uint8 mipLevel )
    {
        Vector4 fullVp( 0, 0, 1, 1 );
        beginRenderPassDescriptor( renderPassDesc, anyTarget, mipLevel,  //
                                   &fullVp, &fullVp, 1u, false, false );
        executeRenderPassDescriptorDelayedActions();
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
    void VulkanRenderSystem::preExtraThreadsStarted() {}
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::postExtraThreadsStarted() {}
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::registerThread() {}
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::unregisterThread() {}
    //-------------------------------------------------------------------------
    const PixelFormatToShaderType *VulkanRenderSystem::getPixelFormatToShaderType( void ) const
    {
        return &mPixelFormatToShaderType;
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::flushCommands( void ) {}
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::beginProfileEvent( const String &eventName ) {}
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::endProfileEvent( void ) {}
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::markProfileEvent( const String &event ) {}
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::initGPUProfiling( void ) {}
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::deinitGPUProfiling( void ) {}
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::beginGPUSampleProfile( const String &name, uint32 *hashCache ) {}
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::endGPUSampleProfile( const String &name ) {}
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::setClipPlanesImpl( const PlaneList &clipPlanes ) {}
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::initialiseFromRenderSystemCapabilities( RenderSystemCapabilities *caps,
                                                                     Window *primary )
    {
        mShaderManager = OGRE_NEW VulkanGpuProgramManager( mActiveDevice );
        mVulkanProgramFactory0 = OGRE_NEW VulkanProgramFactory( mActiveDevice, "glslvk", true );
        mVulkanProgramFactory1 = OGRE_NEW VulkanProgramFactory( mActiveDevice, "glsl", false );
        mVulkanProgramFactory2 = OGRE_NEW VulkanProgramFactory( mActiveDevice, "hlslvk", false );
        mVulkanProgramFactory3 = OGRE_NEW VulkanProgramFactory( mActiveDevice, "hlsl", false );

        HighLevelGpuProgramManager::getSingleton().addFactory( mVulkanProgramFactory0 );
        // HighLevelGpuProgramManager::getSingleton().addFactory( mVulkanProgramFactory1 );
        HighLevelGpuProgramManager::getSingleton().addFactory( mVulkanProgramFactory2 );
        // HighLevelGpuProgramManager::getSingleton().addFactory( mVulkanProgramFactory3 );

        mCache = OGRE_NEW VulkanCache( mActiveDevice );

        Log *defaultLog = LogManager::getSingleton().getDefaultLog();
        if( defaultLog )
        {
            caps->log( defaultLog );
            defaultLog->logMessage( " * Using Reverse Z: " +
                                    StringConverter::toString( mReverseDepth, true ) );
        }
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::beginRenderPassDescriptor( RenderPassDescriptor *desc,
                                                        TextureGpu *anyTarget, uint8 mipLevel,
                                                        const Vector4 *viewportSizes,
                                                        const Vector4 *scissors, uint32 numViewports,
                                                        bool overlaysEnabled, bool warnIfRtvWasFlushed )
    {
        if( desc->mInformationOnly && desc->hasSameAttachments( mCurrentRenderPassDescriptor ) )
            return;

        const bool mDebugSubmissionValidationLayers = false;
        if( mDebugSubmissionValidationLayers )
        {
            // If we get a validation layer here; then the error was generated by the
            // Pass that last called beginRenderPassDescriptor (i.e. not this one)
            endRenderPassDescriptor( false );
            mActiveDevice->commitAndNextCommandBuffer( SubmissionType::FlushOnly );
        }

        const int oldWidth = mCurrentRenderViewport[0].getActualWidth();
        const int oldHeight = mCurrentRenderViewport[0].getActualHeight();
        const int oldX = mCurrentRenderViewport[0].getActualLeft();
        const int oldY = mCurrentRenderViewport[0].getActualTop();
#if OGRE_NO_VIEWPORT_ORIENTATIONMODE == 0
        const OrientationMode oldOrientationMode =
            mCurrentRenderViewport[0].getCurrentTarget()
                ? mCurrentRenderViewport[0].getCurrentTarget()->getOrientationMode()
                : OR_DEGREE_0;
#endif

        VulkanRenderPassDescriptor *currPassDesc =
            static_cast<VulkanRenderPassDescriptor *>( mCurrentRenderPassDescriptor );

        RenderSystem::beginRenderPassDescriptor( desc, anyTarget, mipLevel, viewportSizes, scissors,
                                                 numViewports, overlaysEnabled, warnIfRtvWasFlushed );

        // Calculate the new "lower-left" corner of the viewport to compare with the old one
        const int w = mCurrentRenderViewport[0].getActualWidth();
        const int h = mCurrentRenderViewport[0].getActualHeight();
        const int x = mCurrentRenderViewport[0].getActualLeft();
        const int y = mCurrentRenderViewport[0].getActualTop();
#if OGRE_NO_VIEWPORT_ORIENTATIONMODE == 0
        const OrientationMode orientationMode =
            mCurrentRenderViewport[0].getCurrentTarget()->getOrientationMode();
#endif

        const bool vpChanged = oldX != x || oldY != y || oldWidth != w || oldHeight != h ||
                               numViewports > 1u
#if OGRE_NO_VIEWPORT_ORIENTATIONMODE == 0
                               || oldOrientationMode != orientationMode
#endif
            ;

        VulkanRenderPassDescriptor *newPassDesc = static_cast<VulkanRenderPassDescriptor *>( desc );

        // Determine whether:
        //  1. We need to store current active RenderPassDescriptor
        //  2. We need to perform clears when loading the new RenderPassDescriptor
        uint32 entriesToFlush = 0;
        if( currPassDesc )
        {
            entriesToFlush = currPassDesc->willSwitchTo( newPassDesc, warnIfRtvWasFlushed );

            if( entriesToFlush != 0 )
                currPassDesc->performStoreActions( false );

            // If rendering was interrupted but we're still rendering to the same
            // RTT, willSwitchTo will have returned 0 and thus we won't perform
            // the necessary load actions.
            // If RTTs were different, we need to have performStoreActions
            // called by now (i.e. to emulate StoreAndResolve)
            if( mInterruptedRenderCommandEncoder )
            {
                entriesToFlush = RenderPassDescriptor::All;
                if( warnIfRtvWasFlushed )
                    newPassDesc->checkWarnIfRtvWasFlushed( entriesToFlush );
            }
        }
        else
        {
            entriesToFlush = RenderPassDescriptor::All;
        }

        mEntriesToFlush = entriesToFlush;
        mVpChanged = vpChanged;
        mInterruptedRenderCommandEncoder = false;
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::executeRenderPassDescriptorDelayedActions( void )
    {
        executeRenderPassDescriptorDelayedActions( true );
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::executeRenderPassDescriptorDelayedActions( bool officialCall )
    {
        if( officialCall )
            mInterruptedRenderCommandEncoder = false;

        const bool wasGraphicsOpen =
            mActiveDevice->mGraphicsQueue.getEncoderState() != VulkanQueue::EncoderGraphicsOpen;

        if( mEntriesToFlush )
        {
            mActiveDevice->mGraphicsQueue.endAllEncoders( false );

            VulkanRenderPassDescriptor *newPassDesc =
                static_cast<VulkanRenderPassDescriptor *>( mCurrentRenderPassDescriptor );

            newPassDesc->performLoadActions( mInterruptedRenderCommandEncoder );
        }

        // This is a new command buffer / encoder. State needs to be set again
        if( mEntriesToFlush || !wasGraphicsOpen )
        {
            mActiveDevice->mGraphicsQueue.getGraphicsEncoder();

            VulkanVaoManager *vaoManager = static_cast<VulkanVaoManager *>( mVaoManager );
            vaoManager->bindDrawIdVertexBuffer( mActiveDevice->mGraphicsQueue.mCurrentCmdBuffer );

            if( mStencilEnabled )
            {
                vkCmdSetStencilReference( mActiveDevice->mGraphicsQueue.mCurrentCmdBuffer,
                                          VK_STENCIL_FACE_FRONT_AND_BACK, mStencilRefValue );
            }

            mVpChanged = true;
            mInterruptedRenderCommandEncoder = false;
        }

        flushUAVs();

        const uint32 numViewports = mMaxBoundViewports;

        // If we flushed, viewport and scissor settings got reset.
        if( mVpChanged || numViewports > 1u )
        {
            VkViewport vkVp[16];
            for( size_t i = 0; i < numViewports; ++i )
            {
                vkVp[i].x = mCurrentRenderViewport[i].getActualLeft();
                vkVp[i].y = mCurrentRenderViewport[i].getActualTop();
                vkVp[i].width = mCurrentRenderViewport[i].getActualWidth();
                vkVp[i].height = mCurrentRenderViewport[i].getActualHeight();
                vkVp[i].minDepth = 0;
                vkVp[i].maxDepth = 1;

#if OGRE_NO_VIEWPORT_ORIENTATIONMODE == 0
                if( mCurrentRenderViewport[i].getCurrentTarget()->getOrientationMode() & 0x01 )
                {
                    std::swap( vkVp[i].x, vkVp[i].y );
                    std::swap( vkVp[i].width, vkVp[i].height );
                }
#endif
            }

            vkCmdSetViewport( mActiveDevice->mGraphicsQueue.mCurrentCmdBuffer, 0u, numViewports, vkVp );
        }

        if( mVpChanged || numViewports > 1u )
        {
            VkRect2D scissorRect[16];
            for( size_t i = 0; i < numViewports; ++i )
            {
                scissorRect[i].offset.x = mCurrentRenderViewport[i].getScissorActualLeft();
                scissorRect[i].offset.y = mCurrentRenderViewport[i].getScissorActualTop();
                scissorRect[i].extent.width =
                    static_cast<uint32>( mCurrentRenderViewport[i].getScissorActualWidth() );
                scissorRect[i].extent.height =
                    static_cast<uint32>( mCurrentRenderViewport[i].getScissorActualHeight() );
#if OGRE_NO_VIEWPORT_ORIENTATIONMODE == 0
                if( mCurrentRenderViewport[i].getCurrentTarget()->getOrientationMode() & 0x01 )
                {
                    std::swap( scissorRect[i].offset.x, scissorRect[i].offset.y );
                    std::swap( scissorRect[i].extent.width, scissorRect[i].extent.height );
                }
#endif
            }

            vkCmdSetScissor( mActiveDevice->mGraphicsQueue.mCurrentCmdBuffer, 0u, numViewports,
                             scissorRect );
        }

        mEntriesToFlush = 0;
        mVpChanged = false;
        mInterruptedRenderCommandEncoder = false;
    }
    //-------------------------------------------------------------------------
    inline void VulkanRenderSystem::endRenderPassDescriptor( bool isInterruptingRender )
    {
        if( mCurrentRenderPassDescriptor )
        {
            VulkanRenderPassDescriptor *passDesc =
                static_cast<VulkanRenderPassDescriptor *>( mCurrentRenderPassDescriptor );
            passDesc->performStoreActions( isInterruptingRender );

            mEntriesToFlush = 0;
            mVpChanged = true;

            mInterruptedRenderCommandEncoder = isInterruptingRender;

            if( !isInterruptingRender )
                RenderSystem::endRenderPassDescriptor();
            else
                mEntriesToFlush = RenderPassDescriptor::All;
        }
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::endRenderPassDescriptor( void ) { endRenderPassDescriptor( false ); }
    //-------------------------------------------------------------------------
    TextureGpu *VulkanRenderSystem::createDepthBufferFor( TextureGpu *colourTexture,
                                                          bool preferDepthTexture,
                                                          PixelFormatGpu depthBufferFormat,
                                                          uint16 poolId )
    {
        if( depthBufferFormat == PFG_UNKNOWN )
            depthBufferFormat = DepthBuffer::DefaultDepthBufferFormat;

        return RenderSystem::createDepthBufferFor( colourTexture, preferDepthTexture, depthBufferFormat,
                                                   poolId );
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::notifySwapchainCreated( VulkanWindow *window )
    {
        RenderPassDescriptorSet::const_iterator itor = mRenderPassDescs.begin();
        RenderPassDescriptorSet::const_iterator endt = mRenderPassDescs.end();

        while( itor != endt )
        {
            OGRE_ASSERT_HIGH( dynamic_cast<VulkanRenderPassDescriptor *>( *itor ) );
            VulkanRenderPassDescriptor *renderPassDesc =
                static_cast<VulkanRenderPassDescriptor *>( *itor );
            renderPassDesc->notifySwapchainCreated( window );
            ++itor;
        }
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::notifySwapchainDestroyed( VulkanWindow *window )
    {
        RenderPassDescriptorSet::const_iterator itor = mRenderPassDescs.begin();
        RenderPassDescriptorSet::const_iterator endt = mRenderPassDescs.end();

        while( itor != endt )
        {
            OGRE_ASSERT_HIGH( dynamic_cast<VulkanRenderPassDescriptor *>( *itor ) );
            VulkanRenderPassDescriptor *renderPassDesc =
                static_cast<VulkanRenderPassDescriptor *>( *itor );
            renderPassDesc->notifySwapchainDestroyed( window );
            ++itor;
        }
    }
    //-------------------------------------------------------------------------
    VkRenderPass VulkanRenderSystem::getVkRenderPass( HlmsPassPso passPso, uint8 &outMrtCount )
    {
        uint8 mrtCount = 0;
        for( int i = 0; i < OGRE_MAX_MULTIPLE_RENDER_TARGETS; ++i )
        {
            if( passPso.colourFormat[i] != PFG_NULL )
                mrtCount = static_cast<uint8>( i ) + 1u;
        }
        outMrtCount = mrtCount;

        bool usesResolveAttachments = false;

        uint32 attachmentIdx = 0u;
        uint32 numColourAttachments = 0u;
        VkAttachmentDescription attachments[OGRE_MAX_MULTIPLE_RENDER_TARGETS * 2u + 2u];

        VkAttachmentReference colourAttachRefs[OGRE_MAX_MULTIPLE_RENDER_TARGETS];
        VkAttachmentReference resolveAttachRefs[OGRE_MAX_MULTIPLE_RENDER_TARGETS];
        VkAttachmentReference depthAttachRef;

        memset( attachments, 0, sizeof( attachments ) );
        memset( colourAttachRefs, 0, sizeof( colourAttachRefs ) );
        memset( resolveAttachRefs, 0, sizeof( resolveAttachRefs ) );
        memset( &depthAttachRef, 0, sizeof( depthAttachRef ) );

        for( size_t i = 0; i < mrtCount; ++i )
        {
            resolveAttachRefs[numColourAttachments].attachment = VK_ATTACHMENT_UNUSED;
            resolveAttachRefs[numColourAttachments].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            if( passPso.colourFormat[i] != PFG_NULL )
            {
                colourAttachRefs[numColourAttachments].attachment = attachmentIdx;
                colourAttachRefs[numColourAttachments].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                attachments[attachmentIdx].samples =
                    static_cast<VkSampleCountFlagBits>( passPso.sampleDescription.getColourSamples() );
                attachments[attachmentIdx].format = VulkanMappings::get( passPso.colourFormat[i] );
                attachments[attachmentIdx].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                attachments[attachmentIdx].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                ++attachmentIdx;

                if( passPso.resolveColourFormat[i] != PFG_NULL )
                {
                    usesResolveAttachments = true;

                    attachments[attachmentIdx].samples = VK_SAMPLE_COUNT_1_BIT;
                    attachments[attachmentIdx].format =
                        VulkanMappings::get( passPso.resolveColourFormat[i] );
                    attachments[attachmentIdx].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                    attachments[attachmentIdx].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                    resolveAttachRefs[numColourAttachments].attachment = attachmentIdx;
                    resolveAttachRefs[numColourAttachments].layout =
                        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                    ++attachmentIdx;
                }
                ++numColourAttachments;
            }
        }

        if( passPso.depthFormat != PFG_NULL )
        {
            attachments[attachmentIdx].format = VulkanMappings::get( passPso.depthFormat );
            attachments[attachmentIdx].samples =
                static_cast<VkSampleCountFlagBits>( passPso.sampleDescription.getColourSamples() );
            attachments[attachmentIdx].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            attachments[attachmentIdx].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            depthAttachRef.attachment = attachmentIdx;
            depthAttachRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            ++attachmentIdx;
        }

        VkSubpassDescription subpass;
        memset( &subpass, 0, sizeof( subpass ) );
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.inputAttachmentCount = 0u;
        subpass.colorAttachmentCount = numColourAttachments;
        subpass.pColorAttachments = colourAttachRefs;
        subpass.pResolveAttachments = usesResolveAttachments ? resolveAttachRefs : 0;
        subpass.pDepthStencilAttachment = ( passPso.depthFormat != PFG_NULL ) ? &depthAttachRef : 0;

        VkRenderPassCreateInfo renderPassCreateInfo;
        makeVkStruct( renderPassCreateInfo, VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO );
        renderPassCreateInfo.attachmentCount = attachmentIdx;
        renderPassCreateInfo.pAttachments = attachments;
        renderPassCreateInfo.subpassCount = 1u;
        renderPassCreateInfo.pSubpasses = &subpass;

        VkRenderPass retVal = mCache->getRenderPass( renderPassCreateInfo );
        return retVal;
    }
    //-------------------------------------------------------------------------
    static VkPipelineStageFlags toVkPipelineStageFlags( ResourceLayout::Layout layout,
                                                        const bool bIsDepth )
    {
        switch( layout )
        {
        case ResourceLayout::PresentReady:
            return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        case ResourceLayout::RenderTarget:
        case ResourceLayout::RenderTargetReadOnly:
        case ResourceLayout::Clear:
            if( bIsDepth )
                return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                       VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            else
                return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        case ResourceLayout::Texture:
        case ResourceLayout::Uav:
            return VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                   VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
                   VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT |
                   VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                   VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        case ResourceLayout::MipmapGen:
            return VK_PIPELINE_STAGE_TRANSFER_BIT;
        case ResourceLayout::CopySrc:
        case ResourceLayout::CopyDst:
            return VK_PIPELINE_STAGE_TRANSFER_BIT;
        case ResourceLayout::CopyEnd:
        case ResourceLayout::ResolveDest:
            return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        case ResourceLayout::Undefined:
        case ResourceLayout::NumResourceLayouts:
            return 0;
        }

        return 0;
    }
    //-------------------------------------------------------------------------
    static VkPipelineStageFlags ogreToVkStageFlags( uint8 ogreStageMask )
    {
        VkPipelineStageFlags retVal = 0u;
        if( ogreStageMask & ( 1u << VertexShader ) )
            retVal |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
        if( ogreStageMask & ( 1u << PixelShader ) )
            retVal |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        if( ogreStageMask & ( 1u << GeometryShader ) )
            retVal |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
        if( ogreStageMask & ( 1u << HullShader ) )
            retVal |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
        if( ogreStageMask & ( 1u << DomainShader ) )
            retVal |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
        if( ogreStageMask & ( 1u << GPT_COMPUTE_PROGRAM ) )
            retVal |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

        return retVal;
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::flushPendingAutoResourceLayouts( void )
    {
        mActiveDevice->mGraphicsQueue.endCopyEncoder();
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::executeResourceTransition( const ResourceTransitionArray &rstCollection )
    {
        if( rstCollection.empty() )
            return;

        // Needs to be done now, as it may change layouts of textures we're about to change
        mActiveDevice->mGraphicsQueue.endAllEncoders();

        VkPipelineStageFlags srcStage = 0u;
        VkPipelineStageFlags dstStage = 0u;

        VkMemoryBarrier memBarrier;

        uint32 bufferSrcBarrierBits = 0u;
        uint32 bufferDstBarrierBits = 0u;

        ResourceTransitionArray::const_iterator itor = rstCollection.begin();
        ResourceTransitionArray::const_iterator endt = rstCollection.end();

        while( itor != endt )
        {
            if( itor->resource && itor->resource->isTextureGpu() )
            {
                OGRE_ASSERT_MEDIUM( itor->oldLayout != ResourceLayout::CopyEnd &&
                                    "ResourceLayout::CopyEnd is never in oldLayout" );

                VulkanTextureGpu *texture = static_cast<VulkanTextureGpu *>( itor->resource );
                VkImageMemoryBarrier imageBarrier = texture->getImageMemoryBarrier();
                imageBarrier.oldLayout = VulkanMappings::get( itor->oldLayout, texture );
                imageBarrier.newLayout = VulkanMappings::get( itor->newLayout, texture );

                const bool bIsDepth = PixelFormatGpuUtils::isDepth( texture->getPixelFormat() );

                // If oldAccess == ResourceAccess::Undefined then this texture is used for
                // the first time on a new frame (but not necessarily the first time ever)
                // thus there are no caches needed to flush.
                //
                // dstStage only needs to wait for the transition to happen though
                if( itor->oldAccess != ResourceAccess::Undefined )
                {
                    if( itor->oldAccess & ResourceAccess::Write )
                    {
                        imageBarrier.srcAccessMask =
                            VulkanMappings::getAccessFlags( itor->oldLayout, itor->oldAccess, texture,
                                                            false ) &
                            c_srcValidAccessFlags;
                    }

                    imageBarrier.dstAccessMask = VulkanMappings::getAccessFlags(
                        itor->newLayout, itor->newAccess, texture, true );

                    if( itor->oldLayout != ResourceLayout::Texture &&
                        itor->oldLayout != ResourceLayout::Uav )
                    {
                        srcStage |= toVkPipelineStageFlags( itor->oldLayout, bIsDepth );
                    }

                    if( itor->oldStageMask != 0u )
                        srcStage |= ogreToVkStageFlags( itor->oldStageMask );
                }

                if( itor->newLayout != ResourceLayout::CopyEnd )
                {
                    if( itor->newLayout != ResourceLayout::Texture &&
                        itor->newLayout != ResourceLayout::Uav )
                    {
                        dstStage |= toVkPipelineStageFlags( itor->newLayout, bIsDepth );
                    }

                    if( itor->newStageMask != 0u )
                        dstStage |= ogreToVkStageFlags( itor->newStageMask );
                }

                texture->mCurrLayout = imageBarrier.newLayout;

                mImageBarriers.push_back( imageBarrier );

                if( texture->isMultisample() && !texture->hasMsaaExplicitResolves() )
                {
                    // Rare case where we render to an implicit resolve without resolving
                    // (otherwise newLayout = ResolveDest)
                    //
                    // Or more common case if we need to copy to/from an MSAA texture
                    //
                    // This cannot catch all use cases, but if you fall into something this
                    // doesn't catch, then you should probably be using explicit resolves
                    if( itor->newLayout == ResourceLayout::RenderTarget ||
                        itor->newLayout == ResourceLayout::ResolveDest ||
                        itor->newLayout == ResourceLayout::CopySrc ||
                        itor->newLayout == ResourceLayout::CopyDst )
                    {
                        imageBarrier.image = texture->getMsaaFramebufferName();
                        mImageBarriers.push_back( imageBarrier );
                    }
                }
            }
            else
            {
                srcStage |= ogreToVkStageFlags( itor->oldStageMask );
                dstStage |= ogreToVkStageFlags( itor->newStageMask );

                if( itor->oldAccess & ResourceAccess::Write )
                    bufferSrcBarrierBits |= VK_ACCESS_SHADER_WRITE_BIT;

                if( itor->newAccess & ResourceAccess::Read )
                    bufferDstBarrierBits |= VK_ACCESS_SHADER_READ_BIT;
                if( itor->newAccess & ResourceAccess::Write )
                    bufferDstBarrierBits |= VK_ACCESS_SHADER_WRITE_BIT;
            }
            ++itor;
        }

        uint32 numMemBarriers = 0u;
        if( bufferSrcBarrierBits || bufferDstBarrierBits )
        {
            makeVkStruct( memBarrier, VK_STRUCTURE_TYPE_MEMORY_BARRIER );
            memBarrier.srcAccessMask = bufferSrcBarrierBits & c_srcValidAccessFlags;
            memBarrier.dstAccessMask = bufferDstBarrierBits;
            numMemBarriers = 1u;
        }

        if( srcStage == 0 )
            srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        if( dstStage == 0 )
            dstStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

        vkCmdPipelineBarrier(
            mActiveDevice->mGraphicsQueue.mCurrentCmdBuffer, srcStage & mActiveDevice->mSupportedStages,
            dstStage & mActiveDevice->mSupportedStages, 0, numMemBarriers, &memBarrier, 0u, 0,
            static_cast<uint32>( mImageBarriers.size() ), mImageBarriers.begin() );
        mImageBarriers.clear();
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_hlmsPipelineStateObjectCreated( HlmsPso *newPso )
    {
        size_t numShaderStages = 0u;
        VkPipelineShaderStageCreateInfo shaderStages[NumShaderTypes];

        VulkanRootLayout *rootLayout = 0;

        VulkanProgram *vertexShader = 0;
        VulkanProgram *pixelShader = 0;

        if( !newPso->vertexShader.isNull() )
        {
            vertexShader = static_cast<VulkanProgram *>( newPso->vertexShader->_getBindingDelegate() );
            vertexShader->fillPipelineShaderStageCi( shaderStages[numShaderStages++] );
            rootLayout = VulkanRootLayout::findBest( rootLayout, vertexShader->getRootLayout() );
        }

        if( !newPso->geometryShader.isNull() )
        {
            VulkanProgram *shader =
                static_cast<VulkanProgram *>( newPso->geometryShader->_getBindingDelegate() );
            shader->fillPipelineShaderStageCi( shaderStages[numShaderStages++] );
            rootLayout = VulkanRootLayout::findBest( rootLayout, shader->getRootLayout() );
        }

        if( !newPso->tesselationHullShader.isNull() )
        {
            VulkanProgram *shader =
                static_cast<VulkanProgram *>( newPso->tesselationHullShader->_getBindingDelegate() );
            shader->fillPipelineShaderStageCi( shaderStages[numShaderStages++] );
            rootLayout = VulkanRootLayout::findBest( rootLayout, shader->getRootLayout() );
        }

        if( !newPso->tesselationDomainShader.isNull() )
        {
            VulkanProgram *shader =
                static_cast<VulkanProgram *>( newPso->tesselationDomainShader->_getBindingDelegate() );
            shader->fillPipelineShaderStageCi( shaderStages[numShaderStages++] );
            rootLayout = VulkanRootLayout::findBest( rootLayout, shader->getRootLayout() );
        }

        if( !newPso->pixelShader.isNull() )
        {
            pixelShader = static_cast<VulkanProgram *>( newPso->pixelShader->_getBindingDelegate() );
            pixelShader->fillPipelineShaderStageCi( shaderStages[numShaderStages++] );
            rootLayout = VulkanRootLayout::findBest( rootLayout, pixelShader->getRootLayout() );
        }

        if( !rootLayout )
        {
            String shaderNames =
                "The following shaders cannot be linked. Their Root Layouts are incompatible:\n";
            if( newPso->vertexShader )
            {
                shaderNames += newPso->vertexShader->getName() + "\n";
                static_cast<VulkanProgram *>( newPso->vertexShader->_getBindingDelegate() )
                    ->getRootLayout()
                    ->dump( shaderNames );
            }
            if( newPso->geometryShader )
            {
                shaderNames += newPso->geometryShader->getName() + "\n";
                static_cast<VulkanProgram *>( newPso->geometryShader->_getBindingDelegate() )
                    ->getRootLayout()
                    ->dump( shaderNames );
            }
            if( newPso->tesselationHullShader )
            {
                shaderNames += newPso->tesselationHullShader->getName() + "\n";
                static_cast<VulkanProgram *>( newPso->tesselationHullShader->_getBindingDelegate() )
                    ->getRootLayout()
                    ->dump( shaderNames );
            }
            if( newPso->tesselationDomainShader )
            {
                shaderNames += newPso->tesselationDomainShader->getName() + "\n";
                static_cast<VulkanProgram *>( newPso->tesselationDomainShader->_getBindingDelegate() )
                    ->getRootLayout()
                    ->dump( shaderNames );
            }
            if( newPso->pixelShader )
            {
                shaderNames += newPso->pixelShader->getName() + "\n";
                static_cast<VulkanProgram *>( newPso->pixelShader->_getBindingDelegate() )
                    ->getRootLayout()
                    ->dump( shaderNames );
            }

            LogManager::getSingleton().logMessage( shaderNames, LML_CRITICAL );
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "Shaders cannot be linked together. Their Root Layouts are incompatible. See "
                         "Ogre.log for more info",
                         "VulkanRenderSystem::_hlmsPipelineStateObjectCreated" );
        }

        VkPipelineLayout layout = rootLayout->createVulkanHandles();

        VkPipelineVertexInputStateCreateInfo vertexFormatCi;
        makeVkStruct( vertexFormatCi, VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO );
        FastArray<VkVertexInputBindingDescription> bindingDescriptions;
        FastArray<VkVertexInputAttributeDescription> attributeDescriptions;

        if( !newPso->vertexShader.isNull() )
        {
            VulkanProgram *shader =
                static_cast<VulkanProgram *>( newPso->vertexShader->_getBindingDelegate() );

            shader->getLayoutForPso( newPso->vertexElements, bindingDescriptions,
                                     attributeDescriptions );

            vertexFormatCi.vertexBindingDescriptionCount =
                static_cast<uint32_t>( bindingDescriptions.size() );
            vertexFormatCi.vertexAttributeDescriptionCount =
                static_cast<uint32_t>( attributeDescriptions.size() );
            vertexFormatCi.pVertexBindingDescriptions = bindingDescriptions.begin();
            vertexFormatCi.pVertexAttributeDescriptions = attributeDescriptions.begin();
        }

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyCi;
        makeVkStruct( inputAssemblyCi, VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO );
        inputAssemblyCi.topology = VulkanMappings::get( newPso->operationType );
        inputAssemblyCi.primitiveRestartEnable = newPso->enablePrimitiveRestart;

        VkPipelineTessellationStateCreateInfo tessStateCi;
        makeVkStruct( tessStateCi, VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO );

        VkPipelineViewportStateCreateInfo viewportStateCi;
        makeVkStruct( viewportStateCi, VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO );
        TODO_addVpCount_to_passpso;
        viewportStateCi.viewportCount = 1u;
        viewportStateCi.scissorCount = 1u;

        VkPipelineRasterizationStateCreateInfo rasterState;
        makeVkStruct( rasterState, VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO );
        rasterState.polygonMode = VulkanMappings::get( newPso->macroblock->mPolygonMode );
        rasterState.cullMode = VulkanMappings::get( newPso->macroblock->mCullMode );
        rasterState.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterState.depthClampEnable = newPso->macroblock->mDepthClamp;
        rasterState.depthBiasEnable = newPso->macroblock->mDepthBiasConstant != 0.0f;
        rasterState.depthBiasConstantFactor = newPso->macroblock->mDepthBiasConstant;
        rasterState.depthBiasClamp = 0.0f;
        rasterState.depthBiasSlopeFactor = newPso->macroblock->mDepthBiasSlopeScale;
        rasterState.lineWidth = 1.0f;

        VkPipelineMultisampleStateCreateInfo mssCi;
        makeVkStruct( mssCi, VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO );
        mssCi.rasterizationSamples =
            static_cast<VkSampleCountFlagBits>( newPso->pass.sampleDescription.getColourSamples() );
        mssCi.alphaToCoverageEnable = newPso->blendblock->mAlphaToCoverageEnabled;

        VkPipelineDepthStencilStateCreateInfo depthStencilStateCi;
        makeVkStruct( depthStencilStateCi, VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO );
        depthStencilStateCi.depthTestEnable = newPso->macroblock->mDepthCheck;
        depthStencilStateCi.depthWriteEnable = newPso->macroblock->mDepthWrite;
        CompareFunction depthFunc = newPso->macroblock->mDepthFunc;
        if( mReverseDepth )
            depthFunc = reverseCompareFunction( depthFunc );
        depthStencilStateCi.depthCompareOp = VulkanMappings::get( depthFunc );
        depthStencilStateCi.stencilTestEnable = newPso->pass.stencilParams.enabled;
        if( newPso->pass.stencilParams.enabled )
        {
            depthStencilStateCi.front.failOp =
                VulkanMappings::get( newPso->pass.stencilParams.stencilFront.stencilFailOp );
            depthStencilStateCi.front.passOp =
                VulkanMappings::get( newPso->pass.stencilParams.stencilFront.stencilPassOp );
            depthStencilStateCi.front.depthFailOp =
                VulkanMappings::get( newPso->pass.stencilParams.stencilFront.stencilDepthFailOp );
            depthStencilStateCi.front.compareOp =
                VulkanMappings::get( newPso->pass.stencilParams.stencilFront.compareOp );
            depthStencilStateCi.front.compareMask = newPso->pass.stencilParams.readMask;
            depthStencilStateCi.front.writeMask = newPso->pass.stencilParams.writeMask;
            depthStencilStateCi.front.reference = 0;  // Dynamic state

            depthStencilStateCi.back.failOp =
                VulkanMappings::get( newPso->pass.stencilParams.stencilBack.stencilFailOp );
            depthStencilStateCi.back.passOp =
                VulkanMappings::get( newPso->pass.stencilParams.stencilBack.stencilPassOp );
            depthStencilStateCi.back.depthFailOp =
                VulkanMappings::get( newPso->pass.stencilParams.stencilBack.stencilDepthFailOp );
            depthStencilStateCi.back.compareOp =
                VulkanMappings::get( newPso->pass.stencilParams.stencilBack.compareOp );
            depthStencilStateCi.back.compareMask = newPso->pass.stencilParams.readMask;
            depthStencilStateCi.back.writeMask = newPso->pass.stencilParams.writeMask;
            depthStencilStateCi.back.reference = 0;  // Dynamic state
        }
        depthStencilStateCi.minDepthBounds = 0.0f;
        depthStencilStateCi.maxDepthBounds = 1.0f;

        VkPipelineColorBlendStateCreateInfo blendStateCi;
        makeVkStruct( blendStateCi, VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO );
        blendStateCi.logicOpEnable = false;
        uint8 mrtCount = 0;
        VkRenderPass renderPass = getVkRenderPass( newPso->pass, mrtCount );
        blendStateCi.attachmentCount = mrtCount;
        VkPipelineColorBlendAttachmentState blendStates[OGRE_MAX_MULTIPLE_RENDER_TARGETS];
        memset( blendStates, 0, sizeof( blendStates ) );

        if( newPso->blendblock->mSeparateBlend )
        {
            if( newPso->blendblock->mSourceBlendFactor == SBF_ONE &&
                newPso->blendblock->mDestBlendFactor == SBF_ZERO &&
                newPso->blendblock->mSourceBlendFactorAlpha == SBF_ONE &&
                newPso->blendblock->mDestBlendFactorAlpha == SBF_ZERO )
            {
                blendStates[0].blendEnable = false;
            }
            else
            {
                blendStates[0].blendEnable = true;
                blendStates[0].srcColorBlendFactor =
                    VulkanMappings::get( newPso->blendblock->mSourceBlendFactor );
                blendStates[0].dstColorBlendFactor =
                    VulkanMappings::get( newPso->blendblock->mDestBlendFactor );
                blendStates[0].colorBlendOp = VulkanMappings::get( newPso->blendblock->mBlendOperation );

                blendStates[0].srcAlphaBlendFactor =
                    VulkanMappings::get( newPso->blendblock->mSourceBlendFactorAlpha );
                blendStates[0].dstAlphaBlendFactor =
                    VulkanMappings::get( newPso->blendblock->mDestBlendFactorAlpha );
                blendStates[0].alphaBlendOp = blendStates[0].colorBlendOp;
            }
        }
        else
        {
            if( newPso->blendblock->mSourceBlendFactor == SBF_ONE &&
                newPso->blendblock->mDestBlendFactor == SBF_ZERO )
            {
                blendStates[0].blendEnable = false;
            }
            else
            {
                blendStates[0].blendEnable = true;
                blendStates[0].srcColorBlendFactor =
                    VulkanMappings::get( newPso->blendblock->mSourceBlendFactor );
                blendStates[0].dstColorBlendFactor =
                    VulkanMappings::get( newPso->blendblock->mDestBlendFactor );
                blendStates[0].colorBlendOp = VulkanMappings::get( newPso->blendblock->mBlendOperation );

                blendStates[0].srcAlphaBlendFactor = blendStates[0].srcColorBlendFactor;
                blendStates[0].dstAlphaBlendFactor = blendStates[0].dstColorBlendFactor;
                blendStates[0].alphaBlendOp = blendStates[0].colorBlendOp;
            }
        }
        blendStates[0].colorWriteMask = newPso->blendblock->mBlendChannelMask;

        for( int i = 1; i < mrtCount; ++i )
            blendStates[i] = blendStates[0];

        blendStateCi.pAttachments = blendStates;

        // Having viewport hardcoded into PSO is crazy.
        // It could skyrocket the number of required PSOs and heavily neutralize caches.
        const VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR,
                                                 VK_DYNAMIC_STATE_STENCIL_REFERENCE };

        VkPipelineDynamicStateCreateInfo dynamicStateCi;
        makeVkStruct( dynamicStateCi, VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO );
        dynamicStateCi.dynamicStateCount = sizeof( dynamicStates ) / sizeof( dynamicStates[0] );
        dynamicStateCi.pDynamicStates = dynamicStates;

        VkGraphicsPipelineCreateInfo pipeline;
        makeVkStruct( pipeline, VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO );

        pipeline.layout = layout;
        pipeline.stageCount = static_cast<uint32>( numShaderStages );
        pipeline.pStages = shaderStages;
        pipeline.pVertexInputState = &vertexFormatCi;
        pipeline.pInputAssemblyState = &inputAssemblyCi;
        pipeline.pTessellationState = &tessStateCi;
        pipeline.pViewportState = &viewportStateCi;
        pipeline.pRasterizationState = &rasterState;
        pipeline.pMultisampleState = &mssCi;
        pipeline.pDepthStencilState = &depthStencilStateCi;
        pipeline.pColorBlendState = &blendStateCi;
        pipeline.pDynamicState = &dynamicStateCi;
        pipeline.renderPass = renderPass;

#if OGRE_DEBUG_MODE >= OGRE_DEBUG_HIGH
        mValidationError = false;
#endif

        VkPipeline vulkanPso = 0;
        VkResult result = vkCreateGraphicsPipelines( mActiveDevice->mDevice, VK_NULL_HANDLE, 1u,
                                                     &pipeline, 0, &vulkanPso );
        checkVkResult( result, "vkCreateGraphicsPipelines" );

#if OGRE_DEBUG_MODE >= OGRE_DEBUG_MEDIUM
        if( mValidationError )
        {
            LogManager::getSingleton().logMessage( "Validation error:" );

            // This isn't legal C++ but it's a debug function not intended for production
            GpuProgramPtr *shaders = &newPso->vertexShader;
            for( size_t i = 0u; i < NumShaderTypes; ++i )
            {
                if( !shaders[i] )
                    continue;

                VulkanProgram *shader =
                    static_cast<VulkanProgram *>( shaders[i]->_getBindingDelegate() );

                String debugDump;
                shader->debugDump( debugDump );
                LogManager::getSingleton().logMessage( debugDump );
            }
        }
#endif

        VulkanHlmsPso *pso = new VulkanHlmsPso( vulkanPso, rootLayout );
        newPso->rsData = pso;
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_hlmsPipelineStateObjectDestroyed( HlmsPso *pso )
    {
        if( pso == mPso )
        {
            mUavRenderingDirty = true;
            mGlobalTable.setAllDirty();
            mTableDirty = true;
            mPso = 0;
        }

        OGRE_ASSERT_LOW( pso->rsData );
        VulkanHlmsPso *vulkanPso = static_cast<VulkanHlmsPso *>( pso->rsData );
        delayed_vkDestroyPipeline( mVaoManager, mActiveDevice->mDevice, vulkanPso->pso, 0 );
        delete vulkanPso;
        pso->rsData = 0;
    }

    void VulkanRenderSystem::_hlmsMacroblockCreated( HlmsMacroblock *newBlock )
    {
        Log *defaultLog = LogManager::getSingleton().getDefaultLog();
        if( defaultLog )
        {
            defaultLog->logMessage( String( " _hlmsMacroblockCreated " ) );
        }
    }

    void VulkanRenderSystem::_hlmsMacroblockDestroyed( HlmsMacroblock *block ) {}

    void VulkanRenderSystem::_hlmsBlendblockCreated( HlmsBlendblock *newBlock )
    {
        Log *defaultLog = LogManager::getSingleton().getDefaultLog();
        if( defaultLog )
        {
            defaultLog->logMessage( String( " _hlmsBlendblockCreated " ) );
        }
    }

    void VulkanRenderSystem::_hlmsBlendblockDestroyed( HlmsBlendblock *block ) {}

    void VulkanRenderSystem::_hlmsSamplerblockCreated( HlmsSamplerblock *newBlock )
    {
        Log *defaultLog = LogManager::getSingleton().getDefaultLog();
        if( defaultLog )
        {
            defaultLog->logMessage( String( " _hlmsSamplerblockCreated " ) );
        }

        VkSamplerCreateInfo samplerDescriptor;
        makeVkStruct( samplerDescriptor, VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO );
        samplerDescriptor.minFilter = VulkanMappings::get( newBlock->mMinFilter );
        samplerDescriptor.magFilter = VulkanMappings::get( newBlock->mMagFilter );
        samplerDescriptor.mipmapMode = VulkanMappings::getMipFilter( newBlock->mMipFilter );
        samplerDescriptor.mipLodBias = newBlock->mMipLodBias;
        float maxAllowedAnisotropy = mActiveDevice->mDeviceProperties.limits.maxSamplerAnisotropy;
        samplerDescriptor.maxAnisotropy = newBlock->mMaxAnisotropy > maxAllowedAnisotropy
                                              ? maxAllowedAnisotropy
                                              : newBlock->mMaxAnisotropy;
        samplerDescriptor.anisotropyEnable =
            ( mActiveDevice->mDeviceFeatures.samplerAnisotropy == VK_TRUE ) &&
            ( samplerDescriptor.maxAnisotropy > 1.0f ? VK_TRUE : VK_FALSE );
        samplerDescriptor.addressModeU = VulkanMappings::get( newBlock->mU );
        samplerDescriptor.addressModeV = VulkanMappings::get( newBlock->mV );
        samplerDescriptor.addressModeW = VulkanMappings::get( newBlock->mW );
        samplerDescriptor.unnormalizedCoordinates = VK_FALSE;
        samplerDescriptor.minLod = newBlock->mMinLod;
        samplerDescriptor.maxLod = newBlock->mMaxLod;

        if( newBlock->mCompareFunction != NUM_COMPARE_FUNCTIONS )
        {
            samplerDescriptor.compareEnable = VK_TRUE;
            samplerDescriptor.compareOp = VulkanMappings::get( newBlock->mCompareFunction );
        }

        VkSampler textureSampler;
        VkResult result =
            vkCreateSampler( mActiveDevice->mDevice, &samplerDescriptor, 0, &textureSampler );
        checkVkResult( result, "vkCreateSampler" );

        newBlock->mRsData = textureSampler;
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_hlmsSamplerblockDestroyed( HlmsSamplerblock *block )
    {
        assert( block->mRsData );
        VkSampler textureSampler = static_cast<VkSampler>( block->mRsData );
        delayed_vkDestroySampler( mVaoManager, mActiveDevice->mDevice, textureSampler, 0 );
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_descriptorSetTextureCreated( DescriptorSetTexture *newSet )
    {
        VulkanDescriptorSetTexture *vulkanSet = new VulkanDescriptorSetTexture( *newSet );
        newSet->mRsData = vulkanSet;
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_descriptorSetTextureDestroyed( DescriptorSetTexture *set )
    {
        OGRE_ASSERT_LOW( set->mRsData );
        VulkanDescriptorSetTexture *vulkanSet =
            static_cast<VulkanDescriptorSetTexture *>( set->mRsData );
        delete vulkanSet;
        set->mRsData = 0;
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_descriptorSetTexture2Created( DescriptorSetTexture2 *newSet )
    {
        VulkanDescriptorSetTexture2 *vulkanSet = new VulkanDescriptorSetTexture2( *newSet );
        newSet->mRsData = vulkanSet;
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_descriptorSetTexture2Destroyed( DescriptorSetTexture2 *set )
    {
        OGRE_ASSERT_LOW( set->mRsData );
        VulkanDescriptorSetTexture2 *vulkanSet =
            static_cast<VulkanDescriptorSetTexture2 *>( set->mRsData );
        vulkanSet->destroy( mVaoManager, mActiveDevice->mDevice, *set );
        delete vulkanSet;
        set->mRsData = 0;
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_descriptorSetSamplerCreated( DescriptorSetSampler *newSet )
    {
        VulkanDescriptorSetSampler *vulkanSet = new VulkanDescriptorSetSampler( *newSet, mDummySampler );
        newSet->mRsData = vulkanSet;
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_descriptorSetSamplerDestroyed( DescriptorSetSampler *set )
    {
        OGRE_ASSERT_LOW( set->mRsData );
        VulkanDescriptorSetSampler *vulkanSet =
            static_cast<VulkanDescriptorSetSampler *>( set->mRsData );
        delete vulkanSet;
        set->mRsData = 0;
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_descriptorSetUavCreated( DescriptorSetUav *newSet )
    {
        VulkanDescriptorSetUav *vulkanSet = new VulkanDescriptorSetUav( *newSet );
        newSet->mRsData = vulkanSet;
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::_descriptorSetUavDestroyed( DescriptorSetUav *set )
    {
        OGRE_ASSERT_LOW( set->mRsData );

        VulkanDescriptorSetUav *vulkanSet = reinterpret_cast<VulkanDescriptorSetUav *>( set->mRsData );
        vulkanSet->destroy( *set );
        delete vulkanSet;

        set->mRsData = 0;
    }
    //-------------------------------------------------------------------------
    void VulkanRenderSystem::setStencilBufferParams( uint32 refValue,
                                                     const StencilParams &stencilParams )
    {
        RenderSystem::setStencilBufferParams( refValue, stencilParams );

        // There are two main cases:
        // 1. The active render encoder is valid and will be subsequently used for drawing.
        //      We need to set the stencil reference value on this encoder. We do this below.
        // 2. The active render is invalid or is about to go away.
        //      In this case, we need to set the stencil reference value on the new encoder when it is
        //      created (In this case, the setStencilReferenceValue below in this wasted,
        //      but it is inexpensive).

        // Save this info so we can transfer it into a new encoder if necessary
        mStencilEnabled = stencilParams.enabled;
        if( mStencilEnabled )
        {
            mStencilRefValue = refValue;

            if( mActiveDevice->mGraphicsQueue.getEncoderState() == VulkanQueue::EncoderGraphicsOpen )
            {
                vkCmdSetStencilReference( mActiveDevice->mGraphicsQueue.mCurrentCmdBuffer,
                                          VK_STENCIL_FACE_FRONT_AND_BACK, mStencilRefValue );
            }
        }
    }
    //-------------------------------------------------------------------------
    SampleDescription VulkanRenderSystem::validateSampleDescription( const SampleDescription &sampleDesc,
                                                                     PixelFormatGpu format )
    {
        SampleDescription retVal(
            getMaxUsableSampleCount( mDevice->mDeviceProperties, sampleDesc.getMaxSamples() ),
            sampleDesc.getMsaaPattern() );
        return retVal;
    }
    //-------------------------------------------------------------------------
    bool VulkanRenderSystem::isSameLayout( ResourceLayout::Layout a, ResourceLayout::Layout b,
                                           const TextureGpu *texture, bool bIsDebugCheck ) const
    {
        return VulkanMappings::get( a, texture ) == VulkanMappings::get( b, texture );
    }
}  // namespace Ogre
