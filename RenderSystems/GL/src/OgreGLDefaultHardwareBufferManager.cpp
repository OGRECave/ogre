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
#include "OgreGLDefaultHardwareBufferManager.h"
#include "OgreException.h"

namespace Ogre {

	GLDefaultHardwareVertexBuffer::GLDefaultHardwareVertexBuffer(size_t vertexSize, size_t numVertices, 
																 HardwareBuffer::Usage usage)
	: HardwareVertexBuffer(0, vertexSize, numVertices, usage, true, false) // always software, never shadowed
	{
        mData = static_cast<unsigned char*>(OGRE_MALLOC_SIMD(mSizeInBytes, MEMCATEGORY_GEOMETRY));
	}
	//-----------------------------------------------------------------------
	GLDefaultHardwareVertexBuffer::GLDefaultHardwareVertexBuffer(HardwareBufferManagerBase* mgr, size_t vertexSize, size_t numVertices, 
		HardwareBuffer::Usage usage)
        : HardwareVertexBuffer(mgr, vertexSize, numVertices, usage, true, false) // always software, never shadowed
	{
        mData = static_cast<unsigned char*>(OGRE_MALLOC_SIMD(mSizeInBytes, MEMCATEGORY_GEOMETRY));
	}
	//-----------------------------------------------------------------------
    GLDefaultHardwareVertexBuffer::~GLDefaultHardwareVertexBuffer()
	{
		OGRE_FREE_SIMD(mData, MEMCATEGORY_GEOMETRY);
	}
	//-----------------------------------------------------------------------
    void* GLDefaultHardwareVertexBuffer::lockImpl(size_t offset, size_t length, LockOptions options)
	{
        // Only for use internally, no 'locking' as such
		return mData + offset;
	}
	//-----------------------------------------------------------------------
	void GLDefaultHardwareVertexBuffer::unlockImpl(void)
	{
        // Nothing to do
	}
	//-----------------------------------------------------------------------
    void* GLDefaultHardwareVertexBuffer::lock(size_t offset, size_t length, LockOptions options)
	{
        mIsLocked = true;
		return mData + offset;
	}
	//-----------------------------------------------------------------------
	void GLDefaultHardwareVertexBuffer::unlock(void)
	{
        mIsLocked = false;
        // Nothing to do
	}
	//-----------------------------------------------------------------------
    void GLDefaultHardwareVertexBuffer::readData(size_t offset, size_t length, void* pDest)
	{
		assert((offset + length) <= mSizeInBytes);
		memcpy(pDest, mData + offset, length);
	}
	//-----------------------------------------------------------------------
    void GLDefaultHardwareVertexBuffer::writeData(size_t offset, size_t length, const void* pSource,
			bool discardWholeBuffer)
	{
		assert((offset + length) <= mSizeInBytes);
		// ignore discard, memory is not guaranteed to be zeroised
		memcpy(mData + offset, pSource, length);

	}
	//-----------------------------------------------------------------------

	GLDefaultHardwareIndexBuffer::GLDefaultHardwareIndexBuffer(IndexType idxType, 
		size_t numIndexes, HardwareBuffer::Usage usage) 
		: HardwareIndexBuffer(0, idxType, numIndexes, usage, true, false) // always software, never shadowed
	{
		mData = new unsigned char[mSizeInBytes];
	}
	//-----------------------------------------------------------------------
    GLDefaultHardwareIndexBuffer::~GLDefaultHardwareIndexBuffer()
	{
		delete [] mData;
	}
	//-----------------------------------------------------------------------
    void* GLDefaultHardwareIndexBuffer::lockImpl(size_t offset, size_t length, LockOptions options)
	{
        // Only for use internally, no 'locking' as such
		return mData + offset;
	}
	//-----------------------------------------------------------------------
	void GLDefaultHardwareIndexBuffer::unlockImpl(void)
	{
        // Nothing to do
	}
	//-----------------------------------------------------------------------
    void* GLDefaultHardwareIndexBuffer::lock(size_t offset, size_t length, LockOptions options)
	{
        mIsLocked = true;
		return mData + offset;
	}
	//-----------------------------------------------------------------------
	void GLDefaultHardwareIndexBuffer::unlock(void)
	{
        mIsLocked = false;
        // Nothing to do
	}
	//-----------------------------------------------------------------------
    void GLDefaultHardwareIndexBuffer::readData(size_t offset, size_t length, void* pDest)
	{
		assert((offset + length) <= mSizeInBytes);
		memcpy(pDest, mData + offset, length);
	}
	//-----------------------------------------------------------------------
    void GLDefaultHardwareIndexBuffer::writeData(size_t offset, size_t length, const void* pSource,
			bool discardWholeBuffer)
	{
		assert((offset + length) <= mSizeInBytes);
		// ignore discard, memory is not guaranteed to be zeroised
		memcpy(mData + offset, pSource, length);

	}
	//-----------------------------------------------------------------------
	
	
    //-----------------------------------------------------------------------
    GLDefaultHardwareBufferManagerBase::GLDefaultHardwareBufferManagerBase()
	{
	}
    //-----------------------------------------------------------------------
    GLDefaultHardwareBufferManagerBase::~GLDefaultHardwareBufferManagerBase()
	{
        destroyAllDeclarations();
        destroyAllBindings();
	}
    //-----------------------------------------------------------------------
	HardwareVertexBufferSharedPtr 
        GLDefaultHardwareBufferManagerBase::createVertexBuffer(size_t vertexSize, 
		size_t numVerts, HardwareBuffer::Usage usage, bool useShadowBuffer)
	{
		return HardwareVertexBufferSharedPtr(
			new GLDefaultHardwareVertexBuffer(this, vertexSize, numVerts, usage));
	}
    //-----------------------------------------------------------------------
	HardwareIndexBufferSharedPtr 
        GLDefaultHardwareBufferManagerBase::createIndexBuffer(HardwareIndexBuffer::IndexType itype, 
		size_t numIndexes, HardwareBuffer::Usage usage, bool useShadowBuffer)
	{
		return HardwareIndexBufferSharedPtr(
			new GLDefaultHardwareIndexBuffer(itype, numIndexes, usage) );
	}
    //-----------------------------------------------------------------------
	RenderToVertexBufferSharedPtr 
		GLDefaultHardwareBufferManagerBase::createRenderToVertexBuffer()
	{
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"Cannot create RenderToVertexBuffer in GLDefaultHardwareBufferManagerBase", 
				"GLDefaultHardwareBufferManagerBase::createRenderToVertexBuffer");
	}
	//-----------------------------------------------------------------------
	HardwareUniformBufferSharedPtr 
		GLDefaultHardwareBufferManagerBase::createUniformBuffer(size_t sizeBytes, HardwareBuffer::Usage usage,bool useShadowBuffer, const String& name)
	{
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"Cannot create UniformBuffer in GLDefaultHardwareBufferManagerBase", 
				"GLDefaultHardwareBufferManagerBase::createUniformBuffer");
	}
	//-----------------------------------------------------------------------
    HardwareCounterBufferSharedPtr
        GLDefaultHardwareBufferManagerBase::createCounterBuffer(size_t sizeBytes,
                                                                HardwareBuffer::Usage usage,
                                                                bool useShadowBuffer, const String& name)
	{
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                    "Counter buffers not supported in OpenGL RenderSystem.",
                    "GLDefaultHardwareBufferManagerBase::createCounterBuffer");
	}
}
