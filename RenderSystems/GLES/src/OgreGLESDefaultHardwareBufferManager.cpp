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

#include "OgreGLESDefaultHardwareBufferManager.h"

namespace Ogre {
    GLESDefaultHardwareVertexBuffer::GLESDefaultHardwareVertexBuffer(size_t vertexSize,
                                                                 size_t numVertices,
                                                                 HardwareBuffer::Usage usage)
        : HardwareVertexBuffer(0, vertexSize, numVertices, usage, true, false)
    {
        mData = static_cast<unsigned char*>(OGRE_MALLOC_SIMD(mSizeInBytes, MEMCATEGORY_GEOMETRY));
    }

    GLESDefaultHardwareVertexBuffer::GLESDefaultHardwareVertexBuffer(HardwareBufferManagerBase* mgr,
                                                                     size_t vertexSize,
                                                                     size_t numVertices,
                                                                     HardwareBuffer::Usage usage)
    : HardwareVertexBuffer(mgr, vertexSize, numVertices, usage, true, false)
    {
        mData = static_cast<unsigned char*>(OGRE_MALLOC_SIMD(mSizeInBytes, MEMCATEGORY_GEOMETRY));
    }
    
    GLESDefaultHardwareVertexBuffer::~GLESDefaultHardwareVertexBuffer()
    {
        OGRE_FREE_SIMD(mData, MEMCATEGORY_GEOMETRY);
    }

    void* GLESDefaultHardwareVertexBuffer::lockImpl(size_t offset,
                                                  size_t length,
                                                  LockOptions options)
    {
        return mData + offset;
    }

    void GLESDefaultHardwareVertexBuffer::unlockImpl(void)
    {
        // Nothing to do
    }

    void* GLESDefaultHardwareVertexBuffer::lock(size_t offset,
                                              size_t length,
                                              LockOptions options)
    {
        mIsLocked = true;
        return mData + offset;
    }

    void GLESDefaultHardwareVertexBuffer::unlock(void)
    {
        mIsLocked = false;
        // Nothing to do
    }

    void GLESDefaultHardwareVertexBuffer::readData(size_t offset,
                                                 size_t length,
                                                 void* pDest)
    {
        assert((offset + length) <= mSizeInBytes);
        memcpy(pDest, mData + offset, length);
    }

    void GLESDefaultHardwareVertexBuffer::writeData(size_t offset,
                                                  size_t length,
                                                  const void* pSource,
                                                  bool discardWholeBuffer)
    {
        assert((offset + length) <= mSizeInBytes);
        // ignore discard, memory is not guaranteed to be zeroised
        memcpy(mData + offset, pSource, length);
    }

    GLESDefaultHardwareIndexBuffer::GLESDefaultHardwareIndexBuffer(IndexType idxType,
                                                               size_t numIndexes,
                                                               HardwareBuffer::Usage usage)
        : HardwareIndexBuffer(0, idxType, numIndexes, usage, true, false)
          // always software, never shadowed
    {
		if (idxType == HardwareIndexBuffer::IT_32BIT)
		{
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
				"32 bit hardware buffers are not allowed in OpenGL ES.",
				"GLESDefaultHardwareIndexBuffer");
		}
        mData = new unsigned char[mSizeInBytes];
    }

    GLESDefaultHardwareIndexBuffer::~GLESDefaultHardwareIndexBuffer()
    {
        delete [] mData;
    }

    void* GLESDefaultHardwareIndexBuffer::lockImpl(size_t offset, size_t length, LockOptions options)
    {
        // Only for use internally, no 'locking' as such
        return mData + offset;
    }

    void GLESDefaultHardwareIndexBuffer::unlockImpl(void)
    {
        // Nothing to do
    }

    void* GLESDefaultHardwareIndexBuffer::lock(size_t offset, size_t length, LockOptions options)
    {
        mIsLocked = true;
        return mData + offset;
    }

    void GLESDefaultHardwareIndexBuffer::unlock(void)
    {
        mIsLocked = false;
        // Nothing to do
    }

    void GLESDefaultHardwareIndexBuffer::readData(size_t offset, size_t length, void* pDest)
    {
        assert((offset + length) <= mSizeInBytes);
        memcpy(pDest, mData + offset, length);
    }

    void GLESDefaultHardwareIndexBuffer::writeData(size_t offset, size_t length, const void* pSource,
            bool discardWholeBuffer)
    {
        assert((offset + length) <= mSizeInBytes);
        // ignore discard, memory is not guaranteed to be zeroised
        memcpy(mData + offset, pSource, length);
    }

    GLESDefaultHardwareBufferManagerBase::GLESDefaultHardwareBufferManagerBase()
    {
    }

    GLESDefaultHardwareBufferManagerBase::~GLESDefaultHardwareBufferManagerBase()
    {
        destroyAllDeclarations();
        destroyAllBindings();
    }

    HardwareVertexBufferSharedPtr
        GLESDefaultHardwareBufferManagerBase::createVertexBuffer(size_t vertexSize,
        size_t numVerts, HardwareBuffer::Usage usage, bool useShadowBuffer)
    {
        return HardwareVertexBufferSharedPtr(
            OGRE_NEW GLESDefaultHardwareVertexBuffer(this, vertexSize, numVerts, usage));
    }

    HardwareIndexBufferSharedPtr
        GLESDefaultHardwareBufferManagerBase::createIndexBuffer(HardwareIndexBuffer::IndexType itype,
        size_t numIndexes, HardwareBuffer::Usage usage, bool useShadowBuffer)
    {
        return HardwareIndexBufferSharedPtr(
            OGRE_NEW GLESDefaultHardwareIndexBuffer(itype, numIndexes, usage));
    }

	Ogre::RenderToVertexBufferSharedPtr GLESDefaultHardwareBufferManagerBase::createRenderToVertexBuffer( void )
	{
        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                "Cannot create RenderToVertexBuffer in GLESDefaultHardwareBufferManagerBase", 
                "GLESDefaultHardwareBufferManagerBase::createRenderToVertexBuffer");
	}
}
