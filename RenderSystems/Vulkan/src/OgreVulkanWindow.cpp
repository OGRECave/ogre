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
#include "OgreVulkanWindow.h"

#include "OgreVulkanDevice.h"
#include "OgreVulkanMappings.h"
#include "OgreVulkanRenderSystem.h"
#include "OgreVulkanTextureGpuWindow.h"
#include "OgreVulkanUtils.h"

#include "Vao/OgreVulkanVaoManager.h"

#include "OgreException.h"
#include "OgrePixelFormatGpuUtils.h"
#include "OgreVulkanTextureGpuManager.h"

#include "vulkan/vulkan_core.h"

#define TODO_handleSeparatePresentQueue

namespace Ogre
{
    VulkanWindow::VulkanWindow( const String &title, uint32 width, uint32 height, bool fullscreenMode ) :
        Window( title, width, height, fullscreenMode ),
        mLowestLatencyVSync( false ),
        mHwGamma( false ),
        mClosed( false ),
        mDevice( 0 ),
        mSurfaceKHR( 0 ),
        mSwapchain( 0 ),
        mSwapchainSemaphore( 0 ),
        mSwapchainStatus( SwapchainReleased ),
        mRebuildingSwapchain( false ),
        mSuboptimal( false )
    {
        mFocused = true;
    }
    //-------------------------------------------------------------------------
    VulkanWindow::~VulkanWindow() {}
    //-------------------------------------------------------------------------
    void VulkanWindow::parseSharedParams( const NameValuePairList *miscParams )
    {
        NameValuePairList::const_iterator opt;
        NameValuePairList::const_iterator end = miscParams->end();

        opt = miscParams->find( "title" );
        if( opt != end )
            mTitle = opt->second;
        opt = miscParams->find( "vsync" );
        if( opt != end )
            mVSync = StringConverter::parseBool( opt->second );
        opt = miscParams->find( "vsyncInterval" );
        if( opt != end )
            mVSyncInterval = StringConverter::parseUnsignedInt( opt->second );
        opt = miscParams->find( "FSAA" );
        if( opt != end )
            mRequestedSampleDescription.parseString( opt->second );
        opt = miscParams->find( "gamma" );
        if( opt != end )
            mHwGamma = StringConverter::parseBool( opt->second );
        opt = miscParams->find( "vsync_method" );
        if( opt != end )
            mLowestLatencyVSync = opt->second == "Lowest Latency";
    }
    //-------------------------------------------------------------------------
    PixelFormatGpu VulkanWindow::chooseSurfaceFormat( bool hwGamma )
    {
        uint32 numFormats = 0u;
        VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR( mDevice->mPhysicalDevice, mSurfaceKHR,
                                                                &numFormats, 0 );
        checkVkResult( result, "vkGetPhysicalDeviceSurfaceFormatsKHR" );
        if( numFormats == 0 )
        {
            OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                         "vkGetPhysicalDeviceSurfaceFormatsKHR returned 0 formats!",
                         "VulkanWindow::chooseSurfaceFormat" );
        }

        FastArray<VkSurfaceFormatKHR> formats;
        formats.resize( numFormats );
        result = vkGetPhysicalDeviceSurfaceFormatsKHR( mDevice->mPhysicalDevice, mSurfaceKHR,
                                                       &numFormats, formats.begin() );
        checkVkResult( result, "vkGetPhysicalDeviceSurfaceFormatsKHR" );

        PixelFormatGpu pixelFormat = PFG_UNKNOWN;
        for( size_t i = 0; i < numFormats && pixelFormat == PFG_UNKNOWN; ++i )
        {
            switch( formats[i].format )
            {
            case VK_FORMAT_R8G8B8A8_SRGB:
                if( hwGamma )
                    pixelFormat = PFG_RGBA8_UNORM_SRGB;
                break;
            case VK_FORMAT_B8G8R8A8_SRGB:
                if( hwGamma )
                    pixelFormat = PFG_BGRA8_UNORM_SRGB;
                break;
            case VK_FORMAT_R8G8B8A8_UNORM:
                if( !hwGamma )
                    pixelFormat = PFG_RGBA8_UNORM;
                break;
            case VK_FORMAT_B8G8R8A8_UNORM:
                if( !hwGamma )
                    pixelFormat = PFG_BGRA8_UNORM;
                break;
            default:
                continue;
            }
        }

