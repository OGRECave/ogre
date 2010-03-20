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

#include "OgreGLES2FrameBufferObject.h"
#include "OgreGLES2HardwarePixelBuffer.h"
#include "OgreGLES2FBORenderTexture.h"
#include "OgreGLES2DepthBuffer.h"

namespace Ogre {

//-----------------------------------------------------------------------------
    GLES2FrameBufferObject::GLES2FrameBufferObject(GLES2FBOManager *manager, uint fsaa):
        mManager(manager), mNumSamples(fsaa)
    {
        /// Generate framebuffer object
        glGenFramebuffers(1, &mFB);
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
    
    GLES2FrameBufferObject::~GLES2FrameBufferObject()
    {
        mManager->releaseRenderBuffer(mDepth);
        mManager->releaseRenderBuffer(mStencil);
		mManager->releaseRenderBuffer(mMultisampleColourBuffer);
        /// Delete framebuffer object
        glDeleteFramebuffers(1, &mFB);
        GL_CHECK_ERROR;

		if (mMultisampleFB)
			glDeleteFramebuffers(1, &mMultisampleFB);

        GL_CHECK_ERROR;
    }
    
    void GLES2FrameBufferObject::bindSurface(size_t attachment, const GLES2SurfaceDesc &target)
    {
        assert(attachment < OGRE_MAX_MULTIPLE_RENDER_TARGETS);
        mColour[attachment] = target;
		// Re-initialise
		if(mColour[0].buffer)
			initialise();
    }
    
    void GLES2FrameBufferObject::unbindSurface(size_t attachment)
    {
        assert(attachment < OGRE_MAX_MULTIPLE_RENDER_TARGETS);
        mColour[attachment].buffer = 0;
		// Re-initialise if buffer 0 still bound
		if(mColour[0].buffer)
			initialise();
    }
    
    void GLES2FrameBufferObject::initialise()
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
                "GLES2FrameBufferObject::initialise");
        }

		// If we're doing multisampling, then we need another FBO which contains a
		// renderbuffer which is set up to multisample, and we'll blit it to the final 
		// FBO afterwards to perform the multisample resolve. In that case, the 
		// mMultisampleFB is bound during rendering and is the one with a depth/stencil

        /// Store basic stats
        size_t width = mColour[0].buffer->getWidth();
        size_t height = mColour[0].buffer->getHeight();
        GLuint format = mColour[0].buffer->getGLFormat();

		// Bind simple buffer to add colour attachments
		glBindFramebuffer(GL_FRAMEBUFFER, mFB);
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
                    OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, ss.str(), "GLES2FrameBufferObject::initialise");
                }
                if(mColour[x].buffer->getGLFormat() != format)
                {
                    StringStream ss;
                    ss << "Attachment " << x << " has incompatible format.";
                    OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, ss.str(), "GLES2FrameBufferObject::initialise");
                }
	            mColour[x].buffer->bindToFramebuffer(GL_COLOR_ATTACHMENT0+x, mColour[x].zoffset);
            }
            else
            {
                // Detach
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+x,
                    GL_RENDERBUFFER, 0);
                GL_CHECK_ERROR;
            }
        }

		// Now deal with depth / stencil
		if (mMultisampleFB)
		{
			// Bind multisample buffer
			glBindFramebuffer(GL_FRAMEBUFFER, mMultisampleFB);
            GL_CHECK_ERROR;

			// Create AA render buffer (colour)
			// note, this can be shared too because we blit it to the final FBO
			// right after the render is finished
			mMultisampleColourBuffer = mManager->requestRenderBuffer(format, width, height, mNumSamples);

			// Attach it, because we won't be attaching below and non-multisample has
			// actually been attached to other FBO
			mMultisampleColourBuffer.buffer->bindToFramebuffer(GL_COLOR_ATTACHMENT0, 
				mMultisampleColourBuffer.zoffset);

			// depth & stencil will be dealt with below
		}

        /// Depth buffer is not handled here anymore.
		/// See GLES2FrameBufferObject::attachDepthBuffer() & RenderSystem::setDepthBufferFor()

		/// Do glDrawBuffer calls
		GLenum bufs[OGRE_MAX_MULTIPLE_RENDER_TARGETS];
		GLsizei n=0;
		for(size_t x=0; x<OGRE_MAX_MULTIPLE_RENDER_TARGETS; ++x)
		{
			// Fill attached colour buffers
			if(mColour[x].buffer)
			{
				bufs[x] = GL_COLOR_ATTACHMENT0 + x;
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
        status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        GL_CHECK_ERROR;

        /// Bind main buffer
#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
        // The screen buffer is 1 on iPhone
        glBindFramebuffer(GL_FRAMEBUFFER, 1);
#else
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
        GL_CHECK_ERROR;

        switch(status)
        {
        case GL_FRAMEBUFFER_COMPLETE:
            // All is good
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                "All framebuffer formats with this texture internal format unsupported",
                "GLES2FrameBufferObject::initialise");
        default:
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                "Framebuffer incomplete or other FBO status error",
                "GLES2FrameBufferObject::initialise");
        }
        
    }
    
    void GLES2FrameBufferObject::bind()
    {
        /// Bind it to FBO
		const GLuint fb = mMultisampleFB ? mMultisampleFB : mFB;
		glBindFramebuffer(GL_FRAMEBUFFER, fb);
        GL_CHECK_ERROR;
    }

	void GLES2FrameBufferObject::swapBuffers()
	{
        // Do nothing
	}

	void GLES2FrameBufferObject::attachDepthBuffer( DepthBuffer *depthBuffer )
	{
		GLES2DepthBuffer *glDepthBuffer = static_cast<GLES2DepthBuffer*>(depthBuffer);

		glBindFramebuffer(GL_FRAMEBUFFER, mMultisampleFB ? mMultisampleFB : mFB );

		if( glDepthBuffer )
		{
			GLES2RenderBuffer *depthBuf   = glDepthBuffer->getDepthBuffer();
			GLES2RenderBuffer *stencilBuf = glDepthBuffer->getStencilBuffer();

			//Truly attach depth buffer
			depthBuf->bindToFramebuffer( GL_DEPTH_ATTACHMENT, 0 );

			//Truly attach stencil buffer, if it has one and isn't included w/ the depth buffer
			if( depthBuf != stencilBuf )
				stencilBuf->bindToFramebuffer( GL_STENCIL_ATTACHMENT, 0 );
			else
			{
				glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
											  GL_RENDERBUFFER, 0);
			}
		}
		else
		{
			glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
										  GL_RENDERBUFFER, 0);
			glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
										  GL_RENDERBUFFER, 0);
		}
	}
	//-----------------------------------------------------------------------------
	void GLES2FrameBufferObject::detachDepthBuffer()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, mMultisampleFB ? mMultisampleFB : mFB );
		glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0 );
		glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
									  GL_RENDERBUFFER, 0 );
	}

    size_t GLES2FrameBufferObject::getWidth()
    {
        assert(mColour[0].buffer);
        return mColour[0].buffer->getWidth();
    }
    size_t GLES2FrameBufferObject::getHeight()
    {
        assert(mColour[0].buffer);
        return mColour[0].buffer->getHeight();
    }
    PixelFormat GLES2FrameBufferObject::getFormat()
    {
        assert(mColour[0].buffer);
        return mColour[0].buffer->getFormat();
    }
	GLsizei GLES2FrameBufferObject::getFSAA()
    {
        return mNumSamples;
    }
//-----------------------------------------------------------------------------
}
