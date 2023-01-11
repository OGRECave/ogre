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

#include "OgreException.h"
#include "OgreVector.h"
#include "OgreVulkanMappings.h"
#include "OgreVulkanTextureGpuManager.h"
#include "OgreVulkanUtils.h"
#include "OgreVulkanHardwareBuffer.h"
#include "OgreBitwise.h"
#include "OgreRoot.h"
#include "OgreVulkanTextureGpuWindow.h"

#define TODO_add_resource_transitions

namespace Ogre
{
    VulkanHardwarePixelBuffer::VulkanHardwarePixelBuffer(VulkanTextureGpu* tex, uint32 width, uint32 height, uint32 depth,
                                                        uint8 face, uint32 mip)
        : HardwarePixelBuffer(width, height, depth, tex->getFormat(), tex->getUsage(), false, false), mParent(tex),
        mFace(face), mLevel(mip)
    {
        if(mParent->getUsage() & TU_RENDERTARGET)
        {
            // Create render target for each slice
            mSliceTRT.reserve(mDepth);
            for(size_t zoffset=0; zoffset<mDepth; ++zoffset)
            {
                String name;
                name = "rtt/"+StringConverter::toString((size_t)this) + "/" + mParent->getName();

                RenderTexture *trt = new VulkanRenderTexture(name, this, zoffset, mParent, mFace);
                mSliceTRT.push_back(trt);
                Root::getSingleton().getRenderSystem()->attachRenderTarget(*trt);
            }
        }
    }

    PixelBox VulkanHardwarePixelBuffer::lockImpl(const Box &lockBox,  LockOptions options)
    {
        PixelBox ret(lockBox, mParent->getFormat());

        auto textureManager = static_cast<VulkanTextureGpuManager*>(mParent->getCreator());
        VulkanDevice* device = textureManager->getDevice();

        uint32 target = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        if(options != HBL_DISCARD && options != HBL_WRITE_ONLY)
            target |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        mStagingBuffer.reset(new VulkanHardwareBuffer(target, ret.getConsecutiveSize(), HBU_CPU_ONLY, false, device));

        if (options != HBL_DISCARD && options != HBL_WRITE_ONLY)
        {
            device->mGraphicsQueue.getCopyEncoder(0, mParent, true);

            VkBuffer dstBuffer = mStagingBuffer->getVkBuffer();

            VkBufferImageCopy region;
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;

            region.imageSubresource.aspectMask = VulkanMappings::getImageAspect(mFormat);;
            region.imageSubresource.mipLevel = mLevel;
            region.imageSubresource.baseArrayLayer = mFace;
            region.imageSubresource.layerCount = 1;

            region.imageOffset.x = lockBox.left;
            region.imageOffset.y = lockBox.top;
            region.imageOffset.z = lockBox.front;
            region.imageExtent.width = lockBox.getWidth();
            region.imageExtent.height = lockBox.getHeight();
            region.imageExtent.depth = lockBox.getDepth();

            vkCmdCopyImageToBuffer(device->mGraphicsQueue.mCurrentCmdBuffer, mParent->getFinalTextureName(),
                                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstBuffer, 1u, &region);
            device->mGraphicsQueue.commitAndNextCommandBuffer();
        }

        return PixelBox(lockBox, mParent->getFormat(), mStagingBuffer->lock(options));
    }

    void VulkanHardwarePixelBuffer::unlockImpl()
    {
        mStagingBuffer->unlock();
        VkBufferImageCopy region;
        if (mCurrentLockOptions != HBL_READ_ONLY)
        {
            auto textureManager = static_cast<VulkanTextureGpuManager*>(mParent->getCreator());
            VulkanDevice* device = textureManager->getDevice();
            device->mGraphicsQueue.getCopyEncoder(0, mParent, false);

            VkBuffer srcBuffer = mStagingBuffer->getVkBuffer();

            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;

            region.imageSubresource.aspectMask = VulkanMappings::getImageAspect(mFormat);
            region.imageSubresource.mipLevel = mLevel;
            region.imageSubresource.baseArrayLayer = mFace;
            region.imageSubresource.layerCount = 1;

            region.imageOffset.x = mCurrentLock.left;
            region.imageOffset.y = mCurrentLock.top;
            region.imageOffset.z = mCurrentLock.front;
            region.imageExtent.width = mCurrentLock.getWidth();
            region.imageExtent.height = mCurrentLock.getHeight();
            region.imageExtent.depth = mCurrentLock.getDepth();

            if (mParent->getTextureType() == TEX_TYPE_2D_ARRAY)
            {
                region.imageSubresource.baseArrayLayer = mCurrentLock.front;
                region.imageOffset.z = 0;
            }

            vkCmdCopyBufferToImage(device->mGraphicsQueue.mCurrentCmdBuffer, srcBuffer, mParent->getFinalTextureName(),
                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &region);
        }

        bool finalSlice = region.imageSubresource.baseArrayLayer == mParent->getNumLayers() - 1;
        if((mParent->getUsage() & TU_AUTOMIPMAP) && finalSlice)
            mParent->_autogenerateMipmaps();
        mStagingBuffer.reset();
    }

