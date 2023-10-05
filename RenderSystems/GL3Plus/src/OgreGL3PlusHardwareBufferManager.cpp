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
#include "OgreGL3PlusHardwareBuffer.h"
#include "OgreGL3PlusRenderToVertexBuffer.h"
#include "OgreGL3PlusRenderSystem.h"
#include "OgreRoot.h"
#include "OgreGLVertexArrayObject.h"

namespace Ogre {
    GL3PlusHardwareBufferManager::GL3PlusHardwareBufferManager() : mUniformBufferCount(0), mShaderStorageBufferCount(0)
    {
        mRenderSystem = static_cast<GL3PlusRenderSystem*>(Root::getSingleton().getRenderSystem());
    }

    GL3PlusHardwareBufferManager::~GL3PlusHardwareBufferManager()
    {
        destroyAllDeclarations();
        destroyAllBindings();
    }

    void GL3PlusHardwareBufferManager::notifyContextDestroyed(GLContext* context)
    {
        OGRE_LOCK_MUTEX(mVertexDeclarationsMutex);
        for(auto& d : mVertexDeclarations)
            static_cast<GLVertexArrayObject*>(d)->notifyContextDestroyed(context);
    }

    HardwareVertexBufferSharedPtr
    GL3PlusHardwareBufferManager::createVertexBuffer(size_t vertexSize,
                                                         size_t numVerts,
                                                         HardwareBuffer::Usage usage,
                                                         bool useShadowBuffer)
    {
        auto impl = new GL3PlusHardwareBuffer(GL_ARRAY_BUFFER, vertexSize * numVerts, usage, useShadowBuffer);
        auto buf = std::make_shared<HardwareVertexBuffer>(this, vertexSize, numVerts, impl);
        {
            OGRE_LOCK_MUTEX(mVertexBuffersMutex);
            mVertexBuffers.insert(buf.get());
        }
        return buf;
    }

    HardwareIndexBufferSharedPtr GL3PlusHardwareBufferManager::createIndexBuffer(HardwareIndexBuffer::IndexType itype,
                                                                                     size_t numIndexes,
                                                                                     HardwareBuffer::Usage usage,
                                                                                     bool useShadowBuffer)
    {
        // Calculate the size of the indexes
        auto indexSize = HardwareIndexBuffer::indexSize(itype);
        auto impl = new GL3PlusHardwareBuffer(GL_ELEMENT_ARRAY_BUFFER, indexSize * numIndexes, usage, useShadowBuffer);

        return std::make_shared<HardwareIndexBuffer>(this, itype, numIndexes, impl);
    }

    HardwareBufferPtr GL3PlusHardwareBufferManager::createUniformBuffer(size_t sizeBytes, HardwareBufferUsage usage, bool useShadowBuffer)
    {
        mUniformBufferCount++;
        return std::make_shared<GL3PlusHardwareBuffer>(GL_UNIFORM_BUFFER, sizeBytes, usage, useShadowBuffer);
    }

    HardwareBufferPtr GL3PlusHardwareBufferManager::createShaderStorageBuffer(size_t sizeBytes, HardwareBufferUsage usage, bool useShadowBuffer)
    {
        mShaderStorageBufferCount++;
        return std::make_shared<GL3PlusHardwareBuffer>(GL_SHADER_STORAGE_BUFFER, sizeBytes, usage, useShadowBuffer);
    }

    RenderToVertexBufferSharedPtr GL3PlusHardwareBufferManager::createRenderToVertexBuffer()
    {
        return RenderToVertexBufferSharedPtr(new GL3PlusRenderToVertexBuffer);
    }

    VertexDeclaration* GL3PlusHardwareBufferManager::createVertexDeclarationImpl(void)
    {
        return OGRE_NEW GLVertexArrayObject();
    }

    GLenum GL3PlusHardwareBufferManager::getGLType(VertexElementType type)
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
        case VET_SHORT2_NORM:
        case VET_SHORT4_NORM:
            return GL_SHORT;
        case VET_USHORT1:
        case VET_USHORT2:
        case VET_USHORT3:
        case VET_USHORT4:
        case VET_USHORT2_NORM:
        case VET_USHORT4_NORM:
            return GL_UNSIGNED_SHORT;
        case VET_UBYTE4:
        case VET_UBYTE4_NORM:
        case _DETAIL_SWAP_RB:
            return GL_UNSIGNED_BYTE;
        case VET_BYTE4:
        case VET_BYTE4_NORM:
            return GL_BYTE;
        case VET_INT_10_10_10_2_NORM:
            return GL_INT_2_10_10_10_REV;
        };

        OgreAssert(false, "unknown Vertex Element Type");
        return 0;
    }
}