        if( pixelFormat == PFG_UNKNOWN )
        {
            OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR, "Could not find a suitable Surface format",
                         "VulkanWindow::chooseSurfaceFormat" );
        }

        return pixelFormat;
    }
    //-------------------------------------------------------------------------
    void VulkanWindow::createSwapchain( void )
    {
        mSuboptimal = false;

        VkSurfaceCapabilitiesKHR surfaceCaps;
        VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR( mDevice->mPhysicalDevice,
                                                                     mSurfaceKHR, &surfaceCaps );
        checkVkResult( result, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR" );

        // Swapchain may be smaller/bigger than requested
        setFinalResolution( Math::Clamp( getWidth(), surfaceCaps.minImageExtent.width,
                                         surfaceCaps.maxImageExtent.width ),
                            Math::Clamp( getHeight(), surfaceCaps.minImageExtent.height,
                                         surfaceCaps.maxImageExtent.height ) );

        VkBool32 supported;
        result = vkGetPhysicalDeviceSurfaceSupportKHR(
            mDevice->mPhysicalDevice, mDevice->mGraphicsQueue.mFamilyIdx, mSurfaceKHR, &supported );
        checkVkResult( result, "vkGetPhysicalDeviceSurfaceSupportKHR" );

        if( !supported )
        {
            OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                         "vkGetPhysicalDeviceSurfaceSupportKHR says our KHR Surface is unsupported!",
                         "VulkanWindow::createSwapchain" );
        }

        uint32 numPresentModes = 0u;
        vkGetPhysicalDeviceSurfacePresentModesKHR( mDevice->mPhysicalDevice, mSurfaceKHR,
                                                   &numPresentModes, 0 );
        FastArray<VkPresentModeKHR> presentModes;
        presentModes.resize( numPresentModes );
        vkGetPhysicalDeviceSurfacePresentModesKHR( mDevice->mPhysicalDevice, mSurfaceKHR,
                                                   &numPresentModes, presentModes.begin() );

        // targetPresentModes[0] is the target, targetPresentModes[1] is the fallback
        bool presentModesFound[2] = { false, false };
        VkPresentModeKHR targetPresentModes[2];
        targetPresentModes[0] = VK_PRESENT_MODE_IMMEDIATE_KHR;
        targetPresentModes[1] = VK_PRESENT_MODE_FIFO_KHR;
        if( mVSync )
        {
            targetPresentModes[0] =
                mLowestLatencyVSync ? VK_PRESENT_MODE_MAILBOX_KHR : VK_PRESENT_MODE_FIFO_KHR;
            targetPresentModes[1] =
                mLowestLatencyVSync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_MAILBOX_KHR;
        }

        // FIFO is guaranteed to be present
        VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
        for( size_t i = 0u; i < numPresentModes; ++i )
        {
            if( presentModes[i] == targetPresentModes[0] )
                presentModesFound[0] = true;
            if( presentModes[i] == targetPresentModes[1] )
                presentModesFound[1] = true;
        }

        const String c_presentModeStrs[] = { "IMMEDIATE_KHR",
                                             "MAILBOX_KHR",
                                             "FIFO_KHR",
                                             "FIFO_RELAXED_KHR",
                                             "SHARED_DEMAND_REFRESH_KHR",
                                             "SHARED_CONTINUOUS_REFRESH_KHR" };

        for( size_t i = 0u; i < 2u; ++i )
        {
            LogManager::getSingleton().logMessage( "Trying presentMode = " +
                                                   c_presentModeStrs[targetPresentModes[i]] );
            if( presentModesFound[i] )
            {
                presentMode = targetPresentModes[i];
                break;
            }
            else
            {
                LogManager::getSingleton().logMessage( "PresentMode not available" );
            }
        }

        LogManager::getSingleton().logMessage( "Chosen presentMode = " +
                                               c_presentModeStrs[presentMode] );

        //-----------------------------
        // Create swapchain
        //-----------------------------

        VaoManager *vaoManager = mDevice->mVaoManager;

        // We may sometimes have to wait on the driver to complete internal operations
        // before we can acquire another image to render to.
        // Therefore it is recommended to request at least one more image than the minimum.
        // https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain
        uint32 minImageCount =
            std::max<uint32>( surfaceCaps.minImageCount, vaoManager->getDynamicBufferMultiplier() ) + 1;
        if( surfaceCaps.maxImageCount != 0u )
            minImageCount = std::min<uint32>( minImageCount, surfaceCaps.maxImageCount );

        VkSwapchainCreateInfoKHR swapchainCreateInfo;
        memset( &swapchainCreateInfo, 0, sizeof( swapchainCreateInfo ) );
        swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainCreateInfo.surface = mSurfaceKHR;
        swapchainCreateInfo.minImageCount = minImageCount;
        swapchainCreateInfo.imageFormat = VulkanMappings::get( mTexture->getPixelFormat() );
        swapchainCreateInfo.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
        swapchainCreateInfo.imageExtent.width = getWidth();
        swapchainCreateInfo.imageExtent.height = getHeight();
        swapchainCreateInfo.imageArrayLayers = 1u;
        swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.queueFamilyIndexCount = 0u;
        swapchainCreateInfo.pQueueFamilyIndices = 0;
        // Find a supported composite alpha mode - one of these is guaranteed to be set
        swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        VkCompositeAlphaFlagBitsKHR compositeAlphaFlags[4] = {
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
        };
        for( size_t i = 0; i < sizeof( compositeAlphaFlags ) / sizeof( compositeAlphaFlags[0] ); ++i )
        {
            if( surfaceCaps.supportedCompositeAlpha & compositeAlphaFlags[i] )
            {
                swapchainCreateInfo.compositeAlpha = compositeAlphaFlags[i];
                break;
            }
        }
#if OGRE_NO_VIEWPORT_ORIENTATIONMODE == 0
        if( surfaceCaps.currentTransform <= VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR )
        {
            // We will manually rotate by adapting our projection matrices (fastest)
            // See https://arm-software.github.io/vulkan_best_practice_for_mobile_developers/samples/
            // performance/surface_rotation/surface_rotation_tutorial.html
            swapchainCreateInfo.preTransform = surfaceCaps.currentTransform;

            OrientationMode orientationMode = OR_DEGREE_0;
            switch( surfaceCaps.currentTransform )
            {
            case VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR:
                orientationMode = OR_DEGREE_0;
                break;
            case VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR:
                std::swap( swapchainCreateInfo.imageExtent.width,
                           swapchainCreateInfo.imageExtent.height );
                orientationMode = OR_DEGREE_270;
                break;
            case VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR:
                orientationMode = OR_DEGREE_180;
                break;
            case VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR:
                std::swap( swapchainCreateInfo.imageExtent.width,
                           swapchainCreateInfo.imageExtent.height );
                orientationMode = OR_DEGREE_90;
                break;
            default:
                break;
            }

            mTexture->setOrientationMode( orientationMode );
            if( mDepthBuffer )
                mDepthBuffer->setOrientationMode( orientationMode );
            if( mStencilBuffer )
                mStencilBuffer->setOrientationMode( orientationMode );
        }
        else
        {
            // We do not support mirroring. Force Vulkan to do the flipping for us
            // (could be slower if there is no dedicated HW for it on the phone)
            swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
            mTexture->setOrientationMode( OR_DEGREE_0 );
            if( mDepthBuffer )
                mDepthBuffer->setOrientationMode( OR_DEGREE_0 );
            if( mStencilBuffer )
                mStencilBuffer->setOrientationMode( OR_DEGREE_0 );
        }
#else
        swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
#endif
        swapchainCreateInfo.presentMode = presentMode;

        LogManager::getSingleton().logMessage(
            "surfaceCaps.currentTransform = " +
            StringConverter::toString( surfaceCaps.currentTransform ) );

        //-----------------------------
        // Create swapchain images
        //-----------------------------

        result = vkCreateSwapchainKHR( mDevice->mDevice, &swapchainCreateInfo, 0, &mSwapchain );
        checkVkResult( result, "vkCreateSwapchainKHR" );

        uint32 numSwapchainImages = 0u;
        result = vkGetSwapchainImagesKHR( mDevice->mDevice, mSwapchain, &numSwapchainImages, NULL );
        checkVkResult( result, "vkGetSwapchainImagesKHR" );

        OGRE_ASSERT_LOW( numSwapchainImages > 0u );

        mSwapchainImages.resize( numSwapchainImages );
        result = vkGetSwapchainImagesKHR( mDevice->mDevice, mSwapchain, &numSwapchainImages,
                                          mSwapchainImages.begin() );
        checkVkResult( result, "vkGetSwapchainImagesKHR" );

        acquireNextSwapchain();

        if( mDepthBuffer )
            mDepthBuffer->_transitionTo( GpuResidency::Resident, (uint8 *)0 );
        if( mStencilBuffer && mStencilBuffer != mDepthBuffer )
            mStencilBuffer->_transitionTo( GpuResidency::Resident, (uint8 *)0 );

        mDevice->mRenderSystem->notifySwapchainCreated( this );
    }
    //-------------------------------------------------------------------------
    void VulkanWindow::destroySwapchain( void )
    {
        mDevice->mRenderSystem->notifySwapchainDestroyed( this );

        if( mSwapchainSemaphore )
        {
            mDevice->mVaoManager->notifySemaphoreUnused( mSwapchainSemaphore );
            mSwapchainSemaphore = 0;
        }

        if( mSwapchain )
        {
            vkDestroySwapchainKHR( mDevice->mDevice, mSwapchain, 0 );
            mSwapchain = 0;
        }

        mSwapchainStatus = SwapchainReleased;
    }
    //-------------------------------------------------------------------------
    void VulkanWindow::acquireNextSwapchain( void )
    {
        OGRE_ASSERT_LOW( mSwapchainStatus == SwapchainReleased );
        OGRE_ASSERT_MEDIUM( !mSwapchainSemaphore );

        VulkanVaoManager *vaoManager = mDevice->mVaoManager;

        mSwapchainSemaphore = vaoManager->getAvailableSempaphore();

        uint32 swapchainIdx = 0u;
        VkResult result = vkAcquireNextImageKHR( mDevice->mDevice, mSwapchain, UINT64_MAX,
                                                 mSwapchainSemaphore, VK_NULL_HANDLE, &swapchainIdx );
        if( result != VK_SUCCESS || mSuboptimal )
        {
            // Recreate the swapchain if it's no longer compatible with the surface (OUT_OF_DATE)
            // or no longer optimal for presentation (SUBOPTIMAL)
            if( ( mSuboptimal ||
                  ( result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ) ) &&
                !mRebuildingSwapchain )
            {
                mRebuildingSwapchain = true;
                windowMovedOrResized();
                mRebuildingSwapchain = false;
            }
            else
            {
                LogManager::getSingleton().logMessage(
                    "[VulkanWindow::acquireNextSwapchain] vkAcquireNextImageKHR failed VkResult = " +
                    vkResultToString( result ) );
            }
        }
        else
        {
            mSwapchainStatus = SwapchainAcquired;

            VulkanTextureGpuWindow *vulkanTexture = static_cast<VulkanTextureGpuWindow *>( mTexture );

            vulkanTexture->_setCurrentSwapchain( mSwapchainImages[swapchainIdx], swapchainIdx );
        }
    }
    //-------------------------------------------------------------------------
    void VulkanWindow::destroy( void )
    {
        destroySwapchain();
        if( mSurfaceKHR )
        {
            vkDestroySurfaceKHR( mDevice->mInstance, mSurfaceKHR, 0 );
            mSurfaceKHR = 0;
        }
    }
    //-------------------------------------------------------------------------
    void VulkanWindow::_setDevice( VulkanDevice *device )
    {
        OGRE_ASSERT_LOW( !mDevice );
        mDevice = device;
    }
    //-------------------------------------------------------------------------
    void VulkanWindow::_initialize( TextureGpuManager *textureGpuManager )
    {
        OGRE_EXCEPT( Exception::ERR_INVALID_CALL,
                     "Call _initialize( TextureGpuManager*, const NameValuePairList * ) instead",
                     "VulkanWindow::_initialize" );
    }
    //-------------------------------------------------------------------------
    VkSemaphore VulkanWindow::getImageAcquiredSemaphore( void )
    {
        OGRE_ASSERT_LOW( mSwapchainStatus != SwapchainReleased );
        // It's weird that mSwapchainStatus would be in SwapchainPendingSwap here,
        // however it's not invalid and won't result in race conditions (e.g. swapBuffers was called,
        // then more work was added, but commitAndNextCommandBuffer hasn't yet been called).
        //
        // We assert because it may signify that something weird is going on: if user called
        // swapBuffers(), then he may expect to present everything that has been rendered
        // up until now, without including what came after the swapBuffers call.
        OGRE_ASSERT_MEDIUM( mSwapchainStatus != SwapchainPendingSwap );

        VkSemaphore retVal = 0;
        if( mSwapchainStatus == SwapchainAcquired )
        {
            mSwapchainStatus = SwapchainUsedInRendering;
            retVal = mSwapchainSemaphore;
        }
        return retVal;
    }
    //-------------------------------------------------------------------------
    bool VulkanWindow::isClosed( void ) const { return mClosed; }
    //-------------------------------------------------------------------------
    void VulkanWindow::setVSync( bool vSync, uint32 vSyncInterval )
    {
        // mVSyncInterval is ignored at least for now
        // (we'd need VK_GOOGLE_display_timing or VK_MESA_present_period)
        mVSyncInterval = vSyncInterval & 0x7FFFFFFFu;
        mLowestLatencyVSync = vSyncInterval & 0x80000000u;

        if( mVSync == vSync )
            return;

        mVSync = vSync;

        destroySwapchain();

        if( mDepthBuffer )
            mDepthBuffer->_transitionTo( GpuResidency::OnStorage, (uint8 *)0 );
        if( mStencilBuffer && mStencilBuffer != mDepthBuffer )
            mStencilBuffer->_transitionTo( GpuResidency::OnStorage, (uint8 *)0 );

        createSwapchain();
    }
    //-------------------------------------------------------------------------
    void VulkanWindow::swapBuffers( void )
    {
        if( mSwapchainStatus == SwapchainAcquired || mSwapchainStatus == SwapchainPendingSwap )
        {
            // Ogre never rendered to this window. There's nothing to present.
            // Pass the currently acquired swapchain onto the next frame.
            // Or alternatively, swapBuffers was called twice in a row
            return;
        }

        OGRE_ASSERT_LOW( mSwapchainStatus == SwapchainUsedInRendering );

        OGRE_ASSERT_MEDIUM( mSwapchainSemaphore );
        VulkanVaoManager *vaoManager = mDevice->mVaoManager;
        vaoManager->notifyWaitSemaphoreSubmitted( mSwapchainSemaphore );
        mSwapchainSemaphore = 0;

        mDevice->mGraphicsQueue.mWindowsPendingSwap.push_back( this );
        mSwapchainStatus = SwapchainPendingSwap;
    }
    //-------------------------------------------------------------------------
    void VulkanWindow::_swapBuffers( VkSemaphore queueFinishSemaphore )
    {
        OGRE_ASSERT_LOW( mSwapchainStatus == SwapchainPendingSwap );

        VulkanTextureGpuWindow *vulkanTexture = static_cast<VulkanTextureGpuWindow *>( mTexture );
        const uint32 currentSwapchainIdx = vulkanTexture->getCurrentSwapchainIdx();

        TODO_handleSeparatePresentQueue;
        /*
        if (demo->separate_present_queue) {
            // If we are using separate queues, change image ownership to the
            // present queue before presenting, waiting for the draw complete
            // semaphore and signalling the ownership released semaphore when finished
            VkFence nullFence = VK_NULL_HANDLE;
            pipe_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            submit_info.waitSemaphoreCount = 1;
            submit_info.pWaitSemaphores = &demo->draw_complete_semaphores[demo->frame_index];
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers =
                &demo->swapchain_image_resources[demo->current_buffer].graphics_to_present_cmd;
            submit_info.signalSemaphoreCount = 1;
            submit_info.pSignalSemaphores = &demo->image_ownership_semaphores[demo->frame_index];
            err = vkQueueSubmit(demo->present_queue, 1, &submit_info, nullFence);
            assert(!err);
        }*/

        VkPresentInfoKHR present;
        makeVkStruct( present, VK_STRUCTURE_TYPE_PRESENT_INFO_KHR );
        present.swapchainCount = 1u;
        present.pSwapchains = &mSwapchain;
        present.pImageIndices = &currentSwapchainIdx;
        if( queueFinishSemaphore )
        {
            present.waitSemaphoreCount = 1u;
            present.pWaitSemaphores = &queueFinishSemaphore;
        }

        VkResult result = vkQueuePresentKHR( mDevice->mPresentQueue, &present );

        if( result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR && result != VK_ERROR_OUT_OF_DATE_KHR )
        {
            LogManager::getSingleton().logMessage(
                "[VulkanWindow::swapBuffers] vkQueuePresentKHR: error presenting VkResult = " +
                vkResultToString( result ) );
        }

        if( result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR )
            mSuboptimal = true;

        mSwapchainStatus = SwapchainReleased;
    }
}  // namespace Ogre
