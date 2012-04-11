/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2012 Torus Knot Software Ltd

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

namespace Ogre {
    GLES2HardwareVertexBuffer::GLES2HardwareVertexBuffer(HardwareBufferManagerBase* mgr, 
													   size_t vertexSize,
                                                       size_t numVertices,
                                                       HardwareBuffer::Usage usage,
                                                       bool useShadowBuffer)
        : HardwareVertexBuffer(mgr, vertexSize, numVertices, usage, false, useShadowBuffer)
    {
        if (!useShadowBuffer)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Only supported with shadowBuffer",
                        "GLES2HardwareVertexBuffer");
        }

        glGenBuffers(1, &mBufferId);
        GL_CHECK_ERROR;

        if (!mBufferId)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Cannot create GL ES vertex buffer",
                        "GLES2HardwareVertexBuffer::GLES2HardwareVertexBuffer");
        }

        dynamic_cast<GLES2RenderSystem*>(Root::getSingleton().getRenderSystem())->_bindGLBuffer(GL_ARRAY_BUFFER, mBufferId);
        glBufferData(GL_ARRAY_BUFFER, mSizeInBytes, NULL,
                     GLES2HardwareBufferManager::getGLUsage(usage));
        GL_CHECK_ERROR;
    }

    GLES2HardwareVertexBuffer::~GLES2HardwareVertexBuffer()
    {
        glDeleteBuffers(1, &mBufferId);
        GL_CHECK_ERROR;

        // Delete the cached value
        dynamic_cast<GLES2RenderSystem*>(Root::getSingleton().getRenderSystem())->_deleteGLBuffer(GL_ARRAY_BUFFER, mBufferId);
    }

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

#if GL_OES_mapbuffer
        if (!retPtr)
		{
            GLenum access = 0;
			// Use glMapBuffer
            dynamic_cast<GLES2RenderSystem*>(Root::getSingleton().getRenderSystem())->_bindGLBuffer(GL_ARRAY_BUFFER, mBufferId);

            if(options == HBL_DISCARD)
			{
				// Discard the buffer
				glBufferData(GL_ARRAY_BUFFER, mSizeInBytes, NULL, 
					GLES2HardwareBufferManager::getGLUsage(mUsage));
                GL_CHECK_ERROR;
			}
			if (mUsage & HBU_WRITE_ONLY)
				access = GL_WRITE_ONLY_OES;

			void* pBuffer = glMapBufferOES(GL_ARRAY_BUFFER, access);
            GL_CHECK_ERROR;

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
#if GL_OES_mapbuffer
            dynamic_cast<GLES2RenderSystem*>(Root::getSingleton().getRenderSystem())->_bindGLBuffer(GL_ARRAY_BUFFER, mBufferId);

			if(!glUnmapBufferOES(GL_ARRAY_BUFFER))
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
        dynamic_cast<GLES2RenderSystem*>(Root::getSingleton().getRenderSystem())->_bindGLBuffer(GL_ARRAY_BUFFER, mBufferId);

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
            glBufferData(GL_ARRAY_BUFFER, mSizeInBytes, pSource,
                         GLES2HardwareBufferManager::getGLUsage(mUsage));
            GL_CHECK_ERROR;
        }
        else
        {
            if(discardWholeBuffer)
            {
                glBufferData(GL_ARRAY_BUFFER, mSizeInBytes, NULL, 
                                GLES2HardwareBufferManager::getGLUsage(mUsage));
                GL_CHECK_ERROR;
            }

            glBufferSubData(GL_ARRAY_BUFFER, offset, length, pSource);
            GL_CHECK_ERROR;
        }
    }

    void GLES2HardwareVertexBuffer::_updateFromShadow(void)
    {
        if (mUseShadowBuffer && mShadowUpdated && !mSuppressHardwareUpdate)
        {
            const void *srcData = mShadowBuffer->lock(mLockStart,
                                                       mLockSize,
                                                       HBL_READ_ONLY);

            dynamic_cast<GLES2RenderSystem*>(Root::getSingleton().getRenderSystem())->_bindGLBuffer(GL_ARRAY_BUFFER, mBufferId);

            // DJR - Always update the entire buffer. Much faster on mobiles.
            // A better approach would be to double buffer or ring buffer

            // Update whole buffer if possible, otherwise normal
//            if (mLockStart == 0 && mLockSize == mSizeInBytes)
//            {
                glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)mSizeInBytes, srcData,
                             GLES2HardwareBufferManager::getGLUsage(mUsage));
                GL_CHECK_ERROR;
//            }
//            else
//            {
//                glBufferSubData(GL_ARRAY_BUFFER, mLockStart, mLockSize, srcData);
//                GL_CHECK_ERROR;
//            }

            mShadowBuffer->unlock();
            mShadowUpdated = false;
        }
    }
}
