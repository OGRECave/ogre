/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
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
        else if (name == "GLCONTEXT")
        {
            // Get PBuffer for our internal format
            *static_cast<GLESContext**>(pData) =
                mManager->getContextFor(mPBFormat, mWidth, mHeight);
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
            delete mPBuffers[x].pb;
        }
    }

    RenderTexture *GLESPBRTTManager::createRenderTexture(const String &name,
                                                         const GLESSurfaceDesc &target,
                                                         bool writeGamma, uint fsaa)
    {
        return new GLESPBRenderTexture(this, name, target, writeGamma, fsaa);
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
                delete mPBuffers[ctype].pb;
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
            delete mPBuffers[ctype].pb;
            mPBuffers[ctype].pb = 0;
        }
    }

    GLESContext *GLESPBRTTManager::getContextFor(PixelComponentType ctype,
                                                 size_t width, size_t height)
    {
        // Faster to return main context if the RTT is smaller than the window size
        // and ctype is PCT_BYTE. This must be checked every time because the window might have been resized
        if (ctype == PCT_BYTE)
        {
            if (width <= mMainWindow->getWidth() &&
                height <= mMainWindow->getHeight())
            {
                return mMainContext;
            }
        }
        assert(mPBuffers[ctype].pb);
        return mPBuffers[ctype].pb->getContext();
    }
}
