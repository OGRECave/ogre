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
#include "OgreGL3PlusHardwareIndexBuffer.h"
#include "OgreGL3PlusHardwareUniformBuffer.h"
#include "OgreGL3PlusHardwareVertexBuffer.h"
#include "OgreGL3PlusRenderToVertexBuffer.h"
#include "OgreGL3PlusRenderSystem.h"
#include "OgreRoot.h"
#include "OgreGLVertexArrayObject.h"

namespace Ogre {
    GL3PlusHardwareBufferManager::GL3PlusHardwareBufferManager()
    {
        mRenderSystem = static_cast<GL3PlusRenderSystem*>(Root::getSingleton().getRenderSystem());
    }

    GL3PlusHardwareBufferManager::~GL3PlusHardwareBufferManager()
    {
        mShaderStorageBuffers.clear();

        destroyAllDeclarations();
        destroyAllBindings();
    }

    GL3PlusStateCacheManager * GL3PlusHardwareBufferManager::getStateCacheManager()
    {
        return mRenderSystem->_getStateCacheManager();
    }

    void GL3PlusHardwareBufferManager::notifyContextDestroyed(GLContext* context)
    {
        OGRE_LOCK_MUTEX(mVertexDeclarationsMutex);
        for(VertexDeclarationList::iterator it = mVertexDeclarations.begin(), it_end = mVertexDeclarations.end(); it != it_end; ++it)
            static_cast<GLVertexArrayObject*>(*it)->notifyContextDestroyed(context);
    }

    HardwareVertexBufferSharedPtr
    GL3PlusHardwareBufferManager::createVertexBuffer(size_t vertexSize,
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

    HardwareIndexBufferSharedPtr GL3PlusHardwareBufferManager::createIndexBuffer(HardwareIndexBuffer::IndexType itype,
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

    HardwareUniformBufferSharedPtr GL3PlusHardwareBufferManager::createUniformBuffer(size_t sizeBytes, HardwareBuffer::Usage usage, bool useShadowBuffer, const String& name)
    {
        HardwareUniformBuffer* buf =
            new GL3PlusHardwareUniformBuffer(this, sizeBytes, usage, useShadowBuffer, name, GL_UNIFORM_BUFFER);
        {
            OGRE_LOCK_MUTEX(mUniformBuffersMutex);
            mUniformBuffers.insert(buf);
        }
        return HardwareUniformBufferSharedPtr(buf);
    }

    HardwareUniformBufferSharedPtr GL3PlusHardwareBufferManager::createShaderStorageBuffer(size_t sizeBytes, HardwareBuffer::Usage usage, bool useShadowBuffer, const String& name)
    {
        HardwareUniformBuffer* buf =
            new GL3PlusHardwareUniformBuffer(this, sizeBytes, usage, useShadowBuffer, name, GL_SHADER_STORAGE_BUFFER);
        {
            OGRE_LOCK_MUTEX(mUniformBuffersMutex);
            mShaderStorageBuffers.insert(buf);
        }
        return HardwareUniformBufferSharedPtr(buf);
    }

    HardwareCounterBufferSharedPtr GL3PlusHardwareBufferManager::createCounterBuffer(size_t sizeBytes, HardwareBuffer::Usage usage, bool useShadowBuffer, const String& name)
    {
        HardwareUniformBuffer* buf =
                new GL3PlusHardwareUniformBuffer(this, sizeBytes, usage, useShadowBuffer, name, GL_ATOMIC_COUNTER_BUFFER);

        {
            OGRE_LOCK_MUTEX(mCounterBuffersMutex);
            mCounterBuffers.insert(buf);
        }
        return HardwareCounterBufferSharedPtr(buf);
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
        case VET_COLOUR:
        case VET_COLOUR_ABGR:
        case VET_COLOUR_ARGB:
        case VET_UBYTE4:
        case VET_UBYTE4_NORM:
            return GL_UNSIGNED_BYTE;
        case VET_BYTE4:
        case VET_BYTE4_NORM:
            return GL_BYTE;
        };

        OgreAssert(false, "unknown Vertex Element Type");
        return 0;
    }
}
