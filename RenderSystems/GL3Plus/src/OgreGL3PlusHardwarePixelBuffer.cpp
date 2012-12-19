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

#include "OgreRenderSystem.h"
#include "OgreGL3PlusHardwarePixelBuffer.h"
#include "OgreGL3PlusPixelFormat.h"
#include "OgreGL3PlusFBORenderTexture.h"
#include "OgreGL3PlusGpuProgram.h"
#include "OgreRoot.h"
#include "OgreGLSLLinkProgramManager.h"
#include "OgreGLSLLinkProgram.h"
#include "OgreGLSLProgramPipelineManager.h"
#include "OgreGLSLProgramPipeline.h"

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
    GL3PlusHardwarePixelBuffer::GL3PlusHardwarePixelBuffer(size_t inWidth, size_t inHeight,
                                                   size_t inDepth, PixelFormat inFormat,
                                                   HardwareBuffer::Usage usage)
    : HardwarePixelBuffer(inWidth, inHeight, inDepth, inFormat, usage, false, false),
    mBuffer(inWidth, inHeight, inDepth, inFormat),
    mGLInternalFormat(GL_NONE)
    {
        mCurrentLockOptions = (LockOptions)0;
    }
    
    GL3PlusHardwarePixelBuffer::~GL3PlusHardwarePixelBuffer()
    {
        // Force free buffer
        delete [] (uint8*)mBuffer.data;
    }
    
    void GL3PlusHardwarePixelBuffer::allocateBuffer()
    {
        if (mBuffer.data)
            // Already allocated
            return;
        
        mBuffer.data = new uint8[mSizeInBytes];
        // TODO: use PBO if we're HBU_DYNAMIC
    }
    
    void GL3PlusHardwarePixelBuffer::freeBuffer()
    {
        // Free buffer if we're STATIC to save memory
        if (mUsage & HBU_STATIC)
        {
            delete [] (uint8*)mBuffer.data;
            mBuffer.data = 0;
        }
    }
    
    PixelBox GL3PlusHardwarePixelBuffer::lockImpl(const Image::Box lockBox,  LockOptions options)
    {
        allocateBuffer();
        if(options != HardwareBuffer::HBL_DISCARD) 
        {
            // Download the old contents of the texture
            download(mBuffer);
        }
        mCurrentLockOptions = options;
        mLockedBox = lockBox;
        return mBuffer.getSubVolume(lockBox);
    }
    
    void GL3PlusHardwarePixelBuffer::unlockImpl(void)
    {
        if (mCurrentLockOptions != HardwareBuffer::HBL_READ_ONLY)
        {
            // From buffer to card, only upload if was locked for writing
            upload(mCurrentLock, mLockedBox);
        }
        freeBuffer();
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
            // This also does pixel format conversion if needed
            allocateBuffer();
            scaled = mBuffer.getSubVolume(dstBox);
            Image::scale(src, scaled, Image::FILTER_BILINEAR);
        }
        else if(GL3PlusPixelUtil::getGLOriginFormat(src.format) == 0)
        {
            // Extents match, but format is not accepted as valid source format for GL
            // do conversion in temporary buffer
            allocateBuffer();
            scaled = mBuffer.getSubVolume(dstBox);
            PixelUtil::bulkPixelConversion(src, scaled);
        }
        else
        {
            allocateBuffer();
            // No scaling or conversion needed
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
    
    void GL3PlusHardwarePixelBuffer::bindToFramebuffer(GLenum attachment, size_t zoffset)
    {
        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                    "Framebuffer bind not possible for this pixelbuffer type",
                    "GL3PlusHardwarePixelBuffer::bindToFramebuffer");
    }
    
    // TextureBuffer
    GL3PlusTextureBuffer::GL3PlusTextureBuffer(const String &baseName, GLenum target, GLuint id, 
                                       GLint face, GLint level, Usage usage, bool crappyCard, 
                                       bool writeGamma, uint fsaa)
    : GL3PlusHardwarePixelBuffer(0, 0, 0, PF_UNKNOWN, usage),
    mTarget(target), mTextureID(id), mFace(face), mLevel(level), mSoftwareMipmap(crappyCard), mSliceTRT(0)
    {
        // devise mWidth, mHeight and mDepth and mFormat
        GLint value = 0;
        
        glBindTexture(mTarget, mTextureID);
        GL_CHECK_ERROR

        // Get face identifier
        mFaceTarget = mTarget;
        if(mTarget == GL_TEXTURE_CUBE_MAP)
            mFaceTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face;
        
        // Get width
        glGetTexLevelParameteriv(mFaceTarget, level, GL_TEXTURE_WIDTH, &value);
        GL_CHECK_ERROR
        mWidth = value;
        
        // Get height
        if(target == GL_TEXTURE_1D)
            value = 1;	// Height always 1 for 1D textures
        else
            glGetTexLevelParameteriv(mFaceTarget, level, GL_TEXTURE_HEIGHT, &value);
        mHeight = value;
        GL_CHECK_ERROR
        
        // Get depth
        if(target != GL_TEXTURE_3D && target !=  GL_TEXTURE_2D_ARRAY)
            value = 1; // Depth always 1 for non-3D textures
        else
            glGetTexLevelParameteriv(mFaceTarget, level, GL_TEXTURE_DEPTH, &value);
        mDepth = value;
        GL_CHECK_ERROR
        
        // Get format
        glGetTexLevelParameteriv(mFaceTarget, level, GL_TEXTURE_INTERNAL_FORMAT, &value);
        GL_CHECK_ERROR
        mGLInternalFormat = value;
        mFormat = GL3PlusPixelUtil::getClosestOGREFormat(value);
        
        // Default
        mRowPitch = mWidth;
        mSlicePitch = mHeight*mWidth;
        mSizeInBytes = PixelUtil::getMemorySize(mWidth, mHeight, mDepth, mFormat);
        
        // Log a message
//        std::stringstream str;
//        str << "GL3PlusHardwarePixelBuffer constructed for texture " << mTextureID 
//            << " face " << mFace << " level " << mLevel << ": "
//            << "width=" << mWidth << " height="<< mHeight << " depth=" << mDepth
//            << " format=" << PixelUtil::getFormatName(mFormat) << "(internal 0x"
//		<< std::hex << value << ")";
//        LogManager::getSingleton().logMessage(LML_NORMAL, str.str());
        
        // Set up a pixel box
        mBuffer = PixelBox(mWidth, mHeight, mDepth, mFormat);
        
        if (mWidth==0 || mHeight==0 || mDepth==0)
            // We are invalid, do not allocate a buffer
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
                GL3PlusSurfaceDesc surface;
                surface.buffer = this;
                surface.zoffset = zoffset;
                RenderTexture *trt = GL3PlusRTTManager::getSingleton().createRenderTexture(name, surface, writeGamma, fsaa);
                mSliceTRT.push_back(trt);
                Root::getSingleton().getRenderSystem()->attachRenderTarget(*mSliceTRT[zoffset]);
            }
        }
    }
    
    GL3PlusTextureBuffer::~GL3PlusTextureBuffer()
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
    
    void GL3PlusTextureBuffer::upload(const PixelBox &data, const Image::Box &dest)
    {
        glBindTexture(mTarget, mTextureID);
        GL_CHECK_ERROR

        if (PixelUtil::isCompressed(data.format))
        {
            if(data.format != mFormat || !data.isConsecutive())
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                            "Compressed images must be consecutive, in the source format",
                            "GL3PlusTextureBuffer::upload");
            
            GLenum format = GL3PlusPixelUtil::getClosestGLInternalFormat(mFormat);
            // Data must be consecutive and at beginning of buffer as PixelStorei not allowed
            // for compressed formats
            switch(mTarget) {
                case GL_TEXTURE_1D:
                    // some systems (e.g. old Apple) don't like compressed subimage calls
                    // so prefer non-sub versions
                    if (dest.left == 0)
                    {
                        glCompressedTexImage1D(GL_TEXTURE_1D, mLevel,
                                               format,
                                               dest.getWidth(),
                                               0,
                                               data.getConsecutiveSize(),
                                               data.data);
                    }
                    else
                    {
                        glCompressedTexSubImage1D(GL_TEXTURE_1D, mLevel, 
                                                  dest.left,
                                                  dest.getWidth(),
                                                  format, data.getConsecutiveSize(),
                                                  data.data);
                    }
                    break;
                case GL_TEXTURE_2D:
                case GL_TEXTURE_CUBE_MAP:
                case GL_TEXTURE_RECTANGLE:
                    // some systems (e.g. old Apple) don't like compressed subimage calls
                    // so prefer non-sub versions
                    if (dest.left == 0 && dest.top == 0)
                    {
                        glCompressedTexImage2D(mFaceTarget, mLevel,
                                               format,
                                               dest.getWidth(),
                                               dest.getHeight(),
                                               0,
                                               data.getConsecutiveSize(),
                                               data.data);
                    }
                    else
                    {
                        glCompressedTexSubImage2D(mFaceTarget, mLevel,
                                                  dest.left, dest.top,
                                                  dest.getWidth(), dest.getHeight(),
                                                  format, data.getConsecutiveSize(),
                                                  data.data);
                    }
                    break;
                case GL_TEXTURE_3D:
                case GL_TEXTURE_2D_ARRAY:
                    // some systems (e.g. old Apple) don't like compressed subimage calls
                    // so prefer non-sub versions
//                    if (dest.left == 0 && dest.top == 0 && dest.front == 0)
//                    {
//                        glCompressedTexImage3D(mTarget, mLevel,
//                                               format,
//                                               dest.getWidth(),
//                                               dest.getHeight(),
//                                               dest.getDepth(),
//                                               0,
//                                               data.getConsecutiveSize(),
//                                               data.data);
//                    }
//                    else
//                    {			
                        glCompressedTexSubImage3D(mTarget, mLevel, 
                                                  dest.left, dest.top, dest.front,
                                                  dest.getWidth(), dest.getHeight(), dest.getDepth(),
                                                  format, data.getConsecutiveSize(),
                                                  data.data);
//                    }
                    break;
            }
            GL_CHECK_ERROR

        } 
        else if(mSoftwareMipmap)
        {
            if(data.getWidth() != data.rowPitch)
                glPixelStorei(GL_UNPACK_ROW_LENGTH, data.rowPitch);
            if(data.getHeight()*data.getWidth() != data.slicePitch)
                glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, (data.slicePitch/data.getWidth()));
            if(data.left > 0 || data.top > 0 || data.front > 0)
                glPixelStorei(GL_UNPACK_SKIP_PIXELS, data.left + data.rowPitch * data.top + data.slicePitch * data.front);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            GL_CHECK_ERROR

            buildMipmaps(data);
        } 
        else
        {
            if(data.getWidth() != data.rowPitch)
                glPixelStorei(GL_UNPACK_ROW_LENGTH, data.rowPitch);
            if(data.getHeight()*data.getWidth() != data.slicePitch)
                glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, (data.slicePitch/data.getWidth()));
            if(data.left > 0 || data.top > 0 || data.front > 0)
                glPixelStorei(GL_UNPACK_SKIP_PIXELS, data.left + data.rowPitch * data.top + data.slicePitch * data.front);
            if((data.getWidth()*PixelUtil::getNumElemBytes(data.format)) & 3) {
                // Standard alignment of 4 is not right
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            }
            GL_CHECK_ERROR
            switch(mTarget) {
                case GL_TEXTURE_1D:
                    glTexSubImage1D(GL_TEXTURE_1D, mLevel, 
                                    dest.left,
                                    dest.getWidth(),
                                    GL3PlusPixelUtil::getGLOriginFormat(data.format), GL3PlusPixelUtil::getGLOriginDataType(data.format),
                                    data.data);
                    break;
                case GL_TEXTURE_2D:
                case GL_TEXTURE_CUBE_MAP:
                case GL_TEXTURE_RECTANGLE:
                    glTexSubImage2D(mFaceTarget, mLevel, 
                                    dest.left, dest.top,
                                    dest.getWidth(), dest.getHeight(),
                                    GL3PlusPixelUtil::getGLOriginFormat(data.format), GL3PlusPixelUtil::getGLOriginDataType(data.format),
                                    data.data);
                    break;
                case GL_TEXTURE_3D:
                case GL_TEXTURE_2D_ARRAY:
                    glTexSubImage3D(
                                    mTarget, mLevel, 
                                    dest.left, dest.top, dest.front,
                                    dest.getWidth(), dest.getHeight(), dest.getDepth(),
                                    GL3PlusPixelUtil::getGLOriginFormat(data.format), GL3PlusPixelUtil::getGLOriginDataType(data.format),
                                    data.data);
                    break;
            }	
            GL_CHECK_ERROR

            if (mUsage & TU_AUTOMIPMAP)
            {
                glGenerateMipmap(mTarget);
                GL_CHECK_ERROR
            }
        }
        // Restore defaults
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
        glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    }
    
    //-----------------------------------------------------------------------------  
    void GL3PlusTextureBuffer::download(const PixelBox &data)
    {
        if(data.getWidth() != getWidth() ||
           data.getHeight() != getHeight() ||
           data.getDepth() != getDepth())
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "only download of entire buffer is supported by GL",
                        "GL3PlusTextureBuffer::download");
        glBindTexture( mTarget, mTextureID );
        GL_CHECK_ERROR
        if(PixelUtil::isCompressed(data.format))
        {
            if(data.format != mFormat || !data.isConsecutive())
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                            "Compressed images must be consecutive, in the source format",
                            "GL3PlusTextureBuffer::download");
            // Data must be consecutive and at beginning of buffer as PixelStorei not allowed
            // for compressed formate
            glGetCompressedTexImage(mFaceTarget, mLevel, data.data);
            GL_CHECK_ERROR
        }
        else
        {
            if(data.getWidth() != data.rowPitch)
                glPixelStorei(GL_PACK_ROW_LENGTH, data.rowPitch);
            if(data.getHeight()*data.getWidth() != data.slicePitch)
                glPixelStorei(GL_PACK_IMAGE_HEIGHT, (data.slicePitch/data.getWidth()));
            if(data.left > 0 || data.top > 0 || data.front > 0)
                glPixelStorei(GL_PACK_SKIP_PIXELS, data.left + data.rowPitch * data.top + data.slicePitch * data.front);
            if((data.getWidth()*PixelUtil::getNumElemBytes(data.format)) & 3) {
                // Standard alignment of 4 is not right
                glPixelStorei(GL_PACK_ALIGNMENT, 1);
            }
            GL_CHECK_ERROR
            // We can only get the entire texture
            glGetTexImage(mFaceTarget, mLevel, 
                          GL3PlusPixelUtil::getGLOriginFormat(data.format), GL3PlusPixelUtil::getGLOriginDataType(data.format),
                          data.data);
            GL_CHECK_ERROR
            // Restore defaults
            glPixelStorei(GL_PACK_ROW_LENGTH, 0);
            glPixelStorei(GL_PACK_IMAGE_HEIGHT, 0);
            glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
            glPixelStorei(GL_PACK_ALIGNMENT, 4);
        }
    }
    //-----------------------------------------------------------------------------  
    void GL3PlusTextureBuffer::bindToFramebuffer(GLenum attachment, size_t zoffset)
    {
        assert(zoffset < mDepth);
        switch(mTarget)
        {
            case GL_TEXTURE_1D:
                glFramebufferTexture1D(GL_FRAMEBUFFER, attachment,
                                       mFaceTarget, mTextureID, mLevel);
                break;
            case GL_TEXTURE_2D:
            case GL_TEXTURE_CUBE_MAP:
            case GL_TEXTURE_RECTANGLE:
                if(mFormat == PF_DEPTH)
                    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                           mFaceTarget, mTextureID, mLevel);
                else
                    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                           mFaceTarget, mTextureID, mLevel);
                break;
            case GL_TEXTURE_3D:
            case GL_TEXTURE_2D_ARRAY:
                glFramebufferTexture3D(GL_FRAMEBUFFER, attachment,
                                       mFaceTarget, mTextureID, mLevel, zoffset);
                break;
        }
        GL_CHECK_ERROR
    }
    //-----------------------------------------------------------------------------
    void GL3PlusTextureBuffer::copyFromFramebuffer(size_t zoffset)
    {
        glBindTexture(mTarget, mTextureID);
        GL_CHECK_ERROR
        switch(mTarget)
        {
            case GL_TEXTURE_1D:
                glCopyTexSubImage1D(mFaceTarget, mLevel, 0, 0, 0, mWidth);
                break;
            case GL_TEXTURE_2D:
            case GL_TEXTURE_CUBE_MAP:
            case GL_TEXTURE_RECTANGLE:
                glCopyTexSubImage2D(mFaceTarget, mLevel, 0, 0, 0, 0, mWidth, mHeight);
                break;
            case GL_TEXTURE_3D:
            case GL_TEXTURE_2D_ARRAY:
                glCopyTexSubImage3D(mFaceTarget, mLevel, 0, 0, zoffset, 0, 0, mWidth, mHeight);
                break;
        }
        GL_CHECK_ERROR
    }
    //-----------------------------------------------------------------------------  
    void GL3PlusTextureBuffer::blit(const HardwarePixelBufferSharedPtr &src, const Image::Box &srcBox, const Image::Box &dstBox)
    {
        GL3PlusTextureBuffer *srct = static_cast<GL3PlusTextureBuffer *>(src.getPointer());
        /// Check for FBO support first
        /// Destination texture must be 1D, 2D, 3D, or Cube
        /// Source texture must be 1D, 2D or 3D
        
        // This does not sem to work for RTTs after the first update
        // I have no idea why! For the moment, disable 
        if((src->getUsage() & TU_RENDERTARGET) == 0 &&
           (srct->mTarget == GL_TEXTURE_1D || srct->mTarget == GL_TEXTURE_2D
            || srct->mTarget == GL_TEXTURE_RECTANGLE || srct->mTarget == GL_TEXTURE_3D)
            && mTarget != GL_TEXTURE_2D_ARRAY)
        {
            blitFromTexture(srct, srcBox, dstBox);
        }
        else
        {
            GL3PlusHardwarePixelBuffer::blit(src, srcBox, dstBox);
        }
    }
    
    //-----------------------------------------------------------------------------  
    // Very fast texture-to-texture blitter and hardware bi/trilinear scaling implementation using FBO
    // Destination texture must be 1D, 2D, 3D, or Cube
    // Source texture must be 1D, 2D or 3D
    // Supports compressed formats as both source and destination format, it will use the hardware DXT compressor
    // if available.
    // @author W.J. van der Laan
    void GL3PlusTextureBuffer::blitFromTexture(GL3PlusTextureBuffer *src, const Image::Box &srcBox, const Image::Box &dstBox)
    {
        //std::cerr << "GL3PlusTextureBuffer::blitFromTexture " <<
        //src->mTextureID << ":" << srcBox.left << "," << srcBox.top << "," << srcBox.right << "," << srcBox.bottom << " " << 
        //mTextureID << ":" << dstBox.left << "," << dstBox.top << "," << dstBox.right << "," << dstBox.bottom << std::endl;
        /// Store reference to FBO manager
        GL3PlusFBOManager *fboMan = static_cast<GL3PlusFBOManager *>(GL3PlusRTTManager::getSingletonPtr());
        
        RenderSystem* rsys = Root::getSingleton().getRenderSystem();
        rsys->_disableTextureUnitsFrom(0);
		glActiveTexture(GL_TEXTURE0);
        GL_CHECK_ERROR

        /// Disable alpha, depth and scissor testing, disable blending, 
        /// disable culling, disble lighting, disable fog and reset foreground
        /// colour.
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_SCISSOR_TEST);
        glDisable(GL_BLEND);
        glDisable(GL_CULL_FACE);
        
        GL_CHECK_ERROR

        // Set up source texture
        glBindTexture(src->mTarget, src->mTextureID);
        GL_CHECK_ERROR

        // Set filtering modes depending on the dimensions and source
        if(srcBox.getWidth()==dstBox.getWidth() &&
           srcBox.getHeight()==dstBox.getHeight() &&
           srcBox.getDepth()==dstBox.getDepth())
        {
            // Dimensions match -- use nearest filtering (fastest and pixel correct)
            glTexParameteri(src->mTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            GL_CHECK_ERROR
            glTexParameteri(src->mTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            GL_CHECK_ERROR
        }
        else
        {
            // Dimensions don't match -- use bi or trilinear filtering depending on the
            // source texture.
            if(src->mUsage & TU_AUTOMIPMAP)
            {
                // Automatic mipmaps, we can safely use trilinear filter which
                // brings greatly improved quality for minimisation.
                glTexParameteri(src->mTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                GL_CHECK_ERROR
                glTexParameteri(src->mTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);    
                GL_CHECK_ERROR
            }
            else
            {
                // Manual mipmaps, stay safe with bilinear filtering so that no
                // intermipmap leakage occurs.
                glTexParameteri(src->mTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                GL_CHECK_ERROR
                glTexParameteri(src->mTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                GL_CHECK_ERROR
            }
        }
        // Clamp to edge (fastest)
        glTexParameteri(src->mTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        GL_CHECK_ERROR
        glTexParameteri(src->mTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        GL_CHECK_ERROR
        glTexParameteri(src->mTarget, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        GL_CHECK_ERROR
        
        // Set origin base level mipmap to make sure we source from the right mip
        // level.
        glTexParameteri(src->mTarget, GL_TEXTURE_BASE_LEVEL, src->mLevel);
        GL_CHECK_ERROR
        
        // Store old binding so it can be restored later
        GLint oldfb;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldfb);
        GL_CHECK_ERROR
        
        // Set up temporary FBO
        glBindFramebuffer(GL_FRAMEBUFFER, fboMan->getTemporaryFBO());
        GL_CHECK_ERROR
        
        GLuint tempTex = 0;
        if(!fboMan->checkFormat(mFormat))
        {
            // If target format not directly supported, create intermediate texture
            GLenum tempFormat = GL3PlusPixelUtil::getClosestGLInternalFormat(fboMan->getSupportedAlternative(mFormat));
            glGenTextures(1, &tempTex);
            GL_CHECK_ERROR
            glBindTexture(GL_TEXTURE_2D, tempTex);
            GL_CHECK_ERROR
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
            GL_CHECK_ERROR
            
            // Allocate temporary texture of the size of the destination area
            glTexImage2D(GL_TEXTURE_2D, 0, tempFormat, 
                         dstBox.getWidth(), dstBox.getHeight(),
                         0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
            GL_CHECK_ERROR
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_2D, tempTex, 0);
            GL_CHECK_ERROR
            
            // Set viewport to size of destination slice
            glViewport(0, 0, dstBox.getWidth(), dstBox.getHeight());
            GL_CHECK_ERROR
        }
        else
        {
            // We are going to bind directly, so set viewport to size and position of destination slice
            glViewport(dstBox.left, dstBox.top, dstBox.getWidth(), dstBox.getHeight());
            GL_CHECK_ERROR
        }
        
        // Process each destination slice
        for(size_t slice=dstBox.front; slice<dstBox.back; ++slice)
        {
            if(!tempTex)
            {
                // Bind directly
                if(mFormat == PF_DEPTH)
                    bindToFramebuffer(GL_DEPTH_ATTACHMENT, slice);
                else
                    bindToFramebuffer(GL_COLOR_ATTACHMENT0, slice);
            }
            
            /// Calculate source texture coordinates
            float u1 = (float)srcBox.left / (float)src->mWidth;
            float v1 = (float)srcBox.top / (float)src->mHeight;
            float u2 = (float)srcBox.right / (float)src->mWidth;
            float v2 = (float)srcBox.bottom / (float)src->mHeight;
            /// Calculate source slice for this destination slice
            float w = (float)(slice - dstBox.front) / (float)dstBox.getDepth();
            /// Get slice # in source
            w = w * (float)srcBox.getDepth() + srcBox.front;
            /// Normalise to texture coordinate in 0.0 .. 1.0
            w = (w+0.5f) / (float)src->mDepth;
            
            /// Finally we're ready to rumble	
            glBindTexture(src->mTarget, src->mTextureID);
            GL_CHECK_ERROR
            glEnable(src->mTarget);
            GL_CHECK_ERROR

            GLfloat squareVertices[] = {
                -1.0f, -1.0f,
                1.0f, -1.0f,
                -1.0f,  1.0f,
                1.0f,  1.0f,
            };
            GLfloat texCoords[] = {
                u1, v1, w,
                u2, v1, w,
                u2, v2, w,
                u1, v2, w
            };
            
            GLuint posAttrIndex = 0;
            GLuint texAttrIndex = 0;
            if(Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
            {
                GLSLProgramPipeline* programPipeline = GLSLProgramPipelineManager::getSingleton().getActiveProgramPipeline();
                posAttrIndex = (GLuint)programPipeline->getAttributeIndex(VES_POSITION, 0);
                texAttrIndex = (GLuint)programPipeline->getAttributeIndex(VES_TEXTURE_COORDINATES, 0);
            }
            else
            {
                GLSLLinkProgram* linkProgram = GLSLLinkProgramManager::getSingleton().getActiveLinkProgram();
                posAttrIndex = (GLuint)linkProgram->getAttributeIndex(VES_POSITION, 0);
                texAttrIndex = (GLuint)linkProgram->getAttributeIndex(VES_TEXTURE_COORDINATES, 0);
            }

            // Draw the textured quad
            glVertexAttribPointer(posAttrIndex,
                                  2,
                                  GL_FLOAT,
                                  0,
                                  0,
                                  squareVertices);
            GL_CHECK_ERROR
            glEnableVertexAttribArray(posAttrIndex);
            GL_CHECK_ERROR
            glVertexAttribPointer(texAttrIndex,
                                  3,
                                  GL_FLOAT,
                                  0,
                                  0,
                                  texCoords);
            GL_CHECK_ERROR
            glEnableVertexAttribArray(texAttrIndex);
            GL_CHECK_ERROR
            
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            GL_CHECK_ERROR
            
            glDisable(src->mTarget);
            GL_CHECK_ERROR
            
            if(tempTex)
            {
                // Copy temporary texture
                glBindTexture(mTarget, mTextureID);
                GL_CHECK_ERROR
                switch(mTarget)
                {
                    case GL_TEXTURE_1D:
                        glCopyTexSubImage1D(mFaceTarget, mLevel, 
                                            dstBox.left, 
                                            0, 0, dstBox.getWidth());
                        break;
                    case GL_TEXTURE_2D:
                    case GL_TEXTURE_CUBE_MAP:
                    case GL_TEXTURE_RECTANGLE:
                        glCopyTexSubImage2D(mFaceTarget, mLevel, 
                                            dstBox.left, dstBox.top, 
                                            0, 0, dstBox.getWidth(), dstBox.getHeight());
                        break;
                    case GL_TEXTURE_3D:
                    case GL_TEXTURE_2D_ARRAY:
                        glCopyTexSubImage3D(mFaceTarget, mLevel, 
                                            dstBox.left, dstBox.top, slice, 
                                            0, 0, dstBox.getWidth(), dstBox.getHeight());
                        break;
                }
                GL_CHECK_ERROR
            }
        }
        // Finish up 
        if(!tempTex)
        {
            // Generate mipmaps
            if(mUsage & TU_AUTOMIPMAP)
            {
                glBindTexture(mTarget, mTextureID);
                GL_CHECK_ERROR
                glGenerateMipmap(mTarget);
                GL_CHECK_ERROR
            }
        }
        
        // Reset source texture to sane state
        glBindTexture(src->mTarget, src->mTextureID);
        GL_CHECK_ERROR
        glTexParameteri(src->mTarget, GL_TEXTURE_BASE_LEVEL, 0);
        GL_CHECK_ERROR
        
        // Detach texture from temporary framebuffer
        if(mFormat == PF_DEPTH)
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                      GL_RENDERBUFFER, 0);
        else
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                      GL_RENDERBUFFER, 0);
        GL_CHECK_ERROR
        // Restore old framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, oldfb);
        GL_CHECK_ERROR
        glDeleteTextures(1, &tempTex);
        GL_CHECK_ERROR
    }
    //-----------------------------------------------------------------------------  
    // blitFromMemory doing hardware trilinear scaling
    void GL3PlusTextureBuffer::blitFromMemory(const PixelBox &src_orig, const Image::Box &dstBox)
    {
        // Fall back to normal GLHardwarePixelBuffer::blitFromMemory in case 
        // - FBO is not supported
        // - Either source or target is luminance due doesn't looks like supported by hardware
        // - the source dimensions match the destination ones, in which case no scaling is needed
        if(PixelUtil::isLuminance(src_orig.format) ||
           PixelUtil::isLuminance(mFormat) ||
           (src_orig.getWidth() == dstBox.getWidth() &&
            src_orig.getHeight() == dstBox.getHeight() &&
            src_orig.getDepth() == dstBox.getDepth()))
        {
            GL3PlusHardwarePixelBuffer::blitFromMemory(src_orig, dstBox);
            return;
        }
        if(!mBuffer.contains(dstBox))
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Destination box out of range",
                        "GL3PlusTextureBuffer::blitFromMemory");
        // For scoped deletion of conversion buffer
        MemoryDataStreamPtr buf;
        PixelBox src;
        
        // First, convert the srcbox to a OpenGL compatible pixel format
        if(GL3PlusPixelUtil::getGLOriginFormat(src_orig.format) == 0)
        {
            // Convert to buffer internal format
            buf.bind(new MemoryDataStream(PixelUtil::getMemorySize(src_orig.getWidth(), src_orig.getHeight(),
                                                                        src_orig.getDepth(), mFormat)));
            src = PixelBox(src_orig.getWidth(), src_orig.getHeight(), src_orig.getDepth(), mFormat, buf->getPtr());
            PixelUtil::bulkPixelConversion(src_orig, src);
        }
        else
        {
            // No conversion needed
            src = src_orig;
        }
        
        // Create temporary texture to store source data
        GLuint id;
        GLenum target = (src.getDepth() != 1) ? GL_TEXTURE_3D : GL_TEXTURE_2D;
        
        // Generate texture name
        glGenTextures(1, &id);
        
        // Set texture type
        glBindTexture(target, id);
        
        // Set automatic mipmap generation; nice for minimisation
        glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, 1000 );
        //    glTexParameteri(target, GL_GENERATE_MIPMAP, GL_TRUE );
        
        // Allocate texture memory
        if(target == GL_TEXTURE_3D || target == GL_TEXTURE_2D_ARRAY)
            glTexImage3D(target, 0, src.format, src.getWidth(), src.getHeight(), src.getDepth(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        else
            glTexImage2D(target, 0, src.format, src.getWidth(), src.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        
        // GL texture buffer
        GL3PlusTextureBuffer tex(StringUtil::BLANK, target, id, 0, 0, (Usage)(TU_AUTOMIPMAP|HBU_STATIC_WRITE_ONLY), false, false, 0);
        
        // Upload data to 0,0,0 in temporary texture
        Image::Box tempTarget(0, 0, 0, src.getWidth(), src.getHeight(), src.getDepth());
        tex.upload(src, tempTarget);
        
        // Blit
        blitFromTexture(&tex, tempTarget, dstBox);
        
        // Delete temp texture
        glDeleteTextures(1, &id);
    }
    
    RenderTexture *GL3PlusTextureBuffer::getRenderTarget(size_t zoffset)
    {
        assert(mUsage & TU_RENDERTARGET);
        assert(zoffset < mDepth);
        return mSliceTRT[zoffset];
    }
    
    void GL3PlusTextureBuffer::buildMipmaps(const PixelBox &data)
    {
        PixelBox scaled = data;
        scaled.data = data.data;
        scaled.left = data.left;
        scaled.right = data.right;
        scaled.top = data.top;
        scaled.bottom = data.bottom;
        scaled.front = data.front;
        scaled.back = data.back;
        
        int width = data.getWidth();
        int height = data.getHeight();
        int depth = data.getDepth();

        int logW = computeLog(width);
        int logH = computeLog(height);
        int level = (logW > logH ? logW : logH);
        
        for (int mip = 0; mip <= level; mip++)
        {
            GLenum glFormat = GL3PlusPixelUtil::getGLOriginFormat(scaled.format);
            GLenum dataType = GL3PlusPixelUtil::getGLOriginDataType(scaled.format);
            
            switch(mTarget)
            {
                case GL_TEXTURE_1D:
                    glTexImage1D(mFaceTarget,
                                 mip,
                                 GL_RGBA,
                                 width,
                                 0,
                                 glFormat,
                                 dataType,
                                 scaled.data);
                    GL_CHECK_ERROR
                    break;
                case GL_TEXTURE_2D:
                case GL_TEXTURE_CUBE_MAP:
                case GL_TEXTURE_RECTANGLE:
                    glTexImage2D(mFaceTarget,
                                 mip,
                                 GL_RGBA,
                                 width, height,
                                 0,
                                 glFormat,
                                 dataType,
                                 scaled.data);
                    GL_CHECK_ERROR
                    break;		
                case GL_TEXTURE_3D:
                case GL_TEXTURE_2D_ARRAY:
                    glTexImage3D(mFaceTarget,
                                 mip,
                                 GL_RGBA,
                                 width, height, depth,
                                 0,
                                 glFormat,
                                 dataType,
                                 scaled.data);
                    GL_CHECK_ERROR
                    break;
            }

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
    
    //********* GL3PlusRenderBuffer
    //----------------------------------------------------------------------------- 
    GL3PlusRenderBuffer::GL3PlusRenderBuffer(GLenum format, size_t width, size_t height, GLsizei numSamples):
    GL3PlusHardwarePixelBuffer(width, height, 1, GL3PlusPixelUtil::getClosestOGREFormat(format), HBU_WRITE_ONLY),
    mRenderbufferID(0)
    
    {
        mGLInternalFormat = format;
        // Generate renderbuffer
        glGenRenderbuffers(1, &mRenderbufferID);
        GL_CHECK_ERROR

        // Bind it to FBO
        glBindRenderbuffer(GL_RENDERBUFFER, mRenderbufferID);
        GL_CHECK_ERROR

        // Allocate storage for depth buffer
        if (numSamples > 0)
        {
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, 
                                             numSamples, format, width, height);
        }
        else
        {
            glRenderbufferStorage(GL_RENDERBUFFER, format,
                                  width, height);
        }
        GL_CHECK_ERROR
    }
    //----------------------------------------------------------------------------- 
    GL3PlusRenderBuffer::~GL3PlusRenderBuffer()
    {
        // Generate renderbuffer
        glDeleteRenderbuffers(1, &mRenderbufferID);
        GL_CHECK_ERROR
    }
    //-----------------------------------------------------------------------------  
    void GL3PlusRenderBuffer::bindToFramebuffer(GLenum attachment, size_t zoffset)
    {
        assert(zoffset < mDepth);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment,
                                  GL_RENDERBUFFER, mRenderbufferID);
        GL_CHECK_ERROR
    }
}
