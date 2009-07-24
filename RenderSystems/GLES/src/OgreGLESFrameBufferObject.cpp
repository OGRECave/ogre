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

#include "OgreGLESFrameBufferObject.h"
#include "OgreRoot.h"
#include "OgreGLESHardwarePixelBuffer.h"
#include "OgreGLESFBORenderTexture.h"

namespace Ogre {

//-----------------------------------------------------------------------------
    GLESFrameBufferObject::GLESFrameBufferObject(GLESFBOManager *manager, uint fsaa):
        mManager(manager), mNumSamples(fsaa)
    {
        /// Generate framebuffer object
        glGenFramebuffersOES(1, &mFB);
        GL_CHECK_ERROR;

        mNumSamples = 0;
        mMultisampleFB = 0;

        /// Initialise state
        mDepth.buffer=0;
        mStencil.buffer=0;
        for(size_t x=0; x<OGRE_MAX_MULTIPLE_RENDER_TARGETS; ++x)
        {
            mColour[x].buffer=0;
        }
    }
    
    GLESFrameBufferObject::~GLESFrameBufferObject()
    {
        mManager->releaseRenderBuffer(mDepth);
        mManager->releaseRenderBuffer(mStencil);
		mManager->releaseRenderBuffer(mMultisampleColourBuffer);
        /// Delete framebuffer object
        glDeleteFramebuffersOES(1, &mFB);
        GL_CHECK_ERROR;

		if (mMultisampleFB)
			glDeleteFramebuffersOES(1, &mMultisampleFB);

        GL_CHECK_ERROR;
    }
    
    void GLESFrameBufferObject::bindSurface(size_t attachment, const GLESSurfaceDesc &target)
    {
        assert(attachment < OGRE_MAX_MULTIPLE_RENDER_TARGETS);
        mColour[attachment] = target;
		// Re-initialise
		if(mColour[0].buffer)
			initialise();
    }
    
    void GLESFrameBufferObject::unbindSurface(size_t attachment)
    {
        assert(attachment < OGRE_MAX_MULTIPLE_RENDER_TARGETS);
        mColour[attachment].buffer = 0;
		// Re-initialise if buffer 0 still bound
		if(mColour[0].buffer)
			initialise();
    }
    
