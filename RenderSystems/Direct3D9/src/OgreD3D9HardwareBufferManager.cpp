/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#include "OgreD3D9HardwareBufferManager.h"
#include "OgreD3D9HardwareVertexBuffer.h"
#include "OgreD3D9HardwareIndexBuffer.h"
#include "OgreD3D9VertexDeclaration.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreException.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    D3D9HardwareBufferManager::D3D9HardwareBufferManager(LPDIRECT3DDEVICE9 device)
        : mlpD3DDevice(device)
    {
    }
    //-----------------------------------------------------------------------
    D3D9HardwareBufferManager::~D3D9HardwareBufferManager()
    {
        destroyAllDeclarations();
        destroyAllBindings();
    }
    //-----------------------------------------------------------------------
    HardwareVertexBufferSharedPtr 
    D3D9HardwareBufferManager::
    createVertexBuffer(size_t vertexSize, size_t numVerts, HardwareBuffer::Usage usage,
		bool useShadowBuffer)
    {
		assert (numVerts > 0);
#if OGRE_D3D_MANAGE_BUFFERS
        // Override shadow buffer setting; managed buffers are automatically
        // backed by system memory
        // Don't override shadow buffer if discardable, since then we use
        // unmanaged buffers for speed (avoids write-through overhead)
        if (useShadowBuffer && !(usage & HardwareBuffer::HBU_DISCARDABLE))
        {
            useShadowBuffer = false;
            // Also drop any WRITE_ONLY so we can read direct
            if (usage == HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY)
            {
                usage = HardwareBuffer::HBU_DYNAMIC;
            }
            else if (usage == HardwareBuffer::HBU_STATIC_WRITE_ONLY)
            {
                usage = HardwareBuffer::HBU_STATIC;
            }
        }
#endif
		D3D9HardwareVertexBuffer* vbuf = new D3D9HardwareVertexBuffer(
			vertexSize, numVerts, usage, mlpD3DDevice, false, useShadowBuffer);
		{
			OGRE_LOCK_MUTEX(mVertexBuffersMutex)
			mVertexBuffers.insert(vbuf);
		}
        return HardwareVertexBufferSharedPtr(vbuf);
    }
    //-----------------------------------------------------------------------
	HardwareIndexBufferSharedPtr 
    D3D9HardwareBufferManager::
    createIndexBuffer(HardwareIndexBuffer::IndexType itype, size_t numIndexes, 
        HardwareBuffer::Usage usage, bool useShadowBuffer)
    {
		assert (numIndexes > 0);
#if OGRE_D3D_MANAGE_BUFFERS
        // Override shadow buffer setting; managed buffers are automatically
        // backed by system memory
        if (useShadowBuffer)
        {
            useShadowBuffer = false;
            // Also drop any WRITE_ONLY so we can read direct
            if (usage == HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY)
            {
                usage = HardwareBuffer::HBU_DYNAMIC;
            }
            else if (usage == HardwareBuffer::HBU_STATIC_WRITE_ONLY)
            {
                usage = HardwareBuffer::HBU_STATIC;
            }
        }
#endif
		D3D9HardwareIndexBuffer* idx = new D3D9HardwareIndexBuffer(
			itype, numIndexes, usage, mlpD3DDevice, false, useShadowBuffer);
		{
			OGRE_LOCK_MUTEX(mIndexBuffersMutex)
			mIndexBuffers.insert(idx);
		}
		return HardwareIndexBufferSharedPtr(idx);
            
    }
    //-----------------------------------------------------------------------
    RenderToVertexBufferSharedPtr 
        D3D9HardwareBufferManager::createRenderToVertexBuffer()
    {
        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
            "Direct3D9 does not support render to vertex buffer objects", 
            "D3D9HardwareBufferManager::createRenderToVertexBuffer");
	}
    //-----------------------------------------------------------------------
    VertexDeclaration* D3D9HardwareBufferManager::createVertexDeclarationImpl(void)
    {
        return new D3D9VertexDeclaration(mlpD3DDevice);
    }
    //-----------------------------------------------------------------------
    void D3D9HardwareBufferManager::destroyVertexDeclarationImpl(VertexDeclaration* decl)
    {
        delete decl;
    }
	//-----------------------------------------------------------------------
	void D3D9HardwareBufferManager::releaseDefaultPoolResources(void)
	{
		size_t iCount = 0;
		size_t vCount = 0;

		
		{
			OGRE_LOCK_MUTEX(mVertexBuffersMutex)
			VertexBufferList::iterator v, vend;
			vend = mVertexBuffers.end();
			for (v = mVertexBuffers.begin(); v != vend; ++v)
			{
				D3D9HardwareVertexBuffer* vbuf = 
					static_cast<D3D9HardwareVertexBuffer*>(*v);
				if (vbuf->releaseIfDefaultPool())
					vCount++;
			}
		}

		{
			OGRE_LOCK_MUTEX(mIndexBuffersMutex)
			IndexBufferList::iterator i, iend;
			iend = mIndexBuffers.end();
			for (i = mIndexBuffers.begin(); i != iend; ++i)
			{
				D3D9HardwareIndexBuffer* ibuf = 
					static_cast<D3D9HardwareIndexBuffer*>(*i);
				if (ibuf->releaseIfDefaultPool())
					iCount++;

			}
		}

		LogManager::getSingleton().logMessage("D3D9HardwareBufferManager released:");
		LogManager::getSingleton().logMessage(
			StringConverter::toString(vCount) + " unmanaged vertex buffers");
		LogManager::getSingleton().logMessage(
			StringConverter::toString(iCount) + " unmanaged index buffers");
	}
	//-----------------------------------------------------------------------
	void D3D9HardwareBufferManager::recreateDefaultPoolResources(void)
	{
		size_t iCount = 0;
		size_t vCount = 0;

		{
			OGRE_LOCK_MUTEX(mVertexBuffersMutex)
			VertexBufferList::iterator v, vend;
			vend = mVertexBuffers.end();
			for (v = mVertexBuffers.begin(); v != vend; ++v)
			{
				D3D9HardwareVertexBuffer* vbuf = 
					static_cast<D3D9HardwareVertexBuffer*>(*v);
				if (vbuf->recreateIfDefaultPool(mlpD3DDevice))
					vCount++;
			}
		}

		{
			OGRE_LOCK_MUTEX(mIndexBuffersMutex)
			IndexBufferList::iterator i, iend;
			iend = mIndexBuffers.end();
			for (i = mIndexBuffers.begin(); i != iend; ++i)
			{
				D3D9HardwareIndexBuffer* ibuf = 
					static_cast<D3D9HardwareIndexBuffer*>(*i);
				if (ibuf->recreateIfDefaultPool(mlpD3DDevice))
					iCount++;

			}
		}

		LogManager::getSingleton().logMessage("D3D9HardwareBufferManager recreated:");
		LogManager::getSingleton().logMessage(
			StringConverter::toString(vCount) + " unmanaged vertex buffers");
		LogManager::getSingleton().logMessage(
			StringConverter::toString(iCount) + " unmanaged index buffers");
	}
	//-----------------------------------------------------------------------

}
