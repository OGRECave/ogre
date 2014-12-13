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

#include "Vao/OgreGL3PlusStagingBuffer.h"
#include "Vao/OgreGL3PlusVaoManager.h"
#include "Vao/OgreGL3PlusBufferInterface.h"

#include "OgreStringConverter.h"

namespace Ogre
{
    extern const GLuint64 kOneSecondInNanoSeconds;

    GL3PlusStagingBuffer::GL3PlusStagingBuffer( size_t internalBufferStart, size_t sizeBytes,
                                                VaoManager *vaoManager, bool uploadOnly,
                                                GLuint vboName ) :
        StagingBuffer( internalBufferStart, sizeBytes, vaoManager, uploadOnly ),
        mVboName( vboName ),
        mMappedPtr( 0 ),
        mFenceThreshold( sizeBytes / 4 )
    {
    }
    //-----------------------------------------------------------------------------------
    GL3PlusStagingBuffer::~GL3PlusStagingBuffer()
    {
        if( !mFences.empty() )
            wait( mFences.back().fenceName );

        deleteFences( mFences.begin(), mFences.end() );
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusStagingBuffer::addFence( size_t from, size_t to, bool forceFence )
    {
        assert( to <= mSizeBytes );

        Fence unfencedHazard( from, to );

        mUnfencedHazards.push_back( unfencedHazard );

        size_t start = mUnfencedHazards.front().start;
        size_t end   = mUnfencedHazards.back().end;

        assert( start <= end );

        if( end - start >= mFenceThreshold || forceFence )
        {
            Fence fence( start, end );
            OCGE( fence.fenceName = glFenceSync( GL_SYNC_GPU_COMMANDS_COMPLETE, 0 ) );
            mFences.push_back( fence );

            mUnfencedHazards.clear();
        }
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusStagingBuffer::wait( GLsync syncObj )
    {
        GLbitfield waitFlags    = 0;
        GLuint64 waitDuration   = 0;
        while( true )
        {
            GLenum waitRet = glClientWaitSync( syncObj, waitFlags, waitDuration );
            if( waitRet == GL_ALREADY_SIGNALED || waitRet == GL_CONDITION_SATISFIED )
            {
                return;
            }

            if( waitRet == GL_WAIT_FAILED )
            {
                OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                             "Failure while waiting for a GL Fence. Could be out of GPU memory. "
                             "Update your video card drivers. If that doesn't help, "
                             "contact the developers.", "GL3PlusStagingBuffer::wait" );
                return;
            }

            // After the first time, need to start flushing, and wait for a looong time.
            waitFlags = GL_SYNC_FLUSH_COMMANDS_BIT;
            waitDuration = kOneSecondInNanoSeconds;
        }
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusStagingBuffer::deleteFences( FenceVec::iterator itor, FenceVec::iterator end )
    {
        while( itor != end )
        {
            if( itor->fenceName )
                glDeleteSync( itor->fenceName );
            itor->fenceName = 0;
            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusStagingBuffer::waitIfNeeded(void)
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

        Fence regionToMap( mappingStart, mappingStart + sizeBytes );

        FenceVec::iterator itor = mFences.begin();
        FenceVec::iterator end  = mFences.end();

        FenceVec::iterator lastWaitableFence = end;

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
    void* GL3PlusStagingBuffer::mapImpl( size_t sizeBytes )
    {
        assert( mUploadOnly );

        GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT |
                           GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_FLUSH_EXPLICIT_BIT;

        mMappingCount = sizeBytes;

        waitIfNeeded(); //Will fill mMappingStart

        OCGE( glBindBuffer( GL_COPY_WRITE_BUFFER, mVboName ) );
        OCGE( mMappedPtr = glMapBufferRange( GL_COPY_WRITE_BUFFER, mInternalBufferStart + mMappingStart,
                                              mMappingCount, flags ) );

        return mMappedPtr;
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusStagingBuffer::unmapImpl( const Destination *destinations, size_t numDestinations )
    {
        GLenum target = mUploadOnly ? GL_COPY_WRITE_BUFFER : GL_COPY_READ_BUFFER;
        GLenum oppositeTarget = mUploadOnly ? GL_COPY_READ_BUFFER : GL_COPY_WRITE_BUFFER;

        OCGE( glBindBuffer( target, mVboName ) );
        OCGE( glFlushMappedBufferRange( target,
                                         0 /*mInternalBufferStart + mMappingStart*/,
                                         mMappingCount ) );
        OCGE( glUnmapBuffer( target ) );
        mMappedPtr = 0;

        for( size_t i=0; i<numDestinations; ++i )
        {
            const Destination &dst = destinations[i];

            GL3PlusBufferInterface *bufferInterface = static_cast<GL3PlusBufferInterface*>(
                                                        dst.destination->getBufferInterface() );

            assert( dst.destination->getBufferType() == BT_DEFAULT );

            GLintptr dstOffset = dst.dstOffset + dst.destination->_getInternalBufferStart() *
                                                        dst.destination->getBytesPerElement();

            OCGE( glBindBuffer( oppositeTarget, bufferInterface->getVboName() ) );
            OCGE( glCopyBufferSubData( target, oppositeTarget,
                                        mInternalBufferStart + mMappingStart + dst.srcOffset,
                                        dstOffset, dst.length ) );
        }

        if( !mUploadOnly )
        {
            //Add fence to this region (or at least, track the hazard).
            addFence( mMappingStart, mMappingStart + mMappingCount - 1, false );
        }
    }
    //-----------------------------------------------------------------------------------
    StagingStallType GL3PlusStagingBuffer::uploadWillStall( size_t sizeBytes ) const
    {
        assert( mUploadOnly );

        size_t mappingStart = mMappingStart;

        StagingStallType retVal = STALL_NONE;

        if( mappingStart + sizeBytes > mSizeBytes )
        {
            if( !mUnfencedHazards.empty() )
            {
                Fence regionToMap( 0, sizeBytes );
                Fence hazardousRegion( mUnfencedHazards.front().start, mSizeBytes - 1 );

                if( hazardousRegion.overlaps( regionToMap ) )
                {
                    retVal = STALL_FULL;
                    return retVal;
                }
            }

            mappingStart = 0;
        }

        Fence regionToMap( mappingStart, mappingStart + sizeBytes );

        FenceVec::const_iterator itor = mFences.begin();
        FenceVec::const_iterator end  = mFences.end();

        FenceVec::const_iterator lastWaitableFence = end;

        while( itor != end )
        {
            if( regionToMap.overlaps( *itor ) )
                lastWaitableFence = itor;

            ++itor;
        }

        if( lastWaitableFence != end )
        {
            //Ask GL API to return immediately and tells us about the fence
            GLenum waitRet = glClientWaitSync( lastWaitableFence->fenceName, 0, 0 );
            if( waitRet != GL_ALREADY_SIGNALED && waitRet != GL_CONDITION_SATISFIED )
                retVal = STALL_PARTIAL;
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusStagingBuffer::cleanUnfencedHazards(void)
    {
        if( !mUnfencedHazards.empty() )
            addFence( mUnfencedHazards.front().start, mUnfencedHazards.back().end, true );
    }
    //-----------------------------------------------------------------------------------
    //
    //  DOWNLOADS
    //
    //-----------------------------------------------------------------------------------
    bool GL3PlusStagingBuffer::canDownload( size_t length ) const
    {
        assert( !mUploadOnly );

        FenceVec::const_iterator itor = mAvailableDownloadRegions.begin();
        FenceVec::const_iterator end  = mAvailableDownloadRegions.end();

        while( itor != end && length > itor->length() )
            ++itor;

        return itor != end;
    }
    //-----------------------------------------------------------------------------------
    size_t GL3PlusStagingBuffer::_asyncDownload( BufferPacked *source, size_t srcOffset,
                                                 size_t srcLength )
    {
        size_t freeRegionOffset = getFreeDownloadRegion( srcLength );

        if( freeRegionOffset == -1 )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "Cannot download the request amount of " +
                         StringConverter::toString( srcLength ) + " bytes to this staging buffer. "
                         "Try another one (we're full of requests that haven't been read by CPU yet)",
                         "GL3PlusStagingBuffer::_asyncDownload" );
        }

        assert( !mUploadOnly );
        assert( dynamic_cast<GL3PlusBufferInterface*>( source->getBufferInterface() ) );
        assert( (srcOffset + srcLength) <= source->getTotalSizeBytes() );

        GL3PlusBufferInterface *bufferInterface = static_cast<GL3PlusBufferInterface*>(
                                                            source->getBufferInterface() );

        OCGE( glBindBuffer( GL_COPY_WRITE_BUFFER, mVboName ) );
        OCGE( glBindBuffer( GL_COPY_READ_BUFFER, bufferInterface->getVboName() ) );

        OCGE( glCopyBufferSubData( GL_COPY_WRITE_BUFFER, GL_COPY_READ_BUFFER,
                                   source->_getFinalBufferStart() *
                                    source->getBytesPerElement() + srcOffset,
                                   mInternalBufferStart + freeRegionOffset,
                                   srcLength ) );

        return freeRegionOffset;
    }
    //-----------------------------------------------------------------------------------
    size_t GL3PlusStagingBuffer::getFreeDownloadRegion( size_t length )
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
    void GL3PlusStagingBuffer::_cancelDownload( size_t offset, size_t sizeBytes )
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
    const void* GL3PlusStagingBuffer::_mapForReadImpl( size_t offset, size_t sizeBytes )
    {
        assert( !mUploadOnly );
        GLbitfield flags;

        //TODO: Reading + Persistency is supported, unsynchronized is not.
        flags = GL_MAP_READ_BIT;

        mMappingStart = offset;
        mMappingCount = sizeBytes;

        OCGE( glBindBuffer( GL_COPY_READ_BUFFER, mVboName ) );
        OCGE( mMappedPtr = glMapBufferRange( GL_COPY_READ_BUFFER, mInternalBufferStart + mMappingStart,
                                             mMappingCount, flags ) );

        //Put the mapped region back to our records as "available" for subsequent _asyncDownload
        _cancelDownload( offset, sizeBytes );

        return mMappedPtr;
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusStagingBuffer::mergeContiguousBlocks( FenceVec::iterator blockToMerge,
                                                      FenceVec &blocks )
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
                blockToMerge->end += itor->start;
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
