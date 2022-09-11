/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2017 Torus Knot Software Ltd

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

#ifndef _OgreVulkanTextureGpu_H_
#define _OgreVulkanTextureGpu_H_

#include "OgreVulkanPrerequisites.h"

#include "OgreTexture.h"

#include "OgreHardwarePixelBuffer.h"
#include "OgreVulkanHardwareBuffer.h"
#include "OgreRenderTexture.h"

#include "vk_mem_alloc.h"

namespace Ogre
{
    /** \addtogroup Core
     *  @{
     */
    /** \addtogroup Resources
     *  @{
     */

    class VulkanRenderPassDescriptor;

    namespace ResourceAccess
    {
    /// Enum identifying the texture access privilege
    enum ResourceAccess
    {
        Undefined = 0x00,
        Read = 0x01,
        Write = 0x02,
        ReadWrite = Read | Write
    };
    }

    class VulkanHardwarePixelBuffer : public HardwarePixelBuffer
    {
        VulkanTextureGpu* mParent;
        uint8 mFace;
        uint32 mLevel;
        std::unique_ptr<VulkanHardwareBuffer> mStagingBuffer;
        PixelBox lockImpl(const Box &lockBox,  LockOptions options) override;
        void unlockImpl(void) override;
    public:
        VulkanHardwarePixelBuffer(VulkanTextureGpu* tex, uint32 width, uint32 height, uint32 depth, uint8 face, uint32 mip);
        void blitFromMemory(const PixelBox& src, const Box& dstBox) override;
        void blitToMemory(const Box& srcBox, const PixelBox& dst) override;
    };

    class _OgreVulkanExport VulkanTextureGpu : public Texture
    {
    protected:
        /// The general case is that the whole D3D11 texture will be accessed through the SRV.
        /// That means: createSrv( this->getPixelFormat(), false );
        /// To avoid creating multiple unnecessary copies of the SRV, we keep a cache of that
        /// default SRV with us; and calling createSrv with default params will return
        /// this cache instead.
        VkImageView mDefaultDisplaySrv;

        /// This will not be owned by us if hasAutomaticBatching is true.
        /// It will also not be owned by us if we're not in GpuResidency::Resident
        /// This will always point to:
        ///     * A GL texture owned by us.
        ///     * A 4x4 dummy texture (now owned by us).
        ///     * A 64x64 mipmapped texture of us (but now owned by us).
        ///     * A GL texture not owned by us, but contains the final information.
        VkImage mDisplayTextureName;

        /// When we're transitioning to GpuResidency::Resident but we're not there yet,
        /// we will be either displaying a 4x4 dummy texture or a 64x64 one. However
        /// we reserve a spot to a final place will already be there for us once the
        /// texture data is fully uploaded. This variable contains that texture.
        /// Async upload operations should stack to this variable.
        /// May contain:
        ///     1. 0, if not resident or resident but not yet reached main thread.
        ///     2. The texture
        ///     3. An msaa texture (hasMsaaExplicitResolves == true)
        ///     4. The msaa resolved texture (hasMsaaExplicitResolves==false)
        /// This value may be a renderbuffer instead of a texture if isRenderbuffer() returns true.
        VkImage mFinalTextureName;
        VmaAllocation mAllocation;

        /// Only used when hasMsaaExplicitResolves() == false
        VkImage mMsaaTextureName;
        VmaAllocation mMsaaAllocation;
    public:
        /// The current layout we're in. Including any internal stuff.
        VkImageLayout mCurrLayout;
        /// The layout we're expected to be when rendering or doing compute, rather than when doing
        /// internal stuff (e.g. this variable won't contain VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        /// because that origins from C++ operations and are not expected by the compositor)
        ///
        /// When mCurrLayout != mNextLayout, it means that there is already a layout transition
        /// that will be happening to achieve mCurrLayout = mNextLayout
        VkImageLayout mNextLayout;

    protected:
        void createInternalResourcesImpl( void ) override;
        void freeInternalResourcesImpl( void ) override;

        virtual void createMsaaSurface( void );
        virtual void destroyMsaaSurface( void );
    public:
        uint32 getNumLayers() const { return mTextureType == TEX_TYPE_2D_ARRAY ? mDepth : getNumFaces(); }
        bool hasMsaaExplicitResolves() const { return false; }
        bool isUav() const { return false; }
        bool isMultisample() const { return mFSAA > 1; }
        virtual bool isRenderWindowSpecific() const { return false; }

        VulkanTextureGpu(TextureManager* textureManager, const String& name, ResourceHandle handle,
                         const String& group, bool isManual, ManualResourceLoader* loader);
        virtual ~VulkanTextureGpu();

        VkImageLayout getCurrentLayout( void ) const { return mCurrLayout; }

        virtual void copyTo( TextureGpu *dst, const PixelBox &dstBox, uint8 dstMipLevel,
                             const PixelBox &srcBox, uint8 srcMipLevel,
                             bool keepResolvedTexSynced = true,
                             ResourceAccess::ResourceAccess issueBarriers = ResourceAccess::ReadWrite );

        void _autogenerateMipmaps( bool bUseBarrierSolver = false );

        VkImageType getVulkanTextureType( void ) const;

        VkImageViewType getInternalVulkanTextureViewType( void ) const;

        VkImageView _createView(uint8 mipLevel, uint8 numMipmaps, uint16 arraySlice, uint32 numSlices = 0u,
                                VkImage imageOverride = 0) const;

        VkImageView createView( void ) const;
        VkImageView getDefaultDisplaySrv( void ) const { return mDefaultDisplaySrv; }

        void destroyView( VkImageView imageView );

        /// Returns a fresh VkImageMemoryBarrier filled with common data.
        /// srcAccessMask, dstAccessMask, oldLayout and newLayout must be filled by caller
        VkImageMemoryBarrier getImageMemoryBarrier( void ) const;

        VkImage getDisplayTextureName( void ) const { return mDisplayTextureName; }
        VkImage getFinalTextureName( void ) const { return mFinalTextureName; }
        VkImage getMsaaTextureName( void ) const { return mMsaaTextureName; }
    };

    class VulkanRenderTexture : public RenderTexture
    {
        std::unique_ptr<VulkanTextureGpu> mDepthTexture;
        std::unique_ptr<VulkanRenderPassDescriptor> mRenderPassDescriptor;
    public:
        VulkanRenderTexture(const String& name, HardwarePixelBuffer* buffer, uint32 zoffset, VulkanTextureGpu* target,
                            uint32 face);

        bool requiresTextureFlipping() const override { return true; }

        VulkanRenderPassDescriptor* getRenderPassDescriptor() const { return mRenderPassDescriptor.get(); }
    };

    /** @} */
    /** @} */
}  // namespace Ogre

#endif
