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

#include "OgreException.h"
#include "OgrePixelFormat.h"
#include "OgreVulkanTextureGpuManager.h"
#include "OgreHardwarePixelBuffer.h"
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
#include <android/native_window.h>
#elif OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include "OgreDepthBuffer.h"

#define TODO_handleSeparatePresentQueue

namespace Ogre
{
    VulkanWindow::VulkanWindow( const String &title, uint32 width, uint32 height, bool fullscreenMode ) :
        mLowestLatencyVSync( false ),
        mHwGamma( false ),
        mDevice( 0 ),
        mTexture( 0 ),
        mDepthTexture( 0 ),
        mSurfaceKHR( 0 ),
        mSwapchain( 0 ),
        mCurrentSemaphoreIndex( 0 ),
        mSwapchainStatus( SwapchainReleased ),
        mSurfaceTransform( VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR )
    {
        mActive = true;
        mName = title;
    }
    //-------------------------------------------------------------------------
    VulkanWindow::~VulkanWindow()
    {
        destroy();
    }
    //-------------------------------------------------------------------------
    PixelFormat VulkanWindow::chooseSurfaceFormat( bool hwGamma )
    {
        uint32 numFormats = 0u;
        OGRE_VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(mDevice->mPhysicalDevice, mSurfaceKHR, &numFormats, 0));
        OgreAssert(numFormats > 0, "No surface formats found");

        FastArray<VkSurfaceFormatKHR> formats(numFormats);
        OGRE_VK_CHECK(
            vkGetPhysicalDeviceSurfaceFormatsKHR(mDevice->mPhysicalDevice, mSurfaceKHR, &numFormats, formats.data()));

        PixelFormatGpu pixelFormat = PF_UNKNOWN;
        for( size_t i = 0; i < numFormats && pixelFormat == PF_UNKNOWN; ++i )
        {
            switch( formats[i].format )
            {
            case VK_FORMAT_R8G8B8A8_SRGB:
                if( hwGamma )
                    pixelFormat = PF_A8B8G8R8;//_SRGB;
                break;
            case VK_FORMAT_B8G8R8A8_SRGB:
                if( hwGamma )
                    pixelFormat = PF_A8R8G8B8;//_SRGB;
                break;
            case VK_FORMAT_R8G8B8A8_UNORM:
                if( !hwGamma )
                    pixelFormat = PF_A8B8G8R8;
                break;
            case VK_FORMAT_B8G8R8A8_UNORM:
                if( !hwGamma )
                    pixelFormat = PF_A8R8G8B8;
                break;
            default:
                continue;
            }
        }

