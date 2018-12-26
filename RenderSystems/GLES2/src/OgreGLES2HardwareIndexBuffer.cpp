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

#include "OgreGLES2HardwareIndexBuffer.h"
#include "OgreGLES2HardwareBufferManager.h"
#include "OgreGLES2RenderSystem.h"
#include "OgreRoot.h"
#include "OgreGLUtil.h"
#include "OgreGLES2StateCacheManager.h"
#include "OgreGLNativeSupport.h"

namespace Ogre {
    GLES2HardwareIndexBuffer::GLES2HardwareIndexBuffer(HardwareBufferManagerBase* mgr, 
                                                     IndexType idxType,
                                                     size_t numIndexes,
                                                     HardwareBuffer::Usage usage,
                                                     bool useShadowBuffer)
        : HardwareIndexBuffer(mgr, idxType, numIndexes, usage, false, useShadowBuffer || HANDLE_CONTEXT_LOSS),
          mBuffer(GL_ELEMENT_ARRAY_BUFFER, mSizeInBytes, usage)
    {
        if (!Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_32BIT_INDEX) &&
            idxType == HardwareIndexBuffer::IT_32BIT)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                "32 bit hardware buffers are not supported.",
                "GLES2HardwareIndexBuffer");
        }
    }
    
#if HANDLE_CONTEXT_LOSS
    void GLES2HardwareIndexBuffer::notifyOnContextLost()
    {
        mBuffer.destroyBuffer();
    }
    
    void GLES2HardwareIndexBuffer::notifyOnContextReset()
    {
        mBuffer.createBuffer();
        mShadowUpdated = true;
        _updateFromShadow();
    }
#endif

    void GLES2HardwareIndexBuffer::readData(size_t offset,
                                           size_t length,
                                           void* pDest)
    {
        if(mUseShadowBuffer)
        {
            mShadowBuffer->readData(offset, length, pDest);
        }
        else
        {
            mBuffer.readData(offset, length, pDest);
        }
    }

    void GLES2HardwareIndexBuffer::writeData(size_t offset, size_t length,
                                            const void* pSource,
                                            bool discardWholeBuffer)
    {
        // Update the shadow buffer
        if (mUseShadowBuffer)
        {
            mShadowBuffer->writeData(offset, length, pSource, discardWholeBuffer);
        }

        mBuffer.writeData(offset, length, pSource, discardWholeBuffer);
    }

    void GLES2HardwareIndexBuffer::copyData(HardwareBuffer& srcBuffer, size_t srcOffset,
                                              size_t dstOffset, size_t length, bool discardWholeBuffer)
    {
        // If the buffer is not in system memory we can use ARB_copy_buffers to do an optimised copy.
        if (OGRE_NO_GLES3_SUPPORT || srcBuffer.isSystemMemory())
        {
            HardwareBuffer::copyData(srcBuffer, srcOffset, dstOffset, length, discardWholeBuffer);
        }
        else
        {
            if(mUseShadowBuffer) {
                mShadowBuffer->copyData(srcBuffer, srcOffset, dstOffset, length, discardWholeBuffer);
            }

            mBuffer.copyData(static_cast<GLES2HardwareIndexBuffer&>(srcBuffer).getGLBufferId(),
                    srcOffset, dstOffset, length, discardWholeBuffer);
        }
    }

    void GLES2HardwareIndexBuffer::_updateFromShadow(void)
    {
        if (mUseShadowBuffer && mShadowUpdated && !mSuppressHardwareUpdate)
        {
            HardwareBufferLockGuard shadowLock(mShadowBuffer.get(), mLockStart, mLockSize, HBL_READ_ONLY);
            mBuffer.writeData(mLockStart, mLockSize, shadowLock.pData, false);

            mShadowUpdated = false;
        }
    }
}
