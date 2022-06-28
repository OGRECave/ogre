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
    typedef VulkanRenderPassDescriptor RenderPassDescriptor;

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

    typedef std::unordered_map<uint32, VulkanFrameBufferDescValue> VulkanFrameBufferDescMap;

    class _OgreVulkanExport VulkanRenderPassDescriptor
    {
    public:
        VulkanTextureGpu* mColour[1];
        VulkanTextureGpu* mDepth;
        uint8             mNumColourEntries = 0;
        uint8             mSlice = 0;
    private:
        // 1 per MRT
        // 1 per MRT MSAA resolve
        // 1 for Depth buffer
        // 1 for Stencil buffer
        VkClearValue mClearValues[OGRE_MAX_MULTIPLE_RENDER_TARGETS * 2u + 2u];

        VulkanFrameBufferDescMap::iterator mSharedFboItor;

        /// This value MUST be set to the resolution of any of the textures.
        /// If it's changed arbitrarily then FrameBufferDescKey will not take
        /// this into account (two RenderPassDescriptors will share the same
        /// FBO when they should not)
        uint32 mTargetWidth;
        uint32 mTargetHeight;

        VulkanQueue *mQueue;
        VulkanRenderSystem *mRenderSystem;

        void calculateSharedKey( void );

        static VkClearColorValue getClearColour( const ColourValue &clearColour,
                                                 PixelFormatGpu pixelFormat );
        void setupColourAttachment( const size_t idx, VulkanFrameBufferDescValue &fboDesc,
                                    VkAttachmentDescription *attachments, uint32 &currAttachmIdx,
                                    VkAttachmentReference *colourAttachRefs,
                                    VkAttachmentReference *resolveAttachRefs, const size_t vkIdx,
                                    const bool bResolveTex );
        VkImageView setupDepthAttachment( VkAttachmentDescription &attachment );

        void setupFbo( VulkanFrameBufferDescValue &fboDesc );
        static void destroyFbo( VulkanQueue *queue, VulkanFrameBufferDescValue &fboDesc );
    public:
        VulkanRenderPassDescriptor( VulkanQueue *graphicsQueue, VulkanRenderSystem *renderSystem );
        virtual ~VulkanRenderPassDescriptor();

        void releaseFbo( void );

        virtual void entriesModified( bool createFbo );

        virtual void setClearColour( uint8 idx, const ColourValue &clearColour );
        virtual void setClearDepth( Real clearDepth );
        virtual void setClearStencil( uint32 clearStencil );

        /// Sets the clear colour to all entries. In some APIs may be faster
        /// than calling setClearColour( idx, clearColour ) for each entry
        /// individually.
        virtual void setClearColour( const ColourValue &clearColour );

        VkRenderPass getRenderPass() const;

        void performLoadActions();
        void performStoreActions();
    };

    /** @} */
    /** @} */
}  // namespace Ogre

#include "OgreHeaderSuffix.h"

#endif
