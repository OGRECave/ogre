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

#include "OgreGLES2HardwareBufferManager.h"
#include "OgreGLES2HardwareVertexBuffer.h"
#include "OgreGLES2HardwareIndexBuffer.h"
#include "OgreGLES2HardwareUniformBuffer.h"
#include "OgreGLES2VertexDeclaration.h"
#include "OgreGLES2RenderToVertexBuffer.h"
#include "OgreGLES2RenderSystem.h"
#include "OgreGLES2Support.h"
#include "OgreGLES2Util.h"
#include "OgreRoot.h"

namespace Ogre {
namespace v1 {
    //-----------------------------------------------------------------------
    GLES2HardwareBufferManagerBase::GLES2HardwareBufferManagerBase()
    {
        mStateCacheManager = dynamic_cast<GLES2RenderSystem*>(Root::getSingleton().getRenderSystem())->getGLES2Support()->getStateCacheManager();
    }

    GLES2HardwareBufferManagerBase::~GLES2HardwareBufferManagerBase()
    {
        destroyAllDeclarations();
        destroyAllBindings();
    }

    HardwareVertexBufferSharedPtr
        GLES2HardwareBufferManagerBase::createVertexBuffer(size_t vertexSize,
                                                      size_t numVerts,
                                                      HardwareBuffer::Usage usage,
                                                      bool useShadowBuffer)
    {
        GLES2HardwareVertexBuffer* buf = 0;

        if(getGLES2SupportRef()->checkExtension("GL_EXT_map_buffer_range") || gleswIsSupported(3, 0))
            buf = OGRE_NEW GLES2HardwareVertexBuffer(this, vertexSize, numVerts, usage, useShadowBuffer);
        else
            // always use shadowBuffer
            buf = OGRE_NEW GLES2HardwareVertexBuffer(this, vertexSize, numVerts, usage, true);

        {
            OGRE_LOCK_MUTEX(mVertexBuffersMutex);
            mVertexBuffers.insert(buf);
        }
        return HardwareVertexBufferSharedPtr(buf);
    }

    HardwareIndexBufferSharedPtr GLES2HardwareBufferManagerBase::createIndexBuffer(HardwareIndexBuffer::IndexType itype,
                                                                              size_t numIndexes,
                                                                              HardwareBuffer::Usage usage,
                                                                              bool useShadowBuffer)
    {
        GLES2HardwareIndexBuffer* buf = 0;
        if(getGLES2SupportRef()->checkExtension("GL_EXT_map_buffer_range") || gleswIsSupported(3, 0))
            buf = OGRE_NEW GLES2HardwareIndexBuffer(this, itype, numIndexes, usage, useShadowBuffer);
        else
            // always use shadowBuffer
            buf = OGRE_NEW GLES2HardwareIndexBuffer(this, itype, numIndexes, usage, true);

        {
            OGRE_LOCK_MUTEX(mIndexBuffersMutex);
            mIndexBuffers.insert(buf);
        }
        return HardwareIndexBufferSharedPtr(buf);
    }

    RenderToVertexBufferSharedPtr GLES2HardwareBufferManagerBase::createRenderToVertexBuffer()
    {
#if OGRE_NO_GLES3_SUPPORT == 0
        return RenderToVertexBufferSharedPtr(new GLES2RenderToVertexBuffer());
#else
        // not supported
        return RenderToVertexBufferSharedPtr();
#endif
    }

    VertexDeclaration* GLES2HardwareBufferManagerBase::createVertexDeclarationImpl(void)
    {
        return OGRE_NEW GLES2VertexDeclaration();
    }

    void GLES2HardwareBufferManagerBase::destroyVertexDeclarationImpl(VertexDeclaration* decl)
    {
        if(decl)
            OGRE_DELETE decl;
    }

    GLenum GLES2HardwareBufferManagerBase::getGLUsage(unsigned int usage)
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

    GLenum GLES2HardwareBufferManagerBase::getGLType(unsigned int type)
    {
        switch(type)
        {
            case VET_FLOAT1:
            case VET_FLOAT2:
            case VET_FLOAT3:
            case VET_FLOAT4:
                return GL_FLOAT;
            case VET_SHORT2:
            case VET_SHORT4:
            case VET_SHORT2_SNORM:
            case VET_SHORT4_SNORM:
                return GL_SHORT;
            case VET_USHORT2:
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
#if OGRE_NO_GLES3_SUPPORT == 0
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
#endif
            default:
                return 0;
        };
    }

    //---------------------------------------------------------------------
    HardwareUniformBufferSharedPtr GLES2HardwareBufferManagerBase::createUniformBuffer( size_t sizeBytes, HardwareBuffer::Usage usage, bool useShadowBuffer, const String& name )
    {
#if OGRE_NO_GLES3_SUPPORT == 0
        GLES2HardwareUniformBuffer* buf =
        new GLES2HardwareUniformBuffer(this, sizeBytes, usage, useShadowBuffer, name);
        {
            OGRE_LOCK_MUTEX(mUniformBuffersMutex);
            mUniformBuffers.insert(buf);
        }
        return HardwareUniformBufferSharedPtr(buf);
#else
        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
            "GLES2 does not support uniform buffer objects", 
            "GLES2HardwareBufferManagerBase::createUniformBuffer");
#endif
    }
    //---------------------------------------------------------------------
    HardwareCounterBufferSharedPtr GLES2HardwareBufferManagerBase::createCounterBuffer( size_t sizeBytes, HardwareBuffer::Usage usage, bool useShadowBuffer, const String& name )
    {
        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                    "GLES2 does not support atomic counter buffers",
                    "GLES2HardwareBufferManagerBase::createCounterBuffer");
    }

}
}
