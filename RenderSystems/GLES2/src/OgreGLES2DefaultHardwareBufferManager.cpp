/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
