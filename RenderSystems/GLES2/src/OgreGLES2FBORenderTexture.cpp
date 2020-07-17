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

#include "OgreGLES2FBORenderTexture.h"
#include "OgreGLES2PixelFormat.h"
#include "OgreLogManager.h"
#include "OgreGLES2HardwarePixelBuffer.h"
#include "OgreGLES2FBOMultiRenderTarget.h"
#include "OgreRoot.h"
#include "OgreGLES2RenderSystem.h"
#include "OgreGLUtil.h"
#include "OgreGLNativeSupport.h"

namespace Ogre {

//-----------------------------------------------------------------------------    
    GLES2FBORenderTexture::GLES2FBORenderTexture(GLES2FBOManager *manager, const String &name,
        const GLSurfaceDesc &target, bool writeGamma, uint fsaa):
        GLRenderTexture(name, target, writeGamma, std::min(manager->getMaxFSAASamples(), (int)fsaa)),
        mFB(manager, mFSAA)
    {
        // Bind target to surface 0 and initialise
        mFB.bindSurface(0, target);

        // Get attributes
        mWidth = mFB.getWidth();
        mHeight = mFB.getHeight();
    }
    
    void GLES2FBORenderTexture::getCustomAttribute(const String& name, void* pData)
    {
        if(name == GLRenderTexture::CustomAttributeString_FBO)
        {
            *static_cast<GLES2FrameBufferObject **>(pData) = &mFB;
        }
        else if(name == GLRenderTexture::CustomAttributeString_GLCONTEXT)
        {
            *static_cast<GLContext**>(pData) = mFB.getContext();
        }
    }

    void GLES2FBORenderTexture::swapBuffers()
    {
        mFB.swapBuffers();
    }
    
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID || OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
    void GLES2FBORenderTexture::notifyOnContextLost()
    {
        mFB.notifyOnContextLost();
    }
    
    void GLES2FBORenderTexture::notifyOnContextReset()
    {
        GLSurfaceDesc target;
        target.buffer = static_cast<GLES2HardwarePixelBuffer*>(mBuffer);
        target.zoffset = mZOffset;
        
        mFB.notifyOnContextReset(target);
        
        static_cast<GLES2RenderSystem*>(Ogre::Root::getSingletonPtr()->getRenderSystem())->_createDepthBufferFor(this);
    }
#endif
    
    //-----------------------------------------------------------------------------
    bool GLES2FBORenderTexture::attachDepthBuffer( DepthBuffer *depthBuffer )
    {
        bool result;
        if( (result = GLRenderTexture::attachDepthBuffer( depthBuffer )) )
            mFB.attachDepthBuffer( depthBuffer );

        return result;
    }
    //-----------------------------------------------------------------------------
    void GLES2FBORenderTexture::detachDepthBuffer()
    {
        mFB.detachDepthBuffer();
        GLRenderTexture::detachDepthBuffer();
    }
    //-----------------------------------------------------------------------------
    void GLES2FBORenderTexture::_detachDepthBuffer()
    {
        mFB.detachDepthBuffer();
        GLRenderTexture::_detachDepthBuffer();
    }
   
    // Size of probe texture
    #define PROBE_SIZE 16

    // Stencil and depth formats to be tried
    static const GLenum stencilFormats[] =
    {
        GL_NONE,                    // No stencil
        GL_STENCIL_INDEX1_OES,
        GL_STENCIL_INDEX4_OES,
        GL_STENCIL_INDEX8
    };
    static const uchar stencilBits[] =
    {
        0,
        1,
        4,
        8
    };
    #define STENCILFORMAT_COUNT (sizeof(stencilFormats)/sizeof(GLenum))

    static const GLenum depthFormats[] =
    {
        GL_NONE,
        GL_DEPTH_COMPONENT16
        , GL_DEPTH_COMPONENT24_OES   // Prefer 24 bit depth
        , GL_DEPTH_COMPONENT32_OES
        , GL_DEPTH24_STENCIL8_OES    // Packed depth / stencil
        , GL_DEPTH32F_STENCIL8
    };
    static const uchar depthBits[] =
    {
        0
        ,16
        ,24
        ,32
        ,24
        ,32
    };
    #define DEPTHFORMAT_COUNT (sizeof(depthFormats)/sizeof(GLenum))

    GLES2FBOManager::GLES2FBOManager() : mMaxFSAASamples(0)
    {
        detectFBOFormats();
        
        OGRE_CHECK_GL_ERROR(glGenFramebuffers(1, &mTempFBO));

        // Check multisampling if supported
        if(getGLES2RenderSystem()->hasMinGLVersion(3, 0))
        {
            // Check samples supported
            OGRE_CHECK_GL_ERROR(glGetIntegerv(GL_MAX_SAMPLES_APPLE, &mMaxFSAASamples));
        }
    }

