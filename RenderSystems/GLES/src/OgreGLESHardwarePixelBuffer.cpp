/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/

#include "OgreRenderSystem.h"
#include "OgreGLESRenderTexture.h"
#include "OgreGLESHardwarePixelBuffer.h"
#include "OgreGLESTexture.h"
#include "OgreGLESSupport.h"
#include "OgreGLESPixelFormat.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreBitwise.h"
#include "OgreRoot.h"

static int computeLog(GLuint value)
{
    int i;

    i = 0;

    /* Error! */
    if (value == 0) return -1;

    for (;;)
    {
        if (value & 1)
        {
            /* Error! */
            if (value != 1) return -1;
                return i;
        }
        value = value >> 1;
        i++;
    }
}

namespace Ogre {
    GLESHardwarePixelBuffer::GLESHardwarePixelBuffer(size_t mWidth, size_t mHeight,
                                                     size_t mDepth, PixelFormat mFormat,
                                                     HardwareBuffer::Usage usage)
        : HardwarePixelBuffer(mWidth, mHeight, mDepth, mFormat, usage, false, false),
          mBuffer(mWidth, mHeight, mDepth, mFormat),
          mGLInternalFormat(0)
    {
    }

    GLESHardwarePixelBuffer::~GLESHardwarePixelBuffer()
    {
        // Force free buffer
        delete [] (uint8*)mBuffer.data;
    }

    void GLESHardwarePixelBuffer::allocateBuffer()
    {
        if (mBuffer.data)
            // Already allocated
            return;

        mBuffer.data = new uint8[mSizeInBytes];
        // TODO use PBO if we're HBU_DYNAMIC
    }

    void GLESHardwarePixelBuffer::freeBuffer()
    {
        // Free buffer if we're STATIC to save memory
        if (mUsage & HBU_STATIC)
        {
            delete [] (uint8*)mBuffer.data;
            mBuffer.data = 0;
        }
    }

    PixelBox GLESHardwarePixelBuffer::lockImpl(const Image::Box lockBox,  LockOptions options)
    {
        allocateBuffer();
        if (options != HardwareBuffer::HBL_DISCARD && \
            (mUsage & HardwareBuffer::HBU_WRITE_ONLY) == 0)
        {
            // Download the old contents of the texture
            download(mBuffer);
        }
        mCurrentLockOptions = options;
        return mBuffer.getSubVolume(lockBox);
    }

    void GLESHardwarePixelBuffer::unlockImpl(void)
    {
        if (mCurrentLockOptions != HardwareBuffer::HBL_READ_ONLY)
        {
            // From buffer to card, only upload if was locked for writing
            upload(mCurrentLock);
        }
        freeBuffer();
    }

