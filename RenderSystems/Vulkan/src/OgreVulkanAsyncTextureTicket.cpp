/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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

#include "OgreVulkanAsyncTextureTicket.h"

#include "OgreVulkanMappings.h"
#include "OgreVulkanQueue.h"
#include "OgreVulkanTextureGpu.h"
#include "OgreVulkanUtils.h"
#include "Vao/OgreVulkanVaoManager.h"

#include "OgrePixelFormatGpuUtils.h"
#include "OgreTextureBox.h"
#include "OgreTextureGpuManager.h"

namespace Ogre
{
    VulkanAsyncTextureTicket::VulkanAsyncTextureTicket( uint32 width, uint32 height,
                                                        uint32 depthOrSlices,  //
                                                        TextureTypes::TextureTypes textureType,
                                                        PixelFormatGpu pixelFormatFamily,
                                                        VulkanVaoManager *vaoManager,
                                                        VulkanQueue *queue ) :
        AsyncTextureTicket( width, height, depthOrSlices, textureType, pixelFormatFamily ),
        mDownloadFrame( 0 ),
        mAccurateFence( 0 ),
        mVaoManager( vaoManager ),
        mQueue( queue )
    {
        const uint32 rowAlignment = 4u;
        const size_t sizeBytes = PixelFormatGpuUtils::getSizeBytes( width, height, depthOrSlices, 1u,
                                                                    mPixelFormatFamily, rowAlignment );
        // Vulkan requires offsets to be multiple of the texel's block size.
        const size_t alignment =
            PixelFormatGpuUtils::getSizeBytes( 1u, 1u, 1u, 1u, mPixelFormatFamily, 1u );

        mVboName =
            mVaoManager->allocateRawBuffer( VulkanVaoManager::CPU_READ_WRITE, sizeBytes, alignment );
    }
    //-----------------------------------------------------------------------------------
    VulkanAsyncTextureTicket::~VulkanAsyncTextureTicket()
    {
        if( mStatus == Mapped )
            unmap();

        if( mVboName.mVboName )
            mVaoManager->deallocateRawBuffer( mVboName );

        if( mAccurateFence )
        {
            mQueue->releaseFence( mAccurateFence );
            mAccurateFence = 0;
        }
    }
    //-----------------------------------------------------------------------------------
    void VulkanAsyncTextureTicket::downloadFromGpu( TextureGpu *textureSrc, uint8 mipLevel,
                                                    bool accurateTracking, TextureBox *srcBox )
    {
        AsyncTextureTicket::downloadFromGpu( textureSrc, mipLevel, accurateTracking, srcBox );

        mDownloadFrame = mVaoManager->getFrameCount();

        if( mAccurateFence )
        {
            mQueue->releaseFence( mAccurateFence );
            mAccurateFence = 0;
        }

        TextureBox srcTextureBox;
        TextureBox fullSrcTextureBox( textureSrc->getEmptyBox( mipLevel ) );

        if( !srcBox )
            srcTextureBox = fullSrcTextureBox;
        else
        {
            srcTextureBox = *srcBox;
            srcTextureBox.bytesPerRow = fullSrcTextureBox.bytesPerRow;
            srcTextureBox.bytesPerPixel = fullSrcTextureBox.bytesPerPixel;
            srcTextureBox.bytesPerImage = fullSrcTextureBox.bytesPerImage;
        }

        if( textureSrc->hasAutomaticBatching() )
        {
            // fullSrcTextureBox.sliceStart= textureSrc->getInternalSliceStart();
            // fullSrcTextureBox.numSlices =
            // textureSrc->getTexturePool()->masterTexture->getNumSlices();

            srcTextureBox.sliceStart += textureSrc->getInternalSliceStart();
        }

        assert( dynamic_cast<VulkanTextureGpu *>( textureSrc ) );
        VulkanTextureGpu *srcTextureVk = static_cast<VulkanTextureGpu *>( textureSrc );

        // No need to call mQueue->getCopyEncoder( this, 0, false ); because
        // this is a fresh memory region (unless mStatus == Downloading)
        mQueue->getCopyEncoder( 0, textureSrc, true );
        if( mStatus == Downloading )
        {
            // User called downloadFromGpu twice in a row without mapping.
            // The 2nd copy can't start before the first copy finishes
            VkMemoryBarrier memBarrier;
            makeVkStruct( memBarrier, VK_STRUCTURE_TYPE_MEMORY_BARRIER );
            memBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            memBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            // GPU must stop using this buffer before we can write into it
            vkCmdPipelineBarrier( mQueue->mCurrentCmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                  VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1u, &memBarrier, 0u, 0, 0u, 0 );
        }

        size_t destBytesPerRow = getBytesPerRow();
        // size_t destBytesPerImage = getBytesPerImage();

        if( PixelFormatGpuUtils::isCompressed( mPixelFormatFamily ) )
        {
            destBytesPerRow = 0;
            // destBytesPerImage = 0;
        }

        VkBufferImageCopy region;
        region.bufferOffset = mVboName.mInternalBufferStart;
        region.bufferRowLength = 0;
        if( destBytesPerRow != 0 )
        {
            region.bufferRowLength = static_cast<uint32_t>(
                destBytesPerRow / PixelFormatGpuUtils::getBytesPerPixel( mPixelFormatFamily ) );
        }
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VulkanMappings::getImageAspect( mPixelFormatFamily );
        region.imageSubresource.mipLevel = mipLevel;
        region.imageSubresource.baseArrayLayer = srcTextureBox.sliceStart;
        region.imageSubresource.layerCount = srcTextureBox.numSlices;

        region.imageOffset.x = static_cast<int32_t>( srcTextureBox.x );
        region.imageOffset.y = static_cast<int32_t>( srcTextureBox.y );
        region.imageOffset.z = static_cast<int32_t>( srcTextureBox.z );
        region.imageExtent.width = srcTextureBox.width;
        region.imageExtent.height = srcTextureBox.height;
        region.imageExtent.depth = srcTextureBox.depth;

        vkCmdCopyImageToBuffer( mQueue->mCurrentCmdBuffer, srcTextureVk->getFinalTextureName(),
                                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, mVboName.mVboName, 1u, &region );

        if( accurateTracking )
        {
            mAccurateFence = mQueue->acquireCurrentFence();
            // Flush now for accuracy with downloads.
            mQueue->commitAndNextCommandBuffer();
        }
    }
    //-----------------------------------------------------------------------------------
    TextureBox VulkanAsyncTextureTicket::mapImpl( uint32 slice )
    {
        waitForDownloadToFinish();

        TextureBox retVal;

        retVal = TextureBox( mWidth, mHeight, getDepth(), getNumSlices(),
                             (uint32)PixelFormatGpuUtils::getBytesPerPixel( mPixelFormatFamily ),
                             (uint32)getBytesPerRow(), (uint32)getBytesPerImage() );

        if( PixelFormatGpuUtils::isCompressed( mPixelFormatFamily ) )
            retVal.setCompressedPixelFormat( mPixelFormatFamily );

        retVal.data = mVboName.map();
        retVal.data = retVal.at( 0, 0, slice );
        retVal.numSlices -= slice;

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void VulkanAsyncTextureTicket::unmapImpl( void ) { mVboName.unmap(); }
    //-----------------------------------------------------------------------------------
    void VulkanAsyncTextureTicket::waitForDownloadToFinish( void )
    {
        if( mStatus != Downloading )
            return;  // We're done.

        if( mAccurateFence )
        {
            mAccurateFence = VulkanVaoManager::waitFor( mAccurateFence, mQueue );
        }
        else
        {
            mVaoManager->waitForSpecificFrameToFinish( mDownloadFrame );
            mNumInaccurateQueriesWasCalledInIssuingFrame = 0;
        }

        mStatus = Ready;
    }
    //-----------------------------------------------------------------------------------
    bool VulkanAsyncTextureTicket::queryIsTransferDone( void )
    {
        if( !AsyncTextureTicket::queryIsTransferDone() )
        {
            // Early out. The texture is not even finished being ready.
            // We didn't even start the actual download.
            return false;
        }

        bool retVal = false;

        if( mStatus != Downloading )
        {
            retVal = true;
        }
        else if( mAccurateFence )
        {
            // Ask to return immediately and tell us about the fence
            VkResult result = vkWaitForFences( mQueue->mDevice, 1u, &mAccurateFence, VK_TRUE, 0 );
            if( result != VK_TIMEOUT )
            {
                mQueue->releaseFence( mAccurateFence );
                mAccurateFence = 0;

                checkVkResult( result, "vkWaitForFences" );
                if( mStatus != Mapped )
                    mStatus = Ready;
                retVal = true;
            }
        }
        else
        {
            if( mDownloadFrame == mVaoManager->getFrameCount() )
            {
                if( mNumInaccurateQueriesWasCalledInIssuingFrame > 3 )
                {
                    // User is not calling vaoManager->update(). Likely it's stuck in an
                    // infinite loop checking if we're done, but we'll always return false.
                    // If so, switch to accurate tracking.
                    mAccurateFence = mQueue->acquireCurrentFence();
                    // Flush now for accuracy with downloads.
                    mQueue->commitAndNextCommandBuffer();

                    LogManager::getSingleton().logMessage(
                        "WARNING: Calling AsyncTextureTicket::queryIsTransferDone too "
                        "often with innacurate tracking in the same frame this transfer "
                        "was issued. Switching to accurate tracking. If this is an accident, "
                        "wait until you've rendered a few frames before checking if it's done. "
                        "If this is on purpose, consider calling AsyncTextureTicket::download()"
                        " with accurate tracking enabled.",
                        LML_CRITICAL );
                }

                ++mNumInaccurateQueriesWasCalledInIssuingFrame;
            }

            // We're downloading but have no fence. That means we don't have accurate tracking.
            retVal = mVaoManager->isFrameFinished( mDownloadFrame );
            ++mNumInaccurateQueriesWasCalledInIssuingFrame;
        }

        return retVal;
    }
}  // namespace Ogre
