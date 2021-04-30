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
#include "OgreD3D11HardwareBufferManager.h"
#include "OgreD3D11HardwareBuffer.h"
#include "OgreD3D11VertexDeclaration.h"
#include "OgreD3D11RenderToVertexBuffer.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreD3D11Device.h"

namespace Ogre {
	//-----------------------------------------------------------------------
	D3D11HardwareBufferManager::D3D11HardwareBufferManager(D3D11Device & device)
		: mlpD3DDevice(device)
	{
	}
	//-----------------------------------------------------------------------
	D3D11HardwareBufferManager::~D3D11HardwareBufferManager()
	{
		destroyAllDeclarations();
		destroyAllBindings();
	}
	//-----------------------------------------------------------------------
	HardwareVertexBufferSharedPtr
		D3D11HardwareBufferManager::
		createVertexBuffer(size_t vertexSize, size_t numVerts, HardwareBuffer::Usage usage,
		bool useShadowBuffer)
	{
		assert(numVerts > 0);
		auto impl = new D3D11HardwareBuffer(D3D11HardwareBuffer::VERTEX_BUFFER, vertexSize * numVerts, usage, mlpD3DDevice, useShadowBuffer, false);
        auto buf = std::make_shared<HardwareVertexBuffer>(this, vertexSize, numVerts, impl);
        {
            OGRE_LOCK_MUTEX(mVertexBuffersMutex);
            mVertexBuffers.insert(buf.get());
        }
        return buf;
	}
	//-----------------------------------------------------------------------
	HardwareVertexBufferSharedPtr
		D3D11HardwareBufferManager::
		createStreamOutputVertexBuffer(size_t vertexSize, size_t numVerts, HardwareBuffer::Usage usage,
		bool useShadowBuffer)
	{
		assert(numVerts > 0);

		auto impl = new D3D11HardwareBuffer(D3D11HardwareBuffer::VERTEX_BUFFER, vertexSize * numVerts, usage, mlpD3DDevice, useShadowBuffer, true);
        auto buf = std::make_shared<HardwareVertexBuffer>(this, vertexSize, numVerts, impl);
        {
            OGRE_LOCK_MUTEX(mVertexBuffersMutex);
            mVertexBuffers.insert(buf.get());
        }
        return buf;
	}
	//-----------------------------------------------------------------------
	HardwareIndexBufferSharedPtr
		D3D11HardwareBufferManager::
		createIndexBuffer(HardwareIndexBuffer::IndexType itype, size_t numIndexes,
		HardwareBuffer::Usage usage, bool useShadowBuffer)
	{
		assert(numIndexes > 0);

        auto indexSize = HardwareIndexBuffer::indexSize(itype);
        auto impl = new D3D11HardwareBuffer(D3D11HardwareBuffer::INDEX_BUFFER, indexSize * numIndexes, usage, mlpD3DDevice, useShadowBuffer, false);

        return std::make_shared<HardwareIndexBuffer>(this, itype, numIndexes, impl);
	}
	//-----------------------------------------------------------------------
	RenderToVertexBufferSharedPtr
		D3D11HardwareBufferManager::createRenderToVertexBuffer()
	{
		return RenderToVertexBufferSharedPtr(new D3D11RenderToVertexBuffer(mlpD3DDevice, this));
	}
	//-----------------------------------------------------------------------
	HardwareBufferPtr
		D3D11HardwareBufferManager::createUniformBuffer(size_t sizeBytes, HardwareBufferUsage usage, bool useShadowBuffer)
	{
		assert(sizeBytes > 0);
		return std::make_shared<D3D11HardwareBuffer>(D3D11HardwareBuffer::CONSTANT_BUFFER,
										sizeBytes, usage, mlpD3DDevice, useShadowBuffer, false);
	}
	//-----------------------------------------------------------------------
	VertexDeclaration* D3D11HardwareBufferManager::createVertexDeclarationImpl(void)
	{
		return new D3D11VertexDeclaration(mlpD3DDevice);
	}
}
