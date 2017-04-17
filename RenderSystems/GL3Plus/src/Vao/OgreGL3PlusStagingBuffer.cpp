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
        mFenceThreshold( sizeBytes / 4 ),
        mUnfencedBytes( 0 )
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

        GLFence unfencedHazard( from, to );

        mUnfencedHazards.push_back( unfencedHazard );

        assert( from <= to );

        mUnfencedBytes += to - from;

        if( mUnfencedBytes >= mFenceThreshold || forceFence )
        {
            GLFenceVec::const_iterator itor = mUnfencedHazards.begin();
            GLFenceVec::const_iterator end  = mUnfencedHazards.end();

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
                    GLFence fence( startRange, endRange );
                    OCGE( fence.fenceName = glFenceSync( GL_SYNC_GPU_COMMANDS_COMPLETE, 0 ) );
                    mFences.push_back( fence );

                    startRange  = itor->start;
                    endRange    = itor->end;
                }

                ++itor;
            }

            //Make the last fence.
            GLFence fence( startRange, endRange );
            OCGE( fence.fenceName = glFenceSync( GL_SYNC_GPU_COMMANDS_COMPLETE, 0 ) );
            mFences.push_back( fence );

            mUnfencedHazards.clear();
            mUnfencedBytes = 0;
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
                             "Failure while waiting for a GL GLFence. Could be out of GPU memory. "
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
    void GL3PlusStagingBuffer::deleteFences( GLFenceVec::iterator itor, GLFenceVec::iterator end )
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

        GLFence regionToMap( mappingStart, mappingStart + sizeBytes );

        GLFenceVec::iterator itor = mFences.begin();
        GLFenceVec::iterator end  = mFences.end();

        GLFenceVec::iterator lastWaitableFence = end;

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

        if( mUploadOnly )
        {
            OCGE( glFlushMappedBufferRange( target,
                                            0 /*mInternalBufferStart + mMappingStart*/,
                                            mMappingCount ) );
        }

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

        if( mUploadOnly )
        {
            //Add fence to this region (or at least, track the hazard).
            addFence( mMappingStart, mMappingStart + mMappingCount - 1, false );
        }
    }
    //-----------------------------------------------------------------------------------
    StagingStallType GL3PlusStagingBuffer::uploadWillStall( size_t sizeBytes )
    {
        assert( mUploadOnly );

        size_t mappingStart = mMappingStart;

        StagingStallType retVal = STALL_NONE;

        if( mappingStart + sizeBytes > mSizeBytes )
        {
            if( !mUnfencedHazards.empty() )
            {
                GLFence regionToMap( 0, sizeBytes );
                GLFence hazardousRegion( mUnfencedHazards.front().start, mSizeBytes - 1 );

                if( hazardousRegion.overlaps( regionToMap ) )
                {
                    retVal = STALL_FULL;
                    return retVal;
                }
            }

            mappingStart = 0;
        }

        GLFence regionToMap( mappingStart, mappingStart + sizeBytes );

        GLFenceVec::const_iterator itor = mFences.begin();
        GLFenceVec::const_iterator end  = mFences.end();

        GLFenceVec::const_iterator lastWaitableFence = end;

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
    size_t GL3PlusStagingBuffer::_asyncDownload( BufferPacked *source, size_t srcOffset,
                                                 size_t srcLength )
    {
        size_t freeRegionOffset = getFreeDownloadRegion( srcLength );

        if( freeRegionOffset == (size_t)(-1) )
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

        OCGE( glCopyBufferSubData( GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
                                   source->_getFinalBufferStart() *
                                    source->getBytesPerElement() + srcOffset,
                                   mInternalBufferStart + freeRegionOffset,
                                   srcLength ) );

        return freeRegionOffset;
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
}
