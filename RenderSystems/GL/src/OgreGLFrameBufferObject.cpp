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

#include "OgreGLFrameBufferObject.h"
#include "OgreGLPixelFormat.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreRoot.h"
#include "OgreGLHardwarePixelBuffer.h"
#include "OgreGLFBORenderTexture.h"
#include "OgreGLDepthBufferCommon.h"
#include "OgreGLRenderSystemCommon.h"

namespace Ogre {

//-----------------------------------------------------------------------------
GLFrameBufferObject::GLFrameBufferObject(GLFBOManager* manager, uint fsaa)
    : GLFrameBufferObjectCommon(fsaa, *manager), mManager(manager)
{
    // Generate framebuffer object
    glGenFramebuffersEXT(1, &mFB);
    // check multisampling
    if (GLAD_GL_EXT_framebuffer_blit && GLAD_GL_EXT_framebuffer_multisample)
    {
        // check samples supported
        GLint maxSamples;
        glGetIntegerv(GL_MAX_SAMPLES_EXT, &maxSamples);
        mNumSamples = std::min(mNumSamples, (GLsizei)maxSamples);
    }
    else
    {
        mNumSamples = 0;
    }
    // will we need a second FBO to do multisampling?
    if (mNumSamples)
    {
        glGenFramebuffersEXT(1, &mMultisampleFB);
    }
    else
    {
        mMultisampleFB = 0;
    }
    }
    GLFrameBufferObject::~GLFrameBufferObject()
    {
        mManager->releaseRenderBuffer(mDepth);
        mManager->releaseRenderBuffer(mStencil);
        // Delete framebuffer object
        glDeleteFramebuffersEXT(1, &mFB);        
        if (mMultisampleFB)
            glDeleteFramebuffersEXT(1, &mMultisampleFB);

    }
    void GLFrameBufferObject::initialise()
    {
        // Release depth and stencil, if they were bound
        mManager->releaseRenderBuffer(mDepth);
        mManager->releaseRenderBuffer(mStencil);
        releaseMultisampleColourBuffer();

        // First buffer must be bound
        if(!mColour[0].buffer)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
            "Attachment 0 must have surface attached",
            "GLFrameBufferObject::initialise");
        }

        // If we're doing multisampling, then we need another FBO which contains a
        // renderbuffer which is set up to multisample, and we'll blit it to the final 
        // FBO afterwards to perform the multisample resolve. In that case, the 
        // mMultisampleFB is bound during rendering and is the one with a depth/stencil

        // Store basic stats
        uint32 width = mColour[0].buffer->getWidth();
        uint32 height = mColour[0].buffer->getHeight();
        GLuint format = mColour[0].buffer->getGLFormat();

        auto rsc = Root::getSingleton().getRenderSystem()->getCapabilities();
        ushort maxSupportedMRTs = rsc->getNumMultiRenderTargets();

        // Bind simple buffer to add colour attachments
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFB);

        // Bind all attachment points to frame buffer
        for(unsigned int x=0; x<maxSupportedMRTs; ++x)
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
                    OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, ss.str(), "GLFrameBufferObject::initialise");
                }

                bool isDepth = PixelUtil::isDepth(mColour[x].buffer->getFormat());
                mColour[x].buffer->bindToFramebuffer(
                    isDepth ? GL_DEPTH_ATTACHMENT_EXT : (GL_COLOR_ATTACHMENT0_EXT + x), mColour[x].zoffset);
            }
            else
            {
                // Detach
                glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT+x,
                    GL_RENDERBUFFER_EXT, 0);
            }
        }

        // Now deal with depth / stencil
        if (mMultisampleFB && !PixelUtil::isDepth(getFormat()))
        {
            // Bind multisample buffer
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mMultisampleFB);

            // Create AA render buffer (colour)
            // note, this can be shared too because we blit it to the final FBO
            // right after the render is finished
            initialiseMultisampleColourBuffer(format, width, height);

            // Attach it, because we won't be attaching below and non-multisample has
            // actually been attached to other FBO
            mMultisampleColourBuffer.buffer->bindToFramebuffer(GL_COLOR_ATTACHMENT0_EXT, 
                mMultisampleColourBuffer.zoffset);

            // depth & stencil will be dealt with below

        }

        // Depth buffer is not handled here anymore.
        // See GLFrameBufferObject::attachDepthBuffer() & RenderSystem::setDepthBufferFor()

        // Do glDrawBuffer calls
        GLenum bufs[OGRE_MAX_MULTIPLE_RENDER_TARGETS];
        GLsizei n=0;
        for(unsigned int x=0; x<OGRE_MAX_MULTIPLE_RENDER_TARGETS; ++x)
        {
            // Fill attached colour buffers
            if(mColour[x].buffer)
            {
                bool isDepth = PixelUtil::isDepth(mColour[x].buffer->getFormat());
                bufs[x] = isDepth ? GL_NONE : (GL_COLOR_ATTACHMENT0_EXT + x);
                // Keep highest used buffer + 1
                if(!isDepth)
                    n = x+1;
            }
            else
            {
                bufs[x] = GL_NONE;
            }
        }

        if(glDrawBuffers)
            // Drawbuffer extension supported, use it
            glDrawBuffers(n, bufs);
        else
            // In this case, the capabilities will not show more than 1 simultaneaous render target.
            glDrawBuffer(bufs[0]);
        
        // Check status
        GLuint status;
        status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
        
        // Bind main buffer
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
        
        switch(status)
        {
        case GL_FRAMEBUFFER_COMPLETE_EXT:
            // All is good
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
            "All framebuffer formats with this texture internal format unsupported",
            "GLFrameBufferObject::initialise");
        default:
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
            "Framebuffer incomplete or other FBO status error",
            "GLFrameBufferObject::initialise");
        }
        
    }
    bool GLFrameBufferObject::bind(bool recreateIfNeeded)
    {
        // Bind it to FBO
        const GLuint fb = mMultisampleFB ? mMultisampleFB : mFB;
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
        return mContext != 0;
    }

    void GLFrameBufferObject::swapBuffers()
    {
        if (mMultisampleFB)
        {
            GLint oldfb = 0;
            glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &oldfb);

            // Blit from multisample buffer to final buffer, triggers resolve
            uint32 width = mColour[0].buffer->getWidth();
            uint32 height = mColour[0].buffer->getHeight();
            glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, mMultisampleFB);
            glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, mFB);
            glBlitFramebufferEXT(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

            // Unbind
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, oldfb);
        }
    }

    void GLFrameBufferObject::attachDepthBuffer( DepthBuffer *depthBuffer )
    {
        auto glDepthBuffer = static_cast<GLDepthBufferCommon*>(depthBuffer);

        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mMultisampleFB ? mMultisampleFB : mFB );

        if( glDepthBuffer )
        {
            auto *depthBuf   = glDepthBuffer->getDepthBuffer();
            auto *stencilBuf = glDepthBuffer->getStencilBuffer();

            // Attach depth buffer, if it has one.
            if( depthBuf )
                depthBuf->bindToFramebuffer( GL_DEPTH_ATTACHMENT_EXT, 0 );

            // Attach stencil buffer, if it has one.
            if( stencilBuf )
                stencilBuf->bindToFramebuffer( GL_STENCIL_ATTACHMENT_EXT, 0 );
        }
        else
        {
            glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                          GL_RENDERBUFFER_EXT, 0);
            glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
                                          GL_RENDERBUFFER_EXT, 0);
        }
    }
    //-----------------------------------------------------------------------------
    void GLFrameBufferObject::detachDepthBuffer()
    {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mMultisampleFB ? mMultisampleFB : mFB );
        glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, 0 );
        glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
                                      GL_RENDERBUFFER_EXT, 0 );
    }
}
