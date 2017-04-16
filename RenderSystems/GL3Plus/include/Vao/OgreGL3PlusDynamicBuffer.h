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

#ifndef _Ogre_GL3PlusDynamicBuffer_H_
#define _Ogre_GL3PlusDynamicBuffer_H_

#include "OgreGL3PlusPrerequisites.h"

#include "Vao/OgreBufferPacked.h"

namespace Ogre
{
    /** GL3+ doesn't support mapping the same buffer twice even if the regions
        don't overlap. For performance we keep many buffers as one big buffer,
        but for compatibility reasons (with GL3/DX10 HW) we treat them as
        separate buffers.
    @par
        This class takes care of mapping buffers once while allowing
        BufferInterface to map subregions of it as if they were separate
        buffers.
    @remarks
        This is a very thin/lightweight wrapper around OpenGL glMapBufferRange, thus:
            Caller is responsible for flushing regions before unmapping.
            Caller is responsible for proper synchronization.
            No check is performed to see if two map calls overlap.
    */
    class _OgreGL3PlusExport GL3PlusDynamicBuffer
    {
    protected:
        struct MappedRange
        {
            size_t start;
            size_t count;

            MappedRange( size_t _start, size_t _count ) : start( _start ), count( _count ) {}
        };

        typedef vector<MappedRange>::type MappedRangeVec;

        GLuint  mVboName;
        GLuint  mVboSize;
        void    *mMappedPtr;

        MappedRangeVec mMappedRanges;
        vector<size_t>::type mFreeRanges;

        BufferType mPersistentMethod;

        size_t addMappedRange( size_t start, size_t count );

    public:
        GL3PlusDynamicBuffer( GLuint vboName, GLuint vboSize, GL3PlusVaoManager *vaoManager,
                              BufferType persistentMethod );
        ~GL3PlusDynamicBuffer();

        GLuint getVboName(void) const               { return mVboName; }

        /// Assumes mVboName is already bound to GL_COPY_WRITE_BUFFER!!!
        void* RESTRICT_ALIAS_RETURN map( size_t start, size_t count, size_t &outTicket );

        /// Flushes the region of the given ticket. start is 0-based.
        void flush( size_t ticket, size_t start, size_t count );

        /// Unmaps given ticket (got from @see map).
        /// Assumes mVboName is already bound to GL_COPY_WRITE_BUFFER!!!
        /// The ticket becomes invalid after this.
        void unmap( size_t ticket );
    };
}

#endif
