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

#include "OgreGL3PlusHardwareBufferManager.h"
#include "OgreGL3PlusHardwareUniformBuffer.h"
#include "OgreRoot.h"
#include "OgreGL3PlusRenderSystem.h"

namespace Ogre {
    GL3PlusHardwareUniformBuffer::GL3PlusHardwareUniformBuffer(HardwareBufferManagerBase* mgr, 
                                                               size_t bufferSize,
                                                               HardwareBuffer::Usage usage,
                                                               bool useShadowBuffer, const String& name)
    : HardwareUniformBuffer(mgr, bufferSize, usage, useShadowBuffer, name)
    {
        glGenBuffers(1, &mBufferId);
        GL_CHECK_ERROR

        if (!mBufferId)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Cannot create GL uniform buffer",
                        "GL3PlusHardwareUniformBuffer::GL3PlusHardwareUniformBuffer");
        }

        glBindBuffer(GL_UNIFORM_BUFFER, mBufferId);
        GL_CHECK_ERROR
        glBufferData(GL_UNIFORM_BUFFER, mSizeInBytes, NULL,
                     GL3PlusHardwareBufferManager::getGLUsage(usage));
        GL_CHECK_ERROR

        std::cerr << "creating uniform buffer = " << mBufferId << std::endl;
    }
    
    GL3PlusHardwareUniformBuffer::~GL3PlusHardwareUniformBuffer()
    {
        glDeleteBuffers(1, &mBufferId);
        GL_CHECK_ERROR
    }

    void GL3PlusHardwareUniformBuffer::setGLBufferBinding(GLint binding)
    {
        mBinding = binding;

        // Attach the buffer to the UBO binding
        glBindBufferBase(GL_UNIFORM_BUFFER, mBinding, mBufferId);
        GL_CHECK_ERROR
    }

    void* GL3PlusHardwareUniformBuffer::lockImpl(size_t offset,
                                                 size_t length,
                                                 LockOptions options)
    {
        if (mIsLocked)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Invalid attempt to lock a uniform buffer that has already been locked",
                        "GL3PlusHardwareUniformBuffer::lock");
        }
        
        GLenum access = 0;
        void* retPtr = 0;
        
        // Use glMapBuffer
        glBindBuffer(GL_UNIFORM_BUFFER, mBufferId);
        GL_CHECK_ERROR
        
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
        
        void* pBuffer = glMapBufferRange(GL_UNIFORM_BUFFER, offset, length, access);
        GL_CHECK_ERROR
        
        if(pBuffer == 0)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
                        "Uniform Buffer: Out of memory",
                        "GL3PlusHardwareUniformBuffer::lock");
        }
        
        // return offsetted
        retPtr = static_cast<void*>(static_cast<unsigned char*>(pBuffer) + offset);

		mIsLocked = true;
        return retPtr;
    }
    
    void GL3PlusHardwareUniformBuffer::unlockImpl(void)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, mBufferId);
        GL_CHECK_ERROR
        
        if (mUsage & HBU_WRITE_ONLY)
        {
            glFlushMappedBufferRange(GL_UNIFORM_BUFFER, mLockStart, mLockSize);
            GL_CHECK_ERROR
        }
        
        if(!glUnmapBuffer(GL_UNIFORM_BUFFER))
        {
            GL_CHECK_ERROR
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
                        "Buffer data corrupted, please reload", 
                        "GL3PlusHardwareUniformBuffer::unlock");
        }
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        GL_CHECK_ERROR

        mIsLocked = false;
    }
    
    void GL3PlusHardwareUniformBuffer::readData(size_t offset, size_t length, void* pDest)
    {
        // get data from the real buffer
        glBindBuffer(GL_UNIFORM_BUFFER, mBufferId);
        GL_CHECK_ERROR
        
        glGetBufferSubData(GL_UNIFORM_BUFFER, offset, length, pDest);
        GL_CHECK_ERROR
    }
    
    void GL3PlusHardwareUniformBuffer::writeData(size_t offset,
                                                 size_t length,
                                                 const void* pSource,
                                                 bool discardWholeBuffer)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, mBufferId);
        GL_CHECK_ERROR

        if (offset == 0 && length == mSizeInBytes)
        {
            glBufferData(GL_UNIFORM_BUFFER, mSizeInBytes, pSource,
                         GL3PlusHardwareBufferManager::getGLUsage(mUsage));
            GL_CHECK_ERROR
        }
        else
        {
            if(discardWholeBuffer)
            {
                glBufferData(GL_UNIFORM_BUFFER, mSizeInBytes, NULL, 
                             GL3PlusHardwareBufferManager::getGLUsage(mUsage));
                GL_CHECK_ERROR
            }
            
            glBufferSubData(GL_UNIFORM_BUFFER, offset, length, pSource);
            GL_CHECK_ERROR
        }
    }
    
    void GL3PlusHardwareUniformBuffer::copyData(HardwareBuffer& srcBuffer, size_t srcOffset, 
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
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
            GL_CHECK_ERROR
            
            // Zero out this(destination) buffer
            glBindBuffer(GL_UNIFORM_BUFFER, mBufferId);
            GL_CHECK_ERROR
            glBufferData(GL_UNIFORM_BUFFER, length, 0, GL3PlusHardwareBufferManager::getGLUsage(mUsage));
            GL_CHECK_ERROR
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
            GL_CHECK_ERROR
            
            // Do it the fast way.
            glBindBuffer(GL_COPY_READ_BUFFER, static_cast<GL3PlusHardwareUniformBuffer &>(srcBuffer).getGLBufferId());
            GL_CHECK_ERROR
            glBindBuffer(GL_COPY_WRITE_BUFFER, mBufferId);
            GL_CHECK_ERROR
            
            glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, srcOffset, dstOffset, length);
            GL_CHECK_ERROR
            
            glBindBuffer(GL_COPY_READ_BUFFER, 0);
            GL_CHECK_ERROR
            glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
            GL_CHECK_ERROR
        }
    }
}
