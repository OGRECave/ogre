/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
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
#include "OgreStableHeaders.h"
#include "OgreDefaultHardwareBufferManager.h"

namespace Ogre {

	DefaultHardwareVertexBuffer::DefaultHardwareVertexBuffer(size_t vertexSize, size_t numVertices, 
		HardwareBuffer::Usage usage)
        : HardwareVertexBuffer(0, vertexSize, numVertices, usage, true, false) // always software, never shadowed
	{
        // Allocate aligned memory for better SIMD processing friendly.
        mpData = static_cast<unsigned char*>(OGRE_MALLOC_SIMD(mSizeInBytes, MEMCATEGORY_GEOMETRY));
	}
	//-----------------------------------------------------------------------
    DefaultHardwareVertexBuffer::~DefaultHardwareVertexBuffer()
	{
		OGRE_FREE_SIMD(mpData, MEMCATEGORY_GEOMETRY);
	}
	//-----------------------------------------------------------------------
    void* DefaultHardwareVertexBuffer::lockImpl(size_t offset, size_t length, LockOptions options)
	{
        // Only for use internally, no 'locking' as such
		return mpData + offset;
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
		return mpData + offset;
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
		memcpy(pDest, mpData + offset, length);
	}
	//-----------------------------------------------------------------------
    void DefaultHardwareVertexBuffer::writeData(size_t offset, size_t length, const void* pSource,
			bool discardWholeBuffer)
	{
		assert((offset + length) <= mSizeInBytes);
		// ignore discard, memory is not guaranteed to be zeroised
		memcpy(mpData + offset, pSource, length);

	}
	//-----------------------------------------------------------------------

	DefaultHardwareIndexBuffer::DefaultHardwareIndexBuffer(IndexType idxType, 
		size_t numIndexes, HardwareBuffer::Usage usage) 
		: HardwareIndexBuffer(0, idxType, numIndexes, usage, true, false) // always software, never shadowed
	{
		mpData = OGRE_ALLOC_T(unsigned char, mSizeInBytes, MEMCATEGORY_GEOMETRY);
	}
	//-----------------------------------------------------------------------
    DefaultHardwareIndexBuffer::~DefaultHardwareIndexBuffer()
	{
		OGRE_FREE(mpData, MEMCATEGORY_GEOMETRY);
	}
	//-----------------------------------------------------------------------
    void* DefaultHardwareIndexBuffer::lockImpl(size_t offset, size_t length, LockOptions options)
	{
        // Only for use internally, no 'locking' as such
		return mpData + offset;
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
		return mpData + offset;
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
		memcpy(pDest, mpData + offset, length);
	}
	//-----------------------------------------------------------------------
    void DefaultHardwareIndexBuffer::writeData(size_t offset, size_t length, const void* pSource,
			bool discardWholeBuffer)
	{
		assert((offset + length) <= mSizeInBytes);
		// ignore discard, memory is not guaranteed to be zeroised
		memcpy(mpData + offset, pSource, length);

	}
	//-----------------------------------------------------------------------
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
        DefaultHardwareVertexBuffer* vb = OGRE_NEW DefaultHardwareVertexBuffer(vertexSize, numVerts, usage);
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
	//-----------------------------------------------------------------------
	RenderToVertexBufferSharedPtr
		DefaultHardwareBufferManagerBase::createRenderToVertexBuffer()
	{
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
			"Cannot create RenderToVertexBuffer in DefaultHardwareBufferManagerBase", 
			"DefaultHardwareBufferManagerBase::createRenderToVertexBuffer");
	}
}
