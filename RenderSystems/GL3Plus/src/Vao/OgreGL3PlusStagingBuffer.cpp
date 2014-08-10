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

namespace Ogre
{
    const GLuint64 kOneSecondInNanoSeconds = 1000000000;

    GL3PlusStagingBuffer::GL3PlusStagingBuffer( size_t internalBufferStart, size_t sizeBytes,
                                                VaoManager *vaoManager, bool uploadOnly,
                                                GLuint vboName ) :
        StagingBuffer( internalBufferStart, sizeBytes, vaoManager, uploadOnly ),
        mVboName( vboName ),
        mMappedPtr( 0 )
    {
    }
    //-----------------------------------------------------------------------------------
    GL3PlusStagingBuffer::~GL3PlusStagingBuffer()
    {
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
            OCGLE( fence.fenceName = glFenceSync( GL_SYNC_GPU_COMMANDS_COMPLETE, 0 ) );
            mFences.push_back( fence );
            fence.fenceName = 0; //Prevent the destructor from deleting the sync.

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
    void GL3PlusStagingBuffer::waitIfNeeded(void)
    {
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
            mFences.erase( mFences.begin(), lastWaitableFence + 1 );
        }

        mMappingStart = mappingStart;
    }
    //-----------------------------------------------------------------------------------
    void* GL3PlusStagingBuffer::mapImpl( size_t sizeBytes )
    {
        GLenum target = mUploadOnly ? GL_COPY_WRITE_BUFFER : GL_COPY_READ_BUFFER;
        GLbitfield flags;

        if( mUploadOnly )
        {
            target = GL_COPY_WRITE_BUFFER;
            flags = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT |
                    GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_FLUSH_EXPLICIT_BIT;
        }
        else
        {
            target = GL_COPY_READ_BUFFER;
            flags = GL_MAP_READ_BIT;
        }

        mMappingCount = sizeBytes;

        waitIfNeeded();

        glBindBuffer( target, mVboName );
        OCGLE( mMappedPtr = glMapBufferRange( target, mInternalBufferStart + mMappingStart,
                                              mMappingCount, flags ) );

        return mMappedPtr;
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusStagingBuffer::unmapImpl( const Destination *destinations, size_t numDestinations )
    {
        GLenum target = mUploadOnly ? GL_COPY_WRITE_BUFFER : GL_COPY_READ_BUFFER;
        GLenum oppositeTarget = mUploadOnly ? GL_COPY_READ_BUFFER : GL_COPY_WRITE_BUFFER;

        OCGLE( glBindBuffer( target, mVboName ) );
        OCGLE( glFlushMappedBufferRange( target,
                                         mInternalBufferStart + mMappingStart,
                                         mMappingCount ) );
        OCGLE( glUnmapBuffer( target ) );
        mMappedPtr = 0;

        for( size_t i=0; i<numDestinations; ++i )
        {
            const Destination &dst = destinations[i];

            GL3PlusBufferInterface *bufferInterface = static_cast<GL3PlusBufferInterface*>(
                                                        dst.destination->getBufferInterface() );

            assert( dst.destination->getBufferType() == BT_DEFAULT );

            GLintptr dstOffset = dst.dstOffset + dst.destination->_getInternalBufferStart() *
                                                        dst.destination->getBytesPerElement();

            OCGLE( glBindBuffer( oppositeTarget, bufferInterface->getVboName() ) );
            OCGLE( glCopyBufferSubData( target, oppositeTarget,
                                        mInternalBufferStart + mMappingStart + dst.srcOffset,
                                        dstOffset, dst.length ) );
        }

        //Add fence to this region (or at least, track the hazard).
        addFence( mMappingStart, mMappingStart + mMappingCount - 1, false );
    }
    //-----------------------------------------------------------------------------------
    StagingStallType GL3PlusStagingBuffer::willStall( size_t sizeBytes ) const
    {
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
}
