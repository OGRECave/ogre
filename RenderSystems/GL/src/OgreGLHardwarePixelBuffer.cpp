/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#include "OgreGLHardwarePixelBuffer.h"
#include "OgreGLTexture.h"
#include "OgreGLSupport.h"
#include "OgreGLPixelFormat.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreBitwise.h"
#include "OgreGLFBORenderTexture.h"
#include "OgreRoot.h"

namespace Ogre {
//----------------------------------------------------------------------------- 
GLHardwarePixelBuffer::GLHardwarePixelBuffer(size_t mWidth, size_t mHeight, size_t mDepth,
                PixelFormat mFormat,
                HardwareBuffer::Usage usage):
      HardwarePixelBuffer(mWidth, mHeight, mDepth, mFormat, usage, false, false),
      mBuffer(mWidth, mHeight, mDepth, mFormat),
      mGLInternalFormat(GL_NONE)
{
}

//-----------------------------------------------------------------------------  
GLHardwarePixelBuffer::~GLHardwarePixelBuffer()
{
	// Force free buffer
	delete [] (uint8*)mBuffer.data;
}
//-----------------------------------------------------------------------------  
void GLHardwarePixelBuffer::allocateBuffer()
{
	if(mBuffer.data)
		// Already allocated
		return;
	mBuffer.data = new uint8[mSizeInBytes];
	// TODO: use PBO if we're HBU_DYNAMIC
}
//-----------------------------------------------------------------------------  
void GLHardwarePixelBuffer::freeBuffer()
{
	// Free buffer if we're STATIC to save memory
	if(mUsage & HBU_STATIC)
	{
		delete [] (uint8*)mBuffer.data;
		mBuffer.data = 0;
	}
}
//-----------------------------------------------------------------------------  
PixelBox GLHardwarePixelBuffer::lockImpl(const Image::Box lockBox,  LockOptions options)
{
	allocateBuffer();
	if(options != HardwareBuffer::HBL_DISCARD) 
	{
		// Download the old contents of the texture
		download(mBuffer);
	}
	mCurrentLockOptions = options;
	return mBuffer.getSubVolume(lockBox);
}
//-----------------------------------------------------------------------------  
void GLHardwarePixelBuffer::unlockImpl(void)
{
	if (mCurrentLockOptions != HardwareBuffer::HBL_READ_ONLY)
	{
		// From buffer to card, only upload if was locked for writing
		upload(mCurrentLock, Box(0,0,0,mWidth,mHeight,mDepth));
	}
	
	freeBuffer();
}

//-----------------------------------------------------------------------------  
void GLHardwarePixelBuffer::blitFromMemory(const PixelBox &src, const Image::Box &dstBox)
{
	if(!mBuffer.contains(dstBox))
		OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "destination box out of range",
		 "GLHardwarePixelBuffer::blitFromMemory");
	PixelBox scaled;
	
