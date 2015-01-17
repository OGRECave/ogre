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

#include "OgreGLES2HardwareIndexBuffer.h"
#include "OgreGLES2HardwareBufferManager.h"
#include "OgreGLES2RenderSystem.h"
#include "OgreRoot.h"
#include "OgreGLES2Util.h"
#include "OgreGLES2StateCacheManager.h"

namespace Ogre {
namespace v1 {
    GLES2HardwareIndexBuffer::GLES2HardwareIndexBuffer(HardwareBufferManagerBase* mgr,
                                                     IndexType idxType,
                                                     size_t numIndexes,
                                                     HardwareBuffer::Usage usage,
                                                     bool useShadowBuffer)
        : HardwareIndexBuffer(mgr, idxType, numIndexes, usage, false, useShadowBuffer)
    {
#if OGRE_NO_GLES3_SUPPORT == 1
        if (!Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_32BIT_INDEX) &&
            idxType == HardwareIndexBuffer::IT_32BIT)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                "32 bit hardware buffers are not allowed in OpenGL ES.",
                "GLES2HardwareIndexBuffer");
        }
#endif
        createBuffer();
    }

    GLES2HardwareIndexBuffer::~GLES2HardwareIndexBuffer()
    {
        destroyBuffer();
    }
    
    void GLES2HardwareIndexBuffer::createBuffer()
    {
        OGRE_CHECK_GL_ERROR(glGenBuffers(1, &mBufferId));

        if(getGLES2SupportRef()->checkExtension("GL_EXT_debug_label"))
        {
            OGRE_IF_IOS_VERSION_IS_GREATER_THAN(5.0)
            OGRE_CHECK_GL_ERROR(glLabelObjectEXT(GL_BUFFER_OBJECT_EXT, mBufferId, 0, ("Index Buffer #" + StringConverter::toString(mBufferId)).c_str()));
        }

        if (!mBufferId)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Cannot create GL ES index buffer",
                        "GLES2HardwareIndexBuffer::GLES2HardwareIndexBuffer");
        }
        
        static_cast<GLES2HardwareBufferManagerBase*>(mMgr)->getStateCacheManager()->bindGLBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferId);
        
        OGRE_CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)mSizeInBytes, NULL,
                                         GLES2HardwareBufferManager::getGLUsage(mUsage)));
    }
    
    void GLES2HardwareIndexBuffer::destroyBuffer()
    {
        // Delete the cached value
        static_cast<GLES2HardwareBufferManagerBase*>(mMgr)->getStateCacheManager()->deleteGLBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferId);
    }
    
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID || OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
    void GLES2HardwareIndexBuffer::notifyOnContextLost()
    {
        destroyBuffer();
    }
    
    void GLES2HardwareIndexBuffer::notifyOnContextReset()
    {
        createBuffer();
        mShadowUpdated = true;
        _updateFromShadow();
    }
#endif
    
    void GLES2HardwareIndexBuffer::unlockImpl(void)
    {
        static_cast<GLES2HardwareBufferManagerBase*>(mMgr)->getStateCacheManager()->bindGLBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferId);

#if OGRE_NO_GLES3_SUPPORT == 0 || defined(GL_EXT_map_buffer_range)
        if (mUsage & HBU_WRITE_ONLY)
        {
            OGRE_CHECK_GL_ERROR(glFlushMappedBufferRangeEXT(GL_ELEMENT_ARRAY_BUFFER, mLockStart, mLockSize));
        }
#endif
        GLboolean mapped;
        OGRE_CHECK_GL_ERROR(mapped = glUnmapBufferOES(GL_ELEMENT_ARRAY_BUFFER));
        if(!mapped)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
                "Buffer data corrupted, please reload", 
                "GLES2HardwareIndexBuffer::unlock");
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

        GLenum access = 0;
        static_cast<GLES2HardwareBufferManagerBase*>(mMgr)->getStateCacheManager()->bindGLBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferId);

        void* pBuffer;
#if OGRE_NO_GLES3_SUPPORT == 0 || defined(GL_EXT_map_buffer_range)
        if (mUsage & HBU_WRITE_ONLY)
        {
            access = GL_MAP_WRITE_BIT_EXT;
            access |= GL_MAP_FLUSH_EXPLICIT_BIT_EXT;
            if(options == HBL_DISCARD || options == HBL_NO_OVERWRITE)
            {
                // Discard the buffer
                access |= GL_MAP_INVALIDATE_RANGE_BIT_EXT;
            }
        }
        else if (options == HBL_READ_ONLY)
            access = GL_MAP_READ_BIT_EXT;
        else
            access = GL_MAP_READ_BIT_EXT | GL_MAP_WRITE_BIT_EXT;

        OGRE_CHECK_GL_ERROR(pBuffer = glMapBufferRangeEXT(GL_ELEMENT_ARRAY_BUFFER, offset, length, access));
