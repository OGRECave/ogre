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

#include "OgreGLES2FBORenderTexture.h"
#include "OgreGLES2PixelFormat.h"
#include "OgreLogManager.h"
#include "OgreGLES2HardwarePixelBuffer.h"
#include "OgreGLES2FBOMultiRenderTarget.h"

namespace Ogre {

//-----------------------------------------------------------------------------    
    GLES2FBORenderTexture::GLES2FBORenderTexture(GLES2FBOManager *manager, const String &name,
        const GLES2SurfaceDesc &target, bool writeGamma, uint fsaa):
        GLES2RenderTexture(name, target, writeGamma, fsaa),
        mFB(manager, fsaa)
    {
        // Bind target to surface 0 and initialise
        mFB.bindSurface(0, target);
        GL_CHECK_ERROR;
        // Get attributes
        mWidth = mFB.getWidth();
        mHeight = mFB.getHeight();
    }

    void GLES2FBORenderTexture::getCustomAttribute(const String& name, void* pData)
    {
        if(name=="FBO")
        {
            *static_cast<GLES2FrameBufferObject **>(pData) = &mFB;
        }
    }

	void GLES2FBORenderTexture::swapBuffers(bool waitForVSync)
	{
		mFB.swapBuffers();
	}
	//-----------------------------------------------------------------------------
	bool GLES2FBORenderTexture::attachDepthBuffer( DepthBuffer *depthBuffer )
	{
		bool result;
		if( result = GLES2RenderTexture::attachDepthBuffer( depthBuffer ) )
			mFB.attachDepthBuffer( depthBuffer );

		return result;
	}
	//-----------------------------------------------------------------------------
	void GLES2FBORenderTexture::detachDepthBuffer()
	{
		mFB.detachDepthBuffer();
		GLES2RenderTexture::detachDepthBuffer();
	}
	//-----------------------------------------------------------------------------
	void GLES2FBORenderTexture::_detachDepthBuffer()
	{
		mFB.detachDepthBuffer();
		GLES2RenderTexture::_detachDepthBuffer();
	}
   
    /// Size of probe texture
    #define PROBE_SIZE 16

    /// Stencil and depth formats to be tried
    static const GLenum stencilFormats[] =
    {
        GL_NONE,                    // No stencil
        GL_STENCIL_INDEX8
    };
    static const size_t stencilBits[] =
    {
        0, 8
    };
    #define STENCILFORMAT_COUNT (sizeof(stencilFormats)/sizeof(GLenum))

    static const GLenum depthFormats[] =
    {
        GL_NONE,
        GL_DEPTH_COMPONENT16,
        GL_DEPTH_COMPONENT24_OES,   // Prefer 24 bit depth
        GL_DEPTH24_STENCIL8_OES     // Packed depth / stencil
    };
    static const size_t depthBits[] =
    {
        0,16,24,24
    };
    #define DEPTHFORMAT_COUNT (sizeof(depthFormats)/sizeof(GLenum))

	GLES2FBOManager::GLES2FBOManager()
    {
        detectFBOFormats();
        
        glGenFramebuffers(1, &mTempFBO);
        GL_CHECK_ERROR;
    }

	GLES2FBOManager::~GLES2FBOManager()
	{
		if(!mRenderBufferMap.empty())
		{
			LogManager::getSingleton().logMessage("GL: Warning! GLESFBOManager destructor called, but not all renderbuffers were released.");
		}
        
        glDeleteFramebuffers(1, &mTempFBO);      
        GL_CHECK_ERROR;
	}

    /** Try a certain FBO format, and return the status. Also sets mDepthRB and mStencilRB.
        @returns true    if this combo is supported
                 false   if this combo is not supported
    */
    GLuint GLES2FBOManager::_tryFormat(GLenum depthFormat, GLenum stencilFormat)
    {
        GLuint status, depthRB = 0, stencilRB = 0;

        if(depthFormat != GL_NONE)
        {
            /// Generate depth renderbuffer
            glGenRenderbuffers(1, &depthRB);

            /// Bind it to FBO
            glBindRenderbuffer(GL_RENDERBUFFER, depthRB);
            
            /// Allocate storage for depth buffer
            glRenderbufferStorage(GL_RENDERBUFFER, depthFormat,
                                PROBE_SIZE, PROBE_SIZE);
            
            /// Attach depth
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRB);
        }