	if(src.getWidth() != dstBox.getWidth() ||
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
	else if(GLPixelUtil::getGLOriginFormat(src.format) == 0)
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
//-----------------------------------------------------------------------------  
void GLHardwarePixelBuffer::blitToMemory(const Image::Box &srcBox, const PixelBox &dst)
{
	if(!mBuffer.contains(srcBox))
		OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "source box out of range",
		 "GLHardwarePixelBuffer::blitToMemory");
	if(srcBox.left == 0 && srcBox.right == getWidth() &&
	   srcBox.top == 0 && srcBox.bottom == getHeight() &&
	   srcBox.front == 0 && srcBox.back == getDepth() &&
	   dst.getWidth() == getWidth() &&
	   dst.getHeight() == getHeight() &&
	   dst.getDepth() == getDepth() &&
	   GLPixelUtil::getGLOriginFormat(dst.format) != 0)
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
//-----------------------------------------------------------------------------
void GLHardwarePixelBuffer::upload(const PixelBox &data, const Image::Box &dest)
{
    OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
		"Upload not possible for this pixelbuffer type",
        "GLHardwarePixelBuffer::upload");
}
//-----------------------------------------------------------------------------  
void GLHardwarePixelBuffer::download(const PixelBox &data)
{
    OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Download not possible for this pixelbuffer type",
        "GLHardwarePixelBuffer::download");
}
//-----------------------------------------------------------------------------  
void GLHardwarePixelBuffer::bindToFramebuffer(GLenum attachment, size_t zoffset)
{
    OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Framebuffer bind not possible for this pixelbuffer type",
        "GLHardwarePixelBuffer::bindToFramebuffer");
}
//********* GLTextureBuffer
GLTextureBuffer::GLTextureBuffer(const String &baseName, GLenum target, GLuint id, 
								 GLint face, GLint level, Usage usage, bool crappyCard, 
								 bool writeGamma, uint fsaa):
	GLHardwarePixelBuffer(0, 0, 0, PF_UNKNOWN, usage),
	mTarget(target), mTextureID(id), mFace(face), mLevel(level), mSoftwareMipmap(crappyCard)
{
	// devise mWidth, mHeight and mDepth and mFormat
	GLint value;
	
	glBindTexture( mTarget, mTextureID );
	
	// Get face identifier
	mFaceTarget = mTarget;
	if(mTarget == GL_TEXTURE_CUBE_MAP)
		mFaceTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face;
	
	// Get width
	glGetTexLevelParameteriv(mFaceTarget, level, GL_TEXTURE_WIDTH, &value);
	mWidth = value;
	
	// Get height
	if(target == GL_TEXTURE_1D)
		value = 1;	// Height always 1 for 1D textures
	else
		glGetTexLevelParameteriv(mFaceTarget, level, GL_TEXTURE_HEIGHT, &value);
	mHeight = value;
	
	// Get depth
	if(target != GL_TEXTURE_3D)
		value = 1; // Depth always 1 for non-3D textures
	else
		glGetTexLevelParameteriv(mFaceTarget, level, GL_TEXTURE_DEPTH, &value);
	mDepth = value;

	// Get format
	glGetTexLevelParameteriv(mFaceTarget, level, GL_TEXTURE_INTERNAL_FORMAT, &value);
	mGLInternalFormat = value;
	mFormat = GLPixelUtil::getClosestOGREFormat(value);
	
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
	//	allocateBuffer();
    // Is this a render target?
    if(mUsage & TU_RENDERTARGET)
    {
        // Create render target for each slice
        mSliceTRT.reserve(mDepth);
        for(size_t zoffset=0; zoffset<mDepth; ++zoffset)
        {
            String name;
			name = "rtt/" + StringConverter::toString((size_t)this) + "/" + baseName;
            GLSurfaceDesc target;
            target.buffer = this;
            target.zoffset = zoffset;
            RenderTexture *trt = GLRTTManager::getSingleton().createRenderTexture(name, target, writeGamma, fsaa);
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
void GLTextureBuffer::upload(const PixelBox &data, const Image::Box &dest)
{
	glBindTexture( mTarget, mTextureID );
	if(PixelUtil::isCompressed(data.format))
	{
		if(data.format != mFormat || !data.isConsecutive())
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
			"Compressed images must be consecutive, in the source format",
		 	"GLTextureBuffer::upload");
		GLenum format = GLPixelUtil::getClosestGLInternalFormat(mFormat);
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
				// some systems (e.g. old Apple) don't like compressed subimage calls
				// so prefer non-sub versions
				if (dest.left == 0 && dest.top == 0 && dest.front == 0)
				{
					glCompressedTexImage3DARB(GL_TEXTURE_3D, mLevel,
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
					glCompressedTexSubImage3DARB(GL_TEXTURE_3D, mLevel, 
						dest.left, dest.top, dest.front,
						dest.getWidth(), dest.getHeight(), dest.getDepth(),
						format, data.getConsecutiveSize(),
						data.data);
				}
				break;
		}
		
	} 
	else if(mSoftwareMipmap)
	{
		GLint components = PixelUtil::getComponentCount(mFormat);
		if(data.getWidth() != data.rowPitch)
			glPixelStorei(GL_UNPACK_ROW_LENGTH, data.rowPitch);
		if(data.getHeight()*data.getWidth() != data.slicePitch)
			glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, (data.slicePitch/data.getWidth()));
		if(data.left > 0 || data.top > 0 || data.front > 0)
			glPixelStorei(GL_UNPACK_SKIP_PIXELS, data.left + data.rowPitch * data.top + data.slicePitch * data.front);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		
		switch(mTarget)
		{
		case GL_TEXTURE_1D:
			gluBuild1DMipmaps(
				GL_TEXTURE_1D, components,
				dest.getWidth(),
				GLPixelUtil::getGLOriginFormat(data.format), GLPixelUtil::getGLOriginDataType(data.format),
				data.data);
			break;
		case GL_TEXTURE_2D:
		case GL_TEXTURE_CUBE_MAP:
			gluBuild2DMipmaps(
				mFaceTarget,
				components, dest.getWidth(), dest.getHeight(), 
				GLPixelUtil::getGLOriginFormat(data.format), GLPixelUtil::getGLOriginDataType(data.format), 
				data.data);
			break;		
		case GL_TEXTURE_3D:
			/* Requires GLU 1.3 which is harder to come by than cards doing hardware mipmapping
				Most 3D textures don't need mipmaps?
			gluBuild3DMipmaps(
				GL_TEXTURE_3D, internalFormat, 
				data.getWidth(), data.getHeight(), data.getDepth(),
				GLPixelUtil::getGLOriginFormat(data.format), GLPixelUtil::getGLOriginDataType(data.format),
				data.data);
			*/
			glTexImage3D(
				GL_TEXTURE_3D, 0, components, 
				dest.getWidth(), dest.getHeight(), dest.getDepth(), 0, 
				GLPixelUtil::getGLOriginFormat(data.format), GLPixelUtil::getGLOriginDataType(data.format),
				data.data );
			break;
		}
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
		switch(mTarget) {
			case GL_TEXTURE_1D:
				glTexSubImage1D(GL_TEXTURE_1D, mLevel, 
					dest.left,
					dest.getWidth(),
					GLPixelUtil::getGLOriginFormat(data.format), GLPixelUtil::getGLOriginDataType(data.format),
					data.data);
				break;
			case GL_TEXTURE_2D:
			case GL_TEXTURE_CUBE_MAP:
				glTexSubImage2D(mFaceTarget, mLevel, 
					dest.left, dest.top, 
					dest.getWidth(), dest.getHeight(),
					GLPixelUtil::getGLOriginFormat(data.format), GLPixelUtil::getGLOriginDataType(data.format),
					data.data);
				break;
			case GL_TEXTURE_3D:
				glTexSubImage3D(
					GL_TEXTURE_3D, mLevel, 
					dest.left, dest.top, dest.front,
					dest.getWidth(), dest.getHeight(), dest.getDepth(),
					GLPixelUtil::getGLOriginFormat(data.format), GLPixelUtil::getGLOriginDataType(data.format),
					data.data);
				break;
		}	
	}
	// Restore defaults
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	if (GLEW_VERSION_1_2)
		glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}
//-----------------------------------------------------------------------------  
void GLTextureBuffer::download(const PixelBox &data)
{
	if(data.getWidth() != getWidth() ||
		data.getHeight() != getHeight() ||
		data.getDepth() != getDepth())
		OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "only download of entire buffer is supported by GL",
		 	"GLTextureBuffer::download");
	glBindTexture( mTarget, mTextureID );
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
		if(data.left > 0 || data.top > 0 || data.front > 0)
			glPixelStorei(GL_PACK_SKIP_PIXELS, data.left + data.rowPitch * data.top + data.slicePitch * data.front);
		if((data.getWidth()*PixelUtil::getNumElemBytes(data.format)) & 3) {
			// Standard alignment of 4 is not right
			glPixelStorei(GL_PACK_ALIGNMENT, 1);
		}
		// We can only get the entire texture
		glGetTexImage(mFaceTarget, mLevel, 
			GLPixelUtil::getGLOriginFormat(data.format), GLPixelUtil::getGLOriginDataType(data.format),
			data.data);
		// Restore defaults
		glPixelStorei(GL_PACK_ROW_LENGTH, 0);
		glPixelStorei(GL_PACK_IMAGE_HEIGHT, 0);
		glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
		glPixelStorei(GL_PACK_ALIGNMENT, 4);
	}
}
//-----------------------------------------------------------------------------  
void GLTextureBuffer::bindToFramebuffer(GLenum attachment, size_t zoffset)
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
        glFramebufferTexture3DEXT(GL_FRAMEBUFFER_EXT, attachment,
                            mFaceTarget, mTextureID, mLevel, zoffset);
        break;
    }
}
//-----------------------------------------------------------------------------
void GLTextureBuffer::copyFromFramebuffer(size_t zoffset)
{
    glBindTexture(mTarget, mTextureID);
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
        glCopyTexSubImage3D(mFaceTarget, mLevel, 0, 0, zoffset, 0, 0, mWidth, mHeight);
        break;
    }
}
//-----------------------------------------------------------------------------  
void GLTextureBuffer::blit(const HardwarePixelBufferSharedPtr &src, const Image::Box &srcBox, const Image::Box &dstBox)
{
    GLTextureBuffer *srct = static_cast<GLTextureBuffer *>(src.getPointer());
    /// Check for FBO support first
    /// Destination texture must be 1D, 2D, 3D, or Cube
    /// Source texture must be 1D, 2D or 3D
    if(GLEW_EXT_framebuffer_object &&
        (srct->mTarget==GL_TEXTURE_1D||srct->mTarget==GL_TEXTURE_2D||srct->mTarget==GL_TEXTURE_3D))
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
void GLTextureBuffer::blitFromTexture(GLTextureBuffer *src, const Image::Box &srcBox, const Image::Box &dstBox)
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
    glBindTexture(src->mTarget, src->mTextureID);
    
    /// Set filtering modes depending on the dimensions and source
    if(srcBox.getWidth()==dstBox.getWidth() &&
        srcBox.getHeight()==dstBox.getHeight() &&
        srcBox.getDepth()==dstBox.getDepth())
    {
        /// Dimensions match -- use nearest filtering (fastest and pixel correct)
        glTexParameteri(src->mTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(src->mTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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
            glTexParameteri(src->mTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);    
        }
        else
        {
            /// Manual mipmaps, stay safe with bilinear filtering so that no
            /// intermipmap leakage occurs.
            glTexParameteri(src->mTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(src->mTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
    }
    /// Clamp to edge (fastest)
    glTexParameteri(src->mTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(src->mTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(src->mTarget, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    
    /// Set origin base level mipmap to make sure we source from the right mip
    /// level.
    glTexParameteri(src->mTarget, GL_TEXTURE_BASE_LEVEL, src->mLevel);
    
    /// Store old binding so it can be restored later
    GLint oldfb;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &oldfb);
    
    /// Set up temporary FBO
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboMan->getTemporaryFBO());
    
    GLuint tempTex = 0;
    if(!fboMan->checkFormat(mFormat))
    {
        /// If target format not directly supported, create intermediate texture
        GLenum tempFormat = GLPixelUtil::getClosestGLInternalFormat(fboMan->getSupportedAlternative(mFormat));
        glGenTextures(1, &tempTex);
        glBindTexture(GL_TEXTURE_2D, tempTex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        /// Allocate temporary texture of the size of the destination area
        glTexImage2D(GL_TEXTURE_2D, 0, tempFormat, 
            GLPixelUtil::optionalPO2(dstBox.getWidth()), GLPixelUtil::optionalPO2(dstBox.getHeight()), 
            0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
            GL_TEXTURE_2D, tempTex, 0);
        /// Set viewport to size of destination slice
        glViewport(0, 0, dstBox.getWidth(), dstBox.getHeight());
    }
    else
    {
        /// We are going to bind directly, so set viewport to size and position of destination slice
        glViewport(dstBox.left, dstBox.top, dstBox.getWidth(), dstBox.getHeight());
    }
    
    /// Process each destination slice
    for(size_t slice=dstBox.front; slice<dstBox.back; ++slice)
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
        w = w * (float)srcBox.getDepth() + srcBox.front;
        /// Normalise to texture coordinate in 0.0 .. 1.0
        w = (w+0.5f) / (float)src->mDepth;
        
        /// Finally we're ready to rumble
        glBindTexture(src->mTarget, src->mTextureID);
        glEnable(src->mTarget);
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
        glDisable(src->mTarget);
        
        if(tempTex)
        {
            /// Copy temporary texture
            glBindTexture(mTarget, mTextureID);
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
            glBindTexture(mTarget, mTextureID);
            glGenerateMipmapEXT(mTarget);
        }
    }

    /// Reset source texture to sane state
    glBindTexture(src->mTarget, src->mTextureID);
    glTexParameteri(src->mTarget, GL_TEXTURE_BASE_LEVEL, 0);
    
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
    glDeleteTextures(1, &tempTex);
}
//-----------------------------------------------------------------------------  
/// blitFromMemory doing hardware trilinear scaling
void GLTextureBuffer::blitFromMemory(const PixelBox &src_orig, const Image::Box &dstBox)
{
    /// Fall back to normal GLHardwarePixelBuffer::blitFromMemory in case 
    /// - FBO is not supported
    /// - Either source or target is luminance due doesn't looks like supported by hardware
    /// - the source dimensions match the destination ones, in which case no scaling is needed
    if(!GLEW_EXT_framebuffer_object ||
        PixelUtil::isLuminance(src_orig.format) ||
        PixelUtil::isLuminance(mFormat) ||
        (src_orig.getWidth() == dstBox.getWidth() &&
        src_orig.getHeight() == dstBox.getHeight() &&
        src_orig.getDepth() == dstBox.getDepth()))
    {
        GLHardwarePixelBuffer::blitFromMemory(src_orig, dstBox);
        return;
    }
    if(!mBuffer.contains(dstBox))
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "destination box out of range",
                    "GLTextureBuffer::blitFromMemory");
    /// For scoped deletion of conversion buffer
    MemoryDataStreamPtr buf;
    PixelBox src;
    
    /// First, convert the srcbox to a OpenGL compatible pixel format
    if(GLPixelUtil::getGLOriginFormat(src_orig.format) == 0)
    {
        /// Convert to buffer internal format
        buf.bind(new MemoryDataStream(
                PixelUtil::getMemorySize(src_orig.getWidth(), src_orig.getHeight(), src_orig.getDepth(),
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
    GLenum target = (src.getDepth()!=1)?GL_TEXTURE_3D:GL_TEXTURE_2D;
    GLsizei width = GLPixelUtil::optionalPO2(src.getWidth());
    GLsizei height = GLPixelUtil::optionalPO2(src.getHeight());
    GLsizei depth = GLPixelUtil::optionalPO2(src.getDepth());
    GLenum format = GLPixelUtil::getClosestGLInternalFormat(src.format);
    
    /// Generate texture name
    glGenTextures(1, &id);
    
    /// Set texture type
    glBindTexture(target, id);
    
    /// Set automatic mipmap generation; nice for minimisation
    glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, 1000 );
    glTexParameteri(target, GL_GENERATE_MIPMAP, GL_TRUE );
    
    /// Allocate texture memory
    if(target == GL_TEXTURE_3D)
        glTexImage3D(target, 0, format, width, height, depth, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    else
        glTexImage2D(target, 0, format, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    /// GL texture buffer
    GLTextureBuffer tex(StringUtil::BLANK, target, id, 0, 0, (Usage)(TU_AUTOMIPMAP|HBU_STATIC_WRITE_ONLY), false, false, 0);
    
    /// Upload data to 0,0,0 in temporary texture
	Image::Box tempTarget(0, 0, 0, src.getWidth(), src.getHeight(), src.getDepth());
    tex.upload(src, tempTarget);
    
    /// Blit
    blitFromTexture(&tex, tempTarget, dstBox);
    
    /// Delete temp texture
    glDeleteTextures(1, &id);
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
GLRenderBuffer::GLRenderBuffer(GLenum format, size_t width, size_t height, GLsizei numSamples):
    GLHardwarePixelBuffer(width, height, 1, GLPixelUtil::getClosestOGREFormat(format),HBU_WRITE_ONLY)
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
void GLRenderBuffer::bindToFramebuffer(GLenum attachment, size_t zoffset)
{
    assert(zoffset < mDepth);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, attachment,
                        GL_RENDERBUFFER_EXT, mRenderbufferID);
}

};
