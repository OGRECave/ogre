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
#include "OgreGLHardwarePixelBuffer.h"

#include "OgreTextureManager.h"

#include "OgreGLTexture.h"
#include "OgreGLRenderSystem.h"
#include "OgreGLPixelFormat.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreBitwise.h"
#include "OgreGLFBORenderTexture.h"
#include "OgreRoot.h"
#include "OgreGLStateCacheManager.h"

namespace Ogre {
//----------------------------------------------------------------------------- 
GLHardwarePixelBuffer::GLHardwarePixelBuffer(uint32 inWidth, uint32 inHeight, uint32 inDepth,
                PixelFormat inFormat,
                HardwareBuffer::Usage usage):
        GLHardwarePixelBufferCommon(inWidth, inHeight, inDepth, inFormat, usage)
{
}

//-----------------------------------------------------------------------------  
void GLHardwarePixelBuffer::blitFromMemory(const PixelBox &src, const Box &dstBox)
{
    if(!mBuffer.contains(dstBox))
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "destination box out of range",
         "GLHardwarePixelBuffer::blitFromMemory");
    PixelBox scaled;
    
    if(src.getSize() != dstBox.getSize())
    {
        // Scale to destination size.
        // This also does pixel format conversion if needed
        allocateBuffer();
        scaled = mBuffer.getSubVolume(dstBox);
        Image::scale(src, scaled, Image::FILTER_BILINEAR);
    }
    else if(GLPixelUtil::getGLInternalFormat(src.format) == 0)
    {
        // Extents match, but format is not accepted as valid source format for GL
        // do conversion in temporary buffer
        allocateBuffer();
        scaled = mBuffer.getSubVolume(dstBox);
        PixelUtil::bulkPixelConversion(src, scaled);
    }
    else
    {
        // No scaling or conversion needed
        scaled = src;
    }
    
