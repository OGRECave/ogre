/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2011 Torus Knot Software Ltd

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
#include "OgreD3D11HardwareUniformBuffer.h"
#include "OgreD3D11HardwareBuffer.h"
#include "OgreD3D11Device.h"

namespace Ogre {

    //---------------------------------------------------------------------
    D3D11HardwareUniformBuffer::D3D11HardwareUniformBuffer(HardwareBufferManagerBase* mgr, size_t sizeBytes, HardwareBuffer::Usage usage, 
                                                bool useShadowBuffer, const String& name, D3D11Device & device)
        : HardwareUniformBuffer(mgr, sizeBytes, usage, false /* see below */, name)
        , mBufferImpl(0)
          
    {
        // ensure DefaultHardwareUniformBuffer was not created
        assert(!mShadowBuffer);
        mUseShadowBuffer = useShadowBuffer;

        // everything is done via internal generalisation
        mBufferImpl = new D3D11HardwareBuffer(D3D11HardwareBuffer::CONSTANT_BUFFER, 
                                            mSizeInBytes, mUsage, device, false, useShadowBuffer, false);

    }
    //---------------------------------------------------------------------
    D3D11HardwareUniformBuffer::~D3D11HardwareUniformBuffer()
    {
        SAFE_DELETE(mBufferImpl);
    }
    //---------------------------------------------------------------------
    void* D3D11HardwareUniformBuffer::lock(size_t offset, size_t length, LockOptions options)
	{
		return mBufferImpl->lock(offset, length, options);
	}
	//---------------------------------------------------------------------
	void D3D11HardwareUniformBuffer::unlock(void)
	{
		mBufferImpl->unlock();
	}
	//---------------------------------------------------------------------
	void D3D11HardwareUniformBuffer::readData(size_t offset, size_t length, void* pDest)
	{
		mBufferImpl->readData(offset, length, pDest);
	}
	//---------------------------------------------------------------------
	void D3D11HardwareUniformBuffer::writeData(size_t offset, size_t length, const void* pSource,
		bool discardWholeBuffer)
	{
		mBufferImpl->writeData(offset, length, pSource, discardWholeBuffer);
	}
	//---------------------------------------------------------------------
	void D3D11HardwareUniformBuffer::copyData(HardwareBuffer& srcBuffer, size_t srcOffset, 
		size_t dstOffset, size_t length, bool discardWholeBuffer)
	{
		// check if the other buffer is also a D3D11HardwareUniformBuffer
		if (srcBuffer.isSystemMemory())
		{
			// src is not a D3D11HardwareUniformBuffer - use default copy
			HardwareBuffer::copyData(srcBuffer, srcOffset, dstOffset, length, discardWholeBuffer);
		}
		else
		{
			// src is a D3D11HardwareUniformBuffer use d3d11 optimized copy
			D3D11HardwareUniformBuffer& d3dBuf = static_cast<D3D11HardwareUniformBuffer&>(srcBuffer);
			mBufferImpl->copyData(*(d3dBuf.mBufferImpl), srcOffset, dstOffset, length, discardWholeBuffer);
		}
	}
	//---------------------------------------------------------------------
	bool D3D11HardwareUniformBuffer::isLocked(void) const
	{
		return mBufferImpl->isLocked();
	}
	//---------------------------------------------------------------------
	ID3D11Buffer * D3D11HardwareUniformBuffer::getD3DConstantBuffer( void ) const
	{
		return mBufferImpl->getD3DBuffer();
	}
}

