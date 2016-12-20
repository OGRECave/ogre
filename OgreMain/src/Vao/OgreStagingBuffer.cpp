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

#include "OgreStableHeaders.h"
#include "Vao/OgreStagingBuffer.h"
#include "Vao/OgreVaoManager.h"
#include "OgreException.h"
#include "OgreStringConverter.h"
#include "OgreTimer.h"

namespace Ogre
{
    StagingBuffer::StagingBuffer( size_t internalBufferStart, size_t sizeBytes,
                                  VaoManager *vaoManager, bool uploadOnly ) :
        mInternalBufferStart( internalBufferStart ),
        mSizeBytes( sizeBytes ),
        mUploadOnly( uploadOnly ),
        mVaoManager( vaoManager ),
        mMappingState( MS_UNMAPPED ),
        mMappingStart( 0 ),
        mMappingCount( 0 ),
        mRefCount( 1 ),
        mUnfenceTimeThreshold( vaoManager->getDefaultStagingBufferUnfencedTime() ),
        mLifetimeThreshold( vaoManager->getDefaultStagingBufferLifetime() ),
        mLastUsedTimestamp( vaoManager->getTimer()->getMilliseconds() )
    {
        if( !mUploadOnly )
            mAvailableDownloadRegions.push_back( Fence( 0, mSizeBytes ) );
    }
    //-----------------------------------------------------------------------------------
    StagingBuffer::~StagingBuffer()
    {
    }
    //-----------------------------------------------------------------------------------
    void StagingBuffer::mapChecks( size_t sizeBytes )
    {
        if( !sizeBytes )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "StagingBuffer cannot map 0 bytes",
                         "StagingBuffer::mapChecks" );
        }

        if( sizeBytes > mSizeBytes )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "StagingBuffer (" + StringConverter::toString( mSizeBytes ) +
                         " bytes) is smaller than the mapping request (" +
                         StringConverter::toString( sizeBytes ) + ")",
                         "StagingBuffer::mapChecks" );
        }

        if( mMappingState != MS_UNMAPPED )
        {
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                         "StagingBuffer is already mapped. You can't"
                         " call this function while it's mapped",
                         "StagingBuffer::mapChecks" );
        }
    }
    //-----------------------------------------------------------------------------------
    StagingStallType StagingBuffer::uploadWillStall( size_t sizeBytes )
    {
        assert( mUploadOnly );
        return STALL_PARTIAL;
    }
    //-----------------------------------------------------------------------------------
    void* StagingBuffer::map( size_t sizeBytes )
    {
        assert( mUploadOnly );

        if( mMappingState != MS_UNMAPPED )
        {
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE, "Buffer already mapped!",
                         "StagingBuffer::map" );
        }

        mapChecks( sizeBytes );
        mMappingState = MS_MAPPED;
        return mapImpl( sizeBytes );
    }
    //-----------------------------------------------------------------------------------
    void StagingBuffer::unmap( const Destination *destinations, size_t numDestinations )
    {
        if( mMappingState != MS_MAPPED )
        {
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE, "Unmapping an unmapped buffer!",
                         "StagingBuffer::unmap" );
        }

        assert( ( (!mUploadOnly && !destinations && !numDestinations) ||
                  (mUploadOnly && destinations && numDestinations) ) &&
                "Using an upload staging-buffer for downloads or vice-versa." );

        unmapImpl( destinations, numDestinations );

        mMappingState = MS_UNMAPPED;
        mMappingStart += mMappingCount;

        if( mMappingStart >= mSizeBytes )
            mMappingStart = 0;
    }
    //-----------------------------------------------------------------------------------
    void StagingBuffer::addReferenceCount(void)
    {
        ++mRefCount;

        if( mRefCount == 1 )
            mVaoManager->_notifyStagingBufferLeftZeroRef( this );
    }
    //-----------------------------------------------------------------------------------
    void StagingBuffer::removeReferenceCount(void)
    {
        Timer *timer = mVaoManager->getTimer();
        --mRefCount;

        assert( mRefCount >= 0 );
        mLastUsedTimestamp = timer->getMilliseconds();

        if( mRefCount == 0 )
            mVaoManager->_notifyStagingBufferEnteredZeroRef( this );
    }
    //-----------------------------------------------------------------------------------
    //
    //  DOWNLOADS
    //
    //-----------------------------------------------------------------------------------
    bool StagingBuffer::canDownload( size_t length ) const
    {
        assert( !mUploadOnly );

        FenceVec::const_iterator itor = mAvailableDownloadRegions.begin();
        FenceVec::const_iterator end  = mAvailableDownloadRegions.end();

        while( itor != end && length > itor->length() )
            ++itor;

        return itor != end;
    }
    //-----------------------------------------------------------------------------------
    void StagingBuffer::_cancelDownload( size_t offset, size_t sizeBytes )
    {
        //Put the mapped region back to our records as "available" for subsequent _asyncDownload
        Fence mappedArea( offset, offset + sizeBytes );
#if OGRE_DEBUG_MODE
        FenceVec::const_iterator itor = mAvailableDownloadRegions.begin();
        FenceVec::const_iterator end  = mAvailableDownloadRegions.end();

        while( itor != end )
        {
            assert( !itor->overlaps( mappedArea ) &&
                    "Already called _mapForReadImpl on this area (or part of it!) before!" );
            ++itor;
        }
#endif

        mAvailableDownloadRegions.push_back( mappedArea );

        mergeContiguousBlocks( mAvailableDownloadRegions.end() - 1, mAvailableDownloadRegions );
    }
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
    //-----------------------------------------------------------------------------------
    size_t StagingBuffer::getFreeDownloadRegion( size_t length )
    {
        //Grab the smallest region that fits the request.
        size_t lowestLength = std::numeric_limits<size_t>::max();
        FenceVec::iterator itor = mAvailableDownloadRegions.begin();
        FenceVec::iterator end  = mAvailableDownloadRegions.end();

        FenceVec::iterator itLowest = end;

        while( itor != end )
        {
            size_t freeRegionLength = itor->length();
            if( length <= freeRegionLength && freeRegionLength < lowestLength )
            {
                itLowest = itor;
                lowestLength = freeRegionLength;
            }

            ++itor;
        }

        size_t retVal = -1;

        if( itLowest != end )
        {
            //Got a region! Shrink our records
            retVal = itLowest->start;
            itLowest->start += length;

            //This region is empty. Remove it.
            if( itLowest->start == itLowest->end )
                efficientVectorRemove( mAvailableDownloadRegions, itLowest );
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void StagingBuffer::mergeContiguousBlocks( FenceVec::iterator blockToMerge, FenceVec &blocks )
    {
        FenceVec::iterator itor = blocks.begin();
        FenceVec::iterator end  = blocks.end();

        while( itor != end )
        {
            if( itor->end == blockToMerge->start )
            {
                itor->end = blockToMerge->end;
                size_t idx = itor - blocks.begin();

                //When blockToMerge is the last one, its index won't be the same
                //after removing the other iterator, they will swap.
                if( idx == blocks.size() - 1 )
                    idx = blockToMerge - blocks.begin();

                efficientVectorRemove( blocks, blockToMerge );

                blockToMerge = blocks.begin() + idx;
                itor = blocks.begin();
                end  = blocks.end();
            }
            else if( blockToMerge->end == itor->start )
            {
                blockToMerge->end = itor->end;
                size_t idx = blockToMerge - blocks.begin();

                //When blockToMerge is the last one, its index won't be the same
                //after removing the other iterator, they will swap.
                if( idx == blocks.size() - 1 )
                    idx = itor - blocks.begin();

                efficientVectorRemove( blocks, itor );

                blockToMerge = blocks.begin() + idx;
                itor = blocks.begin();
                end  = blocks.end();
            }
            else
            {
                ++itor;
            }
        }
    }
}
