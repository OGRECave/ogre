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
#include "OgreD3D10HardwareBufferManager.h"
#include "OgreD3D10HardwareVertexBuffer.h"
#include "OgreD3D10HardwareIndexBuffer.h"
#include "OgreD3D10VertexDeclaration.h"
#include "OgreD3D10RenderToVertexBuffer.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreD3D10Device.h"

namespace Ogre {
	//-----------------------------------------------------------------------
	D3D10HardwareBufferManagerBase::D3D10HardwareBufferManagerBase(D3D10Device & device)
		: mlpD3DDevice(device)
	{
	}
	//-----------------------------------------------------------------------
	D3D10HardwareBufferManagerBase::~D3D10HardwareBufferManagerBase()
	{
		destroyAllDeclarations();
		destroyAllBindings();
	}
	//-----------------------------------------------------------------------
	HardwareVertexBufferSharedPtr 
		D3D10HardwareBufferManagerBase::
		createVertexBuffer(size_t vertexSize, size_t numVerts, HardwareBuffer::Usage usage,
		bool useShadowBuffer)
	{
		assert (numVerts > 0);
		D3D10HardwareVertexBuffer* vbuf = new D3D10HardwareVertexBuffer(
			this, vertexSize, numVerts, usage, mlpD3DDevice, false, useShadowBuffer);
		{
			OGRE_LOCK_MUTEX(mVertexBuffersMutex)
				mVertexBuffers.insert(vbuf);
		}
		return HardwareVertexBufferSharedPtr(vbuf);
	}
	//-----------------------------------------------------------------------
	HardwareIndexBufferSharedPtr 
		D3D10HardwareBufferManagerBase::
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
		D3D10HardwareIndexBuffer* idx = new D3D10HardwareIndexBuffer(
			this, itype, numIndexes, usage, mlpD3DDevice, false, useShadowBuffer);
		{
			OGRE_LOCK_MUTEX(mIndexBuffersMutex)
				mIndexBuffers.insert(idx);
		}
		return HardwareIndexBufferSharedPtr(idx);

	}
	//-----------------------------------------------------------------------
	RenderToVertexBufferSharedPtr 
		D3D10HardwareBufferManagerBase::createRenderToVertexBuffer()
	{
		return RenderToVertexBufferSharedPtr(new D3D10RenderToVertexBuffer());
	}
	//-----------------------------------------------------------------------
	VertexDeclaration* D3D10HardwareBufferManagerBase::createVertexDeclarationImpl(void)
	{
		return new D3D10VertexDeclaration(mlpD3DDevice);
	}
	//-----------------------------------------------------------------------
	void D3D10HardwareBufferManagerBase::destroyVertexDeclarationImpl(VertexDeclaration* decl)
	{
		delete decl;
	}
	//-----------------------------------------------------------------------
	void D3D10HardwareBufferManagerBase::releaseDefaultPoolResources(void)
	{
		size_t iCount = 0;
		size_t vCount = 0;


		{
			OGRE_LOCK_MUTEX(mVertexBuffersMutex)
				VertexBufferList::iterator v, vend;
			vend = mVertexBuffers.end();
			for (v = mVertexBuffers.begin(); v != vend; ++v)
			{
				D3D10HardwareVertexBuffer* vbuf = 
					static_cast<D3D10HardwareVertexBuffer*>(*v);
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
				D3D10HardwareIndexBuffer* ibuf = 
					static_cast<D3D10HardwareIndexBuffer*>(*i);
				if (ibuf->releaseIfDefaultPool())
					iCount++;

			}
		}

		LogManager::getSingleton().logMessage("D3D10HardwareBufferManagerBase released:");
		LogManager::getSingleton().logMessage(
			StringConverter::toString(vCount) + " unmanaged vertex buffers");
		LogManager::getSingleton().logMessage(
			StringConverter::toString(iCount) + " unmanaged index buffers");
	}
	//-----------------------------------------------------------------------
	void D3D10HardwareBufferManagerBase::recreateDefaultPoolResources(void)
	{
		size_t iCount = 0;
		size_t vCount = 0;

		{
			OGRE_LOCK_MUTEX(mVertexBuffersMutex)
				VertexBufferList::iterator v, vend;
			vend = mVertexBuffers.end();
			for (v = mVertexBuffers.begin(); v != vend; ++v)
			{
				D3D10HardwareVertexBuffer* vbuf = 
					static_cast<D3D10HardwareVertexBuffer*>(*v);
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
				D3D10HardwareIndexBuffer* ibuf = 
					static_cast<D3D10HardwareIndexBuffer*>(*i);
				if (ibuf->recreateIfDefaultPool(mlpD3DDevice))
					iCount++;

			}
		}

		LogManager::getSingleton().logMessage("D3D10HardwareBufferManagerBase recreated:");
		LogManager::getSingleton().logMessage(
			StringConverter::toString(vCount) + " unmanaged vertex buffers");
		LogManager::getSingleton().logMessage(
			StringConverter::toString(iCount) + " unmanaged index buffers");
	}
	//-----------------------------------------------------------------------

}
