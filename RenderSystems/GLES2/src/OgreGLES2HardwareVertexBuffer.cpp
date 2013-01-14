/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

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
#include "OgreRoot.h"
#include "OgreGLES2RenderSystem.h"
#include "OgreGLES2StateCacheManager.h"

namespace Ogre {
    GLES2HardwareVertexBuffer::GLES2HardwareVertexBuffer(HardwareBufferManagerBase* mgr, 
													   size_t vertexSize,
                                                       size_t numVertices,
                                                       HardwareBuffer::Usage usage,
                                                       bool useShadowBuffer)
        : HardwareVertexBuffer(mgr, vertexSize, numVertices, usage, false, true)
    {
        if (!useShadowBuffer)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Only supported with shadowBuffer",
                        "GLES2HardwareVertexBuffer");
        }

        createBuffer();
    }

    GLES2HardwareVertexBuffer::~GLES2HardwareVertexBuffer()
    {
        destroyBuffer();
    }

    void GLES2HardwareVertexBuffer::createBuffer()
    {
        OGRE_CHECK_GL_ERROR(glGenBuffers(1, &mBufferId));
        
        if (!mBufferId)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Cannot create GL ES vertex buffer",
                        "GLES2HardwareVertexBuffer::GLES2HardwareVertexBuffer");
        }
        
		static_cast<GLES2HardwareBufferManagerBase*>(mMgr)->getStateCacheManager()->bindGLBuffer(GL_ARRAY_BUFFER, mBufferId);
        OGRE_CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, mSizeInBytes, NULL,
                                         GLES2HardwareBufferManager::getGLUsage(mUsage)));
    }
    
    void GLES2HardwareVertexBuffer::destroyBuffer()
    {
        // Delete the cached value
        static_cast<GLES2HardwareBufferManagerBase*>(mMgr)->getStateCacheManager()->deleteGLBuffer(GL_ARRAY_BUFFER, mBufferId);
    }
    
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
    void GLES2HardwareVertexBuffer::notifyOnContextLost()
    {
        destroyBuffer();
    }
    
    void GLES2HardwareVertexBuffer::notifyOnContextReset()
    {
        createBuffer();
        mShadowUpdated = true;
        _updateFromShadow();
    }
#endif
    
    void* GLES2HardwareVertexBuffer::lockImpl(size_t offset,
                                           size_t length,
                                           LockOptions options)
    {
        if (mIsLocked)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Invalid attempt to lock an index buffer that has already been locked",
                        "GLES2HardwareVertexBuffer::lock");
        }

        void* retPtr = 0;

		GLES2HardwareBufferManager* glBufManager = static_cast<GLES2HardwareBufferManager*>(HardwareBufferManager::getSingletonPtr());

		// Try to use scratch buffers for smaller buffers
        if (length < glBufManager->getGLMapBufferThreshold())
        {
			// if this fails, we fall back on mapping
            retPtr = glBufManager->allocateScratch((uint32)length);

            if (retPtr)
            {
                mLockedToScratch = true;
                mScratchOffset = offset;
                mScratchSize = length;
                mScratchPtr = retPtr;
                mScratchUploadOnUnlock = (options != HBL_READ_ONLY);

                if (options != HBL_DISCARD)
                {
					// have to read back the data before returning the pointer
                    readData(offset, length, retPtr);
                }
            }
        }
        else
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Invalid Buffer lockSize",
                        "GLES2HardwareVertexBuffer::lock");
        }

#if GL_OES_mapbuffer || (OGRE_NO_GLES3_SUPPORT == 0)
        if (!retPtr)
		{
            GLenum access = 0;
			// Use glMapBuffer
			static_cast<GLES2HardwareBufferManagerBase*>(mMgr)->getStateCacheManager()->bindGLBuffer(GL_ARRAY_BUFFER, mBufferId);

            if(options == HBL_DISCARD)
			{
				// Discard the buffer
				OGRE_CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, mSizeInBytes, NULL, 
                                                 GLES2HardwareBufferManager::getGLUsage(mUsage)));
			}
			if (mUsage & HBU_WRITE_ONLY)
				access = GL_WRITE_ONLY_OES;

			void* pBuffer;
#if OGRE_NO_GLES3_SUPPORT == 0
            OGRE_CHECK_GL_ERROR(pBuffer = glMapBufferRange(GL_ARRAY_BUFFER, offset, length, access));
#else
            OGRE_CHECK_GL_ERROR(pBuffer = glMapBufferOES(GL_ARRAY_BUFFER, access));
