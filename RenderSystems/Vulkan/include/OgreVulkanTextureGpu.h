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

#include "OgreDescriptorSetTexture.h"
#include "OgreDescriptorSetUav.h"
#include "OgreVulkanPrerequisites.h"

#include "OgreTextureGpu.h"

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

    class _OgreVulkanExport VulkanTextureGpu : public TextureGpu
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

        /// Only used when hasMsaaExplicitResolves() == false
        VkImage mMsaaFramebufferName;

        bool mOwnsSrv;

        size_t mVboPoolIdx;
        size_t mInternalBufferStart;

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
        virtual void createInternalResourcesImpl( void );
        virtual void destroyInternalResourcesImpl( void );

        virtual void createMsaaSurface( void );
        virtual void destroyMsaaSurface( void );

    public:
        VulkanTextureGpu( GpuPageOutStrategy::GpuPageOutStrategy pageOutStrategy, VaoManager *vaoManager,
                          IdString name, uint32 textureFlags, TextureTypes::TextureTypes initialType,
                          TextureGpuManager *textureManager );
        virtual ~VulkanTextureGpu();

        PixelFormatGpu getWorkaroundedPixelFormat( const PixelFormatGpu pixelFormat ) const;

        virtual void setTextureType( TextureTypes::TextureTypes textureType );

        virtual ResourceLayout::Layout getCurrentLayout( void ) const;

        virtual void copyTo( TextureGpu *dst, const TextureBox &dstBox, uint8 dstMipLevel,
                             const TextureBox &srcBox, uint8 srcMipLevel,
                             bool keepResolvedTexSynced = true,
                             ResourceAccess::ResourceAccess issueBarriers = ResourceAccess::ReadWrite );

        virtual void _setNextLayout( ResourceLayout::Layout layout );

        virtual void _autogenerateMipmaps( bool bUseBarrierSolver = false );

        virtual void getSubsampleLocations( vector<Vector2>::type locations );

        virtual void notifyDataIsReady( void );
        virtual bool _isDataReadyImpl( void ) const;

        virtual void _setToDisplayDummyTexture( void );
        virtual void _notifyTextureSlotChanged( const TexturePool *newPool, uint16 slice );

        VkImageSubresourceRange getFullSubresourceRange( void ) const;

        VkImageType getVulkanTextureType( void ) const;

        VkImageViewType getInternalVulkanTextureViewType( void ) const;

        VkImageView _createView( PixelFormatGpu pixelFormat, uint8 mipLevel, uint8 numMipmaps,
                                 uint16 arraySlice, bool cubemapsAs2DArrays, bool forUav,
                                 uint32 numSlices = 0u, VkImage imageOverride = 0 ) const;

        VkImageView createView( const DescriptorSetTexture2::TextureSlot &texSlot,
                                bool bUseCache = true ) const;
        VkImageView createView( DescriptorSetUav::TextureSlot texSlot, bool bUseCache = true );
        VkImageView createView( void ) const;
        VkImageView getDefaultDisplaySrv( void ) const { return mDefaultDisplaySrv; }

        void destroyView( VkImageView imageView );
        void destroyView( DescriptorSetTexture2::TextureSlot texSlot, VkImageView imageView );
        void destroyView( DescriptorSetUav::TextureSlot texSlot, VkImageView imageView );

        /// Returns a fresh VkImageMemoryBarrier filled with common data.
        /// srcAccessMask, dstAccessMask, oldLayout and newLayout must be filled by caller
        VkImageMemoryBarrier getImageMemoryBarrier( void ) const;

        VkImage getDisplayTextureName( void ) const { return mDisplayTextureName; }
        VkImage getFinalTextureName( void ) const { return mFinalTextureName; }
        VkImage getMsaaFramebufferName( void ) const { return mMsaaFramebufferName; }
    };

    class _OgreVulkanExport VulkanTextureGpuRenderTarget : public VulkanTextureGpu
    {
    protected:
        size_t mMsaaVboPoolIdx;
        size_t mMsaaInternalBufferStart;

        uint16 mDepthBufferPoolId;
        bool mPreferDepthTexture;
        PixelFormatGpu mDesiredDepthBufferFormat;
#if OGRE_NO_VIEWPORT_ORIENTATIONMODE == 0
        OrientationMode mOrientationMode;
#endif

        virtual void createMsaaSurface( void );
        virtual void destroyMsaaSurface( void );

    public:
        VulkanTextureGpuRenderTarget( GpuPageOutStrategy::GpuPageOutStrategy pageOutStrategy,
                                      VaoManager *vaoManager, IdString name, uint32 textureFlags,
                                      TextureTypes::TextureTypes initialType,
                                      TextureGpuManager *textureManager );

        virtual void _setDepthBufferDefaults( uint16 depthBufferPoolId, bool preferDepthTexture,
                                              PixelFormatGpu desiredDepthBufferFormat );
        virtual uint16 getDepthBufferPoolId( void ) const;
        virtual bool getPreferDepthTexture( void ) const;
        virtual PixelFormatGpu getDesiredDepthBufferFormat( void ) const;

        virtual void setOrientationMode( OrientationMode orientationMode );
#if OGRE_NO_VIEWPORT_ORIENTATIONMODE == 0
        virtual OrientationMode getOrientationMode( void ) const;
#endif
    };

    /** @} */
    /** @} */
}  // namespace Ogre

#include "OgreHeaderSuffix.h"

#endif