    GLES2FBOManager::~GLES2FBOManager()
    {
        if(!mRenderBufferMap.empty())
        {
            LogManager::getSingleton().logWarning("GLES2FBOManager destructor called, but not all renderbuffers were released.");
        }
        
        OGRE_CHECK_GL_ERROR(glDeleteFramebuffers(1, &mTempFBO));
    }
    
    void GLES2FBOManager::_reload()
    {
        OGRE_CHECK_GL_ERROR(glDeleteFramebuffers(1, &mTempFBO));
        
        detectFBOFormats();

        OGRE_CHECK_GL_ERROR(glGenFramebuffers(1, &mTempFBO));
    }

    void GLES2FBOManager::_createTempFramebuffer(GLuint internalFormat, GLuint fmt, GLenum type, GLuint &fb, GLuint &tid)
    {
        // Create and attach framebuffer
        glGenFramebuffers(1, &fb);
        glBindFramebuffer(GL_FRAMEBUFFER, fb);
        if (internalFormat != GL_NONE)
        {
            if (tid)
                glDeleteTextures(1, &tid);

            // Create and attach texture
            glGenTextures(1, &tid);
            glBindTexture(GL_TEXTURE_2D, tid);

            // Set some default parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, PROBE_SIZE, PROBE_SIZE, 0, fmt, type, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER,
                                   fmt == GL_DEPTH_COMPONENT ? GL_DEPTH_ATTACHMENT : GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_2D, tid, 0);
        }
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
            // Generate depth renderbuffer
            glGenRenderbuffers(1, &depthRB);

            // Bind it to FBO
            glBindRenderbuffer(GL_RENDERBUFFER, depthRB);

            // Allocate storage for depth buffer
            glRenderbufferStorage(GL_RENDERBUFFER, depthFormat,
                                PROBE_SIZE, PROBE_SIZE);

            // Attach depth
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRB);
        }

        // Stencil buffers aren't available on iOS
        if(stencilFormat != GL_NONE)
        {
            // Generate stencil renderbuffer
            glGenRenderbuffers(1, &stencilRB);

            // Bind it to FBO
            glBindRenderbuffer(GL_RENDERBUFFER, stencilRB);

            // Allocate storage for stencil buffer
            glRenderbufferStorage(GL_RENDERBUFFER, stencilFormat, PROBE_SIZE, PROBE_SIZE); 

            // Attach stencil
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, stencilRB);
        }

        status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

        // If status is negative, clean up
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

        // Generate renderbuffer
        glGenRenderbuffers(1, &packedRB);

        // Bind it to FBO
        glBindRenderbuffer(GL_RENDERBUFFER, packedRB);

        // Allocate storage for buffer
        glRenderbufferStorage(GL_RENDERBUFFER, packedFormat, PROBE_SIZE, PROBE_SIZE);

        // Attach depth
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
            GL_RENDERBUFFER, packedRB);

        // Attach stencil
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
            GL_RENDERBUFFER, packedRB);

        GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

        // Detach and destroy
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
        GLES2RenderSystem* rs = getGLES2RenderSystem();
        bool hasGLES3 = rs->hasMinGLVersion(3, 0);
#if OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
        memset(mProps, 0, sizeof(mProps));

        // TODO: Fix that probing all formats slows down startup not just on the web also on Android / iOS
        mProps[PF_A8B8G8R8].valid = true;
        FormatProperties::Mode mode = {1, 0};
        mProps[PF_A8B8G8R8].modes.push_back(mode);

        if(hasGLES3)
        {
            mProps[PF_DEPTH16].valid = true;
            mProps[PF_DEPTH16].modes.push_back(mode);
        }
        LogManager::getSingleton().logMessage("[GLES2] : detectFBOFormats is disabled on this platform (due performance reasons)");
