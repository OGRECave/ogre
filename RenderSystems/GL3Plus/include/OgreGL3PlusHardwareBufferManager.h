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

#ifndef __GL3PlusHardwareBufferManager_H__
#define __GL3PlusHardwareBufferManager_H__

#include "OgreGL3PlusPrerequisites.h"
#include "OgreHardwareBufferManager.h"

namespace Ogre {

    /** Implementation of HardwareBufferManager for OpenGL. */
    class _OgreGL3PlusExport GL3PlusHardwareBufferManager : public HardwareBufferManager
    {
    protected:
        GL3PlusRenderSystem* mRenderSystem;

        size_t mUniformBufferCount;
        size_t mShaderStorageBufferCount;

        VertexDeclaration* createVertexDeclarationImpl(void) override;
    public:
        GL3PlusHardwareBufferManager();
        ~GL3PlusHardwareBufferManager();
        /// Creates a vertex buffer
        HardwareVertexBufferSharedPtr createVertexBuffer(size_t vertexSize,
                                                         size_t numVerts, HardwareBuffer::Usage usage, bool useShadowBuffer = false) override;
        /// Create a hardware vertex buffer
        HardwareIndexBufferSharedPtr createIndexBuffer(
            HardwareIndexBuffer::IndexType itype, size_t numIndexes,
            HardwareBuffer::Usage usage, bool useShadowBuffer = false) override;
        /// Create a uniform buffer
        HardwareBufferPtr createUniformBuffer(size_t sizeBytes, HardwareBufferUsage usage = HBU_CPU_TO_GPU,
                                              bool useShadowBuffer = false) override;

        /// Create a shader storage buffer.
        HardwareBufferPtr createShaderStorageBuffer(size_t sizeBytes,
                                                    HardwareBufferUsage usage = HBU_CPU_TO_GPU,
                                                    bool useShadowBuffer = false);

        /// Create a render to vertex buffer
        RenderToVertexBufferSharedPtr createRenderToVertexBuffer() override;

        size_t getUniformBufferCount() { return mUniformBufferCount; }
        size_t getShaderStorageBufferCount() { return mShaderStorageBufferCount; }

        /// Utility function to get the correct GL type based on VET's
        static GLenum getGLType(VertexElementType type);

        void notifyContextDestroyed(GLContext* context);
    };
}

#endif
