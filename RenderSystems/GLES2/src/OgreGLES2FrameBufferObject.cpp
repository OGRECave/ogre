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

#include "OgreGLES2FrameBufferObject.h"
#include "OgreGLES2HardwarePixelBuffer.h"
#include "OgreGLES2FBORenderTexture.h"
#include "OgreGLES2DepthBuffer.h"
#include "OgreGLUtil.h"
#include "OgreRoot.h"
#include "OgreGLES2RenderSystem.h"
#include "OgreGLNativeSupport.h"

namespace Ogre {

//-----------------------------------------------------------------------------
    GLES2FrameBufferObject::GLES2FrameBufferObject(GLES2FBOManager *manager, uint fsaa):
        GLFrameBufferObjectCommon(fsaa), mManager(manager)
    {
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        GLint oldfb = 0;
        OGRE_CHECK_GL_ERROR(glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldfb));
#endif
        GLES2RenderSystem* rs = getGLES2RenderSystem();
        mContext = rs->_getCurrentContext();
        
        // Generate framebuffer object
        OGRE_CHECK_GL_ERROR(glGenFramebuffers(1, &mFB));

	   if(rs->getCapabilities()->hasCapability(RSC_DEBUG))
       {
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
           OGRE_CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, mFB)); // to avoid GL_INVALID_OPERATION in glLabelObjectEXT(GL_FRAMEBUFFER,...) on iOS 
#endif
           OGRE_CHECK_GL_ERROR(glLabelObjectEXT(GL_FRAMEBUFFER, mFB, 0, ("FBO #" + StringConverter::toString(mFB)).c_str()));
       }

        mNumSamples = std::min(mNumSamples, manager->getMaxFSAASamples());

        // Will we need a second FBO to do multisampling?
        if (mNumSamples)
        {
            OGRE_CHECK_GL_ERROR(glGenFramebuffers(1, &mMultisampleFB));
            if(rs->getCapabilities()->hasCapability(RSC_DEBUG))
            {
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
                OGRE_CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, mMultisampleFB)); // to avoid GL_INVALID_OPERATION in glLabelObjectEXT(GL_FRAMEBUFFER,...) on iOS 
#endif
                OGRE_CHECK_GL_ERROR(glLabelObjectEXT(GL_FRAMEBUFFER, mMultisampleFB, 0, ("MSAA FBO #" + StringConverter::toString(mMultisampleFB)).c_str()));
            }
        }
        else
        {
            mMultisampleFB = 0;
        }

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        OGRE_CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, oldfb));
#endif
    }
    
    GLES2FrameBufferObject::~GLES2FrameBufferObject()
    {
        mManager->releaseRenderBuffer(mDepth);
        mManager->releaseRenderBuffer(mStencil);
        mManager->releaseRenderBuffer(mMultisampleColourBuffer);
        // Delete framebuffer object
        if(mContext && mFB)
        {
            GLRenderSystemCommon* rs = static_cast<GLRenderSystemCommon*>(Root::getSingleton().getRenderSystem());
            rs->_destroyFbo(mContext, mFB);
            
            if (mMultisampleFB)
                rs->_destroyFbo(mContext, mMultisampleFB);
        }
    }
    
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID || OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
    void GLES2FrameBufferObject::notifyOnContextLost()
    {
        mManager->releaseRenderBuffer(mDepth);
        mManager->releaseRenderBuffer(mStencil);
        mManager->releaseRenderBuffer(mMultisampleColourBuffer);
        
        OGRE_CHECK_GL_ERROR(glDeleteFramebuffers(1, &mFB));
        
        if (mMultisampleFB)
            OGRE_CHECK_GL_ERROR(glDeleteFramebuffers(1, &mMultisampleFB));
    }
    
    void GLES2FrameBufferObject::notifyOnContextReset(const GLSurfaceDesc &target)
    {
        // Generate framebuffer object
        OGRE_CHECK_GL_ERROR(glGenFramebuffers(1, &mFB));
        OGRE_CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, mFB));

        // Bind target to surface 0 and initialise
        bindSurface(0, target);
    }
