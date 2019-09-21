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

#include "OgreGLFBORenderTexture.h"
#include "OgreGLPixelFormat.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreRoot.h"
#include "OgreGLHardwarePixelBuffer.h"
#include "OgreGLFBOMultiRenderTarget.h"

namespace Ogre {

//-----------------------------------------------------------------------------    
    GLFBORenderTexture::GLFBORenderTexture(GLFBOManager *manager, const String &name,
        const GLSurfaceDesc &target, bool writeGamma, uint fsaa):
        GLRenderTexture(name, target, writeGamma, fsaa),
        mFB(manager, fsaa)
    {
        // Bind target to surface 0 and initialise
        mFB.bindSurface(0, target);
        // Get attributes
        mWidth = mFB.getWidth();
        mHeight = mFB.getHeight();
    }

    void GLFBORenderTexture::getCustomAttribute(const String& name, void* pData)
    {
        if( name == GLRenderTexture::CustomAttributeString_FBO )
        {
            *static_cast<GLFrameBufferObject **>(pData) = &mFB;
        }
        else if(name == GLRenderTexture::CustomAttributeString_GLCONTEXT)
        {
            *static_cast<GLContext**>(pData) = mFB.getContext();
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

    void GLFBORenderTexture::swapBuffers()
    {
        mFB.swapBuffers();
    }
    //-----------------------------------------------------------------------------
    bool GLFBORenderTexture::attachDepthBuffer( DepthBuffer *depthBuffer )
    {
        bool result;
        if( (result = GLRenderTexture::attachDepthBuffer( depthBuffer )) )
            mFB.attachDepthBuffer( depthBuffer );

        return result;
    }
    //-----------------------------------------------------------------------------
    void GLFBORenderTexture::detachDepthBuffer()
    {
        mFB.detachDepthBuffer();
        GLRenderTexture::detachDepthBuffer();
    }
    //-----------------------------------------------------------------------------
    void GLFBORenderTexture::_detachDepthBuffer()
    {
        mFB.detachDepthBuffer();
        GLRenderTexture::_detachDepthBuffer();
    }
   
/// Size of probe texture
#define PROBE_SIZE 16

/// Stencil and depth formats to be tried
static const GLenum stencilFormats[] =
{
    GL_NONE,                    // No stencil
    GL_STENCIL_INDEX1_EXT,
    GL_STENCIL_INDEX4_EXT,
    GL_STENCIL_INDEX8_EXT,
    GL_STENCIL_INDEX16_EXT
};
static const uchar stencilBits[] =
{
    0, 1, 4, 8, 16
};
#define STENCILFORMAT_COUNT (sizeof(stencilFormats)/sizeof(GLenum))

static const GLenum depthFormats[] =
{
    GL_NONE,
    GL_DEPTH_COMPONENT16,
    GL_DEPTH_COMPONENT24,    // Prefer 24 bit depth
    GL_DEPTH_COMPONENT32,
    GL_DEPTH24_STENCIL8_EXT // packed depth / stencil
};
static const uchar depthBits[] =
{
    0,16,24,32,24
};
#define DEPTHFORMAT_COUNT (sizeof(depthFormats)/sizeof(GLenum))

    GLFBOManager::GLFBOManager(bool atimode):
        mATIMode(atimode)
    {
        detectFBOFormats();
        
        glGenFramebuffersEXT(1, &mTempFBO);
    }

    GLFBOManager::~GLFBOManager()
    {
        if(!mRenderBufferMap.empty())
        {
            LogManager::getSingleton().logWarning("GLFBOManager destructor called, but not all renderbuffers were released.");
        }
        
        glDeleteFramebuffersEXT(1, &mTempFBO);      
    }

    void GLFBOManager::_createTempFramebuffer(GLuint internalfmt, GLuint fmt, GLenum type, GLuint &fb, GLuint &tid)
    {
        // NB we bypass state cache, this method is only called on startup and not after
        // GLStateCacheManager::initializeCache

        glGenFramebuffersEXT(1, &fb);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
        if (fmt != GL_NONE)
        {
            if (tid)
                glDeleteTextures(1, &tid);

            // Create and attach texture
            glGenTextures(1, &tid);
            glBindTexture(GL_TEXTURE_2D, tid);

            // Set some default parameters so it won't fail on NVidia cards
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glTexImage2D(GL_TEXTURE_2D, 0, internalfmt, PROBE_SIZE, PROBE_SIZE, 0, fmt, type, 0);
            glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                                      fmt == GL_DEPTH_COMPONENT ? GL_DEPTH_ATTACHMENT_EXT
                                                                : GL_COLOR_ATTACHMENT0_EXT,
                                      GL_TEXTURE_2D, tid, 0);
        }
        else
        {
            // Draw to nowhere -- stencil/depth only
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
        }
    }

    /** Try a certain FBO format, and return the status. Also sets mDepthRB and mStencilRB.
        @return true    if this combo is supported
                 false   if this combo is not supported
    */
    GLuint GLFBOManager::_tryFormat(GLenum depthFormat, GLenum stencilFormat)
    {
        GLuint status, depthRB = 0, stencilRB = 0;
        bool failed = false; // flag on GL errors

        if(depthFormat != GL_NONE)
        {
            /// Generate depth renderbuffer
            glGenRenderbuffersEXT(1, &depthRB);
            /// Bind it to FBO
            glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthRB);
            
            /// Allocate storage for depth buffer
            glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, depthFormat,
                                PROBE_SIZE, PROBE_SIZE);
            
            /// Attach depth
            glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                    GL_RENDERBUFFER_EXT, depthRB);
        }

        if(stencilFormat != GL_NONE)
        {
            /// Generate stencil renderbuffer
            glGenRenderbuffersEXT(1, &stencilRB);
            /// Bind it to FBO
            glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, stencilRB);
            glGetError(); // NV hack
            /// Allocate storage for stencil buffer
            glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, stencilFormat,
                                PROBE_SIZE, PROBE_SIZE); 
            if(glGetError() != GL_NO_ERROR) // NV hack
                failed = true;
            /// Attach stencil
            glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
                            GL_RENDERBUFFER_EXT, stencilRB);
            if(glGetError() != GL_NO_ERROR) // NV hack
                failed = true;
        }
        
        status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
        /// If status is negative, clean up
        // Detach and destroy
        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, 0);
        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, 0);

        if (depthRB)
            glDeleteRenderbuffersEXT(1, &depthRB);
        if (stencilRB)
            glDeleteRenderbuffersEXT(1, &stencilRB);
        
        return status == GL_FRAMEBUFFER_COMPLETE_EXT && !failed;
    }
    
    /** Try a certain packed depth/stencil format, and return the status.
        @return true    if this combo is supported
                 false   if this combo is not supported
    */
    bool GLFBOManager::_tryPackedFormat(GLenum packedFormat)
    {
        GLuint packedRB = 0;
        bool failed = false; // flag on GL errors

        /// Generate renderbuffer
        glGenRenderbuffersEXT(1, &packedRB);

        /// Bind it to FBO
        glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, packedRB);

        /// Allocate storage for buffer
        glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, packedFormat, PROBE_SIZE, PROBE_SIZE);
        glGetError(); // NV hack

        /// Attach depth
        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
            GL_RENDERBUFFER_EXT, packedRB);
        if(glGetError() != GL_NO_ERROR) // NV hack
            failed = true;

        /// Attach stencil
        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
            GL_RENDERBUFFER_EXT, packedRB);
        if(glGetError() != GL_NO_ERROR) // NV hack
            failed = true;

        GLuint status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);

        /// Detach and destroy
        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, 0);
        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, 0);
        glDeleteRenderbuffersEXT(1, &packedRB);

        return status == GL_FRAMEBUFFER_COMPLETE_EXT && !failed;
    }

    /** Detect which internal formats are allowed as RTT
        Also detect what combinations of stencil and depth are allowed with this internal
        format.
    */
    void GLFBOManager::detectFBOFormats()
    {
        // Try all formats, and report which ones work as target
        GLuint fb = 0, tid = 0;
        GLint old_drawbuffer = 0, old_readbuffer = 0;

        glGetIntegerv (GL_DRAW_BUFFER, &old_drawbuffer);
        glGetIntegerv (GL_READ_BUFFER, &old_readbuffer);

        for(size_t x=0; x<PF_COUNT; ++x)
        {
            mProps[x].valid = false;

            // Fetch GL format token
            GLint internalFormat = GLPixelUtil::getGLInternalFormat((PixelFormat)x);
            GLenum fmt = GLPixelUtil::getGLOriginFormat((PixelFormat)x);
            GLenum type = GLPixelUtil::getGLOriginDataType((PixelFormat)x);

            if(fmt == GL_NONE && x != 0)
                continue;

            // No test for compressed formats
            if(PixelUtil::isCompressed((PixelFormat)x))
                continue;

            // Buggy ATI cards *crash* on non-RGB(A) formats
            int depths[4];
            PixelUtil::getBitDepths((PixelFormat)x, depths);
            if(fmt!=GL_NONE && mATIMode && (!depths[0] || !depths[1] || !depths[2]))
                continue;

            // Create and attach framebuffer
            _createTempFramebuffer(internalFormat, fmt, type, fb, tid);

            // Check status
            GLuint status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);

            // Ignore status in case of fmt==GL_NONE, because no implementation will accept
            // a buffer without *any* attachment. Buffers with only stencil and depth attachment
            // might still be supported, so we must continue probing.
            if(fmt == GL_NONE || status == GL_FRAMEBUFFER_COMPLETE_EXT)
            {
                mProps[x].valid = true;
                StringStream str;
                str << "FBO " << PixelUtil::getFormatName((PixelFormat)x) 
                    << " depth/stencil support: ";

                // For each depth/stencil formats
                for (uchar depth = 0; depth < DEPTHFORMAT_COUNT; ++depth)
                {
                    if (depthFormats[depth] != GL_DEPTH24_STENCIL8_EXT)
                    {
                        // General depth/stencil combination

                        for (uchar stencil = 0; stencil < STENCILFORMAT_COUNT; ++stencil)
                        {
                            //StringStream l;
                            //l << "Trying " << PixelUtil::getFormatName((PixelFormat)x) 
                            //  << " D" << depthBits[depth] 
                            //  << "S" << stencilBits[stencil];
                            //LogManager::getSingleton().logMessage(l.str());

                            if (_tryFormat(depthFormats[depth], stencilFormats[stencil]))
                            {
                                /// Add mode to allowed modes
                                str << StringUtil::format("D%dS%d ", depthBits[depth], stencilBits[stencil]);
                                FormatProperties::Mode mode;
                                mode.depth = depth;
                                mode.stencil = stencil;
                                mProps[x].modes.push_back(mode);
                            }
                            else
                            {
                                // There is a small edge case that FBO is trashed during the test
                                // on some drivers resulting in undefined behavior
                                glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
                                glDeleteFramebuffersEXT(1, &fb);

                                // Workaround for NVIDIA / Linux 169.21 driver problem
                                // see http://www.ogre3d.org/phpBB2/viewtopic.php?t=38037&start=25
                                glFinish();

                                _createTempFramebuffer(internalFormat, fmt, type, fb, tid);
                            }
                        }
                    }
                    else
                    {
                        // Packed depth/stencil format
                        if (_tryPackedFormat(depthFormats[depth]))
                        {
                            /// Add mode to allowed modes
                            str << "Packed-D" << int(depthBits[depth]) << "S8";
                            FormatProperties::Mode mode;
                            mode.depth = depth;
                            mode.stencil = 0;   // unuse
                            mProps[x].modes.push_back(mode);
                        }
                        else
                        {
                            // There is a small edge case that FBO is trashed during the test
                            // on some drivers resulting in undefined behavior
                            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
                            glDeleteFramebuffersEXT(1, &fb);

                            // Workaround for NVIDIA / Linux 169.21 driver problem
                            // see http://www.ogre3d.org/phpBB2/viewtopic.php?t=38037&start=25
                            glFinish();

                            _createTempFramebuffer(internalFormat, fmt, type, fb, tid);
                        }
                    }
                }

                LogManager::getSingleton().logMessage(str.str());

            }
            // Delete texture and framebuffer
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
            glDeleteFramebuffersEXT(1, &fb);
            
            // Workaround for NVIDIA / Linux 169.21 driver problem
            // see http://www.ogre3d.org/phpBB2/viewtopic.php?t=38037&start=25
            glFinish();
            
            if (fmt != GL_NONE)
            {
                glDeleteTextures(1, &tid);
                tid = 0;
            }
        }

        // It seems a bug in nVidia driver: glBindFramebufferEXT should restore
        // draw and read buffers, but in some unclear circumstances it won't.
        glDrawBuffer(old_drawbuffer);
        glReadBuffer(old_readbuffer);

        String fmtstring = "";
        for(size_t x=0; x<PF_COUNT; ++x)
        {
            if(mProps[x].valid)
                fmtstring += PixelUtil::getFormatName((PixelFormat)x)+" ";
        }
        LogManager::getSingleton().logMessage("[GL] : Valid FBO targets " + fmtstring);
    }
    void GLFBOManager::getBestDepthStencil(PixelFormat internalFormat, GLenum *depthFormat, GLenum *stencilFormat)
    {
        const FormatProperties &props = mProps[internalFormat];
        /// Decide what stencil and depth formats to use
        /// [best supported for internal format]
        size_t bestmode=0;
        int bestscore=-1;
        bool requestDepthOnly = PixelUtil::isDepth(internalFormat);
        for(size_t mode=0; mode<props.modes.size(); mode++)
        {
#if 0
            /// Always prefer D24S8
            if(stencilBits[props.modes[mode].stencil]==8 &&
                depthBits[props.modes[mode].depth]==24)
            {
                bestmode = mode;
                break;
            }
#endif
            int desirability = 0;
            /// Find most desirable mode
            /// desirability == 0            if no depth, no stencil
            /// desirability == 1000...2000  if no depth, stencil
            /// desirability == 2000...3000  if depth, no stencil
            /// desirability == 3000+        if depth and stencil
            /// beyond this, the total numer of bits (stencil+depth) is maximised
            if(props.modes[mode].stencil && !requestDepthOnly)
                desirability += 1000;
            if(props.modes[mode].depth)
                desirability += 2000;
            if(depthBits[props.modes[mode].depth]==24) // Prefer 24 bit for now
                desirability += 500;
            if((depthFormats[props.modes[mode].depth]==GL_DEPTH24_STENCIL8_EXT) && !requestDepthOnly) // Prefer 24/8 packed
                desirability += 5000;
            desirability += stencilBits[props.modes[mode].stencil] + depthBits[props.modes[mode].depth];
            
            if(desirability>bestscore)
            {
                bestscore = desirability;
                bestmode = mode;
            }
        }
        *depthFormat = depthFormats[props.modes[bestmode].depth];
        *stencilFormat = requestDepthOnly ? 0 : stencilFormats[props.modes[bestmode].stencil];
    }

    GLFBORenderTexture *GLFBOManager::createRenderTexture(const String &name, 
        const GLSurfaceDesc &target, bool writeGamma, uint fsaa)
    {
        GLFBORenderTexture *retval = new GLFBORenderTexture(this, name, target, writeGamma, fsaa);
        return retval;
    }
    //---------------------------------------------------------------------
    void GLFBOManager::bind(RenderTarget *target)
    {
        /// Check if the render target is in the rendertarget->FBO map
        if(auto fbo = dynamic_cast<GLRenderTarget*>(target)->getFBO())
            fbo->bind(true);
        else
            // Old style context (window/pbuffer) or copying render texture
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    }
    
    GLSurfaceDesc GLFBOManager::requestRenderBuffer(GLenum format, uint32 width, uint32 height, uint fsaa)
    {
        GLSurfaceDesc retval;
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
                GLRenderBuffer *rb = new GLRenderBuffer(format, width, height, fsaa);
                mRenderBufferMap[key] = RBRef(rb);
                retval.buffer = rb;
                retval.zoffset = 0;
                retval.numSamples = fsaa;
            }
        }
        //std::cerr << "Requested renderbuffer with format " << std::hex << format << std::dec << " of " << width << "x" << height << " :" << retval.buffer << std::endl;
        return retval;
    }
}