#else
        // Try all formats, and report which ones work as target
        GLuint fb = 0, tid = 0;

        const size_t depthCount = hasGLES3 ? DEPTHFORMAT_COUNT : DEPTHFORMAT_COUNT - 1; // 32_8 is not available on GLES2
        const uchar stencilStep = hasGLES3 ? 3 : 1; // 1 and 4 bit not available on GLES3

        for(size_t x = 0; x < PF_COUNT; ++x)
        {
            mProps[x].valid = false;

            // Fetch GL format token
            GLint internalFormat = GLES2PixelUtil::getGLInternalFormat((PixelFormat)x);
            GLenum fmt = GLES2PixelUtil::getGLOriginFormat((PixelFormat)x);
            GLenum type = GLES2PixelUtil::getGLOriginDataType((PixelFormat)x);

            // Note: letting PF_UNKNOWN pass here is for pure depth/ stencil formats
            // however there are reports that this crashes some unspecified android devices
            if((internalFormat == GL_NONE || fmt == GL_NONE || type == GL_NONE) && (x != 0))
                continue;

            // not color-renderable in GLES
            if(fmt == GL_BGRA_EXT)
                continue;

            // No test for compressed formats
            if(PixelUtil::isCompressed((PixelFormat)x))
                continue;

            // Create and attach framebuffer
            _createTempFramebuffer(internalFormat, fmt, type, fb, tid);

            // Ignore status in case of fmt==GL_NONE, because no implementation will accept
            // a buffer without *any* attachment. Buffers with only stencil and depth attachment
            // might still be supported, so we must continue probing.
            if(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
            {
                mProps[x].valid = true;
                StringStream str;
                str << "FBO " << PixelUtil::getFormatName((PixelFormat)x) 
                    << " depth/stencil support: ";

                // For each depth/stencil formats
                for (uchar depth = 0; depth < depthCount; ++depth)
                {
                    if (depthFormats[depth] != GL_DEPTH24_STENCIL8 && depthFormats[depth] != GL_DEPTH32F_STENCIL8)
                    {
                        // General depth/stencil combination

                        for (uchar stencil = 0; stencil < STENCILFORMAT_COUNT; stencil += stencilStep)
                        {
//                            StringStream l;
//                            l << "Trying " << PixelUtil::getFormatName((PixelFormat)x) 
//                              << " D" << depthBits[depth] 
//                              << "S" << stencilBits[stencil];
//                            LogManager::getSingleton().logMessage(l.str());

                            if (_tryFormat(depthFormats[depth], stencilFormats[stencil]))
                            {
                                // Add mode to allowed modes
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
                                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                                glDeleteFramebuffers(1, &fb);

                                _createTempFramebuffer(internalFormat, fmt, type, fb, tid);
                            }
                        }
                    }
                    else if(hasGLES3 || rs->checkExtension("GL_OES_packed_depth_stencil") )
                    {
                        // Packed depth/stencil format
                        if (_tryPackedFormat(depthFormats[depth]))
                        {
                            // Add mode to allowed modes
                            str << "Packed-D" << int(depthBits[depth]) << "S8 ";
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

                            _createTempFramebuffer(internalFormat, fmt, type, fb, tid);
                        }
                    }
                }
                LogManager::getSingleton().logMessage(str.str());
            }

            // Delete texture and framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDeleteFramebuffers(1, &fb);
            
            if (internalFormat != GL_NONE)
            {
                glDeleteTextures(1, &tid);
                tid = 0;
            }
        }

        // Clear any errors
        glGetError();
#endif
        String fmtstring;
        for(size_t x = 0; x < PF_COUNT; ++x)
        {
            if(mProps[x].valid)
                fmtstring += PixelUtil::getFormatName((PixelFormat)x)+" ";
        }
        LogManager::getSingleton().logMessage("[GLES2] : Valid FBO targets " + fmtstring);

    }

    void GLES2FBOManager::getBestDepthStencil(PixelFormat internalFormat, GLenum *depthFormat, GLenum *stencilFormat)
    {
        const FormatProperties &props = mProps[internalFormat];
        if (props.modes.size() == 0 ) {
            *depthFormat = 0;
            *stencilFormat = 0;
            return;
        }
        // Decide what stencil and depth formats to use
        // [best supported for internal format]
        size_t bestmode = 0;
        int bestscore = -1;
        bool requestDepthOnly = PixelUtil::isDepth(internalFormat);

        for(size_t mode = 0; mode < props.modes.size(); mode++)
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
            if(depthFormats[props.modes[mode].depth] == GL_DEPTH24_STENCIL8_OES) // Prefer 24/8 packed
                desirability += 5000;
            if(depthFormats[props.modes[mode].depth] == GL_DEPTH32F_STENCIL8) // Prefer 32F/8 packed
                desirability += 5000;
            desirability += stencilBits[props.modes[mode].stencil] + depthBits[props.modes[mode].depth];
            
            if(desirability > bestscore)
            {
                bestscore = desirability;
                bestmode = mode;
            }
        }
        *depthFormat = depthFormats[props.modes[bestmode].depth];
        *stencilFormat = requestDepthOnly ? 0 : stencilFormats[props.modes[bestmode].stencil];
    }

    GLES2FBORenderTexture *GLES2FBOManager::createRenderTexture(const String &name, 
        const GLSurfaceDesc &target, bool writeGamma, uint fsaa)
    {
        GLES2FBORenderTexture *retval = new GLES2FBORenderTexture(this, name, target, writeGamma, fsaa);
        return retval;
    }

    void GLES2FBOManager::bind(RenderTarget *target)
    {
        if(auto fbo = dynamic_cast<GLRenderTarget*>(target)->getFBO())
            fbo->bind(true);
        else
        {
            // Non-multisampled screen buffer is FBO #1 on iOS, multisampled is yet another,
            // so give the target ability to influence decision which FBO to use
            GLuint mainfbo = 0;
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
            target->getCustomAttribute("GLFBO", &mainfbo);
#endif
            OGRE_CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, mainfbo));
        }
    }
    
    GLSurfaceDesc GLES2FBOManager::requestRenderBuffer(GLenum format, uint32 width, uint32 height, uint fsaa)
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
}
