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

#include "OgreGL3PlusHardwareBufferManager.h"
#include "OgreGL3PlusHardwareCounterBuffer.h"
#include "OgreGL3PlusHardwareIndexBuffer.h"
#include "OgreGL3PlusHardwareUniformBuffer.h"
#include "OgreGL3PlusHardwareVertexBuffer.h"
#include "OgreGL3PlusRenderToVertexBuffer.h"
#include "OgreRoot.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    // Scratch pool management (32 bit structure)
    struct GL3PlusScratchBufferAlloc
    {
        /// Size in bytes
        uint32 size: 31;
        /// Free? (pack with size)
        uint32 free: 1;
    };
    #define SCRATCH_POOL_SIZE 1 * 1024 * 1024
    #define SCRATCH_ALIGNMENT 32

    GL3PlusHardwareBufferManagerBase::GL3PlusHardwareBufferManagerBase()
		: mScratchBufferPool(NULL), mMapBufferThreshold(OGRE_GL_DEFAULT_MAP_BUFFER_THRESHOLD)
    {
        // Init scratch pool
        // TODO make it a configurable size?
        // 32-bit aligned buffer
        mScratchBufferPool = static_cast<char*>(OGRE_MALLOC_ALIGN(SCRATCH_POOL_SIZE,
                                                                  MEMCATEGORY_GEOMETRY,
                                                                  SCRATCH_ALIGNMENT));
        GL3PlusScratchBufferAlloc* ptrAlloc = (GL3PlusScratchBufferAlloc*)mScratchBufferPool;
        ptrAlloc->size = SCRATCH_POOL_SIZE - sizeof(GL3PlusScratchBufferAlloc);
        ptrAlloc->free = 1;

		// Win32 machines with ATI GPU are having issues glMapBuffer, looks like buffer corruption
		// disable for now until we figure out where the problem lies			
#	if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		if (Root::getSingleton().getRenderSystem()->getCapabilities()->getVendor() == GPU_AMD) 
		{
			mMapBufferThreshold = 0xffffffffUL  /* maximum unsigned long value */;
		}
#	endif

    }
    //-----------------------------------------------------------------------
    GL3PlusHardwareBufferManagerBase::~GL3PlusHardwareBufferManagerBase()
    {
        destroyAllDeclarations();
        destroyAllBindings();

        OGRE_FREE_ALIGN(mScratchBufferPool, MEMCATEGORY_GEOMETRY, SCRATCH_ALIGNMENT);
    }

    HardwareVertexBufferSharedPtr
        GL3PlusHardwareBufferManagerBase::createVertexBuffer(size_t vertexSize,
                                                      size_t numVerts,
                                                      HardwareBuffer::Usage usage,
                                                      bool useShadowBuffer)
    {
        GL3PlusHardwareVertexBuffer* buf =
            OGRE_NEW GL3PlusHardwareVertexBuffer(this, vertexSize, numVerts, usage, useShadowBuffer);
        {
            OGRE_LOCK_MUTEX(mVertexBuffersMutex);
            mVertexBuffers.insert(buf);
        }
        return HardwareVertexBufferSharedPtr(buf);
    }

    HardwareIndexBufferSharedPtr GL3PlusHardwareBufferManagerBase::createIndexBuffer(HardwareIndexBuffer::IndexType itype,
                                                                              size_t numIndexes,
                                                                              HardwareBuffer::Usage usage,
                                                                              bool useShadowBuffer)
    {
        GL3PlusHardwareIndexBuffer* buf =
            new GL3PlusHardwareIndexBuffer(this, itype, numIndexes, usage, useShadowBuffer);
        {
            OGRE_LOCK_MUTEX(mIndexBuffersMutex);
            mIndexBuffers.insert(buf);
        }
        return HardwareIndexBufferSharedPtr(buf);
    }

    HardwareUniformBufferSharedPtr GL3PlusHardwareBufferManagerBase::createUniformBuffer(size_t sizeBytes, HardwareBuffer::Usage usage, bool useShadowBuffer, const String& name)
    {
        GL3PlusHardwareUniformBuffer* buf =
        new GL3PlusHardwareUniformBuffer(this, sizeBytes, usage, useShadowBuffer, name);
        {
            OGRE_LOCK_MUTEX(mUniformBuffersMutex);
            mUniformBuffers.insert(buf);
        }
        return HardwareUniformBufferSharedPtr(buf);
    }

    HardwareCounterBufferSharedPtr GL3PlusHardwareBufferManagerBase::createCounterBuffer(size_t sizeBytes, HardwareBuffer::Usage usage, bool useShadowBuffer, const String& name)
    {
        GL3PlusHardwareCounterBuffer* buf =
        new GL3PlusHardwareCounterBuffer(this, name);
        {
            OGRE_LOCK_MUTEX(mCounterBuffersMutex);
            mCounterBuffers.insert(buf);
        }
        return HardwareCounterBufferSharedPtr(buf);
    }

	RenderToVertexBufferSharedPtr GL3PlusHardwareBufferManagerBase::createRenderToVertexBuffer()
	{
		return RenderToVertexBufferSharedPtr(new GL3PlusRenderToVertexBuffer);
	}

    GLenum GL3PlusHardwareBufferManagerBase::getGLUsage(unsigned int usage)
    {
        switch(usage)
        {
            case HardwareBuffer::HBU_STATIC:
            case HardwareBuffer::HBU_STATIC_WRITE_ONLY:
                return GL_STATIC_DRAW;
            case HardwareBuffer::HBU_DYNAMIC:
            case HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY:
                return GL_DYNAMIC_DRAW;
            case HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE:
                return GL_STREAM_DRAW;
            default:
                return GL_DYNAMIC_DRAW;
        };
    }

    GLenum GL3PlusHardwareBufferManagerBase::getGLType(unsigned int type)
    {
        switch(type)
        {
            case VET_FLOAT1:
            case VET_FLOAT2:
            case VET_FLOAT3:
            case VET_FLOAT4:
                return GL_FLOAT;
            case VET_DOUBLE1:
            case VET_DOUBLE2:
            case VET_DOUBLE3:
            case VET_DOUBLE4:
                return GL_DOUBLE;
            case VET_INT1:
            case VET_INT2:
            case VET_INT3:
            case VET_INT4:
                return GL_INT;
            case VET_UINT1:
            case VET_UINT2:
            case VET_UINT3:
            case VET_UINT4:
                return GL_UNSIGNED_INT;
            case VET_SHORT1:
            case VET_SHORT2:
            case VET_SHORT3:
            case VET_SHORT4:
                return GL_SHORT;
            case VET_USHORT1:
            case VET_USHORT2:
            case VET_USHORT3:
            case VET_USHORT4:
                return GL_UNSIGNED_SHORT;
            case VET_COLOUR:
            case VET_COLOUR_ABGR:
            case VET_COLOUR_ARGB:
            case VET_UBYTE4:
                return GL_UNSIGNED_BYTE;
            default:
                return 0;
        };
    }

    void* GL3PlusHardwareBufferManagerBase::allocateScratch(uint32 size)
    {
        // simple forward link search based on alloc sizes
        // not that fast but the list should never get that long since not many
        // locks at once (hopefully)
        OGRE_LOCK_MUTEX(mScratchMutex);

        // Alignment - round up the size to 32 bits
        // control blocks are 32 bits too so this packs nicely
        if (size % 4 != 0)
        {
            size += 4 - (size % 4);
        }

        uint32 bufferPos = 0;
        while (bufferPos < SCRATCH_POOL_SIZE)
        {
            GL3PlusScratchBufferAlloc* pNext = (GL3PlusScratchBufferAlloc*)(mScratchBufferPool + bufferPos);
            // Big enough?
            if (pNext->free && pNext->size >= size)
            {
                // split? And enough space for control block
                if(pNext->size > size + sizeof(GL3PlusScratchBufferAlloc))
                {
                    uint32 offset = (uint32)sizeof(GL3PlusScratchBufferAlloc) + size;

                    GL3PlusScratchBufferAlloc* pSplitAlloc = (GL3PlusScratchBufferAlloc*)
                        (mScratchBufferPool + bufferPos + offset);
                    pSplitAlloc->free = 1;
                    // split size is remainder minus new control block
                    pSplitAlloc->size = pNext->size - size - sizeof(GL3PlusScratchBufferAlloc);

                    // New size of current
                    pNext->size = size;
                }
                // allocate and return
                pNext->free = 0;

                // return pointer just after this control block (++ will do that for us)
                return ++pNext;
            }

            bufferPos += (uint32)sizeof(GL3PlusScratchBufferAlloc) + pNext->size;
        }

        // no available alloc
        return 0;
    }

    void GL3PlusHardwareBufferManagerBase::deallocateScratch(void* ptr)
    {
        OGRE_LOCK_MUTEX(mScratchMutex);

        // Simple linear search dealloc
        uint32 bufferPos = 0;
        GL3PlusScratchBufferAlloc* pLast = 0;
        while (bufferPos < SCRATCH_POOL_SIZE)
        {
            GL3PlusScratchBufferAlloc* pCurrent = (GL3PlusScratchBufferAlloc*)(mScratchBufferPool + bufferPos);

            // Pointers match?
            if ((mScratchBufferPool + bufferPos + sizeof(GL3PlusScratchBufferAlloc)) == ptr)
            {
                // dealloc
                pCurrent->free = 1;
                // merge with previous
                if (pLast && pLast->free)
                {
                    // adjust buffer pos
                    bufferPos -= (pLast->size + (uint32)sizeof(GL3PlusScratchBufferAlloc));
                    // merge free space
                    pLast->size += pCurrent->size + sizeof(GL3PlusScratchBufferAlloc);
                    pCurrent = pLast;
                }

                // merge with next
                uint32 offset = bufferPos + pCurrent->size + (uint32)sizeof(GL3PlusScratchBufferAlloc);
                if (offset < SCRATCH_POOL_SIZE)
                {
                    GL3PlusScratchBufferAlloc* pNext = (GL3PlusScratchBufferAlloc*)(
                        mScratchBufferPool + offset);
                    if (pNext->free)
                    {
                        pCurrent->size += pNext->size + sizeof(GL3PlusScratchBufferAlloc);
                    }
                }

                // done
                return;
            }

            bufferPos += (uint32)sizeof(GL3PlusScratchBufferAlloc) + pCurrent->size;
            pLast = pCurrent;

        }

        // Should never get here unless there's a corruption
        assert(false && "Memory deallocation error");
    }
	//---------------------------------------------------------------------
	size_t GL3PlusHardwareBufferManagerBase::getGLMapBufferThreshold() const
	{
		return mMapBufferThreshold;
	}
	//---------------------------------------------------------------------
	void GL3PlusHardwareBufferManagerBase::setGLMapBufferThreshold( const size_t value )
	{
		mMapBufferThreshold = value;
	}
}