    void GLESHardwarePixelBuffer::blitFromMemory(const PixelBox &src, const Image::Box &dstBox)
    {
        if (!mBuffer.contains(dstBox))
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "destination box out of range",
                        "GLESHardwarePixelBuffer::blitFromMemory");
        }

        PixelBox scaled;

        if (src.getWidth() != dstBox.getWidth() ||
            src.getHeight() != dstBox.getHeight() ||
            src.getDepth() != dstBox.getDepth())
        {
            // Scale to destination size. Use DevIL and not iluScale because ILU screws up for 
            // floating point textures and cannot cope with 3D images.
            // This also does pixel format conversion if needed
            allocateBuffer();
            scaled = mBuffer.getSubVolume(dstBox);
            Image::scale(src, scaled, Image::FILTER_BILINEAR);
        }
        else if ((src.format != mFormat) ||
                 ((GLESPixelUtil::getGLOriginFormat(src.format) == 0) &&
                  (src.format != PF_R8G8B8)))
        {
            // Extents match, but format is not accepted as valid source format for GL
            // do conversion in temporary buffer
            allocateBuffer();
            scaled = mBuffer.getSubVolume(dstBox);

            PixelUtil::bulkPixelConversion(src, scaled);
        }
        else
        {
            scaled = src;
            if (src.format == PF_R8G8B8)
            {
                scaled.format = PF_B8G8R8;
                PixelUtil::bulkPixelConversion(src, scaled);
            }

            // No scaling or conversion needed
            // Set extents for upload
            scaled.left = dstBox.left;
            scaled.right = dstBox.right;
            scaled.top = dstBox.top;
            scaled.bottom = dstBox.bottom;
            scaled.front = dstBox.front;
            scaled.back = dstBox.back;
        }

        upload(scaled);
        freeBuffer();
    }

    void GLESHardwarePixelBuffer::blitToMemory(const Image::Box &srcBox, const PixelBox &dst)
    {
        if (!mBuffer.contains(srcBox))
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "source box out of range",
                        "GLESHardwarePixelBuffer::blitToMemory");
        }

        if (srcBox.left == 0 && srcBox.right == getWidth() &&
            srcBox.top == 0 && srcBox.bottom == getHeight() &&
            srcBox.front == 0 && srcBox.back == getDepth() &&
            dst.getWidth() == getWidth() &&
            dst.getHeight() == getHeight() &&
            dst.getDepth() == getDepth() &&
            GLESPixelUtil::getGLOriginFormat(dst.format) != 0)
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

    void GLESHardwarePixelBuffer::upload(const PixelBox &data)
    {
        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                    "Upload not possible for this pixelbuffer type",
                    "GLESHardwarePixelBuffer::upload");
    }

    void GLESHardwarePixelBuffer::download(const PixelBox &data)
    {
        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                    "Download not possible for this pixelbuffer type",
                    "GLESHardwarePixelBuffer::download");
    }

    void GLESHardwarePixelBuffer::bindToFramebuffer(GLenum attachment, size_t zoffset)
    {
        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                    "Framebuffer bind not possible for this pixelbuffer type",
                    "GLESHardwarePixelBuffer::bindToFramebuffer");
    }

    // TextureBuffer
    GLESTextureBuffer::GLESTextureBuffer(const String &baseName, GLuint id, 
                                         GLint width, GLint height,
                                         GLint format,
                                         GLint face, GLint level, Usage usage, bool crappyCard,
                                         bool writeGamma, uint fsaa)
        : GLESHardwarePixelBuffer(0, 0, 0, PF_UNKNOWN, usage),
          mTextureID(id), mFace(face), mLevel(level), mSoftwareMipmap(crappyCard)
    {
        GL_CHECK_ERROR;
        glBindTexture(GL_TEXTURE_2D, mTextureID);
        GL_CHECK_ERROR;

        // TODO verify who get this
        mWidth = width;
        mHeight = height;
        mDepth = 1;

        mGLInternalFormat = format;
        mFormat = GLESPixelUtil::getClosestOGREFormat(format);

        mRowPitch = mWidth;
        mSlicePitch = mHeight*mWidth;
        mSizeInBytes = PixelUtil::getMemorySize(mWidth, mHeight, mDepth, mFormat);
        mBuffer = PixelBox(mWidth, mHeight, mDepth, mFormat);

        if (mWidth==0 || mHeight==0 || mDepth==0)
            return;

        if (mUsage & TU_RENDERTARGET)
        {
            mSliceTRT.reserve(mDepth);
            for(size_t zoffset=0; zoffset<mDepth; ++zoffset)
            {
                String name;
                name = "rtt/" + StringConverter::toString((size_t)this) + "/" + baseName;

                GLESSurfaceDesc target;

                target.buffer = this;
                target.zoffset = zoffset;

                RenderTexture *trt =
                    GLESRTTManager::getSingleton().createRenderTexture(name,
                                                                       target,
                                                                       writeGamma,
                                                                       fsaa);
                mSliceTRT.push_back(trt);
                Root::getSingleton().getRenderSystem()->attachRenderTarget(*mSliceTRT[zoffset]);
            }
        }
    }

    GLESTextureBuffer::~GLESTextureBuffer()
    {
        if (mUsage & TU_RENDERTARGET)
        {
            // Delete all render targets that are not yet deleted via _clearSliceRTT because the rendertarget
            // was deleted by the user.
            for (SliceTRT::const_iterator it = mSliceTRT.begin(); it != mSliceTRT.end(); ++it)
            {
                Root::getSingleton().getRenderSystem()->destroyRenderTarget((*it)->getName());
            }
        }
    }

    void GLESTextureBuffer::upload(const PixelBox &data)
    {
        GLenum gl_format = GLESPixelUtil::getGLOriginFormat(data.format);

        glBindTexture(GL_TEXTURE_2D, mTextureID);
        GL_CHECK_ERROR;

        if (PixelUtil::isCompressed(data.format))
        {
            if(data.format != mFormat || !data.isConsecutive())
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                            "Compressed images must be consecutive, in the source format",
                            "GLESHardwarePixelBuffer::upload");

            GLenum format = GLESPixelUtil::getClosestGLInternalFormat(mFormat);
            // Data must be consecutive and at beginning of buffer as PixelStorei not allowed
            // for compressed formats
            if (data.left == 0 && data.top == 0)
            {
                glCompressedTexImage2D(GL_TEXTURE_2D, mLevel,
                                       format,
                                       data.getWidth(),
                                       data.getHeight(),
                                       0,
                                       data.getConsecutiveSize(),
                                       data.data);
                GL_CHECK_ERROR;
            }
            else
            {
                glCompressedTexSubImage2D(GL_TEXTURE_2D, mLevel,
                                          data.left, data.top,
                                          data.getWidth(), data.getHeight(),
                                          format, data.getConsecutiveSize(),
                                          data.data);

                GL_CHECK_ERROR;
            }
        }
        else if (mSoftwareMipmap)
        {
            if (data.getWidth() != data.rowPitch)
            {
                // TODO
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                            "Unsupported texture format",
                            "GLESHardwarePixelBuffer::upload");
            }

            if (data.getHeight() * data.getWidth() != data.slicePitch)
            {
                // TODO
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                            "Unsupported texture format",
                            "GLESHardwarePixelBuffer::upload");
            }

            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            buildMipmaps(data);
        }
        else
        {
            if(data.getWidth() != data.rowPitch)
            {
                // TODO
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                            "Unsupported texture format",
                            "GLESHardwarePixelBuffer::upload");
            }

            if(data.getHeight()*data.getWidth() != data.slicePitch)
            {
                // TODO
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                    "Unsupported texture format",
                    "GLESHardwarePixelBuffer::upload");
            }

            if ((data.getWidth() * PixelUtil::getNumElemBytes(data.format)) & 3) {
                // Standard alignment of 4 is not right
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                GL_CHECK_ERROR;
            }

			GL_CHECK_ERROR;
            glTexSubImage2D(GL_TEXTURE_2D,
                            mLevel,
                            data.left, data.top,
                            data.getWidth(), data.getHeight(),
                            gl_format,
                            GLESPixelUtil::getGLOriginDataType(data.format),
                            data.data);
            GL_CHECK_ERROR;
        }

        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        GL_CHECK_ERROR;
    }

    void GLESTextureBuffer::copyFromFramebuffer(size_t zoffset)
    {
        glBindTexture(GL_TEXTURE_2D, mTextureID);
        glCopyTexSubImage2D(GL_TEXTURE_2D, mLevel, 0, 0, 0, 0, mWidth, mHeight);
    }

    RenderTexture *GLESTextureBuffer::getRenderTarget(size_t zoffset)
    {
        assert(mUsage & TU_RENDERTARGET);
        assert(zoffset < mDepth);
        return mSliceTRT[zoffset];
    }

    void GLESTextureBuffer::buildMipmaps(const PixelBox &data)
    {
        int width;
        int height;
        int mip = 0;
        int logW;
        int logH;
        int level;
        PixelBox scaled = data;
        scaled.data = data.data;
        scaled.left = data.left;
        scaled.right = data.right;
        scaled.top = data.top;
        scaled.bottom = data.bottom;
        scaled.front = data.front;
        scaled.back = data.back;

        GLenum glFormat = GLESPixelUtil::getGLOriginFormat(data.format);
        GLenum dataType = GLESPixelUtil::getGLOriginDataType(data.format);
        width = data.getWidth();
        height = data.getHeight();

        logW = computeLog(width);
        logH = computeLog(height);
        level = (logW > logH ? logW : logH);

        for (int mip = 0; mip <= level; mip++)
        {
            glFormat = GLESPixelUtil::getGLOriginFormat(scaled.format);
            dataType = GLESPixelUtil::getGLOriginDataType(scaled.format);

            glTexImage2D(GL_TEXTURE_2D,
                         mip,
                         glFormat,
                         width, height,
                         0,
                         glFormat,
                         dataType,
                         scaled.data);

            GL_CHECK_ERROR;

            if (mip != 0)
            {
                delete[] (uint8*) scaled.data;
                scaled.data = 0;
            }

            if (width > 1)
            {
                width = width / 2;
            }

            if (height > 1)
            {
                height = height / 2;
            }

            int sizeInBytes = PixelUtil::getMemorySize(width, height, 1,
                                                       data.format);
            scaled = PixelBox(width, height, 1, data.format);
            scaled.data = new uint8[sizeInBytes];
            Image::scale(data, scaled, Image::FILTER_LINEAR);
        }
    }
}
