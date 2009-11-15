/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
        return OGRE_NEW GLESTexture(this, name, handle, group, isManual, loader, mGLSupport);
    }

    //-----------------------------------------------------------------------------
    void GLESTextureManager::createWarningTexture()
    {
        // Generate warning texture
        size_t width = 8;
        size_t height = 8;

        // TODO convert to 5_6_5
        uint32* data = OGRE_NEW_FIX_FOR_WIN32 uint32[width * height]; // 0xXXRRGGBB

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
#if GL_OES_rgb8_rgba8
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8_OES, width, height, 0, GL_BGRA_EXT,
                     GL_UNSIGNED_BYTE, (void*)data);
#else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGRA_EXT,
                     GL_UNSIGNED_BYTE, (void*)data);
#endif
        GL_CHECK_ERROR;
        // Free memory
        OGRE_DELETE [] data;
    }

    PixelFormat GLESTextureManager::getNativeFormat(TextureType ttype, PixelFormat format, int usage)
    {
        // Adjust requested parameters to capabilities
        const RenderSystemCapabilities *caps = Root::getSingleton().getRenderSystem()->getCapabilities();

        // Check compressed texture support
        // if a compressed format not supported, revert to PF_A8R8G8B8
        if (PixelUtil::isCompressed(format) &&
            !caps->hasCapability(RSC_TEXTURE_COMPRESSION_DXT) && !caps->hasCapability(RSC_TEXTURE_COMPRESSION_PVRTC))
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

        // Check native format
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
