/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
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

#include "OgreGLESRenderTexture.h"
#include "OgreGLESPixelFormat.h"
#include "OgreGLESHardwarePixelBuffer.h"

#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreRoot.h"

namespace Ogre {
    template<> GLESRTTManager* Singleton<GLESRTTManager>::ms_Singleton = 0;

    GLESRTTManager::~GLESRTTManager()
    {
    }

    MultiRenderTarget* GLESRTTManager::createMultiRenderTarget(const String & name)
    {
        // TODO: Check rendersystem capabilities before throwing the exception
        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
                    "MultiRenderTarget can only be used with GL_OES_framebuffer_object extension",
                    "GLESRTTManager::createMultiRenderTarget");
    }

    PixelFormat GLESRTTManager::getSupportedAlternative(PixelFormat format)
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
                format = PF_A8R8G8B8;
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
        return PF_A8R8G8B8;
    }

    GLESRenderTexture::GLESRenderTexture(const String &name,
                                         const GLESSurfaceDesc &target,
                                         bool writeGamma,
                                         uint fsaa)
        : RenderTexture(target.buffer, target.zoffset)
    {
        mName = name;
        mHwGamma = writeGamma;
        mFSAA = fsaa;
    }

    GLESRenderTexture::~GLESRenderTexture()
    {
    }

    GLESCopyingRenderTexture::GLESCopyingRenderTexture(GLESCopyingRTTManager *manager,
                                                       const String &name,
                                                       const GLESSurfaceDesc &target,
                                                       bool writeGamma, uint fsaa)
        : GLESRenderTexture(name, target, writeGamma, fsaa)
    {
    }

    void GLESCopyingRenderTexture::getCustomAttribute(const String& name, void* pData)
    {
        if (name=="TARGET")
        {
            GLESSurfaceDesc &target = *static_cast<GLESSurfaceDesc*>(pData);
            target.buffer = static_cast<GLESHardwarePixelBuffer*>(mBuffer);
            target.zoffset = mZOffset;
        }
    }

    GLESCopyingRTTManager::GLESCopyingRTTManager()
    {
    }

    GLESCopyingRTTManager::~GLESCopyingRTTManager()
    {
    }

    RenderTexture *GLESCopyingRTTManager::createRenderTexture(const String &name,
                                                              const GLESSurfaceDesc &target,
                                                              bool writeGamma, uint fsaa)
    {
        return OGRE_NEW GLESCopyingRenderTexture(this, name, target, writeGamma, fsaa);
    }

    bool GLESCopyingRTTManager::checkFormat(PixelFormat format)
    {
        return true;
    }

    void GLESCopyingRTTManager::bind(RenderTarget *target)
    {
        // Nothing to do here
    }

    void GLESCopyingRTTManager::unbind(RenderTarget *target)
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
}