        // Stencil buffers aren't available on iPhone
        if(stencilFormat != GL_NONE)
        {
            /// Generate stencil renderbuffer
            glGenRenderbuffers(1, &stencilRB);
            
            /// Bind it to FBO
            glBindRenderbuffer(GL_RENDERBUFFER, stencilRB);

            /// Allocate storage for stencil buffer
            glRenderbufferStorage(GL_RENDERBUFFER, stencilFormat, PROBE_SIZE, PROBE_SIZE); 

            /// Attach stencil
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, stencilRB);
        }

        status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

        /// If status is negative, clean up
        // Detach and destroy
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);

        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);

        if (depthRB)
            glDeleteRenderbuffers(1, &depthRB);

        if (stencilRB)
            glDeleteRenderbuffers(1, &stencilRB);
        
        return status == GL_FRAMEBUFFER_COMPLETE;
    }
    
    /** Try a certain packed depth/stencil format, and return the status.
        @returns true    if this combo is supported
                 false   if this combo is not supported
    */
    bool GLES2FBOManager::_tryPackedFormat(GLenum packedFormat)
    {
        GLuint packedRB;

        /// Generate renderbuffer
        glGenRenderbuffers(1, &packedRB);

        /// Bind it to FBO
        glBindRenderbuffer(GL_RENDERBUFFER, packedRB);

        /// Allocate storage for buffer
        glRenderbufferStorage(GL_RENDERBUFFER, packedFormat, PROBE_SIZE, PROBE_SIZE);

        /// Attach depth
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
            GL_RENDERBUFFER, packedRB);

        /// Attach stencil
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
            GL_RENDERBUFFER, packedRB);

        GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

        /// Detach and destroy
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
        glDeleteRenderbuffers(1, &packedRB);

        return status == GL_FRAMEBUFFER_COMPLETE;
    }

    /** Detect which internal formats are allowed as RTT
        Also detect what combinations of stencil and depth are allowed with this internal
        format.
    */
    void GLES2FBOManager::detectFBOFormats()
    {
        // Try all formats, and report which ones work as target
        GLuint fb, tid;
        GLenum target = GL_TEXTURE_2D;

        for(size_t x=0; x<PF_COUNT; ++x)
        {
            mProps[x].valid = false;

			// Fetch GL format token
			GLenum fmt = GLES2PixelUtil::getGLInternalFormat((PixelFormat)x);
            if(fmt == GL_NONE && x!=0)
                continue;

			// No test for compressed formats
			if(PixelUtil::isCompressed((PixelFormat)x))
				continue;

            // Create and attach framebuffer
            glGenFramebuffers(1, &fb);
            GL_CHECK_ERROR;
            glBindFramebuffer(GL_FRAMEBUFFER, fb);
            GL_CHECK_ERROR;
            if (fmt!=GL_NONE)
            {
				// Create and attach texture
				glGenTextures(1, &tid);
                GL_CHECK_ERROR;
				glBindTexture(target, tid);
                GL_CHECK_ERROR;
				
                // Set some default parameters
                glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
                GL_CHECK_ERROR;
                glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                GL_CHECK_ERROR;
                glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                GL_CHECK_ERROR;
                glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                GL_CHECK_ERROR;
                            
				glTexImage2D(target, 0, fmt, PROBE_SIZE, PROBE_SIZE, 0, fmt, GL_UNSIGNED_BYTE, 0);
                GL_CHECK_ERROR;
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                target, tid, 0);
                GL_CHECK_ERROR;
            }

            // Check status
            GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            GL_CHECK_ERROR;

			// Ignore status in case of fmt==GL_NONE, because no implementation will accept
			// a buffer without *any* attachment. Buffers with only stencil and depth attachment
			// might still be supported, so we must continue probing.
            if(fmt == GL_NONE || status == GL_FRAMEBUFFER_COMPLETE)
            {
                mProps[x].valid = true;
				StringUtil::StrStreamType str;
				str << "FBO " << PixelUtil::getFormatName((PixelFormat)x) 
					<< " depth/stencil support: ";

                // For each depth/stencil formats
                for (size_t depth = 0; depth < DEPTHFORMAT_COUNT; ++depth)
                {
                    if (depthFormats[depth] != GL_DEPTH24_STENCIL8_OES)
                    {
                        // General depth/stencil combination

                        for (size_t stencil = 0; stencil < STENCILFORMAT_COUNT; ++stencil)
                        {
                            //StringUtil::StrStreamType l;
                            //l << "Trying " << PixelUtil::getFormatName((PixelFormat)x) 
                            //	<< " D" << depthBits[depth] 
                            //	<< "S" << stencilBits[stencil];
                            //LogManager::getSingleton().logMessage(l.str());

                            if (_tryFormat(depthFormats[depth], stencilFormats[stencil]))
                            {
                                /// Add mode to allowed modes
                                str << "D" << depthBits[depth] << "S" << stencilBits[stencil] << " ";
                                FormatProperties::Mode mode;
                                mode.depth = depth;
                                mode.stencil = stencil;
                                mProps[x].modes.push_back(mode);
                            }
                        }
                    }
                    else
                    {
                        // Packed depth/stencil format
                        if (_tryPackedFormat(depthFormats[depth]))
                        {
                            /// Add mode to allowed modes
                            str << "Packed-D" << depthBits[depth] << "S" << 8 << " ";
                            FormatProperties::Mode mode;
                            mode.depth = depth;
                            mode.stencil = 0;   // unuse
                            mProps[x].modes.push_back(mode);
                        }
                    }
                }
                LogManager::getSingleton().logMessage(str.str());
            }

            // Delete texture and framebuffer
#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
            // The screen buffer is 1 on iPhone
            glBindFramebuffer(GL_FRAMEBUFFER, 1);
#else
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
            GL_CHECK_ERROR;
            glDeleteFramebuffers(1, &fb);
            GL_CHECK_ERROR;
			
            if (fmt!=GL_NONE)
                glDeleteTextures(1, &tid);
            GL_CHECK_ERROR;
        }

		String fmtstring;
        for(size_t x=0; x<PF_COUNT; ++x)
        {
            if(mProps[x].valid)
                fmtstring += PixelUtil::getFormatName((PixelFormat)x)+" ";
        }
        LogManager::getSingleton().logMessage("[GL] : Valid FBO targets " + fmtstring);
    }

    void GLES2FBOManager::getBestDepthStencil(GLenum internalFormat, GLenum *depthFormat, GLenum *stencilFormat)
    {
        const FormatProperties &props = mProps[internalFormat];
        /// Decide what stencil and depth formats to use
        /// [best supported for internal format]
        size_t bestmode=0;
        int bestscore=-1;
        for(size_t mode=0; mode<props.modes.size(); mode++)
        {
            int desirability = 0;
            /// Find most desirable mode
            /// desirability == 0            if no depth, no stencil
            /// desirability == 1000...2000  if no depth, stencil
            /// desirability == 2000...3000  if depth, no stencil
            /// desirability == 3000+        if depth and stencil
            /// beyond this, the total numer of bits (stencil+depth) is maximised
            if(props.modes[mode].stencil)
                desirability += 1000;
            if(props.modes[mode].depth)
                desirability += 2000;
            if(depthBits[props.modes[mode].depth]==24) // Prefer 24 bit for now
                desirability += 500;
			if(depthFormats[props.modes[mode].depth]==GL_DEPTH24_STENCIL8_OES) // Prefer 24/8 packed 
				desirability += 5000;
            desirability += stencilBits[props.modes[mode].stencil] + depthBits[props.modes[mode].depth];
            
            if(desirability>bestscore)
            {
                bestscore = desirability;
                bestmode = mode;
            }
        }
        *depthFormat = depthFormats[props.modes[bestmode].depth];
        *stencilFormat = stencilFormats[props.modes[bestmode].stencil];
    }

    GLES2FBORenderTexture *GLES2FBOManager::createRenderTexture(const String &name, 
		const GLES2SurfaceDesc &target, bool writeGamma, uint fsaa)
    {
        GLES2FBORenderTexture *retval = OGRE_NEW GLES2FBORenderTexture(this, name, target, writeGamma, fsaa);
        return retval;
    }
	MultiRenderTarget *GLES2FBOManager::createMultiRenderTarget(const String & name)
	{
		return OGRE_NEW GLES2FBOMultiRenderTarget(this, name);
	}

    void GLES2FBOManager::bind(RenderTarget *target)
    {
        /// Check if the render target is in the rendertarget->FBO map
        GLES2FrameBufferObject *fbo = 0;
        target->getCustomAttribute("FBO", &fbo);
        if(fbo)
            fbo->bind();
        else
            // Old style context (window/pbuffer) or copying render texture
#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
            // The screen buffer is 1 on iPhone
            glBindFramebuffer(GL_FRAMEBUFFER, 1);
#else
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
        GL_CHECK_ERROR;
    }
    
    GLES2SurfaceDesc GLES2FBOManager::requestRenderBuffer(GLenum format, size_t width, size_t height, uint fsaa)
    {
        GLES2SurfaceDesc retval;
        retval.buffer = 0; // Return 0 buffer if GL_NONE is requested
        if(format != GL_NONE)
        {
            RBFormat key(format, width, height, fsaa);
            RenderBufferMap::iterator it = mRenderBufferMap.find(key);
            if(it != mRenderBufferMap.end())
            {
                retval.buffer = it->second.buffer;
                retval.zoffset = 0;
				retval.numSamples = fsaa;
                // Increase refcount
                ++it->second.refcount;
            }
            else
            {
                // New one
                GLES2RenderBuffer *rb = OGRE_NEW GLES2RenderBuffer(format, width, height, fsaa);
                mRenderBufferMap[key] = RBRef(rb);
                retval.buffer = rb;
                retval.zoffset = 0;
				retval.numSamples = fsaa;
            }
        }
//        std::cerr << "Requested renderbuffer with format " << std::hex << format << std::dec << " of " << width << "x" << height << " :" << retval.buffer << std::endl;
        return retval;
    }
    //-----------------------------------------------------------------------
    void GLES2FBOManager::requestRenderBuffer(const GLES2SurfaceDesc &surface)
    {
        if(surface.buffer == 0)
            return;
        RBFormat key(surface.buffer->getGLFormat(), surface.buffer->getWidth(), surface.buffer->getHeight(), surface.numSamples);
        RenderBufferMap::iterator it = mRenderBufferMap.find(key);
        assert(it != mRenderBufferMap.end());
        if (it != mRenderBufferMap.end())   // Just in case
        {
            assert(it->second.buffer == surface.buffer);
            // Increase refcount
            ++it->second.refcount;
        }
    }
    //-----------------------------------------------------------------------
    void GLES2FBOManager::releaseRenderBuffer(const GLES2SurfaceDesc &surface)
    {
        if(surface.buffer == 0)
            return;
        RBFormat key(surface.buffer->getGLFormat(), surface.buffer->getWidth(), surface.buffer->getHeight(), surface.numSamples);
        RenderBufferMap::iterator it = mRenderBufferMap.find(key);
        if(it != mRenderBufferMap.end())
		{
			// Decrease refcount
			--it->second.refcount;
			if(it->second.refcount==0)
			{
				// If refcount reaches zero, delete buffer and remove from map
				OGRE_DELETE it->second.buffer;
				mRenderBufferMap.erase(it);
				//std::cerr << "Destroyed renderbuffer of format " << std::hex << key.format << std::dec
				//        << " of " << key.width << "x" << key.height << std::endl;
			}
		}
    }
}
