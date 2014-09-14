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
#ifndef _OgreConstBufferPool_H_
#define _OgreConstBufferPool_H_

#include "OgrePrerequisites.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Resources
    *  @{
    */

    class ConstBufferPoolUser;

    /** Maintains a pool of buffers grouped by hash ID. Keeping them separated
        by hash prevents mixing materials in the same buffer (which has limited
        space) that could never be batched together in the same draw call.

        When a buffer is full and has used all of its free slots, a new buffer
        is allocated.
    */
    class _OgreExport ConstBufferPool
    {
    public:
        struct BufferPool
        {
            uint32                  hash;
            vector<uint32>::type    freeSlots;
            ConstBufferPacked       *materialBuffer;

            BufferPool( uint32 _hash, uint32 slotsPerPool, ConstBufferPacked *_materialBuffer );
        };

    protected:
        typedef vector<BufferPool*>::type           BufferPoolVec;
        typedef map<uint32, BufferPoolVec>::type    BufferPoolVecMap;

        typedef vector<ConstBufferPoolUser*>::type  ConstBufferPoolUserVec;

        BufferPoolVecMap    mPools;
        uint32              mSlotsPerPool;
        size_t              mBufferSize;
        VaoManager          *mVaoManager;

        ConstBufferPoolUserVec mDirtyUsers;

    public:
        ConstBufferPool( uint32 slotsPerPool, size_t bufferSize, VaoManager *vaoManager );
        virtual ~ConstBufferPool();

        /// Requests a slot and fills 'user'. Automatically schedules for update
        void requestSlot( uint32 hash, ConstBufferPoolUser *user );
        /// Releases a slot requested with requestSlot.
        void releaseSlot( ConstBufferPoolUser *user );

        void scheduleForUpdate( ConstBufferPoolUser *dirtyUser );

        virtual void _changeRenderSystem( RenderSystem *newRs );
    };

    class _OgreExport ConstBufferPoolUser
    {
        friend class ConstBufferPool;

        friend bool OrderConstBufferPoolUserByPoolThenSlot( const ConstBufferPoolUser *_l,
                                                            const ConstBufferPoolUser *_r );

        uint32                      mAssignedSlot;
        ConstBufferPool::BufferPool *mAssignedPool;
        ConstBufferPool             *mPoolOwner;
        bool                        mDirty;

    public:
        ConstBufferPoolUser();

        uint32 getAssignedSlot(void) const                              { return mAssignedSlot; }
        const ConstBufferPool::BufferPool* getAssignedPool(void) const  { return mAssignedPool; }
    };

    inline bool OrderConstBufferPoolUserByPoolThenSlot( const ConstBufferPoolUser *_l,
                                                        const ConstBufferPoolUser *_r )
    {
        return _l->mAssignedPool < _r->mAssignedPool && _l->mAssignedSlot < _r->mAssignedSlot;
    }

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