        OgreAssert(pixelFormat != PF_UNKNOWN, "No suitable surface format found");
        return pixelFormat;
    }
    //-------------------------------------------------------------------------
    void VulkanWindow::createSwapchain( void )
    {
        mTexture->setWidth(mWidth);
        mTexture->setHeight(mHeight);
        mTexture->createInternalResources();

        mDepthTexture->setWidth(mWidth);
        mDepthTexture->setHeight(mHeight);
        mDepthTexture->setNumMipmaps(0);
        mDepthTexture->createInternalResources();

        VkSurfaceCapabilitiesKHR surfaceCaps;
        OGRE_VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mDevice->mPhysicalDevice, mSurfaceKHR, &surfaceCaps));

        // Swapchain may be smaller/bigger than requested
        mWidth = Math::Clamp(mWidth, surfaceCaps.minImageExtent.width, surfaceCaps.maxImageExtent.width);
        mHeight =
            Math::Clamp(mHeight, surfaceCaps.minImageExtent.height, surfaceCaps.maxImageExtent.height);

        VkBool32 supported;
        OGRE_VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(mDevice->mPhysicalDevice, mDevice->mGraphicsQueue.mFamilyIdx,
                                                           mSurfaceKHR, &supported));
        OgreAssert(supported, "KHR Surface is unsupported");

        uint32 numPresentModes = 0u;
        vkGetPhysicalDeviceSurfacePresentModesKHR(mDevice->mPhysicalDevice, mSurfaceKHR, &numPresentModes, 0);
        std::vector<VkPresentModeKHR> presentModes(numPresentModes);
        vkGetPhysicalDeviceSurfacePresentModesKHR(mDevice->mPhysicalDevice, mSurfaceKHR, &numPresentModes,
                                                  presentModes.data());

        // FIFO is guaranteed to be present
        VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

        if (!mVSync &&
            std::find(presentModes.begin(), presentModes.end(), VK_PRESENT_MODE_IMMEDIATE_KHR) != presentModes.end())
        {
            presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
        }

        const char* c_presentModeStrs[] = {"IMMEDIATE_KHR",
                                           "MAILBOX_KHR",
                                           "FIFO_KHR",
                                           "FIFO_RELAXED_KHR",
                                           "SHARED_DEMAND_REFRESH_KHR",
                                           "SHARED_CONTINUOUS_REFRESH_KHR"};

        LogManager::getSingleton().stream() << "[VulkanWindow] presentMode = " << c_presentModeStrs[presentMode];

        //-----------------------------
        // Create swapchain
        //-----------------------------

        // try to get triple buffering by default
        // https://github.com/KhronosGroup/Vulkan-Samples/blob/master/samples/performance/swapchain_images/swapchain_images_tutorial.md
        auto minImageCount = surfaceCaps.minImageCount + 1;
        if (surfaceCaps.maxImageCount != 0u)
            minImageCount = std::min(minImageCount, surfaceCaps.maxImageCount);

        VkSwapchainCreateInfoKHR swapchainCreateInfo = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
        swapchainCreateInfo.surface = mSurfaceKHR;
        swapchainCreateInfo.minImageCount = minImageCount;
        swapchainCreateInfo.imageFormat = VulkanMappings::get(mTexture->getFormat());
        swapchainCreateInfo.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
        swapchainCreateInfo.imageExtent.width = getWidth();
        swapchainCreateInfo.imageExtent.height = getHeight();
        swapchainCreateInfo.imageArrayLayers = 1u;
        swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
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
        for (auto flag : compositeAlphaFlags)
        {
            if (surfaceCaps.supportedCompositeAlpha & flag)
            {
                swapchainCreateInfo.compositeAlpha = flag;
                break;
            }
        }

#if 0
        // https://developer.android.com/games/optimize/vulkan-prerotation
        mSurfaceTransform = surfaceCaps.currentTransform;
        if (mSurfaceTransform & (VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR | VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR))
        {
            std::swap(swapchainCreateInfo.imageExtent.width, swapchainCreateInfo.imageExtent.height);
            std::swap(mWidth, mHeight);
        }
        swapchainCreateInfo.preTransform = mSurfaceTransform;
        LogManager::getSingleton().stream() << "[VulkanWindow] SurfaceTransform = " << mSurfaceTransform;
