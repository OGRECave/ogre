/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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

#include "OgreConstBufferPool.h"

#include "Vao/OgreVaoManager.h"
#include "Vao/OgreStagingBuffer.h"
#include "Vao/OgreConstBufferPacked.h"
#include "Vao/OgreTexBufferPacked.h"
#include "OgreRenderSystem.h"
#include "OgreProfiler.h"

namespace Ogre
{
    ConstBufferPool::ConstBufferPool( uint32 bytesPerSlot, const ExtraBufferParams &extraBufferParams ) :
        mBytesPerSlot( bytesPerSlot ),
        mSlotsPerPool( 0 ),
        mBufferSize( 0 ),
        mExtraBufferParams( extraBufferParams ),
        _mVaoManager( 0 ),
    #if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS && OGRE_PLATFORM != OGRE_PLATFORM_ANDROID
        mOptimizationStrategy( LowerCpuOverhead )
    #else
        mOptimizationStrategy( LowerGpuOverhead )
    #endif
    {
    }
    //-----------------------------------------------------------------------------------
    ConstBufferPool::~ConstBufferPool()
    {
        destroyAllPools();
    }
    //-----------------------------------------------------------------------------------
    void ConstBufferPool::destroyAllPools(void)
    {
        {
            BufferPoolVecMap::const_iterator itor = mPools.begin();
            BufferPoolVecMap::const_iterator end  = mPools.end();

            while( itor != end )
            {
                BufferPoolVec::const_iterator it = itor->second.begin();
                BufferPoolVec::const_iterator en = itor->second.end();

                while( it != en )
                {
                    _mVaoManager->destroyConstBuffer( (*it)->materialBuffer );

                    if( (*it)->extraBuffer )
                    {
                        if( mExtraBufferParams.useTextureBuffers )
                        {
                            assert( dynamic_cast<TexBufferPacked*>( (*it)->extraBuffer ) );
                            _mVaoManager->destroyTexBuffer( static_cast<TexBufferPacked*>(
                                                                (*it)->extraBuffer ) );
                        }
                        else
                        {
                            assert( dynamic_cast<ConstBufferPacked*>( (*it)->extraBuffer ) );
                            _mVaoManager->destroyConstBuffer( static_cast<ConstBufferPacked*>(
                                                                  (*it)->extraBuffer ) );
                        }
                    }

                    delete *it;

                    ++it;
                }

                ++itor;
            }

            mPools.clear();
        }

        {
            ConstBufferPoolUserVec::const_iterator itor = mUsers.begin();
            ConstBufferPoolUserVec::const_iterator end  = mUsers.end();

            while( itor != end )
            {
                (*itor)->mAssignedSlot  = 0;
                (*itor)->mAssignedPool  = 0;
                (*itor)->mGlobalIndex   = -1;
                (*itor)->mDirty         = false;
                ++itor;
            }

            mUsers.clear();
        }
    }
    //-----------------------------------------------------------------------------------
    void ConstBufferPool::uploadDirtyDatablocks(void)
    {
        if( mDirtyUsers.empty() )
            return;

        OgreProfileExhaustive( "ConstBufferPool::uploadDirtyDatablocks" );

        const size_t materialSizeInGpu = mBytesPerSlot;
        const size_t extraBufferSizeInGpu = mExtraBufferParams.bytesPerSlot;

        std::sort( mDirtyUsers.begin(), mDirtyUsers.end(), OrderConstBufferPoolUserByPoolThenSlot );

        const size_t uploadSize = (materialSizeInGpu + extraBufferSizeInGpu) * mDirtyUsers.size();
        StagingBuffer *stagingBuffer = _mVaoManager->getStagingBuffer( uploadSize, true );

        StagingBuffer::DestinationVec destinations;
        StagingBuffer::DestinationVec extraDestinations;

        destinations.reserve( mDirtyUsers.size() );
        extraDestinations.reserve( mDirtyUsers.size() );

        ConstBufferPoolUserVec::const_iterator itor = mDirtyUsers.begin();
        ConstBufferPoolUserVec::const_iterator end  = mDirtyUsers.end();

        char *bufferStart = reinterpret_cast<char*>( stagingBuffer->map( uploadSize ) );
        char *data      = bufferStart;
        char *extraData = bufferStart + materialSizeInGpu * mDirtyUsers.size();

        while( itor != end )
        {
            const size_t srcOffset = static_cast<size_t>( data - bufferStart );
            const size_t dstOffset = (*itor)->getAssignedSlot() * materialSizeInGpu;

            (*itor)->uploadToConstBuffer( data );
            data += materialSizeInGpu;

            (*itor)->mDirty = false;

            const BufferPool *usersPool = (*itor)->getAssignedPool();

            StagingBuffer::Destination dst( usersPool->materialBuffer, dstOffset,
                                            srcOffset, materialSizeInGpu );

            if( !destinations.empty() )
            {
                StagingBuffer::Destination &lastElement = destinations.back();

                if( lastElement.destination == dst.destination &&
                    (lastElement.dstOffset + lastElement.length == dst.dstOffset) )
                {
                    lastElement.length += dst.length;
                }
                else
                {
                    destinations.push_back( dst );
                }
            }
            else
            {
                destinations.push_back( dst );
            }

            if( usersPool->extraBuffer )
            {
                (*itor)->uploadToExtraBuffer( extraData );

                const size_t extraSrcOffset = static_cast<size_t>( extraData - bufferStart );
                const size_t extraDstOffset = (*itor)->getAssignedSlot() * extraBufferSizeInGpu;

                extraData += extraBufferSizeInGpu;

                StagingBuffer::Destination extraDst( usersPool->extraBuffer, extraDstOffset,
                                                     extraSrcOffset, extraBufferSizeInGpu );

                if( !extraDestinations.empty() )
                {
                    StagingBuffer::Destination &lastElement = extraDestinations.back();

                    if( lastElement.destination == dst.destination &&
                        (lastElement.dstOffset + lastElement.length == dst.dstOffset) )
                    {
                        lastElement.length += dst.length;
                    }
                    else
                    {
                        extraDestinations.push_back( extraDst );
                    }
                }
                else
                {
                    extraDestinations.push_back( extraDst );
                }
            }

            ++itor;
        }

        destinations.insert( destinations.end(), extraDestinations.begin(), extraDestinations.end() );

        stagingBuffer->unmap( destinations );
        stagingBuffer->removeReferenceCount();

        mDirtyUsers.clear();
    }
    //-----------------------------------------------------------------------------------
    void ConstBufferPool::requestSlot( uint32 hash, ConstBufferPoolUser *user, bool wantsExtraBuffer )
    {
        if( user->mAssignedPool )
            releaseSlot( user );

        BufferPoolVecMap::iterator it = mPools.find( hash );

        if( it == mPools.end() )
        {
            mPools[hash] = BufferPoolVec();
            it = mPools.find( hash );
        }

        BufferPoolVec &bufferPool = it->second;

        BufferPoolVec::iterator itor = bufferPool.begin();
        BufferPoolVec::iterator end  = bufferPool.end();

        while( itor != end && (*itor)->freeSlots.empty() )
            ++itor;

        if( itor == end )
        {
            ConstBufferPacked *materialBuffer = _mVaoManager->createConstBuffer( mBufferSize, BT_DEFAULT,
                                                                                 0, false );
            BufferPacked *extraBuffer = 0;

            if( mExtraBufferParams.bytesPerSlot && wantsExtraBuffer )
            {
                if( mExtraBufferParams.useTextureBuffers )
                {
                    extraBuffer = _mVaoManager->createTexBuffer( PF_FLOAT32_RGBA,
                                                                 mExtraBufferParams.bytesPerSlot *
                                                                                    mSlotsPerPool,
                                                                 mExtraBufferParams.bufferType,
                                                                 0, false );
                }
                else
                {
                    extraBuffer = _mVaoManager->createConstBuffer( mExtraBufferParams.bytesPerSlot *
                                                                                    mSlotsPerPool,
                                                                   mExtraBufferParams.bufferType,
                                                                   0, false );
                }
            }

            BufferPool *newPool = new BufferPool( hash, mSlotsPerPool, materialBuffer, extraBuffer );
            bufferPool.push_back( newPool );
            itor = bufferPool.end() - 1;
        }

        BufferPool *pool = *itor;
        user->mAssignedSlot = pool->freeSlots.back();
        user->mAssignedPool = pool;
        user->mGlobalIndex  = mUsers.size();
        //user->mPoolOwner    = this;

        //If this assert triggers, you need to be consistent between your hashes
        //and the wantsExtraBuffer flag so that you land in the right pool.
        assert( (pool->extraBuffer != 0) == wantsExtraBuffer &&
                "The pool was first requested with/without an extra buffer "
                "but now the opposite is being requested" );

        mUsers.push_back( user );

        pool->freeSlots.pop_back();

        scheduleForUpdate( user );
    }
    //-----------------------------------------------------------------------------------
    void ConstBufferPool::releaseSlot( ConstBufferPoolUser *user )
    {
        BufferPool *pool = user->mAssignedPool;

        if( user->mDirty )
        {
            ConstBufferPoolUserVec::iterator it = std::find( mDirtyUsers.begin(),
                                                             mDirtyUsers.end(), user );

            if( it != mDirtyUsers.end() )
                efficientVectorRemove( mDirtyUsers, it );
        }

        assert( user->mAssignedSlot < mSlotsPerPool );
        assert( std::find( pool->freeSlots.begin(),
                           pool->freeSlots.end(),
                           user->mAssignedSlot ) == pool->freeSlots.end() );

        pool->freeSlots.push_back( user->mAssignedSlot );
        user->mAssignedSlot = 0;
        user->mAssignedPool = 0;
        //user->mPoolOwner    = 0;
        user->mDirty        = false;

        assert( user->mGlobalIndex < mUsers.size() && user == *(mUsers.begin() + user->mGlobalIndex) &&
                "mGlobalIndex out of date or argument doesn't belong to this pool manager" );
        ConstBufferPoolUserVec::iterator itor = mUsers.begin() + user->mGlobalIndex;
        itor = efficientVectorRemove( mUsers, itor );

        //The node that was at the end got swapped and has now a different index
        if( itor != mUsers.end() )
            (*itor)->mGlobalIndex = itor - mUsers.begin();
    }
    //-----------------------------------------------------------------------------------
    void ConstBufferPool::scheduleForUpdate( ConstBufferPoolUser *dirtyUser )
    {
        if( !dirtyUser->mDirty )
        {
            mDirtyUsers.push_back( dirtyUser );
            dirtyUser->mDirty = true;
        }
    }
    //-----------------------------------------------------------------------------------
    size_t ConstBufferPool::getPoolIndex( ConstBufferPoolUser *user ) const
    {
        BufferPool *pool = user->mAssignedPool;

        assert( user->mAssignedSlot < mSlotsPerPool );
        assert( std::find( pool->freeSlots.begin(),
                           pool->freeSlots.end(),
                           user->mAssignedSlot ) == pool->freeSlots.end() );

        BufferPoolVecMap::const_iterator itor = mPools.find( pool->hash );
        assert( itor != mPools.end() && "Error or argument doesn't belong to this pool manager." );

        const BufferPoolVec &poolVec = itor->second;
        BufferPoolVec::const_iterator it = std::find( poolVec.begin(), poolVec.end(), pool );

        return it - poolVec.begin();
    }
    //-----------------------------------------------------------------------------------
    struct OldUserRecord
    {
        ConstBufferPoolUser *user;
        uint32              hash;
        bool                wantsExtraBuffer;
        OldUserRecord( ConstBufferPoolUser *_user, uint32 _hash, bool _wantsExtraBuffer ) :
            user( _user ), hash( _hash ), wantsExtraBuffer( _wantsExtraBuffer ) {}
    };
    typedef std::vector<OldUserRecord> OldUserRecordVec;
    void ConstBufferPool::setOptimizationStrategy( OptimizationStrategy optimizationStrategy )
    {
        if( mOptimizationStrategy != optimizationStrategy )
        {
            mDirtyUsers.clear();

            OldUserRecordVec oldUserRecords;
            {
                //Save all the data we need before we destroy the pools.
                ConstBufferPoolUserVec::const_iterator itor = mUsers.begin();
                ConstBufferPoolUserVec::const_iterator end  = mUsers.end();

                while( itor != end )
                {
                    OldUserRecord record( *itor, (*itor)->mAssignedPool->hash,
                                          (*itor)->mAssignedPool->extraBuffer != 0 );
                    oldUserRecords.push_back( record );
                    ++itor;
                }
            }

            destroyAllPools();
            mOptimizationStrategy = optimizationStrategy;
            if( _mVaoManager )
            {
                if( mOptimizationStrategy == LowerCpuOverhead )
                    mBufferSize = std::min<size_t>( _mVaoManager->getConstBufferMaxSize(), 64 * 1024 );
                else
                    mBufferSize = mBytesPerSlot;

                mSlotsPerPool = mBufferSize / mBytesPerSlot;
            }

            {
                //Recreate the pools.
                OldUserRecordVec::const_iterator itor = oldUserRecords.begin();
                OldUserRecordVec::const_iterator end  = oldUserRecords.end();

                while( itor != end )
                {
                    this->requestSlot( itor->hash, itor->user, itor->wantsExtraBuffer );
                    itor->user->notifyOptimizationStrategyChanged();
                    ++itor;
                }
            }
        }
    }
    //-----------------------------------------------------------------------------------
    ConstBufferPool::OptimizationStrategy ConstBufferPool::getOptimizationStrategy() const
    {
        return mOptimizationStrategy;
    }
    //-----------------------------------------------------------------------------------
    void ConstBufferPool::_changeRenderSystem( RenderSystem *newRs )
    {
        if( _mVaoManager )
        {
            destroyAllPools();
            mDirtyUsers.clear();
            _mVaoManager = 0;
        }

        if( newRs )
        {
            _mVaoManager = newRs->getVaoManager();

            if( mOptimizationStrategy == LowerCpuOverhead )
                mBufferSize = std::min<size_t>( _mVaoManager->getConstBufferMaxSize(), 64 * 1024 );
            else
                mBufferSize = mBytesPerSlot;

            mSlotsPerPool = mBufferSize / mBytesPerSlot;
        }
    }
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    ConstBufferPool::BufferPool::BufferPool( uint32 _hash, uint32 slotsPerPool,
                                             ConstBufferPacked *_materialBuffer,
                                             BufferPacked *_extraBuffer ) :
        hash( _hash ),
        materialBuffer( _materialBuffer ),
        extraBuffer( _extraBuffer )
    {
        freeSlots.reserve( slotsPerPool );
        for( uint32 i=0; i<slotsPerPool; ++i )
            freeSlots.push_back( (slotsPerPool - i) - 1 );
    }
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    ConstBufferPool::ExtraBufferParams::ExtraBufferParams( size_t _bytesPerSlot, BufferType _bufferType,
                                                           bool _useTextureBuffers ) :
        bytesPerSlot( _bytesPerSlot ),
        bufferType( _bufferType ),
        useTextureBuffers( _useTextureBuffers )
    {
    }
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    ConstBufferPoolUser::ConstBufferPoolUser() :
        mAssignedSlot( 0 ),
        mAssignedPool( 0 ),
        //mPoolOwner( 0 ),
        mGlobalIndex( -1 ),
        mDirty( false )
    {
    }
}
