/*
  -----------------------------------------------------------------------------
  This source file is part of OGRE
  (Object-oriented Graphics Rendering Engine)
  For the latest info, see http://www.ogre3d.org

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

#include "OgreGLRenderTexture.h"
#include "OgreGLHardwarePixelBufferCommon.h"
#include "OgreGLRenderSystemCommon.h"
#include "OgreRoot.h"

namespace Ogre {

    const String GLRenderTexture::CustomAttributeString_FBO = "FBO";
    const String GLRenderTexture::CustomAttributeString_TARGET = "TARGET";
    const String GLRenderTexture::CustomAttributeString_GLCONTEXT = "GLCONTEXT";

    template<> GLRTTManager* Singleton<GLRTTManager>::msSingleton = NULL;

    GLFrameBufferObjectCommon::GLFrameBufferObjectCommon(int32 fsaa)
        : mFB(0), mMultisampleFB(0), mNumSamples(fsaa)
    {
        auto* rs = static_cast<GLRenderSystemCommon*>(
            Root::getSingleton().getRenderSystem());
        mContext = rs->_getCurrentContext();

        // Initialise state
        mDepth.buffer = 0;
        mStencil.buffer = 0;
        for(size_t x = 0; x < OGRE_MAX_MULTIPLE_RENDER_TARGETS; ++x)
        {
            mColour[x].buffer=0;
        }
    }

    void GLFrameBufferObjectCommon::bindSurface(size_t attachment, const GLSurfaceDesc &target)
    {
        assert(attachment < OGRE_MAX_MULTIPLE_RENDER_TARGETS);
        mColour[attachment] = target;
        // Re-initialise
        if(mColour[0].buffer)
            initialise();
    }

    void GLFrameBufferObjectCommon::unbindSurface(size_t attachment)
    {
        assert(attachment < OGRE_MAX_MULTIPLE_RENDER_TARGETS);
        mColour[attachment].buffer = 0;
        // Re-initialise if buffer 0 still bound
        if(mColour[0].buffer)
            initialise();
    }

    uint32 GLFrameBufferObjectCommon::getWidth() const
    {
        assert(mColour[0].buffer);
        return mColour[0].buffer->getWidth();
    }
    uint32 GLFrameBufferObjectCommon::getHeight() const
    {
        assert(mColour[0].buffer);
        return mColour[0].buffer->getHeight();
    }
    PixelFormat GLFrameBufferObjectCommon::getFormat() const
    {
        assert(mColour[0].buffer);
        return mColour[0].buffer->getFormat();
    }

    GLRTTManager* GLRTTManager::getSingletonPtr(void)
    {
        return msSingleton;
    }
    GLRTTManager& GLRTTManager::getSingleton(void)
    {
        assert( msSingleton );  return ( *msSingleton );
    }

    GLRTTManager::GLRTTManager() {}
    // need to implement in cpp due to how Ogre::Singleton works
    GLRTTManager::~GLRTTManager() {}

    PixelFormat GLRTTManager::getSupportedAlternative(PixelFormat format)
    {
        if (checkFormat(format))
        {
            return format;
        }

        /// Find first alternative
        PixelComponentType pct = PixelUtil::getComponentType(format);

        switch (pct)
        {
        case PCT_BYTE:
            format = PF_BYTE_RGBA; // native endian
            break;
        case PCT_SHORT:
            format = PF_SHORT_RGBA;
            break;
        case PCT_FLOAT16:
            format = PF_FLOAT16_RGBA;
            break;
        case PCT_FLOAT32:
            format = PF_FLOAT32_RGBA;
            break;
        case PCT_COUNT:
        default:
            break;
        }

        if (checkFormat(format))
            return format;

        /// If none at all, return to default
        return PF_BYTE_RGBA; // native endian
    }

    void GLRTTManager::releaseRenderBuffer(const GLSurfaceDesc &surface)
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
                //                              std::cerr << "Destroyed renderbuffer of format " << std::hex << key.format << std::dec
                //                                      << " of " << key.width << "x" << key.height << std::endl;
            }
        }
    }

    GLRenderTexture::GLRenderTexture(const String &name,
                                               const GLSurfaceDesc &target,
                                               bool writeGamma,
                                               uint fsaa)
        : RenderTexture(target.buffer, target.zoffset)
    {
        mName = name;
        mHwGamma = writeGamma;
        mFSAA = fsaa;
    }
}
