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

#ifndef _Ogre_MetalDynamicBuffer_H_
#define _Ogre_MetalDynamicBuffer_H_

#include "OgreMetalPrerequisites.h"

#include "Vao/OgreBufferPacked.h"

namespace Ogre
{
    /** Metal doesn't "map". You can directly access the unsynchronized contents
        For performance we keep many buffers as one big buffer, but for compatibility
        reasons (with GL3/DX10 HW) we treat them as separate buffers.
    @par
        This class treats mapping in a GL-style. The most usefulness is that in OS X
        we need to inform what regions were modified (flush). On iOS it serves
        no real purpose and is just a pass through with some asserts for consistency
        checking.
    @remarks
        This is a very thin/lightweight wrapper around MTLBuffer, thus:
            Caller is responsible for flushing regions before unmapping.
            Caller is responsible for proper synchronization.
            No check is performed to see if two map calls overlap.
    */
    class _OgreMetalExport MetalDynamicBuffer
    {
    protected:
        struct MappedRange
        {
            size_t start;
            size_t count;

            MappedRange( size_t _start, size_t _count ) : start( _start ), count( _count ) {}
        };

        typedef vector<MappedRange>::type MappedRangeVec;

        id<MTLBuffer>   mVboName;
        size_t          mVboSize;
        void            *mMappedPtr;

        MappedRangeVec mMappedRanges;
        vector<size_t>::type mFreeRanges;

        size_t addMappedRange( size_t start, size_t count );

    public:
        MetalDynamicBuffer( id<MTLBuffer> vboName, size_t vboSize );
        ~MetalDynamicBuffer();

        id<MTLBuffer> getVboName(void) const        { return mVboName; }

        void* RESTRICT_ALIAS_RETURN map( size_t start, size_t count, size_t &outTicket );

        /// Flushes the region of the given ticket. start is 0-based.
        void flush( size_t ticket, size_t start, size_t count );

        /// Unmaps given ticket (got from @see map).
        /// The ticket becomes invalid after this.
        void unmap( size_t ticket );
    };
}

#endif