    void VulkanHardwarePixelBuffer::blitFromMemory(const PixelBox& src, const Box& dstBox)
    {
        OgreAssert(src.getSize() == dstBox.getSize(), "scaling currently not supported");
        // convert to image native format if necessary
        if(src.format != mFormat)
        {
            std::vector<uint8> buffer;
            buffer.resize(PixelUtil::getMemorySize(src.getWidth(), src.getHeight(), src.getDepth(), mFormat));
            PixelBox converted = PixelBox(src.getWidth(), src.getHeight(), src.getDepth(), mFormat, buffer.data());
            PixelUtil::bulkPixelConversion(src, converted);
            blitFromMemory(converted, dstBox); // recursive call
            return;
        }

        auto ptr = lock(dstBox, HBL_WRITE_ONLY).data;

        memcpy(ptr, src.data, src.getConsecutiveSize());

        unlock();
    }
    void VulkanHardwarePixelBuffer::blitToMemory(const Box& srcBox, const PixelBox& dst)
    {
        OgreAssert(srcBox.getSize() == dst.getSize(), "scaling currently not supported");
        auto src = lock(srcBox, HBL_READ_ONLY);
        PixelUtil::bulkPixelConversion(src, dst);
        unlock();
    }

    VulkanTextureGpu::VulkanTextureGpu(TextureManager* textureManager, const String& name, ResourceHandle handle,
                         const String& group, bool isManual, ManualResourceLoader* loader) :
        Texture(textureManager, name, handle, group, isManual, loader ),
        mDefaultDisplaySrv( 0 ),
        mDisplayTextureName( 0 ),
        mFinalTextureName( 0 ),
        mAllocation(VK_NULL_HANDLE),
        mMsaaTextureName( 0 ),
        mMsaaAllocation(VK_NULL_HANDLE),
        mCurrLayout( VK_IMAGE_LAYOUT_UNDEFINED ),
        mNextLayout( VK_IMAGE_LAYOUT_UNDEFINED )
    {

    }
    //-----------------------------------------------------------------------------------
    VulkanTextureGpu::~VulkanTextureGpu() { unload(); }
    //-----------------------------------------------------------------------------------
    void VulkanTextureGpu::createInternalResourcesImpl( void )
    {
        if( mFormat == PF_UNKNOWN )
            return;  // Nothing to do

        // Adjust format if required.
        mFormat = TextureManager::getSingleton().getNativeFormat(mTextureType, mFormat, mUsage);

        VkImageCreateInfo imageInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
        imageInfo.imageType = getVulkanTextureType();
        imageInfo.extent.width = getWidth();
        imageInfo.extent.height = getHeight();
        imageInfo.extent.depth = getDepth();
        imageInfo.mipLevels = mNumMipmaps + 1;
        imageInfo.arrayLayers = getNumFaces();
        imageInfo.flags = 0;
        imageInfo.format = VulkanMappings::get( mFormat, mHwGamma );
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if(mTextureType == TEX_TYPE_2D_ARRAY)
            std::swap(imageInfo.extent.depth, imageInfo.arrayLayers);

        if( hasMsaaExplicitResolves() )
        {
            imageInfo.samples = VkSampleCountFlagBits(std::max(mFSAA, 1u));
        }
        else
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

        if( mTextureType == TEX_TYPE_CUBE_MAP /*|| mTextureType == TextureTypes::TypeCubeArray*/ )
            imageInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

        imageInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;

        if (PixelUtil::isDepth(mFormat))
            imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        else if(mUsage & TU_RENDERTARGET)
            imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        if( isUav() )
            imageInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;

        VulkanDevice* device = static_cast<VulkanTextureGpuManager*>(mCreator)->getDevice();

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        OGRE_VK_CHECK(vmaCreateImage(device->getAllocator(), &imageInfo, &allocInfo, &mFinalTextureName, &mAllocation, 0));

        String textureName = getName();
        setObjectName( device->mDevice, (uint64_t)mFinalTextureName,
                       VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, textureName.c_str() );

        OgreAssert(device->mGraphicsQueue.getEncoderState() != VulkanQueue::EncoderGraphicsOpen,
                   "interrupting RenderPass not supported");
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

        if( PixelUtil::isDepth( mFormat ) )
            imageBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        vkCmdPipelineBarrier( device->mGraphicsQueue.mCurrentCmdBuffer,
                                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                0, 0u, 0, 0u, 0, 1u, &imageBarrier );

        mCurrLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        mNextLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // create
        uint32 depth = mDepth;
        for (uint8 face = 0; face < getNumFaces(); face++)
        {
            uint32 width = mWidth;
            uint32 height = mHeight;

            for (uint32 mip = 0; mip <= getNumMipmaps(); mip++)
            {
                auto buf = std::make_shared<VulkanHardwarePixelBuffer>(this, width, height, depth, face, mip);
                mSurfaceList.push_back(buf);
                if (width > 1)
                    width = width / 2;
                if (height > 1)
                    height = height / 2;
                if (depth > 1 && mTextureType != TEX_TYPE_2D_ARRAY)
                    depth = depth / 2;
            }
        }

        mDefaultDisplaySrv = _createView(0, 0, 0, getNumLayers());

        if( mFSAA > 1 && !hasMsaaExplicitResolves() )
            createMsaaSurface();
    }
    //-----------------------------------------------------------------------------------
    void VulkanTextureGpu::freeInternalResourcesImpl( void )
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
        auto textureManager = static_cast<VulkanTextureGpuManager*>(mCreator);
        VulkanDevice *device = textureManager->getDevice();
        device->mGraphicsQueue.notifyTextureDestroyed( this );

