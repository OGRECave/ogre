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
#include "OgreGL3PlusRenderSystem.h"
#include "OgreGL3PlusStateCacheManager.h"

namespace Ogre {

    GL3PlusFBORenderTexture::GL3PlusFBORenderTexture(
        GL3PlusFBOManager *manager, const String &name,
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

    void GL3PlusFBORenderTexture::getCustomAttribute(const String& name, void* pData)
    {
        if(name == GLRenderTexture::CustomAttributeString_FBO)
        {
            *static_cast<GL3PlusFrameBufferObject **>(pData) = &mFB;
        }
        else if(name == GLRenderTexture::CustomAttributeString_GLCONTEXT)
        {
            *static_cast<GLContext**>(pData) = getContext();
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

    bool GL3PlusFBORenderTexture::attachDepthBuffer( DepthBuffer *depthBuffer )
    {
        bool result;
        if( (result = GLRenderTexture::attachDepthBuffer( depthBuffer )) )
            mFB.attachDepthBuffer( depthBuffer );

        return result;
    }

    void GL3PlusFBORenderTexture::detachDepthBuffer()
    {
        mFB.detachDepthBuffer();
        GLRenderTexture::detachDepthBuffer();
    }

    void GL3PlusFBORenderTexture::_detachDepthBuffer()
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
            GL_STENCIL_INDEX1,
            GL_STENCIL_INDEX4,
            GL_STENCIL_INDEX8,
            GL_STENCIL_INDEX16
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
            GL_DEPTH_COMPONENT24,   // Prefer 24 bit depth
            GL_DEPTH_COMPONENT32,
            GL_DEPTH_COMPONENT32F,
            GL_DEPTH24_STENCIL8,    // Packed depth / stencil
            GL_DEPTH32F_STENCIL8
        };
    static const uchar depthBits[] =
        {
            0,16,24,32,32,24,32
        };
#define DEPTHFORMAT_COUNT (sizeof(depthFormats)/sizeof(GLenum))

    GL3PlusFBOManager::GL3PlusFBOManager(GL3PlusRenderSystem* renderSystem) : mRenderSystem(renderSystem)
    {
        detectFBOFormats();
    }

    GL3PlusFBOManager::~GL3PlusFBOManager()
    {
        if(!mRenderBufferMap.empty())
        {
            LogManager::getSingleton().logWarning("GL3PlusFBOManager destructor called, but not all renderbuffers were released.");
        }
    }

    GL3PlusStateCacheManager* GL3PlusFBOManager::getStateCacheManager()
    {
        return mRenderSystem->_getStateCacheManager();
    }

    void GL3PlusFBOManager::_createTempFramebuffer(GLuint internalFormat, GLuint fmt, GLenum dataType, GLuint &fb, GLuint &tid)
    {
        OGRE_CHECK_GL_ERROR(glGenFramebuffers(1, &fb));
        mRenderSystem->_getStateCacheManager()->bindGLFrameBuffer( GL_DRAW_FRAMEBUFFER, fb );
        if (fmt != GL_NONE)
        {
            if (tid)
            {
                OGRE_CHECK_GL_ERROR(glDeleteTextures(1, &tid));
                mRenderSystem->_getStateCacheManager()->invalidateStateForTexture( tid );
            }

            // Create and attach texture
            OGRE_CHECK_GL_ERROR(glGenTextures(1, &tid));
            mRenderSystem->_getStateCacheManager()->bindGLTexture( GL_TEXTURE_2D, tid );

            // Set some default parameters
            OGRE_CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0));
            OGRE_CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0));
            OGRE_CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
            OGRE_CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
            OGRE_CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
            OGRE_CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

