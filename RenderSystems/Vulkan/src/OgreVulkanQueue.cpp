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

#include "OgreVulkanQueue.h"

#include "OgreVulkanDevice.h"
#include "OgreVulkanMappings.h"
#include "OgreVulkanRenderSystem.h"
#include "OgreVulkanTextureGpu.h"
#include "OgreVulkanWindow.h"

#include "OgreException.h"
#include "OgrePixelFormat.h"
#include "OgreStringConverter.h"

#include "OgreVulkanUtils.h"
#include "OgreVulkanDescriptorPool.h"

#define TODO_findRealPresentQueue
#define TODO_we_assume_has_stencil

namespace Ogre
{
    // Mask away read flags from srcAccessMask
    static const uint32 c_srcValidAccessFlags =
        0xFFFFFFFF ^
        ( VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_INDEX_READ_BIT |
          VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_UNIFORM_READ_BIT |
          VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT |
          VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
          VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_HOST_READ_BIT | VK_ACCESS_MEMORY_READ_BIT );

    VulkanQueue::VulkanQueue() :
        mDevice( 0 ),
        mFamily( NumQueueFamilies ),
        mFamilyIdx( 0u ),
        mQueueIdx( 0u ),
        mQueue( 0 ),
        mCurrentCmdBuffer( 0 ),
        mOwnerDevice( 0 ),
        mNumFramesInFlight( 3 ),
        mCurrentFrameIdx( 0 ),
        mRenderSystem( 0 ),
        mCurrentFence( 0 ),
        mEncoderState( EncoderClosed ),
        mCopyEndReadSrcBufferFlags( 0 ),
        mCopyEndReadDstBufferFlags( 0 ),
        mCopyEndReadDstTextureFlags( 0 ),
        mCopyStartWriteSrcBufferFlags( 0 )
    {
    }
    //-------------------------------------------------------------------------
    VulkanQueue::~VulkanQueue() { destroy(); }
    //-------------------------------------------------------------------------
    void VulkanQueue::setQueueData( VulkanDevice *owner, QueueFamily family, uint32 familyIdx,
                                    uint32 queueIdx )
    {
        mOwnerDevice = owner;
        mFamily = family;
        mFamilyIdx = familyIdx;
        mQueueIdx = queueIdx;
    }
    //-------------------------------------------------------------------------
    void VulkanQueue::init( VkDevice device, VkQueue queue, VulkanRenderSystem *renderSystem )
    {
        mDevice = device;
        mQueue = queue;
        mRenderSystem = renderSystem;

        mPerFrameData.resize( mNumFramesInFlight );

        VkCommandPoolCreateInfo cmdPoolCreateInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        cmdPoolCreateInfo.queueFamilyIndex = mFamilyIdx;

        VkCommandBufferAllocateInfo allocateInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocateInfo.commandBufferCount = 1u;

        VkFenceCreateInfo fenceCi = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        fenceCi.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (auto& fd : mPerFrameData)
        {
            OGRE_VK_CHECK(vkCreateCommandPool(mDevice, &cmdPoolCreateInfo, 0, &fd.mCommandPool));
            allocateInfo.commandPool = fd.mCommandPool;
            OGRE_VK_CHECK(vkAllocateCommandBuffers( mDevice, &allocateInfo, &fd.mCommandBuffer ));
            OGRE_VK_CHECK(vkCreateFence(mDevice, &fenceCi, 0, &fd.mProtectingFence));
        }

        newCommandBuffer();
    }

    void VulkanQueue::destroy()
    {
        if( mDevice )
        {
            vkDeviceWaitIdle( mDevice );

            for(size_t i = 0; i < mPerFrameData.size(); ++i)
            {
                _waitOnFrame(i);
            }

            for(auto& fd : mPerFrameData)
            {
                vkDestroyFence( mDevice, fd.mProtectingFence, 0 );
                vkDestroyCommandPool( mDevice, fd.mCommandPool, 0 );
            }

            mDevice = 0;
        }
    }

