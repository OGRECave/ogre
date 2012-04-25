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

#include "OgreGLES2HardwareIndexBuffer.h"
#include "OgreGLES2HardwareBufferManager.h"
#include "OgreGLES2RenderSystem.h"
#include "OgreRoot.h"
#include "OgreGLES2Util.h"

namespace Ogre {
    GLES2HardwareIndexBuffer::GLES2HardwareIndexBuffer(HardwareBufferManagerBase* mgr, 
													 IndexType idxType,
                                                     size_t numIndexes,
                                                     HardwareBuffer::Usage usage,
                                                     bool useShadowBuffer)
        : HardwareIndexBuffer(mgr, idxType, numIndexes, usage, false, useShadowBuffer)
    {
		if (!dynamic_cast<GLES2RenderSystem*>(Root::getSingleton().getRenderSystem())->getGLES2Support()->checkExtension("GL_OES_element_index_uint") && idxType == HardwareIndexBuffer::IT_32BIT)
		{
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
				"32 bit hardware buffers are not allowed in OpenGL ES.",
				"GLES2HardwareIndexBuffer");
		}

		if (!useShadowBuffer)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Only support with shadowBuffer",
                        "GLES2HardwareIndexBuffer");
        }

        glGenBuffers(1, &mBufferId);
        GL_CHECK_ERROR;

        if (!mBufferId)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                "Cannot create GL ES index buffer",
                "GLES2HardwareIndexBuffer::GLES2HardwareIndexBuffer");
        }

        dynamic_cast<GLES2RenderSystem*>(Root::getSingleton().getRenderSystem())->_bindGLBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferId);

        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)mSizeInBytes, NULL,
                     GLES2HardwareBufferManager::getGLUsage(usage));
        GL_CHECK_ERROR;
    }

    GLES2HardwareIndexBuffer::~GLES2HardwareIndexBuffer()
    {
        glDeleteBuffers(1, &mBufferId);
        GL_CHECK_ERROR;
        
        // Delete the cached value
        dynamic_cast<GLES2RenderSystem*>(Root::getSingleton().getRenderSystem())->_deleteGLBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferId);
    }

    void GLES2HardwareIndexBuffer::unlockImpl(void)
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
            dynamic_cast<GLES2RenderSystem*>(Root::getSingleton().getRenderSystem())->_bindGLBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferId);

			if(!glUnmapBufferOES(GL_ELEMENT_ARRAY_BUFFER))
			{
				OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
					"Buffer data corrupted, please reload", 
					"GLES2HardwareIndexBuffer::unlock");
			}
#else
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Lock to scratch is only supported",
                        "GLES2HardwareIndexBuffer::unlockImpl");
#endif
        }
        mIsLocked = false;
    }

    void* GLES2HardwareIndexBuffer::lockImpl(size_t offset,
                                            size_t length,
                                            LockOptions options)
    {
        if(mIsLocked)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Invalid attempt to lock an index buffer that has already been locked",
                        "GLES2HardwareIndexBuffer::lock");
        }

        void* retPtr = 0;
		GLES2HardwareBufferManager* glBufManager = static_cast<GLES2HardwareBufferManager*>(HardwareBufferManager::getSingletonPtr());

        if(length < glBufManager->getGLMapBufferThreshold())
        {
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
                    readData(offset, length, retPtr);
                }
            }
        }
        else
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Invalid Buffer lockSize",
                        "GLES2HardwareIndexBuffer::lock");
        }

#if GL_OES_mapbuffer
		if (!retPtr)
		{
            GLenum access = 0;
            dynamic_cast<GLES2RenderSystem*>(Root::getSingleton().getRenderSystem())->_bindGLBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferId);
			// Use glMapBuffer
			if(options == HBL_DISCARD)
			{
				// Discard the buffer
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)mSizeInBytes, NULL, 
					GLES2HardwareBufferManager::getGLUsage(mUsage));
                GL_CHECK_ERROR;
			}
			if (mUsage & HBU_WRITE_ONLY)
				access = GL_WRITE_ONLY_OES;

			void* pBuffer = glMapBufferOES(GL_ELEMENT_ARRAY_BUFFER, access);
            GL_CHECK_ERROR;

			if(pBuffer == 0)
			{
				OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
					"Index Buffer: Out of memory", 
					"GLES2HardwareIndexBuffer::lock");
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

    void GLES2HardwareIndexBuffer::readData(size_t offset,
                                           size_t length,
                                           void* pDest)
    {
        if(mUseShadowBuffer)
        {
            // Get data from the shadow buffer
            void* srcData = mShadowBuffer->lock(offset, length, HBL_READ_ONLY);
            memcpy(pDest, srcData, length);
            mShadowBuffer->unlock();
        }
        else
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Reading hardware buffer is not supported",
                        "GLES2HardwareIndexBuffer::readData");
        }
    }

    void GLES2HardwareIndexBuffer::writeData(size_t offset, size_t length,
                                            const void* pSource,
                                            bool discardWholeBuffer)
    {
        dynamic_cast<GLES2RenderSystem*>(Root::getSingleton().getRenderSystem())->_bindGLBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferId);

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
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)mSizeInBytes, pSource,
                         GLES2HardwareBufferManager::getGLUsage(mUsage));
            GL_CHECK_ERROR;
        }
        else
        {
            if (discardWholeBuffer)
            {
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)mSizeInBytes, NULL,
                                GLES2HardwareBufferManager::getGLUsage(mUsage));
                GL_CHECK_ERROR;
            }

            // Now update the real buffer
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, (GLintptr)offset, (GLsizeiptr)length, pSource);
            GL_CHECK_ERROR;
        }
    }

    void GLES2HardwareIndexBuffer::_updateFromShadow(void)
    {
        if (mUseShadowBuffer && mShadowUpdated && !mSuppressHardwareUpdate)
        {
            const void *srcData = mShadowBuffer->lock(mLockStart, mLockSize,
                                                       HBL_READ_ONLY);

            dynamic_cast<GLES2RenderSystem*>(Root::getSingleton().getRenderSystem())->_bindGLBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferId);
            GL_CHECK_ERROR;

            // DJR - Always update the entire buffer. Much faster on mobiles.
            // A better approach would be to double buffer or ring buffer

            // Update whole buffer if possible, otherwise normal
//            if (mLockStart == 0 && mLockSize == mSizeInBytes)
//            {
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)mSizeInBytes, srcData,
                             GLES2HardwareBufferManager::getGLUsage(mUsage));
                GL_CHECK_ERROR;
//            }
//            else
//            {
//                glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,
//                                (GLintptr)mLockStart, (GLsizeiptr)mLockSize, srcData);
//                GL_CHECK_ERROR;
//            }

            mShadowBuffer->unlock();
            mShadowUpdated = false;
        }
    }
}
