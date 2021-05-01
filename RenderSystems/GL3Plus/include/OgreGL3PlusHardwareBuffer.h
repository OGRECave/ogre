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
#ifndef __GL3PlusHARDWAREBUFFER_H__
#define __GL3PlusHARDWAREBUFFER_H__

#include "OgreGL3PlusPrerequisites.h"
#include "OgreHardwareBuffer.h"

namespace Ogre {
    class GL3PlusHardwareBuffer : public HardwareBuffer
    {
    private:
        GLenum mTarget;
        GLuint mBufferId;
        GL3PlusRenderSystem* mRenderSystem;

        // for UBO/ SSBO
        GLint mBindingPoint;

        /// Utility function to get the correct GL usage based on HBU's
        static GLenum getGLUsage(uint32 usage);

        void writeDataImpl(size_t offset, size_t length, const void* pSource, bool discardWholeBuffer);
    public:
        void* lockImpl(size_t offset, size_t length, LockOptions options) override;
        void unlockImpl() override;

        GL3PlusHardwareBuffer(GLenum target, size_t sizeInBytes, uint32 usage, bool useShadowBuffer = false);
        ~GL3PlusHardwareBuffer();

        void readData(size_t offset, size_t length, void* pDest) override;

        void writeData(size_t offset, size_t length, const void* pSource, bool discardWholeBuffer) override;

        void copyData(HardwareBuffer& srcBuffer, size_t srcOffset, size_t dstOffset, size_t length,
                      bool discardWholeBuffer) override;

        void _updateFromShadow() override;

        GLuint getGLBufferId(void) const { return mBufferId; }

        GLenum getTarget() const { return mTarget; }

        void setGLBufferBinding(GLint binding);
        GLint getGLBufferBinding(void) const { return mBindingPoint; }
        void bind() { setGLBufferBinding(mBindingPoint); }
    };
}
#endif // __GL3PlusHARDWAREBUFFER_H__