    //-------------------------------------------------------------------------
    void VulkanQueue::newCommandBuffer( void )
    {
        _waitOnFrame(mCurrentFrameIdx);

        vkResetCommandPool(mDevice, mPerFrameData[mCurrentFrameIdx].mCommandPool, 0);
        mCurrentCmdBuffer = mPerFrameData[mCurrentFrameIdx].mCommandBuffer;
        mCurrentFence = mPerFrameData[mCurrentFrameIdx].mProtectingFence;

        VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer( mCurrentCmdBuffer, &beginInfo );
    }
    //-------------------------------------------------------------------------
    void VulkanQueue::endCommandBuffer( void )
    {
        if( mCurrentCmdBuffer )
        {
            endAllEncoders();

            OGRE_VK_CHECK(vkEndCommandBuffer( mCurrentCmdBuffer ));
        }
    }
    //-------------------------------------------------------------------------
    void VulkanQueue::getGraphicsEncoder( void )
    {
        if( mEncoderState != EncoderGraphicsOpen )
        {
            endCopyEncoder();
            endComputeEncoder();

            mEncoderState = EncoderGraphicsOpen;
        }
    }
    //-------------------------------------------------------------------------
    void VulkanQueue::getComputeEncoder( void )
    {
        if( mEncoderState != EncoderComputeOpen )
        {
            endRenderEncoder();
            endCopyEncoder();

            mEncoderState = EncoderComputeOpen;
        }
    }
    //-------------------------------------------------------------------------
    VkPipelineStageFlags VulkanQueue::deriveStageFromBufferAccessFlags( VkAccessFlags accessFlags )
    {
        VkPipelineStageFlags stage = 0;
        if( accessFlags & VK_ACCESS_INDIRECT_COMMAND_READ_BIT )
            stage |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
        if( accessFlags & ( VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT ) )
        {
            stage |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
        }
        if( accessFlags &
            ( VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT ) )
        {
            stage |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                     VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
                     VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT |
                     VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                     VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        }
        if( accessFlags & ( VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT ) )
        {
            stage |= VK_PIPELINE_STAGE_TRANSFER_BIT;
        }

        return stage;
    }
    //-------------------------------------------------------------------------
    VkPipelineStageFlags VulkanQueue::deriveStageFromTextureAccessFlags( VkAccessFlags accessFlags )
    {
        VkPipelineStageFlags stage = 0;
        if( accessFlags & ( VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT ) )
        {
            stage |=
                VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        }
        if( accessFlags &
            ( VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT ) )
        {
            stage |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        }
        if( accessFlags & ( VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT ) )
        {
            stage |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                     VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
                     VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT |
                     VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                     VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        }
        if( accessFlags & ( VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT ) )
        {
            stage |= VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        if( accessFlags & VK_ACCESS_INPUT_ATTACHMENT_READ_BIT )
            stage |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

        return stage;
    }
    //-------------------------------------------------------------------------
    void VulkanQueue::insertRestoreBarrier( VulkanTextureGpu *vkTexture,
                                            const VkImageLayout newTransferLayout )
    {
        const VkImageLayout oldLayout = vkTexture->mCurrLayout;

        const VkImageLayout otherTransferLayout =
            newTransferLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
                ? VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
                : VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

        const VkAccessFlags accessFlags = newTransferLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
                                              ? VK_ACCESS_TRANSFER_READ_BIT
                                              : VK_ACCESS_TRANSFER_WRITE_BIT;

        if( oldLayout == newTransferLayout )
        {
            // Nothing to do. A restore barrier has already been inserted
            // If the assert fails, then the texture transitioned
            // to this layout without us knowing
            OGRE_ASSERT_HIGH( std::find( mImageMemBarrierPtrs.begin(), mImageMemBarrierPtrs.end(),
                                         vkTexture ) != mImageMemBarrierPtrs.end() &&
                              "Only this class should set VK_IMAGE_LAYOUT_TRANSFER_*_OPTIMAL" );
        }
        else if( oldLayout == otherTransferLayout )
        {
            // A restore barrier has already been inserted, but it needs modification
            FastArray<TextureGpu *>::iterator itor =
                std::find( mImageMemBarrierPtrs.begin(), mImageMemBarrierPtrs.end(), vkTexture );

            // If the assert fails, then the texture transitioned
            // to this layout without us knowing
            OGRE_ASSERT_LOW( itor != mImageMemBarrierPtrs.end() &&
                             "Only this class should set VK_IMAGE_LAYOUT_TRANSFER_*_OPTIMAL" );

            const size_t idx = ( size_t )( itor - mImageMemBarrierPtrs.begin() );
            VkImageMemoryBarrier &imageMemBarrier = *( mImageMemBarriers.begin() + idx );
            imageMemBarrier.srcAccessMask = accessFlags & c_srcValidAccessFlags;
            imageMemBarrier.oldLayout = newTransferLayout;
        }
        else
        {
            // First time we see this texture
            VkImageMemoryBarrier imageMemBarrier = vkTexture->getImageMemoryBarrier();
            imageMemBarrier.srcAccessMask = accessFlags & c_srcValidAccessFlags;
            imageMemBarrier.dstAccessMask = VulkanMappings::get( vkTexture );
            if( newTransferLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL )
            {
                // We need to block subsequent stages from writing to this texture
                // until we're done copying from it (but they can read)
                imageMemBarrier.dstAccessMask &= (VkAccessFlags)~VK_ACCESS_SHADER_READ_BIT;
                mCopyEndReadDstTextureFlags |= imageMemBarrier.dstAccessMask;
            }

            imageMemBarrier.oldLayout = newTransferLayout;
            imageMemBarrier.newLayout = vkTexture->mNextLayout;
            mImageMemBarriers.push_back( imageMemBarrier );
            mImageMemBarrierPtrs.push_back( vkTexture );
        }
    }
    //-------------------------------------------------------------------------
    void VulkanQueue::prepareForUpload( const BufferPacked *buffer, TextureGpu *texture )
    {
        VkAccessFlags bufferAccessFlags = 0;

        if( buffer )
        {
            BufferPackedDownloadMap::iterator it = mCopyDownloadBuffers.find( buffer );

            if( it == mCopyDownloadBuffers.end() )
                ;//bufferAccessFlags = VulkanMappings::get( buffer->getBufferPackedType() );
            else
            {
                if( !it->second )
                {
                    // bufferAccessFlags = VK_ACCESS_TRANSFER_WRITE_BIT;
                    // We assume consecutive writes means we're writing to non-overlapping areas
                    // Do not wait for previous transfers.
                    bufferAccessFlags = 0;
                }
                else
                    bufferAccessFlags = VK_ACCESS_TRANSFER_READ_BIT;
            }

            mCopyDownloadBuffers[buffer] = false;

            mCopyEndReadSrcBufferFlags |= VK_ACCESS_TRANSFER_WRITE_BIT;
        }

        OGRE_ASSERT_HIGH( !texture || dynamic_cast<VulkanTextureGpu *>( texture ) );
        VulkanTextureGpu *vkTexture = static_cast<VulkanTextureGpu *>( texture );

        VkAccessFlags texAccessFlags = 0;

        if( texture )
        {
            TextureGpuDownloadMap::iterator it = mCopyDownloadTextures.find( vkTexture );

            if( vkTexture->mCurrLayout == VK_IMAGE_LAYOUT_UNDEFINED )
            {
                // This texture must just have been created
                texAccessFlags = 0;
            }
            else if( it == mCopyDownloadTextures.end() )
            {
                if( vkTexture->mCurrLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL ||
                    vkTexture->mCurrLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL )
                {
                    OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                                 "Texture " + vkTexture->getName() +
                                     " is already in CopySrc or CopyDst layout, externally set. Perhaps "
                                     "you need to call RenderSystem::flushTextureCopyOperations",
                                 "VulkanQueue::prepareForUpload" );
                }

                texAccessFlags = VulkanMappings::get( texture );
            }
            else
            {
                if( !it->second )
                {
                    OGRE_ASSERT_MEDIUM( vkTexture->mCurrLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL );
                    // texAccessFlags = VK_ACCESS_TRANSFER_WRITE_BIT;
                    // We assume consecutive writes means we're writing to non-overlapping areas
                    // Do not wait for previous transfers.
                    texAccessFlags = 0;
                }
                else
                {
                    OGRE_ASSERT_MEDIUM( vkTexture->mCurrLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL );
                    texAccessFlags = VK_ACCESS_TRANSFER_READ_BIT;
                }
            }

            // We need to block subsequent stages from accessing this texture at all
            // until we're done copying into it
            mCopyEndReadDstTextureFlags |= VulkanMappings::get( texture );
            mCopyDownloadTextures[vkTexture] = false;
        }

        // One buffer barrier is enough for all buffers.
        // Unless we already issued a transfer to this same buffer
        const bool bNeedsBufferBarrier =
            ( bufferAccessFlags &&
              ( mCopyEndReadDstBufferFlags & bufferAccessFlags ) != bufferAccessFlags ) ||
            ( bufferAccessFlags & ( VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT ) );

        const bool bNeedsTexTransition =
            vkTexture && vkTexture->mCurrLayout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

        mCopyEndReadDstBufferFlags |= bufferAccessFlags;

        // Trigger the barrier if we actually have to wait.
        // And only if we haven't issued this barrier already
        if( bNeedsBufferBarrier || bNeedsTexTransition )
        {
            VkPipelineStageFlags srcStage = 0;

            uint32 numMemBarriers = 0u;
            VkMemoryBarrier memBarrier = {VK_STRUCTURE_TYPE_MEMORY_BARRIER};
            if( bNeedsBufferBarrier )
            {
                // GPU must stop using this buffer before we can write into it
                memBarrier.srcAccessMask = bufferAccessFlags & c_srcValidAccessFlags;
                memBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

                srcStage |= deriveStageFromBufferAccessFlags( bufferAccessFlags );
                numMemBarriers = 1u;
            }

            uint32 numImageMemBarriers = 0u;
            VkImageMemoryBarrier imageMemBarrier;
            if( bNeedsTexTransition )
            {
                // GPU must stop using this texture before we can write into it
                // Also we need to do a transition
                imageMemBarrier = vkTexture->getImageMemoryBarrier();
                imageMemBarrier.srcAccessMask = texAccessFlags & c_srcValidAccessFlags;
                imageMemBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                imageMemBarrier.oldLayout = vkTexture->mCurrLayout;
                imageMemBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

                if( texAccessFlags == 0u )
                {
                    if( bufferAccessFlags == 0u )
                    {
                        // Wait for nothing. We're only issuing a barrier
                        // because of the texture transition
                        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                    }
                }
                else
                {
                    srcStage |= deriveStageFromTextureAccessFlags( texAccessFlags );
                }

                insertRestoreBarrier( vkTexture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL );

                vkTexture->mCurrLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                numImageMemBarriers = 1u;
            }

            // Wait until earlier render, compute and transfers are done so we can copy what
            // they wrote (unless we're only here for a texture transition)
            vkCmdPipelineBarrier( mCurrentCmdBuffer, srcStage & mOwnerDevice->mSupportedStages,
                                  VK_PIPELINE_STAGE_TRANSFER_BIT, 0, numMemBarriers, &memBarrier, 0u, 0,
                                  numImageMemBarriers, &imageMemBarrier );
        }
    }
    //-------------------------------------------------------------------------
    void VulkanQueue::prepareForDownload( const BufferPacked *buffer, VulkanTextureGpu *texture )
    {
        VkAccessFlags bufferAccessFlags = 0;
        VkPipelineStageFlags srcStage = 0;

        // Evaluate the stages which blocks us before we can begin our transfer
        if( buffer )
        {
            BufferPackedDownloadMap::iterator it = mCopyDownloadBuffers.find( buffer );

            if( it == mCopyDownloadBuffers.end() )
            {
                if( /*buffer->getBufferPackedType() == BP_TYPE_UAV*/ 0 )
                {
                    bufferAccessFlags = VK_ACCESS_SHADER_WRITE_BIT;
                    srcStage |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                                VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
                                VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT |
                                VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT |
                                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
                }
                // else
                //{
                // If the buffer is not BT_TYPE_UAV, the GPU won't modify these buffers,
                // we can start downloading right away without waiting
                //}
            }
            else
            {
                if( !it->second )
                {
                    bufferAccessFlags = VK_ACCESS_TRANSFER_WRITE_BIT;
                    srcStage |= VK_PIPELINE_STAGE_TRANSFER_BIT;
                }
                else
                    bufferAccessFlags = 0;  // Consecutive reads don't require waiting
            }

            mCopyDownloadBuffers[buffer] = true;

            mCopyEndReadSrcBufferFlags |= VK_ACCESS_TRANSFER_READ_BIT;
        }

        OGRE_ASSERT_HIGH( !texture || dynamic_cast<VulkanTextureGpu *>( texture ) );
        VulkanTextureGpu *vkTexture = static_cast<VulkanTextureGpu *>( texture );

        VkAccessFlags texAccessFlags = 0;

        if( texture )
        {
            TextureGpuDownloadMap::iterator it = mCopyDownloadTextures.find( vkTexture );

            if( it == mCopyDownloadTextures.end() )
            {
                if( vkTexture->mCurrLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL ||
                    vkTexture->mCurrLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL )
                {
                    OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                                 "Texture " + vkTexture->getName() +
                                     " is already in CopySrc or CopyDst layout, externally set. Perhaps "
                                     "you need to call RenderSystem::flushTextureCopyOperations",
                                 "VulkanQueue::prepareForDownload" );
                }

                if( texture->isUav() )
                {
                    texAccessFlags |= VK_ACCESS_SHADER_WRITE_BIT;
                    srcStage |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                                VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
                                VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT |
                                VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT |
                                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
                }

                if( texture->getUsage() & TU_RENDERTARGET )
                {
                    texAccessFlags |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                    if( !PixelUtil::isDepth( texture->getFormat() ) )
                    {
                        texAccessFlags |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                        srcStage |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                    }
                    else
                    {
                        texAccessFlags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                        srcStage |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                    VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                    }
                }
            }
            else
            {
                if( !it->second )
                {
                    OGRE_ASSERT_MEDIUM( vkTexture->mCurrLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL );
                    texAccessFlags = VK_ACCESS_TRANSFER_WRITE_BIT;
                    srcStage |= VK_PIPELINE_STAGE_TRANSFER_BIT;
                }
                else
                {
                    OGRE_ASSERT_MEDIUM( vkTexture->mCurrLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL );
                    texAccessFlags = 0;  // Consecutive reads don't require waiting
                }
            }

            mCopyDownloadTextures[vkTexture] = true;
        }

