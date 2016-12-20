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

#ifndef _Ogre_D3D11StagingBuffer_H_
#define _Ogre_D3D11StagingBuffer_H_

#include "OgreD3D11Prerequisites.h"

#include "Vao/OgreStagingBuffer.h"

namespace Ogre
{
    /** NOTE FOR D3D12 PORTING: The GL3Plus implementation should be used as reference, since it
        is much lower level than this implementation for D3D11.

        A staging buffer is a buffer that resides on the GPU and be written to/from both CPU & GPU
        However the access in both cases is limited. GPUs can only copy (i.e. memcpy) to another
        real buffer (can't be used directly as i.e. texture or vertex buffer) and CPUs can only
        map it.
        In other words, a staging buffer is an intermediate buffer to transfer data between
        CPU & GPU
    */
    class _OgreD3D11Export D3D11StagingBuffer : public StagingBuffer
    {
    protected:
        /// mVboName is not deleted by us (the VaoManager does) as we may have
        /// only been assigned a chunk of the buffer, not the whole thing.
        ID3D11Buffer    *mVboName;
        void            *mMappedPtr;

        D3D11Device     &mDevice;

        //------------------------------------
        // Begin used for uploads
        //------------------------------------
        /*typedef vector<GLFence>::type GLFenceVec;
        GLFenceVec mFences;

        /// Regions of memory that were unmapped but haven't
        /// been fenced due to not passing the threshold yet.
        GLFenceVec mUnfencedHazards;*/
        //------------------------------------
        // End used for uploads
        //------------------------------------

        /// Checks all the fences and compares them against current state (i.e. mMappingStart &
        /// mMappingCount), and stalls if needed (synchronize); also book-keeps mFences and
        /// mUnfencedHazards.
        /// May modify mMappingStart.
        void waitIfNeeded(void);

        virtual void* mapImpl( size_t sizeBytes );
        virtual void unmapImpl( const Destination *destinations, size_t numDestinations );

        virtual const void* _mapForReadImpl( size_t offset, size_t sizeBytes );

    public:
        D3D11StagingBuffer( size_t sizeBytes, VaoManager *vaoManager, bool uploadOnly,
                            ID3D11Buffer *stagingBuffer, D3D11Device &device );
        virtual ~D3D11StagingBuffer();

        virtual StagingStallType uploadWillStall( size_t sizeBytes );

        virtual size_t _asyncDownload( BufferPacked *source, size_t srcOffset, size_t srcLength );

        ID3D11Buffer* getBufferName(void) const     { return mVboName; }
    };
}

#endif
