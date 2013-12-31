/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
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

#include "OgreGLESPBRenderTexture.h"
#include "OgreGLESPBuffer.h"
#include "OgreGLESContext.h"
#include "OgreGLESPixelFormat.h"
#include "OgreGLESHardwarePixelBuffer.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreRoot.h"


namespace Ogre {
    GLESPBuffer::GLESPBuffer(PixelComponentType format, size_t width, size_t height)
        : mFormat(format),
          mWidth(width),
          mHeight(height)
    {
    }

    GLESPBuffer::~GLESPBuffer()
    {
    }

    GLESPBRenderTexture::GLESPBRenderTexture(GLESPBRTTManager *manager, const String &name,
                                             const GLESSurfaceDesc &target,
                                             bool writeGamma, uint fsaa)
        : GLESRenderTexture(name, target, writeGamma, fsaa),
          mManager(manager)
    {
        mPBFormat = PixelUtil::getComponentType(target.buffer->getFormat());
        mManager->requestPBuffer(mPBFormat, mWidth, mHeight);
    }

    GLESPBRenderTexture::~GLESPBRenderTexture()
    {
        // Release PBuffer
        mManager->releasePBuffer(mPBFormat);
    }

    void GLESPBRenderTexture::getCustomAttribute(const String& name, void* pData)
    {
        if (name == "TARGET")
        {
            GLESSurfaceDesc &target = *static_cast<GLESSurfaceDesc*>(pData);
            target.buffer = static_cast<GLESHardwarePixelBuffer*>(mBuffer);
            target.zoffset = mZOffset;
        }
    }
    
    GLESPBRTTManager::GLESPBRTTManager(GLESSupport *support, RenderTarget *mainwindow)
        : mSupport(support),
          mMainWindow(mainwindow),
          mMainContext(0)
    {
        mMainWindow->getCustomAttribute("GLCONTEXT", &mMainContext);
    }

    GLESPBRTTManager::~GLESPBRTTManager()
    {
        // Delete remaining PBuffers
        for (size_t x = 0; x < PCT_COUNT; ++x)
        {
            OGRE_DELETE mPBuffers[x].pb;
        }
    }

    RenderTexture *GLESPBRTTManager::createRenderTexture(const String &name,
                                                         const GLESSurfaceDesc &target,
                                                         bool writeGamma, uint fsaa)
    {
        return OGRE_NEW GLESPBRenderTexture(this, name, target, writeGamma, fsaa);
    }

    bool GLESPBRTTManager::checkFormat(PixelFormat format)
    {
        return true;
    }

    void GLESPBRTTManager::bind(RenderTarget *target)
    {
        // Nothing to do here
        // Binding of context is done by GL subsystem, as contexts are also used for RenderWindows
    }

    void GLESPBRTTManager::unbind(RenderTarget *target)
    {
        // Copy on unbind
        GLESSurfaceDesc surface;
        surface.buffer = 0;
        target->getCustomAttribute("TARGET", &surface);
        if (surface.buffer)
        {
            static_cast<GLESTextureBuffer*>(surface.buffer)->copyFromFramebuffer(surface.zoffset);
        }
    }

    void GLESPBRTTManager::requestPBuffer(PixelComponentType ctype, size_t width, size_t height)
    {
        // Check size
        if (mPBuffers[ctype].pb)
        {
            if (mPBuffers[ctype].pb->getWidth() < width ||
                mPBuffers[ctype].pb->getHeight() < height)
            {
                // If the current PBuffer is too small, destroy it and create a new one
                OGRE_DELETE mPBuffers[ctype].pb;
                mPBuffers[ctype].pb = 0;
            }
        }

        if (!mPBuffers[ctype].pb)
        {
            // Create pbuffer via rendersystem
            mPBuffers[ctype].pb = mSupport->createPBuffer(ctype, width, height);
        }
        ++mPBuffers[ctype].refcount;
    }

    void GLESPBRTTManager::releasePBuffer(PixelComponentType ctype)
    {
        --mPBuffers[ctype].refcount;
        if (mPBuffers[ctype].refcount == 0)
        {
            OGRE_DELETE mPBuffers[ctype].pb;
            mPBuffers[ctype].pb = 0;
        }
    }
}
