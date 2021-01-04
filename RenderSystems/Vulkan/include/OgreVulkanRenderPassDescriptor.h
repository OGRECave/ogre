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

#ifndef _OgreVulkanRenderPassDescriptor_H_
#define _OgreVulkanRenderPassDescriptor_H_

#include "OgreVulkanPrerequisites.h"

#include "OgreCommon.h"
#include "OgrePixelFormat.h"

#include "vulkan/vulkan_core.h"

#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Core
     *  @{
     */
    /** \addtogroup Resources
     *  @{
     */
    // forward compatibility defines
    class VulkanRenderPassDescriptor;
    struct VulkanFrameBufferDescKey;

    typedef VulkanRenderPassDescriptor RenderPassDescriptor;
    typedef VulkanFrameBufferDescKey FrameBufferDescKey;

    struct VulkanFrameBufferDescKey
    {
        uint8             numColourEntries = 1;
        VulkanTextureGpu* colour[1] = {};
        VulkanTextureGpu* depth = 0;
        VulkanFrameBufferDescKey();
        VulkanFrameBufferDescKey( const RenderPassDescriptor &desc );

        bool operator<( const VulkanFrameBufferDescKey &other ) const;
    };

    struct VulkanFlushOnlyDescValue
    {
        uint16 refCount;
        VulkanFlushOnlyDescValue();
    };

    struct VulkanFrameBufferDescValue
    {
        uint16 refCount;

        uint32 mNumImageViews;
        VkImageView mImageViews[OGRE_MAX_MULTIPLE_RENDER_TARGETS * 2u + 2u];
        FastArray<VkImageView> mWindowImageViews;  // Only used by windows
        // We normally need just one, but Windows are a special case
        // because we need to have one per swapchain image, hence FastArray
        //
        // A single VkFramebuffer contains the VkRenderPass + the actual imageviews + resolution
        FastArray<VkFramebuffer> mFramebuffers;

        /// Contains baked info of load/store/clear
        /// Doesn't reference ImageViews, however it mentions them by attachmentIdx
        /// which makes VkRenderPass difficult to actually share
        ///
        /// Thus we generate VkRenderPass and FBOs together
        VkRenderPass mRenderPass;

        VulkanFrameBufferDescValue();
    };

    typedef std::map<VulkanFrameBufferDescKey, VulkanFrameBufferDescValue> VulkanFrameBufferDescMap;
    typedef std::map<FrameBufferDescKey, VulkanFlushOnlyDescValue> VulkanFlushOnlyDescMap;

    class _OgreVulkanExport VulkanRenderPassDescriptor
    {
    public:
        VulkanTextureGpu* mColour[1];
        VulkanTextureGpu* mDepth;
        uint8                   mNumColourEntries = 0;
    private:
        /// When true, beginRenderPassDescriptor & endRenderPassDescriptor won't actually
        /// load/store this pass descriptor; but will still set the mCurrentRenderPassDescriptor
        /// so we have required information by some passes.
        /// Examples of these are stencil passes.
        public: bool            mInformationOnly = false;

        // 1 per MRT
        // 1 per MRT MSAA resolve
        // 1 for Depth buffer
        // 1 for Stencil buffer
        VkClearValue mClearValues[OGRE_MAX_MULTIPLE_RENDER_TARGETS * 2u + 2u];

        VulkanFrameBufferDescMap::iterator mSharedFboItor;
        /// We need mSharedFboItor & mSharedFboFlushItor because:
        ///     - In Vulkan FrameBufferObjects include load/store actions embedded,
        ///       so we want to share them. That's mSharedFboItor for
        ///     - But a clear pass followed by the same pass that performs a load can be
        ///       merged together as if they were one pass. That's what mSharedFboFlushItor
        ///       is for. Metal only has mSharedFboItor because load/store actions are not
        ///       embedded into API objects
        VulkanFlushOnlyDescMap::iterator mSharedFboFlushItor;

        /// This value MUST be set to the resolution of any of the textures.
        /// If it's changed arbitrarily then FrameBufferDescKey will not take
        /// this into account (two RenderPassDescriptors will share the same
        /// FBO when they should not)
        uint32 mTargetWidth;
        uint32 mTargetHeight;

        VulkanQueue *mQueue;
        VulkanRenderSystem *mRenderSystem;

#if OGRE_DEBUG_MODE && OGRE_PLATFORM == OGRE_PLATFORM_LINUX
        void *mCallstackBacktrace[32];
        size_t mNumCallstackEntries;
#endif

        void checkRenderWindowStatus( void );
        void calculateSharedKey( void );
        void calculateSharedFlushOnlyKey( void );

        static VkClearColorValue getClearColour( const ColourValue &clearColour,
                                                 PixelFormatGpu pixelFormat );
        void setupColourAttachment( const size_t idx, VulkanFrameBufferDescValue &fboDesc,
                                    VkAttachmentDescription *attachments, uint32 &currAttachmIdx,
                                    VkAttachmentReference *colourAttachRefs,
                                    VkAttachmentReference *resolveAttachRefs, const size_t vkIdx,
                                    const bool bResolveTex );
        VkImageView setupDepthAttachment( VkAttachmentDescription &attachment );

        void setupFbo( VulkanFrameBufferDescValue &fboDesc );
        void releaseFbo( void );
        static void destroyFbo( VulkanQueue *queue, VulkanFrameBufferDescValue &fboDesc );
    public:
        enum {
            All = 1,
            Colour0 = All,
            Depth = All,
            Stencil = All
        };

        void checkWarnIfRtvWasFlushed( uint32 entriesToFlush ) {}

        VulkanRenderPassDescriptor( VulkanQueue *graphicsQueue, VulkanRenderSystem *renderSystem );
        virtual ~VulkanRenderPassDescriptor();

        void notifySwapchainCreated( VulkanWindow *window );
        void notifySwapchainDestroyed( VulkanWindow *window );

        virtual void entriesModified( uint32 entryTypes );

        virtual void setClearColour( uint8 idx, const ColourValue &clearColour );
        virtual void setClearDepth( Real clearDepth );
        virtual void setClearStencil( uint32 clearStencil );

        /// Sets the clear colour to all entries. In some APIs may be faster
        /// than calling setClearColour( idx, clearColour ) for each entry
        /// individually.
        virtual void setClearColour( const ColourValue &clearColour );

        uint32 willSwitchTo( VulkanRenderPassDescriptor *newDesc, bool warnIfRtvWasFlushed ) const;

        void performLoadActions( bool renderingWasInterrupted );
        void performStoreActions( bool isInterruptingRendering );
    };

    /** @} */
    /** @} */
}  // namespace Ogre

#include "OgreHeaderSuffix.h"

#endif
