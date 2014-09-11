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

#include "OgreGLES2DefaultHardwareBufferManager.h"
#include "OgreRoot.h"
#include "OgreGLES2RenderSystem.h"
#include "OgreGLES2Util.h"

namespace Ogre {
    GLES2DefaultHardwareVertexBuffer::GLES2DefaultHardwareVertexBuffer(size_t vertexSize,
                                                                 size_t numVertices,
                                                                 HardwareBuffer::Usage usage)
        : HardwareVertexBuffer(0, vertexSize, numVertices, usage, true, false)
    {
        mData = static_cast<unsigned char*>(OGRE_MALLOC_SIMD(mSizeInBytes, MEMCATEGORY_GEOMETRY));
    }

    GLES2DefaultHardwareVertexBuffer::GLES2DefaultHardwareVertexBuffer(HardwareBufferManagerBase* mgr,
                                                                       size_t vertexSize,
                                                                       size_t numVertices,
                                                                       HardwareBuffer::Usage usage)
        : HardwareVertexBuffer(mgr, vertexSize, numVertices, usage, true, false)
    {
        mData = static_cast<unsigned char*>(OGRE_MALLOC_SIMD(mSizeInBytes, MEMCATEGORY_GEOMETRY));
    }
    
    GLES2DefaultHardwareVertexBuffer::~GLES2DefaultHardwareVertexBuffer()
    {
        OGRE_FREE_SIMD(mData, MEMCATEGORY_GEOMETRY);
    }

    void* GLES2DefaultHardwareVertexBuffer::lockImpl(size_t offset,
                                                  size_t length,
                                                  LockOptions options)
    {
        return mData + offset;
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
        return mData + offset;
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
        memcpy(pDest, mData + offset, length);
    }

    void GLES2DefaultHardwareVertexBuffer::writeData(size_t offset,
                                                  size_t length,
                                                  const void* pSource,
                                                  bool discardWholeBuffer)
    {
        assert((offset + length) <= mSizeInBytes);
        // ignore discard, memory is not guaranteed to be zeroised
        memcpy(mData + offset, pSource, length);
    }

    GLES2DefaultHardwareIndexBuffer::GLES2DefaultHardwareIndexBuffer(IndexType idxType,
                                                               size_t numIndexes,
                                                               HardwareBuffer::Usage usage)
        : HardwareIndexBuffer(0, idxType, numIndexes, usage, true, false)
          // always software, never shadowed
    {
#if OGRE_NO_GLES3_SUPPORT == 1
		if (!Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_32BIT_INDEX) &&
            idxType == HardwareIndexBuffer::IT_32BIT)
		{
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
				"32 bit hardware buffers are not allowed in OpenGL ES.",
				"GLES2DefaultHardwareIndexBuffer");
		}
#endif
        mData = new unsigned char[mSizeInBytes];
    }

    GLES2DefaultHardwareIndexBuffer::~GLES2DefaultHardwareIndexBuffer()
    {
        delete [] mData;
    }

    void* GLES2DefaultHardwareIndexBuffer::lockImpl(size_t offset, size_t length, LockOptions options)
    {
        // Only for use internally, no 'locking' as such
        return mData + offset;
    }

    void GLES2DefaultHardwareIndexBuffer::unlockImpl(void)
    {
        // Nothing to do
    }

    void* GLES2DefaultHardwareIndexBuffer::lock(size_t offset, size_t length, LockOptions options)
    {
        mIsLocked = true;
        return mData + offset;
    }

    void GLES2DefaultHardwareIndexBuffer::unlock(void)
    {
        mIsLocked = false;
        // Nothing to do
    }

    void GLES2DefaultHardwareIndexBuffer::readData(size_t offset, size_t length, void* pDest)
    {
        assert((offset + length) <= mSizeInBytes);
        memcpy(pDest, mData + offset, length);
    }

    void GLES2DefaultHardwareIndexBuffer::writeData(size_t offset, size_t length, const void* pSource,
            bool discardWholeBuffer)
    {
        assert((offset + length) <= mSizeInBytes);
        // ignore discard, memory is not guaranteed to be zeroised
        memcpy(mData + offset, pSource, length);
    }

    GLES2DefaultHardwareUniformBuffer::GLES2DefaultHardwareUniformBuffer(size_t bufferSize,
                                                                             HardwareBuffer::Usage usage,
                                                                             bool useShadowBuffer, const String& name)
    : HardwareUniformBuffer(0, bufferSize, usage, useShadowBuffer, name)
    {
        mData = static_cast<unsigned char*>(OGRE_MALLOC_SIMD(mSizeInBytes, MEMCATEGORY_GEOMETRY));
    }

    GLES2DefaultHardwareUniformBuffer::GLES2DefaultHardwareUniformBuffer(HardwareBufferManagerBase* mgr,
                                                                             size_t bufferSize,
                                                                             HardwareBuffer::Usage usage,
                                                                             bool useShadowBuffer, const String& name)
    : HardwareUniformBuffer(mgr, bufferSize, usage, useShadowBuffer, name)
    {
        mData = static_cast<unsigned char*>(OGRE_MALLOC_SIMD(mSizeInBytes, MEMCATEGORY_GEOMETRY));
    }

    GLES2DefaultHardwareUniformBuffer::~GLES2DefaultHardwareUniformBuffer()
    {
        OGRE_FREE_SIMD(mData, MEMCATEGORY_GEOMETRY);
    }

    void* GLES2DefaultHardwareUniformBuffer::lockImpl(size_t offset,
                                                        size_t length,
                                                        LockOptions options)
    {
        return mData + offset;
    }

    void GLES2DefaultHardwareUniformBuffer::unlockImpl(void)
    {
        // Nothing to do
    }

    void* GLES2DefaultHardwareUniformBuffer::lock(size_t offset,
                                                    size_t length,
                                                    LockOptions options)
    {
        mIsLocked = true;
        return mData + offset;
    }

    void GLES2DefaultHardwareUniformBuffer::unlock(void)
    {
        mIsLocked = false;
        // Nothing to do
    }

    void GLES2DefaultHardwareUniformBuffer::readData(size_t offset,
                                                       size_t length,
                                                       void* pDest)
    {
        assert((offset + length) <= mSizeInBytes);
        memcpy(pDest, mData + offset, length);
    }

    void GLES2DefaultHardwareUniformBuffer::writeData(size_t offset,
                                                        size_t length,
                                                        const void* pSource,
                                                        bool discardWholeBuffer)
    {
        assert((offset + length) <= mSizeInBytes);
        // ignore discard, memory is not guaranteed to be zeroised
        memcpy(mData + offset, pSource, length);
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

    HardwareUniformBufferSharedPtr
    GLES2DefaultHardwareBufferManagerBase::createUniformBuffer(size_t sizeBytes, HardwareBuffer::Usage usage,
                                                                 bool useShadowBuffer, const String& name)
	{
        if(!gleswIsSupported(3, 0))
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        "GLES2 does not support uniform buffer objects",
                        "GLES2DefaultHardwareBufferManager::createUniformBuffer");
        }

        return HardwareUniformBufferSharedPtr(new GLES2DefaultHardwareUniformBuffer(this, sizeBytes, usage, useShadowBuffer, name));
	}
    
	Ogre::RenderToVertexBufferSharedPtr GLES2DefaultHardwareBufferManagerBase::createRenderToVertexBuffer( void )
	{
        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                "Cannot create RenderToVertexBuffer in GLES2DefaultHardwareBufferManagerBase", 
                "GLES2DefaultHardwareBufferManagerBase::createRenderToVertexBuffer");
	}
}