    upload(scaled, dstBox);
    freeBuffer();
}
//-----------------------------------------------------------------------------  
void GLHardwarePixelBuffer::blitToMemory(const Box &srcBox, const PixelBox &dst)
{
    if(!mBuffer.contains(srcBox))
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "source box out of range",
         "GLHardwarePixelBuffer::blitToMemory");
    if(srcBox.getOrigin() == Vector3i(0, 0 ,0) &&
       srcBox.getSize() == getSize() &&
       dst.getSize() == getSize() &&
       GLPixelUtil::getGLInternalFormat(dst.format) != 0)
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
//********* GLTextureBuffer
GLTextureBuffer::GLTextureBuffer(GLRenderSystem* renderSystem, GLTexture* parent, GLint face,
                                 GLint level, uint32 width, uint32 height, uint32 depth)
    : GLHardwarePixelBuffer(width, height, depth, parent->getFormat(), (Usage)parent->getUsage()),
      mTarget(parent->getGLTextureTarget()), mFaceTarget(0), mTextureID(parent->getGLID()),
      mFace(face), mLevel(level), mHwGamma(parent->isHardwareGammaEnabled()), mSliceTRT(0),
      mRenderSystem(renderSystem)
{
    // Get face identifier
    mFaceTarget = mTarget;
    if(mTarget == GL_TEXTURE_CUBE_MAP)
        mFaceTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face;
    
    // Get format
    mGLInternalFormat = GLPixelUtil::getGLInternalFormat(mFormat, mHwGamma);
    
    // Default
    mRowPitch = mWidth;
    mSlicePitch = mHeight*mWidth;
    mSizeInBytes = PixelUtil::getMemorySize(mWidth, mHeight, mDepth, mFormat);
    
    // Log a message
    /*
    std::stringstream str;
    str << "GLHardwarePixelBuffer constructed for texture " << mTextureID 
        << " face " << mFace << " level " << mLevel << ": "
        << "width=" << mWidth << " height="<< mHeight << " depth=" << mDepth
        << "format=" << PixelUtil::getFormatName(mFormat) << "(internal 0x"
        << std::hex << value << ")";
    LogManager::getSingleton().logMessage( 
                LML_NORMAL, str.str());
    */
    // Set up pixel box
    mBuffer = PixelBox(mWidth, mHeight, mDepth, mFormat);
    
    if(mWidth==0 || mHeight==0 || mDepth==0)
        /// We are invalid, do not allocate a buffer
        return;
    // Allocate buffer
    //if(mUsage & HBU_STATIC)
    //  allocateBuffer();
    // Is this a render target?
    if(mUsage & TU_RENDERTARGET)
    {
        // Create render target for each slice
        mSliceTRT.reserve(mDepth);
        for(uint32 zoffset=0; zoffset<mDepth; ++zoffset)
        {
            String name;
            name = "rtt/" + StringConverter::toString((size_t)this) + "/" + parent->getName();
            GLSurfaceDesc surface;
            surface.buffer = this;
            surface.zoffset = zoffset;
            RenderTexture *trt = GLRTTManager::getSingleton().createRenderTexture(name, surface, mHwGamma, parent->getFSAA());
            mSliceTRT.push_back(trt);
            Root::getSingleton().getRenderSystem()->attachRenderTarget(*mSliceTRT[zoffset]);
        }
    }
}
GLTextureBuffer::~GLTextureBuffer()
{
    if(mUsage & TU_RENDERTARGET)
    {
        // Delete all render targets that are not yet deleted via _clearSliceRTT because the rendertarget
        // was deleted by the user.
        for (SliceTRT::const_iterator it = mSliceTRT.begin(); it != mSliceTRT.end(); ++it)
        {
            Root::getSingleton().getRenderSystem()->destroyRenderTarget((*it)->getName());
        }
    }
}
//-----------------------------------------------------------------------------
void GLTextureBuffer::upload(const PixelBox &data, const Box &dest)
{
    mRenderSystem->_getStateCacheManager()->bindGLTexture( mTarget, mTextureID );
    if(PixelUtil::isCompressed(data.format))
    {
        if(data.format != mFormat || !data.isConsecutive())
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
            "Compressed images must be consecutive, in the source format",
            "GLTextureBuffer::upload");
        GLenum format = GLPixelUtil::getGLInternalFormat(mFormat, mHwGamma);
        // Data must be consecutive and at beginning of buffer as PixelStorei not allowed
        // for compressed formats
        switch(mTarget) {
            case GL_TEXTURE_1D:
                // some systems (e.g. old Apple) don't like compressed subimage calls
                // so prefer non-sub versions
                if (dest.left == 0)
                {
                    glCompressedTexImage1DARB(GL_TEXTURE_1D, mLevel,
                        format,
                        dest.getWidth(),
                        0,
                        data.getConsecutiveSize(),
                        data.data);
                }
                else
                {
                    glCompressedTexSubImage1DARB(GL_TEXTURE_1D, mLevel, 
                        dest.left,
                        dest.getWidth(),
                        format, data.getConsecutiveSize(),
                        data.data);
                }
                break;
            case GL_TEXTURE_2D:
            case GL_TEXTURE_CUBE_MAP:
                // some systems (e.g. old Apple) don't like compressed subimage calls
                // so prefer non-sub versions
                if (dest.left == 0 && dest.top == 0)
                {
                    glCompressedTexImage2DARB(mFaceTarget, mLevel,
                        format,
                        dest.getWidth(),
                        dest.getHeight(),
                        0,
                        data.getConsecutiveSize(),
                        data.data);
                }
                else
                {
                    glCompressedTexSubImage2DARB(mFaceTarget, mLevel, 
                        dest.left, dest.top, 
                        dest.getWidth(), dest.getHeight(),
                        format, data.getConsecutiveSize(),
                        data.data);
                }
                break;
            case GL_TEXTURE_3D:
            case GL_TEXTURE_2D_ARRAY_EXT:
                // some systems (e.g. old Apple) don't like compressed subimage calls
                // so prefer non-sub versions
                if (dest.left == 0 && dest.top == 0 && dest.front == 0)
                {
                    glCompressedTexImage3DARB(mTarget, mLevel,
                        format,
                        dest.getWidth(),
                        dest.getHeight(),
                        dest.getDepth(),
                        0,
                        data.getConsecutiveSize(),
                        data.data);
                }
                else
                {           
                    glCompressedTexSubImage3DARB(mTarget, mLevel, 
                        dest.left, dest.top, dest.front,
                        dest.getWidth(), dest.getHeight(), dest.getDepth(),
                        format, data.getConsecutiveSize(),
                        data.data);
                }
                break;
        }
        
    }
    else
    {
        if(data.getWidth() != data.rowPitch)
            glPixelStorei(GL_UNPACK_ROW_LENGTH, data.rowPitch);
        if(data.getWidth() > 0 && data.getHeight()*data.getWidth() != data.slicePitch)
            glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, (data.slicePitch/data.getWidth()));
        if((data.getWidth()*PixelUtil::getNumElemBytes(data.format)) & 3) {
            // Standard alignment of 4 is not right
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        }

        void* pdata = data.getTopLeftFrontPixelPtr();

        switch(mTarget) {
            case GL_TEXTURE_1D:
                glTexSubImage1D(GL_TEXTURE_1D, mLevel, 
                    dest.left,
                    dest.getWidth(),
                    GLPixelUtil::getGLOriginFormat(data.format), GLPixelUtil::getGLOriginDataType(data.format),
                    pdata);
                break;
            case GL_TEXTURE_2D:
            case GL_TEXTURE_CUBE_MAP:
                glTexSubImage2D(mFaceTarget, mLevel, 
                    dest.left, dest.top, 
                    dest.getWidth(), dest.getHeight(),
                    GLPixelUtil::getGLOriginFormat(data.format), GLPixelUtil::getGLOriginDataType(data.format),
                    pdata);
                break;
            case GL_TEXTURE_3D:
            case GL_TEXTURE_2D_ARRAY_EXT:
                glTexSubImage3D(
                    mTarget, mLevel, 
                    dest.left, dest.top, dest.front,
                    dest.getWidth(), dest.getHeight(), dest.getDepth(),
                    GLPixelUtil::getGLOriginFormat(data.format), GLPixelUtil::getGLOriginDataType(data.format),
                    pdata);
                break;
        }   
    }

    // TU_AUTOMIPMAP is only enabled when there are no custom mips
    // so we do not have to care about overwriting
    if((mUsage & TU_AUTOMIPMAP) && (mLevel == 0))
    {
        glGenerateMipmapEXT(mTarget);
    }

    // Restore defaults
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}
//-----------------------------------------------------------------------------  
void GLTextureBuffer::download(const PixelBox &data)
{
    if(data.getSize() != getSize())
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "only download of entire buffer is supported by GL",
            "GLTextureBuffer::download");
    mRenderSystem->_getStateCacheManager()->bindGLTexture( mTarget, mTextureID );
    if(PixelUtil::isCompressed(data.format))
    {
        if(data.format != mFormat || !data.isConsecutive())
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
            "Compressed images must be consecutive, in the source format",
            "GLTextureBuffer::download");
        // Data must be consecutive and at beginning of buffer as PixelStorei not allowed
        // for compressed formate
        glGetCompressedTexImageARB(mFaceTarget, mLevel, data.data);
    } 
    else
    {
        if(data.getWidth() != data.rowPitch)
            glPixelStorei(GL_PACK_ROW_LENGTH, data.rowPitch);
        if(data.getHeight()*data.getWidth() != data.slicePitch)
            glPixelStorei(GL_PACK_IMAGE_HEIGHT, (data.slicePitch/data.getWidth()));
        if((data.getWidth()*PixelUtil::getNumElemBytes(data.format)) & 3) {
            // Standard alignment of 4 is not right
            glPixelStorei(GL_PACK_ALIGNMENT, 1);
        }
        // We can only get the entire texture
        glGetTexImage(mFaceTarget, mLevel, 
            GLPixelUtil::getGLOriginFormat(data.format), GLPixelUtil::getGLOriginDataType(data.format),
            data.getTopLeftFrontPixelPtr());
        // Restore defaults
        glPixelStorei(GL_PACK_ROW_LENGTH, 0);
        glPixelStorei(GL_PACK_IMAGE_HEIGHT, 0);
        glPixelStorei(GL_PACK_ALIGNMENT, 4);
    }
}
//-----------------------------------------------------------------------------  
void GLTextureBuffer::bindToFramebuffer(uint32 attachment, uint32 zoffset)
{
    assert(zoffset < mDepth);
    switch(mTarget)
    {
    case GL_TEXTURE_1D:
        glFramebufferTexture1DEXT(GL_FRAMEBUFFER_EXT, attachment,
                            mFaceTarget, mTextureID, mLevel);
        break;
    case GL_TEXTURE_2D:
    case GL_TEXTURE_CUBE_MAP:
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachment,
                            mFaceTarget, mTextureID, mLevel);
        break;
    case GL_TEXTURE_3D:
    case GL_TEXTURE_2D_ARRAY_EXT:
        glFramebufferTexture3DEXT(GL_FRAMEBUFFER_EXT, attachment,
                            mFaceTarget, mTextureID, mLevel, zoffset);
        break;
    }
}
//-----------------------------------------------------------------------------
void GLTextureBuffer::copyFromFramebuffer(uint32 zoffset)
{
    mRenderSystem->_getStateCacheManager()->bindGLTexture(mTarget, mTextureID);
    switch(mTarget)
    {
    case GL_TEXTURE_1D:
        glCopyTexSubImage1D(mFaceTarget, mLevel, 0, 0, 0, mWidth);
        break;
    case GL_TEXTURE_2D:
    case GL_TEXTURE_CUBE_MAP:
        glCopyTexSubImage2D(mFaceTarget, mLevel, 0, 0, 0, 0, mWidth, mHeight);
        break;
    case GL_TEXTURE_3D:
    case GL_TEXTURE_2D_ARRAY_EXT:
        glCopyTexSubImage3D(mFaceTarget, mLevel, 0, 0, zoffset, 0, 0, mWidth, mHeight);
        break;
    }
}
//-----------------------------------------------------------------------------  
void GLTextureBuffer::blit(const HardwarePixelBufferSharedPtr &src, const Box &srcBox, const Box &dstBox)
{
    GLTextureBuffer *srct = static_cast<GLTextureBuffer *>(src.get());
    /// Check for FBO support first
    /// Destination texture must be 1D, 2D, 3D, or Cube
    /// Source texture must be 1D, 2D or 3D
    
    // This does not seem to work for RTTs after the first update
    // I have no idea why! For the moment, disable 
    if(GLEW_EXT_framebuffer_object &&
        (srct->mTarget==GL_TEXTURE_1D||srct->mTarget==GL_TEXTURE_2D
         ||srct->mTarget==GL_TEXTURE_3D)&&mTarget!=GL_TEXTURE_2D_ARRAY_EXT)
    {
        blitFromTexture(srct, srcBox, dstBox);
    }
    else
    {
        GLHardwarePixelBuffer::blit(src, srcBox, dstBox);
    }
}