#else
        // Use glMapBuffer
        if(options == HBL_DISCARD || options == HBL_NO_OVERWRITE)
        {
            // Discard the buffer
            OGRE_CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)mSizeInBytes, NULL,
                                             GLES2HardwareBufferManager::getGLUsage(mUsage)));
        }
        if (mUsage & HBU_WRITE_ONLY)
            access = GL_WRITE_ONLY_OES;

        OGRE_CHECK_GL_ERROR(pBuffer = glMapBufferOES(GL_ELEMENT_ARRAY_BUFFER, access));
#endif
        if(pBuffer == 0)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
                "Index Buffer: Out of memory", 
                "GLES2HardwareIndexBuffer::lock");
        }

        // return offsetted
        void *retPtr = static_cast<void*>(
            static_cast<unsigned char*>(pBuffer) + offset);
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
            if(getGLES2SupportRef()->checkExtension("GL_EXT_map_buffer_range") || gleswIsSupported(3, 0))
            {
                // Map the buffer range then copy out of it into our destination buffer
                void* srcData;
                OGRE_CHECK_GL_ERROR(srcData = glMapBufferRangeEXT(GL_ELEMENT_ARRAY_BUFFER, offset, length, GL_MAP_READ_BIT_EXT));
                memcpy(pDest, srcData, length);

                // Unmap the buffer since we are done.
                GLboolean mapped;
                OGRE_CHECK_GL_ERROR(mapped = glUnmapBufferOES(GL_ELEMENT_ARRAY_BUFFER));
                if(!mapped)
                {
                    OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                                "Buffer data corrupted, please reload",
                                "GLES2HardwareIndexBuffer::readData");
                }
            }
            else
            {
                OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                            "Reading hardware buffer is not supported",
                            "GLES2HardwareIndexBuffer::readData");
            }
        }
    }

    void GLES2HardwareIndexBuffer::writeData(size_t offset, size_t length,
                                            const void* pSource,
                                            bool discardWholeBuffer)
    {
        static_cast<GLES2HardwareBufferManagerBase*>(mMgr)->getStateCacheManager()->bindGLBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferId);

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
            OGRE_CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)mSizeInBytes, pSource,
                                             GLES2HardwareBufferManager::getGLUsage(mUsage)));
        }
        else
        {
            if (discardWholeBuffer)
            {
                OGRE_CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)mSizeInBytes, NULL,
                                                 GLES2HardwareBufferManager::getGLUsage(mUsage)));
            }

            // Now update the real buffer
            OGRE_CHECK_GL_ERROR(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, (GLintptr)offset, (GLsizeiptr)length, pSource));
        }
    }

#if OGRE_NO_GLES3_SUPPORT == 0
    void GLES2HardwareIndexBuffer::copyData(HardwareBuffer& srcBuffer, size_t srcOffset,
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
            OGRE_CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER, length, 0, GLES2HardwareBufferManager::getGLUsage(mUsage)));
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

            // Do it the fast way.
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_COPY_READ_BUFFER, static_cast<GLES2HardwareIndexBuffer &>(srcBuffer).getGLBufferId()));
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_COPY_WRITE_BUFFER, mBufferId));

            OGRE_CHECK_GL_ERROR(glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, srcOffset, dstOffset, length));

            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_COPY_READ_BUFFER, 0));
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_COPY_WRITE_BUFFER, 0));
        }
    }
#endif

    void GLES2HardwareIndexBuffer::_updateFromShadow(void)
    {
        if (mUseShadowBuffer && mShadowUpdated && !mSuppressHardwareUpdate)
        {
            const void *srcData = mShadowBuffer->lock(mLockStart, mLockSize, HBL_READ_ONLY);

            static_cast<GLES2HardwareBufferManagerBase*>(mMgr)->getStateCacheManager()->bindGLBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferId);

            OGRE_CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)mSizeInBytes, srcData,
                                             GLES2HardwareBufferManager::getGLUsage(mUsage)));

            mShadowBuffer->unlock();
            mShadowUpdated = false;
        }
    }
}
}
