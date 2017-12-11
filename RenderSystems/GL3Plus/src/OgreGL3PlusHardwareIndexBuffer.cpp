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

#include "OgreGL3PlusHardwareIndexBuffer.h"
#include "OgreGL3PlusHardwareBufferManager.h"
#include "OgreGL3PlusRenderSystem.h"
#include "OgreRoot.h"

namespace Ogre {
namespace v1 {
    GL3PlusHardwareIndexBuffer::GL3PlusHardwareIndexBuffer(
        HardwareBufferManagerBase* mgr,
        IndexType idxType,
        size_t numIndexes,
        HardwareBuffer::Usage usage,
        bool useShadowBuffer)
    : HardwareIndexBuffer(mgr, idxType, numIndexes, usage, false, useShadowBuffer), mLockedToScratch(false),
        mScratchOffset(0), mScratchSize(0), mScratchPtr(0), mScratchUploadOnUnlock(false)
    {
        OGRE_CHECK_GL_ERROR(glGenBuffers(1, &mBufferId));

        if (!mBufferId)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Cannot create GL index buffer",
                        "GL3PlusHardwareIndexBuffer::GL3PlusHardwareIndexBuffer");
        }

        OGRE_CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferId));

        OGRE_CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER, mSizeInBytes, NULL,
                                         GL3PlusHardwareBufferManager::getGLUsage(usage)));
        //        std::cerr << "creating index buffer " << mBufferId << std::endl;
    }

    GL3PlusHardwareIndexBuffer::~GL3PlusHardwareIndexBuffer()
    {
        OGRE_CHECK_GL_ERROR(glDeleteBuffers(1, &mBufferId));
    }

    void* GL3PlusHardwareIndexBuffer::lockImpl(size_t offset,
                                               size_t length,
                                               LockOptions options)
    {
        if(mIsLocked)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Invalid attempt to lock an index buffer that has already been locked",
                        "GL3PlusHardwareIndexBuffer::lock");
        }

        void* retPtr = 0;
        GLenum access = 0;

        OGRE_CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferId));

        assert( ((mUsage & HBU_WRITE_ONLY && options != HBL_NORMAL && options != HBL_READ_ONLY) ||
                !(mUsage & HBU_WRITE_ONLY)) &&
                "Reading from a write-only buffer! Create the buffer without HBL_WRITE_ONLY bit" );

        // Use glMapBuffer
        if (mUsage & HBU_WRITE_ONLY)
        {
            access |= GL_MAP_WRITE_BIT;
            access |= GL_MAP_FLUSH_EXPLICIT_BIT;
            if (options == HBL_DISCARD || options == HBL_NO_OVERWRITE)
            {
                // Discard the buffer
                access |= GL_MAP_INVALIDATE_RANGE_BIT;
            }

            if( options == HBL_NO_OVERWRITE )
                access |= GL_MAP_UNSYNCHRONIZED_BIT;
        }
        else if (options == HBL_READ_ONLY)
            access |= GL_MAP_READ_BIT;
        else
            access |= GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;


        void* pBuffer = 0;
        OGRE_CHECK_GL_ERROR(pBuffer = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, offset, length, access));

        if(pBuffer == 0)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Index Buffer: Out of memory",
                        "GL3PlusHardwareIndexBuffer::lock");
        }

        // pBuffer is already offsetted in glMapBufferRange
        retPtr = pBuffer;

        mLockedToScratch = false;


        mIsLocked = true;
        return retPtr;
    }

    void GL3PlusHardwareIndexBuffer::unlockImpl(void)
    {
        if (mLockedToScratch)
        {
            if (mScratchUploadOnUnlock)
            {
                // have to write the data back to vertex buffer
                writeData(mScratchOffset, mScratchSize, mScratchPtr,
                          mScratchOffset == 0 && mScratchSize == getSizeInBytes());
            }

            static_cast<GL3PlusHardwareBufferManager*>(
                HardwareBufferManager::getSingletonPtr())->deallocateScratch(mScratchPtr);

            mLockedToScratch = false;
        }
        else
        {
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferId));

            if (mUsage & HBU_WRITE_ONLY)
            {
                OGRE_CHECK_GL_ERROR(glFlushMappedBufferRange(GL_ELEMENT_ARRAY_BUFFER, mLockStart, mLockSize));
            }

            GLboolean mapped;
            OGRE_CHECK_GL_ERROR(mapped = glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER));
            if(!mapped)
            {
                OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                            "Buffer data corrupted, please reload",
                            "GL3PlusHardwareIndexBuffer::unlock");
            }
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
        }
        mIsLocked = false;
    }

    void GL3PlusHardwareIndexBuffer::readData(size_t offset,
                                              size_t length,
                                              void* pDest)
    {
        if(mUseShadowBuffer)
        {
            // get data from the shadow buffer
            void* srcData = mShadowBuffer->lock(offset, length, HBL_READ_ONLY);
            memcpy(pDest, srcData, length);
            mShadowBuffer->unlock();
        }
        else
        {
            OGRE_CHECK_GL_ERROR(glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mBufferId ));
            OGRE_CHECK_GL_ERROR(glGetBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, length, pDest));
        }
    }

    void GL3PlusHardwareIndexBuffer::writeData(size_t offset, size_t length,
                                               const void* pSource,
                                               bool discardWholeBuffer)
    {
        OGRE_CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferId));

        // Update the shadow buffer
        if (mUseShadowBuffer)
        {
            void* destData = mShadowBuffer->lock(offset, length,
                                                 discardWholeBuffer ? HBL_DISCARD : HBL_NORMAL);
            memcpy(destData, pSource, length);
            mShadowBuffer->unlock();
        }

        if (offset == 0 && length == mSizeInBytes)
        {
            OGRE_CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER, mSizeInBytes, pSource,
                                             GL3PlusHardwareBufferManager::getGLUsage(mUsage)));
        }
        else
        {
            if (discardWholeBuffer)
            {
                OGRE_CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER, mSizeInBytes, NULL,
                                                 GL3PlusHardwareBufferManager::getGLUsage(mUsage)));
            }

            // Now update the real buffer
            OGRE_CHECK_GL_ERROR(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, length, pSource));
        }
    }

    void GL3PlusHardwareIndexBuffer::copyData(HardwareBuffer& srcBuffer, size_t srcOffset,
                                              size_t dstOffset, size_t length, bool discardWholeBuffer)
    {
        // If the buffer is not in system memory we can use ARB_copy_buffers to do an optimised copy.
        if (srcBuffer.isSystemMemory())
        {
            HardwareBuffer::copyData(srcBuffer, srcOffset, dstOffset, length, discardWholeBuffer);
        }
        else
        {
            // Unbind the current buffer
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

            // Zero out this(destination) buffer
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferId));
            OGRE_CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER, length, 0, GL3PlusHardwareBufferManager::getGLUsage(mUsage)));
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

            // Do it the fast way.
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_COPY_READ_BUFFER, static_cast<GL3PlusHardwareIndexBuffer &>(srcBuffer).getGLBufferId()));
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_COPY_WRITE_BUFFER, mBufferId));

            OGRE_CHECK_GL_ERROR(glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, srcOffset, dstOffset, length));

            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_COPY_READ_BUFFER, 0));
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_COPY_WRITE_BUFFER, 0));
        }
    }

    void GL3PlusHardwareIndexBuffer::_updateFromShadow(void)
    {
        if (mUseShadowBuffer && mShadowUpdated && !mSuppressHardwareUpdate)
        {
            const void *srcData = mShadowBuffer->lock(mLockStart, mLockSize,
                                                      HBL_READ_ONLY);

            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferId));

            // Update whole buffer if possible, otherwise normal
            if (mLockStart == 0 && mLockSize == mSizeInBytes)
            {
                OGRE_CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER, mSizeInBytes, srcData,
                                                 GL3PlusHardwareBufferManager::getGLUsage(mUsage)));
            }
            else
            {
                OGRE_CHECK_GL_ERROR(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,
                                                    mLockStart, mLockSize, srcData));
            }

            mShadowBuffer->unlock();
            mShadowUpdated = false;
        }
    }

}
}