            OGRE_CHECK_GL_ERROR(glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, PROBE_SIZE, PROBE_SIZE, 0, fmt, dataType, 0));

            OGRE_CHECK_GL_ERROR(glFramebufferTexture2D(
                GL_DRAW_FRAMEBUFFER, fmt == GL_DEPTH_COMPONENT ? GL_DEPTH_ATTACHMENT : GL_COLOR_ATTACHMENT0,
                GL_TEXTURE_2D, tid, 0));
        }
        else
        {
            // Draw to nowhere -- stencil/depth only
            OGRE_CHECK_GL_ERROR(glDrawBuffer(GL_NONE));
            OGRE_CHECK_GL_ERROR(glReadBuffer(GL_NONE));
        }
    }

    /** Try a certain FBO format, and return the status. Also sets mDepthRB and mStencilRB.
        @returns true    if this combo is supported
        false   if this combo is not supported
    */
    GLuint GL3PlusFBOManager::_tryFormat(GLenum depthFormat, GLenum stencilFormat)
    {
        GLuint status, depthRB = 0, stencilRB = 0;

        if (depthFormat != GL_NONE)
        {
            // Generate depth renderbuffer
            OGRE_CHECK_GL_ERROR(glGenRenderbuffers(1, &depthRB));

            // Bind it to FBO
            mRenderSystem->_getStateCacheManager()->bindGLRenderBuffer( depthRB );

            // Allocate storage for depth buffer
            OGRE_CHECK_GL_ERROR(glRenderbufferStorage(GL_RENDERBUFFER, depthFormat,
                                                      PROBE_SIZE, PROBE_SIZE));

            // Attach depth
            OGRE_CHECK_GL_ERROR(glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRB));
        }

        if (stencilFormat != GL_NONE)
        {
            // Generate stencil renderbuffer
            OGRE_CHECK_GL_ERROR(glGenRenderbuffers(1, &stencilRB));
            // Bind it to FBO
            mRenderSystem->_getStateCacheManager()->bindGLRenderBuffer( stencilRB );

            // Allocate storage for stencil buffer
            OGRE_CHECK_GL_ERROR(glRenderbufferStorage(GL_RENDERBUFFER, stencilFormat, PROBE_SIZE, PROBE_SIZE));

            // Attach stencil
            OGRE_CHECK_GL_ERROR(glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                                                          GL_RENDERBUFFER, stencilRB));
        }

        OGRE_CHECK_GL_ERROR(status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));

        // If status is negative, clean up
        // Detach and destroy
        OGRE_CHECK_GL_ERROR(glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0));
        OGRE_CHECK_GL_ERROR(glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0));

        if (depthRB)
            mRenderSystem->_getStateCacheManager()->deleteGLRenderBuffer(depthRB);

        if (stencilRB)
            mRenderSystem->_getStateCacheManager()->deleteGLRenderBuffer(stencilRB);

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
        OGRE_CHECK_GL_ERROR(glGenRenderbuffers(1, &packedRB));

        // Bind it to FBO
        mRenderSystem->_getStateCacheManager()->bindGLRenderBuffer( packedRB );

        // Allocate storage for buffer
        OGRE_CHECK_GL_ERROR(glRenderbufferStorage(GL_RENDERBUFFER, packedFormat, PROBE_SIZE, PROBE_SIZE));

        // Attach depth
        OGRE_CHECK_GL_ERROR(glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                                      GL_RENDERBUFFER, packedRB));

        // Attach stencil
        OGRE_CHECK_GL_ERROR(glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                                                      GL_RENDERBUFFER, packedRB));

        GLuint status;
        OGRE_CHECK_GL_ERROR(status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));

        // Detach and destroy
        OGRE_CHECK_GL_ERROR(glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0));
        OGRE_CHECK_GL_ERROR(glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0));
        mRenderSystem->_getStateCacheManager()->deleteGLRenderBuffer(packedRB);

        return status == GL_FRAMEBUFFER_COMPLETE;
    }

    /** Detect which internal formats are allowed as RTT
        Also detect what combinations of stencil and depth are allowed with this internal
        format.
    */
    void GL3PlusFBOManager::detectFBOFormats()
    {
        // is glGetInternalformativ supported?
        // core since GL 4.2: see https://www.opengl.org/wiki/GLAPI/glGetInternalformat
        // NOTE! GL_FRAMEBUFFER_RENDERABLE is supported only if the GL version is 4.3 or higher
        bool hasInternalFormatQuery = mRenderSystem->hasMinGLVersion(4, 3) || mRenderSystem->checkExtension("GL_ARB_internalformat_query2");

        // Try all formats, and report which ones work as target
        GLuint fb = 0, tid = 0;

        bool formatSupported;
        GLint params;

        for(int x = 0; x < PF_COUNT; ++x)
        {
            mProps[x].valid = false;

            // Fetch GL format token
            GLenum internalFormat = GL3PlusPixelUtil::getGLInternalFormat((PixelFormat)x);
            GLenum originFormat = GL3PlusPixelUtil::getGLOriginFormat((PixelFormat)x);
            GLenum dataType = GL3PlusPixelUtil::getGLOriginDataType((PixelFormat)x);
            if(internalFormat == GL_NONE && x != 0)
                continue;

            // No test for compressed formats
            if(PixelUtil::isCompressed((PixelFormat)x))
                continue;

            if (hasInternalFormatQuery) {
                OGRE_CHECK_GL_ERROR(
                        glGetInternalformativ(GL_RENDERBUFFER, internalFormat, GL_FRAMEBUFFER_RENDERABLE, 1, &params));
                formatSupported = params == GL_FULL_SUPPORT;
            } else {
                // Create and attach framebuffer
                _createTempFramebuffer(internalFormat, originFormat, dataType, fb, tid);

                // Check status
                GLuint status;
                OGRE_CHECK_GL_ERROR(status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));
                formatSupported = status == GL_FRAMEBUFFER_COMPLETE;
            }

            // Ignore status in case of fmt==GL_NONE, because no implementation will accept
            // a buffer without *any* attachment. Buffers with only stencil and depth attachment
            // might still be supported, so we must continue probing.
            if(internalFormat == GL_NONE || formatSupported)
            {
                mProps[x].valid = true;
                StringStream str;
                str << "FBO " << PixelUtil::getFormatName((PixelFormat)x)
                    << " depth/stencil support: ";

                // For each depth/stencil formats
                for (uchar depth = 0; depth < DEPTHFORMAT_COUNT; ++depth)
                {
                    if ((depthFormats[depth] != GL_DEPTH24_STENCIL8) && (depthFormats[depth] != GL_DEPTH32F_STENCIL8))
                    {
                        // General depth/stencil combination

                        if(hasInternalFormatQuery) {
                            OGRE_CHECK_GL_ERROR(
                                    glGetInternalformativ(GL_RENDERBUFFER, depthFormats[depth], GL_FRAMEBUFFER_RENDERABLE, 1, &params));

                            // skip unsupported depth unless it is GL_NONE, as we still want D0Sxx formats
                            if(params != GL_FULL_SUPPORT && depthFormats[depth] != GL_NONE) {
                                continue;
                            }
                        }

                        for (uchar stencil = 0; stencil < STENCILFORMAT_COUNT; ++stencil)
                        {
                            //                            StringStream l;
                            //                            l << "Trying " << PixelUtil::getFormatName((PixelFormat)x)
                            //                                  << " D" << depthBits[depth]
                            //                                  << "S" << stencilBits[stencil];
                            //                            LogManager::getSingleton().logMessage(l.str());

                            if (hasInternalFormatQuery) {
                                OGRE_CHECK_GL_ERROR(
                                        glGetInternalformativ( GL_RENDERBUFFER, stencilFormats[stencil], GL_FRAMEBUFFER_RENDERABLE, 1, &params));

                                // skip unsupported stencil unless it is GL_NONE, as we still want DxxS0 formats
                                formatSupported = params == GL_FULL_SUPPORT || stencilFormats[stencil] == GL_NONE;
                            } else {
                                formatSupported = _tryFormat(depthFormats[depth], stencilFormats[stencil]) != 0;
                            }

                            if (formatSupported)
                            {
                                // Add mode to allowed modes
                                str << StringUtil::format("D%dS%d ", depthBits[depth], stencilBits[stencil]);
                                FormatProperties::Mode mode = {depth, stencil};
                                mProps[x].modes.push_back(mode);
                            }
                        }
                    }
                    else
                    {
                        // Packed depth/stencil format
                        if (hasInternalFormatQuery) {
                            OGRE_CHECK_GL_ERROR(
                                    glGetInternalformativ( GL_RENDERBUFFER, depthFormats[depth], GL_FRAMEBUFFER_RENDERABLE, 1, &params));

                            formatSupported = params == GL_FULL_SUPPORT;
                        } else {
                            formatSupported = _tryPackedFormat(depthFormats[depth]);
                        }

                        if (formatSupported)
                        {
                            // Add mode to allowed modes
                            str << "Packed-D" << int(depthBits[depth]) << "S8 ";
                            FormatProperties::Mode mode = {depth, 0}; // stencil unused
                            mProps[x].modes.push_back(mode);
                        }
                    }
                }
                LogManager::getSingleton().logMessage(str.str());
            }

            if (!hasInternalFormatQuery) {
                // Delete texture and framebuffer
                mRenderSystem->_getStateCacheManager()->bindGLFrameBuffer( GL_DRAW_FRAMEBUFFER, 0 );
                mRenderSystem->_getStateCacheManager()->deleteGLRenderBuffer(fb);

                if (internalFormat != GL_NONE) 
                {
                    OGRE_CHECK_GL_ERROR(glDeleteTextures(1, &tid));
                    mRenderSystem->_getStateCacheManager()->invalidateStateForTexture( tid );
                    tid = 0;
                }
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

    void GL3PlusFBOManager::getBestDepthStencil(PixelFormat internalFormat, GLenum *depthFormat, GLenum *stencilFormat)
    {
        const FormatProperties &props = mProps[internalFormat];
        // Decide what stencil and depth formats to use
        // [best supported for internal format]
        size_t bestmode=0;
        int bestscore=-1;
        bool requestDepthOnly = PixelUtil::isDepth(internalFormat);
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
        *stencilFormat = requestDepthOnly ? 0 : stencilFormats[props.modes[bestmode].stencil];
    }

    GL3PlusFBORenderTexture *GL3PlusFBOManager::createRenderTexture(const String &name,
                                                                    const GLSurfaceDesc &target, bool writeGamma, uint fsaa)
    {
        GL3PlusFBORenderTexture *retval = new GL3PlusFBORenderTexture(this, name, target, writeGamma, fsaa);
        return retval;
    }

    GLSurfaceDesc GL3PlusFBOManager::requestRenderBuffer(GLenum format, uint32 width, uint32 height, uint fsaa)
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

}
