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
#include "OgreGL3PlusHardwareVertexBuffer.h"
#include "OgreRoot.h"
#include "OgreGL3PlusRenderSystem.h"
#include "OgreGL3PlusStateCacheManager.h"

namespace Ogre {

    GL3PlusHardwareVertexBuffer::GL3PlusHardwareVertexBuffer(
        HardwareBufferManagerBase* mgr,
        size_t vertexSize,
        size_t numVertices,
        HardwareBuffer::Usage usage,
        bool useShadowBuffer)
    : HardwareVertexBuffer(mgr, vertexSize, numVertices, usage, false, useShadowBuffer),
      mBuffer(GL_ARRAY_BUFFER, mSizeInBytes, usage)
    {
    }

    void GL3PlusHardwareVertexBuffer::readData(size_t offset, size_t length, void* pDest)
    {
        if (mUseShadowBuffer)
        {
            mShadowBuffer->readData(offset, length, pDest);
        }
        else
        {
            mBuffer.readData(offset, length, pDest);
        }
    }

    void GL3PlusHardwareVertexBuffer::writeData(size_t offset, size_t length, const void* pSource,
                                                bool discardWholeBuffer)
    {
        // Update the shadow buffer
        if(mUseShadowBuffer)
        {
            mShadowBuffer->writeData(offset, length, pSource, discardWholeBuffer);
        }

        mBuffer.writeData(offset, length, pSource, discardWholeBuffer);
    }

    void GL3PlusHardwareVertexBuffer::copyData(HardwareBuffer& srcBuffer, size_t srcOffset,
                                               size_t dstOffset, size_t length,
                                               bool discardWholeBuffer)
    {
        // If the buffer is not in system memory we can use ARB_copy_buffers to do an optimised copy.
        if (srcBuffer.isSystemMemory())
        {
            HardwareBuffer::copyData(srcBuffer, srcOffset, dstOffset, length, discardWholeBuffer);
        }
        else
        {
            if(mUseShadowBuffer) {
                mShadowBuffer->copyData(srcBuffer, srcOffset, dstOffset, length, discardWholeBuffer);
            }

            mBuffer.copyData(static_cast<GL3PlusHardwareVertexBuffer&>(srcBuffer).getGLBufferId(),
                             srcOffset, dstOffset, length, discardWholeBuffer);
        }
    }

    void GL3PlusHardwareVertexBuffer::_updateFromShadow(void)
    {
        if (mUseShadowBuffer && mShadowUpdated && !mSuppressHardwareUpdate)
        {
            HardwareBufferLockGuard shadowLock(mShadowBuffer.get(), mLockStart, mLockSize, HBL_READ_ONLY);
            mBuffer.writeData(mLockStart, mLockSize, shadowLock.pData, false);

            mShadowUpdated = false;
        }
    }


}
