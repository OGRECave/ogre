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

#include "OgreRenderSystem.h"
#include "OgreGL3PlusHardwareBufferManager.h"
#include "OgreGL3PlusHardwarePixelBuffer.h"
#include "OgreGL3PlusPixelFormat.h"
#include "OgreGL3PlusFBORenderTexture.h"
#include "OgreGL3PlusStateCacheManager.h"
#include "OgreGL3PlusRenderSystem.h"

#include "OgreRoot.h"
#include "OgreGLSLProgramManager.h"
#include "OgreGLSLMonolithicProgram.h"
#include "OgreGLSLSeparableProgram.h"

namespace Ogre {
    GL3PlusRenderBuffer::GL3PlusRenderBuffer(
        GLenum format, uint32 width, uint32 height, GLsizei numSamples)
        : GLHardwarePixelBufferCommon(
            width, height, 1,
            GL3PlusPixelUtil::getClosestOGREFormat(format), HBU_GPU_ONLY),
          mRenderbufferID(0)
    {
        mGLInternalFormat = format;
        // Generate renderbuffer
        OGRE_CHECK_GL_ERROR(glGenRenderbuffers(1, &mRenderbufferID));

        mRenderSystem = static_cast<GL3PlusRenderSystem*>(Root::getSingleton().getRenderSystem());
        // Bind it to FBO
        mRenderSystem->_getStateCacheManager()->bindGLRenderBuffer( mRenderbufferID );

        // Allocate storage for depth buffer
        if (numSamples > 0)
        {
            OGRE_CHECK_GL_ERROR(glRenderbufferStorageMultisample(GL_RENDERBUFFER,
                                                                 numSamples, format, width, height));
        }
        else
        {
            OGRE_CHECK_GL_ERROR(glRenderbufferStorage(GL_RENDERBUFFER, format,
                                                      width, height));
        }
    }

    GL3PlusRenderBuffer::~GL3PlusRenderBuffer()
    {
        // Delete renderbuffer
        if(GL3PlusStateCacheManager* stateCacheManager = mRenderSystem->_getStateCacheManager())
            stateCacheManager->deleteGLRenderBuffer(mRenderbufferID);
    }

    void GL3PlusRenderBuffer::bindToFramebuffer(uint32 attachment, uint32 zoffset)
    {
        assert(zoffset < mDepth);
        OGRE_CHECK_GL_ERROR(glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment,
                                                      GL_RENDERBUFFER, mRenderbufferID));
    }


}
