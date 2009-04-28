/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/

#include "OgreGLESDefaultHardwareBufferManager.h"

namespace Ogre {
    GLESDefaultHardwareVertexBuffer::GLESDefaultHardwareVertexBuffer(size_t vertexSize,
                                                                 size_t numVertices,
                                                                 HardwareBuffer::Usage usage)
        : HardwareVertexBuffer(0, vertexSize, numVertices, usage, true, false)
    {
        mpData = static_cast<unsigned char*>(OGRE_MALLOC_SIMD(mSizeInBytes, MEMCATEGORY_GEOMETRY));
    }

    GLESDefaultHardwareVertexBuffer::~GLESDefaultHardwareVertexBuffer()
    {
        OGRE_FREE_SIMD(mpData, MEMCATEGORY_GEOMETRY);
    }

    void* GLESDefaultHardwareVertexBuffer::lockImpl(size_t offset,
                                                  size_t length,
                                                  LockOptions options)
    {
        return mpData + offset;
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
        return mpData + offset;
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
        memcpy(pDest, mpData + offset, length);
    }

    void GLESDefaultHardwareVertexBuffer::writeData(size_t offset,
                                                  size_t length,
                                                  const void* pSource,
                                                  bool discardWholeBuffer)
    {
        assert((offset + length) <= mSizeInBytes);
        // ignore discard, memory is not guaranteed to be zeroised
        memcpy(mpData + offset, pSource, length);
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
        mpData = new unsigned char[mSizeInBytes];
    }

    GLESDefaultHardwareIndexBuffer::~GLESDefaultHardwareIndexBuffer()
    {
        delete [] mpData;
    }

    void* GLESDefaultHardwareIndexBuffer::lockImpl(size_t offset, size_t length, LockOptions options)
    {
        // Only for use internally, no 'locking' as such
        return mpData + offset;
    }

    void GLESDefaultHardwareIndexBuffer::unlockImpl(void)
    {
        // Nothing to do
    }

    void* GLESDefaultHardwareIndexBuffer::lock(size_t offset, size_t length, LockOptions options)
    {
        mIsLocked = true;
        return mpData + offset;
    }

    void GLESDefaultHardwareIndexBuffer::unlock(void)
    {
        mIsLocked = false;
        // Nothing to do
    }

    void GLESDefaultHardwareIndexBuffer::readData(size_t offset, size_t length, void* pDest)
    {
        assert((offset + length) <= mSizeInBytes);
        memcpy(pDest, mpData + offset, length);
    }

    void GLESDefaultHardwareIndexBuffer::writeData(size_t offset, size_t length, const void* pSource,
            bool discardWholeBuffer)
    {
        assert((offset + length) <= mSizeInBytes);
        // ignore discard, memory is not guaranteed to be zeroised
        memcpy(mpData + offset, pSource, length);
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
            new GLESDefaultHardwareVertexBuffer(vertexSize, numVerts, usage));
    }

    HardwareIndexBufferSharedPtr
        GLESDefaultHardwareBufferManagerBase::createIndexBuffer(HardwareIndexBuffer::IndexType itype,
        size_t numIndexes, HardwareBuffer::Usage usage, bool useShadowBuffer)
    {
        return HardwareIndexBufferSharedPtr(
            new GLESDefaultHardwareIndexBuffer(itype, numIndexes, usage));
    }

	Ogre::RenderToVertexBufferSharedPtr GLESDefaultHardwareBufferManagerBase::createRenderToVertexBuffer( void )
	{
	        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
	                "Cannot create RenderToVertexBuffer in GLESDefaultHardwareBufferManagerBase", 
	                "GLESDefaultHardwareBufferManagerBase::createRenderToVertexBuffer");

		Ogre::RenderToVertexBufferSharedPtr  todo;
		return todo;
	}
}
