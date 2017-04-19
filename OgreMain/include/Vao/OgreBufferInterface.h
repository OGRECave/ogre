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

#ifndef _Ogre_BufferInterface_H_
#define _Ogre_BufferInterface_H_

#include "Vao/OgreBufferPacked.h"

namespace Ogre
{
    /** Most (if not all) buffers, can be treated with the same code.
        Hence most equivalent functionality is encapsulated here.
    */
    class _OgreExport BufferInterface
    {
    protected:
        BufferPacked *mBuffer;

    public:
        BufferInterface();
        virtual ~BufferInterface() {}

        void upload( const void *data, size_t elementStart, size_t elementCount );

        virtual void* RESTRICT_ALIAS_RETURN map( size_t elementStart, size_t elementCount,
                                                 MappingState prevMappingState,
                                                 bool advanceFrame = true ) = 0;
        virtual void unmap( UnmapOptions unmapOption,
                            size_t flushStartElem = 0, size_t flushSizeElem = 0 ) = 0;
        virtual void advanceFrame(void) = 0;
        virtual void regressFrame(void) = 0;

        virtual void _notifyBuffer( BufferPacked *buffer )  { mBuffer = buffer; }
    };
}

#endif
