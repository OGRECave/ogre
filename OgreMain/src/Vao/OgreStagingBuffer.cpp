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
        mLifetimeThreshold( vaoManager->mDefaultStagingBufferLifetime ),
        mLastUsedTimestamp( vaoManager->getTimer()->getMilliseconds() )
    {
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
    StagingStallType StagingBuffer::willStall( size_t sizeBytes ) const
    {
        return STALL_PARTIAL;
    }
    //-----------------------------------------------------------------------------------
    void* StagingBuffer::map( size_t sizeBytes )
    {
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

        assert( (!mUploadOnly && !destinations && !numDestinations ||
                mUploadOnly && destinations && numDestinations) &&
                "Using an upload staging-buffer for downloads or vice-versa." );

        unmapImpl( destinations, numDestinations );

        mMappingState = MS_UNMAPPED;
        mMappingStart += mMappingCount;
        mMappingStart = mMappingStart % mSizeBytes;
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
}
