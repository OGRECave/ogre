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

#ifndef __DefaultHardwareBufferManager_H__
#define __DefaultHardwareBufferManager_H__

#include "OgrePrerequisites.h"
#include "OgreHardwareBufferManager.h"
#include "OgreHardwareIndexBuffer.h"
#include "OgreHardwareVertexBuffer.h"

namespace Ogre {
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup RenderSystem
    *  @{
    */

    /// Specialisation of HardwareBuffer for emulation
    class _OgreExport DefaultHardwareBuffer : public HardwareBuffer
    {
    private:
        unsigned char* mData;
        void* lockImpl(size_t offset, size_t length, LockOptions options) override;
        void unlockImpl(void) override;
    public:
        DefaultHardwareBuffer(size_t sizeInBytes);
        ~DefaultHardwareBuffer();
        void readData(size_t offset, size_t length, void* pDest) override;
        void writeData(size_t offset, size_t length, const void* pSource, bool discardWholeBuffer = false) override;
    };

    typedef DefaultHardwareBuffer DefaultHardwareUniformBuffer;

    class _OgreExport DefaultHardwareVertexBuffer : public HardwareVertexBuffer
    {
    public:
        DefaultHardwareVertexBuffer(size_t vertexSize, size_t numVertices, Usage = HBU_CPU_ONLY)
            : HardwareVertexBuffer(NULL, vertexSize, numVertices,
                                   new DefaultHardwareBuffer(vertexSize * numVertices))
        {
        }
    };

    class _OgreExport DefaultHardwareIndexBuffer : public HardwareIndexBuffer
    {
    public:
        DefaultHardwareIndexBuffer(IndexType idxType, size_t numIndexes, Usage = HBU_CPU_ONLY)
            : HardwareIndexBuffer(NULL, idxType, numIndexes,
                                  new DefaultHardwareBuffer(indexSize(idxType) * numIndexes))
        {
        }
    };

    /** Specialisation of HardwareBufferManagerBase to emulate hardware buffers.
    @remarks
        You might want to instantiate this class if you want to utilise
        classes like MeshSerializer without having initialised the 
        rendering system (which is required to create a 'real' hardware
        buffer manager).
    */
    class _OgreExport DefaultHardwareBufferManagerBase : public HardwareBufferManagerBase
    {
    public:
        DefaultHardwareBufferManagerBase();
        ~DefaultHardwareBufferManagerBase();
        /// Creates a vertex buffer
        HardwareVertexBufferSharedPtr 
            createVertexBuffer(size_t vertexSize, size_t numVerts, 
                HardwareBuffer::Usage usage, bool useShadowBuffer = false) override;
        /// Create a hardware index buffer
        HardwareIndexBufferSharedPtr 
            createIndexBuffer(HardwareIndexBuffer::IndexType itype, size_t numIndexes, 
                HardwareBuffer::Usage usage, bool useShadowBuffer = false) override;
        /// Create a hardware uniform buffer
        HardwareBufferPtr createUniformBuffer(size_t sizeBytes, HardwareBufferUsage = HBU_CPU_ONLY,
                                              bool = false) override
        {
            return std::make_shared<DefaultHardwareBuffer>(sizeBytes);
        }
    };

    /// DefaultHardwareBufferManager as a Singleton
    class _OgreExport DefaultHardwareBufferManager : public HardwareBufferManager
    {
        std::unique_ptr<HardwareBufferManagerBase> mImpl;
    public:
        DefaultHardwareBufferManager() : mImpl(new DefaultHardwareBufferManagerBase()) {}
        ~DefaultHardwareBufferManager()
        {
            // have to do this before mImpl is gone
            destroyAllDeclarations();
            destroyAllBindings();
        }

        HardwareVertexBufferSharedPtr
            createVertexBuffer(size_t vertexSize, size_t numVerts, HardwareBuffer::Usage usage,
            bool useShadowBuffer = false)
        {
            return mImpl->createVertexBuffer(vertexSize, numVerts, usage, useShadowBuffer);
        }

        HardwareIndexBufferSharedPtr
            createIndexBuffer(HardwareIndexBuffer::IndexType itype, size_t numIndexes,
            HardwareBuffer::Usage usage, bool useShadowBuffer = false)
        {
            return mImpl->createIndexBuffer(itype, numIndexes, usage, useShadowBuffer);
        }

        RenderToVertexBufferSharedPtr createRenderToVertexBuffer()
        {
            return mImpl->createRenderToVertexBuffer();
        }

        HardwareBufferPtr createUniformBuffer(size_t sizeBytes, HardwareBufferUsage usage,
                                              bool useShadowBuffer)
        {
            return mImpl->createUniformBuffer(sizeBytes, usage, useShadowBuffer);
        }
    };

    /** @} */
    /** @} */

}

#endif
