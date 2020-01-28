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
#include "OgreD3D11HardwareVertexBuffer.h"
#include "OgreD3D11HardwareBuffer.h"
#include "OgreD3D11Device.h"

namespace Ogre {

    //---------------------------------------------------------------------
    D3D11HardwareVertexBuffer::D3D11HardwareVertexBuffer(HardwareBufferManagerBase* mgr, size_t vertexSize, 
        size_t numVertices, HardwareBuffer::Usage usage, D3D11Device & device, 
        bool useSystemMemory, bool useShadowBuffer, bool streamOut)
        : HardwareVertexBuffer(mgr, vertexSize, numVertices, usage, useSystemMemory, false /* see below */),
          mBufferImpl(0)
          
    {
        // ensure DefaultHardwareVertexBuffer was not created
        assert(!mShadowBuffer);
        mUseShadowBuffer = useShadowBuffer;

        // everything is done via internal generalisation
        mBufferImpl = new D3D11HardwareBuffer(D3D11HardwareBuffer::VERTEX_BUFFER, 
            mSizeInBytes, mUsage, device, useSystemMemory, useShadowBuffer, streamOut);

    }
    //---------------------------------------------------------------------
    D3D11HardwareVertexBuffer::~D3D11HardwareVertexBuffer()
    {
        SAFE_DELETE(mBufferImpl);
    }
    //---------------------------------------------------------------------
    void* D3D11HardwareVertexBuffer::lock(size_t offset, size_t length, LockOptions options)
    {
        return mBufferImpl->lock(offset, length, options);
    }
    //---------------------------------------------------------------------
    void D3D11HardwareVertexBuffer::unlock(void)
    {
        mBufferImpl->unlock();
    }
    //---------------------------------------------------------------------
    void D3D11HardwareVertexBuffer::readData(size_t offset, size_t length, void* pDest)
    {
        mBufferImpl->readData(offset, length, pDest);
    }
    //---------------------------------------------------------------------
    void D3D11HardwareVertexBuffer::writeData(size_t offset, size_t length, const void* pSource,
        bool discardWholeBuffer)
    {
        mBufferImpl->writeData(offset, length, pSource, discardWholeBuffer);
    }
    //---------------------------------------------------------------------
    void D3D11HardwareVertexBuffer::copyData(HardwareBuffer& srcBuffer, size_t srcOffset, 
        size_t dstOffset, size_t length, bool discardWholeBuffer)
    {
        // check if the other buffer is also a D3D11HardwareVertexBuffer
        if (srcBuffer.isSystemMemory())
        {
            // src is not a D3D11HardwareVertexBuffer - use default copy
            HardwareBuffer::copyData(srcBuffer, srcOffset, dstOffset, length, discardWholeBuffer);
        }
        else
        {
            // src is a D3D11HardwareVertexBuffer use d3d11 optimized copy
            D3D11HardwareVertexBuffer& d3dBuf = static_cast<D3D11HardwareVertexBuffer&>(srcBuffer);

			mBufferImpl->copyData(*(d3dBuf.mBufferImpl), srcOffset, dstOffset, length, discardWholeBuffer);
		}
	}
	//---------------------------------------------------------------------
	bool D3D11HardwareVertexBuffer::isLocked(void) const
	{
		return mBufferImpl->isLocked();
	}
	//---------------------------------------------------------------------
    //---------------------------------------------------------------------
    ID3D11Buffer * D3D11HardwareVertexBuffer::getD3DVertexBuffer( void ) const
    {
        return mBufferImpl->getD3DBuffer();
    }
}

