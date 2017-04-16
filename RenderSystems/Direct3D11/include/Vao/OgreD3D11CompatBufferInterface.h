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

#ifndef _Ogre_D3D11CompatBufferInterface_H_
#define _Ogre_D3D11CompatBufferInterface_H_

#include "OgreD3D11Prerequisites.h"

#include "Vao/OgreBufferInterface.h"

namespace Ogre
{
    /** In D3D11, const buffers can't be bound by offset. Their size at
        creation defines the size to be bound to the shader, and the max
        limit is 64kb.
        This is unless we're using D3D11.1 on Windows 8.1; which for simplicity
        we treat it the same as D3D11.
    @par
        This limitation prevents us from allocating 3x size for triple buffering.
    @par
        Therefore we need the traditional scheme of mapping with NO_OVERWRITE
        and then use DISCARD when we've filled the buffer.
    @par
        In D3D11.0, only index and vertex buffers can be mapped with NO_OVERWRITE.
        That leaves only DISCARD to be used on those systems.
        NO_OVERWRITE is used if used D3D 11.1 though.
    @par
        This buffer interface is for compatibility with these systems.
    */
    class _OgreD3D11Export D3D11CompatBufferInterface : public BufferInterface
    {
    protected:
        size_t          mVboPoolIdx;
        ID3D11Buffer    *mVboName;
        void            *mMappedPtr;

        D3D11Device     &mDevice;

    public:
        D3D11CompatBufferInterface( size_t vboPoolIdx, ID3D11Buffer *d3dBuffer, D3D11Device &device );
        ~D3D11CompatBufferInterface();

        size_t getVboPoolIndex(void)                { return mVboPoolIdx; }
        ID3D11Buffer* getVboName(void) const        { return mVboName; }

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
