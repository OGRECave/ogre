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

    GL3PlusHardwarePixelBuffer::GL3PlusHardwarePixelBuffer(uint32 inWidth, uint32 inHeight,
                                                           uint32 inDepth, PixelFormat inFormat,
                                                           HardwareBuffer::Usage usage)
        : GLHardwarePixelBufferCommon(inWidth, inHeight, inDepth, inFormat, usage)
    {
        mRenderSystem = static_cast<GL3PlusRenderSystem*>(Root::getSingleton().getRenderSystem());
    }

    void GL3PlusHardwarePixelBuffer::blitFromMemory(const PixelBox &src, const Box &dstBox)
    {
        if (!mBuffer.contains(dstBox))
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Destination box out of range",
                        "GL3PlusHardwarePixelBuffer::blitFromMemory");
        }

        PixelBox scaled;

        if (src.getSize() != dstBox.getSize())
        {
            // Scale to destination size.
            // This also does pixel format conversion if needed.
            allocateBuffer();
            scaled = mBuffer.getSubVolume(dstBox);
            Image::scale(src, scaled, Image::FILTER_BILINEAR);
        }
        else if (GL3PlusPixelUtil::getGLInternalFormat(src.format) == 0)
        {
            // Extents match, but format is not accepted as valid
            // source format for GL. Do conversion in temporary buffer.
            allocateBuffer();
            scaled = mBuffer.getSubVolume(dstBox);
            PixelUtil::bulkPixelConversion(src, scaled);
        }
        else
        {
            // No scaling or conversion needed.
            scaled = src;
        }

        upload(scaled, dstBox);
        freeBuffer();
    }

    void GL3PlusHardwarePixelBuffer::blitToMemory(const Box &srcBox, const PixelBox &dst)
    {
        if (!mBuffer.contains(srcBox))
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "source box out of range",
                        "GL3PlusHardwarePixelBuffer::blitToMemory");
        }

        if (srcBox.getOrigin() == Vector3i(0, 0 ,0) &&
            srcBox.getSize() == getSize() &&
            dst.getSize() == getSize() &&
            GL3PlusPixelUtil::getGLInternalFormat(dst.format) != 0)
        {
            // The direct case: the user wants the entire texture in a format supported by GL
            // so we don't need an intermediate buffer
            download(dst);
        }
        else
        {
            // Use buffer for intermediate copy
            allocateBuffer();
            // Download entire buffer
            download(mBuffer);
            if(srcBox.getSize() != dst.getSize())
            {
                // We need scaling
                Image::scale(mBuffer.getSubVolume(srcBox), dst, Image::FILTER_BILINEAR);
            }
            else
            {
                // Just copy the bit that we need
                PixelUtil::bulkPixelConversion(mBuffer.getSubVolume(srcBox), dst);
            }
            freeBuffer();
        }
    }

    //********* GL3PlusRenderBuffer
    GL3PlusRenderBuffer::GL3PlusRenderBuffer(
        GLenum format, uint32 width, uint32 height, GLsizei numSamples)
        : GL3PlusHardwarePixelBuffer(
            width, height, 1,
            GL3PlusPixelUtil::getClosestOGREFormat(format), HBU_GPU_ONLY),
          mRenderbufferID(0)
    {
        mGLInternalFormat = format;
        // Generate renderbuffer
        OGRE_CHECK_GL_ERROR(glGenRenderbuffers(1, &mRenderbufferID));

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