        vkDestroyImageView(device->mDevice, mDefaultDisplaySrv, 0);
        mDefaultDisplaySrv = 0;

        vmaDestroyImage(device->getAllocator(), mFinalTextureName, mAllocation);

        destroyMsaaSurface();

        mCurrLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        mNextLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    }
    //-----------------------------------------------------------------------------------
    void VulkanTextureGpu::copyTo( TextureGpu *dst, const PixelBox &dstBox, uint8 dstMipLevel,
                                   const PixelBox &srcBox, uint8 srcMipLevel,
                                   bool keepResolvedTexSynced,
                                   ResourceAccess::ResourceAccess issueBarriers )
    {
        //TextureGpu::copyTo( dst, dstBox, dstMipLevel, srcBox, srcMipLevel, issueBarriers );

        OGRE_ASSERT_HIGH( dynamic_cast<VulkanTextureGpu *>( dst ) );

        VulkanTextureGpu *dstTexture = static_cast<VulkanTextureGpu *>( dst );
        VulkanTextureGpuManager *textureManager =
            static_cast<VulkanTextureGpuManager *>( mCreator );
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

        const uint32 sourceSlice = srcBox.front;// + getInternalSliceStart();
        const uint32 destinationSlice = dstBox.front;// + dstTexture->getInternalSliceStart();
        const uint32 numSlices = dstBox.getDepth() != 0 ? dstBox.getDepth() : dstTexture->getDepth();

        region.srcSubresource.aspectMask = VulkanMappings::getImageAspect( this->getFormat() );
        region.srcSubresource.mipLevel = srcMipLevel;
        region.srcSubresource.baseArrayLayer = sourceSlice;
        region.srcSubresource.layerCount = numSlices;

        region.srcOffset.x = static_cast<int32_t>( srcBox.left );
        region.srcOffset.y = static_cast<int32_t>( srcBox.top );
        region.srcOffset.z = static_cast<int32_t>( srcBox.front );

        region.dstSubresource.aspectMask = VulkanMappings::getImageAspect( dst->getFormat() );
        region.dstSubresource.mipLevel = dstMipLevel;
        region.dstSubresource.baseArrayLayer = destinationSlice;
        region.dstSubresource.layerCount = numSlices;

        region.dstOffset.x = dstBox.left;
        region.dstOffset.y = dstBox.top;
        region.dstOffset.z = dstBox.front;

        region.extent.width = srcBox.getWidth();
        region.extent.height = srcBox.getHeight();
        region.extent.depth = srcBox.getDepth();

        VkImage srcTextureName = this->mFinalTextureName;
        VkImage dstTextureName = dstTexture->mFinalTextureName;

        if( this->isMultisample() && !this->hasMsaaExplicitResolves() )
            srcTextureName = this->mMsaaTextureName;
        if( dstTexture->isMultisample() && !dstTexture->hasMsaaExplicitResolves() )
            dstTextureName = dstTexture->mMsaaTextureName;

        vkCmdCopyImage( device->mGraphicsQueue.mCurrentCmdBuffer, srcTextureName, mCurrLayout,
                        dstTextureName, dstTexture->mCurrLayout, 1u, &region );

        if( dstTexture->isMultisample() && !dstTexture->hasMsaaExplicitResolves() &&
            keepResolvedTexSynced )
        {
            TODO_add_resource_transitions;  // We must add res. transitions and then restore them

            // Must keep the resolved texture up to date.
            VkImageResolve resolve = {};
            resolve.srcSubresource = region.dstSubresource;
            resolve.dstSubresource = region.dstSubresource;
            resolve.extent.width = getWidth();
            resolve.extent.height = getHeight();
            resolve.extent.depth = getDepth();

            vkCmdResolveImage( device->mGraphicsQueue.mCurrentCmdBuffer,
                               dstTexture->mMsaaTextureName, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                               dstTexture->mFinalTextureName, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u,
                               &resolve );
        }

        // Do not perform the sync if notifyDataIsReady hasn't been called yet (i.e. we're
        // still building the HW mipmaps, and the texture will never be ready)
        /*if( dst->_isDataReadyImpl() &&
            dst->getGpuPageOutStrategy() == GpuPageOutStrategy::AlwaysKeepSystemRamCopy )
        {
            dst->_syncGpuResidentToSystemRam();
        }*/
    }
    //-----------------------------------------------------------------------------------
    void VulkanTextureGpu::_autogenerateMipmaps( bool bUseBarrierSolver )
    {
        // TODO: Integrate FidelityFX Single Pass Downsampler - SPD
        //
        // https://gpuopen.com/fidelityfx-spd/
        // https://github.com/GPUOpen-Effects/FidelityFX-SPD
        VulkanTextureGpuManager *textureManager =
            static_cast<VulkanTextureGpuManager *>( mCreator );
        VulkanDevice *device = textureManager->getDevice();

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

        const uint32 numSlices = getNumLayers();

        VkImageMemoryBarrier imageBarrier = getImageMemoryBarrier();

        imageBarrier.subresourceRange.levelCount = 1u;

        const uint32 internalWidth = getWidth();
        const uint32 internalHeight = getHeight();

        for( size_t i = 1u; i <= mNumMipmaps; ++i )
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

            region.srcSubresource.aspectMask = VulkanMappings::getImageAspect( this->getFormat() );
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

            if(mTextureType == TEX_TYPE_2D_ARRAY)
            {
                region.srcOffsets[1].z = 1;
                region.dstOffsets[1].z = 1;
            }

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
    VkImageType VulkanTextureGpu::getVulkanTextureType( void ) const
    {
        // clang-format off
        switch( mTextureType )
        {
        case TEX_TYPE_1D:          return VK_IMAGE_TYPE_1D;
        case TEX_TYPE_2D:          return VK_IMAGE_TYPE_2D;
        case TEX_TYPE_2D_ARRAY:    return VK_IMAGE_TYPE_2D;
        case TEX_TYPE_CUBE_MAP:    return VK_IMAGE_TYPE_2D;
        case TEX_TYPE_3D:          return VK_IMAGE_TYPE_3D;
        case TEX_TYPE_EXTERNAL_OES: break;
        }
        // clang-format on

        return VK_IMAGE_TYPE_2D;
    }
    //-----------------------------------------------------------------------------------
    VkImageViewType VulkanTextureGpu::getInternalVulkanTextureViewType( void ) const
    {
        // clang-format off
        switch( getTextureType() )
        {
        case TEX_TYPE_1D:          return VK_IMAGE_VIEW_TYPE_1D;
        case TEX_TYPE_2D:          return VK_IMAGE_VIEW_TYPE_2D;
        case TEX_TYPE_2D_ARRAY:    return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        case TEX_TYPE_CUBE_MAP:    return VK_IMAGE_VIEW_TYPE_CUBE;
        case TEX_TYPE_3D:          return VK_IMAGE_VIEW_TYPE_3D;
        case TEX_TYPE_EXTERNAL_OES: break;
        }
        // clang-format on

        return VK_IMAGE_VIEW_TYPE_2D;
    }
    //-----------------------------------------------------------------------------------
    VkImageView VulkanTextureGpu::_createView( uint8 mipLevel,
                                               uint8 numMipmaps, uint16 arraySlice,
                                               uint32 numSlices,
                                               VkImage imageOverride ) const
    {
        VkImageViewType texType = this->getInternalVulkanTextureViewType();

        if (numSlices == 1u && mTextureType == TEX_TYPE_CUBE_MAP)
        {
            texType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        }

        if( !numMipmaps )
            numMipmaps = mNumMipmaps - mipLevel + 1;

        OGRE_ASSERT_LOW( numMipmaps <= (mNumMipmaps - mipLevel + 1) &&
                         "Asking for more mipmaps than the texture has!" );

        auto textureManager = static_cast<VulkanTextureGpuManager*>(TextureManager::getSingletonPtr());
        VulkanDevice *device = textureManager->getDevice();

        VkImageViewCreateInfo imageViewCi = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        imageViewCi.image = imageOverride ? imageOverride : mFinalTextureName;
        imageViewCi.viewType = texType;
        imageViewCi.format = VulkanMappings::get( mFormat, mHwGamma );

        if (PixelUtil::isLuminance(mFormat) && !PixelUtil::isDepth(mFormat))
        {
            if (PixelUtil::getComponentCount(mFormat) == 2)
            {
                imageViewCi.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_R,
                                          VK_COMPONENT_SWIZZLE_G};
            }
            else
            {
                imageViewCi.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_R,
                                          VK_COMPONENT_SWIZZLE_ONE};
            }
        }
        else if (mFormat == PF_A8)
        {
            imageViewCi.components = {VK_COMPONENT_SWIZZLE_ONE, VK_COMPONENT_SWIZZLE_ONE, VK_COMPONENT_SWIZZLE_ONE,
                                      VK_COMPONENT_SWIZZLE_R};
        }

        // Using both depth & stencil aspects in an image view for texture sampling is illegal
        // Thus prefer depth over stencil. We only use both flags for FBOs
        imageViewCi.subresourceRange.aspectMask = VulkanMappings::getImageAspect(mFormat, imageOverride == 0);
        imageViewCi.subresourceRange.baseMipLevel = mipLevel;
        imageViewCi.subresourceRange.levelCount = numMipmaps;
        imageViewCi.subresourceRange.baseArrayLayer = arraySlice;
        if( numSlices == 0u )
            imageViewCi.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
        else
            imageViewCi.subresourceRange.layerCount = numSlices;

        VkImageViewUsageCreateInfo flagRestriction = {VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO};
        if( textureManager->canRestrictImageViewUsage() && isUav() )
        {
            // Some formats (e.g. *_SRGB formats) do not support USAGE_STORAGE_BIT at all
            // Thus we need to mark when this view won't be using that bit.
            //
            // If VK_KHR_maintenance2 is not available then we cross our fingers
            // and hope the driver doesn't stop us from doing it (it should work)
            //
            // The validation layers will complain though. This was a major Vulkan oversight.
            imageViewCi.pNext = &flagRestriction;
            flagRestriction.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            flagRestriction.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
            if (mUsage & TU_RENDERTARGET)
            {
                flagRestriction.usage |= PixelUtil::isDepth(mFormat) ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
                                                                     : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            }
        }

        VkImageView imageView;
        OGRE_VK_CHECK(vkCreateImageView( device->mDevice, &imageViewCi, 0, &imageView ));
        return imageView;
    }
    //-----------------------------------------------------------------------------------
    void VulkanTextureGpu::destroyView( VkImageView imageView )
    {
        //VulkanTextureGpuManager *textureManager =
        //    static_cast<VulkanTextureGpuManager *>( mCreator );
        //VulkanDevice *device = textureManager->getDevice();

        //delayed_vkDestroyImageView( textureManager->getVaoManager(), device->mDevice, imageView, 0 );
    }
    //-----------------------------------------------------------------------------------
    VkImageView VulkanTextureGpu::createView( void ) const
    {
        OGRE_ASSERT_MEDIUM( mDefaultDisplaySrv &&
                            "Either the texture wasn't properly loaded or _setToDisplayDummyTexture "
                            "wasn't called when it should have been" );
        return mDefaultDisplaySrv;
    }
    //-----------------------------------------------------------------------------------
    VkImageMemoryBarrier VulkanTextureGpu::getImageMemoryBarrier( void ) const
    {
        VkImageMemoryBarrier imageMemBarrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        imageMemBarrier.image = mFinalTextureName;
        imageMemBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemBarrier.subresourceRange.aspectMask = VulkanMappings::getImageAspect(mFormat);
        imageMemBarrier.subresourceRange.baseMipLevel = 0u;
        imageMemBarrier.subresourceRange.levelCount = mNumMipmaps + 1;
        imageMemBarrier.subresourceRange.baseArrayLayer = 0;
        imageMemBarrier.subresourceRange.layerCount = getNumLayers();
        return imageMemBarrier;
    }
    //-----------------------------------------------------------------------------------
    void VulkanTextureGpu::createMsaaSurface( void )
    {
        VkImageCreateInfo imageInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
        imageInfo.imageType = getVulkanTextureType();
        imageInfo.extent.width = getWidth();
        imageInfo.extent.height = getHeight();
        imageInfo.extent.depth = getDepth();
        imageInfo.mipLevels = 1u;
        imageInfo.arrayLayers = 1u;
        imageInfo.format = VulkanMappings::get( mFormat );
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VkSampleCountFlagBits( mFSAA );
        imageInfo.flags = 0;
        imageInfo.usage |= PixelUtil::isDepth( mFormat )
                               ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
                               : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        VulkanDevice* device = static_cast<VulkanTextureGpuManager*>(mCreator)->getDevice();

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        OGRE_VK_CHECK(
            vmaCreateImage(device->getAllocator(), &imageInfo, &allocInfo, &mMsaaTextureName, &mMsaaAllocation, 0));

        String textureName = getName() + "/MsaaImplicit";
        setObjectName(device->mDevice, (uint64_t)mMsaaTextureName, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                      textureName.c_str());

        // Immediately transition to its only state
        VkImageMemoryBarrier imageBarrier = this->getImageMemoryBarrier();
        imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        if( PixelUtil::isDepth( mFormat ) )
            imageBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        imageBarrier.image = mMsaaTextureName;
        vkCmdPipelineBarrier( device->mGraphicsQueue.mCurrentCmdBuffer,
                              VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0,
                              0u, 0, 0u, 0, 1u, &imageBarrier );
    }
    //-----------------------------------------------------------------------------------
    void VulkanTextureGpu::destroyMsaaSurface( void )
    {
        if(!mMsaaTextureName)
            return;

        VulkanDevice *device = static_cast<VulkanTextureGpuManager*>(mCreator)->getDevice();
        vmaDestroyImage(device->getAllocator(), mMsaaTextureName, mMsaaAllocation);
    }

    VulkanRenderTexture::VulkanRenderTexture(const String& name, HardwarePixelBuffer* buffer, uint32 zoffset,
                                             VulkanTextureGpu* target, uint32 face)
        : RenderTexture(buffer, zoffset)
    {
        mName = name;

        auto texMgr = TextureManager::getSingletonPtr();
        VulkanDevice* device = static_cast<VulkanTextureGpuManager*>(texMgr)->getDevice();

        target->setFSAA(1, "");

        bool depthTarget = PixelUtil::isDepth(target->getFormat());

        if(!depthTarget)
        {
            mDepthTexture.reset(new VulkanTextureGpu(texMgr, mName+"/Depth", 0, "", true, 0));
            mDepthTexture->setWidth(target->getWidth());
            mDepthTexture->setHeight(target->getHeight());
            mDepthTexture->setFormat(PF_DEPTH32);
            mDepthTexture->createInternalResources();
            mDepthTexture->setFSAA(1, "");
        }

        mRenderPassDescriptor.reset(new VulkanRenderPassDescriptor(&device->mGraphicsQueue, device->mRenderSystem));
        mRenderPassDescriptor->mColour[0] = depthTarget ? 0 : target;
        mRenderPassDescriptor->mSlice = face;
        mRenderPassDescriptor->mDepth = depthTarget ? target : mDepthTexture.get();
        mRenderPassDescriptor->mNumColourEntries = int(depthTarget == 0);
        mRenderPassDescriptor->entriesModified(true);
    }
}  // namespace Ogre
