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

#include "OgreVulkanTextureGpu.h"

#include "OgreVulkanDelayedFuncs.h"
#include "Vao/OgreVulkanVaoManager.h"

#include "OgrePixelFormatGpuUtils.h"

#include "OgreException.h"
#include "OgreTextureBox.h"
#include "OgreVector2.h"
#include "OgreVulkanMappings.h"
#include "OgreVulkanTextureGpuManager.h"
#include "OgreVulkanUtils.h"

#define TODO_add_resource_transitions

namespace Ogre
{
    VulkanTextureGpu::VulkanTextureGpu( GpuPageOutStrategy::GpuPageOutStrategy pageOutStrategy,
                                        VaoManager *vaoManager, IdString name, uint32 textureFlags,
                                        TextureTypes::TextureTypes initialType,
                                        TextureGpuManager *textureManager ) :
        TextureGpu( pageOutStrategy, vaoManager, name, textureFlags, initialType, textureManager ),
        mDefaultDisplaySrv( 0 ),
        mDisplayTextureName( 0 ),
        mFinalTextureName( 0 ),
        mMsaaFramebufferName( 0 ),
        mOwnsSrv( false ),
        mVboPoolIdx( 0u ),
        mInternalBufferStart( 0u ),
        mCurrLayout( VK_IMAGE_LAYOUT_UNDEFINED ),
        mNextLayout( VK_IMAGE_LAYOUT_UNDEFINED )
    {
        _setToDisplayDummyTexture();
    }
    //-----------------------------------------------------------------------------------
    VulkanTextureGpu::~VulkanTextureGpu() { destroyInternalResourcesImpl(); }
    //-----------------------------------------------------------------------------------
    PixelFormatGpu VulkanTextureGpu::getWorkaroundedPixelFormat( const PixelFormatGpu pixelFormat ) const
    {
        PixelFormatGpu retVal = pixelFormat;
#ifdef OGRE_VK_WORKAROUND_ADRENO_D32_FLOAT
        if( Workarounds::mAdrenoD32FloatBug && isTexture() && isRenderToTexture() )
        {
            if( pixelFormat == PFG_D32_FLOAT )
                retVal = PFG_D24_UNORM;
            else if( pixelFormat == PFG_D32_FLOAT_S8X24_UINT )
                retVal = PFG_D24_UNORM_S8_UINT;
        }
#endif
        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void VulkanTextureGpu::createInternalResourcesImpl( void )
    {
        if( mPixelFormat == PFG_NULL )
            return;  // Nothing to do

        const PixelFormatGpu finalPixelFormat = getWorkaroundedPixelFormat( mPixelFormat );

        VkImageCreateInfo imageInfo;
        makeVkStruct( imageInfo, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO );
        imageInfo.imageType = getVulkanTextureType();
        imageInfo.extent.width = getInternalWidth();
        imageInfo.extent.height = getInternalHeight();
        imageInfo.extent.depth = getDepth();
        imageInfo.mipLevels = mNumMipmaps;
        imageInfo.arrayLayers = getNumSlices();
        imageInfo.flags = 0;
        if( !isReinterpretable() )
            imageInfo.format = VulkanMappings::get( finalPixelFormat );
        else
        {
            imageInfo.format = VulkanMappings::get( PixelFormatGpuUtils::getFamily( finalPixelFormat ) );
            imageInfo.flags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
        }
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        if( hasMsaaExplicitResolves() )
        {
            imageInfo.samples =
                static_cast<VkSampleCountFlagBits>( mSampleDescription.getColourSamples() );
        }
        else
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

        if( mTextureType == TextureTypes::TypeCube || mTextureType == TextureTypes::TypeCubeArray )
            imageInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

        if( isTexture() )
            imageInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
        if( isRenderToTexture() )
        {
            imageInfo.usage |= PixelFormatGpuUtils::isDepth( finalPixelFormat )
                                   ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
                                   : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }
        if( isUav() )
            imageInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;

        String textureName = getNameStr();

        VulkanTextureGpuManager *textureManager =
            static_cast<VulkanTextureGpuManager *>( mTextureManager );
        VulkanDevice *device = textureManager->getDevice();

        VkResult imageResult = vkCreateImage( device->mDevice, &imageInfo, 0, &mFinalTextureName );
        checkVkResult( imageResult, "vkCreateImage" );

        setObjectName( device->mDevice, (uint64_t)mFinalTextureName,
                       VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, textureName.c_str() );

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements( device->mDevice, mFinalTextureName, &memRequirements );

        VulkanVaoManager *vaoManager =
            static_cast<VulkanVaoManager *>( textureManager->getVaoManager() );
        VkDeviceMemory deviceMemory =
            vaoManager->allocateTexture( memRequirements, mVboPoolIdx, mInternalBufferStart );

        VkResult result =
            vkBindImageMemory( device->mDevice, mFinalTextureName, deviceMemory, mInternalBufferStart );
        checkVkResult( result, "vkBindImageMemory" );

        if( isPoolOwner() )
        {
            device->mGraphicsQueue.endAllEncoders();

            // Pool owners transition all its slices to read_only_optimal to avoid the validation layers
            // from complaining the unused (and untouched) slices are in the wrong layout.
            // We wait for no stage, and no stage waits for us. No caches are flushed.
            //
            // Later our TextureGpus using individual slices will perform an
            // undefined -> read_only_optimal transition on the individual slices & mips
            // to fill the data; and those transitions will be the ones who take care of blocking
            // previous/later stages in their respective barriers
            VkImageMemoryBarrier imageBarrier = this->getImageMemoryBarrier();
            imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            vkCmdPipelineBarrier( device->mGraphicsQueue.mCurrentCmdBuffer,
                                  VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                  0, 0u, 0, 0u, 0, 1u, &imageBarrier );

            mCurrLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            mNextLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }
        else
        {
            mCurrLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            mNextLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            if( isRenderToTexture() || isUav() )
            {
                if( isRenderToTexture() )
                {
                    // Assume render textures always start ready to render
                    if( PixelFormatGpuUtils::isDepth( finalPixelFormat ) )
                    {
                        mCurrLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                        mNextLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                    }
                    else
                    {
                        mCurrLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                        mNextLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                    }
                }
                else if( isUav() )
                {
                    // Non-RenderToTextures UAVs are assumed to start in R/W mode
                    mCurrLayout = VK_IMAGE_LAYOUT_GENERAL;
                    mNextLayout = VK_IMAGE_LAYOUT_GENERAL;
                }

                VkImageMemoryBarrier imageBarrier = this->getImageMemoryBarrier();
                imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                imageBarrier.newLayout = mCurrLayout;
                vkCmdPipelineBarrier(
                    device->mGraphicsQueue.mCurrentCmdBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0u, 0, 0u, 0, 1u, &imageBarrier );
            }
        }

        if( mSampleDescription.isMultisample() && !hasMsaaExplicitResolves() )
            createMsaaSurface();
    }
    //-----------------------------------------------------------------------------------
    void VulkanTextureGpu::destroyInternalResourcesImpl( void )
    {
        // If 'this' is being destroyed: We must call notifyTextureDestroyed
        //
        // If 'this' is only being transitioned to OnStorage:
        // Our VkImage is being destroyed; and there may be pending image operations on it.
        // This wouldn't be a problem because the vkDestroyImage call is delayed.
        // However if the texture is later transitioned again to Resident, mCurrLayout & mNextLayout
        // will get out of sync when endCopyEncoder gets called.
        //
        // e.g. if a texture performs:
        //      OnStorage -> Resident -> <upload operation> -> OnStorage -> Resident ->
        //      endCopyEncoder -> <upload operation> -> endCopyEncoder
        //
        // then the 1st endCopyEncoder will set mCurrLayout to SHADER_READ_ONLY_OPTIMAL because
        // it thinks it changed the layout of the current mFinalTextureName, but it actually
        // changed the layout of the previous mFinalTextureName which is scheduled to be destroyed
        VulkanTextureGpuManager *textureManager =
            static_cast<VulkanTextureGpuManager *>( mTextureManager );
        VulkanDevice *device = textureManager->getDevice();
        device->mGraphicsQueue.notifyTextureDestroyed( this );

        if( mDefaultDisplaySrv && mOwnsSrv )
        {
            destroyView( mDefaultDisplaySrv );
            mDefaultDisplaySrv = 0;
            mOwnsSrv = false;
        }

        if( !hasAutomaticBatching() )
        {
            VulkanTextureGpuManager *textureManager =
                static_cast<VulkanTextureGpuManager *>( mTextureManager );
            VulkanDevice *device = textureManager->getDevice();
            if( mFinalTextureName )
            {
                VkMemoryRequirements memRequirements;
                vkGetImageMemoryRequirements( device->mDevice, mFinalTextureName, &memRequirements );

                VulkanVaoManager *vaoManager =
                    static_cast<VulkanVaoManager *>( textureManager->getVaoManager() );

                delayed_vkDestroyImage( vaoManager, device->mDevice, mFinalTextureName, 0 );
                mFinalTextureName = 0;

                vaoManager->deallocateTexture( mVboPoolIdx, mInternalBufferStart, memRequirements.size );
            }

            destroyMsaaSurface();
        }
        else
        {
            if( mTexturePool )
            {
                // This will end up calling _notifyTextureSlotChanged,
                // setting mTexturePool & mInternalSliceStart to 0
                mTextureManager->_releaseSlotFromTexture( this );
            }

            mFinalTextureName = 0;
            mMsaaFramebufferName = 0;
        }

        mCurrLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        mNextLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        _setToDisplayDummyTexture();
    }
    //-----------------------------------------------------------------------------------
    void VulkanTextureGpu::createMsaaSurface( void ) {}
    //-----------------------------------------------------------------------------------
    void VulkanTextureGpu::destroyMsaaSurface( void ) {}
    //-----------------------------------------------------------------------------------
    void VulkanTextureGpu::notifyDataIsReady( void )
    {
        assert( mResidencyStatus == GpuResidency::Resident );
        assert( mFinalTextureName || mPixelFormat == PFG_NULL );

        if( mDefaultDisplaySrv && mOwnsSrv )
        {
            destroyView( mDefaultDisplaySrv );
            mDefaultDisplaySrv = 0;
            mOwnsSrv = false;
        }

        mDisplayTextureName = mFinalTextureName;

        if( isTexture() )
        {
            DescriptorSetTexture2::TextureSlot texSlot(
                DescriptorSetTexture2::TextureSlot::makeEmpty() );
            if( hasAutomaticBatching() )
            {
                VulkanTextureGpu *masterTex =
                    static_cast<VulkanTextureGpu *>( mTexturePool->masterTexture );
                mDefaultDisplaySrv = masterTex->mDefaultDisplaySrv;
                mOwnsSrv = false;
            }
            else
            {
                mDefaultDisplaySrv = createView( texSlot, false );
                mOwnsSrv = true;
            }
        }

        notifyAllListenersTextureChanged( TextureGpuListener::ReadyForRendering );
    }
    //-----------------------------------------------------------------------------------
    bool VulkanTextureGpu::_isDataReadyImpl( void ) const
    {
        return mDisplayTextureName == mFinalTextureName;
    }
    //-----------------------------------------------------------------------------------
    void VulkanTextureGpu::_setToDisplayDummyTexture( void )
    {
        if( !mTextureManager )
        {
            assert( isRenderWindowSpecific() );
            return;  // This can happen if we're a window and we're on shutdown
        }

        if( mDefaultDisplaySrv && mOwnsSrv )
        {
            destroyView( mDefaultDisplaySrv );
            mDefaultDisplaySrv = 0;
            mOwnsSrv = false;
        }

        VulkanTextureGpuManager *textureManagerVk =
            static_cast<VulkanTextureGpuManager *>( mTextureManager );
        if( hasAutomaticBatching() )
        {
            mDisplayTextureName =
                textureManagerVk->getBlankTextureVulkanName( TextureTypes::Type2DArray );

            if( isTexture() )
            {
                mDefaultDisplaySrv = textureManagerVk->getBlankTextureView( TextureTypes::Type2DArray );
                mOwnsSrv = false;
            }
        }
        else
        {
            mDisplayTextureName = textureManagerVk->getBlankTextureVulkanName( mTextureType );
            if( isTexture() )
            {
                mDefaultDisplaySrv = textureManagerVk->getBlankTextureView( mTextureType );
                mOwnsSrv = false;
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void VulkanTextureGpu::_notifyTextureSlotChanged( const TexturePool *newPool, uint16 slice )
    {
        TextureGpu::_notifyTextureSlotChanged( newPool, slice );

        _setToDisplayDummyTexture();

        mCurrLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        mNextLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        if( mTexturePool )
        {
            OGRE_ASSERT_HIGH( dynamic_cast<VulkanTextureGpu *>( mTexturePool->masterTexture ) );
            VulkanTextureGpu *masterTexture =
                static_cast<VulkanTextureGpu *>( mTexturePool->masterTexture );
            mFinalTextureName = masterTexture->mFinalTextureName;
        }

        notifyAllListenersTextureChanged( TextureGpuListener::PoolTextureSlotChanged );
    }
    //-----------------------------------------------------------------------------------
    ResourceLayout::Layout VulkanTextureGpu::getCurrentLayout( void ) const
    {
        switch( mCurrLayout )
        {
        case VK_IMAGE_LAYOUT_UNDEFINED:
            return ResourceLayout::Undefined;
        case VK_IMAGE_LAYOUT_GENERAL:
            return ResourceLayout::Uav;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            return ResourceLayout::RenderTarget;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
            return ResourceLayout::RenderTargetReadOnly;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            return ResourceLayout::Texture;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            return ResourceLayout::CopySrc;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            return ResourceLayout::CopyDst;
        case VK_IMAGE_LAYOUT_PREINITIALIZED:
            return ResourceLayout::Undefined;
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
            return ResourceLayout::RenderTarget;
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            return ResourceLayout::PresentReady;
        case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR:
        case VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV:
        case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT:
        case VK_IMAGE_LAYOUT_MAX_ENUM:
            return ResourceLayout::Undefined;
        }
    }
    //-----------------------------------------------------------------------------------
    void VulkanTextureGpu::setTextureType( TextureTypes::TextureTypes textureType )
    {
        const TextureTypes::TextureTypes oldType = mTextureType;
        TextureGpu::setTextureType( textureType );

        if( oldType != mTextureType && mDisplayTextureName != mFinalTextureName )
            _setToDisplayDummyTexture();
    }
    //-----------------------------------------------------------------------------------
    void VulkanTextureGpu::copyTo( TextureGpu *dst, const TextureBox &dstBox, uint8 dstMipLevel,
                                   const TextureBox &srcBox, uint8 srcMipLevel,
                                   bool keepResolvedTexSynced,
                                   ResourceAccess::ResourceAccess issueBarriers )
    {
        TextureGpu::copyTo( dst, dstBox, dstMipLevel, srcBox, srcMipLevel, issueBarriers );

        OGRE_ASSERT_HIGH( dynamic_cast<VulkanTextureGpu *>( dst ) );

        VulkanTextureGpu *dstTexture = static_cast<VulkanTextureGpu *>( dst );
        VulkanTextureGpuManager *textureManager =
            static_cast<VulkanTextureGpuManager *>( mTextureManager );
        VulkanDevice *device = textureManager->getDevice();

        if( issueBarriers & ResourceAccess::Read )
            device->mGraphicsQueue.getCopyEncoder( 0, this, true );
        else
        {
            // This won't generate barriers, but it will close all other encoders
            // and open the copy one
            device->mGraphicsQueue.getCopyEncoder( 0, 0, true );
        }

        if( issueBarriers & ResourceAccess::Write )
            device->mGraphicsQueue.getCopyEncoder( 0, dstTexture, false );

        VkImageCopy region;

        const uint32 sourceSlice = srcBox.sliceStart + getInternalSliceStart();
        const uint32 destinationSlice = dstBox.sliceStart + dstTexture->getInternalSliceStart();
        const uint32 numSlices = dstBox.numSlices != 0 ? dstBox.numSlices : dstTexture->getNumSlices();

        region.srcSubresource.aspectMask = VulkanMappings::getImageAspect( this->getPixelFormat() );
        region.srcSubresource.mipLevel = srcMipLevel;
        region.srcSubresource.baseArrayLayer = sourceSlice;
        region.srcSubresource.layerCount = numSlices;

        region.srcOffset.x = static_cast<int32_t>( srcBox.x );
        region.srcOffset.y = static_cast<int32_t>( srcBox.y );
        region.srcOffset.z = static_cast<int32_t>( srcBox.z );

        region.dstSubresource.aspectMask = VulkanMappings::getImageAspect( dst->getPixelFormat() );
        region.dstSubresource.mipLevel = dstMipLevel;
        region.dstSubresource.baseArrayLayer = destinationSlice;
        region.dstSubresource.layerCount = numSlices;

        region.dstOffset.x = dstBox.x;
        region.dstOffset.y = dstBox.y;
        region.dstOffset.z = dstBox.z;

        region.extent.width = srcBox.width;
        region.extent.height = srcBox.height;
        region.extent.depth = srcBox.depth;

        VkImage srcTextureName = this->mFinalTextureName;
        VkImage dstTextureName = dstTexture->mFinalTextureName;

        if( this->isMultisample() && !this->hasMsaaExplicitResolves() )
            srcTextureName = this->mMsaaFramebufferName;
        if( dstTexture->isMultisample() && !dstTexture->hasMsaaExplicitResolves() )
            dstTextureName = dstTexture->mMsaaFramebufferName;

        vkCmdCopyImage( device->mGraphicsQueue.mCurrentCmdBuffer, srcTextureName, mCurrLayout,
                        dstTextureName, dstTexture->mCurrLayout, 1u, &region );

        if( dstTexture->isMultisample() && !dstTexture->hasMsaaExplicitResolves() &&
            keepResolvedTexSynced )
        {
            TODO_add_resource_transitions;  // We must add res. transitions and then restore them

            // Must keep the resolved texture up to date.
            VkImageResolve resolve;
            memset( &resolve, 0, sizeof( resolve ) );
            resolve.srcSubresource = region.dstSubresource;
            resolve.dstSubresource = region.dstSubresource;
            resolve.extent.width = getInternalWidth();
            resolve.extent.height = getInternalHeight();
            resolve.extent.depth = getDepth();

            vkCmdResolveImage( device->mGraphicsQueue.mCurrentCmdBuffer,
                               dstTexture->mMsaaFramebufferName, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                               dstTexture->mFinalTextureName, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u,
                               &resolve );
        }

        // Do not perform the sync if notifyDataIsReady hasn't been called yet (i.e. we're
        // still building the HW mipmaps, and the texture will never be ready)
        if( dst->_isDataReadyImpl() &&
            dst->getGpuPageOutStrategy() == GpuPageOutStrategy::AlwaysKeepSystemRamCopy )
        {
            dst->_syncGpuResidentToSystemRam();
        }
    }
    //-----------------------------------------------------------------------------------
    void VulkanTextureGpu::_setNextLayout( ResourceLayout::Layout layout )
    {
        OGRE_ASSERT_LOW( ( layout != ResourceLayout::CopySrc && layout != ResourceLayout::CopyDst &&
                           layout != ResourceLayout::CopyEnd ) &&
                         "CopySrc/Dst layouts are automanaged. "
                         "Cannot explicitly transition to these layouts" );
        mNextLayout = VulkanMappings::get( layout, this );
    }
    //-----------------------------------------------------------------------------------
    void VulkanTextureGpu::_autogenerateMipmaps( bool bUseBarrierSolver )
    {
        OGRE_ASSERT_LOW( allowsAutoMipmaps() );

        // TODO: Integrate FidelityFX Single Pass Downsampler - SPD
        //
        // https://gpuopen.com/fidelityfx-spd/
        // https://github.com/GPUOpen-Effects/FidelityFX-SPD
        VulkanTextureGpuManager *textureManager =
            static_cast<VulkanTextureGpuManager *>( mTextureManager );
        VulkanDevice *device = textureManager->getDevice();

        if( bUseBarrierSolver )
        {
            RenderSystem *renderSystem = textureManager->getRenderSystem();
            ResourceTransitionArray resourceTransitions;
            renderSystem->getBarrierSolver().resolveTransition(
                resourceTransitions, this, ResourceLayout::MipmapGen, ResourceAccess::ReadWrite, 0u );
            renderSystem->executeResourceTransition( resourceTransitions );
        }
        else
        {
            const bool callerIsCompositor = mCurrLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

            if( callerIsCompositor )
                device->mGraphicsQueue.getCopyEncoder( 0, 0, true );
            else
            {
                // We must transition to VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
                // By the time we exit _autogenerateMipmaps, the texture will
                // still be in VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, thus
                // endCopyEncoder will perform as expected
                device->mGraphicsQueue.getCopyEncoder( 0, this, true );
            }
        }

        const size_t numMipmaps = mNumMipmaps;
        const uint32 numSlices = getNumSlices();

        VkImageMemoryBarrier imageBarrier = getImageMemoryBarrier();

        imageBarrier.subresourceRange.levelCount = 1u;

        const uint32 internalWidth = getInternalWidth();
        const uint32 internalHeight = getInternalHeight();

        for( size_t i = 1u; i < numMipmaps; ++i )
        {
            // Convert the dst mipmap 'i' to TRANSFER_DST_OPTIMAL. Does not have to wait
            // on anything because previous barriers (compositor or getCopyEncoder)
            // have already waited
            imageBarrier.subresourceRange.baseMipLevel = static_cast<uint32_t>( i );
            imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageBarrier.srcAccessMask = 0;
            imageBarrier.dstAccessMask = 0;
            vkCmdPipelineBarrier( device->mGraphicsQueue.mCurrentCmdBuffer,
                                  VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                  0, 0u, 0, 0u, 0, 1u, &imageBarrier );

            VkImageBlit region;

            region.srcSubresource.aspectMask = VulkanMappings::getImageAspect( this->getPixelFormat() );
            region.srcSubresource.mipLevel = static_cast<uint32_t>( i - 1u );
            region.srcSubresource.baseArrayLayer = 0u;
            region.srcSubresource.layerCount = numSlices;

            region.srcOffsets[0].x = 0;
            region.srcOffsets[0].y = 0;
            region.srcOffsets[0].z = 0;

            region.srcOffsets[1].x = static_cast<int32_t>( std::max( internalWidth >> ( i - 1u ), 1u ) );
            region.srcOffsets[1].y =
                static_cast<int32_t>( std::max( internalHeight >> ( i - 1u ), 1u ) );
            region.srcOffsets[1].z = static_cast<int32_t>( std::max( getDepth() >> ( i - 1u ), 1u ) );

            region.dstSubresource.aspectMask = region.srcSubresource.aspectMask;
            region.dstSubresource.mipLevel = static_cast<uint32_t>( i );
            region.dstSubresource.baseArrayLayer = 0u;
            region.dstSubresource.layerCount = numSlices;

            region.dstOffsets[0].x = 0;
            region.dstOffsets[0].y = 0;
            region.dstOffsets[0].z = 0;

            region.dstOffsets[1].x = static_cast<int32_t>( std::max( internalWidth >> i, 1u ) );
            region.dstOffsets[1].y = static_cast<int32_t>( std::max( internalHeight >> i, 1u ) );
            region.dstOffsets[1].z = static_cast<int32_t>( std::max( getDepth() >> i, 1u ) );

            vkCmdBlitImage( device->mGraphicsQueue.mCurrentCmdBuffer, mFinalTextureName, mCurrLayout,
                            mFinalTextureName, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &region,
                            VK_FILTER_LINEAR );

            // Wait for vkCmdBlitImage on mip i to finish before advancing to mip i+1
            // Also transition src mip 'i' to TRANSFER_SRC_OPTIMAL
            imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            vkCmdPipelineBarrier( device->mGraphicsQueue.mCurrentCmdBuffer,
                                  VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0u,
                                  0, 0u, 0, 1u, &imageBarrier );
        }
    }
    //-----------------------------------------------------------------------------------
    VkImageSubresourceRange VulkanTextureGpu::getFullSubresourceRange( void ) const
    {
        VkImageSubresourceRange retVal;
        retVal.aspectMask = VulkanMappings::getImageAspect( getWorkaroundedPixelFormat( mPixelFormat ) );
        retVal.baseMipLevel = 0u;
        retVal.levelCount = VK_REMAINING_MIP_LEVELS;
        retVal.baseArrayLayer = 0u;
        retVal.layerCount = VK_REMAINING_ARRAY_LAYERS;
        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void VulkanTextureGpu::getSubsampleLocations( vector<Vector2>::type locations )
    {
        uint8 msaaCount = mSampleDescription.getMaxSamples();
        locations.reserve( msaaCount );
        for( size_t i = 0; i < msaaCount; ++i )
            locations.push_back( Vector2( 0, 0 ) );
    }
    //-----------------------------------------------------------------------------------
    VkImageType VulkanTextureGpu::getVulkanTextureType( void ) const
    {
        // clang-format off
        switch( mTextureType )
        {
        case TextureTypes::Unknown:         return VK_IMAGE_TYPE_2D;
        case TextureTypes::Type1D:          return VK_IMAGE_TYPE_1D;
        case TextureTypes::Type1DArray:     return VK_IMAGE_TYPE_1D;
        case TextureTypes::Type2D:          return VK_IMAGE_TYPE_2D;
        case TextureTypes::Type2DArray:     return VK_IMAGE_TYPE_2D;
        case TextureTypes::TypeCube:        return VK_IMAGE_TYPE_2D;
        case TextureTypes::TypeCubeArray:   return VK_IMAGE_TYPE_2D;
        case TextureTypes::Type3D:          return VK_IMAGE_TYPE_3D;
        }
        // clang-format on

        return VK_IMAGE_TYPE_2D;
    }
    //-----------------------------------------------------------------------------------
    VkImageViewType VulkanTextureGpu::getInternalVulkanTextureViewType( void ) const
    {
        // clang-format off
        switch( getInternalTextureType() )
        {
        case TextureTypes::Unknown:         return VK_IMAGE_VIEW_TYPE_2D;
        case TextureTypes::Type1D:          return VK_IMAGE_VIEW_TYPE_1D;
        case TextureTypes::Type1DArray:     return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
        case TextureTypes::Type2D:          return VK_IMAGE_VIEW_TYPE_2D;
        case TextureTypes::Type2DArray:     return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        case TextureTypes::TypeCube:        return VK_IMAGE_VIEW_TYPE_CUBE;
        case TextureTypes::TypeCubeArray:   return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
        case TextureTypes::Type3D:          return VK_IMAGE_VIEW_TYPE_3D;
        }
        // clang-format on

        return VK_IMAGE_VIEW_TYPE_2D;
    }
    //-----------------------------------------------------------------------------------
    VkImageView VulkanTextureGpu::_createView( PixelFormatGpu pixelFormat, uint8 mipLevel,
                                               uint8 numMipmaps, uint16 arraySlice,
                                               bool cubemapsAs2DArrays, bool forUav, uint32 numSlices,
                                               VkImage imageOverride ) const
    {
        OGRE_ASSERT_LOW( ( forUav || imageOverride || isTexture() ) &&
                         "This texture is marked as 'TextureFlags::NotTexture', which "
                         "means it can't be used for reading as a regular texture." );

        if( pixelFormat == PFG_UNKNOWN )
        {
            pixelFormat = mPixelFormat;
            if( forUav )
                pixelFormat = PixelFormatGpuUtils::getEquivalentLinear( pixelFormat );
        }

        pixelFormat = getWorkaroundedPixelFormat( pixelFormat );

        VkImageViewType texType = this->getInternalVulkanTextureViewType();

        if( ( cubemapsAs2DArrays || forUav || numSlices == 1u ) &&
            ( mTextureType == TextureTypes::TypeCube || mTextureType == TextureTypes::TypeCubeArray ) )
        {
            texType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        }

        if( !numMipmaps )
            numMipmaps = mNumMipmaps - mipLevel;

        OGRE_ASSERT_LOW( numMipmaps <= mNumMipmaps - mipLevel &&
                         "Asking for more mipmaps than the texture has!" );

        VulkanTextureGpuManager *textureManager =
            static_cast<VulkanTextureGpuManager *>( mTextureManager );
        VulkanDevice *device = textureManager->getDevice();

        VkImageViewCreateInfo imageViewCi;
        makeVkStruct( imageViewCi, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO );
        imageViewCi.image = imageOverride ? imageOverride : mDisplayTextureName;
        imageViewCi.viewType = texType;
        imageViewCi.format = VulkanMappings::get( pixelFormat );

        // Using both depth & stencil aspects in an image view for texture sampling is illegal
        // Thus prefer depth over stencil. We only use both flags for FBOs
        imageViewCi.subresourceRange.aspectMask =
            VulkanMappings::getImageAspect( pixelFormat, imageOverride == 0 );
        imageViewCi.subresourceRange.baseMipLevel = mipLevel;
        imageViewCi.subresourceRange.levelCount = numMipmaps;
        imageViewCi.subresourceRange.baseArrayLayer = arraySlice;
        if( numSlices == 0u )
            imageViewCi.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
        else
            imageViewCi.subresourceRange.layerCount = numSlices;

        VkImageViewUsageCreateInfo flagRestriction;
        if( textureManager->canRestrictImageViewUsage() && isUav() && !forUav )
        {
            // Some formats (e.g. *_SRGB formats) do not support USAGE_STORAGE_BIT at all
            // Thus we need to mark when this view won't be using that bit.
            //
            // If VK_KHR_maintenance2 is not available then we cross our fingers
            // and hope the driver doesn't stop us from doing it (it should work)
            //
            // The validation layers will complain though. This was a major Vulkan oversight.
            makeVkStruct( flagRestriction, VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO );
            imageViewCi.pNext = &flagRestriction;
            flagRestriction.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            if( isTexture() )
                flagRestriction.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
            if( isRenderToTexture() )
            {
                flagRestriction.usage |= PixelFormatGpuUtils::isDepth( pixelFormat )
                                             ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
                                             : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            }
        }

        VkImageView imageView;
        VkResult result = vkCreateImageView( device->mDevice, &imageViewCi, 0, &imageView );
        checkVkResult( result, "vkCreateImageView" );

        return imageView;
    }
    //-----------------------------------------------------------------------------------
    void VulkanTextureGpu::destroyView( VkImageView imageView )
    {
        VulkanTextureGpuManager *textureManager =
            static_cast<VulkanTextureGpuManager *>( mTextureManager );
        VulkanDevice *device = textureManager->getDevice();

        delayed_vkDestroyImageView( textureManager->getVaoManager(), device->mDevice, imageView, 0 );
    }
    //-----------------------------------------------------------------------------------
    void VulkanTextureGpu::destroyView( DescriptorSetTexture2::TextureSlot texSlot,
                                        VkImageView imageView )
    {
        VulkanTextureGpuManager *textureManager =
            static_cast<VulkanTextureGpuManager *>( mTextureManager );
        textureManager->destroyView( texSlot, imageView );
    }
    //-----------------------------------------------------------------------------------
    void VulkanTextureGpu::destroyView( DescriptorSetUav::TextureSlot texSlot, VkImageView imageView )
    {
        VulkanTextureGpuManager *textureManager =
            static_cast<VulkanTextureGpuManager *>( mTextureManager );
        textureManager->destroyView( texSlot, imageView );
    }
    //-----------------------------------------------------------------------------------
    VkImageView VulkanTextureGpu::createView( void ) const
    {
        OGRE_ASSERT_MEDIUM( isTexture() &&
                            "This texture is marked as 'TextureFlags::NotTexture', which "
                            "means it can't be used for reading as a regular texture." );
        OGRE_ASSERT_MEDIUM( mDefaultDisplaySrv &&
                            "Either the texture wasn't properly loaded or _setToDisplayDummyTexture "
                            "wasn't called when it should have been" );
        return mDefaultDisplaySrv;
    }
    //-----------------------------------------------------------------------------------
    VkImageView VulkanTextureGpu::createView( const DescriptorSetTexture2::TextureSlot &texSlot,
                                              bool bUseCache ) const
    {
        if( bUseCache )
        {
            VulkanTextureGpuManager *textureManager =
                static_cast<VulkanTextureGpuManager *>( mTextureManager );
            return textureManager->createView( texSlot );
        }
        else
        {
            return _createView( texSlot.pixelFormat, texSlot.mipmapLevel, texSlot.numMipmaps,
                                texSlot.textureArrayIndex, texSlot.cubemapsAs2DArrays, false );
        }
    }
    //-----------------------------------------------------------------------------------
    VkImageView VulkanTextureGpu::createView( DescriptorSetUav::TextureSlot texSlot, bool bUseCache )
    {
        if( bUseCache )
        {
            VulkanTextureGpuManager *textureManager =
                static_cast<VulkanTextureGpuManager *>( mTextureManager );
            return textureManager->createView( texSlot );
        }
        else
        {
            return _createView( texSlot.pixelFormat, texSlot.mipmapLevel, 1u, texSlot.textureArrayIndex,
                                false, true );
        }
    }
    //-----------------------------------------------------------------------------------
    VkImageMemoryBarrier VulkanTextureGpu::getImageMemoryBarrier( void ) const
    {
        VkImageMemoryBarrier imageMemBarrier;
        makeVkStruct( imageMemBarrier, VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER );
        imageMemBarrier.image = mFinalTextureName;
        imageMemBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemBarrier.subresourceRange.aspectMask =
            VulkanMappings::getImageAspect( getWorkaroundedPixelFormat( mPixelFormat ) );
        imageMemBarrier.subresourceRange.baseMipLevel = 0u;
        imageMemBarrier.subresourceRange.levelCount = mNumMipmaps;
        imageMemBarrier.subresourceRange.baseArrayLayer = mInternalSliceStart;
        imageMemBarrier.subresourceRange.layerCount = getNumSlices();
        return imageMemBarrier;
    }
    //-----------------------------------------------------------------------------------
    VulkanTextureGpuRenderTarget::VulkanTextureGpuRenderTarget(
        GpuPageOutStrategy::GpuPageOutStrategy pageOutStrategy, VaoManager *vaoManager, IdString name,
        uint32 textureFlags, TextureTypes::TextureTypes initialType,
        TextureGpuManager *textureManager ) :
        VulkanTextureGpu( pageOutStrategy, vaoManager, name, textureFlags, initialType, textureManager ),
        mMsaaVboPoolIdx( 0u ),
        mMsaaInternalBufferStart( 0u ),
        mDepthBufferPoolId( 1u ),
        mPreferDepthTexture( false ),
        mDesiredDepthBufferFormat( PFG_UNKNOWN )
#if OGRE_NO_VIEWPORT_ORIENTATIONMODE == 0
        ,
        mOrientationMode( msDefaultOrientationMode )
#endif
    {
    }
    //-----------------------------------------------------------------------------------
    void VulkanTextureGpuRenderTarget::_setDepthBufferDefaults( uint16 depthBufferPoolId,
                                                                bool preferDepthTexture,
                                                                PixelFormatGpu desiredDepthBufferFormat )
    {
        assert( isRenderToTexture() );
        mDepthBufferPoolId = depthBufferPoolId;
        mPreferDepthTexture = preferDepthTexture;
        mDesiredDepthBufferFormat = desiredDepthBufferFormat;
    }
    //-----------------------------------------------------------------------------------
    uint16 VulkanTextureGpuRenderTarget::getDepthBufferPoolId( void ) const
    {
        return mDepthBufferPoolId;
    }
    //-----------------------------------------------------------------------------------
    bool VulkanTextureGpuRenderTarget::getPreferDepthTexture( void ) const
    {
        return mPreferDepthTexture;
    }
    //-----------------------------------------------------------------------------------
    PixelFormatGpu VulkanTextureGpuRenderTarget::getDesiredDepthBufferFormat( void ) const
    {
        return mDesiredDepthBufferFormat;
    }
    //-----------------------------------------------------------------------------------
    void VulkanTextureGpuRenderTarget::createMsaaSurface( void )
    {
        const PixelFormatGpu finalPixelFormat = getWorkaroundedPixelFormat( mPixelFormat );

        VkImageCreateInfo imageInfo;
        makeVkStruct( imageInfo, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO );
        imageInfo.imageType = getVulkanTextureType();
        imageInfo.extent.width = getInternalWidth();
        imageInfo.extent.height = getInternalHeight();
        imageInfo.extent.depth = getDepth();
        imageInfo.mipLevels = 1u;
        imageInfo.arrayLayers = 1u;
        imageInfo.format = VulkanMappings::get( finalPixelFormat );
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = static_cast<VkSampleCountFlagBits>( mSampleDescription.getColourSamples() );
        imageInfo.flags = 0;
        imageInfo.usage |= PixelFormatGpuUtils::isDepth( finalPixelFormat )
                               ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
                               : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        String textureName = getNameStr() + "/MsaaImplicit";

        VulkanTextureGpuManager *textureManager =
            static_cast<VulkanTextureGpuManager *>( mTextureManager );
        VulkanDevice *device = textureManager->getDevice();

        VkResult imageResult = vkCreateImage( device->mDevice, &imageInfo, 0, &mMsaaFramebufferName );
        checkVkResult( imageResult, "vkCreateImage" );

        setObjectName( device->mDevice, (uint64_t)mMsaaFramebufferName,
                       VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, textureName.c_str() );

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements( device->mDevice, mMsaaFramebufferName, &memRequirements );

        VulkanVaoManager *vaoManager =
            static_cast<VulkanVaoManager *>( textureManager->getVaoManager() );
        VkDeviceMemory deviceMemory =
            vaoManager->allocateTexture( memRequirements, mMsaaVboPoolIdx, mMsaaInternalBufferStart );

        VkResult result = vkBindImageMemory( device->mDevice, mMsaaFramebufferName, deviceMemory,
                                             mMsaaInternalBufferStart );
        checkVkResult( result, "vkBindImageMemory" );

        // Immediately transition to its only state
        VkImageMemoryBarrier imageBarrier = this->getImageMemoryBarrier();
        imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        imageBarrier.image = mMsaaFramebufferName;
        vkCmdPipelineBarrier( device->mGraphicsQueue.mCurrentCmdBuffer,
                              VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0,
                              0u, 0, 0u, 0, 1u, &imageBarrier );
    }
    //-----------------------------------------------------------------------------------
    void VulkanTextureGpuRenderTarget::destroyMsaaSurface( void )
    {
        if( mMsaaFramebufferName )
        {
            VulkanTextureGpuManager *textureManager =
                static_cast<VulkanTextureGpuManager *>( mTextureManager );
            VulkanDevice *device = textureManager->getDevice();

            VkMemoryRequirements memRequirements;
            vkGetImageMemoryRequirements( device->mDevice, mMsaaFramebufferName, &memRequirements );

            VulkanVaoManager *vaoManager =
                static_cast<VulkanVaoManager *>( textureManager->getVaoManager() );

            delayed_vkDestroyImage( vaoManager, device->mDevice, mMsaaFramebufferName, 0 );
            mMsaaFramebufferName = 0;

            vaoManager->deallocateTexture( mMsaaVboPoolIdx, mMsaaInternalBufferStart,
                                           memRequirements.size );
        }
    }
    //-----------------------------------------------------------------------------------
    void VulkanTextureGpuRenderTarget::setOrientationMode( OrientationMode orientationMode )
    {
        OGRE_ASSERT_LOW( mResidencyStatus == GpuResidency::OnStorage || isRenderWindowSpecific() );
#if OGRE_NO_VIEWPORT_ORIENTATIONMODE == 0
        mOrientationMode = orientationMode;
#endif
    }
    //-----------------------------------------------------------------------------------
#if OGRE_NO_VIEWPORT_ORIENTATIONMODE == 0
    OrientationMode VulkanTextureGpuRenderTarget::getOrientationMode( void ) const
    {
        return mOrientationMode;
    }
#endif
}  // namespace Ogre
