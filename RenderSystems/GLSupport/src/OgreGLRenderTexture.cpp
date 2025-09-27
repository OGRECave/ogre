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
#include "OgreViewport.h"

namespace Ogre {

    const String GLRenderTexture::CustomAttributeString_FBO = "FBO";
    const String GLRenderTexture::CustomAttributeString_TARGET = "TARGET";
    const String GLRenderTexture::CustomAttributeString_GLCONTEXT = "GLCONTEXT";

    template<> GLRTTManager* Singleton<GLRTTManager>::msSingleton = NULL;

    GLFrameBufferObjectCommon::GLFrameBufferObjectCommon()
        : mContext(NULL), mFB(0), mMultisampleFB(0), mNumSamples(0), mRTTManager(GLRTTManager::getSingletonPtr())
    {
        // Initialise state
        mDepth.buffer = 0;
        mStencil.buffer = 0;
        for(auto & x : mColour)
        {
            x.buffer=0;
        }
    }

    GLFrameBufferObjectCommon::~GLFrameBufferObjectCommon()
    {
        mRTTManager->releaseRenderBuffer(mMultisampleColourBuffer);
    }

    void GLFrameBufferObjectCommon::bindSurface(size_t attachment, const GLSurfaceDesc &target)
    {
        assert(attachment < OGRE_MAX_MULTIPLE_RENDER_TARGETS);
        mColour[attachment] = target;
        // Re-initialise
        if(mColour[0].buffer)
        {
            if(mColour[1].buffer)
            {
                mColour[0].numSamples = 0;
                mContext = 0; // force re-init - e.g. when adding depth to MRT
            }
            mNumSamples = mColour[0].numSamples;
            bind(true);
            initialise();
        }
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

        if(PixelUtil::isDepth(format))
        {
            switch (format)
            {
                default:
                case PF_DEPTH16:
                    format = PF_FLOAT16_R;
                    break;
                case PF_DEPTH24_STENCIL8:
                case PF_DEPTH32F:
                case PF_DEPTH32:
                    format = PF_FLOAT32_R;
                    break;
            }
        }
        else
        {
            /// Find first alternative
            switch (PixelUtil::getComponentType(format))
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
            default:
                break;
            }
        }

        if (checkFormat(format))
            return format;

        /// If none at all, return to default
        return PF_BYTE_RGBA; // native endian
    }

    static uint32 getKey(unsigned format, uint32 width, uint32 height, uint fsaa, uint16 poolId)
    {
        uint32 key = HashCombine(0, format);
        key = HashCombine(key, width);
        key = HashCombine(key, height);
        key = HashCombine(key, fsaa);
        key = HashCombine(key, poolId);
        return key;
    }

    GLSurfaceDesc GLRTTManager::requestRenderBuffer(unsigned format, uint32 width, uint32 height, uint fsaa, uint16 poolId)
    {
        GLSurfaceDesc retval;
        retval.buffer = 0; // Return 0 buffer if GL_NONE is requested
        if (format != 0)
        {
            uint32 key = getKey(format, width, height, fsaa, poolId);
            RenderBufferMap::iterator it = mRenderBufferMap.find(key);
            if (it != mRenderBufferMap.end())
            {
                retval.buffer = it->second.buffer;
                // Increase refcount
                ++it->second.refcount;
            }
            else
            {
                // New one
                retval.buffer = createNewRenderBuffer(format, width, height, fsaa);
                mRenderBufferMap[key] = retval.buffer;
            }

            retval.zoffset = 0;
            retval.numSamples = fsaa;
            retval.poolId = poolId;
        }
        // std::cerr << "Requested renderbuffer with format " << std::hex << format << std::dec << " of " << width <<
        // "x" << height << " :" << retval.buffer << std::endl;
        return retval;
    }

    void GLRTTManager::releaseRenderBuffer(const GLSurfaceDesc &surface)
    {
        if(surface.buffer == 0)
            return;
        uint32 key = getKey(surface.buffer->getGLFormat(), surface.buffer->getWidth(), surface.buffer->getHeight(),
                            surface.numSamples, surface.poolId);
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

    void GLFrameBufferObjectCommon::requestRenderBuffer(unsigned format, uint32 width, uint32 height)
    {
        mMultisampleColourBuffer = mRTTManager->requestRenderBuffer(format, width, height, mNumSamples, mPoolId);
    }

    GLRenderTexture::GLRenderTexture(const String &name,
                                               const GLSurfaceDesc &target,
                                               bool writeGamma)
        : RenderTexture(target.buffer, target.zoffset)
    {
        mName = name;
        mHwGamma = writeGamma;
    }
}
