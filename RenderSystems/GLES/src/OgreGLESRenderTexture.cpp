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
        return new GLESCopyingRenderTexture(this, name, target, writeGamma, fsaa);
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
