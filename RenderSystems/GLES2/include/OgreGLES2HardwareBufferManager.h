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

#ifndef __GLES2HardwareBufferManager_H__
#define __GLES2HardwareBufferManager_H__

#include "OgreGLES2Prerequisites.h"
#include "OgreHardwareBufferManager.h"
#include "OgreGLES2RenderSystem.h"

namespace Ogre {
    class GLES2StateCacheManager;

    /** Implementation of HardwareBufferManager for OpenGL ES. */
    class _OgreGLES2Export GLES2HardwareBufferManager : public HardwareBufferManager
    {
        protected:
            GLES2RenderSystem* mRenderSystem;
            /// Internal method for creates a new vertex declaration, may be overridden by certain rendering APIs
            VertexDeclaration* createVertexDeclarationImpl(void);
        public:
            GLES2HardwareBufferManager();
            virtual ~GLES2HardwareBufferManager();
            /// Creates a vertex buffer
            HardwareVertexBufferSharedPtr createVertexBuffer(size_t vertexSize,
                size_t numVerts, HardwareBuffer::Usage usage, bool useShadowBuffer = false);
            /// Create a hardware vertex buffer
            HardwareIndexBufferSharedPtr createIndexBuffer(
                HardwareIndexBuffer::IndexType itype, size_t numIndexes,
                HardwareBuffer::Usage usage, bool useShadowBuffer = false);
            /// Create a render to vertex buffer
            RenderToVertexBufferSharedPtr createRenderToVertexBuffer();
            HardwareUniformBufferSharedPtr
            createUniformBuffer(size_t sizeBytes, HardwareBuffer::Usage usage, bool useShadowBuffer, const String& name = "");
            /// Create a uniform buffer
            HardwareUniformBufferSharedPtr createUniformBuffer(size_t sizeBytes, HardwareBuffer::Usage usage,
                                                               bool useShadowBuffer, size_t binding, const String& name = "");
            /// Utility function to get the correct GL type based on VET's
            static GLenum getGLType(VertexElementType type);

            GLES2StateCacheManager * getStateCacheManager();
            void notifyContextDestroyed(GLContext* context);
    };
}

#endif