//-----------------------------------------------------------------------------  
/// Very fast texture-to-texture blitter and hardware bi/trilinear scaling implementation using FBO
/// Destination texture must be 1D, 2D, 3D, or Cube
/// Source texture must be 1D, 2D or 3D
/// Supports compressed formats as both source and destination format, it will use the hardware DXT compressor
/// if available.
/// @author W.J. van der Laan
void GLTextureBuffer::blitFromTexture(GLTextureBuffer *src, const Box &srcBox, const Box &dstBox)
{
    //std::cerr << "GLTextureBuffer::blitFromTexture " <<
    //src->mTextureID << ":" << srcBox.left << "," << srcBox.top << "," << srcBox.right << "," << srcBox.bottom << " " << 
    //mTextureID << ":" << dstBox.left << "," << dstBox.top << "," << dstBox.right << "," << dstBox.bottom << std::endl;
    /// Store reference to FBO manager
    GLFBOManager *fboMan = static_cast<GLFBOManager *>(GLRTTManager::getSingletonPtr());
    
    /// Save and clear GL state for rendering
    glPushAttrib(GL_COLOR_BUFFER_BIT | GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | 
        GL_FOG_BIT | GL_LIGHTING_BIT | GL_POLYGON_BIT | GL_SCISSOR_BIT | GL_STENCIL_BUFFER_BIT |
        GL_TEXTURE_BIT | GL_VIEWPORT_BIT);

    // Important to disable all other texture units
    RenderSystem* rsys = Root::getSingleton().getRenderSystem();
    rsys->_disableTextureUnitsFrom(0);
    mRenderSystem->_getStateCacheManager()->activateGLTextureUnit(0);

    /// Disable alpha, depth and scissor testing, disable blending, 
    /// disable culling, disble lighting, disable fog and reset foreground
    /// colour.
    mRenderSystem->_getStateCacheManager()->setEnabled(GL_ALPHA_TEST, false);
    mRenderSystem->_getStateCacheManager()->setEnabled(GL_DEPTH_TEST, false);
    mRenderSystem->_getStateCacheManager()->setEnabled(GL_SCISSOR_TEST, false);
    mRenderSystem->_getStateCacheManager()->setEnabled(GL_BLEND, false);
    mRenderSystem->_getStateCacheManager()->setEnabled(GL_CULL_FACE, false);
    mRenderSystem->_getStateCacheManager()->setEnabled(GL_LIGHTING, false);
    mRenderSystem->_getStateCacheManager()->setEnabled(GL_FOG, false);
    
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
    
    /// Set up source texture
    mRenderSystem->_getStateCacheManager()->bindGLTexture(src->mTarget, src->mTextureID);
    
    /// Set filtering modes depending on the dimensions and source
    if(srcBox.getSize()==dstBox.getSize())
    {
        /// Dimensions match -- use nearest filtering (fastest and pixel correct)
        mRenderSystem->_getStateCacheManager()->setTexParameteri(src->mTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        mRenderSystem->_getStateCacheManager()->setTexParameteri(src->mTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    else
    {
        /// Dimensions don't match -- use bi or trilinear filtering depending on the
        /// source texture.
        if(src->mUsage & TU_AUTOMIPMAP)
        {
            /// Automatic mipmaps, we can safely use trilinear filter which
            /// brings greatly imporoved quality for minimisation.
            mRenderSystem->_getStateCacheManager()->setTexParameteri(src->mTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            mRenderSystem->_getStateCacheManager()->setTexParameteri(src->mTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        else
        {
            /// Manual mipmaps, stay safe with bilinear filtering so that no
            /// intermipmap leakage occurs.
            mRenderSystem->_getStateCacheManager()->setTexParameteri(src->mTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            mRenderSystem->_getStateCacheManager()->setTexParameteri(src->mTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
    }

    /// Clamp to edge (fastest)
    mRenderSystem->_getStateCacheManager()->setTexParameteri(src->mTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    mRenderSystem->_getStateCacheManager()->setTexParameteri(src->mTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    mRenderSystem->_getStateCacheManager()->setTexParameteri(src->mTarget, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    
    /// Set origin base level mipmap to make sure we source from the right mip
    /// level.
    mRenderSystem->_getStateCacheManager()->setTexParameteri(src->mTarget, GL_TEXTURE_BASE_LEVEL, src->mLevel);
    
    /// Store old binding so it can be restored later
    GLint oldfb;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &oldfb);
    
    /// Set up temporary FBO
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboMan->getTemporaryFBO());
    
    TexturePtr tempTex;
    if(!fboMan->checkFormat(mFormat))
    {
        /// If target format not directly supported, create intermediate texture
        tempTex = TextureManager::getSingleton().createManual(
            "GLBlitFromTextureTMP", ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME, TEX_TYPE_2D,
            dstBox.getWidth(), dstBox.getHeight(), dstBox.getDepth(), 0,
            fboMan->getSupportedAlternative(mFormat));

        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
            GL_TEXTURE_2D, static_pointer_cast<GLTexture>(tempTex)->getGLID(), 0);
        /// Set viewport to size of destination slice
        mRenderSystem->_getStateCacheManager()->setViewport(0, 0, dstBox.getWidth(), dstBox.getHeight());
    }
    else
    {
        /// We are going to bind directly, so set viewport to size and position of destination slice
        mRenderSystem->_getStateCacheManager()->setViewport(dstBox.left, dstBox.top, dstBox.getWidth(), dstBox.getHeight());
    }
    
    /// Process each destination slice
    for(uint32 slice=dstBox.front; slice<dstBox.back; ++slice)
    {
        if(!tempTex)
        {
            /// Bind directly
            bindToFramebuffer(GL_COLOR_ATTACHMENT0_EXT, slice);
        }
        /// Calculate source texture coordinates
        float u1 = (float)srcBox.left / (float)src->mWidth;
        float v1 = (float)srcBox.top / (float)src->mHeight;
        float u2 = (float)srcBox.right / (float)src->mWidth;
        float v2 = (float)srcBox.bottom / (float)src->mHeight;
        /// Calculate source slice for this destination slice
        float w = (float)(slice - dstBox.front) / (float)dstBox.getDepth();
        /// Get slice # in source
        w = w * (float)(srcBox.getDepth() + srcBox.front);
        /// Normalise to texture coordinate in 0.0 .. 1.0
        w = (w+0.5f) / (float)src->mDepth;
        
        /// Finally we're ready to rumble   
        mRenderSystem->_getStateCacheManager()->bindGLTexture(src->mTarget, src->mTextureID);
        mRenderSystem->_getStateCacheManager()->setEnabled(src->mTarget, true);
        glBegin(GL_QUADS);
        glTexCoord3f(u1, v1, w);
        glVertex2f(-1.0f, -1.0f);
        glTexCoord3f(u2, v1, w);
        glVertex2f(1.0f, -1.0f);
        glTexCoord3f(u2, v2, w);
        glVertex2f(1.0f, 1.0f);
        glTexCoord3f(u1, v2, w);
        glVertex2f(-1.0f, 1.0f);
        glEnd();
        mRenderSystem->_getStateCacheManager()->setEnabled(src->mTarget, false);
        
        if(tempTex)
        {
            /// Copy temporary texture
            mRenderSystem->_getStateCacheManager()->bindGLTexture(mTarget, mTextureID);
            switch(mTarget)
            {
            case GL_TEXTURE_1D:
                glCopyTexSubImage1D(mFaceTarget, mLevel, 
                    dstBox.left, 
                    0, 0, dstBox.getWidth());
                break;
            case GL_TEXTURE_2D:
            case GL_TEXTURE_CUBE_MAP:
                glCopyTexSubImage2D(mFaceTarget, mLevel, 
                    dstBox.left, dstBox.top, 
                    0, 0, dstBox.getWidth(), dstBox.getHeight());
                break;
            case GL_TEXTURE_3D:
            case GL_TEXTURE_2D_ARRAY_EXT:
                glCopyTexSubImage3D(mFaceTarget, mLevel, 
                    dstBox.left, dstBox.top, slice, 
                    0, 0, dstBox.getWidth(), dstBox.getHeight());
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
            mRenderSystem->_getStateCacheManager()->bindGLTexture(mTarget, mTextureID);
            glGenerateMipmapEXT(mTarget);
        }
    }

    /// Reset source texture to sane state
    mRenderSystem->_getStateCacheManager()->bindGLTexture(src->mTarget, src->mTextureID);
    mRenderSystem->_getStateCacheManager()->setTexParameteri(src->mTarget, GL_TEXTURE_BASE_LEVEL, 0);
    
    /// Detach texture from temporary framebuffer
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                    GL_RENDERBUFFER_EXT, 0);
    /// Restore old framebuffer
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, oldfb);
    /// Restore matrix stacks and render state
    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glPopAttrib();

    if(tempTex)
        TextureManager::getSingleton().remove(tempTex);
}
//-----------------------------------------------------------------------------  
/// blitFromMemory doing hardware trilinear scaling
void GLTextureBuffer::blitFromMemory(const PixelBox &src, const Box &dstBox)
{
    /// Fall back to normal GLHardwarePixelBuffer::blitFromMemory in case 
    /// - FBO is not supported
    /// - the source dimensions match the destination ones, in which case no scaling is needed
    if (!GLEW_EXT_framebuffer_object ||
        (src.getSize() == dstBox.getSize()))
    {
        GLHardwarePixelBuffer::blitFromMemory(src, dstBox);
        return;
    }
    if(!mBuffer.contains(dstBox))
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "destination box out of range",
                    "GLTextureBuffer::blitFromMemory");

    TextureType type = (src.getDepth() != 1) ? TEX_TYPE_3D : TEX_TYPE_2D;

    // Set automatic mipmap generation; nice for minimisation
    TexturePtr tex = TextureManager::getSingleton().createManual(
        "GLBlitFromMemoryTMP", ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME, type,
        src.getWidth(), src.getHeight(), src.getDepth(), MIP_UNLIMITED, src.format);

    // Upload data to 0,0,0 in temporary texture
    Box tempTarget(src.getSize());
    tex->getBuffer()->blitFromMemory(src, tempTarget);

    // Blit from texture
    blit(tex->getBuffer(), tempTarget, dstBox);

    // Delete temp texture
    TextureManager::getSingleton().remove(tex);
}
//-----------------------------------------------------------------------------    

RenderTexture *GLTextureBuffer::getRenderTarget(size_t zoffset)
{
    assert(mUsage & TU_RENDERTARGET);
    assert(zoffset < mDepth);
    return mSliceTRT[zoffset];
}
//********* GLRenderBuffer
//----------------------------------------------------------------------------- 
GLRenderBuffer::GLRenderBuffer(GLenum format, uint32 width, uint32 height, GLsizei numSamples):
    GLHardwarePixelBuffer(width, height, 1, GLPixelUtil::getClosestOGREFormat(format),HBU_WRITE_ONLY),
    mRenderbufferID(0)
{
    mGLInternalFormat = format;
    /// Generate renderbuffer
    glGenRenderbuffersEXT(1, &mRenderbufferID);
    /// Bind it to FBO
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, mRenderbufferID);
    
    /// Allocate storage for depth buffer
    if (numSamples > 0)
    {
        glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, 
            numSamples, format, width, height);
    }
    else
    {
        glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, format,
                            width, height);
    }
}
//----------------------------------------------------------------------------- 
GLRenderBuffer::~GLRenderBuffer()
{
    /// Generate renderbuffer
    glDeleteRenderbuffersEXT(1, &mRenderbufferID);
}
//-----------------------------------------------------------------------------  
void GLRenderBuffer::bindToFramebuffer(uint32 attachment, uint32 zoffset)
{
    assert(zoffset < mDepth);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, attachment,
                        GL_RENDERBUFFER_EXT, mRenderbufferID);
}

}
