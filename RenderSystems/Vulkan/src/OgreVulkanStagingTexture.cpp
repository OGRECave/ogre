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

#include "OgreVulkanStagingTexture.h"

#include "OgreVulkanDevice.h"
#include "OgreVulkanTextureGpu.h"
#include "OgreVulkanUtils.h"
#include "Vao/OgreVulkanDynamicBuffer.h"
#include "Vao/OgreVulkanVaoManager.h"

namespace Ogre
{
    VulkanStagingTexture::VulkanStagingTexture( VaoManager *vaoManager, PixelFormatGpu formatFamily,
                                                size_t size, size_t internalBufferStart,
                                                size_t vboPoolIdx, VkBuffer vboName,
                                                VulkanDynamicBuffer *dynamicBuffer ) :
        StagingTextureBufferImpl( vaoManager, formatFamily, size, internalBufferStart, vboPoolIdx ),
        mVboName( vboName ),
        mDynamicBuffer( dynamicBuffer ),
        mUnmapTicket( std::numeric_limits<size_t>::max() ),
        mMappedPtr( 0 ),
        mLastMappedPtr( 0 )
    {
        mMappedPtr = mDynamicBuffer;
        mLastMappedPtr = mMappedPtr;
    }
    //-----------------------------------------------------------------------------------
    VulkanStagingTexture::~VulkanStagingTexture()
    {
        assert( mUnmapTicket == std::numeric_limits<size_t>::max() );

        if( mDynamicBuffer )
            mDynamicBuffer = 0;
        mMappedPtr = 0;
    }
    //-----------------------------------------------------------------------------------
    void VulkanStagingTexture::_unmapBuffer( void )
    {
        if( mUnmapTicket != std::numeric_limits<size_t>::max() )
        {
            mDynamicBuffer->flush( mUnmapTicket, 0u, mSize );
            mDynamicBuffer->unmap( mUnmapTicket );
            mUnmapTicket = std::numeric_limits<size_t>::max();
            mMappedPtr = 0;
        }
    }
    //-----------------------------------------------------------------------------------
    bool VulkanStagingTexture::supportsFormat( uint32 width, uint32 height, uint32 depth, uint32 slices,
                                               PixelFormatGpu pixelFormat ) const
    {
        // Vulkan requires offsets to be multiple of the texel's block size. Accepting a texture with
        // different texel size could unalign us.
        //
        // We use getSizeBytes rather than getBytesPerPixel because the former covers compressed formats
        if( PixelFormatGpuUtils::getSizeBytes( 1u, 1u, 1u, 1u, mFormatFamily, 1u ) !=
            PixelFormatGpuUtils::getSizeBytes( 1u, 1u, 1u, 1u, pixelFormat, 1u ) )
        {
            return false;
        }
        return StagingTextureBufferImpl::supportsFormat( width, height, depth, slices, pixelFormat );
    }
    //-----------------------------------------------------------------------------------
    bool VulkanStagingTexture::belongsToUs( const TextureBox &box )
    {
        return box.data >= mLastMappedPtr &&
               box.data <= static_cast<uint8 *>( mLastMappedPtr ) + mCurrentOffset;
    }
    //-----------------------------------------------------------------------------------
    void *RESTRICT_ALIAS_RETURN VulkanStagingTexture::mapRegionImplRawPtr( void )
    {
        return static_cast<uint8 *>( mMappedPtr ) + mCurrentOffset;
    }
    //-----------------------------------------------------------------------------------
    void VulkanStagingTexture::startMapRegion()
    {
        StagingTextureBufferImpl::startMapRegion();

        OGRE_ASSERT_MEDIUM( mUnmapTicket == std::numeric_limits<size_t>::max() &&
                            "VulkanStagingTexture still mapped!" );
        mMappedPtr = mDynamicBuffer->map( mInternalBufferStart, mSize, mUnmapTicket );
        mLastMappedPtr = mMappedPtr;
    }
    //-----------------------------------------------------------------------------------
    void VulkanStagingTexture::stopMapRegion()
    {
        OGRE_ASSERT_MEDIUM( mUnmapTicket != std::numeric_limits<size_t>::max() &&
                            "VulkanStagingTexture already unmapped!" );

        mDynamicBuffer->flush( mUnmapTicket, 0u, mSize );
        mDynamicBuffer->unmap( mUnmapTicket );
        mUnmapTicket = std::numeric_limits<size_t>::max();

        mMappedPtr = 0;

        StagingTextureBufferImpl::stopMapRegion();
    }
    //-----------------------------------------------------------------------------------
    void VulkanStagingTexture::upload( const TextureBox &srcBox, TextureGpu *dstTexture, uint8 mipLevel,
                                       const TextureBox *cpuSrcBox, const TextureBox *dstBox,
                                       bool skipSysRamCopy )
    {
        StagingTextureBufferImpl::upload( srcBox, dstTexture, mipLevel, cpuSrcBox, dstBox,
                                          skipSysRamCopy );

        VulkanVaoManager *vaoManager = static_cast<VulkanVaoManager *>( mVaoManager );
        VulkanDevice *device = vaoManager->getDevice();

        device->mGraphicsQueue.getCopyEncoder( 0, dstTexture, false );

        size_t bytesPerRow = srcBox.bytesPerRow;
        // size_t bytesPerImage = srcBox.bytesPerImage;

        // We can't trust mFormatFamily because supportsFormat accepts any format
        const PixelFormatGpu pixelFormat = dstTexture->getPixelFormat();

        if( PixelFormatGpuUtils::isCompressed( pixelFormat ) )
        {
            bytesPerRow = 0;
            // bytesPerImage = 0;
        }

        assert( dynamic_cast<VulkanTextureGpu *>( dstTexture ) );
        VulkanTextureGpu *dstTextureVulkan = static_cast<VulkanTextureGpu *>( dstTexture );

        const size_t distToStart =
            ( size_t )( static_cast<uint8 *>( srcBox.data ) - static_cast<uint8 *>( mLastMappedPtr ) );
        const VkDeviceSize offsetPtr = mInternalBufferStart + distToStart;

        const uint32 destinationSlice =
            ( dstBox ? dstBox->sliceStart : 0 ) + dstTexture->getInternalSliceStart();
        const uint32 numSlices = ( dstBox ? dstBox->numSlices : dstTexture->getNumSlices() );
        uint32 xPos = static_cast<uint32>( dstBox ? dstBox->x : 0 );
        uint32 yPos = static_cast<uint32>( dstBox ? dstBox->y : 0 );
        uint32 zPos = static_cast<uint32>( dstBox ? dstBox->z : 0 );

        VkBufferImageCopy region;
        region.bufferOffset = offsetPtr;
        region.bufferRowLength = 0;
        if( bytesPerRow != 0 )
        {
            region.bufferRowLength = static_cast<uint32_t>(
                bytesPerRow / PixelFormatGpuUtils::getBytesPerPixel( pixelFormat ) );
        }
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = mipLevel;
        region.imageSubresource.baseArrayLayer = destinationSlice;
        region.imageSubresource.layerCount = numSlices;

        region.imageOffset.x = static_cast<int32_t>( xPos );
        region.imageOffset.y = static_cast<int32_t>( yPos );
        region.imageOffset.z = static_cast<int32_t>( zPos );
        region.imageExtent.width = srcBox.width;
        region.imageExtent.height = srcBox.height;
        region.imageExtent.depth = srcBox.depth;

        vkCmdCopyBufferToImage( device->mGraphicsQueue.mCurrentCmdBuffer, mVboName,
                                dstTextureVulkan->getFinalTextureName(),
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &region );
    }
}  // namespace Ogre
