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
#include "OgreStableHeaders.h"
#include "OgreDefaultHardwareBufferManager.h"

namespace Ogre {

    DefaultHardwareBuffer::DefaultHardwareBuffer(size_t sizeInBytes)
    : HardwareBuffer(HBU_CPU_ONLY, true, false) // always software, never shadowed
    {
        mSizeInBytes = sizeInBytes;
        // Allocate aligned memory for better SIMD processing friendly.
        mData = (uchar*)AlignedMemory::allocate(mSizeInBytes);
    }
    //-----------------------------------------------------------------------
    DefaultHardwareBuffer::~DefaultHardwareBuffer()
    {
        AlignedMemory::deallocate(mData);
    }
    //-----------------------------------------------------------------------
    void* DefaultHardwareBuffer::lockImpl(size_t offset, size_t length, LockOptions options)
    {
        // Only for use internally, no 'locking' as such
        return mData + offset;
    }
    //-----------------------------------------------------------------------
    void DefaultHardwareBuffer::unlockImpl(void)
    {
        // Nothing to do
    }
    //-----------------------------------------------------------------------
    void DefaultHardwareBuffer::readData(size_t offset, size_t length, void* pDest)
    {
        assert((offset + length) <= mSizeInBytes);
        memcpy(pDest, mData + offset, length);
    }
    //-----------------------------------------------------------------------
    void DefaultHardwareBuffer::writeData(size_t offset, size_t length, const void* pSource,
                                          bool discardWholeBuffer)
    {
        assert((offset + length) <= mSizeInBytes);
        // ignore discard, memory is not guaranteed to be zeroised
        memcpy(mData + offset, pSource, length);
    }
    //-----------------------------------------------------------------------
    DefaultHardwareBufferManagerBase::DefaultHardwareBufferManagerBase()
    {
    }
    //-----------------------------------------------------------------------
    DefaultHardwareBufferManagerBase::~DefaultHardwareBufferManagerBase()
    {
        destroyAllDeclarations();
        destroyAllBindings(); 
    }
    //-----------------------------------------------------------------------
    HardwareVertexBufferSharedPtr 
        DefaultHardwareBufferManagerBase::createVertexBuffer(size_t vertexSize, 
        size_t numVerts, HardwareBuffer::Usage usage, bool useShadowBuffer)
    {
        DefaultHardwareVertexBuffer* vb = OGRE_NEW DefaultHardwareVertexBuffer(this, vertexSize, numVerts, usage);
        return HardwareVertexBufferSharedPtr(vb);
    }
    //-----------------------------------------------------------------------
    HardwareIndexBufferSharedPtr 
        DefaultHardwareBufferManagerBase::createIndexBuffer(HardwareIndexBuffer::IndexType itype, 
        size_t numIndexes, HardwareBuffer::Usage usage, bool useShadowBuffer)
    {
        DefaultHardwareIndexBuffer* ib = OGRE_NEW DefaultHardwareIndexBuffer(itype, numIndexes, usage);
        return HardwareIndexBufferSharedPtr(ib);
    }
    HardwareUniformBufferSharedPtr 
        DefaultHardwareBufferManagerBase::createUniformBuffer(size_t sizeBytes, 
                                    HardwareBuffer::Usage usage, bool useShadowBuffer, const String& name)
    {
        DefaultHardwareUniformBuffer* ub = OGRE_NEW DefaultHardwareUniformBuffer(this, sizeBytes, usage, useShadowBuffer);
        return HardwareUniformBufferSharedPtr(ub);
    }
    HardwareCounterBufferSharedPtr
    DefaultHardwareBufferManagerBase::createCounterBuffer(size_t sizeBytes,
                                                          HardwareBuffer::Usage usage, bool useShadowBuffer, const String& name)
    {
        DefaultHardwareUniformBuffer* ub = OGRE_NEW DefaultHardwareUniformBuffer(this, sizeBytes, usage, useShadowBuffer);
        return HardwareCounterBufferSharedPtr(ub);
    }
}
