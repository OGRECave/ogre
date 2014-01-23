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

#include "OgreGLES2HardwareBufferManager.h"

#if OGRE_NO_GLES3_SUPPORT == 0
#include "OgreGLES2HardwareUniformBuffer.h"
#include "OgreRoot.h"
#include "OgreGLES2RenderSystem.h"
#include "OgreGLES2Util.h"

namespace Ogre {
    GLES2HardwareUniformBuffer::GLES2HardwareUniformBuffer(HardwareBufferManagerBase* mgr, 
                                                               size_t bufferSize,
                                                               HardwareBuffer::Usage usage,
                                                               bool useShadowBuffer, const String& name)
    : HardwareUniformBuffer(mgr, bufferSize, usage, useShadowBuffer, name)
    {
        OGRE_CHECK_GL_ERROR(glGenBuffers(1, &mBufferId));

        if(getGLES2SupportRef()->checkExtension("GL_EXT_debug_label"))
        {
            OGRE_IF_IOS_VERSION_IS_GREATER_THAN(5.0)
            OGRE_CHECK_GL_ERROR(glLabelObjectEXT(GL_BUFFER_OBJECT_EXT, mBufferId, 0, ("Uniform Buffer #" + StringConverter::toString(mBufferId)).c_str()));
        }

        if (!mBufferId)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Cannot create GL uniform buffer",
                        "GLES2HardwareUniformBuffer::GLES2HardwareUniformBuffer");
        }

        OGRE_CHECK_GL_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, mBufferId));
        OGRE_CHECK_GL_ERROR(glBufferData(GL_UNIFORM_BUFFER, mSizeInBytes, NULL,
                                         GLES2HardwareBufferManager::getGLUsage(usage)));

//        std::cerr << "creating uniform buffer = " << mBufferId << std::endl;
    }
    
    GLES2HardwareUniformBuffer::~GLES2HardwareUniformBuffer()
    {
        OGRE_CHECK_GL_ERROR(glDeleteBuffers(1, &mBufferId));
    }

    void GLES2HardwareUniformBuffer::setGLBufferBinding(GLint binding)
    {
        mBinding = binding;

        // Attach the buffer to the UBO binding
        OGRE_CHECK_GL_ERROR(glBindBufferBase(GL_UNIFORM_BUFFER, mBinding, mBufferId));
    }

    void* GLES2HardwareUniformBuffer::lockImpl(size_t offset,
                                                 size_t length,
                                                 LockOptions options)
    {
        if (mIsLocked)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Invalid attempt to lock a uniform buffer that has already been locked",
                        "GLES2HardwareUniformBuffer::lock");
        }
        
        GLenum access = 0;
        void* retPtr = 0;
        
        // Use glMapBuffer
        OGRE_CHECK_GL_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, mBufferId));
        
        if (mUsage & HBU_WRITE_ONLY)
        {
            access |= GL_MAP_WRITE_BIT;
            access |= GL_MAP_FLUSH_EXPLICIT_BIT;
            if(options == HBL_DISCARD)
            {
                // Discard the buffer
                access |= GL_MAP_INVALIDATE_RANGE_BIT;
            }
        }
        else if (options == HBL_READ_ONLY)
            access |= GL_MAP_READ_BIT;
        else
            access |= GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;

        access |= GL_MAP_UNSYNCHRONIZED_BIT;
        
        void* pBuffer;
        OGRE_CHECK_GL_ERROR(pBuffer = glMapBufferRange(GL_UNIFORM_BUFFER, offset, length, access));
        
        if(pBuffer == 0)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
                        "Uniform Buffer: Out of memory",
                        "GLES2HardwareUniformBuffer::lock");
        }
        
        // return offsetted
        retPtr = static_cast<void*>(static_cast<unsigned char*>(pBuffer) + offset);

		mIsLocked = true;
        return retPtr;
    }
    
    void GLES2HardwareUniformBuffer::unlockImpl(void)
    {
        OGRE_CHECK_GL_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, mBufferId));
        
        if (mUsage & HBU_WRITE_ONLY)
        {
            OGRE_CHECK_GL_ERROR(glFlushMappedBufferRange(GL_UNIFORM_BUFFER, mLockStart, mLockSize));
        }

        GLboolean mapped;
        OGRE_CHECK_GL_ERROR(mapped = glUnmapBuffer(GL_UNIFORM_BUFFER));
        if(!mapped)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Buffer data corrupted, please reload", 
                        "GLES2HardwareUniformBuffer::unlock");
        }
        OGRE_CHECK_GL_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, 0));

        mIsLocked = false;
    }
    
    void GLES2HardwareUniformBuffer::readData(size_t offset, size_t length, void* pDest)
    {
        // Get data from the real buffer
        OGRE_CHECK_GL_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, mBufferId));

		OGRE_CHECK_GL_ERROR(glGetBufferPointerv(GL_UNIFORM_BUFFER, GL_BUFFER_MAP_POINTER, &pDest));
    }
    
    void GLES2HardwareUniformBuffer::writeData(size_t offset,
                                                 size_t length,
                                                 const void* pSource,
                                                 bool discardWholeBuffer)
    {
        OGRE_CHECK_GL_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, mBufferId));

        if (offset == 0 && length == mSizeInBytes)
        {
            OGRE_CHECK_GL_ERROR(glBufferData(GL_UNIFORM_BUFFER, mSizeInBytes, pSource,
                                             GLES2HardwareBufferManager::getGLUsage(mUsage)));
        }
        else
        {
            if(discardWholeBuffer)
            {
                OGRE_CHECK_GL_ERROR(glBufferData(GL_UNIFORM_BUFFER, mSizeInBytes, NULL,
                                                 GLES2HardwareBufferManager::getGLUsage(mUsage)));
            }
            
            OGRE_CHECK_GL_ERROR(glBufferSubData(GL_UNIFORM_BUFFER, offset, length, pSource));
        }
    }
    
    void GLES2HardwareUniformBuffer::copyData(HardwareBuffer& srcBuffer, size_t srcOffset, 
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
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, 0));

            // Zero out this(destination) buffer
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, mBufferId));
            OGRE_CHECK_GL_ERROR(glBufferData(GL_UNIFORM_BUFFER, length, 0, GLES2HardwareBufferManager::getGLUsage(mUsage)));
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, 0));

            // Do it the fast way.
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_COPY_READ_BUFFER, static_cast<GLES2HardwareUniformBuffer &>(srcBuffer).getGLBufferId()));
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_COPY_WRITE_BUFFER, mBufferId));

            OGRE_CHECK_GL_ERROR(glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, srcOffset, dstOffset, length));

            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_COPY_READ_BUFFER, 0));
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_COPY_WRITE_BUFFER, 0));
        }
    }
}
#endif
