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
#include "OgreD3D9HardwareVertexBuffer.h"
#include "OgreD3D9Mappings.h"
#include "OgreException.h"
#include "OgreD3D9HardwareBufferManager.h"

namespace Ogre {

	//---------------------------------------------------------------------
    D3D9HardwareVertexBuffer::D3D9HardwareVertexBuffer(size_t vertexSize, 
        size_t numVertices, HardwareBuffer::Usage usage, LPDIRECT3DDEVICE9 pDev, 
        bool useSystemMemory, bool useShadowBuffer)
        : HardwareVertexBuffer(vertexSize, numVertices, usage, useSystemMemory, useShadowBuffer)
    {
        // Create the vertex buffer
#if OGRE_D3D_MANAGE_BUFFERS
		mD3DPool = useSystemMemory? D3DPOOL_SYSTEMMEM : 
			// If not system mem, use managed pool UNLESS buffer is discardable
			// if discardable, keeping the software backing is expensive
			(usage & HardwareBuffer::HBU_DISCARDABLE)? D3DPOOL_DEFAULT : D3DPOOL_MANAGED;
#else
		mD3DPool = useSystemMemory? D3DPOOL_SYSTEMMEM : D3DPOOL_DEFAULT;
#endif
        HRESULT hr = pDev->CreateVertexBuffer(
            static_cast<UINT>(mSizeInBytes), 
            D3D9Mappings::get(usage), 
            0, // No FVF here, thankyou
			mD3DPool,
            &mlpD3DBuffer,
            NULL);
        if (FAILED(hr))
        {
			String msg = DXGetErrorDescription9(hr);
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"Cannot create D3D9 vertex buffer: " + msg, 
                "D3D9HardwareVertexBuffer::D3D9HardwareVertexBuffer");
        }

    }
	//---------------------------------------------------------------------
    D3D9HardwareVertexBuffer::~D3D9HardwareVertexBuffer()
    {
        SAFE_RELEASE(mlpD3DBuffer);
    }
	//---------------------------------------------------------------------
    void* D3D9HardwareVertexBuffer::lockImpl(size_t offset, 
        size_t length, LockOptions options)
    {
        void* pBuf;
        HRESULT hr = mlpD3DBuffer->Lock(
            static_cast<UINT>(offset), 
            static_cast<UINT>(length), 
            &pBuf,
            D3D9Mappings::get(options, mUsage));

        if (FAILED(hr))
        {
			String msg = DXGetErrorDescription9(hr);
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"Cannot lock D3D9 vertex buffer: " + msg, 
                "D3D9HardwareVertexBuffer::lock");
        }

        return pBuf;
    }
	//---------------------------------------------------------------------
	void D3D9HardwareVertexBuffer::unlockImpl(void)
    {
        HRESULT hr = mlpD3DBuffer->Unlock();
    }
	//---------------------------------------------------------------------
    void D3D9HardwareVertexBuffer::readData(size_t offset, size_t length, 
        void* pDest)
    {
        // There is no functional interface in D3D, just do via manual 
        // lock, copy & unlock
        void* pSrc = this->lock(offset, length, HardwareBuffer::HBL_READ_ONLY);
        memcpy(pDest, pSrc, length);
        this->unlock();

    }
	//---------------------------------------------------------------------
    void D3D9HardwareVertexBuffer::writeData(size_t offset, size_t length, 
            const void* pSource,
			bool discardWholeBuffer)
    {
        // There is no functional interface in D3D, just do via manual 
        // lock, copy & unlock
        void* pDst = this->lock(offset, length, 
            discardWholeBuffer ? HardwareBuffer::HBL_DISCARD : HardwareBuffer::HBL_NORMAL);
        memcpy(pDst, pSource, length);
        this->unlock();
    }
	//---------------------------------------------------------------------
	bool D3D9HardwareVertexBuffer::releaseIfDefaultPool(void)
	{
		if (mD3DPool == D3DPOOL_DEFAULT)
		{
			SAFE_RELEASE(mlpD3DBuffer);
			return true;
		}
		return false;
	}
	//---------------------------------------------------------------------
	bool D3D9HardwareVertexBuffer::recreateIfDefaultPool(LPDIRECT3DDEVICE9 pDev)
	{
		if (mD3DPool == D3DPOOL_DEFAULT)
		{
			HRESULT hr = pDev->CreateVertexBuffer(
				static_cast<UINT>(mSizeInBytes), 
				D3D9Mappings::get(mUsage), 
				0, // No FVF here, thankyou
				mD3DPool,
				&mlpD3DBuffer,
				NULL);
			if (FAILED(hr))
			{
				String msg = DXGetErrorDescription9(hr);
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
					"Cannot restore D3D9 vertex buffer: " + msg, 
					"D3D9HardwareVertexBuffer::recreateIfDefaultPool");
			}
			return true;
		}
		return false;
	}
	//---------------------------------------------------------------------
}
