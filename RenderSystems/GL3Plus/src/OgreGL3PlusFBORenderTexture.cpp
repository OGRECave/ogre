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
#include "OgreGL3PlusPrerequisites.h"

#include "OgreGL3PlusFBORenderTexture.h"
#include "OgreGL3PlusPixelFormat.h"
#include "OgreLogManager.h"
#include "OgreGL3PlusHardwarePixelBuffer.h"
#include "OgreGL3PlusFBOMultiRenderTarget.h"

#define TEMP_FBOS 2

namespace Ogre {

//-----------------------------------------------------------------------------    
    GL3PlusFBORenderTexture::GL3PlusFBORenderTexture(GL3PlusFBOManager *manager, const String &name,
        const GL3PlusSurfaceDesc &target, bool writeGamma, uint fsaa):
        GL3PlusRenderTexture(name, target, writeGamma, fsaa),
        mFB(manager, fsaa)
    {
        // Bind target to surface 0 and initialise
        mFB.bindSurface(0, target);

        // Get attributes
        mWidth = mFB.getWidth();
        mHeight = mFB.getHeight();
    }

    void GL3PlusFBORenderTexture::getCustomAttribute(const String& name, void* pData)
    {
        if(name == GL3PlusRenderTexture::CustomAttributeString_FBO)
        {
            *static_cast<GL3PlusFrameBufferObject **>(pData) = &mFB;
        }
		else if (name == "GL_FBOID")
		{
            *static_cast<GLuint*>(pData) = mFB.getGLFBOID();
		}
		else if (name == "GL_MULTISAMPLEFBOID")
		{
            *static_cast<GLuint*>(pData) = mFB.getGLMultisampleFBOID();
		}
    }

	void GL3PlusFBORenderTexture::swapBuffers()
	{
		mFB.swapBuffers();
	}
	//-----------------------------------------------------------------------------
	bool GL3PlusFBORenderTexture::attachDepthBuffer( DepthBuffer *depthBuffer )
	{
		bool result;
		if( (result = GL3PlusRenderTexture::attachDepthBuffer( depthBuffer )) )
			mFB.attachDepthBuffer( depthBuffer );

		return result;
	}
	//-----------------------------------------------------------------------------
	void GL3PlusFBORenderTexture::detachDepthBuffer()
	{
		mFB.detachDepthBuffer();
		GL3PlusRenderTexture::detachDepthBuffer();
	}
	//-----------------------------------------------------------------------------
	void GL3PlusFBORenderTexture::_detachDepthBuffer()
	{
		mFB.detachDepthBuffer();
		GL3PlusRenderTexture::_detachDepthBuffer();
	}
   
    // Size of probe texture
    #define PROBE_SIZE 16

    // Stencil and depth formats to be tried
    static const GLenum stencilFormats[] =
    {
        GL_NONE,                    // No stencil
        GL_STENCIL_INDEX1,
        GL_STENCIL_INDEX4,
        GL_STENCIL_INDEX8,
        GL_STENCIL_INDEX16
    };
    static const size_t stencilBits[] =
    {
        0, 1, 4, 8, 16
    };
    #define STENCILFORMAT_COUNT (sizeof(stencilFormats)/sizeof(GLenum))

    static const GLenum depthFormats[] =
    {
        GL_NONE,
        GL_DEPTH_COMPONENT16,
        GL_DEPTH_COMPONENT24,   // Prefer 24 bit depth
        GL_DEPTH_COMPONENT32,
        GL_DEPTH_COMPONENT32F,
        GL_DEPTH24_STENCIL8,    // Packed depth / stencil
        GL_DEPTH32F_STENCIL8
    };
    static const size_t depthBits[] =
    {
        0,16,24,32,32,24,32
    };
    #define DEPTHFORMAT_COUNT (sizeof(depthFormats)/sizeof(GLenum))

	GL3PlusFBOManager::GL3PlusFBOManager()
    {
        detectFBOFormats();
        
        mTempFBO.resize(TEMP_FBOS, 0);

        for (int i = 0; i < TEMP_FBOS; i++)
        {
            OGRE_CHECK_GL_ERROR(glGenFramebuffers(1, &mTempFBO[i]));
        }
    }

