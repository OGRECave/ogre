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

#include "OgreEAGLES2Context.h"
#include "OgreEAGL2Support.h"
#include "OgreGLES2RenderSystem.h"
#include "OgreRoot.h"

namespace Ogre {
    EAGLES2Context::EAGLES2Context(CAEAGLLayer *drawable, EAGLSharegroup *group)
        : 
        mBackingWidth(0),
        mBackingHeight(0),
        mViewRenderbuffer(0),
        mViewFramebuffer(0),
        mDepthRenderbuffer(0),
        mIsMultiSampleSupported(false),
        mNumSamples(0),
        mSampleFramebuffer(0),
        mSampleRenderbuffer(0)
    {

        mDrawable = [drawable retain];

#if OGRE_NO_GLES3_SUPPORT == 0
        EAGLRenderingAPI renderingAPI = kEAGLRenderingAPIOpenGLES3;
#else
        EAGLRenderingAPI renderingAPI = kEAGLRenderingAPIOpenGLES2;
#endif
        // If the group argument is not NULL, then we assume that an externally created EAGLSharegroup
        // is to be used and a context is created using that group.
        if(group)
        {
            mContext = [[EAGLContext alloc] initWithAPI:renderingAPI sharegroup:group];
        }
        else
        {
            mContext = [[EAGLContext alloc] initWithAPI:renderingAPI];
        }

        if (!mContext || ![EAGLContext setCurrentContext:mContext])
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        "Unable to create a suitable EAGLContext",
                        __FUNCTION__);
        }

#ifdef __IPHONE_7_1
        OGRE_IF_IOS_VERSION_IS_GREATER_THAN(7.1)
        {
            [mContext setMultiThreaded:YES];
        }
#endif
    }

    EAGLES2Context::~EAGLES2Context()
    {
        GLES2RenderSystem *rs =
            static_cast<GLES2RenderSystem*>(Root::getSingleton().getRenderSystem());

        rs->_unregisterContext(this);

        destroyFramebuffer();

        if ([EAGLContext currentContext] == mContext)
        {
            [EAGLContext setCurrentContext:nil];
        }
        
        [mContext release];
        [mDrawable release];
    }

    void EAGLES2Context::bindSampleFramebuffer()
    {
        if(mIsMultiSampleSupported && mNumSamples > 0)
        {
            // Bind the FSAA buffer if we're doing multisampling
            OGRE_CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, mSampleFramebuffer));
        }
    }

    bool EAGLES2Context::createFramebuffer()
    {
        destroyFramebuffer();

        OGRE_CHECK_GL_ERROR(glGenFramebuffers(1, &mViewFramebuffer));
        OGRE_CHECK_GL_ERROR(glGenRenderbuffers(1, &mViewRenderbuffer));
        OGRE_CHECK_GL_ERROR(glLabelObjectEXT(GL_BUFFER_OBJECT_EXT, mViewFramebuffer, 0, "View Framebuffer"));
        OGRE_CHECK_GL_ERROR(glLabelObjectEXT(GL_BUFFER_OBJECT_EXT, mViewRenderbuffer, 0, "View Renderbuffer"));

        OGRE_CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, mViewFramebuffer));
        OGRE_CHECK_GL_ERROR(glBindRenderbuffer(GL_RENDERBUFFER, mViewRenderbuffer));

        if(![mContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:(id<EAGLDrawable>) mDrawable])
        {
            glGetError();
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        "Failed to bind the drawable to a renderbuffer object",
                        __FUNCTION__);
            return false;
        }

        OGRE_CHECK_GL_ERROR(glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &mBackingWidth));
        OGRE_CHECK_GL_ERROR(glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &mBackingHeight));
        OGRE_CHECK_GL_ERROR(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, mViewRenderbuffer));