        // One buffer barrier is enough for all buffers.
        // Unless we already issued a transfer to this same buffer
        const bool bNeedsBufferBarrier =
            ( bufferAccessFlags &&
              ( mCopyStartWriteSrcBufferFlags & bufferAccessFlags ) != bufferAccessFlags ) ||
            ( bufferAccessFlags & ( VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT ) );

        mCopyStartWriteSrcBufferFlags |= bufferAccessFlags;

        const bool bNeedsTexTransition =
            vkTexture && vkTexture->mCurrLayout != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

        // Trigger the barrier if we actually have to wait.
        // And only if we haven't issued this barrier already
        if( bNeedsBufferBarrier || bNeedsTexTransition )
        {
            uint32 numMemBarriers = 0u;
            VkMemoryBarrier memBarrier = {VK_STRUCTURE_TYPE_MEMORY_BARRIER};
            if( bNeedsBufferBarrier )
            {
                memBarrier.srcAccessMask = bufferAccessFlags & c_srcValidAccessFlags;
                memBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                numMemBarriers = 1u;
            }

            uint32 numImageMemBarriers = 0u;
            VkImageMemoryBarrier imageMemBarrier;
            if( bNeedsTexTransition )
            {
                imageMemBarrier = vkTexture->getImageMemoryBarrier();
                imageMemBarrier.srcAccessMask = texAccessFlags & c_srcValidAccessFlags;
                imageMemBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                imageMemBarrier.oldLayout = vkTexture->mCurrLayout;
                imageMemBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

                insertRestoreBarrier( vkTexture, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL );

                vkTexture->mCurrLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                numImageMemBarriers = 1u;

                if( !srcStage )
                {
                    // If we're here the texture is read-only and we only
                    // need the barrier to perform a layout transition
                    srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                }
            }

            // Wait until earlier render, compute and transfers are done so we can copy what
            // they wrote (unless we're only here for a texture transition)
            vkCmdPipelineBarrier( mCurrentCmdBuffer, srcStage & mOwnerDevice->mSupportedStages,
                                  VK_PIPELINE_STAGE_TRANSFER_BIT, 0, numMemBarriers, &memBarrier, 0u, 0,
                                  numImageMemBarriers, &imageMemBarrier );
        }
    }
    //-------------------------------------------------------------------------
    void VulkanQueue::getCopyEncoder( const BufferPacked *buffer, VulkanTextureGpu *texture,
                                      const bool bDownload )
    {
        OgreAssert(mEncoderState != EncoderGraphicsOpen, "interrupting RenderPass not supported");
        if( mEncoderState != EncoderCopyOpen )
        {
            endRenderEncoder();
            endComputeEncoder();

            mEncoderState = EncoderCopyOpen;

            // Submission guarantees the host write being complete, as per
            // khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#synchronization-submission-host-writes
            // So no need for a barrier before the transfer
            //
            // The only exception is when writing from CPU to GPU when a command that uses that region
            // has already been submitted via vkQueueSubmit (and you're using vkCmdWaitEvents to wait
            // for the CPU to write the data and give ok to the GPU).
            // Which Ogre does not do (too complex to get right).
        }

        if( bDownload )
            prepareForDownload( buffer, texture );
        else
            prepareForUpload( buffer, texture );
    }
    //-------------------------------------------------------------------------
    void VulkanQueue::getCopyEncoderV1Buffer( const bool bDownload )
    {
        OgreAssert(mEncoderState != EncoderGraphicsOpen, "interrupting RenderPass not supported");
        if( mEncoderState != EncoderCopyOpen )
        {
            endRenderEncoder();
            endComputeEncoder();

            mEncoderState = EncoderCopyOpen;
        }

        if( !bDownload )
        {
            // V1 buffers are only used for vertex and index buffers
            // We assume v1 buffers don't try to write then read (or read then write) in a row
            const VkAccessFlags bufferAccessFlags =
                VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_INDEX_READ_BIT;

            if( ( mCopyEndReadDstBufferFlags & bufferAccessFlags ) != bufferAccessFlags )
            {
                uint32 numMemBarriers = 0u;
                VkMemoryBarrier memBarrier = {VK_STRUCTURE_TYPE_MEMORY_BARRIER};
                memBarrier.srcAccessMask = bufferAccessFlags & c_srcValidAccessFlags;
                memBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                numMemBarriers = 1u;

                // GPU must stop using this buffer before we can write into it
                vkCmdPipelineBarrier( mCurrentCmdBuffer, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                                      VK_PIPELINE_STAGE_TRANSFER_BIT, 0, numMemBarriers, &memBarrier, 0u,
                                      0, 0u, 0 );
            }

            mCopyEndReadDstBufferFlags |= bufferAccessFlags;
            mCopyEndReadSrcBufferFlags |= VK_ACCESS_TRANSFER_WRITE_BIT;
        }
        else
        {
            mCopyEndReadSrcBufferFlags |= VK_ACCESS_TRANSFER_READ_BIT;
        }
    }
    //-------------------------------------------------------------------------
    void VulkanQueue::endCopyEncoder( void )
    {
        if( mEncoderState != EncoderCopyOpen )
            return;

        if( mCopyEndReadDstBufferFlags || !mImageMemBarrierPtrs.empty() )
        {
            VkPipelineStageFlags dstStage = 0;

            uint32 numMemBarriers = 0u;
            VkMemoryBarrier memBarrier = {VK_STRUCTURE_TYPE_MEMORY_BARRIER};
            if( mCopyEndReadDstBufferFlags )
            {
                memBarrier.srcAccessMask = mCopyEndReadSrcBufferFlags & c_srcValidAccessFlags;
                memBarrier.dstAccessMask = mCopyEndReadDstBufferFlags;

                // Evaluate the stages we can unblock when our transfers are done
                dstStage |= deriveStageFromBufferAccessFlags( memBarrier.dstAccessMask );
                numMemBarriers = 1u;
            }

            dstStage |= deriveStageFromTextureAccessFlags( mCopyEndReadDstTextureFlags );

            if( dstStage == 0u )
            {
                // Nothing needs to wait for us. Can happen if all we're
                // doing is copying from read-only textures (rare)
                dstStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

#if OGRE_DEBUG_MODE
                FastArray<TextureGpu *>::const_iterator itor = mImageMemBarrierPtrs.begin();
                FastArray<TextureGpu *>::const_iterator endt = mImageMemBarrierPtrs.end();

                while( itor != endt )
                {
                    OgreAssert( (( *itor )->getUsage() & TU_RENDERTARGET) == 0/*&& !( *itor )->isUav()*/,
                                        "endCopyEncoder says nothing will wait on this texture(s) but "
                                        "we don't know if a subsequent stage will write to it" );
                    ++itor;
                }
#endif
            }

            // Wait until earlier render, compute and transfers are done
            // Block render, compute and transfers until we're done
            vkCmdPipelineBarrier( mCurrentCmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                  dstStage & mOwnerDevice->mSupportedStages, 0, numMemBarriers,
                                  &memBarrier, 0u, 0, static_cast<uint32_t>( mImageMemBarriers.size() ),
                                  mImageMemBarriers.data() );

            mImageMemBarriers.clear();
            mImageMemBarrierPtrs.clear();

            TextureGpuDownloadMap::const_iterator itor = mCopyDownloadTextures.begin();
            TextureGpuDownloadMap::const_iterator endt = mCopyDownloadTextures.end();

            while( itor != endt )
            {
                itor->first->mCurrLayout = itor->first->mNextLayout;
                ++itor;
            }
        }

        mCopyEndReadSrcBufferFlags = 0;
        mCopyEndReadDstBufferFlags = 0;
        mCopyEndReadDstTextureFlags = 0;
        mCopyStartWriteSrcBufferFlags = 0;

        mCopyDownloadTextures.clear();
        mCopyDownloadBuffers.clear();

        mEncoderState = EncoderClosed;
    }
    //-------------------------------------------------------------------------
    void VulkanQueue::endRenderEncoder( const bool endRenderPassDesc )
    {
        if( mEncoderState != EncoderGraphicsOpen )
            return;
        mRenderSystem->_notifyActiveEncoderEnded();
        if( endRenderPassDesc )
            mRenderSystem->endRenderPassDescriptor();
        mEncoderState = EncoderClosed;
    }
    //-------------------------------------------------------------------------
    void VulkanQueue::endComputeEncoder( void )
    {
        if( mEncoderState != EncoderComputeOpen )
            return;

        mEncoderState = EncoderClosed;
        mRenderSystem->_notifyActiveComputeEnded();
    }
    //-------------------------------------------------------------------------
    void VulkanQueue::endAllEncoders( const bool endRenderPassDesc )
    {
        endCopyEncoder();
        endRenderEncoder( endRenderPassDesc );
        endComputeEncoder();
    }
    //-------------------------------------------------------------------------
    void VulkanQueue::notifyTextureDestroyed( VulkanTextureGpu *texture )
    {
        if( mEncoderState == EncoderCopyOpen )
        {
            bool needsToFlush = false;
            TextureGpuDownloadMap::const_iterator itor = mCopyDownloadTextures.find( texture );

            if( itor != mCopyDownloadTextures.end() )
                needsToFlush = true;
            else
            {
                FastArray<TextureGpu *>::const_iterator it2 =
                    std::find( mImageMemBarrierPtrs.begin(), mImageMemBarrierPtrs.end(), texture );
                if( it2 != mImageMemBarrierPtrs.end() )
                    needsToFlush = true;
            }

            if( needsToFlush )
            {
                // If this asserts triggers, then the texture is probably being referenced
                // by something else doing anything on the texture and was interrupted
                // midway (since Ogre must ensure the texture ends in TRANSFER_SRC/DST_OPTIMAL
                // if the copy encoder is holding a reference.
                OGRE_ASSERT_LOW( texture->mCurrLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL ||
                                 texture->mCurrLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL );
                endCopyEncoder();
            }
        }
    }
    //-------------------------------------------------------------------------
    void VulkanQueue::addWindowToWaitFor( VkSemaphore imageAcquisitionSemaph )
    {
        OGRE_ASSERT_MEDIUM( mFamily == Graphics );
        mGpuWaitFlags.push_back( VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT );
        mGpuWaitSemaphForCurrCmdBuff.push_back( imageAcquisitionSemaph );
    }
    //-------------------------------------------------------------------------
    void VulkanQueue::queueForDeletion(VkBuffer buffer, VkDeviceMemory memory)
    {
       mPerFrameData[mCurrentFrameIdx].mBufferGraveyard.push_back({buffer, memory});
    }
    void VulkanQueue::queueForDeletion(const std::shared_ptr<VulkanDescriptorPool>& descriptorPool)
    {
        mPerFrameData[mCurrentFrameIdx].mDescriptorPoolGraveyard.push_back(descriptorPool);
    }
    void VulkanQueue::_waitOnFrame( uint8 frameIdx )
    {
        VkFence fence = mPerFrameData[frameIdx].mProtectingFence;
        vkWaitForFences( mDevice, 1, &fence, VK_TRUE, UINT64_MAX );

        // it is safe to free staging buffers now
        for(auto bm : mPerFrameData[frameIdx].mBufferGraveyard)
        {
            vkDestroyBuffer(mDevice, bm.first, nullptr);
            vkFreeMemory(mDevice, bm.second, nullptr);
        }
        mPerFrameData[frameIdx].mBufferGraveyard.clear();
        mPerFrameData[frameIdx].mDescriptorPoolGraveyard.clear();
    }
    //-------------------------------------------------------------------------
    bool VulkanQueue::_isFrameFinished( uint8 frameIdx )
    {
        VkFence fence = mPerFrameData[frameIdx].mProtectingFence;
        VkResult ret = vkWaitForFences( mDevice, 1, &fence, VK_TRUE, 0u );
        if( ret != VK_TIMEOUT )
        {
            OGRE_VK_CHECK(ret);
            //recycleFences( fences );
            return true;
        }

        return false;
    }
    //-------------------------------------------------------------------------
    void VulkanQueue::commitAndNextCommandBuffer( SubmissionType::SubmissionType submissionType )
    {
        endCommandBuffer();

        VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submitInfo.commandBufferCount   = 1;
        submitInfo.pCommandBuffers      = &mCurrentCmdBuffer;

        if( !mGpuWaitSemaphForCurrCmdBuff.empty() )
        {
            // We need to wait on these semaphores so that rendering can
            // only happen start the swapchain is done presenting
            submitInfo.waitSemaphoreCount = mGpuWaitSemaphForCurrCmdBuff.size();
            submitInfo.pWaitSemaphores = mGpuWaitSemaphForCurrCmdBuff.data();
            submitInfo.pWaitDstStageMask = mGpuWaitFlags.data();
        }

        if( submissionType >= SubmissionType::NewFrameIdx )
        {
            if( submissionType >= SubmissionType::EndFrameAndSwap )
            {
                // Get semaphores so that presentation can wait for this job to finish rendering
                // (one for each window that will be swapped)
                for (auto w : mWindowsPendingSwap)
                {
                    mGpuSignalSemaphForCurrCmdBuff.push_back(w->getRenderFinishedSemaphore());
                    w->setImageFence(mCurrentFence);
                }
            }

            if( !mGpuSignalSemaphForCurrCmdBuff.empty() )
            {
                // We need to signal these semaphores so that presentation
                // can only happen after we're done rendering (presentation may not be the
                // only thing waiting for us though; thus we must set this with NewFrameIdx
                // and not just with EndFrameAndSwap)
                submitInfo.signalSemaphoreCount = mGpuSignalSemaphForCurrCmdBuff.size();
                submitInfo.pSignalSemaphores = mGpuSignalSemaphForCurrCmdBuff.data();
            }
        }

        OGRE_VK_CHECK(vkResetFences(mDevice, 1, &mCurrentFence) );
        vkQueueSubmit( mQueue, 1u, &submitInfo, mCurrentFence );
        mGpuWaitSemaphForCurrCmdBuff.clear();
        mCurrentCmdBuffer = VK_NULL_HANDLE;

        if( submissionType >= SubmissionType::EndFrameAndSwap )
        {
            for (auto w : mWindowsPendingSwap)
                w->_swapBuffers();
        }

        if( submissionType >= SubmissionType::NewFrameIdx )
        {
            mCurrentFrameIdx = (mCurrentFrameIdx + 1) % mPerFrameData.size();
        }

        newCommandBuffer();

        if( submissionType >= SubmissionType::EndFrameAndSwap )
        {
            // acquireNextImage must be called after newCommandBuffer()
            for (auto w : mWindowsPendingSwap)
                w->acquireNextImage();
            mWindowsPendingSwap.clear();

            mGpuSignalSemaphForCurrCmdBuff.clear();
        }
    }
}  // namespace Ogre