#endif

			if(pBuffer == 0)
			{
				OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
					"Vertex Buffer: Out of memory", "GLES2HardwareVertexBuffer::lock");
			}

			// return offsetted
			retPtr = static_cast<void*>(
				static_cast<unsigned char*>(pBuffer) + offset);

			mLockedToScratch = false;
		}
#endif
		mIsLocked = true;
        return retPtr;
    }

    void GLES2HardwareVertexBuffer::unlockImpl(void)
    {
        if (mLockedToScratch)
        {
            if (mScratchUploadOnUnlock)
            {
                // have to write the data back to vertex buffer
                writeData(mScratchOffset, mScratchSize, mScratchPtr,
                          mScratchOffset == 0 && mScratchSize == getSizeInBytes());
            }

            static_cast<GLES2HardwareBufferManager*>(
                HardwareBufferManager::getSingletonPtr())->deallocateScratch(mScratchPtr);

            mLockedToScratch = false;
        }
        else
        {
#if GL_OES_mapbuffer || (OGRE_NO_GLES3_SUPPORT == 0)
			static_cast<GLES2HardwareBufferManagerBase*>(mMgr)->getStateCacheManager()->bindGLBuffer(GL_ARRAY_BUFFER, mBufferId);

#if OGRE_NO_GLES3_SUPPORT == 0
            if (mUsage & HBU_WRITE_ONLY)
            {
                OGRE_CHECK_GL_ERROR(glFlushMappedBufferRange(GL_ARRAY_BUFFER, mLockStart, mLockSize));
            }
#endif
            GLboolean mapped;
            OGRE_CHECK_GL_ERROR(mapped = glUnmapBufferOES(GL_ARRAY_BUFFER));
			if(!mapped)
			{
				OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
					"Buffer data corrupted, please reload", 
					"GLES2HardwareVertexBuffer::unlock");
			}
#else
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Only locking to scratch is supported",
                        "GLES2HardwareVertexBuffer::unlockImpl");
#endif
        }

        mIsLocked = false;
    }

    void GLES2HardwareVertexBuffer::readData(size_t offset, size_t length, void* pDest)
    {
        if (mUseShadowBuffer)
        {
            void* srcData = mShadowBuffer->lock(offset, length, HBL_READ_ONLY);
            memcpy(pDest, srcData, length);
            mShadowBuffer->unlock();
        }
        else
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Read hardware buffer is not supported",
                        "GLES2HardwareVertexBuffer::readData");
        }
    }

    void GLES2HardwareVertexBuffer::writeData(size_t offset,
                                           size_t length,
                                           const void* pSource,
                                           bool discardWholeBuffer)
    {
		static_cast<GLES2HardwareBufferManagerBase*>(mMgr)->getStateCacheManager()->bindGLBuffer(GL_ARRAY_BUFFER, mBufferId);

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
                                             GLES2HardwareBufferManager::getGLUsage(mUsage)));
        }
        else
        {
            if(discardWholeBuffer)
            {
                OGRE_CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, mSizeInBytes, NULL,
                                                 GLES2HardwareBufferManager::getGLUsage(mUsage)));
            }

            OGRE_CHECK_GL_ERROR(glBufferSubData(GL_ARRAY_BUFFER, offset, length, pSource));
        }
    }

#if OGRE_NO_GLES3_SUPPORT == 0
    void GLES2HardwareVertexBuffer::copyData(HardwareBuffer& srcBuffer, size_t srcOffset,
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
            OGRE_CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, length, 0, GLES2HardwareBufferManager::getGLUsage(mUsage)));
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

            // Do it the fast way.
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_COPY_READ_BUFFER, static_cast<GLES2HardwareVertexBuffer &>(srcBuffer).getGLBufferId()));
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_COPY_WRITE_BUFFER, mBufferId));

            OGRE_CHECK_GL_ERROR(glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, srcOffset, dstOffset, length));

            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_COPY_READ_BUFFER, 0));
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_COPY_WRITE_BUFFER, 0));
        }
    }
#endif
    void GLES2HardwareVertexBuffer::_updateFromShadow(void)
    {
        if (mUseShadowBuffer && mShadowUpdated && !mSuppressHardwareUpdate)
        {
            const void *srcData = mShadowBuffer->lock(mLockStart,
                                                       mLockSize,
                                                       HBL_READ_ONLY);

			static_cast<GLES2HardwareBufferManagerBase*>(mMgr)->getStateCacheManager()->bindGLBuffer(GL_ARRAY_BUFFER, mBufferId);

            OGRE_CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)mSizeInBytes, srcData,
                                             GLES2HardwareBufferManager::getGLUsage(mUsage)));

            mShadowBuffer->unlock();
            mShadowUpdated = false;
        }
    }
}
