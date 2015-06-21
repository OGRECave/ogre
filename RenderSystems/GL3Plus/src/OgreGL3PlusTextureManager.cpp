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

#include "OgreGL3PlusTextureManager.h"
#include "OgreGL3PlusRenderTexture.h"
#include "OgreGL3PlusDepthTexture.h"
#include "OgreGL3PlusNullTexture.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"

namespace Ogre {
    GL3PlusTextureManager::GL3PlusTextureManager(GL3PlusSupport& support)
        : TextureManager(), mGLSupport(support), mWarningTextureID(0)//, mImages()
    {
        // Register with group manager
        ResourceGroupManager::getSingleton()._registerResourceManager(mResourceType, this);

        createWarningTexture();
    }

    GL3PlusTextureManager::~GL3PlusTextureManager()
    {
        // Unregister with group manager
        ResourceGroupManager::getSingleton()._unregisterResourceManager(mResourceType);

        // Delete warning texture
        OGRE_CHECK_GL_ERROR(glDeleteTextures(1, &mWarningTextureID));
    }

    Resource* GL3PlusTextureManager::createImpl(const String& name, ResourceHandle handle,
                                                const String& group, bool isManual,
                                                ManualResourceLoader* loader,
                                                const NameValuePairList* createParams)
    {
        if( createParams )
        {
            if( createParams->find( "DepthTexture" ) != createParams->end() )
            {
                const bool shareableDepthBuffer = createParams->find( "shareableDepthBuffer" ) !=
                                                                                createParams->end();
                return new GL3PlusDepthTexture( shareableDepthBuffer, this, name, handle, group,
                                                isManual, loader, mGLSupport );
            }

            NameValuePairList::const_iterator it = createParams->find( "SpecialFormat" );
            if( it != createParams->end() && it->second == "PF_NULL" )
            {
                return new GL3PlusNullTexture( this, name, handle, group,
                                               isManual, loader, mGLSupport );
            }
        }

        return new GL3PlusTexture(this, name, handle, group, isManual, loader, mGLSupport);
    }


    // TexturePtr GL3PlusTextureManager::createManual(const String & name, const String& group,
    //                                                TextureType texType, uint width, uint height, uint depth, int numMipmaps,
    //                                                PixelFormat format, int usage, ManualResourceLoader* loader, bool hwGamma, 
    //                                                uint fsaa, const String& fsaaHint)
    // {
    //     TexturePtr texture = TextureManager::createManual(
    //         name, group, texType, 
    //         width, height, depth, numMipmaps,
    //         format, usage, loader, hwGamma, 
    //         fsaa, fsaaHint);

    //     if (texture->getUsage() == TU_DYNAMIC_SHADER)
    //     {
    //         registerImage(texture);
    //     }

    //     return texture;
    // }

    
    void GL3PlusTextureManager::createWarningTexture()
    {
        // Generate warning texture
        uint32 width = 8;
        uint32 height = 8;

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
        OGRE_CHECK_GL_ERROR(glGenTextures(1, &mWarningTextureID));
        OGRE_CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, mWarningTextureID));
        OGRE_CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0));
        OGRE_CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0));
        OGRE_CHECK_GL_ERROR(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, (void*)data));

        // Free memory
        delete [] data;
    }

    PixelFormat GL3PlusTextureManager::getNativeFormat(TextureType ttype, PixelFormat format, int usage)
    {
        // Adjust requested parameters to capabilities
        const RenderSystemCapabilities *caps = Root::getSingleton().getRenderSystem()->getCapabilities();

        // Check compressed texture support
        // if a compressed format not supported, revert to PF_A8R8G8B8
        if(PixelUtil::isCompressed(format) &&
           !caps->hasCapability( RSC_TEXTURE_COMPRESSION_DXT ))
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
            return GL3PlusRTTManager::getSingleton().getSupportedAlternative(format);
        }

        // Supported
        return format;
    }

    bool GL3PlusTextureManager::isHardwareFilteringSupported(TextureType ttype, PixelFormat format, int usage,
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

    // void GL3PlusTextureManager::registerImage(TexturePtr texture)
    // {
    //     mImages.push_back(texture);
    // }

    //FIXME Should this become a standard Texture class feature?
    // void GL3PlusTextureManager::bindImages()
    // {
    //     //FIXME currently produces a GL_INVALID_OPERATION, so temporarily run once
    //     // static bool images_bound = false;

    //     TexturePtrList::iterator texture = mImages.begin();
    //     TexturePtrList::iterator end = mImages.end();

    //     // if (!images_bound && !mImages.empty()) {
    //         for (; texture != end; texture++)
    //         {
    //             //std::cout << "IMAGE LOAD/STORE" << std::endl;
    //             GL3PlusTexturePtr tex = texture->staticCast<GL3PlusTexture>();
    //             //TODO This needs to be redone so that:
    //             // * binding point (first parameter) and possibly other parameters come from shader
    //             // * simple conversion of shader format to GLenum format
    //             // * material scripts can create images
    //             //OGRE_CHECK_GL_ERROR(glBindImageTexture(0, tex->getGLID(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8));
    //         }
    //         // images_bound = true;
    //     // }
    // }
}
