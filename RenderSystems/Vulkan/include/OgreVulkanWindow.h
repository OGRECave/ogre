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
#ifndef _OgreVulkanWindow_H_
#define _OgreVulkanWindow_H_

#include "OgreVulkanPrerequisites.h"

#include "OgreWindow.h"

namespace Ogre
{
    class VulkanWindow : public Window
    {
    public:
        enum Backend
        {
            BackendX11 = 1u << 0u
        };
        enum SwapchainStatus
        {
            /// We already called VulkanWindow::acquireNextSwapchain.
            ///
            /// Can only go into this state if we're coming from SwapchainReleased
            SwapchainAcquired,
            /// We already called VulkanWindow::getImageAcquiredSemaphore.
            /// Further calls to getImageAcquiredSemaphore will return null.
            /// Ogre is rendering or intends to into this swapchain.
            ///
            /// Can only go into this state if we're coming from SwapchainAcquired
            SwapchainUsedInRendering,
            /// We've come from SwapchainUsedInRendering and are waiting for
            /// VulkanDevice::commitAndNextCommandBuffer to present us
            SwapchainPendingSwap,
            /// We don't own a swapchain. Ogre cannot render to this window.
            ///
            /// This status should not last long unless we're not initialized yet.
            SwapchainReleased
        };

        bool mLowestLatencyVSync;
        bool mHwGamma;
        bool mClosed;

        VulkanDevice *mDevice;

        VkSurfaceKHR mSurfaceKHR;
        VkSwapchainKHR mSwapchain;
        FastArray<VkImage> mSwapchainImages;
        /// Note: We need a semaphore per frame, not per swapchain.
        ///
        /// Makes Queue execution wait until the acquired image is done presenting
        VkSemaphore mSwapchainSemaphore;
        SwapchainStatus mSwapchainStatus;
        bool mRebuildingSwapchain;
        bool mSuboptimal;

        void parseSharedParams( const NameValuePairList *miscParams );

        PixelFormatGpu chooseSurfaceFormat( bool hwGamma );
        void createSwapchain( void );
        void destroySwapchain( void );

    public:
        void acquireNextSwapchain( void );

    public:
        VulkanWindow( const String &title, uint32 width, uint32 height, bool fullscreenMode );
        virtual ~VulkanWindow();

        virtual void destroy( void );

        void _setDevice( VulkanDevice *device );
        virtual void _initialize( TextureGpuManager *textureGpuManager );
        virtual void _initialize( TextureGpuManager *textureGpuManager,
                                  const NameValuePairList *miscParams ) = 0;

        /// Returns null if getImageAcquiredSemaphore has already been called during this frame
        VkSemaphore getImageAcquiredSemaphore( void );

        size_t getNumSwapchains( void ) const { return mSwapchainImages.size(); }
        VkImage getSwapchainImage( size_t idx ) const { return mSwapchainImages[idx]; }

        virtual bool isClosed( void ) const;

        virtual void setVSync( bool vSync, uint32 vSyncInterval );

        /// Tells our VulkanDevice that the next commitAndNextCommandBuffer call should present us
        /// Calling swapBuffers during the command buffer that is rendering to us is key for
        /// good performance; otherwise Ogre may split the commands that render to this window
        /// and the command that presents this window into two queue submissions.
        virtual void swapBuffers( void );

        /** Actually performs present. Called by VulkanDevice::commitAndNextCommandBuffer
        @param queueFinishSemaphore
            Makes our present request wait until the Queue is done executing before we can present
        */
        void _swapBuffers( VkSemaphore queueFinishSemaphore );
    };
}  // namespace Ogre

#endif
