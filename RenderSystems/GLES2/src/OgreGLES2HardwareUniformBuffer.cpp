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

#include "OgreGLES2HardwareUniformBuffer.h"
#include "OgreRoot.h"
#include "OgreGLES2RenderSystem.h"
#include "OgreGLUtil.h"
#include "OgreGLNativeSupport.h"

namespace Ogre {
    GLES2HardwareUniformBuffer::GLES2HardwareUniformBuffer(HardwareBufferManagerBase* mgr, 
                                                               size_t bufferSize,
                                                               HardwareBuffer::Usage usage,
                                                               bool useShadowBuffer, const String& name)
    : HardwareUniformBuffer(mgr, bufferSize, usage, useShadowBuffer, name),
      mBuffer(GL_UNIFORM_BUFFER, bufferSize, usage),
      mBinding(0)
    {
    }
    
    void GLES2HardwareUniformBuffer::setGLBufferBinding(GLint binding)
    {
        mBinding = binding;

        // Attach the buffer to the UBO binding
        OGRE_CHECK_GL_ERROR(glBindBufferBase(GL_UNIFORM_BUFFER, mBinding, getGLBufferId()));
    }
    
    void GLES2HardwareUniformBuffer::readData(size_t offset, size_t length, void* pDest)
    {
        mBuffer.readData(offset, length, pDest);
    }
    
    void GLES2HardwareUniformBuffer::writeData(size_t offset,
                                                 size_t length,
                                                 const void* pSource,
                                                 bool discardWholeBuffer)
    {
        mBuffer.writeData(offset, length, pSource, discardWholeBuffer);
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
            mBuffer.copyData(static_cast<GLES2HardwareUniformBuffer&>(srcBuffer).getGLBufferId(),
                                srcOffset, dstOffset, length, discardWholeBuffer);
        }
    }
}
