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

#ifndef _Ogre_GL3PlusBufferInterface_H_
#define _Ogre_GL3PlusBufferInterface_H_

#include "OgreGL3PlusPrerequisites.h"

#include "Vao/OgreBufferInterface.h"

namespace Ogre
{
    /** For GL3+, most (if not all) buffers, can be treated with the same code.
        Hence most equivalent functionality is encapsulated here.
    */
    class _OgreGL3PlusExport GL3PlusBufferInterface : public BufferInterface
    {
    protected:
        size_t  mVboPoolIdx;
        GLuint  mVboName;
        void    *mMappedPtr;

        size_t  mUnmapTicket;
        GL3PlusDynamicBuffer *mDynamicBuffer;

        /** @see BufferPacked::map.
        @param bAdvanceFrame
            When false, it doesn't really advance the mBuffer->mFinalBufferStart pointer,
            it just calculates the next index without advancing and returns its value.
            i.e. the value after advancing (was 0, would be incremented to 1, function returns 1).
        @return
            The 'next frame' index in the range [0; vaoManager->getDynamicBufferMultiplier())
            i.e. the value after advancing (was 0, gets incremented to 1, function returns 1).
        */
        size_t advanceFrame( bool bAdvanceFrame );

    public:
        GL3PlusBufferInterface( size_t vboPoolIdx, GLuint vboName,
                                GL3PlusDynamicBuffer *dynamicBuffer );
        ~GL3PlusBufferInterface();

        size_t getVboPoolIndex(void)                { return mVboPoolIdx; }
        GLuint getVboName(void) const               { return mVboName; }

        /// Only use this function for the first upload
        void _firstUpload( void *data, size_t elementStart, size_t elementCount );

        virtual void* RESTRICT_ALIAS_RETURN map( size_t elementStart, size_t elementCount,
                                                 MappingState prevMappingState,
                                                 bool advanceFrame = true );
        virtual void unmap( UnmapOptions unmapOption,
                            size_t flushStartElem = 0, size_t flushSizeElem = 0 );
        virtual void advanceFrame(void);
        virtual void regressFrame(void);
    };
}

#endif
