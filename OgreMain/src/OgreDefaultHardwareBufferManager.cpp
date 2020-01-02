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

    DefaultHardwareVertexBuffer::DefaultHardwareVertexBuffer(size_t vertexSize, size_t numVertices, 
                                                             HardwareBuffer::Usage usage)
    : HardwareVertexBuffer(0, vertexSize, numVertices, usage, true, false) // always software, never shadowed
    {
        // Allocate aligned memory for better SIMD processing friendly.
        mData = static_cast<unsigned char*>(OGRE_MALLOC_SIMD(mSizeInBytes, MEMCATEGORY_GEOMETRY));
    }
    //-----------------------------------------------------------------------
    DefaultHardwareVertexBuffer::DefaultHardwareVertexBuffer(HardwareBufferManagerBase* mgr, size_t vertexSize, size_t numVertices, 
        HardwareBuffer::Usage usage)
        : HardwareVertexBuffer(mgr, vertexSize, numVertices, usage, true, false) // always software, never shadowed
    {
        // Allocate aligned memory for better SIMD processing friendly.
        mData = static_cast<unsigned char*>(OGRE_MALLOC_SIMD(mSizeInBytes, MEMCATEGORY_GEOMETRY));
    }
    //-----------------------------------------------------------------------
    DefaultHardwareVertexBuffer::~DefaultHardwareVertexBuffer()
    {
        OGRE_FREE_SIMD(mData, MEMCATEGORY_GEOMETRY);
    }
    //-----------------------------------------------------------------------
    void* DefaultHardwareVertexBuffer::lockImpl(size_t offset, size_t length, LockOptions options)
    {
        // Only for use internally, no 'locking' as such
        return mData + offset;
    }
    //-----------------------------------------------------------------------
    void DefaultHardwareVertexBuffer::unlockImpl(void)
    {
        // Nothing to do
    }
    //-----------------------------------------------------------------------
    void* DefaultHardwareVertexBuffer::lock(size_t offset, size_t length, LockOptions options)
    {
        mIsLocked = true;
        return mData + offset;
    }
    //-----------------------------------------------------------------------
    void DefaultHardwareVertexBuffer::unlock(void)
    {
        mIsLocked = false;
        // Nothing to do
    }
    //-----------------------------------------------------------------------
    void DefaultHardwareVertexBuffer::readData(size_t offset, size_t length, void* pDest)
    {
        assert((offset + length) <= mSizeInBytes);
        memcpy(pDest, mData + offset, length);
    }
    //-----------------------------------------------------------------------
    void DefaultHardwareVertexBuffer::writeData(size_t offset, size_t length, const void* pSource,
            bool discardWholeBuffer)
    {
        assert((offset + length) <= mSizeInBytes);
        // ignore discard, memory is not guaranteed to be zeroised
        memcpy(mData + offset, pSource, length);

    }
    //-----------------------------------------------------------------------

    DefaultHardwareIndexBuffer::DefaultHardwareIndexBuffer(IndexType idxType, 
        size_t numIndexes, HardwareBuffer::Usage usage) 
        : HardwareIndexBuffer(0, idxType, numIndexes, usage, true, false) // always software, never shadowed
    {
        mData = OGRE_ALLOC_T(unsigned char, mSizeInBytes, MEMCATEGORY_GEOMETRY);
    }
    //-----------------------------------------------------------------------
    DefaultHardwareIndexBuffer::~DefaultHardwareIndexBuffer()
    {
        OGRE_FREE(mData, MEMCATEGORY_GEOMETRY);
    }
    //-----------------------------------------------------------------------
    void* DefaultHardwareIndexBuffer::lockImpl(size_t offset, size_t length, LockOptions options)
    {
        // Only for use internally, no 'locking' as such
        return mData + offset;
    }
    //-----------------------------------------------------------------------
    void DefaultHardwareIndexBuffer::unlockImpl(void)
    {
        // Nothing to do
    }
    //-----------------------------------------------------------------------
    void* DefaultHardwareIndexBuffer::lock(size_t offset, size_t length, LockOptions options)
    {
        mIsLocked = true;
        return mData + offset;
    }
    //-----------------------------------------------------------------------
    void DefaultHardwareIndexBuffer::unlock(void)
    {
        mIsLocked = false;
        // Nothing to do
    }
    //-----------------------------------------------------------------------
    void DefaultHardwareIndexBuffer::readData(size_t offset, size_t length, void* pDest)
    {
        assert((offset + length) <= mSizeInBytes);
        memcpy(pDest, mData + offset, length);
    }
    //-----------------------------------------------------------------------
    void DefaultHardwareIndexBuffer::writeData(size_t offset, size_t length, const void* pSource,
            bool discardWholeBuffer)
    {
        assert((offset + length) <= mSizeInBytes);
        // ignore discard, memory is not guaranteed to be zeroised
        memcpy(mData + offset, pSource, length);

    }
    //-----------------------------------------------------------------------
    DefaultHardwareUniformBuffer::DefaultHardwareUniformBuffer(HardwareBufferManagerBase* mgr, size_t sizeBytes, HardwareBuffer::Usage usage, bool useShadowBuffer, const String& name)
        : HardwareUniformBuffer(mgr, sizeBytes, usage, useShadowBuffer, name)
    {
        // Allocate aligned memory for better SIMD processing friendly.
        mData = static_cast<unsigned char*>(OGRE_MALLOC_SIMD(mSizeInBytes, MEMCATEGORY_GEOMETRY));
    }
    //-----------------------------------------------------------------------
    DefaultHardwareUniformBuffer::~DefaultHardwareUniformBuffer()
    {
        OGRE_FREE_SIMD(mData, MEMCATEGORY_GEOMETRY);
    }
    //-----------------------------------------------------------------------
    void* DefaultHardwareUniformBuffer::lockImpl(size_t offset, size_t length, LockOptions options)
    {
        // Only for use internally, no 'locking' as such
        return mData + offset;
    }
    //-----------------------------------------------------------------------
    void DefaultHardwareUniformBuffer::unlockImpl(void)
    {
        // Nothing to do
    }
    /*
    bool DefaultHardwareUniformBuffer::updateStructure(const Any& renderSystemInfo)
    {
        // Nothing to do
        return true;
    }
    */
    //-----------------------------------------------------------------------
    void* DefaultHardwareUniformBuffer::lock(size_t offset, size_t length, LockOptions options)
    {
        mIsLocked = true;
        return mData + offset;
    }
    //-----------------------------------------------------------------------
    void DefaultHardwareUniformBuffer::unlock(void)
    {
        mIsLocked = false;
        // Nothing to do
    }
    //-----------------------------------------------------------------------
    void DefaultHardwareUniformBuffer::readData(size_t offset, size_t length, void* pDest)
    {
        assert((offset + length) <= mSizeInBytes);
        memcpy(pDest, mData + offset, length);
    }
    //-----------------------------------------------------------------------
    void DefaultHardwareUniformBuffer::writeData(size_t offset, size_t length, const void* pSource,
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
