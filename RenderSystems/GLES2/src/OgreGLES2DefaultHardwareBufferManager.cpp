/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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

#include "OgreGLES2DefaultHardwareBufferManager.h"

namespace Ogre {
    GLES2DefaultHardwareVertexBuffer::GLES2DefaultHardwareVertexBuffer(size_t vertexSize,
                                                                 size_t numVertices,
                                                                 HardwareBuffer::Usage usage)
        : HardwareVertexBuffer(0, vertexSize, numVertices, usage, true, false)
    {
        mpData = static_cast<unsigned char*>(OGRE_MALLOC_SIMD(mSizeInBytes, MEMCATEGORY_GEOMETRY));
    }

    GLES2DefaultHardwareVertexBuffer::~GLES2DefaultHardwareVertexBuffer()
    {
        OGRE_FREE_SIMD(mpData, MEMCATEGORY_GEOMETRY);
    }

    void* GLES2DefaultHardwareVertexBuffer::lockImpl(size_t offset,
                                                  size_t length,
                                                  LockOptions options)
    {
        return mpData + offset;
    }

    void GLES2DefaultHardwareVertexBuffer::unlockImpl(void)
    {
        // Nothing to do
    }

    void* GLES2DefaultHardwareVertexBuffer::lock(size_t offset,
                                              size_t length,
                                              LockOptions options)
    {
        mIsLocked = true;
        return mpData + offset;
    }

    void GLES2DefaultHardwareVertexBuffer::unlock(void)
    {
        mIsLocked = false;
        // Nothing to do
    }

    void GLES2DefaultHardwareVertexBuffer::readData(size_t offset,
                                                 size_t length,
                                                 void* pDest)
    {
        assert((offset + length) <= mSizeInBytes);
        memcpy(pDest, mpData + offset, length);
    }

    void GLES2DefaultHardwareVertexBuffer::writeData(size_t offset,
                                                  size_t length,
                                                  const void* pSource,
                                                  bool discardWholeBuffer)
    {
        assert((offset + length) <= mSizeInBytes);
        // ignore discard, memory is not guaranteed to be zeroised
        memcpy(mpData + offset, pSource, length);
    }

    GLES2DefaultHardwareIndexBuffer::GLES2DefaultHardwareIndexBuffer(IndexType idxType,
                                                               size_t numIndexes,
                                                               HardwareBuffer::Usage usage)
        : HardwareIndexBuffer(0, idxType, numIndexes, usage, true, false)
          // always software, never shadowed
    {
		if (idxType == HardwareIndexBuffer::IT_32BIT)
		{
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
				"32 bit hardware buffers are not allowed in OpenGL ES.",
				"GLES2DefaultHardwareIndexBuffer");
		}
        mpData = OGRE_NEW_FIX_FOR_WIN32 unsigned char[mSizeInBytes];
    }

    GLES2DefaultHardwareIndexBuffer::~GLES2DefaultHardwareIndexBuffer()
    {
        OGRE_DELETE [] mpData;
    }

    void* GLES2DefaultHardwareIndexBuffer::lockImpl(size_t offset, size_t length, LockOptions options)
    {
        // Only for use internally, no 'locking' as such
        return mpData + offset;
    }

    void GLES2DefaultHardwareIndexBuffer::unlockImpl(void)
    {
        // Nothing to do
    }

    void* GLES2DefaultHardwareIndexBuffer::lock(size_t offset, size_t length, LockOptions options)
    {
        mIsLocked = true;
        return mpData + offset;
    }

    void GLES2DefaultHardwareIndexBuffer::unlock(void)
    {
        mIsLocked = false;
        // Nothing to do
    }

    void GLES2DefaultHardwareIndexBuffer::readData(size_t offset, size_t length, void* pDest)
    {
        assert((offset + length) <= mSizeInBytes);
        memcpy(pDest, mpData + offset, length);
    }

    void GLES2DefaultHardwareIndexBuffer::writeData(size_t offset, size_t length, const void* pSource,
            bool discardWholeBuffer)
    {
        assert((offset + length) <= mSizeInBytes);
        // ignore discard, memory is not guaranteed to be zeroised
        memcpy(mpData + offset, pSource, length);
    }

    GLES2DefaultHardwareBufferManagerBase::GLES2DefaultHardwareBufferManagerBase()
    {
    }

    GLES2DefaultHardwareBufferManagerBase::~GLES2DefaultHardwareBufferManagerBase()
    {
        destroyAllDeclarations();
        destroyAllBindings();
    }

    HardwareVertexBufferSharedPtr
        GLES2DefaultHardwareBufferManagerBase::createVertexBuffer(size_t vertexSize,
        size_t numVerts, HardwareBuffer::Usage usage, bool useShadowBuffer)
    {
        return HardwareVertexBufferSharedPtr(
            OGRE_NEW GLES2DefaultHardwareVertexBuffer(vertexSize, numVerts, usage));
    }

    HardwareIndexBufferSharedPtr
        GLES2DefaultHardwareBufferManagerBase::createIndexBuffer(HardwareIndexBuffer::IndexType itype,
        size_t numIndexes, HardwareBuffer::Usage usage, bool useShadowBuffer)
    {
        return HardwareIndexBufferSharedPtr(
            OGRE_NEW GLES2DefaultHardwareIndexBuffer(itype, numIndexes, usage));
    }

	Ogre::RenderToVertexBufferSharedPtr GLES2DefaultHardwareBufferManagerBase::createRenderToVertexBuffer( void )
	{
        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                "Cannot create RenderToVertexBuffer in GLES2DefaultHardwareBufferManagerBase", 
                "GLES2DefaultHardwareBufferManagerBase::createRenderToVertexBuffer");
	}
}
