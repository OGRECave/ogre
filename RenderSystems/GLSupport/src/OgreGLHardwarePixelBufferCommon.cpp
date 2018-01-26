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
#include "OgreGLHardwarePixelBufferCommon.h"

namespace Ogre
{
GLHardwarePixelBufferCommon::GLHardwarePixelBufferCommon(uint32 inWidth, uint32 inHeight,
                                                       uint32 inDepth, PixelFormat inFormat,
                                                       HardwareBuffer::Usage usage)
    : HardwarePixelBuffer(inWidth, inHeight, inDepth, inFormat, usage, false, false),
      mBuffer(inWidth, inHeight, inDepth, inFormat),
      mGLInternalFormat(0)
{
    mCurrentLockOptions = (LockOptions)0;
}

GLHardwarePixelBufferCommon::~GLHardwarePixelBufferCommon()
{
    // Force free buffer
    delete[] mBuffer.data;
}

void GLHardwarePixelBufferCommon::allocateBuffer()
{
    if (mBuffer.data)
        // Already allocated
        return;

    mBuffer.data = new uint8[mSizeInBytes];
}

void GLHardwarePixelBufferCommon::freeBuffer()
{
    // Free buffer if we're STATIC to save memory
    if (mUsage & HBU_STATIC)
    {
        delete[] mBuffer.data;
        mBuffer.data = 0;
    }
}

PixelBox GLHardwarePixelBufferCommon::lockImpl(const Box& lockBox, LockOptions options)
{
    allocateBuffer();
    if (!((mUsage & HBU_WRITE_ONLY) || (options == HBL_DISCARD) || (options == HBL_WRITE_ONLY)))
    {
        // Download the old contents of the texture
        download(mBuffer);
    }
    mCurrentLockOptions = options;
    mLockedBox = lockBox;
    return mBuffer.getSubVolume(lockBox);
}

void GLHardwarePixelBufferCommon::unlockImpl(void)
{
    if (mCurrentLockOptions != HBL_READ_ONLY)
    {
        // From buffer to card, only upload if was locked for writing.
        upload(mCurrentLock, mLockedBox);
    }
    freeBuffer();
}

void GLHardwarePixelBufferCommon::upload(const PixelBox &data, const Box &dest)
{
    OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                "Upload not possible for this pixelbuffer type",
                "GLHardwarePixelBufferCommon::upload");
}

void GLHardwarePixelBufferCommon::download(const PixelBox &data)
{
    OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                "Download not possible for this pixelbuffer type",
                "GLHardwarePixelBufferCommon::download");
}

void GLHardwarePixelBufferCommon::bindToFramebuffer(uint32 attachment, uint32 zoffset)
{
    OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                "Framebuffer bind not possible for this pixelbuffer type",
                "GLHardwarePixelBufferCommon::bindToFramebuffer");
}

} /* namespace Ogre */