	GL3PlusFBOManager::~GL3PlusFBOManager()
	{
		if(!mRenderBufferMap.empty())
		{
			LogManager::getSingleton().logMessage("GL: Warning! GL3PlusFBOManager destructor called, but not all renderbuffers were released.", LML_CRITICAL);
		}
        
        for (int i = 0; i < TEMP_FBOS; i++)
        {
            OGRE_CHECK_GL_ERROR(glDeleteFramebuffers(1, &mTempFBO[i]));
        }
	}

    void GL3PlusFBOManager::_createTempFramebuffer(int ogreFormat, GLuint internalFormat, GLuint fmt, GLenum dataType, GLuint &fb, GLuint &tid)
    {
        glGenFramebuffers(1, &fb);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb);
        if (fmt != GL_NONE)
        {
            if (tid)
                glDeleteTextures(1, &tid);

            // Create and attach texture
            glGenTextures(1, &tid);
            glBindTexture(GL_TEXTURE_2D, tid);

            // Set some default parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, PROBE_SIZE, PROBE_SIZE, 0, fmt, dataType, 0);

            if(ogreFormat == PF_DEPTH)
                glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, tid, 0);
            else
                glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tid, 0);
        }
        else
        {
            // Draw to nowhere -- stencil/depth only
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
        }
    }

    /** Try a certain FBO format, and return the status. Also sets mDepthRB and mStencilRB.
        @returns true    if this combo is supported
                 false   if this combo is not supported
    */
    GLuint GL3PlusFBOManager::_tryFormat(GLenum depthFormat, GLenum stencilFormat)
    {
        GLuint status, depthRB = 0, stencilRB = 0;

        if(depthFormat != GL_NONE)
        {
            // Generate depth renderbuffer
            glGenRenderbuffers(1, &depthRB);

            // Bind it to FBO
            glBindRenderbuffer(GL_RENDERBUFFER, depthRB);

            // Allocate storage for depth buffer
            glRenderbufferStorage(GL_RENDERBUFFER, depthFormat,
                                PROBE_SIZE, PROBE_SIZE);

            // Attach depth
            glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRB);
        }

        if(stencilFormat != GL_NONE)
        {
            // Generate stencil renderbuffer
            glGenRenderbuffers(1, &stencilRB);
            // Bind it to FBO
            glBindRenderbuffer(GL_RENDERBUFFER, stencilRB);

            // Allocate storage for stencil buffer
            glRenderbufferStorage(GL_RENDERBUFFER, stencilFormat, PROBE_SIZE, PROBE_SIZE); 

            // Attach stencil
            glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, stencilRB);
        }

        status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);

        // If status is negative, clean up
        // Detach and destroy
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);

        if (depthRB)
            glDeleteRenderbuffers(1, &depthRB);

        if (stencilRB)
            glDeleteRenderbuffers(1, &stencilRB);
        
        return status == GL_FRAMEBUFFER_COMPLETE;
    }
    
    /** Try a certain packed depth/stencil format, and return the status.
        @return true    if this combo is supported
                 false   if this combo is not supported
    */
    bool GL3PlusFBOManager::_tryPackedFormat(GLenum packedFormat)
    {
        GLuint packedRB;

        // Generate renderbuffer
        glGenRenderbuffers(1, &packedRB);

        // Bind it to FBO
        glBindRenderbuffer(GL_RENDERBUFFER, packedRB);

        // Allocate storage for buffer
        glRenderbufferStorage(GL_RENDERBUFFER, packedFormat, PROBE_SIZE, PROBE_SIZE);

        // Attach depth
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
            GL_RENDERBUFFER, packedRB);

        // Attach stencil
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
            GL_RENDERBUFFER, packedRB);

        GLuint status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);

        // Detach and destroy
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
        glDeleteRenderbuffers(1, &packedRB);

        return status == GL_FRAMEBUFFER_COMPLETE;
    }

    /** Detect which internal formats are allowed as RTT
        Also detect what combinations of stencil and depth are allowed with this internal
        format.
    */
    void GL3PlusFBOManager::detectFBOFormats()
    {
        // Try all formats, and report which ones work as target
        GLuint fb = 0, tid = 0;

        for(int x = 0; x < PF_COUNT; ++x)
        {
            mProps[x].valid = false;

			// Fetch GL format token
			GLenum fmt = GL3PlusPixelUtil::getGLInternalFormat((PixelFormat)x);
			GLenum fmt2 = GL3PlusPixelUtil::getGLOriginFormat((PixelFormat)x);
			GLenum type = GL3PlusPixelUtil::getGLOriginDataType((PixelFormat)x);
            if(fmt == GL_NONE && x != 0)
                continue;

			// No test for compressed formats
			if(PixelUtil::isCompressed((PixelFormat)x))
				continue;

            // Create and attach framebuffer
            _createTempFramebuffer(x, fmt, fmt2, type, fb, tid);

            // Check status
            GLuint status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);

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
                    if ((depthFormats[depth] != GL_DEPTH24_STENCIL8) && (depthFormats[depth] != GL_DEPTH32F_STENCIL8))
                    {
                        // General depth/stencil combination

                        for (size_t stencil = 0; stencil < STENCILFORMAT_COUNT; ++stencil)
                        {
//                            StringUtil::StrStreamType l;
//                            l << "Trying " << PixelUtil::getFormatName((PixelFormat)x) 
//                            	<< " D" << depthBits[depth] 
//                            	<< "S" << stencilBits[stencil];
//                            LogManager::getSingleton().logMessage(l.str());

                            if (_tryFormat(depthFormats[depth], stencilFormats[stencil]))
                            {
                                // Add mode to allowed modes
                                str << "D" << depthBits[depth] << "S" << stencilBits[stencil] << " ";
                                FormatProperties::Mode mode;
                                mode.depth = depth;
                                mode.stencil = stencil;
                                mProps[x].modes.push_back(mode);
                            }
                            else
                            {
                                // There is a small edge case that FBO is trashed during the test
                                // on some drivers resulting in undefined behavior
                                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                                glDeleteFramebuffers(1, &fb);

                                // Workaround for NVIDIA / Linux 169.21 driver problem
                                // see http://www.ogre3d.org/phpBB2/viewtopic.php?t=38037&start=25
                                glFinish();

                                _createTempFramebuffer(x, fmt, fmt2, type, fb, tid);
                            }
                        }
                    }
                    else
                    {
                        // Packed depth/stencil format
                        if (_tryPackedFormat(depthFormats[depth]))
                        {
                            // Add mode to allowed modes
                            str << "Packed-D" << depthBits[depth] << "S" << 8 << " ";
                            FormatProperties::Mode mode;
                            mode.depth = depth;
                            mode.stencil = 0;   // unuse
                            mProps[x].modes.push_back(mode);
                        }
                        else
                        {
                            // There is a small edge case that FBO is trashed during the test
                            // on some drivers resulting in undefined behavior
                            glBindFramebuffer(GL_FRAMEBUFFER, 0);
                            glDeleteFramebuffers(1, &fb);

                            // Workaround for NVIDIA / Linux 169.21 driver problem
                            // see http://www.ogre3d.org/phpBB2/viewtopic.php?t=38037&start=25
                            glFinish();

                            _createTempFramebuffer(x, fmt, fmt2, type, fb, tid);
                        }
                    }
                }
                LogManager::getSingleton().logMessage(str.str());
            }

            // Delete texture and framebuffer
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glDeleteFramebuffers(1, &fb);
			
            if (fmt != GL_NONE)
            {
                glDeleteTextures(1, &tid);
                tid = 0;
            }
        }

		String fmtstring = "";
        for(size_t x = 0; x < PF_COUNT; ++x)
        {
            if(mProps[x].valid)
                fmtstring += PixelUtil::getFormatName((PixelFormat)x)+" ";
        }
        LogManager::getSingleton().logMessage("[GL] : Valid FBO targets " + fmtstring);
    }

    void GL3PlusFBOManager::getBestDepthStencil(GLenum internalFormat, GLenum *depthFormat, GLenum *stencilFormat)
    {
        const FormatProperties &props = mProps[internalFormat];
        // Decide what stencil and depth formats to use
        // [best supported for internal format]
        size_t bestmode=0;
        int bestscore=-1;
        bool requestDepthOnly = internalFormat == PF_DEPTH;
        for(size_t mode=0; mode<props.modes.size(); mode++)
        {
            int desirability = 0;
            // Find most desirable mode
            // desirability == 0            if no depth, no stencil
            // desirability == 1000...2000  if no depth, stencil
            // desirability == 2000...3000  if depth, no stencil
            // desirability == 3000+        if depth and stencil
            // beyond this, the total number of bits (stencil+depth) is maximised
            if(props.modes[mode].stencil && !requestDepthOnly)
                desirability += 1000;
            if(props.modes[mode].depth)
                desirability += 2000;
            if(depthBits[props.modes[mode].depth]==24) // Prefer 24 bit for now
                desirability += 500;
			if((depthFormats[props.modes[mode].depth]==GL_DEPTH24_STENCIL8 || depthFormats[props.modes[mode].depth]==GL_DEPTH32F_STENCIL8) && !requestDepthOnly) // Prefer 24/8 packed
				desirability += 5000;
            desirability += stencilBits[props.modes[mode].stencil] + depthBits[props.modes[mode].depth];
            
            if(desirability>bestscore)
            {
                bestscore = desirability;
                bestmode = mode;
            }
        }
        *depthFormat = depthFormats[props.modes[bestmode].depth];
        if(requestDepthOnly)
            *stencilFormat = 0;
        else
            *stencilFormat = stencilFormats[props.modes[bestmode].stencil];
    }

    GL3PlusFBORenderTexture *GL3PlusFBOManager::createRenderTexture(const String &name, 
		const GL3PlusSurfaceDesc &target, bool writeGamma, uint fsaa)
    {
        GL3PlusFBORenderTexture *retval = new GL3PlusFBORenderTexture(this, name, target, writeGamma, fsaa);
        return retval;
    }
	MultiRenderTarget *GL3PlusFBOManager::createMultiRenderTarget(const String & name)
	{
		return new GL3PlusFBOMultiRenderTarget(this, name);
	}

    void GL3PlusFBOManager::bind(RenderTarget *target)
    {
        // Check if the render target is in the rendertarget->FBO map
        GL3PlusFrameBufferObject *fbo = 0;
        target->getCustomAttribute(GL3PlusRenderTexture::CustomAttributeString_FBO, &fbo);
        if(fbo)
            fbo->bind();
        else
            // Old style context (window/pbuffer) or copying render texture
            OGRE_CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }
    
    GL3PlusSurfaceDesc GL3PlusFBOManager::requestRenderBuffer(GLenum format, uint32 width, uint32 height, uint fsaa)
    {
        GL3PlusSurfaceDesc retval;
        retval.buffer = 0; // Return 0 buffer if GL_NONE is requested
        if(format != GL_NONE)
        {
            RBFormat key(format, width, height, fsaa);
            RenderBufferMap::iterator it = mRenderBufferMap.find(key);
            if(it != mRenderBufferMap.end() && (it->second.refcount == 0))
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
                GL3PlusRenderBuffer *rb = new GL3PlusRenderBuffer(format, width, height, fsaa);
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
    void GL3PlusFBOManager::requestRenderBuffer(const GL3PlusSurfaceDesc &surface)
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
    void GL3PlusFBOManager::releaseRenderBuffer(const GL3PlusSurfaceDesc &surface)
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
				delete it->second.buffer;
				mRenderBufferMap.erase(it);
//				std::cerr << "Destroyed renderbuffer of format " << std::hex << key.format << std::dec
//				        << " of " << key.width << "x" << key.height << std::endl;
			}
        }
    }

    GLuint GL3PlusFBOManager::getTemporaryFBO(size_t id)
    {
        assert(id < mTempFBO.size());

        return mTempFBO[id];
    }
}
