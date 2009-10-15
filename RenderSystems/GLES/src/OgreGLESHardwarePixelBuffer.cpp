/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
Copyright (c) 2000-2009 Torus Knot Software Ltd

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
#include "OgreGLESRenderTexture.h"
#include "OgreGLESHardwarePixelBuffer.h"
#include "OgreGLESPixelFormat.h"
#include "OgreGLESFBORenderTexture.h"
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
        OGRE_DELETE [] (uint8*)mBuffer.data;
    }

    void GLESHardwarePixelBuffer::allocateBuffer()
    {
        if (mBuffer.data)
            // Already allocated
            return;

        mBuffer.data = OGRE_NEW uint8[mSizeInBytes];
        // TODO use PBO if we're HBU_DYNAMIC
    }

    void GLESHardwarePixelBuffer::freeBuffer()
    {
        // Free buffer if we're STATIC to save memory
        if (mUsage & HBU_STATIC)
        {
            OGRE_DELETE [] (uint8*)mBuffer.data;
            mBuffer.data = 0;
        }
    }

    PixelBox GLESHardwarePixelBuffer::lockImpl(const Image::Box lockBox,  LockOptions options)
    {
        allocateBuffer();
        if (options != HardwareBuffer::HBL_DISCARD &&
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
            upload(mCurrentLock, Box(0, 0, 0, mWidth, mHeight, mDepth));
        }
        freeBuffer();
    }

    void GLESHardwarePixelBuffer::blitFromMemory(const PixelBox &src, const Image::Box &dstBox)
    {
        if (!mBuffer.contains(dstBox))
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Destination box out of range",
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
                 ((GLESPixelUtil::getGLOriginFormat(src.format) == 0) && (src.format != PF_R8G8B8)))
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

        upload(scaled, dstBox);
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

    void GLESHardwarePixelBuffer::upload(const PixelBox &data, const Image::Box &dest)
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
    GLESTextureBuffer::GLESTextureBuffer(const String &baseName, GLenum target, GLuint id, 
                                         GLint width, GLint height, GLint format, GLint face, 
                                         GLint level, Usage usage, bool crappyCard, 
                                         bool writeGamma, uint fsaa)
    : GLESHardwarePixelBuffer(0, 0, 0, PF_UNKNOWN, usage),
        mTarget(target), mTextureID(id), mFace(face), mLevel(level), mSoftwareMipmap(crappyCard)
    {
        GL_CHECK_ERROR;
        glBindTexture(GL_TEXTURE_2D, mTextureID);
        GL_CHECK_ERROR;

        // Get face identifier
        mFaceTarget = mTarget;

        // TODO verify who get this
        mWidth = width;
        mHeight = height;
        mDepth = 1;

        mGLInternalFormat = format;
        mFormat = GLESPixelUtil::getClosestOGREFormat(format);

        mRowPitch = mWidth;
        mSlicePitch = mHeight*mWidth;
        mSizeInBytes = PixelUtil::getMemorySize(mWidth, mHeight, mDepth, mFormat);

        // Log a message
//        std::stringstream str;
//        str << "GLESHardwarePixelBuffer constructed for texture " << mTextureID 
//            << " face " << mFace << " level " << mLevel << ":"
//            << " width=" << mWidth << " height="<< mHeight << " depth=" << mDepth
//            << " format=" << PixelUtil::getFormatName(mFormat);
//        LogManager::getSingleton().logMessage( 
//                    LML_NORMAL, str.str());

        // Set up a pixel box
        mBuffer = PixelBox(mWidth, mHeight, mDepth, mFormat);
        
        if (mWidth==0 || mHeight==0 || mDepth==0)
            /// We are invalid, do not allocate a buffer
            return;

        // Is this a render target?
        if (mUsage & TU_RENDERTARGET)
        {
            // Create render target for each slice
            mSliceTRT.reserve(mDepth);
            for(size_t zoffset=0; zoffset<mDepth; ++zoffset)
            {
                String name;
                name = "rtt/" + StringConverter::toString((size_t)this) + "/" + baseName;
                GLESSurfaceDesc target;
                target.buffer = this;
                target.zoffset = zoffset;
                RenderTexture *trt = GLESRTTManager::getSingleton().createRenderTexture(name, target, writeGamma, fsaa);
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

    void GLESTextureBuffer::upload(const PixelBox &data, const Image::Box &dest)
    {
        glBindTexture(mTarget, mTextureID);
        GL_CHECK_ERROR;

        if (PixelUtil::isCompressed(data.format))
        {
            if(data.format != mFormat || !data.isConsecutive())
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                            "Compressed images must be consecutive, in the source format",
                            "GLESTextureBuffer::upload");

            GLenum format = GLESPixelUtil::getClosestGLInternalFormat(mFormat);
            // Data must be consecutive and at beginning of buffer as PixelStorei not allowed
            // for compressed formats
            if (dest.left == 0 && dest.top == 0)
            {
                glCompressedTexImage2D(mFaceTarget, mLevel,
                                       format,
                                       dest.getWidth(),
                                       dest.getHeight(),
                                       0,
                                       data.getConsecutiveSize(),
                                       data.data);
                GL_CHECK_ERROR;
            }
            else
            {
                glCompressedTexSubImage2D(mFaceTarget, mLevel,
                                          dest.left, dest.top,
                                          dest.getWidth(), dest.getHeight(),
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
            GL_CHECK_ERROR;
            buildMipmaps(data);
        }
        else
        {
            if(data.getWidth() != data.rowPitch)
            {
                // TODO
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                            "Unsupported texture format",
                            "GLESTextureBuffer::upload");
            }

            if(data.getHeight()*data.getWidth() != data.slicePitch)
            {
                // TODO
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                    "Unsupported texture format",
                    "GLESTextureBuffer::upload");
            }

            if ((data.getWidth() * PixelUtil::getNumElemBytes(data.format)) & 3) {
                // Standard alignment of 4 is not right
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                GL_CHECK_ERROR;
            }

            glTexSubImage2D(mFaceTarget,
                            mLevel,
                            dest.left, dest.top,
                            dest.getWidth(), dest.getHeight(),
                            GLESPixelUtil::getGLOriginFormat(data.format),
                            GLESPixelUtil::getGLOriginDataType(data.format),
                            data.data);
            GL_CHECK_ERROR;
        }

        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        GL_CHECK_ERROR;
    }

    //-----------------------------------------------------------------------------  
    void GLESTextureBuffer::download(const PixelBox &data)
    {
        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, 
                    "Downloading texture buffers is not supported by OpenGL ES",
                    "GLESTextureBuffer::download");
    }
    //-----------------------------------------------------------------------------  
    void GLESTextureBuffer::bindToFramebuffer(GLenum attachment, size_t zoffset)
    {
        assert(zoffset < mDepth);
        glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, attachment,
                                  mFaceTarget, mTextureID, mLevel);
        GL_CHECK_ERROR;
    }
    
    void GLESTextureBuffer::copyFromFramebuffer(size_t zoffset)
    {
        glBindTexture(GL_TEXTURE_2D, mTextureID);
        GL_CHECK_ERROR;
        glCopyTexSubImage2D(GL_TEXTURE_2D, mLevel, 0, 0, 0, 0, mWidth, mHeight);
        GL_CHECK_ERROR;
    }

    //-----------------------------------------------------------------------------  
    void GLESTextureBuffer::blit(const HardwarePixelBufferSharedPtr &src, const Image::Box &srcBox, const Image::Box &dstBox)
    {
        GLESTextureBuffer *srct = static_cast<GLESTextureBuffer *>(src.getPointer());
        /// TODO: Check for FBO support first
        /// Destination texture must be 2D
        /// Source texture must be 2D
        if(srct->mTarget == GL_TEXTURE_2D)
        {
            blitFromTexture(srct, srcBox, dstBox);
        }
        else
        {
            GLESHardwarePixelBuffer::blit(src, srcBox, dstBox);
        }
    }
    
    //-----------------------------------------------------------------------------  
    /// Very fast texture-to-texture blitter and hardware bi/trilinear scaling implementation using FBO
    /// Destination texture must be 1D, 2D, 3D, or Cube
    /// Source texture must be 1D, 2D or 3D
    /// Supports compressed formats as both source and destination format, it will use the hardware DXT compressor
    /// if available.
    /// @author W.J. van der Laan
    void GLESTextureBuffer::blitFromTexture(GLESTextureBuffer *src, const Image::Box &srcBox, const Image::Box &dstBox)
    {
//        std::cerr << "GLESTextureBuffer::blitFromTexture " <<
//        src->mTextureID << ":" << srcBox.left << "," << srcBox.top << "," << srcBox.right << "," << srcBox.bottom << " " << 
//        mTextureID << ":" << dstBox.left << "," << dstBox.top << "," << dstBox.right << "," << dstBox.bottom << std::endl;

        /// Store reference to FBO manager
        GLESFBOManager *fboMan = static_cast<GLESFBOManager *>(GLESRTTManager::getSingletonPtr());
        
        /// Save and clear GL state for rendering
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |  GL_STENCIL_BUFFER_BIT);
        
        /// Disable alpha, depth and scissor testing, disable blending, 
        /// disable culling, disble lighting, disable fog and reset foreground
        /// colour.
        glDisable(GL_ALPHA_TEST);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_SCISSOR_TEST);
        glDisable(GL_BLEND);
        glDisable(GL_CULL_FACE);
        glDisable(GL_LIGHTING);
        glDisable(GL_FOG);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        GL_CHECK_ERROR;

        /// Save and reset matrices
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glMatrixMode(GL_TEXTURE);
        glPushMatrix();
        glLoadIdentity();
        GL_CHECK_ERROR;
        
        /// Set up source texture
        glBindTexture(src->mTarget, src->mTextureID);
        GL_CHECK_ERROR;
        
        /// Set filtering modes depending on the dimensions and source
        if(srcBox.getWidth()==dstBox.getWidth() &&
           srcBox.getHeight()==dstBox.getHeight() &&
           srcBox.getDepth()==dstBox.getDepth())
        {
            /// Dimensions match -- use nearest filtering (fastest and pixel correct)
            glTexParameteri(src->mTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            GL_CHECK_ERROR;
            glTexParameteri(src->mTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            GL_CHECK_ERROR;
        }
        else
        {
            /// Dimensions don't match -- use bi or trilinear filtering depending on the
            /// source texture.
            if(src->mUsage & TU_AUTOMIPMAP)
            {
                /// Automatic mipmaps, we can safely use trilinear filter which
                /// brings greatly imporoved quality for minimisation.
                glTexParameteri(src->mTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                GL_CHECK_ERROR;
                glTexParameteri(src->mTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);    
                GL_CHECK_ERROR;
            }
            else
            {
                /// Manual mipmaps, stay safe with bilinear filtering so that no
                /// intermipmap leakage occurs.
                glTexParameteri(src->mTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                GL_CHECK_ERROR;
                glTexParameteri(src->mTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                GL_CHECK_ERROR;
            }
        }
        /// Clamp to edge (fastest)
        glTexParameteri(src->mTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        GL_CHECK_ERROR;
        glTexParameteri(src->mTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        GL_CHECK_ERROR;
        
        /// Store old binding so it can be restored later
        GLint oldfb;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING_OES, &oldfb);
        GL_CHECK_ERROR;

        /// Set up temporary FBO
        glBindFramebufferOES(GL_FRAMEBUFFER_OES, fboMan->getTemporaryFBO());
        GL_CHECK_ERROR;

        GLuint tempTex = 0;
        if(!fboMan->checkFormat(mFormat))
        {
            /// If target format not directly supported, create intermediate texture
            GLenum tempFormat = GLESPixelUtil::getClosestGLInternalFormat(fboMan->getSupportedAlternative(mFormat));
            glGenTextures(1, &tempTex);
            GL_CHECK_ERROR;
            glBindTexture(GL_TEXTURE_2D, tempTex);
            GL_CHECK_ERROR;
            /// Allocate temporary texture of the size of the destination area
            glTexImage2D(GL_TEXTURE_2D, 0, tempFormat, 
                         GLESPixelUtil::optionalPO2(dstBox.getWidth()), GLESPixelUtil::optionalPO2(dstBox.getHeight()), 
                         0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
            GL_CHECK_ERROR;
            glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES,
                                      GL_TEXTURE_2D, tempTex, 0);
            GL_CHECK_ERROR;
            /// Set viewport to size of destination slice
            glViewport(0, 0, dstBox.getWidth(), dstBox.getHeight());
            GL_CHECK_ERROR;
        }
        else
        {
            /// We are going to bind directly, so set viewport to size and position of destination slice
            glViewport(dstBox.left, dstBox.top, dstBox.getWidth(), dstBox.getHeight());
            GL_CHECK_ERROR;
        }
        
        /// Process each destination slice
        for(size_t slice=dstBox.front; slice<dstBox.back; ++slice)
        {
            if(!tempTex)
            {
                /// Bind directly
                bindToFramebuffer(GL_COLOR_ATTACHMENT0_OES, slice);
            }
            
            /// Finally we're ready to rumble
            glBindTexture(src->mTarget, src->mTextureID);
            GL_CHECK_ERROR;
            glEnable(src->mTarget);
            GL_CHECK_ERROR;

            glDisable(src->mTarget);
            GL_CHECK_ERROR;

            if(tempTex)
            {
                /// Copy temporary texture
                glBindTexture(mTarget, mTextureID);
                GL_CHECK_ERROR;
                switch(mTarget)
                {
                    case GL_TEXTURE_2D:
                        glCopyTexSubImage2D(mFaceTarget, mLevel, 
                                            dstBox.left, dstBox.top, 
                                            0, 0, dstBox.getWidth(), dstBox.getHeight());
                        GL_CHECK_ERROR;
                        break;
                }
            }
        }
        /// Finish up 
        if(!tempTex)
        {
            /// Generate mipmaps
            if(mUsage & TU_AUTOMIPMAP)
            {
                glBindTexture(mTarget, mTextureID);
                GL_CHECK_ERROR;
                glGenerateMipmapOES(mTarget);
                GL_CHECK_ERROR;
            }
        }
        
        /// Reset source texture to sane state
        glBindTexture(src->mTarget, src->mTextureID);
        GL_CHECK_ERROR;
        
        /// Detach texture from temporary framebuffer
        glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES,
                                     GL_RENDERBUFFER_OES, 0);
        GL_CHECK_ERROR;
        /// Restore old framebuffer
        glBindFramebufferOES(GL_FRAMEBUFFER_OES, oldfb);
        GL_CHECK_ERROR;
        /// Restore matrix stacks and render state
        glMatrixMode(GL_TEXTURE);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        GL_CHECK_ERROR;
        glDeleteTextures(1, &tempTex);
        GL_CHECK_ERROR;
    }
    //-----------------------------------------------------------------------------  
    /// blitFromMemory doing hardware trilinear scaling
    void GLESTextureBuffer::blitFromMemory(const PixelBox &src_orig, const Image::Box &dstBox)
    {
        /// Fall back to normal GLHardwarePixelBuffer::blitFromMemory in case 
        /// - FBO is not supported
        /// - Either source or target is luminance due doesn't looks like supported by hardware
        /// - the source dimensions match the destination ones, in which case no scaling is needed
        // TODO: Check that extension is NOT available
        if(//!GLEW_EXT_framebuffer_object ||
           PixelUtil::isLuminance(src_orig.format) ||
           PixelUtil::isLuminance(mFormat) ||
           (src_orig.getWidth() == dstBox.getWidth() &&
            src_orig.getHeight() == dstBox.getHeight() &&
            src_orig.getDepth() == dstBox.getDepth()))
        {
            GLESHardwarePixelBuffer::blitFromMemory(src_orig, dstBox);
            return;
        }
        if(!mBuffer.contains(dstBox))
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Destination box out of range",
                        "GLESTextureBuffer::blitFromMemory");
        /// For scoped deletion of conversion buffer
        MemoryDataStreamPtr buf;
        PixelBox src;
        
        /// First, convert the srcbox to a OpenGL compatible pixel format
        if(GLESPixelUtil::getGLOriginFormat(src_orig.format) == 0)
        {
            /// Convert to buffer internal format
            buf.bind(OGRE_NEW MemoryDataStream(PixelUtil::getMemorySize(src_orig.getWidth(), src_orig.getHeight(), src_orig.getDepth(),
                                                                   mFormat)));
            src = PixelBox(src_orig.getWidth(), src_orig.getHeight(), src_orig.getDepth(), mFormat, buf->getPtr());
            PixelUtil::bulkPixelConversion(src_orig, src);
        }
        else
        {
            /// No conversion needed
            src = src_orig;
        }
        
        /// Create temporary texture to store source data
        GLuint id;
        GLenum target = GL_TEXTURE_2D;
        GLsizei width = GLESPixelUtil::optionalPO2(src.getWidth());
        GLsizei height = GLESPixelUtil::optionalPO2(src.getHeight());
        GLenum format = GLESPixelUtil::getClosestGLInternalFormat(src.format);
        
        /// Generate texture name
        glGenTextures(1, &id);
        GL_CHECK_ERROR;
        
        /// Set texture type
        glBindTexture(target, id);
        GL_CHECK_ERROR;
        
        /// Set automatic mipmap generation; nice for minimisation
        glTexParameteri(target, GL_GENERATE_MIPMAP, GL_TRUE );
        GL_CHECK_ERROR;

        /// Allocate texture memory
        glTexImage2D(target, 0, format, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        GL_CHECK_ERROR;

        /// GL texture buffer
        GLESTextureBuffer tex(StringUtil::BLANK, target, id, width, height, format,
                              0, 0, (Usage)(TU_AUTOMIPMAP|HBU_STATIC_WRITE_ONLY), false, false, 0);
        
        /// Upload data to 0,0,0 in temporary texture
        Image::Box tempTarget(0, 0, 0, src.getWidth(), src.getHeight(), src.getDepth());
        tex.upload(src, tempTarget);
        
        /// Blit
        blitFromTexture(&tex, tempTarget, dstBox);
        
        /// Delete temp texture
        glDeleteTextures(1, &id);
        GL_CHECK_ERROR;
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
                OGRE_DELETE[] (uint8*) scaled.data;
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
            scaled.data = OGRE_NEW uint8[sizeInBytes];
            Image::scale(data, scaled, Image::FILTER_LINEAR);
        }
    }
    
    //********* GLESRenderBuffer
    //----------------------------------------------------------------------------- 
    GLESRenderBuffer::GLESRenderBuffer(GLenum format, size_t width, size_t height, GLsizei numSamples):
    GLESHardwarePixelBuffer(width, height, 1, GLESPixelUtil::getClosestOGREFormat(format),HBU_WRITE_ONLY)
    {
        mGLInternalFormat = format;
        /// Generate renderbuffer
        glGenRenderbuffersOES(1, &mRenderbufferID);
        GL_CHECK_ERROR;
        /// Bind it to FBO
        glBindRenderbufferOES(GL_RENDERBUFFER_OES, mRenderbufferID);
        GL_CHECK_ERROR;
        
        /// Allocate storage for depth buffer
        if (numSamples > 0)
        {
//            glRenderbufferStorageMultisampleOES(GL_RENDERBUFFER_OES, 
//                                                numSamples, format, width, height);
        }
        else
        {
            glRenderbufferStorageOES(GL_RENDERBUFFER_OES, format,
                                     width, height);
            GL_CHECK_ERROR;
        }
    }
    //----------------------------------------------------------------------------- 
    GLESRenderBuffer::~GLESRenderBuffer()
    {
        /// Generate renderbuffer
        glDeleteRenderbuffersOES(1, &mRenderbufferID);
        GL_CHECK_ERROR;
    }
    //-----------------------------------------------------------------------------  
    void GLESRenderBuffer::bindToFramebuffer(GLenum attachment, size_t zoffset)
    {
        assert(zoffset < mDepth);
        glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, attachment,
                                     GL_RENDERBUFFER_OES, mRenderbufferID);
        GL_CHECK_ERROR;
    }
};
