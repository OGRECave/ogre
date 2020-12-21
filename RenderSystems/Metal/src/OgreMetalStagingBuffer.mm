/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2016 Torus Knot Software Ltd

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

#include "OgreMetalStagingBuffer.h"

#include "OgreMetalHardwareBufferCommon.h"
#include "OgreMetalDevice.h"

#include "OgreStringConverter.h"

#import <Metal/MTLBlitCommandEncoder.h>
#import <Metal/MTLCommandBuffer.h>

namespace Ogre
{
    //-----------------------------------------------------------------------------------
    const void* StagingBuffer::_mapForRead( size_t offset, size_t sizeBytes )
    {
        assert( !mUploadOnly );

        if( mMappingState != MS_UNMAPPED )
        {
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE, "Buffer already mapped!",
                         "StagingBuffer::_mapForRead" );
        }

        mMappingState = MS_MAPPED;

        return _mapForReadImpl( offset, sizeBytes );
    }
    void* StagingBuffer::map( size_t sizeBytes )
    {
        assert( mUploadOnly );

        if( mMappingState != MS_UNMAPPED )
        {
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE, "Buffer already mapped!",
                         "StagingBuffer::map" );
        }

        // mapChecks( sizeBytes );
        mMappingState = MS_MAPPED;
        return mapImpl( sizeBytes );
    }

    MetalStagingBuffer::MetalStagingBuffer( size_t internalBufferStart, size_t sizeBytes,
                                            bool uploadOnly,
                                            id<MTLBuffer> vboName, MetalDevice *device ) :
        mVboName( vboName ),
        mMappedPtr( 0 ),
        mDevice( device ),
        mMappingState( MS_UNMAPPED ),
        mMappingStart( 0 ),
        mMappingCount( 0 ),
        mFenceThreshold( sizeBytes / 4 ),
        mUnfencedBytes( 0 )
    {
        mInternalBufferStart = internalBufferStart;
        mSizeBytes = sizeBytes;
        mUploadOnly = uploadOnly;
    }
    //-----------------------------------------------------------------------------------
    MetalStagingBuffer::~MetalStagingBuffer()
    {
        if( !mFences.empty() )
            wait( mFences.back().fenceName );

        deleteFences( mFences.begin(), mFences.end() );

        [mVboName setPurgeableState:MTLPurgeableStateEmpty];
        mVboName = 0;
    }
    //-----------------------------------------------------------------------------------
    void MetalStagingBuffer::addFence( size_t from, size_t to, bool forceFence )
    {
        assert( to <= mSizeBytes );

        MetalFence unfencedHazard( from, to );

        mUnfencedHazards.push_back( unfencedHazard );

        assert( from <= to );

        mUnfencedBytes += to - from;

        if( mUnfencedBytes >= mFenceThreshold || forceFence )
        {
            MetalFenceVec::const_iterator itor = mUnfencedHazards.begin();
            MetalFenceVec::const_iterator end  = mUnfencedHazards.end();

            size_t startRange = itor->start;
            size_t endRange   = itor->end;

            while( itor != end )
            {
                if( endRange <= itor->end )
                {
                    //Keep growing (merging) the fences into one fence
                    endRange = itor->end;
                }
                else
                {
                    //We wrapped back to 0. Can't keep merging. Make a fence.
                    MetalFence fence( startRange, endRange );
                    fence.fenceName = dispatch_semaphore_create( 0 );

                    __block dispatch_semaphore_t blockSemaphore = fence.fenceName;
                    [mDevice->mCurrentCommandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer)
                    {
                        dispatch_semaphore_signal( blockSemaphore );
                    }];
                    mFences.push_back( fence );

                    startRange  = itor->start;
                    endRange    = itor->end;
                }

                ++itor;
            }

            //Make the last fence.
            MetalFence fence( startRange, endRange );
            fence.fenceName = dispatch_semaphore_create( 0 );
            __block dispatch_semaphore_t blockSemaphore = fence.fenceName;
            [mDevice->mCurrentCommandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer)
            {
                dispatch_semaphore_signal( blockSemaphore );
            }];

            //Flush the device for accuracy in the fences.
            mDevice->commitAndNextCommandBuffer();
            mFences.push_back( fence );

            mUnfencedHazards.clear();
            mUnfencedBytes = 0;
        }
    }
    //-----------------------------------------------------------------------------------
    void MetalStagingBuffer::wait( dispatch_semaphore_t syncObj )
    {
        long result = dispatch_semaphore_wait( syncObj, DISPATCH_TIME_FOREVER );

        if( result != 0 )
        {
            OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                         "Failure while waiting for a MetalFence. Could be out of GPU memory. "
                         "Update your video card drivers. If that doesn't help, "
                         "contact the developers. Error code: " + StringConverter::toString( result ),
                         "MetalStagingBuffer::wait" );
        }
    }
    //-----------------------------------------------------------------------------------
    void MetalStagingBuffer::deleteFences( MetalFenceVec::iterator itor, MetalFenceVec::iterator end )
    {
        while( itor != end )
        {
            //ARC will destroy it
            itor->fenceName = 0;
            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void MetalStagingBuffer::waitIfNeeded(void)
    {
        assert( mUploadOnly );

        size_t mappingStart = mMappingStart;
        size_t sizeBytes    = mMappingCount;

        if( mappingStart + sizeBytes > mSizeBytes )
        {
            if( !mUnfencedHazards.empty() )
            {
                //mUnfencedHazards will be cleared in addFence
                addFence( mUnfencedHazards.front().start, mSizeBytes - 1, true );
            }

            //Wraps around the ring buffer. Sadly we can't do advanced virtual memory
            //manipulation to keep the virtual space contiguous, so we'll have to reset to 0
            mappingStart = 0;
        }

        MetalFence regionToMap( mappingStart, mappingStart + sizeBytes );

        MetalFenceVec::iterator itor = mFences.begin();
        MetalFenceVec::iterator end  = mFences.end();

        MetalFenceVec::iterator lastWaitableFence = end;

        while( itor != end )
        {
            if( regionToMap.overlaps( *itor ) )
                lastWaitableFence = itor;

            ++itor;
        }

        if( lastWaitableFence != end )
        {
            wait( lastWaitableFence->fenceName );
            deleteFences( mFences.begin(), lastWaitableFence + 1 );
            mFences.erase( mFences.begin(), lastWaitableFence + 1 );
        }

        mMappingStart = mappingStart;
    }
    //-----------------------------------------------------------------------------------
    void* MetalStagingBuffer::mapImpl( size_t sizeBytes )
    {
        assert( mUploadOnly );

        mMappingCount = alignToNextMultiple( sizeBytes, 4u );

        waitIfNeeded(); //Will fill mMappingStart

        mMappedPtr = reinterpret_cast<uint8*>( [mVboName contents] ) +
                mInternalBufferStart + mMappingStart;

        return mMappedPtr;
    }
    //-----------------------------------------------------------------------------------
    StagingStallType MetalStagingBuffer::uploadWillStall( size_t sizeBytes )
    {
        assert( mUploadOnly );

        size_t mappingStart = mMappingStart;

        StagingStallType retVal = STALL_NONE;

        if( mappingStart + sizeBytes > mSizeBytes )
        {
            if( !mUnfencedHazards.empty() )
            {
                MetalFence regionToMap( 0, sizeBytes );
                MetalFence hazardousRegion( mUnfencedHazards.front().start, mSizeBytes - 1 );

                if( hazardousRegion.overlaps( regionToMap ) )
                {
                    retVal = STALL_FULL;
                    return retVal;
                }
            }

            mappingStart = 0;
        }

        MetalFence regionToMap( mappingStart, mappingStart + sizeBytes );

        MetalFenceVec::iterator itor = mFences.begin();
        MetalFenceVec::iterator end  = mFences.end();

        MetalFenceVec::iterator lastWaitableFence = end;

        while( itor != end )
        {
            if( regionToMap.overlaps( *itor ) )
                lastWaitableFence = itor;

            ++itor;
        }

        if( lastWaitableFence != end )
        {
            //Ask Metal to return immediately and tells us about the fence
            long waitRet = dispatch_semaphore_wait( lastWaitableFence->fenceName, DISPATCH_TIME_NOW );

            if( waitRet != 0 )
            {
                retVal = STALL_PARTIAL;
            }
            else
            {
                deleteFences( mFences.begin(), lastWaitableFence + 1 );
                mFences.erase( mFences.begin(), lastWaitableFence + 1 );
            }
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void MetalStagingBuffer::cleanUnfencedHazards(void)
    {
        if( !mUnfencedHazards.empty() )
            addFence( mUnfencedHazards.front().start, mUnfencedHazards.back().end, true );
    }
    //-----------------------------------------------------------------------------------
    void MetalStagingBuffer::_notifyDeviceStalled(void)
    {
        deleteFences( mFences.begin(), mFences.end() );
        mFences.clear();
        mUnfencedHazards.clear();
        mUnfencedBytes = 0;
    }
    //-----------------------------------------------------------------------------------
    void MetalStagingBuffer::_unmapToV1( MetalHardwareBufferCommon *hwBuffer,
                                         size_t lockStart, size_t lockSize )
    {
        assert( mUploadOnly );

        if( mMappingState != MS_MAPPED )
        {
            //This stuff would normally be done in StagingBuffer::unmap
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE, "Unmapping an unmapped buffer!",
                         "MetalStagingBuffer::unmap" );
        }

//        if( mUploadOnly )
//        {
//#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS
//            NSRange range = NSMakeRange( mInternalBufferStart + mMappingStart, mMappingCount );
//            [mVboName didModifyRange:range];
//#endif
//        }

        mMappedPtr = 0;

        __unsafe_unretained id<MTLBlitCommandEncoder> blitEncoder = mDevice->getBlitEncoder();
        [blitEncoder copyFromBuffer:mVboName
                                    sourceOffset:mInternalBufferStart + mMappingStart
                                    toBuffer:hwBuffer->getBufferNameForGpuWrite()
                                    destinationOffset:lockStart
                                    size:alignToNextMultiple( lockSize, 4u )];

        if( mUploadOnly )
        {
            //Add fence to this region (or at least, track the hazard).
            addFence( mMappingStart, mMappingStart + mMappingCount - 1, false );
        }

        //This stuff would normally be done in StagingBuffer::unmap
        mMappingState = MS_UNMAPPED;
        mMappingStart += mMappingCount;

        if( mMappingStart >= mSizeBytes )
            mMappingStart = 0;
    }
    //-----------------------------------------------------------------------------------
    //
    //  DOWNLOADS
    //
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    const void* MetalStagingBuffer::_mapForReadImpl( size_t offset, size_t sizeBytes )
    {
        assert( !mUploadOnly );

        mMappingStart = offset;
        mMappingCount = alignToNextMultiple( sizeBytes, 4u );

        mMappedPtr = reinterpret_cast<uint8*>( [mVboName contents] ) +
                mInternalBufferStart + mMappingStart;

        //Put the mapped region back to our records as "available" for subsequent _asyncDownload
        //_cancelDownload( offset, sizeBytes );

        return mMappedPtr;
    }
    //-----------------------------------------------------------------------------------
    size_t MetalStagingBuffer::_asyncDownloadV1( MetalHardwareBufferCommon *source,
                                                 size_t srcOffset, size_t srcLength )
    {
        //Metal has alignment restrictions of 4 bytes for offset and size in copyFromBuffer
        size_t freeRegionOffset = /*getFreeDownloadRegion*/( alignToNextMultiple( srcLength, 4u ) );

        if( freeRegionOffset == (size_t)(-1) )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "Cannot download the request amount of " +
                         StringConverter::toString( srcLength ) + " bytes to this staging buffer. "
                         "Try another one (we're full of requests that haven't been read by CPU yet)",
                         "MetalStagingBuffer::_asyncDownload" );
        }

        assert( !mUploadOnly );
        assert( (srcOffset + srcLength) <= source->getSizeInBytes() );

        size_t extraOffset = 0;
        if( srcOffset & 0x03 )
        {
            //Not multiple of 4. Backtrack to make it multiple of 4, then add this value
            //to the return value so it gets correctly mapped in _mapForRead.
            extraOffset = srcOffset & 0x03;
            srcOffset -= extraOffset;
        }

        size_t srcOffsetStart = 0;
        __unsafe_unretained id<MTLBuffer> srcBuffer = source->getBufferName( srcOffsetStart );

        __unsafe_unretained id<MTLBlitCommandEncoder> blitEncoder = mDevice->getBlitEncoder();
        [blitEncoder copyFromBuffer:srcBuffer
                                    sourceOffset:srcOffset + srcOffsetStart
                                    toBuffer:mVboName
                                    destinationOffset:mInternalBufferStart + freeRegionOffset
                                    size:alignToNextMultiple( srcLength, 4u )];
#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS
        [blitEncoder synchronizeResource:mVboName];
#endif

        return freeRegionOffset + extraOffset;
    }
}
