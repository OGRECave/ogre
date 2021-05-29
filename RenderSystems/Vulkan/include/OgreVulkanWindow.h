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

#include "OgreRenderWindow.h"
#include "OgreVulkanTextureGpuWindow.h"
#include "vulkan/vulkan_core.h"

namespace Ogre
{
    class VulkanWindow : public RenderWindow
    {
    public:
        enum SwapchainStatus
        {
            /// We already called VulkanWindow::acquireNextImage.
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

        bool mVisible;
        bool mHidden;

        bool mVSync;
        int mVSyncInterval;

        VulkanDevice *mDevice;
        VulkanTextureGpuWindow* mTexture;
        VulkanTextureGpuRenderTarget* mDepthTexture;

        VkSurfaceKHR mSurfaceKHR;
        VkSwapchainKHR mSwapchain;
        std::vector<VkImage> mSwapchainImages;
        std::vector<VkImageView> mSwapchainImageViews;

        /// Makes Queue execution wait until the acquired image is done presenting
        std::vector<VkSemaphore> mImageReadySemaphores;
        std::vector<VkSemaphore> mRenderFinishedSemaphores;
        int mCurrentSemaphoreIndex;
        SwapchainStatus mSwapchainStatus;
        bool mRebuildingSwapchain;
        bool mSuboptimal;

        VkSurfaceTransformFlagBitsKHR mSurfaceTransform;

        PixelFormat chooseSurfaceFormat( bool hwGamma );
        void createSwapchain( void );
        void destroySwapchain( void );

        void createSurface(size_t windowHandle);

    public:
        void acquireNextImage( void );

    public:
        VulkanWindow( const String &title, uint32 width, uint32 height, bool fullscreenMode );
        virtual ~VulkanWindow();

        static const char *getRequiredExtensionName();

        VulkanTextureGpu* getTexture() { return mTexture; }
        VulkanTextureGpu* getDepthTexture() { return mDepthTexture; }

        VkSurfaceTransformFlagBitsKHR getSurfaceTransform() const { return mSurfaceTransform; }

        virtual void destroy( void );

        virtual void reposition( int32 leftPt, int32 topPt ) {}

        void setVisible( bool visible ) override { mVisible = visible; }
        bool isVisible( void ) const override { return mVisible; }
        void setHidden( bool hidden ) { mHidden = hidden; }
        bool isHidden( void ) const { return mHidden; }

        PixelFormat suggestPixelFormat() const { return mTexture->getFormat(); }
        void copyContentsToMemory(const Box& src, const PixelBox &dst, FrameBuffer buffer = FB_AUTO);
        bool requiresTextureFlipping() const { return false; }

        void resize(unsigned int widthPt, unsigned int heightPt) override;

        void _setDevice( VulkanDevice *device );

        void create(const String& name, unsigned int widthPt, unsigned int heightPt, bool fullScreen,
                    const NameValuePairList* miscParams);

        /// Returns null if getImageAcquiredSemaphore has already been called during this frame
        VkSemaphore getImageAcquiredSemaphore( void );

        VkSemaphore getRenderFinishedSemaphore() const;

        const std::vector<VkImageView>& getSwapchainImageViews() const { return mSwapchainImageViews; }

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
        void _swapBuffers();
    };
}  // namespace Ogre

#endif
