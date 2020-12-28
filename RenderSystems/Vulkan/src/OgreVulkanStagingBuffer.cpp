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

#include "Vao/OgreVulkanStagingBuffer.h"

#include "Vao/OgreVulkanBufferInterface.h"
#include "Vao/OgreVulkanDynamicBuffer.h"
#include "Vao/OgreVulkanVaoManager.h"

#include "OgreStringConverter.h"

#include "OgreVulkanDevice.h"
#include "OgreVulkanHardwareBufferCommon.h"
#include "OgreVulkanQueue.h"
#include "OgreVulkanUtils.h"

namespace Ogre
{
    VulkanStagingBuffer::VulkanStagingBuffer( size_t vboIdx, size_t internalBufferStart,
                                              size_t sizeBytes, VaoManager *vaoManager, bool uploadOnly,
                                              VkBuffer vboName, VulkanDynamicBuffer *dynamicBuffer ) :
        StagingBuffer( internalBufferStart, sizeBytes, vaoManager, uploadOnly ),
        mMappedPtr( 0 ),
        mVboName( vboName ),
        mDynamicBuffer( dynamicBuffer ),
        mUnmapTicket( std::numeric_limits<size_t>::max() ),
        mFenceThreshold( sizeBytes / 4 ),
        mVboPoolIdx( vboIdx ),
        mUnfencedBytes( 0 )
    {
    }
    //-----------------------------------------------------------------------------------
    VulkanStagingBuffer::~VulkanStagingBuffer()
    {
        if( !mFences.empty() )
            wait( mFences.back().fenceName );

        deleteFences( mFences.begin(), mFences.end() );

        VulkanVaoManager *vaoManager = static_cast<VulkanVaoManager *>( mVaoManager );
        vaoManager->deallocateVbo( mVboPoolIdx, mInternalBufferStart, getMaxSize(), BT_DYNAMIC_DEFAULT,
                                   !mUploadOnly );
    }
    //-----------------------------------------------------------------------------------
    void VulkanStagingBuffer::addFence( size_t from, size_t to, bool forceFence )
    {
        assert( to <= mSizeBytes );

        VulkanFence unfencedHazard( from, to );

        mUnfencedHazards.push_back( unfencedHazard );

        assert( from <= to );

        mUnfencedBytes += to - from;

        VulkanVaoManager *vaoManager = static_cast<VulkanVaoManager *>( mVaoManager );
        VulkanDevice *device = vaoManager->getDevice();

        if( mUnfencedBytes >= mFenceThreshold || forceFence )
        {
            VulkanFenceVec::const_iterator itor = mUnfencedHazards.begin();
            VulkanFenceVec::const_iterator endt = mUnfencedHazards.end();

            size_t startRange = itor->start;
            size_t endRange = itor->end;

            while( itor != endt )
            {
                if( endRange <= itor->end )
                {
                    // Keep growing (merging) the fences into one fence
                    endRange = itor->end;
                }
                else
                {
                    // We wrapped back to 0. Can't keep merging. Make a fence.
                    VulkanFence fence( startRange, endRange );
                    fence.fenceName = device->mGraphicsQueue.acquireCurrentFence();
                    mFences.push_back( fence );

                    startRange = itor->start;
                    endRange = itor->end;
                }

                ++itor;
            }

            // Make the last fence.
            VulkanFence fence( startRange, endRange );
            fence.fenceName = device->mGraphicsQueue.acquireCurrentFence();

            // Flush the device for accuracy in the fences.
            device->commitAndNextCommandBuffer();
            mFences.push_back( fence );

            mUnfencedHazards.clear();
            mUnfencedBytes = 0;
        }
    }
    //-----------------------------------------------------------------------------------
    void VulkanStagingBuffer::deleteFences( VulkanFenceVec::iterator itor, VulkanFenceVec::iterator end )
    {
        VulkanVaoManager *vaoManager = static_cast<VulkanVaoManager *>( mVaoManager );
        VulkanDevice *device = vaoManager->getDevice();
        while( itor != end )
        {
            device->mGraphicsQueue.releaseFence( itor->fenceName );
            itor->fenceName = 0;
            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void VulkanStagingBuffer::wait( VkFence syncObj )
    {
        VulkanVaoManager *vaoManager = static_cast<VulkanVaoManager *>( mVaoManager );
        VulkanDevice *device = vaoManager->getDevice();

        VkResult result = vkWaitForFences( device->mDevice, 1, &syncObj, VK_TRUE,
                                           UINT64_MAX );  // You can't wait forever in Vulkan?!?
        checkVkResult( result, "VulkanStagingBuffer::wait" );
    }
    //-----------------------------------------------------------------------------------
    void VulkanStagingBuffer::waitIfNeeded()
    {
        assert( mUploadOnly );

        size_t mappingStart = mMappingStart;
        size_t sizeBytes = mMappingCount;

        if( mappingStart + sizeBytes > mSizeBytes )
        {
            if( !mUnfencedHazards.empty() )
            {
                // mUnfencedHazards will be cleared in addFence
                addFence( mUnfencedHazards.front().start, mSizeBytes - 1, true );
            }

            // Wraps around the ring buffer. Sadly we can't do advanced virtual memory
            // manipulation to keep the virtual space contiguous, so we'll have to reset to 0
            mappingStart = 0;
        }

        VulkanFence regionToMap( mappingStart, mappingStart + sizeBytes );

        VulkanFenceVec::iterator itor = mFences.begin();
        VulkanFenceVec::iterator endt = mFences.end();

        VulkanFenceVec::iterator lastWaitableFence = endt;

        while( itor != endt )
        {
            if( regionToMap.overlaps( *itor ) )
                lastWaitableFence = itor;

            ++itor;
        }

        if( lastWaitableFence != endt )
        {
            wait( lastWaitableFence->fenceName );
            deleteFences( mFences.begin(), lastWaitableFence + 1 );
            mFences.erase( mFences.begin(), lastWaitableFence + 1 );
        }

        mMappingStart = mappingStart;
    }
    //-----------------------------------------------------------------------------------
    void *VulkanStagingBuffer::mapImpl( size_t sizeBytes )
    {
        assert( mUploadOnly );

        mMappingCount = sizeBytes;

        OGRE_ASSERT_MEDIUM( mUnmapTicket == std::numeric_limits<size_t>::max() &&
                            "VulkanStagingBuffer still mapped!" );

        waitIfNeeded();  // Will fill mMappingStart

        mMappedPtr =
            mDynamicBuffer->map( mInternalBufferStart + mMappingStart, sizeBytes, mUnmapTicket );

        return mMappedPtr;
    }
    //-----------------------------------------------------------------------------------
    void VulkanStagingBuffer::unmapImpl( const Destination *destinations, size_t numDestinations )
    {
        VulkanVaoManager *vaoManager = static_cast<VulkanVaoManager *>( mVaoManager );
        VulkanDevice *device = vaoManager->getDevice();

        VkCommandBuffer cmdBuffer = device->mGraphicsQueue.mCurrentCmdBuffer;

        OGRE_ASSERT_MEDIUM( mUnmapTicket != std::numeric_limits<size_t>::max() &&
                            "VulkanStagingBuffer already unmapped!" );

        mDynamicBuffer->flush( mUnmapTicket, 0u, mMappingCount );
        mDynamicBuffer->unmap( mUnmapTicket );
        mUnmapTicket = std::numeric_limits<size_t>::max();
        mMappedPtr = 0;

        for( size_t i = 0; i < numDestinations; ++i )
        {
            const Destination &dst = destinations[i];

            VulkanBufferInterface *bufferInterface =
                static_cast<VulkanBufferInterface *>( dst.destination->getBufferInterface() );

            assert( dst.destination->getBufferType() == BT_DEFAULT );

            device->mGraphicsQueue.getCopyEncoder( dst.destination, 0, false );

            size_t dstOffset = dst.dstOffset + dst.destination->_getInternalBufferStart() *
                                                   dst.destination->getBytesPerElement();

            VkBufferCopy region;
            region.srcOffset = mInternalBufferStart + mMappingStart + dst.srcOffset;
            region.dstOffset = dstOffset;
            region.size = dst.length;
            vkCmdCopyBuffer( cmdBuffer, mVboName, bufferInterface->getVboName(), 1u, &region );
        }

        if( mUploadOnly )
        {
            // Add fence to this region (or at least, track the hazard).
            addFence( mMappingStart, mMappingStart + mMappingCount - 1, false );
        }
    }
    //-----------------------------------------------------------------------------------
    StagingStallType VulkanStagingBuffer::uploadWillStall( size_t sizeBytes )
    {
        assert( mUploadOnly );
        return STALL_NONE;
    }
    //-----------------------------------------------------------------------------------
    //
    //  DOWNLOADS
    //
    //-----------------------------------------------------------------------------------
    size_t VulkanStagingBuffer::_asyncDownload( BufferPacked *source, size_t srcOffset,
                                                size_t srcLength )
    {
        size_t freeRegionOffset = getFreeDownloadRegion( srcLength );
        size_t errorCode = (size_t)-1;

        if( freeRegionOffset == errorCode )
        {
            OGRE_EXCEPT(
                Exception::ERR_INVALIDPARAMS,
                "Cannot download the request amount of " + StringConverter::toString( srcLength ) +
                    " bytes to this staging buffer. "
                    "Try another one (we're full of requests that haven't been read by CPU yet)",
                "VulkanStagingBuffer::_asyncDownload" );
        }

        assert( !mUploadOnly );
        assert( dynamic_cast<VulkanBufferInterface *>( source->getBufferInterface() ) );
        assert( ( srcOffset + srcLength ) <= source->getTotalSizeBytes() );

        VulkanBufferInterface *bufferInterface =
            static_cast<VulkanBufferInterface *>( source->getBufferInterface() );

        VulkanVaoManager *vaoManager = static_cast<VulkanVaoManager *>( mVaoManager );
        VulkanDevice *device = vaoManager->getDevice();

        device->mGraphicsQueue.getCopyEncoder( source, 0, true );

        VkBufferCopy region;
        region.srcOffset = source->_getFinalBufferStart() * source->getBytesPerElement() + srcOffset;
        region.dstOffset = mInternalBufferStart + freeRegionOffset;
        region.size = srcLength;
        vkCmdCopyBuffer( device->mGraphicsQueue.mCurrentCmdBuffer, bufferInterface->getVboName(),
                         mVboName, 1u, &region );

        return freeRegionOffset;
    }
    //-----------------------------------------------------------------------------------
    void VulkanStagingBuffer::_unmapToV1( v1::VulkanHardwareBufferCommon *hwBuffer, size_t lockStart,
                                          size_t lockSize )
    {
        assert( mUploadOnly );

        if( mMappingState != MS_MAPPED )
        {
            // This stuff would normally be done in StagingBuffer::unmap
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE, "Unmapping an unmapped buffer!",
                         "VulkanStagingBuffer::unmap" );
        }

        mDynamicBuffer->flush( mUnmapTicket, 0u, mMappingCount );
        mDynamicBuffer->unmap( mUnmapTicket );
        mUnmapTicket = std::numeric_limits<size_t>::max();
        mMappedPtr = 0;

        VulkanVaoManager *vaoManager = static_cast<VulkanVaoManager *>( mVaoManager );
        VulkanDevice *device = vaoManager->getDevice();

        device->mGraphicsQueue.getCopyEncoderV1Buffer( false );

        size_t dstOffsetStart = 0;
        VkBuffer dstBuffer = hwBuffer->getBufferNameForGpuWrite( dstOffsetStart );

        VkBufferCopy region;
        region.srcOffset = mInternalBufferStart + mMappingStart;
        region.dstOffset = lockStart + dstOffsetStart;
        region.size = alignToNextMultiple( lockSize, 4u );
        vkCmdCopyBuffer( device->mGraphicsQueue.mCurrentCmdBuffer, mVboName, dstBuffer, 1u, &region );

        if( mUploadOnly )
        {
            // Add fence to this region (or at least, track the hazard).
            addFence( mMappingStart, mMappingStart + mMappingCount - 1, false );
        }

        // This stuff would normally be done in StagingBuffer::unmap
        mMappingState = MS_UNMAPPED;
        mMappingStart += mMappingCount;

        if( mMappingStart >= mSizeBytes )
            mMappingStart = 0;
    }
    //-----------------------------------------------------------------------------------
    size_t VulkanStagingBuffer::_asyncDownloadV1( v1::VulkanHardwareBufferCommon *source,
                                                  size_t srcOffset, size_t srcLength )
    {
        // Vulkan has alignment restrictions of 4 bytes for offset and size in copyFromBuffer
        size_t freeRegionOffset = getFreeDownloadRegion( srcLength );

        if( freeRegionOffset == ( size_t )( -1 ) )
        {
            OGRE_EXCEPT(
                Exception::ERR_INVALIDPARAMS,
                "Cannot download the request amount of " + StringConverter::toString( srcLength ) +
                    " bytes to this staging buffer. "
                    "Try another one (we're full of requests that haven't been read by CPU yet)",
                "VulkanStagingBuffer::_asyncDownload" );
        }

        assert( !mUploadOnly );
        assert( ( srcOffset + srcLength ) <= source->getSizeBytes() );

        size_t extraOffset = 0;
        if( srcOffset & 0x03 )
        {
            // Not multiple of 4. Backtrack to make it multiple of 4, then add this value
            // to the return value so it gets correctly mapped in _mapForRead.
            extraOffset = srcOffset & 0x03;
            srcOffset -= extraOffset;
        }

        size_t srcOffsetStart = 0;

        VulkanVaoManager *vaoManager = static_cast<VulkanVaoManager *>( mVaoManager );
        VulkanDevice *device = vaoManager->getDevice();

        device->mGraphicsQueue.getCopyEncoderV1Buffer( true );

        VkBuffer srcBuffer = source->getBufferName( srcOffsetStart );

        VkBufferCopy region;
        region.srcOffset = srcOffset + srcOffsetStart;
        region.dstOffset = mInternalBufferStart + freeRegionOffset;
        region.size = alignToNextMultiple( srcLength, 4u );
        vkCmdCopyBuffer( device->mGraphicsQueue.mCurrentCmdBuffer, srcBuffer, mVboName, 1u, &region );

        return freeRegionOffset + extraOffset;
    }
    //-----------------------------------------------------------------------------------
    void VulkanStagingBuffer::_notifyDeviceStalled()
    {
        deleteFences( mFences.begin(), mFences.end() );
        mFences.clear();
        mUnfencedHazards.clear();
        mUnfencedBytes = 0;
    }
    //-----------------------------------------------------------------------------------
    const void *VulkanStagingBuffer::_mapForReadImpl( size_t offset, size_t sizeBytes )
    {
        assert( !mUploadOnly );

        mMappingStart = offset;
        mMappingCount = sizeBytes;

        OGRE_ASSERT_MEDIUM( mUnmapTicket == std::numeric_limits<size_t>::max() &&
                            "VulkanStagingBuffer::_mapForReadImpl still mapped!" );
        mMappedPtr =
            mDynamicBuffer->map( mInternalBufferStart + mMappingStart, mMappingCount, mUnmapTicket );

        // Put the mapped region back to our records as "available" for subsequent _asyncDownload
        _cancelDownload( offset, sizeBytes );

        return mMappedPtr;
    }
}  // namespace Ogre
