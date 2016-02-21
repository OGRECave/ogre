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

#include "OgreEAGLESContext.h"
#include "OgreGLESRenderSystem.h"
#include "OgreRoot.h"

namespace Ogre {
    EAGLESContext::EAGLESContext(CAEAGLLayer *drawable, EAGLSharegroup *group)
        : 
        mBackingWidth(0),
        mBackingHeight(0),
        mViewRenderbuffer(0),
        mViewFramebuffer(0),
        mDepthRenderbuffer(0),
        mIsMultiSampleSupported(false),
        mNumSamples(0),
        mFSAAFramebuffer(0),
        mFSAARenderbuffer(0)
    {

        mDrawable = [drawable retain];

        // If the group argument is not NULL, then we assume that an externally created EAGLSharegroup
        // is to be used and a context is created using that group.
        if(group)
        {
            mContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1 sharegroup:group];
        }
        else
        {
            mContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
        }
        
        if (!mContext || ![EAGLContext setCurrentContext:mContext])
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        "Unable to create a suitable EAGLContext",
                        __FUNCTION__);
        }
    }

    EAGLESContext::~EAGLESContext()
    {
        GLESRenderSystem *rs =
            static_cast<GLESRenderSystem*>(Root::getSingleton().getRenderSystem());

        rs->_unregisterContext(this);

        destroyFramebuffer();

        if ([EAGLContext currentContext] == mContext)
        {
            [EAGLContext setCurrentContext:nil];
        }
        
        [mContext release];
        [mDrawable release];
    }

    bool EAGLESContext::createFramebuffer()
    {
        destroyFramebuffer();

        glGenFramebuffersOES(1, &mViewFramebuffer);
        GL_CHECK_ERROR
        glGenRenderbuffersOES(1, &mViewRenderbuffer);
        GL_CHECK_ERROR
        
        glBindFramebufferOES(GL_FRAMEBUFFER_OES, mViewFramebuffer);
        GL_CHECK_ERROR
        glBindRenderbufferOES(GL_RENDERBUFFER_OES, mViewRenderbuffer);
        GL_CHECK_ERROR
        
        if(![mContext renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(id<EAGLDrawable>) mDrawable])
        {
            GL_CHECK_ERROR
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        "Failed to bind the drawable to a renderbuffer object",
                        __FUNCTION__);
            return false;
        }

        glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &mBackingWidth);
        GL_CHECK_ERROR
        glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &mBackingHeight);
        GL_CHECK_ERROR
        glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, mViewRenderbuffer);
        GL_CHECK_ERROR

#if GL_APPLE_framebuffer_multisample
        if(mIsMultiSampleSupported && mNumSamples > 0)
        {
            // Determine how many MSAS samples to use
            GLint maxSamplesAllowed;
            glGetIntegerv(GL_MAX_SAMPLES_APPLE, &maxSamplesAllowed);
            int samplesToUse = (mNumSamples > maxSamplesAllowed) ? maxSamplesAllowed : mNumSamples;
            
            // Create the FSAA framebuffer (offscreen)
            glGenFramebuffersOES(1, &mFSAAFramebuffer);
            GL_CHECK_ERROR
            glBindFramebufferOES(GL_FRAMEBUFFER_OES, mFSAAFramebuffer);
            GL_CHECK_ERROR
            
            /* Create the offscreen MSAA color buffer.
             * After rendering, the contents of this will be blitted into mFSAAFramebuffer */
            glGenRenderbuffersOES(1, &mFSAARenderbuffer);
            GL_CHECK_ERROR
            glBindRenderbufferOES(GL_RENDERBUFFER_OES, mFSAARenderbuffer);
            GL_CHECK_ERROR
            glRenderbufferStorageMultisampleAPPLE(GL_RENDERBUFFER_OES, samplesToUse, GL_RGBA8_OES, mBackingWidth, mBackingHeight);
            GL_CHECK_ERROR
            glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, mFSAARenderbuffer);
            GL_CHECK_ERROR
            
            // Create the FSAA depth buffer
            glGenRenderbuffersOES(1, &mDepthRenderbuffer);
            GL_CHECK_ERROR
            glBindRenderbufferOES(GL_RENDERBUFFER_OES, mDepthRenderbuffer);
            GL_CHECK_ERROR
            glRenderbufferStorageMultisampleAPPLE(GL_RENDERBUFFER_OES, samplesToUse, GL_DEPTH_COMPONENT24_OES, mBackingWidth, mBackingHeight);
            GL_CHECK_ERROR
            glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, mDepthRenderbuffer);
            GL_CHECK_ERROR

            // Validate the FSAA framebuffer
            if(glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES)
            {
                GL_CHECK_ERROR
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                            "Failed to make a complete FSAA framebuffer object",
                            __FUNCTION__);
                return false;
            }
        }
        else
#endif
        {
            glGenRenderbuffersOES(1, &mDepthRenderbuffer);
            GL_CHECK_ERROR
            glBindRenderbufferOES(GL_RENDERBUFFER_OES, mDepthRenderbuffer);
            GL_CHECK_ERROR
            glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, mBackingWidth, mBackingHeight);
            GL_CHECK_ERROR
            glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, mDepthRenderbuffer);
            GL_CHECK_ERROR
        }

        glBindFramebufferOES(GL_FRAMEBUFFER_OES, mViewFramebuffer);
        if(glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES)
        {
            GL_CHECK_ERROR
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        "Failed to make a complete framebuffer object",
                        __FUNCTION__);
            return false;
        }

        return true;
    }

    void EAGLESContext::destroyFramebuffer()
    {
        glDeleteFramebuffersOES(1, &mViewFramebuffer);
        mViewFramebuffer = 0;
        glDeleteRenderbuffersOES(1, &mViewRenderbuffer);
        mViewRenderbuffer = 0;
        
        if(mFSAARenderbuffer)
        {
            glDeleteRenderbuffersOES(1, &mFSAARenderbuffer);
            mFSAARenderbuffer = 0;
        }

        if(mFSAAFramebuffer)
        {
            glDeleteFramebuffersOES(1, &mFSAAFramebuffer);
            mFSAAFramebuffer = 0;
        }
        
        if(mDepthRenderbuffer)
        {
            glDeleteRenderbuffersOES(1, &mDepthRenderbuffer);
            mDepthRenderbuffer = 0;
        }
    }

    void EAGLESContext::setCurrent()
    {
        GLboolean ret = [EAGLContext setCurrentContext:mContext];
        if (!ret)
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        "Failed to make context current",
                        __FUNCTION__);
        }
    }

    void EAGLESContext::endCurrent()
    {
        // Do nothing
    }

    GLESContext * EAGLESContext::clone() const
    {
        return const_cast<EAGLESContext *>(this);
    }

	CAEAGLLayer * EAGLESContext::getDrawable() const
	{
		return mDrawable;
	}

	EAGLContext * EAGLESContext::getContext() const
	{
		return mContext;
	}
}
