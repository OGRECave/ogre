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

#include "OgreGL3PlusHardwareBufferManager.h"
#include "OgreGL3PlusHardwareShaderStorageBuffer.h"
#include "OgreRoot.h"
#include "OgreGL3PlusRenderSystem.h"

namespace Ogre {
namespace v1 {
    GL3PlusHardwareShaderStorageBuffer::GL3PlusHardwareShaderStorageBuffer(
        HardwareBufferManagerBase* mgr,
        size_t bufferSize,
        HardwareBuffer::Usage usage,
        bool useShadowBuffer, const String& name)
        : HardwareUniformBuffer(mgr, bufferSize, usage, useShadowBuffer, name)
    {
        OGRE_CHECK_GL_ERROR(glGenBuffers(1, &mBufferId));

        if (!mBufferId)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Cannot create GL shader storage buffer",
                        "GL3PlusHardwareShaderStorageBuffer::GL3PlusHardwareShaderStorageBuffer");
        }

        OGRE_CHECK_GL_ERROR(glBindBuffer(GL_SHADER_STORAGE_BUFFER, mBufferId));
        OGRE_CHECK_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, mSizeInBytes, NULL,
                                         GL3PlusHardwareBufferManager::getGLUsage(usage)));

        //        std::cerr << "creating shader storage buffer = " << mBufferId << std::endl;
    }

    GL3PlusHardwareShaderStorageBuffer::~GL3PlusHardwareShaderStorageBuffer()
    {
        OGRE_CHECK_GL_ERROR(glDeleteBuffers(1, &mBufferId));
    }

    void GL3PlusHardwareShaderStorageBuffer::setGLBufferBinding(GLint binding)
    {
        mBinding = binding;

        // Attach the buffer to the UBO binding
        OGRE_CHECK_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, mBinding, mBufferId));
    }

    void* GL3PlusHardwareShaderStorageBuffer::lockImpl(size_t offset,
                                                       size_t length,
                                                       LockOptions options)
    {
        if (mIsLocked)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Invalid attempt to lock a shader storage buffer that has already been locked",
                        "GL3PlusHardwareShaderStorageBuffer::lock");
        }

        GLenum access = 0;
        void* retPtr = 0;

        // Use glMapBuffer
        OGRE_CHECK_GL_ERROR(glBindBuffer(GL_SHADER_STORAGE_BUFFER, mBufferId));

        assert( ((mUsage & HBU_WRITE_ONLY && options != HBL_NORMAL && options != HBL_READ_ONLY) ||
                !(mUsage & HBU_WRITE_ONLY)) &&
                "Reading from a write-only buffer! Create the buffer without HBL_WRITE_ONLY bit" );

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

        //FIXME Is this correct usage for shader storage buffers?
        if( options == HBL_NO_OVERWRITE )
            access |= GL_MAP_UNSYNCHRONIZED_BIT;

        void* pBuffer = 0;
        OGRE_CHECK_GL_ERROR(pBuffer = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, offset, length, access));

        if(pBuffer == 0)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Shader Storage Buffer: Out of memory",
                        "GL3PlusHardwareShaderStorageBuffer::lock");
        }

        // pBuffer is already offsetted in glMapBufferRange
        retPtr = pBuffer;

        mIsLocked = true;
        return retPtr;
    }

    void GL3PlusHardwareShaderStorageBuffer::unlockImpl(void)
    {
        OGRE_CHECK_GL_ERROR(glBindBuffer(GL_SHADER_STORAGE_BUFFER, mBufferId));

        if (mUsage & HBU_WRITE_ONLY)
        {
            OGRE_CHECK_GL_ERROR(glFlushMappedBufferRange(GL_SHADER_STORAGE_BUFFER, mLockStart, mLockSize));
        }

        GLboolean mapped;
        OGRE_CHECK_GL_ERROR(mapped = glUnmapBuffer(GL_SHADER_STORAGE_BUFFER));
        if(!mapped)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Buffer data corrupted, please reload",
                        "GL3PlusHardwareShaderStorageBuffer::unlock");
        }
        OGRE_CHECK_GL_ERROR(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));

        mIsLocked = false;
    }

    void GL3PlusHardwareShaderStorageBuffer::readData(size_t offset, size_t length, void* pDest)
    {
        // Get data from the real buffer
        OGRE_CHECK_GL_ERROR(glBindBuffer(GL_SHADER_STORAGE_BUFFER, mBufferId));

        //FIXME May not be implemented for GL_SHADER_STORAGE_BUFFER?
        OGRE_CHECK_GL_ERROR(glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, length, pDest));
    }

    void GL3PlusHardwareShaderStorageBuffer::writeData(size_t offset,
                                                       size_t length,
                                                       const void* pSource,
                                                       bool discardWholeBuffer)
    {
        OGRE_CHECK_GL_ERROR(glBindBuffer(GL_SHADER_STORAGE_BUFFER, mBufferId));

        if (offset == 0 && length == mSizeInBytes)
        {
            OGRE_CHECK_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, mSizeInBytes, pSource,
                                             GL3PlusHardwareBufferManager::getGLUsage(mUsage)));
        }
        else
        {
            if(discardWholeBuffer)
            {
                OGRE_CHECK_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, mSizeInBytes, NULL,
                                                 GL3PlusHardwareBufferManager::getGLUsage(mUsage)));
            }

            OGRE_CHECK_GL_ERROR(glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, length, pSource));
        }
    }

    void GL3PlusHardwareShaderStorageBuffer::copyData(HardwareBuffer& srcBuffer, size_t srcOffset,
                                                      size_t dstOffset, size_t length, bool discardWholeBuffer)
    {
        if (srcBuffer.isSystemMemory())
        {
            HardwareBuffer::copyData(srcBuffer, srcOffset, dstOffset, length, discardWholeBuffer);
        }
        else
        {
            // Unbind the current buffer
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));

            // Zero out this(destination) buffer
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_SHADER_STORAGE_BUFFER, mBufferId));
            OGRE_CHECK_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, length, 0, GL3PlusHardwareBufferManager::getGLUsage(mUsage)));
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));

            // Do it the fast way.
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_COPY_READ_BUFFER, static_cast<GL3PlusHardwareShaderStorageBuffer &>(srcBuffer).getGLBufferId()));
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_COPY_WRITE_BUFFER, mBufferId));

            //FIXME Perhaps not implemented for shader storage buffers?
            OGRE_CHECK_GL_ERROR(glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, srcOffset, dstOffset, length));

            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_COPY_READ_BUFFER, 0));
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_COPY_WRITE_BUFFER, 0));
        }
    }
}
}
