/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
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

#include "OgreGLESHardwareIndexBuffer.h"
#include "OgreGLESHardwareBufferManager.h"
#include "OgreException.h"
#include "OgreLogManager.h"

namespace Ogre {
    GLESHardwareIndexBuffer::GLESHardwareIndexBuffer(HardwareBufferManagerBase* mgr, 
													 IndexType idxType,
                                                     size_t numIndexes,
                                                     HardwareBuffer::Usage usage,
                                                     bool useShadowBuffer)
        : HardwareIndexBuffer(mgr, idxType, numIndexes, usage, false, true)
    {
		if (idxType == HardwareIndexBuffer::IT_32BIT)
		{
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
				"32 bit hardware buffers are not allowed in OpenGL ES.",
				"GLESHardwareIndexBuffer");
		}

		if (!useShadowBuffer)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Only support with shadowBuffer",
                        "GLESHardwareIndexBuffer");
        }

        createBuffer();
    }

    GLESHardwareIndexBuffer::~GLESHardwareIndexBuffer()
    {
        destroyBuffer();
    }
    
    void GLESHardwareIndexBuffer::createBuffer()
    {
        glGenBuffers(1, &mBufferId);
        GL_CHECK_ERROR;
        
        if (!mBufferId)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Cannot create GL ES index buffer",
                        "GLESHardwareIndexBuffer::GLESHardwareIndexBuffer");
        }
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferId);
        GL_CHECK_ERROR;
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mSizeInBytes, NULL,
                     GLESHardwareBufferManager::getGLUsage(mUsage));
        GL_CHECK_ERROR;
    }
    
    void GLESHardwareIndexBuffer::destroyBuffer()
    {
        glDeleteBuffers(1, &mBufferId);
        GL_CHECK_ERROR;
    }
    
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
    void GLESHardwareIndexBuffer::notifyOnContextLost()
    {
        destroyBuffer();
    }
    
    void GLESHardwareIndexBuffer::notifyOnContextReset()
    {
        createBuffer();
        mShadowUpdated = true;
        _updateFromShadow();
    }
#endif

    void GLESHardwareIndexBuffer::unlockImpl(void)
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
            static_cast<GLESHardwareBufferManager*>(
                    HardwareBufferManager::getSingletonPtr())->deallocateScratch(mScratchPtr);

            mLockedToScratch = false;
        }
        else
        {
#if defined(GL_GLEXT_PROTOTYPES)
			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mBufferId );
            
			if(!glUnmapBufferOES( GL_ELEMENT_ARRAY_BUFFER ))
			{
				OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
                            "Buffer data corrupted, please reload", 
                            "GLESHardwareIndexBuffer::unlock");
			}
#else
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Lock to scratch is only supported",
                        "GLESHardwareIndexBuffer::unlockImpl");
#endif
        }
        mIsLocked = false;
    }

    void* GLESHardwareIndexBuffer::lockImpl(size_t offset,
                                            size_t length,
                                            LockOptions options)
    {
        if(mIsLocked)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Invalid attempt to lock an index buffer that has already been locked",
                        "GLESHardwareIndexBuffer::lock");
        }

        void* retPtr = 0;
		GLESHardwareBufferManager* glBufManager = static_cast<GLESHardwareBufferManager*>(HardwareBufferManager::getSingletonPtr());

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

                if (options != HBL_DISCARD && options != HBL_NO_OVERWRITE)
                {
                    readData(offset, length, retPtr);
                }
            }
        }
        else
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Invalid Buffer lockSize",
                        "GLESHardwareIndexBuffer::lock");
        }

#if defined(GL_GLEXT_PROTOTYPES)
        if (!retPtr)
		{
            GLenum access = 0;
			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mBufferId );
			// Use glMapBuffer
			if(options == HBL_DISCARD || options == HBL_NO_OVERWRITE)
			{
				// Discard the buffer
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, mSizeInBytes, NULL, 
                                GLESHardwareBufferManager::getGLUsage(mUsage));
			}
			if (mUsage & HBU_WRITE_ONLY)
				access = GL_WRITE_ONLY_OES;
            
			void* pBuffer = glMapBufferOES( GL_ELEMENT_ARRAY_BUFFER, access );
            
			if(pBuffer == 0)
			{
				OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
                            "Index Buffer: Out of memory", 
                            "GLESHardwareIndexBuffer::lock");
			}
            
			// return offsetted
			retPtr = static_cast<void*>(static_cast<unsigned char*>(pBuffer) + offset);
            
			mLockedToScratch = false;
		}
		mIsLocked = true;
#endif
        
        return retPtr;
    }

    void GLESHardwareIndexBuffer::readData(size_t offset,
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
                        "GLESHardwareIndexBuffer::readData");
        }
    }

    void GLESHardwareIndexBuffer::writeData(size_t offset, size_t length,
                                            const void* pSource,
                                            bool discardWholeBuffer)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferId);
        GL_CHECK_ERROR;

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
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, mSizeInBytes, pSource,
                         GLESHardwareBufferManager::getGLUsage(mUsage));
            GL_CHECK_ERROR;
        }
        else
        {
            if (discardWholeBuffer)
            {
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, mSizeInBytes, NULL,
                                GLESHardwareBufferManager::getGLUsage(mUsage));
            }

            // Now update the real buffer
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, length, pSource);
            GL_CHECK_ERROR;
        }
    }

    void GLESHardwareIndexBuffer::_updateFromShadow(void)
    {
        if (mUseShadowBuffer && mShadowUpdated && !mSuppressHardwareUpdate)
        {
            const void *srcData = mShadowBuffer->lock(mLockStart, mLockSize,
                                                       HBL_READ_ONLY);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferId);
            GL_CHECK_ERROR;

            // Update whole buffer if possible, otherwise normal
//            if (mLockStart == 0 && mLockSize == mSizeInBytes)
//            {
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, mSizeInBytes, srcData,
                             GLESHardwareBufferManager::getGLUsage(mUsage));
                GL_CHECK_ERROR;
//            }
//            else
//            {
//                glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,
//                                mLockStart, mLockSize, srcData);
//                GL_CHECK_ERROR;
 //           }

            mShadowBuffer->unlock();
            mShadowUpdated = false;
        }
    }
}