    void GLESFrameBufferObject::initialise()
    {
		// Release depth and stencil, if they were bound
        mManager->releaseRenderBuffer(mDepth);
        mManager->releaseRenderBuffer(mStencil);
		mManager->releaseRenderBuffer(mMultisampleColourBuffer);
        /// First buffer must be bound
        if(!mColour[0].buffer)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                "Attachment 0 must have surface attached",
                "GLESFrameBufferObject::initialise");
        }

		// If we're doing multisampling, then we need another FBO which contains a
		// renderbuffer which is set up to multisample, and we'll blit it to the final 
		// FBO afterwards to perform the multisample resolve. In that case, the 
		// mMultisampleFB is bound during rendering and is the one with a depth/stencil

        /// Store basic stats
        size_t width = mColour[0].buffer->getWidth();
        size_t height = mColour[0].buffer->getHeight();
        GLuint format = mColour[0].buffer->getGLFormat();
        PixelFormat ogreFormat = mColour[0].buffer->getFormat();

		// Bind simple buffer to add colour attachments
		glBindFramebufferOES(GL_FRAMEBUFFER_OES, mFB);
        GL_CHECK_ERROR;

        /// Bind all attachment points to frame buffer
        for(size_t x=0; x<OGRE_MAX_MULTIPLE_RENDER_TARGETS; ++x)
        {
            if(mColour[x].buffer)
            {
                if(mColour[x].buffer->getWidth() != width || mColour[x].buffer->getHeight() != height)
                {
                    StringStream ss;
                    ss << "Attachment " << x << " has incompatible size ";
                    ss << mColour[x].buffer->getWidth() << "x" << mColour[x].buffer->getHeight();
                    ss << ". It must be of the same as the size of surface 0, ";
                    ss << width << "x" << height;
                    ss << ".";
                    OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, ss.str(), "GLESFrameBufferObject::initialise");
                }
                if(mColour[x].buffer->getGLFormat() != format)
                {
                    StringStream ss;
                    ss << "Attachment " << x << " has incompatible format.";
                    OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, ss.str(), "GLESFrameBufferObject::initialise");
                }
	            mColour[x].buffer->bindToFramebuffer(GL_COLOR_ATTACHMENT0_OES+x, mColour[x].zoffset);
            }
            else
            {
                // Detach
                glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES+x,
                    GL_RENDERBUFFER_OES, 0);
                GL_CHECK_ERROR;
            }
        }

		// Now deal with depth / stencil
		if (mMultisampleFB)
		{
			// Bind multisample buffer
			glBindFramebufferOES(GL_FRAMEBUFFER_OES, mMultisampleFB);
            GL_CHECK_ERROR;

			// Create AA render buffer (colour)
			// note, this can be shared too because we blit it to the final FBO
			// right after the render is finished
			mMultisampleColourBuffer = mManager->requestRenderBuffer(format, width, height, mNumSamples);

			// Attach it, because we won't be attaching below and non-multisample has
			// actually been attached to other FBO
			mMultisampleColourBuffer.buffer->bindToFramebuffer(GL_COLOR_ATTACHMENT0_OES, 
				mMultisampleColourBuffer.zoffset);

			// depth & stencil will be dealt with below
		}

        /// Find suitable depth and stencil format that is compatible with colour format
        GLenum depthFormat, stencilFormat;
        mManager->getBestDepthStencil(ogreFormat, &depthFormat, &stencilFormat);
        
        /// Request surfaces
        mDepth = mManager->requestRenderBuffer(depthFormat, width, height, mNumSamples);
		if (depthFormat == GL_DEPTH24_STENCIL8_EXT)
		{
			// bind same buffer to depth and stencil attachments
            mManager->requestRenderBuffer(mDepth);
			mStencil = mDepth;
		}
		else
		{
			// separate stencil
			mStencil = mManager->requestRenderBuffer(stencilFormat, width, height, mNumSamples);
		}
        
        /// Attach/detach surfaces
        if(mDepth.buffer)
        {
            mDepth.buffer->bindToFramebuffer(GL_DEPTH_ATTACHMENT_OES, mDepth.zoffset);
        }
        else
        {
            glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES,
                GL_RENDERBUFFER_OES, 0);
            GL_CHECK_ERROR;
        }
        if(mStencil.buffer)
        {
            mStencil.buffer->bindToFramebuffer(GL_STENCIL_ATTACHMENT_OES, mStencil.zoffset);
        }
        else
        {
            glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_STENCIL_ATTACHMENT_OES,
                GL_RENDERBUFFER_OES, 0);
            GL_CHECK_ERROR;
        }

		/// Do glDrawBuffer calls
		GLenum bufs[OGRE_MAX_MULTIPLE_RENDER_TARGETS];
		GLsizei n=0;
		for(size_t x=0; x<OGRE_MAX_MULTIPLE_RENDER_TARGETS; ++x)
		{
			// Fill attached colour buffers
			if(mColour[x].buffer)
			{
				bufs[x] = GL_COLOR_ATTACHMENT0_OES + x;
				// Keep highest used buffer + 1
				n = x+1;
			}
			else
			{
				bufs[x] = GL_NONE;
			}
		}

        /// Check status
        GLuint status;
        status = glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES);
        GL_CHECK_ERROR;

        /// Bind main buffer
#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
        // The screen buffer is 1 on iPhone
        glBindFramebufferOES(GL_FRAMEBUFFER_OES, 1);
#else
        glBindFramebufferOES(GL_FRAMEBUFFER_OES, 0);
#endif
        GL_CHECK_ERROR;

        switch(status)
        {
        case GL_FRAMEBUFFER_COMPLETE_OES:
            // All is good
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED_OES:
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                "All framebuffer formats with this texture internal format unsupported",
                "GLESFrameBufferObject::initialise");
        default:
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                "Framebuffer incomplete or other FBO status error",
                "GLESFrameBufferObject::initialise");
        }
        
    }
    
    void GLESFrameBufferObject::bind()
    {
        /// Bind it to FBO
		if (mMultisampleFB)
			glBindFramebufferOES(GL_FRAMEBUFFER_OES, mMultisampleFB);
		else
			glBindFramebufferOES(GL_FRAMEBUFFER_OES, mFB);
        GL_CHECK_ERROR;
    }

	void GLESFrameBufferObject::swapBuffers()
	{
        // Do nothing
	}

    size_t GLESFrameBufferObject::getWidth()
    {
        assert(mColour[0].buffer);
        return mColour[0].buffer->getWidth();
    }
    size_t GLESFrameBufferObject::getHeight()
    {
        assert(mColour[0].buffer);
        return mColour[0].buffer->getHeight();
    }
    PixelFormat GLESFrameBufferObject::getFormat()
    {
        assert(mColour[0].buffer);
        return mColour[0].buffer->getFormat();
    }
//-----------------------------------------------------------------------------
}
