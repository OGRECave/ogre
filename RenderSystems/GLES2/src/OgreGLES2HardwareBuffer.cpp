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

#include "OgreGLES2HardwareBuffer.h"
#include "OgreRoot.h"
#include "OgreGLES2RenderSystem.h"
#include "OgreGLES2StateCacheManager.h"
#include "OgreDefaultHardwareBufferManager.h"

namespace Ogre {
    GLES2HardwareBuffer::GLES2HardwareBuffer(GLenum target, size_t sizeInBytes, uint32 usage, bool useShadowBuffer)
        : HardwareBuffer(usage, false, useShadowBuffer || HANDLE_CONTEXT_LOSS), mTarget(target)
    {
        mSizeInBytes = sizeInBytes;
        mRenderSystem = static_cast<GLES2RenderSystem*>(Root::getSingleton().getRenderSystem());
        createBuffer();

        if (useShadowBuffer || HANDLE_CONTEXT_LOSS)
        {
            mShadowBuffer.reset(new DefaultHardwareBuffer(mSizeInBytes));
        }
    }

    GLES2HardwareBuffer::~GLES2HardwareBuffer()
    {
        destroyBuffer();
    }

    void GLES2HardwareBuffer::createBuffer()
    {
        OGRE_CHECK_GL_ERROR(glGenBuffers(1, &mBufferId));

        if (!mBufferId)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Cannot create GL ES buffer",
                        "GLES2HardwareBuffer::createBuffer");
        }
        
        mRenderSystem->_getStateCacheManager()->bindGLBuffer(mTarget, mBufferId);
        OGRE_CHECK_GL_ERROR(glBufferData(mTarget, mSizeInBytes, NULL, getGLUsage(mUsage)));
    }

    void GLES2HardwareBuffer::destroyBuffer()
    {
        // Delete the cached value
        if(GLES2StateCacheManager* stateCacheManager = mRenderSystem->_getStateCacheManager())
            stateCacheManager->deleteGLBuffer(mTarget, mBufferId);
    }

    void* GLES2HardwareBuffer::lockImpl(size_t offset, size_t length,
                                        HardwareBuffer::LockOptions options)
    {
        GLenum access = 0;

        // Use glMapBuffer
        mRenderSystem->_getStateCacheManager()->bindGLBuffer(mTarget, mBufferId);

        bool writeOnly =
            options == HardwareBuffer::HBL_WRITE_ONLY ||
            ((mUsage & HBU_DETAIL_WRITE_ONLY) &&
             options != HardwareBuffer::HBL_READ_ONLY && options != HardwareBuffer::HBL_NORMAL);

        void* pBuffer = NULL;
        if(mRenderSystem->getCapabilities()->hasCapability(RSC_MAPBUFFER))
        {
            if (writeOnly)
            {
                access = GL_MAP_WRITE_BIT_EXT;
                if (options == HBL_NO_OVERWRITE)
                    access |= GL_MAP_UNSYNCHRONIZED_BIT_EXT;
                if (options == HBL_DISCARD)
                    OGRE_CHECK_GL_ERROR(glBufferData(mTarget, mSizeInBytes, NULL, getGLUsage(mUsage)));
            }
            else if (options == HardwareBuffer::HBL_READ_ONLY)
                access = GL_MAP_READ_BIT_EXT;
            else
                access = GL_MAP_READ_BIT_EXT | GL_MAP_WRITE_BIT_EXT;

            OGRE_CHECK_GL_ERROR(pBuffer = glMapBufferRangeEXT(mTarget, offset, length, access));
        }

        if (!pBuffer)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Buffer: Out of memory",
                        "GLES2HardwareBuffer::lock");
        }

        // pBuffer is already offsetted in glMapBufferRange
        return static_cast<uint8*>(pBuffer);
    }

    void GLES2HardwareBuffer::unlockImpl()
    {
        mRenderSystem->_getStateCacheManager()->bindGLBuffer(mTarget, mBufferId);

        if(mRenderSystem->getCapabilities()->hasCapability(RSC_MAPBUFFER)) {
            GLboolean mapped;
            OGRE_CHECK_GL_ERROR(mapped = glUnmapBufferOES(mTarget));
            if(!mapped)
            {
                OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Buffer data corrupted, please reload",
                            "GLES2HardwareBuffer::unlock");
            }
        }
    }

    void GLES2HardwareBuffer::readData(size_t offset, size_t length, void* pDest)
    {
        if (mShadowBuffer)
        {
            mShadowBuffer->readData(offset, length, pDest);
            return;
        }

        OgreAssert(mRenderSystem->getCapabilities()->hasCapability(RSC_MAPBUFFER),
                   "Read hardware buffer is not supported");

        mRenderSystem->_getStateCacheManager()->bindGLBuffer(mTarget, mBufferId);
        // Map the buffer range then copy out of it into our destination buffer
        void* srcData;
        OGRE_CHECK_GL_ERROR(srcData = glMapBufferRangeEXT(mTarget, offset, length, GL_MAP_READ_BIT_EXT));
        memcpy(pDest, srcData, length);

        // Unmap the buffer since we are done.
        GLboolean mapped;
        OGRE_CHECK_GL_ERROR(mapped = glUnmapBufferOES(mTarget));
        if(!mapped)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Buffer data corrupted, please reload",
                        "GLES2HardwareBuffer::readData");
        }
    }

    void GLES2HardwareBuffer::writeData(size_t offset, size_t length, const void* pSource,
                                        bool discardWholeBuffer)
    {
        if (mShadowBuffer)
        {
            mShadowBuffer->writeData(offset, length, pSource, discardWholeBuffer);
        }

        writeDataImpl(offset, length, pSource, discardWholeBuffer);
    }

    void GLES2HardwareBuffer::writeDataImpl(size_t offset, size_t length, const void* pSource,
                                        bool discardWholeBuffer)
    {
        mRenderSystem->_getStateCacheManager()->bindGLBuffer(mTarget, mBufferId);

        if (offset == 0 && length == mSizeInBytes)
        {
            OGRE_CHECK_GL_ERROR(glBufferData(mTarget, mSizeInBytes, pSource, getGLUsage(mUsage)));
        }
        else
        {
            if(discardWholeBuffer)
            {
                OGRE_CHECK_GL_ERROR(glBufferData(mTarget, mSizeInBytes, NULL, getGLUsage(mUsage)));
            }

            OGRE_CHECK_GL_ERROR(glBufferSubData(mTarget, offset, length, pSource));
        }
    }

    void GLES2HardwareBuffer::copyData(HardwareBuffer& srcBuffer, size_t srcOffset, size_t dstOffset,
                                       size_t length, bool discardWholeBuffer)
    {
        if(!mRenderSystem->hasMinGLVersion(3, 0))
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "GLES3 needed");

        if (mShadowBuffer)
        {
            mShadowBuffer->copyData(srcBuffer, srcOffset, dstOffset, length, discardWholeBuffer);
        }
        // Zero out this(destination) buffer
        OGRE_CHECK_GL_ERROR(glBindBuffer(mTarget, mBufferId));
        OGRE_CHECK_GL_ERROR(glBufferData(mTarget, length, 0, getGLUsage(mUsage)));
        OGRE_CHECK_GL_ERROR(glBindBuffer(mTarget, 0));

        // Do it the fast way.
        OGRE_CHECK_GL_ERROR(glBindBuffer(GL_COPY_READ_BUFFER,
                                         static_cast<GLES2HardwareBuffer&>(srcBuffer).getGLBufferId()));
        OGRE_CHECK_GL_ERROR(glBindBuffer(GL_COPY_WRITE_BUFFER, mBufferId));

        OGRE_CHECK_GL_ERROR(glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, srcOffset, dstOffset, length));

        OGRE_CHECK_GL_ERROR(glBindBuffer(GL_COPY_READ_BUFFER, 0));
        OGRE_CHECK_GL_ERROR(glBindBuffer(GL_COPY_WRITE_BUFFER, 0));
    }

    GLenum GLES2HardwareBuffer::getGLUsage(unsigned int usage)
    {
        return (usage == HBU_GPU_TO_CPU) ? GL_STATIC_READ
                                         : (usage == HBU_GPU_ONLY) ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;
    }

    void GLES2HardwareBuffer::_updateFromShadow(void)
    {
        if (mShadowBuffer && mShadowUpdated && !mSuppressHardwareUpdate)
        {
            HardwareBufferLockGuard shadowLock(mShadowBuffer.get(), mLockStart, mLockSize, HBL_READ_ONLY);
            writeDataImpl(mLockStart, mLockSize, shadowLock.pData, false);

            mShadowUpdated = false;
        }
    }

    void GLES2HardwareBuffer::setGLBufferBinding(GLint binding)
    {
        mBindingPoint = binding;

        // Attach the buffer to the UBO binding
        OGRE_CHECK_GL_ERROR(glBindBufferBase(mTarget, mBindingPoint, mBufferId));
    }

#if HANDLE_CONTEXT_LOSS
    void GLES2HardwareBuffer::notifyOnContextLost()
    {
        destroyBuffer();
    }

    void GLES2HardwareBuffer::notifyOnContextReset()
    {
        createBuffer();
        mShadowUpdated = true;
        _updateFromShadow();
    }
#endif
}
