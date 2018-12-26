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
#include "OgreGLHardwareBufferManager.h"
#include "OgreGLHardwareVertexBuffer.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreGLStateCacheManager.h"

namespace Ogre {

    //---------------------------------------------------------------------
    GLHardwareVertexBuffer::GLHardwareVertexBuffer(HardwareBufferManagerBase* mgr, size_t vertexSize, 
        size_t numVertices, HardwareBuffer::Usage usage, bool useShadowBuffer)
        : HardwareVertexBuffer(mgr, vertexSize, numVertices, usage, false, useShadowBuffer), mLockedToScratch(false),
        mScratchOffset(0), mScratchSize(0), mScratchPtr(0), mScratchUploadOnUnlock(false)
    {
        glGenBuffersARB( 1, &mBufferId );

        if (!mBufferId)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
                "Cannot create GL vertex buffer", 
                "GLHardwareVertexBuffer::GLHardwareVertexBuffer");
        }

        static_cast<GLHardwareBufferManager*>(mMgr)->getStateCacheManager()->bindGLBuffer(GL_ARRAY_BUFFER_ARB, mBufferId);

        // Initialise mapped buffer and set usage
        glBufferDataARB(GL_ARRAY_BUFFER_ARB, mSizeInBytes, NULL, 
            GLHardwareBufferManager::getGLUsage(usage));

        //std::cerr << "creating vertex buffer = " << mBufferId << std::endl;
    }
    //---------------------------------------------------------------------
    GLHardwareVertexBuffer::~GLHardwareVertexBuffer()
    {
        if(GLStateCacheManager* stateCacheManager = static_cast<GLHardwareBufferManager*>(mMgr)->getStateCacheManager())
            stateCacheManager->deleteGLBuffer(GL_ARRAY_BUFFER_ARB, mBufferId);
    }
    //---------------------------------------------------------------------
    void* GLHardwareVertexBuffer::lockImpl(size_t offset, 
        size_t length, LockOptions options)
    {
        if(mIsLocked)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                "Invalid attempt to lock an vertex buffer that has already been locked",
                "GLHardwareVertexBuffer::lock");
        }


        void* retPtr = 0;

        GLHardwareBufferManager* glBufManager = static_cast<GLHardwareBufferManager*>(HardwareBufferManager::getSingletonPtr());

        // Try to use scratch buffers for smaller buffers
        if( length < glBufManager->getGLMapBufferThreshold() )
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

                if (options != HBL_DISCARD && options != HBL_NO_OVERWRITE)
                {
                    // have to read back the data before returning the pointer
                    readData(offset, length, retPtr);
                }
            }
        }
        
        if (!retPtr)
        {
            GLenum access = 0;
            // Use glMapBuffer
            static_cast<GLHardwareBufferManager*>(mMgr)->getStateCacheManager()->bindGLBuffer(GL_ARRAY_BUFFER_ARB, mBufferId);
            // Use glMapBuffer
            if(options == HBL_DISCARD || options == HBL_NO_OVERWRITE) // TODO: check possibility to use GL_MAP_UNSYNCHRONIZED_BIT for HBL_NO_OVERWRITE locking promise
            {
                // Discard the buffer
                glBufferDataARB(GL_ARRAY_BUFFER_ARB, mSizeInBytes, NULL, 
                    GLHardwareBufferManager::getGLUsage(mUsage));
            }
            if (mUsage & HBU_WRITE_ONLY)
                access = GL_WRITE_ONLY_ARB;
            else if (options == HBL_READ_ONLY)
                access = GL_READ_ONLY_ARB;
            else
                access = GL_READ_WRITE_ARB;

            void* pBuffer = glMapBufferARB( GL_ARRAY_BUFFER_ARB, access);
            if(pBuffer == 0)
            {
                OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
                    "Vertex Buffer: Out of memory", "GLHardwareVertexBuffer::lock");
            }

            // return offsetted
            retPtr = static_cast<void*>(
                static_cast<unsigned char*>(pBuffer) + offset);

            mLockedToScratch = false;
        }
        mIsLocked = true;
        return retPtr;
    }
    //---------------------------------------------------------------------
    void GLHardwareVertexBuffer::unlockImpl(void)
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
            static_cast<GLHardwareBufferManager*>(
                HardwareBufferManager::getSingletonPtr())->deallocateScratch(mScratchPtr);

            mLockedToScratch = false;
        }
        else
        {
            static_cast<GLHardwareBufferManager*>(mMgr)->getStateCacheManager()->bindGLBuffer(GL_ARRAY_BUFFER_ARB, mBufferId);

            if(!glUnmapBufferARB( GL_ARRAY_BUFFER_ARB ))
            {
                OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
                    "Buffer data corrupted, please reload", 
                    "GLHardwareVertexBuffer::unlock");
            }
        }

        mIsLocked = false;
    }
    //---------------------------------------------------------------------
    void GLHardwareVertexBuffer::readData(size_t offset, size_t length, 
        void* pDest)
    {
        if(mUseShadowBuffer)
        {
            mShadowBuffer->readData(offset, length, pDest);
        }
        else
        {
            // get data from the real buffer
            static_cast<GLHardwareBufferManager*>(mMgr)->getStateCacheManager()->bindGLBuffer(GL_ARRAY_BUFFER_ARB, mBufferId);

            glGetBufferSubDataARB(GL_ARRAY_BUFFER_ARB, offset, length, pDest);
        }
    }
    //---------------------------------------------------------------------
    void GLHardwareVertexBuffer::writeData(size_t offset, size_t length, 
            const void* pSource, bool discardWholeBuffer)
    {
        static_cast<GLHardwareBufferManager*>(mMgr)->getStateCacheManager()->bindGLBuffer(GL_ARRAY_BUFFER_ARB, mBufferId);

        // Update the shadow buffer
        if(mUseShadowBuffer)
        {
            mShadowBuffer->writeData(offset, length, pSource, discardWholeBuffer);
        }

        if (offset == 0 && length == mSizeInBytes)
        {
            glBufferDataARB(GL_ARRAY_BUFFER_ARB, mSizeInBytes, pSource, 
                GLHardwareBufferManager::getGLUsage(mUsage));
        }
        else
        {
            if(discardWholeBuffer)
            {
                glBufferDataARB(GL_ARRAY_BUFFER_ARB, mSizeInBytes, NULL, 
                    GLHardwareBufferManager::getGLUsage(mUsage));
            }

            // Now update the real buffer
            glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, offset, length, pSource); 
        }
    }
    //---------------------------------------------------------------------
    void GLHardwareVertexBuffer::_updateFromShadow(void)
    {
        if (mUseShadowBuffer && mShadowUpdated && !mSuppressHardwareUpdate)
        {
            HardwareBufferLockGuard shadowLock(mShadowBuffer.get(), mLockStart, mLockSize, HBL_READ_ONLY);

            static_cast<GLHardwareBufferManager*>(mMgr)->getStateCacheManager()->bindGLBuffer(GL_ARRAY_BUFFER_ARB, mBufferId);

            // Update whole buffer if possible, otherwise normal
            if (mLockStart == 0 && mLockSize == mSizeInBytes)
            {
                glBufferDataARB(GL_ARRAY_BUFFER_ARB, mSizeInBytes, shadowLock.pData,
                    GLHardwareBufferManager::getGLUsage(mUsage));
            }
            else
            {
                glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, mLockStart, mLockSize, shadowLock.pData);
            }

            mShadowUpdated = false;
        }
    }
}
