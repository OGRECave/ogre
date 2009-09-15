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
#include "OgreD3D9HardwareBufferManager.h"
#include "OgreD3D9HardwareVertexBuffer.h"
#include "OgreD3D9HardwareIndexBuffer.h"
#include "OgreD3D9VertexDeclaration.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreException.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    D3D9HardwareBufferManagerBase::D3D9HardwareBufferManagerBase()       
    {
    }
    //-----------------------------------------------------------------------
    D3D9HardwareBufferManagerBase::~D3D9HardwareBufferManagerBase()
    {
        destroyAllDeclarations();
        destroyAllBindings();
    }
    //-----------------------------------------------------------------------
    HardwareVertexBufferSharedPtr 
    D3D9HardwareBufferManagerBase::
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
			this, vertexSize, numVerts, usage, false, useShadowBuffer);
		{
			OGRE_LOCK_MUTEX(mVertexBuffersMutex)
			mVertexBuffers.insert(vbuf);
		}
        return HardwareVertexBufferSharedPtr(vbuf);
    }
    //-----------------------------------------------------------------------
	HardwareIndexBufferSharedPtr 
    D3D9HardwareBufferManagerBase::
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
			this, itype, numIndexes, usage, false, useShadowBuffer);
		{
			OGRE_LOCK_MUTEX(mIndexBuffersMutex)
			mIndexBuffers.insert(idx);
		}
		return HardwareIndexBufferSharedPtr(idx);
            
    }
    //-----------------------------------------------------------------------
    RenderToVertexBufferSharedPtr 
        D3D9HardwareBufferManagerBase::createRenderToVertexBuffer()
    {
        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
            "Direct3D9 does not support render to vertex buffer objects", 
            "D3D9HardwareBufferManagerBase::createRenderToVertexBuffer");
	}
    //-----------------------------------------------------------------------
    VertexDeclaration* D3D9HardwareBufferManagerBase::createVertexDeclarationImpl(void)
    {
        return new D3D9VertexDeclaration();
    }
    //-----------------------------------------------------------------------
    void D3D9HardwareBufferManagerBase::destroyVertexDeclarationImpl(VertexDeclaration* decl)
    {
        delete decl;
    }
}
