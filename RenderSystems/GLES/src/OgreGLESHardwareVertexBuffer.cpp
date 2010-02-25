/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
Copyright (c) 2000-2009 Torus Knot Software Ltd

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

#include "OgreGLESHardwareBufferManager.h"
#include "OgreGLESHardwareVertexBuffer.h"
#include "OgreException.h"
#include "OgreLogManager.h"

namespace Ogre {
    GLESHardwareVertexBuffer::GLESHardwareVertexBuffer(HardwareBufferManagerBase* mgr, 
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
                        "GLESHardwareVertexBuffer");
        }

        glGenBuffers(1, &mBufferId);
        GL_CHECK_ERROR;

        if (!mBufferId)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Cannot create GL vertex buffer",
                        "GLESHardwareVertexBuffer::GLESHardwareVertexBuffer");
        }

        glBindBuffer(GL_ARRAY_BUFFER, mBufferId);
        GL_CHECK_ERROR;
        glBufferData(GL_ARRAY_BUFFER, mSizeInBytes, NULL,
                     GLESHardwareBufferManager::getGLUsage(usage));
        GL_CHECK_ERROR;
    }

    GLESHardwareVertexBuffer::~GLESHardwareVertexBuffer()
    {
        glDeleteBuffers(1, &mBufferId);
        GL_CHECK_ERROR;
    }

    void* GLESHardwareVertexBuffer::lockImpl(size_t offset,
                                           size_t length,
                                           LockOptions options)
    {
        GLenum access = 0;

        if (mIsLocked)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Invalid attempt to lock an index buffer that has already been locked",
                        "GLESHardwareVertexBuffer::lock");
        }

        void* retPtr = 0;

        if (length < OGRE_GL_MAP_BUFFER_THRESHOLD)
        {
            retPtr = static_cast<GLESHardwareBufferManager*>(
                HardwareBufferManager::getSingletonPtr())->allocateScratch((uint32)length);
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
                        "GLESHardwareVertexBuffer::lock");
        }

#if GL_OES_mapbuffer
        if (!retPtr)
		{
			// Use glMapBuffer
			glBindBuffer( GL_ARRAY_BUFFER, mBufferId );
			// Use glMapBuffer
			if(options == HBL_DISCARD)
			{
				// Discard the buffer
				glBufferData(GL_ARRAY_BUFFER, mSizeInBytes, NULL, 
                                GLESHardwareBufferManager::getGLUsage(mUsage));
                
			}
			if (mUsage & HBU_WRITE_ONLY)
				access = GL_WRITE_ONLY_OES;
            
			void* pBuffer = glMapBufferOES( GL_ARRAY_BUFFER, access);
            
			if(pBuffer == 0)
			{
				OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
                            "Vertex Buffer: Out of memory", "GLESHardwareVertexBuffer::lock");
			}
            
			// return offsetted
			retPtr = static_cast<void*>(static_cast<unsigned char*>(pBuffer) + offset);
            
			mLockedToScratch = false;
		}
#endif
		mIsLocked = true;
        return retPtr;
    }

    void GLESHardwareVertexBuffer::unlockImpl(void)
    {
        if (mLockedToScratch)
        {
            if (mScratchUploadOnUnlock)
            {
                    // have to write the data back to vertex buffer
                    writeData(mScratchOffset, mScratchSize, mScratchPtr,
                              mScratchOffset == 0 && mScratchSize == getSizeInBytes());
            }

            static_cast<GLESHardwareBufferManager*>(
                HardwareBufferManager::getSingletonPtr())->deallocateScratch(mScratchPtr);

            mLockedToScratch = false;
        }
        else
        {
#if GL_OES_mapbuffer
			glBindBuffer(GL_ARRAY_BUFFER, mBufferId);
            
			if(!glUnmapBufferOES( GL_ARRAY_BUFFER ))
			{
				OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
                            "Buffer data corrupted, please reload", 
                            "GLESHardwareVertexBuffer::unlock");
			}
#else
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Only locking to scratch is supported",
                        "GLESHardwareVertexBuffer::unlockImpl");
#endif
        }
        mIsLocked = false;
    }

    void GLESHardwareVertexBuffer::readData(size_t offset, size_t length,
        void* pDest)
    {
        if (mUseShadowBuffer)
        {
            void* srcData = mpShadowBuffer->lock(offset, length, HBL_READ_ONLY);
            memcpy(pDest, srcData, length);
            mpShadowBuffer->unlock();
        }
        else
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Read hardware buffer is not supported",
                        "GLESHardwareVertexBuffer::readData");
        }
    }

    void GLESHardwareVertexBuffer::writeData(size_t offset,
                                           size_t length,
                                           const void* pSource,
                                           bool discardWholeBuffer)
    {
        glBindBuffer(GL_ARRAY_BUFFER, mBufferId);
        GL_CHECK_ERROR;

        // Update the shadow buffer
        if(mUseShadowBuffer)
        {
            void* destData = mpShadowBuffer->lock(offset, length,
                                                  discardWholeBuffer ? HBL_DISCARD : HBL_NORMAL);
            memcpy(destData, pSource, length);
            mpShadowBuffer->unlock();
        }

        if (offset == 0 && length == mSizeInBytes)
        {
            glBufferData(GL_ARRAY_BUFFER, mSizeInBytes, pSource,
                         GLESHardwareBufferManager::getGLUsage(mUsage));
            GL_CHECK_ERROR;
        }
        else
        {
            if(discardWholeBuffer)
            {
                glBufferData(GL_ARRAY_BUFFER, mSizeInBytes, NULL, 
                                GLESHardwareBufferManager::getGLUsage(mUsage));
            }

            glBufferSubData(GL_ARRAY_BUFFER, offset, length, pSource);
            GL_CHECK_ERROR;
        }
    }

    void GLESHardwareVertexBuffer::_updateFromShadow(void)
    {
        if (mUseShadowBuffer && mShadowUpdated && !mSuppressHardwareUpdate)
        {
            const void *srcData = mpShadowBuffer->lock(mLockStart,
                                                       mLockSize,
                                                       HBL_READ_ONLY);

            glBindBuffer(GL_ARRAY_BUFFER, mBufferId);
            GL_CHECK_ERROR;

            // Update whole buffer if possible, otherwise normal
            if (mLockStart == 0 && mLockSize == mSizeInBytes)
            {
                glBufferData(GL_ARRAY_BUFFER, mSizeInBytes, srcData,
                             GLESHardwareBufferManager::getGLUsage(mUsage));
                GL_CHECK_ERROR;
            }
            else
            {
                glBufferSubData(GL_ARRAY_BUFFER, mLockStart, mLockSize, srcData);
                GL_CHECK_ERROR;
            }

            mpShadowBuffer->unlock();
            mShadowUpdated = false;
        }
    }
}