#if GL_APPLE_framebuffer_multisample || OGRE_NO_GLES3_SUPPORT == 0
        if(mIsMultiSampleSupported && mNumSamples > 0)
        {
            // Determine how many MSAA samples to use
            GLint maxSamplesAllowed;
            glGetIntegerv(GL_MAX_SAMPLES_APPLE, &maxSamplesAllowed);
            int samplesToUse = (mNumSamples > maxSamplesAllowed) ? maxSamplesAllowed : mNumSamples;
            
            // Create the FSAA framebuffer (offscreen)
            OGRE_CHECK_GL_ERROR(glGenFramebuffers(1, &mSampleFramebuffer));
            OGRE_CHECK_GL_ERROR(glLabelObjectEXT(GL_BUFFER_OBJECT_EXT, mSampleFramebuffer, 0, "FSAA Framebuffer"));
            OGRE_CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, mSampleFramebuffer));

            /* Create the offscreen MSAA color buffer.
             * After rendering, the contents of this will be blitted into mFSAAFramebuffer */
            OGRE_CHECK_GL_ERROR(glGenRenderbuffers(1, &mSampleRenderbuffer));
            OGRE_CHECK_GL_ERROR(glLabelObjectEXT(GL_BUFFER_OBJECT_EXT, mSampleRenderbuffer, 0, "FSAA Colour Renderbuffer"));
            OGRE_CHECK_GL_ERROR(glBindRenderbuffer(GL_RENDERBUFFER, mSampleRenderbuffer));
            OGRE_CHECK_GL_ERROR(glRenderbufferStorageMultisampleAPPLE(GL_RENDERBUFFER, samplesToUse, GL_RGBA8_OES, mBackingWidth, mBackingHeight));
            OGRE_CHECK_GL_ERROR(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, mSampleRenderbuffer));

            // Create the FSAA depth buffer
            OGRE_CHECK_GL_ERROR(glGenRenderbuffers(1, &mDepthRenderbuffer));
            OGRE_CHECK_GL_ERROR(glLabelObjectEXT(GL_BUFFER_OBJECT_EXT, mDepthRenderbuffer, 0, "Depth Renderbuffer"));
            OGRE_CHECK_GL_ERROR(glBindRenderbuffer(GL_RENDERBUFFER, mDepthRenderbuffer));
            OGRE_CHECK_GL_ERROR(glRenderbufferStorageMultisampleAPPLE(GL_RENDERBUFFER, samplesToUse, GL_DEPTH_COMPONENT16, mBackingWidth, mBackingHeight));
            OGRE_CHECK_GL_ERROR(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthRenderbuffer));

            // Validate the FSAA framebuffer
            if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                glGetError();
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                            "Failed to make a complete FSAA framebuffer object",
                            __FUNCTION__);
                return false;
            }
        }
        else
#endif
        {
            OGRE_CHECK_GL_ERROR(glGenRenderbuffers(1, &mDepthRenderbuffer));
            OGRE_CHECK_GL_ERROR(glLabelObjectEXT(GL_BUFFER_OBJECT_EXT, mDepthRenderbuffer, 0, "Depth Renderbuffer"));
            OGRE_CHECK_GL_ERROR(glBindRenderbuffer(GL_RENDERBUFFER, mDepthRenderbuffer));
            OGRE_CHECK_GL_ERROR(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, mBackingWidth, mBackingHeight));
            OGRE_CHECK_GL_ERROR(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthRenderbuffer));
        }

        OGRE_CHECK_GL_ERROR(glBindRenderbuffer(GL_RENDERBUFFER, mViewRenderbuffer));
        OGRE_CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, mViewFramebuffer));
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            glGetError();
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        "Failed to make a complete framebuffer object",
                        __FUNCTION__);
            return false;
        }

        return true;
    }

    void EAGLES2Context::destroyFramebuffer()
    {
        OGRE_CHECK_GL_ERROR(glDeleteFramebuffers(1, &mViewFramebuffer));
        mViewFramebuffer = 0;
        OGRE_CHECK_GL_ERROR(glDeleteRenderbuffers(1, &mViewRenderbuffer));
        mViewRenderbuffer = 0;
        
        if(mSampleRenderbuffer)
        {
            OGRE_CHECK_GL_ERROR(glDeleteRenderbuffers(1, &mSampleRenderbuffer));
            mSampleRenderbuffer = 0;
        }

        if(mSampleFramebuffer)
        {
            OGRE_CHECK_GL_ERROR(glDeleteFramebuffers(1, &mSampleFramebuffer));
            mSampleFramebuffer = 0;
        }
        
        if(mDepthRenderbuffer)
        {
            OGRE_CHECK_GL_ERROR(glDeleteRenderbuffers(1, &mDepthRenderbuffer));
            mDepthRenderbuffer = 0;
        }
    }

    void EAGLES2Context::setCurrent()
    {
        BOOL ret = [EAGLContext setCurrentContext:mContext];
        if (!ret)
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        "Failed to make context current",
                        __FUNCTION__);
        }
    }

    void EAGLES2Context::endCurrent()
    {
        // Do nothing
    }

    GLES2Context * EAGLES2Context::clone() const
    {
		return new EAGLES2Context(mDrawable, [mContext sharegroup]);
    }

	CAEAGLLayer * EAGLES2Context::getDrawable() const
	{
		return mDrawable;
	}

	EAGLContext * EAGLES2Context::getContext() const
	{
		return mContext;
	}
}
