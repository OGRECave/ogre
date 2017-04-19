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

#ifndef _Ogre_D3D11BufferInterface_H_
#define _Ogre_D3D11BufferInterface_H_

#include "OgreD3D11Prerequisites.h"

#include "Vao/OgreBufferInterface.h"

namespace Ogre
{
    /** For D3D11, most (if not all) buffers, can be treated with the same code.
        Hence most equivalent functionality is encapsulated here.
    */
    class _OgreD3D11Export D3D11BufferInterface : public BufferInterface
    {
    protected:
        size_t          mVboPoolIdx;
        ID3D11Buffer    *mVboName;
        void            *mMappedPtr;

        size_t              mUnmapTicket;
        D3D11DynamicBuffer  *mDynamicBuffer;

        /// Used to store initial data of BT_IMMUTABLE buffers and delay their
        /// creation as much as possible (so that we can batch them together)
        void *mInitialData;

        size_t advanceFrame( bool bAdvanceFrame );

    public:
        D3D11BufferInterface( size_t vboPoolIdx, ID3D11Buffer *d3dBuffer,
                              D3D11DynamicBuffer *dynamicBuffer );
        ~D3D11BufferInterface();

        size_t getVboPoolIndex(void)                { return mVboPoolIdx; }
        ID3D11Buffer* getVboName(void) const        { return mVboName; }
        D3D11DynamicBuffer* getDynamicBuffer(void) const    { return mDynamicBuffer; }

        /// Only use this function for the first upload
        void _firstUpload( void *data );

        const void* _getInitialData(void) const     { return mInitialData; }
        void _deleteInitialData(void);
        void _setVboName( size_t vboPoolIdx, ID3D11Buffer *vboName, size_t internalBufferStartBytes );

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
