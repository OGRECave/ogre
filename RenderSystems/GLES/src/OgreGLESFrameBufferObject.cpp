/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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

#include "OgreGLESFrameBufferObject.h"
#include "OgreRoot.h"
#include "OgreGLESHardwarePixelBuffer.h"
#include "OgreGLESFBORenderTexture.h"
#include "OgreGLESDepthBuffer.h"

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

        /// Depth buffer is not handled here anymore.
		/// See GLESFrameBufferObject::attachDepthBuffer() & RenderSystem::setDepthBufferFor()

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
		const GLuint fb = mMultisampleFB ? mMultisampleFB : mFB;
		glBindFramebufferOES(GL_FRAMEBUFFER_OES, fb);
        GL_CHECK_ERROR;
    }

	void GLESFrameBufferObject::swapBuffers()
	{
        // Do nothing
	}

	void GLESFrameBufferObject::attachDepthBuffer( DepthBuffer *depthBuffer )
	{
		GLESDepthBuffer *glDepthBuffer = static_cast<GLESDepthBuffer*>(depthBuffer);

		glBindFramebufferOES(GL_FRAMEBUFFER_OES, mMultisampleFB ? mMultisampleFB : mFB );

		if( glDepthBuffer )
		{
			GLESRenderBuffer *depthBuf   = glDepthBuffer->getDepthBuffer();
			GLESRenderBuffer *stencilBuf = glDepthBuffer->getStencilBuffer();

			//Truly attach depth buffer
			depthBuf->bindToFramebuffer( GL_DEPTH_ATTACHMENT_OES, 0 );

			//Truly attach stencil buffer, if it has one and isn't included w/ the depth buffer
			if( depthBuf != stencilBuf )
				stencilBuf->bindToFramebuffer( GL_STENCIL_ATTACHMENT_OES, 0 );
			else
			{
				glFramebufferRenderbufferOES( GL_FRAMEBUFFER_OES, GL_STENCIL_ATTACHMENT_OES,
											  GL_RENDERBUFFER_OES, 0);
			}
		}
		else
		{
			glFramebufferRenderbufferOES( GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES,
										  GL_RENDERBUFFER_OES, 0);
			glFramebufferRenderbufferOES( GL_FRAMEBUFFER_OES, GL_STENCIL_ATTACHMENT_OES,
										  GL_RENDERBUFFER_OES, 0);
		}
	}
	//-----------------------------------------------------------------------------
	void GLESFrameBufferObject::detachDepthBuffer()
	{
		glBindFramebufferOES(GL_FRAMEBUFFER_OES, mMultisampleFB ? mMultisampleFB : mFB );
		glFramebufferRenderbufferOES( GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, 0 );
		glFramebufferRenderbufferOES( GL_FRAMEBUFFER_OES, GL_STENCIL_ATTACHMENT_OES,
									  GL_RENDERBUFFER_OES, 0 );
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
	GLsizei GLESFrameBufferObject::getFSAA()
    {
        return mNumSamples;
    }
//-----------------------------------------------------------------------------
}
