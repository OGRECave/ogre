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

namespace Ogre {
    GL3PlusHardwareVertexBuffer::GL3PlusHardwareVertexBuffer(HardwareBufferManagerBase* mgr, 
													   size_t vertexSize,
                                                       size_t numVertices,
                                                       HardwareBuffer::Usage usage,
                                                       bool useShadowBuffer)
        : HardwareVertexBuffer(mgr, vertexSize, numVertices, usage, false, false)
    {
        OGRE_CHECK_GL_ERROR(glGenBuffers(1, &mBufferId));

        if (!mBufferId)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Cannot create GL vertex buffer",
                        "GL3PlusHardwareVertexBuffer::GL3PlusHardwareVertexBuffer");
        }

        OGRE_CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, mBufferId));
        OGRE_CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, mSizeInBytes, NULL,
                                         GL3PlusHardwareBufferManager::getGLUsage(usage)));
//        std::cerr << "creating vertex buffer = " << mBufferId << std::endl;
    }

    GL3PlusHardwareVertexBuffer::~GL3PlusHardwareVertexBuffer()
    {
        OGRE_CHECK_GL_ERROR(glDeleteBuffers(1, &mBufferId));
    }

    void* GL3PlusHardwareVertexBuffer::lockImpl(size_t offset,
                                           size_t length,
                                           LockOptions options)
    {
        if (mIsLocked)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Invalid attempt to lock a vertex buffer that has already been locked",
                        "GL3PlusHardwareVertexBuffer::lock");
        }

        GLenum access = 0;
        void* retPtr = 0;

        if (!retPtr)
		{
			// Use glMapBuffer
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, mBufferId));

			if (mUsage & HBU_WRITE_ONLY)
            {
				access |= GL_MAP_WRITE_BIT;
                access |= GL_MAP_FLUSH_EXPLICIT_BIT;
                if(options == HBL_DISCARD || options == HBL_NO_OVERWRITE)
                {
                    // Discard the buffer
                    access |= GL_MAP_INVALIDATE_RANGE_BIT;
                }
                access |= GL_MAP_UNSYNCHRONIZED_BIT;
            }
			else if (options == HBL_READ_ONLY)
				access |= GL_MAP_READ_BIT;
			else
				access |= GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;

            // FIXME: Big stall here
            void* pBuffer;
            OGRE_CHECK_GL_ERROR(pBuffer = glMapBufferRange(GL_ARRAY_BUFFER, offset, length, access));

			if(pBuffer == 0)
			{
				OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
					"Vertex Buffer: Out of memory",
                    "GL3PlusHardwareVertexBuffer::lock");
			}

			// return offsetted
			retPtr = static_cast<void*>(static_cast<unsigned char*>(pBuffer) + offset);

			mLockedToScratch = false;
		}
		mIsLocked = true;
        return retPtr;
    }

    void GL3PlusHardwareVertexBuffer::unlockImpl(void)
    {
        if (mLockedToScratch)
        {
            if (mScratchUploadOnUnlock)
            {
                // have to write the data back to vertex buffer
                writeData(mScratchOffset, mScratchSize, mScratchPtr,
                          mScratchOffset == 0 && mScratchSize == getSizeInBytes());
            }

			// deallocate from scratch buffer
            static_cast<GL3PlusHardwareBufferManager*>(
                HardwareBufferManager::getSingletonPtr())->deallocateScratch(mScratchPtr);

            mLockedToScratch = false;
        }
        else
        {
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, mBufferId));

			if (mUsage & HBU_WRITE_ONLY)
            {
                OGRE_CHECK_GL_ERROR(glFlushMappedBufferRange(GL_ARRAY_BUFFER, mLockStart, mLockSize));
            }

            GLboolean mapped;
            OGRE_CHECK_GL_ERROR(mapped = glUnmapBuffer(GL_ARRAY_BUFFER));
			if(!mapped)
			{
				OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
					"Buffer data corrupted, please reload", 
					"GL3PlusHardwareVertexBuffer::unlock");
			}
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
        }

        mIsLocked = false;
    }

    void GL3PlusHardwareVertexBuffer::readData(size_t offset, size_t length, void* pDest)
    {
        if (mUseShadowBuffer)
        {
            void* srcData = mShadowBuffer->lock(offset, length, HBL_READ_ONLY);
            memcpy(pDest, srcData, length);
            mShadowBuffer->unlock();
        }
        else
        {
            // get data from the real buffer
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, mBufferId));

            OGRE_CHECK_GL_ERROR(glGetBufferSubData(GL_ARRAY_BUFFER, offset, length, pDest));
        }
    }

    void GL3PlusHardwareVertexBuffer::writeData(size_t offset,
                                           size_t length,
                                           const void* pSource,
                                           bool discardWholeBuffer)
    {
        OGRE_CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, mBufferId));

        // Update the shadow buffer
        if(mUseShadowBuffer)
        {
            void* destData = mShadowBuffer->lock(offset, length,
                                                  discardWholeBuffer ? HBL_DISCARD : HBL_NORMAL);
            memcpy(destData, pSource, length);
            mShadowBuffer->unlock();
        }

        if (offset == 0 && length == mSizeInBytes)
        {
            OGRE_CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, mSizeInBytes, pSource,
                                             GL3PlusHardwareBufferManager::getGLUsage(mUsage)));
        }
        else
        {
            if(discardWholeBuffer)
            {
                OGRE_CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, mSizeInBytes, NULL, 
                                                 GL3PlusHardwareBufferManager::getGLUsage(mUsage)));
            }

            OGRE_CHECK_GL_ERROR(glBufferSubData(GL_ARRAY_BUFFER, offset, length, pSource));
        }
    }

    void GL3PlusHardwareVertexBuffer::copyData(HardwareBuffer& srcBuffer, size_t srcOffset, 
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
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

            // Zero out this(destination) buffer
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, mBufferId));
            OGRE_CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, length, 0, GL3PlusHardwareBufferManager::getGLUsage(mUsage)));
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

            // Do it the fast way.
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_COPY_READ_BUFFER, static_cast<GL3PlusHardwareVertexBuffer &>(srcBuffer).getGLBufferId()));
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_COPY_WRITE_BUFFER, mBufferId));

            OGRE_CHECK_GL_ERROR(glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, srcOffset, dstOffset, length));

            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_COPY_READ_BUFFER, 0));
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_COPY_WRITE_BUFFER, 0));
        }
    }

    void GL3PlusHardwareVertexBuffer::_updateFromShadow(void)
    {
        if (mUseShadowBuffer && mShadowUpdated && !mSuppressHardwareUpdate)
        {
            const void *srcData = mShadowBuffer->lock(mLockStart,
                                                       mLockSize,
                                                       HBL_READ_ONLY);

            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, mBufferId));

            // Update whole buffer if possible, otherwise normal
            if (mLockStart == 0 && mLockSize == mSizeInBytes)
            {
                OGRE_CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, mSizeInBytes, srcData,
                                                 GL3PlusHardwareBufferManager::getGLUsage(mUsage)));
            }
            else
            {
                OGRE_CHECK_GL_ERROR(glBufferSubData(GL_ARRAY_BUFFER, mLockStart, mLockSize, srcData));
            }

            mShadowBuffer->unlock();
            mShadowUpdated = false;
        }
    }
}
