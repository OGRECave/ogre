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

#ifndef _OgreMemoryTrackAlloc_H_
#define _OgreMemoryTrackAlloc_H_

#if OGRE_MEMORY_ALLOCATOR == OGRE_MEMORY_ALLOCATOR_TRACK

#include <memory>

#include "OgreAlignedAllocator.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    extern const size_t OGRE_TRACK_POOL_SIZE;

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Memory
    *  @{
    */
    /** An allocation policy that preallocates a fixed amount of memory and always
        returns a deterministic pool. Very useful for catching memory corruption
        errors (but not memory leaks). It uses huge amounts of RAM, so you better
        use 64-bit builds.
    */
    class _OgreExport TrackAllocPolicy
    {
        static char     *MemoryPool;
        static size_t   CurrentOffset;
        static char     Magic[4];
        static size_t   RandomOffset;
    public:
        static DECL_MALLOC void* allocateBytes(size_t count, 
#if OGRE_MEMORY_TRACKER
            const char* file = 0, int line = 0, const char* func = 0
#else
            const char*  = 0, int  = 0, const char* = 0
#endif
            );

        static void deallocateBytes(void* _ptr);

        /// Get the maximum size of a single allocation
        static inline size_t getMaxAllocationSize()
        {
            return OGRE_TRACK_POOL_SIZE;
        }
    private:
        // no instantiation
        TrackAllocPolicy()
        { }
    };

    /** @See TrackAllocPolicy
    */
    template <size_t Alignment = 0>
    class TrackAlignedAllocPolicy
    {
    public:
        // compile-time check alignment is available.
        typedef int IsValidAlignment
            [Alignment <= 128 && ((Alignment & (Alignment-1)) == 0) ? +1 : -1];

        static inline DECL_MALLOC void* allocateBytes(size_t count, 
#if OGRE_MEMORY_TRACKER
            const char* file = 0, int line = 0, const char* func = 0
#else
            const char*  = 0, int  = 0, const char* = 0
#endif
            )
        {
            if( !count )
                return 0;

            size_t _alignment = Alignment ? Alignment : 16; //Should be OGRE_SIMD_ALIGNMENT, but compiler error
            uint8 *tmp = (uint8*)TrackAllocPolicy::allocateBytes( count + _alignment
#if OGRE_MEMORY_TRACKER
            , file, line, func
#endif
            );

            //Align...
            uint8 *mem_block = (uint8*)( (uint32) (tmp + _alignment - 1) & (uint32)(~(_alignment - 1)) );

            //Special case where malloc have already satisfied the alignment
            //We must add alignment to mem_block to mantain alignment AND
            //to avoid that afree causes an ACCESS VIOLATION becuase
            //(*(mem_block-1)) is beyond our visibility
            if (mem_block == tmp)
                mem_block += _alignment;

            //How far are from the real start of our memory
            //block?
            *(mem_block-1) = (uint8) (mem_block-tmp);

            return (void*)mem_block;
        }

        static inline void deallocateBytes(void* ptr)
        {
            uint8 *realAddress;

            if( !ptr )
                return;

            realAddress  = (uint8*)ptr;
            realAddress -= *(realAddress-1);

            TrackAllocPolicy::deallocateBytes( realAddress );
        }

        /// Get the maximum size of a single allocation
        static inline size_t getMaxAllocationSize()
        {
            return OGRE_TRACK_POOL_SIZE;
        }
    private:
        // No instantiation
        TrackAlignedAllocPolicy()
        { }
    };
    /** @} */
    /** @} */

}// namespace Ogre


#include "OgreHeaderSuffix.h"

#endif

#endif // _OgreMemoryTrackAlloc_H_
