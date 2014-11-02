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
#include "OgreRenderSystem.h"

namespace Ogre
{
    ConstBufferPool::ConstBufferPool( uint32 bytesPerSlot ) :
        mBytesPerSlot( bytesPerSlot ),
        mSlotsPerPool( 0 ),
        mBufferSize( 0 ),
        mVaoManager( 0 )
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
        BufferPoolVecMap::const_iterator itor = mPools.begin();
        BufferPoolVecMap::const_iterator end  = mPools.end();

        while( itor != end )
        {
            BufferPoolVec::const_iterator it = itor->second.begin();
            BufferPoolVec::const_iterator en = itor->second.end();

            while( it != en )
            {
                if( (*it)->materialBuffer )
                {
                    mVaoManager->destroyConstBuffer( (*it)->materialBuffer );
                    delete *it;
                }
                ++it;
            }

            ++itor;
        }

        mPools.clear();
    }
    //-----------------------------------------------------------------------------------
    void ConstBufferPool::requestSlot( uint32 hash, ConstBufferPoolUser *user )
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
            ConstBufferPacked *materialBuffer = mVaoManager->createConstBuffer( mBufferSize, BT_DEFAULT,
                                                                                0, false );

            BufferPool *newPool = new BufferPool( hash, mSlotsPerPool, materialBuffer );
            bufferPool.push_back( newPool );
            itor = bufferPool.end() - 1;
        }

        BufferPool *pool = *itor;
        user->mAssignedSlot = pool->freeSlots.back();
        user->mAssignedPool = pool;
        user->mPoolOwner    = this;

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
        user->mPoolOwner    = 0;
        user->mDirty        = true;
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
    void ConstBufferPool::_changeRenderSystem( RenderSystem *newRs )
    {
        if( mVaoManager )
        {
            destroyAllPools();
            mDirtyUsers.clear();
            mVaoManager = 0;
        }

        if( newRs )
        {
            mVaoManager = newRs->getVaoManager();

            mBufferSize = std::min<size_t>( mVaoManager->getConstBufferMaxSize(), 64 * 1024 );
            mSlotsPerPool = mBufferSize / mBytesPerSlot;
        }
    }
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    ConstBufferPool::BufferPool::BufferPool( uint32 _hash, uint32 slotsPerPool,
                                             ConstBufferPacked *_materialBuffer ) :
        hash( _hash ),
        materialBuffer( _materialBuffer )
    {
        freeSlots.reserve( slotsPerPool );
        for( uint32 i=0; i<slotsPerPool; ++i )
            freeSlots.push_back( (slotsPerPool - i) - 1 );
    }
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    ConstBufferPoolUser::ConstBufferPoolUser() :
        mAssignedSlot( 0 ),
        mAssignedPool( 0 ),
        mPoolOwner( 0 ),
        mDirty( false )
    {
    }
}