#endif
        swapchainCreateInfo.preTransform = mSurfaceTransform;
        swapchainCreateInfo.presentMode = presentMode;

        //-----------------------------
        // Create swapchain images
        //-----------------------------
        OGRE_VK_CHECK(vkCreateSwapchainKHR(mDevice->mDevice, &swapchainCreateInfo, 0, &mSwapchain));

        uint32 numSwapchainImages = 0u;
        OGRE_VK_CHECK(vkGetSwapchainImagesKHR(mDevice->mDevice, mSwapchain, &numSwapchainImages, NULL));

        OGRE_ASSERT_LOW( numSwapchainImages > 0u );

        mSwapchainImages.resize( numSwapchainImages );
        OGRE_VK_CHECK(
            vkGetSwapchainImagesKHR(mDevice->mDevice, mSwapchain, &numSwapchainImages, mSwapchainImages.data()));

        mSwapchainImageViews.resize(numSwapchainImages);
        mImageReadySemaphores.resize(numSwapchainImages);
        mRenderFinishedSemaphores.resize(numSwapchainImages);
        mImageFences.resize(numSwapchainImages);

        VkSemaphoreCreateInfo semaphoreCreateInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        for (uint32 i = 0; i < numSwapchainImages; i++)
        {
            mSwapchainImageViews[i] = mTexture->_createView(0, 1, 0, 1u, mSwapchainImages[i]);
            OGRE_VK_CHECK(vkCreateSemaphore(mDevice->mDevice, &semaphoreCreateInfo, 0, &mImageReadySemaphores[i]));
            OGRE_VK_CHECK(vkCreateSemaphore(mDevice->mDevice, &semaphoreCreateInfo, 0, &mRenderFinishedSemaphores[i]));
        }

        acquireNextImage();

        mRenderPassDescriptor->mColour[0] = mTexture;
        mRenderPassDescriptor->mDepth = mDepthTexture;
        mRenderPassDescriptor->mNumColourEntries = 1;
        mRenderPassDescriptor->entriesModified(true);
    }
    //-------------------------------------------------------------------------
    void VulkanWindow::destroySwapchain( void )
    {
        for(auto imf : mImageFences)
        {
            if(imf == VK_NULL_HANDLE)
                continue;
            OGRE_VK_CHECK(vkWaitForFences(mDevice->mDevice, 1, &imf, VK_TRUE, UINT64_MAX));
        }

        mTexture->unload();
        mDepthTexture->unload();
        mRenderPassDescriptor->releaseFbo();
        mDevice->mRenderSystem->notifySwapchainDestroyed();

        for(auto iv : mSwapchainImageViews)
        {
            vkDestroyImageView(mDevice->mDevice, iv, 0);
        }

        for(auto s : mImageReadySemaphores)
        {
            vkDestroySemaphore( mDevice->mDevice, s, 0 );
        }

        for(auto s : mRenderFinishedSemaphores)
        {
            vkDestroySemaphore( mDevice->mDevice, s, 0 );
        }

        vkDestroySwapchainKHR(mDevice->mDevice, mSwapchain, 0);
        mSwapchain = 0;

        mSwapchainStatus = SwapchainReleased;
    }
    //-------------------------------------------------------------------------
    void VulkanWindow::acquireNextImage( void )
    {
        OGRE_ASSERT_LOW( mSwapchainStatus == SwapchainReleased );

        mCurrentSemaphoreIndex = (mCurrentSemaphoreIndex + 1) % mImageReadySemaphores.size();
        auto semaphore = mImageReadySemaphores[mCurrentSemaphoreIndex];

        uint32 imageIdx = 0u;
        auto res =
            vkAcquireNextImageKHR(mDevice->mDevice, mSwapchain, UINT64_MAX, semaphore, VK_NULL_HANDLE, &imageIdx);
        if (res != VK_ERROR_OUT_OF_DATE_KHR && res != VK_SUBOPTIMAL_KHR && res != VK_SUCCESS)
        {
            LogManager::getSingleton().logError("vkAcquireNextImageKHR failed with" + vkResultToString(res));
            return;
        }

        if(mImageFences[imageIdx])
            OGRE_VK_CHECK(vkWaitForFences(mDevice->mDevice, 1, &mImageFences[imageIdx], VK_TRUE, UINT64_MAX));

        mSwapchainStatus = SwapchainAcquired;
        mTexture->_setCurrentImage( mSwapchainImages[imageIdx], imageIdx );
    }

    void VulkanWindow::resize(uint width, uint height)
    {
        if (mClosed)
            return;

        if (mWidth == width && mHeight == height)
            return;

        if (width != 0 && height != 0)
        {
            RenderWindow::resize(width, height);

            // recreate swapchain
            mDevice->stall();
            destroySwapchain();
            createSwapchain();
        }
    }

    void VulkanWindow::windowMovedOrResized()
    {
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
        // read size from handle
        auto width = ANativeWindow_getWidth((ANativeWindow*)mWindowHandle);
        auto height = ANativeWindow_getHeight((ANativeWindow*)mWindowHandle);
        resize(width, height);
#endif
    }

    void VulkanWindow::copyContentsToMemory(const Box& src, const PixelBox &dst, FrameBuffer buffer)
    {
        mTexture->getBuffer()->blitToMemory(src, dst);
    }

    //-------------------------------------------------------------------------
    void VulkanWindow::destroy( void )
    {
        destroySwapchain();
        delete mTexture;
        delete mDepthTexture;
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
    void VulkanWindow::createSurface(size_t windowHandle)
    {
#if defined(VK_USE_PLATFORM_XLIB_KHR)
        Display *dpy = XOpenDisplay(NULL);

        VkXlibSurfaceCreateInfoKHR surfCreateInfo = {VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR};
        surfCreateInfo.dpy = dpy;
        surfCreateInfo.window = (Window)windowHandle;;

        int scr = DefaultScreen( dpy );
        if (!vkGetPhysicalDeviceXlibPresentationSupportKHR(mDevice->mPhysicalDevice, mDevice->mGraphicsQueue.mFamilyIdx,
                                                           dpy, DefaultVisual(dpy, scr)->visualid))
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Vulkan not supported on given X11 window");
        }

        OGRE_VK_CHECK(vkCreateXlibSurfaceKHR(mDevice->mInstance, &surfCreateInfo, 0, &mSurfaceKHR));
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
        HINSTANCE hInst = NULL;
        static TCHAR staticVar;
        GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, &staticVar, &hInst);

        VkWin32SurfaceCreateInfoKHR surfCreateInfo = {VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
        surfCreateInfo.hinstance = hInst;
        surfCreateInfo.hwnd = (HWND)windowHandle;

        OGRE_VK_CHECK(vkCreateWin32SurfaceKHR(mDevice->mInstance, &surfCreateInfo, 0, &mSurfaceKHR));
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
        VkAndroidSurfaceCreateInfoKHR surfCreateInfo = {VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR};
        surfCreateInfo.window = (ANativeWindow*)windowHandle;

        OGRE_VK_CHECK(vkCreateAndroidSurfaceKHR(mDevice->mInstance, &surfCreateInfo, 0, &mSurfaceKHR));
#else
        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unsupported Vulkan platform");
#endif
    }

    void VulkanWindow::createSurface(size_t wlSurface, size_t wlDisplay)
    {
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
        VkWaylandSurfaceCreateInfoKHR surfCreateInfo = {VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR};
        surfCreateInfo.display = (struct wl_display*)wlDisplay;
        surfCreateInfo.surface = (struct wl_surface*)wlSurface;

        OGRE_VK_CHECK(vkCreateWaylandSurfaceKHR(mDevice->mInstance, &surfCreateInfo, 0, &mSurfaceKHR));
#else
        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unsupported Vulkan platform");
#endif
    }

    const char *VulkanWindow::getRequiredExtensionName()
    {
#if defined(VK_USE_PLATFORM_XLIB_KHR)
        return VK_KHR_XLIB_SURFACE_EXTENSION_NAME;
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
        return VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME;
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
        return VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
        return VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
#endif
        return "";
    }

    void VulkanWindow::create(const String& name, unsigned int width, unsigned int height, bool fullScreen,
                              const NameValuePairList* miscParams)
    {
        mActive = true;
        mVisible = true;
        mClosed = false;
        mHwGamma = false;
        mWidth = width;
        mHeight = height;
        mFSAA = 1;

        size_t wlDisplay = 0;

        if( miscParams )
        {
            NameValuePairList::const_iterator end = miscParams->end();

            auto opt = miscParams->find( "externalWindowHandle" );
            if( opt != end )
                mWindowHandle = StringConverter::parseSizeT( opt->second );

            opt = miscParams->find( "externalWlSurface" );
            if( opt != end )
                mWindowHandle = StringConverter::parseSizeT( opt->second );


            opt = miscParams->find( "externalWlDisplay" );
            if( opt != end )
                wlDisplay = StringConverter::parseSizeT( opt->second );

            opt = miscParams->find( "vsync" );
            if( opt != end )
                mVSync = StringConverter::parseBool( opt->second );
            opt = miscParams->find( "vsyncInterval" );
            if( opt != end )
                mVSyncInterval = StringConverter::parseUnsignedInt( opt->second );
            opt = miscParams->find( "FSAA" );
            if( opt != end )
                mFSAA = StringConverter::parseUnsignedInt(opt->second);
            opt = miscParams->find( "gamma" );
            if( opt != end )
                mHwGamma = StringConverter::parseBool( opt->second );
        }

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
        OgreAssert( mWindowHandle && wlDisplay, "externalWlSurface and externalWlDisplay required");
        createSurface(mWindowHandle, wlDisplay);
#else
        OgreAssert( mWindowHandle, "externalWindowHandle required");
        createSurface(mWindowHandle);
        (void)wlDisplay;
#endif

        auto texMgr = TextureManager::getSingletonPtr();
        mTexture = new VulkanTextureGpuWindow("RenderWindow", TEX_TYPE_2D, texMgr, this);;
        mTexture->setFormat(chooseSurfaceFormat(mHwGamma));
        mTexture->setHardwareGammaEnabled(mHwGamma);
        mTexture->setFSAA(mFSAA, "");

        mDepthTexture = new VulkanTextureGpu(texMgr, "RenderWindow DepthBuffer", 0, "", true, 0);
        mDepthTexture->setFormat(PF_DEPTH32);
        mDepthTexture->setFSAA(mFSAA, "");
#if 0
        mStencilBuffer = 0;
        if( PixelFormatGpuUtils::isStencil( mDepthBuffer->getPixelFormat() ) )
            mStencilBuffer = mDepthBuffer;
#endif

        mRenderPassDescriptor.reset(new VulkanRenderPassDescriptor(&mDevice->mGraphicsQueue, mDevice->mRenderSystem));
        createSwapchain();
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
        // TODO OGRE_ASSERT_MEDIUM( mSwapchainStatus != SwapchainPendingSwap );

        VkSemaphore retVal = 0;
        if( mSwapchainStatus == SwapchainAcquired )
        {
            mSwapchainStatus = SwapchainUsedInRendering;
            retVal = mImageReadySemaphores[mCurrentSemaphoreIndex];
        }
        return retVal;
    }

    VkSemaphore VulkanWindow::getRenderFinishedSemaphore() const
    {
        return mRenderFinishedSemaphores[mCurrentSemaphoreIndex];
    }

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
        createSwapchain();
    }
    //-------------------------------------------------------------------------
    void VulkanWindow::swapBuffers( void )
    {
        mSwapchainStatus = SwapchainUsedInRendering;
        if( mSwapchainStatus == SwapchainAcquired || mSwapchainStatus == SwapchainPendingSwap )
        {
            // Ogre never rendered to this window. There's nothing to present.
            // Pass the currently acquired swapchain onto the next frame.
            // Or alternatively, swapBuffers was called twice in a row
            return;
        }

        OGRE_ASSERT_LOW( mSwapchainStatus == SwapchainUsedInRendering );

        mDevice->mGraphicsQueue.mWindowsPendingSwap.push_back( this );
        mSwapchainStatus = SwapchainPendingSwap;

        //endRenderPassDescriptor();
        mDevice->commitAndNextCommandBuffer( SubmissionType::EndFrameAndSwap );
    }
    //-------------------------------------------------------------------------
    void VulkanWindow::_swapBuffers()
    {
        OGRE_ASSERT_LOW( mSwapchainStatus == SwapchainPendingSwap );

        auto queueFinishSemaphore = getRenderFinishedSemaphore();
        const uint32 currentImageIdx = mTexture->getCurrentImageIdx();

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

        VkPresentInfoKHR present = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
        present.swapchainCount = 1u;
        present.pSwapchains = &mSwapchain;
        present.pImageIndices = &currentImageIdx;
        present.waitSemaphoreCount = 1u;
        present.pWaitSemaphores = &queueFinishSemaphore;

        VkResult result = vkQueuePresentKHR( mDevice->mPresentQueue, &present );

        if( result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR && result != VK_ERROR_OUT_OF_DATE_KHR )
        {
            LogManager::getSingleton().logMessage(
                "[VulkanWindow::swapBuffers] vkQueuePresentKHR: error presenting VkResult = " +
                vkResultToString( result ) );
        }

        if( result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR )
            LogManager::getSingleton().logMessage("[VulkanWindow::swapBuffers] swapchain suboptimal or out fo date");

        mSwapchainStatus = SwapchainReleased;
    }
}  // namespace Ogre
