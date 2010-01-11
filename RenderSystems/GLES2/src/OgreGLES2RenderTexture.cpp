/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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

#include "OgreGLES2RenderTexture.h"
#include "OgreGLES2HardwarePixelBuffer.h"

namespace Ogre {
    template<> GLES2RTTManager* Singleton<GLES2RTTManager>::ms_Singleton = 0;

    GLES2RTTManager::~GLES2RTTManager()
    {
    }

    MultiRenderTarget* GLES2RTTManager::createMultiRenderTarget(const String & name)
    {
        // TODO: Check rendersystem capabilities before throwing the exception
        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
                    "MultiRenderTarget is not supported",
                    "GLES2RTTManager::createMultiRenderTarget");
    }

    PixelFormat GLES2RTTManager::getSupportedAlternative(PixelFormat format)
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

    GLES2RenderTexture::GLES2RenderTexture(const String &name,
                                         const GLES2SurfaceDesc &target,
                                         bool writeGamma,
                                         uint fsaa)
        : RenderTexture(target.buffer, target.zoffset)
    {
        mName = name;
        mHwGamma = writeGamma;
        mFSAA = fsaa;
    }

    GLES2RenderTexture::~GLES2RenderTexture()
    {
    }

    GLES2CopyingRenderTexture::GLES2CopyingRenderTexture(GLES2CopyingRTTManager *manager,
                                                       const String &name,
                                                       const GLES2SurfaceDesc &target,
                                                       bool writeGamma, uint fsaa)
        : GLES2RenderTexture(name, target, writeGamma, fsaa)
    {
    }

    void GLES2CopyingRenderTexture::getCustomAttribute(const String& name, void* pData)
    {
        if (name=="TARGET")
        {
            GLES2SurfaceDesc &target = *static_cast<GLES2SurfaceDesc*>(pData);
            target.buffer = static_cast<GLES2HardwarePixelBuffer*>(mBuffer);
            target.zoffset = mZOffset;
        }
    }

    GLES2CopyingRTTManager::GLES2CopyingRTTManager()
    {
    }

    GLES2CopyingRTTManager::~GLES2CopyingRTTManager()
    {
    }

    RenderTexture *GLES2CopyingRTTManager::createRenderTexture(const String &name,
                                                              const GLES2SurfaceDesc &target,
                                                              bool writeGamma, uint fsaa)
    {
        return OGRE_NEW GLES2CopyingRenderTexture(this, name, target, writeGamma, fsaa);
    }

    bool GLES2CopyingRTTManager::checkFormat(PixelFormat format)
    {
        return true;
    }

    void GLES2CopyingRTTManager::bind(RenderTarget *target)
    {
        // Nothing to do here
    }

    void GLES2CopyingRTTManager::unbind(RenderTarget *target)
    {
        // Copy on unbind
        GLES2SurfaceDesc surface;
        surface.buffer = 0;
        target->getCustomAttribute("TARGET", &surface);
        if (surface.buffer)
        {
            static_cast<GLES2TextureBuffer*>(surface.buffer)->copyFromFramebuffer(surface.zoffset);
        }
    }
}
