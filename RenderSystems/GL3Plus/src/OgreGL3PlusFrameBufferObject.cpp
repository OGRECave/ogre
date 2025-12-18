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

#include "OgreGL3PlusFrameBufferObject.h"
#include "OgreGL3PlusHardwarePixelBuffer.h"
#include "OgreGL3PlusFBORenderTexture.h"
#include "OgreGLDepthBufferCommon.h"
#include "OgreGL3PlusRenderSystem.h"
#include "OgreRoot.h"
#include <sstream>

namespace Ogre {

GL3PlusFrameBufferObject::GL3PlusFrameBufferObject()
    : GLFrameBufferObjectCommon()
{
}
    
    GL3PlusFrameBufferObject::~GL3PlusFrameBufferObject()
    {
        mRTTManager->releaseRenderBuffer(mDepth);
        mRTTManager->releaseRenderBuffer(mStencil);
        // Delete framebuffer object
        if(mContext && mFB)
        {
            GLRenderSystemCommon* rs = static_cast<GLRenderSystemCommon*>(Root::getSingleton().getRenderSystem());
            rs->_destroyFbo(mContext, mFB);
            
            if (mMultisampleFB)
                rs->_destroyFbo(mContext, mMultisampleFB);
        }
    }
    
    void GL3PlusFrameBufferObject::initialise()
    {
        assert(mContext == (static_cast<GLRenderSystemCommon*>(Root::getSingleton().getRenderSystem()))->_getCurrentContext());

        // Release depth and stencil, if they were bound
        mRTTManager->releaseRenderBuffer(mDepth);
        mRTTManager->releaseRenderBuffer(mStencil);
        for(int i = 0; i < OGRE_MAX_MULTIPLE_RENDER_TARGETS; ++i)
            mRTTManager->releaseRenderBuffer(mMultisampleColourBuffer[i]);
        // First buffer must be bound
        if(!mColour[0].buffer)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                "Attachment 0 must have surface attached",
                "GL3PlusFrameBufferObject::initialise");
        }

        // If we're doing multisampling, then we need another FBO which contains a
        // renderbuffer which is set up to multisample, and we'll blit it to the final 
        // FBO afterwards to perform the multisample resolve. In that case, the 
        // mMultisampleFB is bound during rendering and is the one with a depth/stencil

        // Store basic stats
        uint32 width = mColour[0].buffer->getWidth();
        uint32 height = mColour[0].buffer->getHeight();
        ushort maxSupportedMRTs = Root::getSingleton().getRenderSystem()->getCapabilities()->getNumMultiRenderTargets();

        // Bind simple buffer to add colour attachments
        auto rs = static_cast<GL3PlusRenderSystem*>(Root::getSingleton().getRenderSystem());
        rs->_getStateCacheManager()->bindGLFrameBuffer( GL_FRAMEBUFFER, mFB );

        // Bind all attachment points to frame buffer
        for(unsigned int x = 0; x < maxSupportedMRTs; ++x)
        {
            if(mColour[x].buffer)
            {
                bool isDepth = PixelUtil::isDepth(mColour[x].buffer->getFormat());

                if(mColour[x].buffer->getWidth() != width || mColour[x].buffer->getHeight() != height)
                {
                    StringStream ss;
                    ss << "Attachment " << x << " has incompatible size ";
                    ss << mColour[x].buffer->getWidth() << "x" << mColour[x].buffer->getHeight();
                    ss << ". It must be of the same as the size of surface 0, ";
                    ss << width << "x" << height;
                    ss << ".";
                    OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, ss.str(), "GL3PlusFrameBufferObject::initialise");
                }

                mColour[x].buffer->bindToFramebuffer(
                    isDepth ? GL_DEPTH_ATTACHMENT : (GL_COLOR_ATTACHMENT0 + x), mColour[x].zoffset);
            }
            else
            {
                // Detach
                OGRE_CHECK_GL_ERROR(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+x, GL_RENDERBUFFER, 0));
            }
        }

        if (mMultisampleFB)
        {
            // Bind multisample buffer
            rs->_getStateCacheManager()->bindGLFrameBuffer( GL_FRAMEBUFFER, mMultisampleFB );

            for(unsigned int x = 0; x < maxSupportedMRTs; ++x)
            {
                if(!mColour[x].buffer)
                    continue;
                createAndBindRenderBuffer(mColour[x].buffer->getGLFormat(), width, height, x);
            }
        }

        // Depth buffer is not handled here anymore.
        // See GL3PlusFrameBufferObject::attachDepthBuffer() & RenderSystem::setDepthBufferFor()

        // Do glDrawBuffer calls
        GLenum bufs[OGRE_MAX_MULTIPLE_RENDER_TARGETS];
        GLsizei n=0;
        for(unsigned int x=0; x<maxSupportedMRTs; ++x)
        {
            // Fill attached colour buffers
            if(mColour[x].buffer)
            {
                bool isDepth = PixelUtil::isDepth(mColour[x].buffer->getFormat());

                bufs[x] = isDepth ? GL_NONE : (GL_COLOR_ATTACHMENT0 + x);
                // Keep highest used buffer + 1
                if(!isDepth)
                    n = x+1;
            }
            else
            {
                bufs[x] = GL_NONE;
            }
        }

        // Drawbuffer extension supported, use it
        OGRE_CHECK_GL_ERROR(glDrawBuffers(n, bufs));
        
        // Check status
        GLuint status;
        OGRE_CHECK_GL_ERROR(status = glCheckFramebufferStatus(GL_FRAMEBUFFER));

        switch(status)
        {
        case GL_FRAMEBUFFER_COMPLETE:
            // All is good
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                "All framebuffer formats with this texture internal format unsupported",
                "GL3PlusFrameBufferObject::initialise");
        default:
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                "Framebuffer incomplete or other FBO status error",
                "GL3PlusFrameBufferObject::initialise");
        }
        
    }
    
    bool GL3PlusFrameBufferObject::bind(bool recreateIfNeeded)
    {
        auto rs = static_cast<GL3PlusRenderSystem*>(Root::getSingleton().getRenderSystem());
        GLContext* currentContext = rs->_getCurrentContext();
        if(mContext && mContext != currentContext) // FBO is unusable with current context, destroy it
        {
            if(mFB != 0)
                rs->_destroyFbo(mContext, mFB);
            if(mMultisampleFB != 0)
                rs->_destroyFbo(mContext, mMultisampleFB);
            
            mContext = 0;
            mFB = 0;
            mMultisampleFB = 0;
        }

        if(!mContext && recreateIfNeeded) // create FBO lazy or recreate after destruction
        {
            mContext = currentContext;
            
            // Generate framebuffer object
            OGRE_CHECK_GL_ERROR(glGenFramebuffers(1, &mFB));
            
            // Check samples supported
            rs->_getStateCacheManager()->bindGLFrameBuffer( GL_FRAMEBUFFER, mFB );
            
            GLint maxSamples;
            OGRE_CHECK_GL_ERROR(glGetIntegerv(GL_MAX_SAMPLES, &maxSamples));
            mNumSamples = std::min(mNumSamples, (GLsizei)maxSamples);
            
            // Will we need a second FBO to do multisampling?
            if (mNumSamples)
            {
                OGRE_CHECK_GL_ERROR(glGenFramebuffers(1, &mMultisampleFB));
            }
            else
            {
                mMultisampleFB = 0;
            }
            
            // Re-initialise
            if(mColour[0].buffer)
                initialise();
        }

        if(mContext)
	        rs->_getStateCacheManager()->bindGLFrameBuffer(GL_FRAMEBUFFER, mMultisampleFB ? mMultisampleFB : mFB);

        return mContext != 0;
    }

    void GL3PlusFrameBufferObject::swapBuffers()
    {
        if (mMultisampleFB)
        {
            auto rs = static_cast<GL3PlusRenderSystem*>(Root::getSingleton().getRenderSystem());
            GLint oldfb = 0;
            OGRE_CHECK_GL_ERROR(glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldfb));

            // Blit from multisample buffer to final buffer, triggers resolve
            uint32 width = mColour[0].buffer->getWidth();
            uint32 height = mColour[0].buffer->getHeight();
            rs->_getStateCacheManager()->bindGLFrameBuffer( GL_READ_FRAMEBUFFER, mMultisampleFB );
            rs->_getStateCacheManager()->bindGLFrameBuffer( GL_DRAW_FRAMEBUFFER, mFB );

            std::vector<GLenum> invalidateAttachments = { GL_DEPTH_ATTACHMENT };

            for(unsigned int x=0; x<OGRE_MAX_MULTIPLE_RENDER_TARGETS; ++x)
            {
                if(!mColour[x].buffer)
                    continue;
                OGRE_CHECK_GL_ERROR(glReadBuffer((GL_COLOR_ATTACHMENT0 + x)));
                OGRE_CHECK_GL_ERROR(glDrawBuffer((GL_COLOR_ATTACHMENT0 + x)));
                OGRE_CHECK_GL_ERROR(glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST));
                invalidateAttachments.push_back(GL_COLOR_ATTACHMENT0 + x);
            }

            if (rs->hasMinGLVersion(4, 3))
            {
                OGRE_CHECK_GL_ERROR(glInvalidateFramebuffer(GL_READ_FRAMEBUFFER, invalidateAttachments.size(),
                                                            invalidateAttachments.data()));
            }

            // Unbind
            rs->_getStateCacheManager()->bindGLFrameBuffer( GL_FRAMEBUFFER, oldfb );
        }
    }

    void GL3PlusFrameBufferObject::attachDepthBuffer( DepthBuffer *depthBuffer )
    {
        bind(true); // recreate FBO if unusable with current context, bind it

        GLDepthBufferCommon *glDepthBuffer = static_cast<GLDepthBufferCommon*>(depthBuffer);
        if( glDepthBuffer )
        {
            auto *depthBuf   = glDepthBuffer->getDepthBuffer();
            auto *stencilBuf = glDepthBuffer->getStencilBuffer();

            // Attach depth buffer, if it has one.
            if( depthBuf )
                depthBuf->bindToFramebuffer( GL_DEPTH_ATTACHMENT, 0 );
            // Attach stencil buffer, if it has one.
            if( stencilBuf )
                stencilBuf->bindToFramebuffer( GL_STENCIL_ATTACHMENT, 0 );
        }
        else
        {
            OGRE_CHECK_GL_ERROR(glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                                          GL_RENDERBUFFER, 0));
            OGRE_CHECK_GL_ERROR(glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                                                          GL_RENDERBUFFER, 0));
        }
    }
    
    void GL3PlusFrameBufferObject::detachDepthBuffer()
    {
        if(bind(false)) // bind or destroy FBO if it is unusable with current context
        {
            OGRE_CHECK_GL_ERROR(glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0 ));
            OGRE_CHECK_GL_ERROR(glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0 ));
        }
    }

}
