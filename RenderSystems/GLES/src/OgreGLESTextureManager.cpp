/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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

#include "OgreGLESTextureManager.h"
#include "OgreGLESRenderTexture.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"


namespace Ogre {
    GLESTextureManager::GLESTextureManager(GLESSupport& support)
        : TextureManager(), mGLSupport(support), mWarningTextureID(0)
    {
        GL_CHECK_ERROR;
        // Register with group manager
        ResourceGroupManager::getSingleton()._registerResourceManager(mResourceType, this);

        createWarningTexture();
    }

    GLESTextureManager::~GLESTextureManager()
    {
        // Unregister with group manager
        ResourceGroupManager::getSingleton()._unregisterResourceManager(mResourceType);
        // Delete warning texture
        glDeleteTextures(1, &mWarningTextureID);
        GL_CHECK_ERROR;
    }

    Resource* GLESTextureManager::createImpl(const String& name, ResourceHandle handle, 
                                           const String& group, bool isManual,
                                           ManualResourceLoader* loader,
                                           const NameValuePairList* createParams)
    {
        return new GLESTexture(this, name, handle, group, isManual, loader, mGLSupport);
    }

    //-----------------------------------------------------------------------------
    void GLESTextureManager::createWarningTexture()
    {
        // Generate warning texture
        size_t width = 8;
        size_t height = 8;

        // TODO convert to 5_6_5
        uint32* data = new uint32[width * height]; // 0xXXRRGGBB

        // Yellow/black stripes
        for(size_t y = 0; y < height; ++y)
        {
            for(size_t x = 0; x < width; ++x)
            {
                data[y * width + x] = (((x + y) % 8) < 4) ? 0x000000 : 0xFFFF00;
            }
        }

        // Create GL resource
        glGenTextures(1, &mWarningTextureID);
        GL_CHECK_ERROR;
        glBindTexture(GL_TEXTURE_2D, mWarningTextureID);
        GL_CHECK_ERROR;
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, (void*)data);
        GL_CHECK_ERROR;
        // Free memory
        delete [] data;
    }

    PixelFormat GLESTextureManager::getNativeFormat(TextureType ttype, PixelFormat format, int usage)
    {
        // Adjust requested parameters to capabilities
        const RenderSystemCapabilities *caps = Root::getSingleton().getRenderSystem()->getCapabilities();

        // Check compressed texture support
        // if a compressed format not supported, revert to PF_A8R8G8B8
        if (PixelUtil::isCompressed(format) &&
            (!caps->hasCapability(RSC_TEXTURE_COMPRESSION_DXT) || !caps->hasCapability(RSC_TEXTURE_COMPRESSION_PVRTC)))
        {
            return PF_A8R8G8B8;
        }
        // if floating point textures not supported, revert to PF_A8R8G8B8
        if (PixelUtil::isFloatingPoint(format) &&
            !caps->hasCapability(RSC_TEXTURE_FLOAT))
        {
            return PF_A8R8G8B8;
        }

        // Check if this is a valid rendertarget format
        if (usage & TU_RENDERTARGET)
        {
            /// Get closest supported alternative
            /// If mFormat is supported it's returned
            return GLESRTTManager::getSingleton().getSupportedAlternative(format);
        }

        // Supported
        return format;
    }

    bool GLESTextureManager::isHardwareFilteringSupported(TextureType ttype, PixelFormat format, int usage,
            bool preciseFormatOnly)
    {
        if (format == PF_UNKNOWN)
        {
            return false;
        }

        // Check natively format
        PixelFormat nativeFormat = getNativeFormat(ttype, format, usage);
        if (preciseFormatOnly && format != nativeFormat)
        {
            return false;
        }

        // Assume non-floating point is supported always
        if (!PixelUtil::isFloatingPoint(nativeFormat))
        {
            return true;
        }

        return false;
    }
}
