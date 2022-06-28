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
#include "OgreGLHardwareBuffer.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreGLStateCacheManager.h"
#include "OgreRoot.h"
#include "OgreGLRenderSystem.h"

namespace Ogre {

    //---------------------------------------------------------------------
    GLHardwareVertexBuffer::GLHardwareVertexBuffer(GLenum target, size_t sizeInBytes,
        Usage usage, bool useShadowBuffer)
        : HardwareBuffer(usage, false, useShadowBuffer), mTarget(target), mLockedToScratch(false),
        mScratchOffset(0), mScratchSize(0), mScratchPtr(0), mScratchUploadOnUnlock(false)
    {
        mSizeInBytes = sizeInBytes;
        mRenderSystem = static_cast<GLRenderSystem*>(Root::getSingleton().getRenderSystem());

        glGenBuffersARB( 1, &mBufferId );

        if (!mBufferId)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Cannot create GL buffer");
        }

        mRenderSystem->_getStateCacheManager()->bindGLBuffer(mTarget, mBufferId);

        // Initialise mapped buffer and set usage
        glBufferDataARB(mTarget, mSizeInBytes, NULL, GLHardwareBufferManager::getGLUsage(usage));

        //std::cerr << "creating vertex buffer = " << mBufferId << std::endl;
    }
    //---------------------------------------------------------------------
    GLHardwareVertexBuffer::~GLHardwareVertexBuffer()
    {
        if(GLStateCacheManager* stateCacheManager = mRenderSystem->_getStateCacheManager())
            stateCacheManager->deleteGLBuffer(mTarget, mBufferId);
    }
    //---------------------------------------------------------------------
    void* GLHardwareVertexBuffer::lockImpl(size_t offset, 
        size_t length, LockOptions options)
    {
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
            mRenderSystem->_getStateCacheManager()->bindGLBuffer(mTarget, mBufferId);
            // Use glMapBuffer
            if(options == HBL_DISCARD) // TODO: check possibility to use GL_MAP_UNSYNCHRONIZED_BIT for HBL_NO_OVERWRITE locking promise
            {
                // Discard the buffer
                glBufferDataARB(mTarget, mSizeInBytes, NULL, GLHardwareBufferManager::getGLUsage(mUsage));
            }
            if (mUsage & HBU_DETAIL_WRITE_ONLY)
                access = GL_WRITE_ONLY_ARB;
            else if (options == HBL_READ_ONLY)
                access = GL_READ_ONLY_ARB;
            else
                access = GL_READ_WRITE_ARB;

            void* pBuffer = glMapBufferARB( mTarget, access);
            if(pBuffer == 0)
            {
                OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Buffer: Out of memory");
            }

            // return offsetted
            retPtr = static_cast<void*>(static_cast<unsigned char*>(pBuffer) + offset);

            mLockedToScratch = false;
        }

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
            mRenderSystem->_getStateCacheManager()->bindGLBuffer(mTarget, mBufferId);

            if(!glUnmapBufferARB( mTarget ))
            {
                OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Buffer data corrupted, please reload");
            }
        }
    }
    //---------------------------------------------------------------------
    void GLHardwareVertexBuffer::readData(size_t offset, size_t length, 
        void* pDest)
    {
        if(mShadowBuffer)
        {
            mShadowBuffer->readData(offset, length, pDest);
        }
        else
        {
            // get data from the real buffer
            mRenderSystem->_getStateCacheManager()->bindGLBuffer(mTarget, mBufferId);

            glGetBufferSubDataARB(mTarget, offset, length, pDest);
        }
    }
    //---------------------------------------------------------------------
    void GLHardwareVertexBuffer::writeData(size_t offset, size_t length, 
            const void* pSource, bool discardWholeBuffer)
    {
        mRenderSystem->_getStateCacheManager()->bindGLBuffer(mTarget, mBufferId);

        // Update the shadow buffer
        if(mShadowBuffer)
        {
            mShadowBuffer->writeData(offset, length, pSource, discardWholeBuffer);
        }

        if (offset == 0 && length == mSizeInBytes)
        {
            glBufferDataARB(mTarget, mSizeInBytes, pSource,
                GLHardwareBufferManager::getGLUsage(mUsage));
        }
        else
        {
            if(discardWholeBuffer)
            {
                glBufferDataARB(mTarget, mSizeInBytes, NULL,
                    GLHardwareBufferManager::getGLUsage(mUsage));
            }

            // Now update the real buffer
            glBufferSubDataARB(mTarget, offset, length, pSource);
        }
    }
    //---------------------------------------------------------------------
    void GLHardwareVertexBuffer::_updateFromShadow(void)
    {
        if (mShadowBuffer && mShadowUpdated && !mSuppressHardwareUpdate)
        {
            HardwareBufferLockGuard shadowLock(mShadowBuffer.get(), mLockStart, mLockSize, HBL_READ_ONLY);

            mRenderSystem->_getStateCacheManager()->bindGLBuffer(mTarget, mBufferId);

            // Update whole buffer if possible, otherwise normal
            if (mLockStart == 0 && mLockSize == mSizeInBytes)
            {
                glBufferDataARB(mTarget, mSizeInBytes, shadowLock.pData,
                    GLHardwareBufferManager::getGLUsage(mUsage));
            }
            else
            {
                glBufferSubDataARB(mTarget, mLockStart, mLockSize, shadowLock.pData);
            }

            mShadowUpdated = false;
        }
    }
}