#endif
    
    void GLES2FrameBufferObject::initialise()
    {
        GLES2RenderSystem* rs = getGLES2RenderSystem();
        assert(mContext == rs->_getCurrentContext());
        
        // Release depth and stencil, if they were bound
        mManager->releaseRenderBuffer(mDepth);
        mManager->releaseRenderBuffer(mStencil);
        mManager->releaseRenderBuffer(mMultisampleColourBuffer);
        // First buffer must be bound
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

        // Store basic stats
        uint32 width = mColour[0].buffer->getWidth();
        uint32 height = mColour[0].buffer->getHeight();
        GLuint format = mColour[0].buffer->getGLFormat();
        ushort maxSupportedMRTs = rs->getCapabilities()->getNumMultiRenderTargets();

        // Bind simple buffer to add colour attachments
        OGRE_CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, mFB));

        bool isDepth = PixelUtil::isDepth(getFormat());

        // Bind all attachment points to frame buffer
        for(unsigned int x = 0; x < maxSupportedMRTs; ++x)
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

                mColour[x].buffer->bindToFramebuffer(
                    isDepth ? GL_DEPTH_ATTACHMENT : (GL_COLOR_ATTACHMENT0 + x), mColour[x].zoffset);
            }
            else
            {
                // Detach
                OGRE_CHECK_GL_ERROR(glFramebufferRenderbuffer(GL_FRAMEBUFFER, static_cast<GLenum>(GL_COLOR_ATTACHMENT0+x), GL_RENDERBUFFER, 0));
            }
        }

        // Now deal with depth / stencil
        if (mMultisampleFB)
        {
            // Bind multisample buffer
            OGRE_CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, mMultisampleFB));

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

        // Depth buffer is not handled here anymore.
        // See GLES2FrameBufferObject::attachDepthBuffer() & RenderSystem::setDepthBufferFor()

        if(rs->hasMinGLVersion(3, 0) && OGRE_PLATFORM != OGRE_PLATFORM_EMSCRIPTEN) // ED on Emscripten
        {
            GLenum bufs[OGRE_MAX_MULTIPLE_RENDER_TARGETS];
            GLsizei n=0;
            for(unsigned int x=0; x<maxSupportedMRTs; ++x)
            {
                // Fill attached colour buffers
                if(mColour[x].buffer)
                {
                    bufs[x] = isDepth ? GL_DEPTH_ATTACHMENT : (GL_COLOR_ATTACHMENT0 + x);
                    // Keep highest used buffer + 1
                    n = x+1;
                }
                else
                {
                    bufs[x] = GL_NONE;
                }
            }

            // Drawbuffer extension supported, use it
            if(!isDepth)
                OGRE_CHECK_GL_ERROR(glDrawBuffers(n, bufs));
        }
        // Check status
        GLuint status;
        OGRE_CHECK_GL_ERROR(status = glCheckFramebufferStatus(GL_FRAMEBUFFER));

        // Bind main buffer
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        // The screen buffer is 1 on iOS
        OGRE_CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, 1));
#else
        OGRE_CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, 0));
#endif

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
    
    bool GLES2FrameBufferObject::bind(bool recreateIfNeeded)
    {
        GLRenderSystemCommon* rs = static_cast<GLRenderSystemCommon*>(Root::getSingleton().getRenderSystem());
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
#ifdef DEBUG
            if(Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_DEBUG))
            {
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
                OGRE_CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, mFB)); // to avoid GL_INVALID_OPERATION in glLabelObjectEXT(GL_FRAMEBUFFER,...) on iOS
#endif
                OGRE_CHECK_GL_ERROR(glLabelObjectEXT(GL_FRAMEBUFFER, mFB, 0, ("FBO ##" + StringConverter::toString(mFB)).c_str()));
            }
#endif
            
            // Will we need a second FBO to do multisampling?
            if (mNumSamples)
            {
                OGRE_CHECK_GL_ERROR(glGenFramebuffers(1, &mMultisampleFB));
#ifdef DEBUG
                if(Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_DEBUG))
                {
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
                    OGRE_CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, mMultisampleFB)); // to avoid GL_INVALID_OPERATION in glLabelObjectEXT(GL_FRAMEBUFFER,...) on iOS
#endif
                    OGRE_CHECK_GL_ERROR(glLabelObjectEXT(GL_FRAMEBUFFER, mMultisampleFB, 0, ("MSAA FBO ##" + StringConverter::toString(mMultisampleFB)).c_str()));
                }
#endif
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
            OGRE_CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, mMultisampleFB ? mMultisampleFB : mFB));

        return mContext != 0;
    }

    void GLES2FrameBufferObject::swapBuffers()
    {
        if (mMultisampleFB)
        {
            GLint oldfb = 0;
            OGRE_CHECK_GL_ERROR(glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldfb));

            // Blit from multisample buffer to final buffer, triggers resolve
            uint32 width = mColour[0].buffer->getWidth();
            uint32 height = mColour[0].buffer->getHeight();
            OGRE_CHECK_GL_ERROR(glBindFramebuffer(GL_READ_FRAMEBUFFER, mMultisampleFB));
            OGRE_CHECK_GL_ERROR(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFB));
            OGRE_CHECK_GL_ERROR(glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST));
            // Unbind
            OGRE_CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, oldfb));
        }
    }

    void GLES2FrameBufferObject::attachDepthBuffer( DepthBuffer *depthBuffer )
    {
        bind(true); // recreate FBO if unusable with current context, bind it

        GLES2DepthBuffer *glDepthBuffer = static_cast<GLES2DepthBuffer*>(depthBuffer);

        if( glDepthBuffer )
        {
            auto *depthBuf   = glDepthBuffer->getDepthBuffer();
            auto *stencilBuf = glDepthBuffer->getStencilBuffer();

            //Attach depth buffer, if it has one.
            if( depthBuf )
                depthBuf->bindToFramebuffer( GL_DEPTH_ATTACHMENT, 0 );

            //Attach stencil buffer, if it has one.
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
    //-----------------------------------------------------------------------------
    void GLES2FrameBufferObject::detachDepthBuffer()
    {
        if(bind(false)) // bind or destroy FBO if unusable with current context
        {
            OGRE_CHECK_GL_ERROR(glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0 ));
            OGRE_CHECK_GL_ERROR(glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0 ));
        }
    }
//-----------------------------------------------------------------------------
}
