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

#include "OgreRoot.h"
#include "OgreGLSLMonolithicProgramManager.h"
#include "OgreGLSLMonolithicProgram.h"
#include "OgreGLSLSeparableProgramManager.h"
#include "OgreGLSLSeparableProgram.h"

namespace Ogre {
namespace v1 {
    GL3PlusHardwarePixelBuffer::GL3PlusHardwarePixelBuffer(uint32 inWidth, uint32 inHeight,
                                                           uint32 inDepth, PixelFormat inFormat,
                                                           bool hwGamma,
                                                           HardwareBuffer::Usage usage)
        : HardwarePixelBuffer(inWidth, inHeight, inDepth, inFormat, hwGamma, usage, false, false),
          mBuffer(inWidth, inHeight, inDepth, inFormat),
          mGLInternalFormat(GL_NONE)
    {
        mCurrentLockOptions = (LockOptions)0;
    }

    GL3PlusHardwarePixelBuffer::~GL3PlusHardwarePixelBuffer()
    {
        // Force free buffer
        uint8 *p = reinterpret_cast<uint8*>( mBuffer.data );
        delete [] p;
        mBuffer.data = 0;
    }

    void GL3PlusHardwarePixelBuffer::allocateBuffer( size_t bytes )
    {
        if (mBuffer.data)
            // Already allocated
            return;

        mBuffer.data = new uint8[bytes];
    }

    void GL3PlusHardwarePixelBuffer::freeBuffer()
    {
        // Free buffer if we're STATIC to save memory
        if (mUsage & HBU_STATIC)
        {
            uint8 *p = reinterpret_cast<uint8*>( mBuffer.data );
            delete [] p;
            mBuffer.data = 0;
        }
    }

    PixelBox GL3PlusHardwarePixelBuffer::lockImpl( const Image::Box &lockBox, LockOptions options )
    {
        //Allocate memory for the entire image, as the buffer
        //maynot be freed and be reused in subsequent calls.
        allocateBuffer( PixelUtil::getMemorySize( mWidth, mHeight, mDepth, mFormat ) );

        mBuffer = PixelBox( lockBox.getWidth(), lockBox.getHeight(),
                            lockBox.getDepth(), mFormat, mBuffer.data );
        mCurrentLock = mBuffer;
        mCurrentLock.left    = lockBox.left;
        mCurrentLock.right  += lockBox.left;
        mCurrentLock.top     = lockBox.top;
        mCurrentLock.bottom += lockBox.top;

        if(options != HardwareBuffer::HBL_DISCARD)
        {
            // Download the old contents of the texture
            download( mCurrentLock );
        }
        mCurrentLockOptions = options;
        mLockedBox = lockBox;
        mCurrentLock = mBuffer;

        return mBuffer;
    }

    void GL3PlusHardwarePixelBuffer::unlockImpl(void)
    {
        if (mCurrentLockOptions != HardwareBuffer::HBL_READ_ONLY)
        {
            // From buffer to card, only upload if was locked for writing.
            upload(mCurrentLock, mLockedBox);
        }
        freeBuffer();

        mBuffer = PixelBox( mWidth, mHeight, mDepth, mFormat, mBuffer.data );
    }

    void GL3PlusHardwarePixelBuffer::blitFromMemory(const PixelBox &src, const Image::Box &dstBox)
    {
        if (!mBuffer.contains(dstBox))
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Destination box out of range",
                        "GL3PlusHardwarePixelBuffer::blitFromMemory");
        }

        PixelBox scaled;

        if (src.getWidth() != dstBox.getWidth() ||
            src.getHeight() != dstBox.getHeight() ||
            src.getDepth() != dstBox.getDepth())
        {
            // Scale to destination size.
            // This also does pixel format conversion if needed.
            allocateBuffer( mSizeInBytes );
            scaled = mBuffer.getSubVolume(dstBox);
            Image::scale(src, scaled, Image::FILTER_BILINEAR);
        }
        else if (GL3PlusPixelUtil::getGLOriginFormat(src.format) == 0)
        {
            // Extents match, but format is not accepted as valid
            // source format for GL. Do conversion in temporary buffer.
            allocateBuffer( mSizeInBytes );
            scaled = mBuffer.getSubVolume(dstBox);
            PixelUtil::bulkPixelConversion(src, scaled);
        }
        else
        {
            allocateBuffer( mSizeInBytes );
            // No scaling or conversion needed.
            scaled = src;
        }

        upload(scaled, dstBox);
        freeBuffer();
    }

    void GL3PlusHardwarePixelBuffer::blitToMemory(const Image::Box &srcBox, const PixelBox &dst)
    {
        if (!mBuffer.contains(srcBox))
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "source box out of range",
                        "GL3PlusHardwarePixelBuffer::blitToMemory");
        }

        if (srcBox.left == 0 && srcBox.right == getWidth() &&
            srcBox.top == 0 && srcBox.bottom == getHeight() &&
            srcBox.front == 0 && srcBox.back == getDepth() &&
            dst.getWidth() == getWidth() &&
            dst.getHeight() == getHeight() &&
            dst.getDepth() == getDepth() &&
            GL3PlusPixelUtil::getGLOriginFormat(dst.format) != 0)
        {
            // The direct case: the user wants the entire texture in a format supported by GL
            // so we don't need an intermediate buffer
            download(dst);
        }
        else
        {
            // Use buffer for intermediate copy
            allocateBuffer( mSizeInBytes );
            // Download entire buffer
            download(mBuffer);
            if(srcBox.getWidth() != dst.getWidth() ||
               srcBox.getHeight() != dst.getHeight() ||
               srcBox.getDepth() != dst.getDepth())
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

    void GL3PlusHardwarePixelBuffer::upload(const PixelBox &data, const Image::Box &dest)
    {
        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                    "Upload not possible for this pixelbuffer type",
                    "GL3PlusHardwarePixelBuffer::upload");
    }

    void GL3PlusHardwarePixelBuffer::download(const PixelBox &data)
    {
        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                    "Download not possible for this pixelbuffer type",
                    "GL3PlusHardwarePixelBuffer::download");
    }

    void GL3PlusHardwarePixelBuffer::bindToFramebuffer(GLenum attachment, uint32 zoffset)
    {
        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                    "Framebuffer bind not possible for this pixelbuffer type",
                    "GL3PlusHardwarePixelBuffer::bindToFramebuffer");
    }

    //********* GL3PlusRenderBuffer
    GL3PlusRenderBuffer::GL3PlusRenderBuffer(
        GLenum format, uint32 width, uint32 height, GLsizei numSamples)
        : GL3PlusHardwarePixelBuffer(
            width, height, 1,
            GL3PlusPixelUtil::getClosestOGREFormat(format), false, HBU_WRITE_ONLY),
          mRenderbufferID(0)
    {
        mGLInternalFormat = format;
        // Generate renderbuffer
        OGRE_CHECK_GL_ERROR(glGenRenderbuffers(1, &mRenderbufferID));

        // Bind it to FBO
        OGRE_CHECK_GL_ERROR(glBindRenderbuffer(GL_RENDERBUFFER, mRenderbufferID));

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
        // Generate renderbuffer
        OGRE_CHECK_GL_ERROR(glDeleteRenderbuffers(1, &mRenderbufferID));
    }

    void GL3PlusRenderBuffer::bindToFramebuffer(GLenum attachment, uint32 zoffset)
    {
        assert(zoffset < mDepth);
        OGRE_CHECK_GL_ERROR(glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment,
                                                      GL_RENDERBUFFER, mRenderbufferID));
    }

}
}
