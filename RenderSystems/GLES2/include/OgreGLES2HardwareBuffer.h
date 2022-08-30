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

#ifndef __GLES2HardwareBuffer_H__
#define __GLES2HardwareBuffer_H__

#include "OgreGLES2Prerequisites.h"
#include "OgreHardwareBuffer.h"
#include "OgreGLES2ManagedResource.h"

namespace Ogre {
    class GLES2RenderSystem;
    class GLES2HardwareBuffer : public HardwareBuffer MANAGED_RESOURCE
    {
        private:
            GLenum mTarget;
            GLuint mBufferId;
            GLES2RenderSystem* mRenderSystem;

            // for UBO/ SSBO
            GLint mBindingPoint;

            /// Utility function to get the correct GL usage based on HBU's
            static GLenum getGLUsage(uint32 usage);

            void writeDataImpl(size_t offset, size_t length, const void* pSource, bool discardWholeBuffer);
        public:
            void createBuffer();

            void destroyBuffer();

            void* lockImpl(size_t offset, size_t length, HardwareBuffer::LockOptions options) override;

            void unlockImpl() override;

            GLES2HardwareBuffer(GLenum target, size_t sizeInBytes, GLenum usage, bool useShadowBuffer);
            ~GLES2HardwareBuffer();

            void readData(size_t offset, size_t length, void* pDest) override;

            void writeData(size_t offset, size_t length, const void* pSource,
                           bool discardWholeBuffer = false) override;

            void copyData(HardwareBuffer& srcBuffer, size_t srcOffset, size_t dstOffset, size_t length,
                          bool discardWholeBuffer) override;

            void _updateFromShadow() override;

            GLuint getGLBufferId(void) const { return mBufferId; }

            void setGLBufferBinding(GLint binding);
            GLint getGLBufferBinding(void) const { return mBindingPoint; }

#if HANDLE_CONTEXT_LOSS
            void notifyOnContextLost() override;
            void notifyOnContextReset() override;
#endif
    };
}

#endif
