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
#include "OgreGL3PlusStateCacheManager.h"
#include "OgreGL3PlusRenderSystem.h"
#include "OgreGLRenderTexture.h"
#include "OgreRoot.h"
#include "OgreGL3PlusPixelFormat.h"

#ifndef GL_EXT_texture_filter_anisotropic
#define GL_TEXTURE_MAX_ANISOTROPY_EXT     0x84FE
#endif

namespace Ogre {
    GLint GL3PlusSampler::getCombinedMinMipFilter(FilterOptions min, FilterOptions mip)
    {
        switch(min)
        {
        case FO_ANISOTROPIC:
        case FO_LINEAR:
            switch (mip)
            {
            case FO_ANISOTROPIC:
            case FO_LINEAR:
                // linear min, linear mip
                return GL_LINEAR_MIPMAP_LINEAR;
            case FO_POINT:
                // linear min, point mip
                return GL_LINEAR_MIPMAP_NEAREST;
            case FO_NONE:
                // linear min, no mip
                return GL_LINEAR;
            }
            break;
        case FO_POINT:
        case FO_NONE:
            switch (mip)
            {
            case FO_ANISOTROPIC:
            case FO_LINEAR:
                // nearest min, linear mip
                return GL_NEAREST_MIPMAP_LINEAR;
            case FO_POINT:
                // nearest min, point mip
                return GL_NEAREST_MIPMAP_NEAREST;
            case FO_NONE:
                // nearest min, no mip
                return GL_NEAREST;
            }
            break;
        }

        // should never get here
        return 0;
    }

    GLint GL3PlusSampler::getTextureAddressingMode(TextureAddressingMode tam)
    {
        switch (tam)
        {
        default:
        case TextureUnitState::TAM_WRAP:
            return GL_REPEAT;
        case TextureUnitState::TAM_MIRROR:
            return GL_MIRRORED_REPEAT;
        case TextureUnitState::TAM_CLAMP:
            return GL_CLAMP_TO_EDGE;
        case TextureUnitState::TAM_BORDER:
            return GL_CLAMP_TO_BORDER;
        }
    }

    GL3PlusSampler::GL3PlusSampler(GL3PlusRenderSystem* rs) : mSamplerId(0)
    {
        glGenSamplers(1, &mSamplerId);
    }
    GL3PlusSampler::~GL3PlusSampler()
    {
        glDeleteSamplers(1, &mSamplerId);
    }
    void GL3PlusSampler::bind(uint32 unit)
    {
        OGRE_CHECK_GL_ERROR(glBindSampler(unit, mSamplerId));

        if(!mDirty)
            return;

        OGRE_CHECK_GL_ERROR(glSamplerParameteri(mSamplerId, GL_TEXTURE_WRAP_S,
                                                getTextureAddressingMode(mAddressMode.u)));
        OGRE_CHECK_GL_ERROR(glSamplerParameteri(mSamplerId, GL_TEXTURE_WRAP_T,
                                                getTextureAddressingMode(mAddressMode.v)));
        OGRE_CHECK_GL_ERROR(glSamplerParameteri(mSamplerId, GL_TEXTURE_WRAP_R,
                                                getTextureAddressingMode(mAddressMode.w)));

        bool reversedZ = Root::getSingleton().getRenderSystem()->isReverseDepthBufferEnabled();

        if (mAddressMode.u == TAM_BORDER || mAddressMode.v == TAM_BORDER || mAddressMode.w == TAM_BORDER)
        {
            auto borderColour = (reversedZ && mCompareEnabled) ? ColourValue::White - mBorderColour : mBorderColour;
            OGRE_CHECK_GL_ERROR(glSamplerParameterfv( mSamplerId, GL_TEXTURE_BORDER_COLOR, borderColour.ptr()));
        }
        OGRE_CHECK_GL_ERROR(glSamplerParameterf(mSamplerId, GL_TEXTURE_LOD_BIAS, mMipmapBias));

        auto caps = Root::getSingleton().getRenderSystem()->getCapabilities();
        if (caps->hasCapability(RSC_ANISOTROPY))
            OGRE_CHECK_GL_ERROR(
                glSamplerParameteri(mSamplerId, GL_TEXTURE_MAX_ANISOTROPY_EXT,
                                    std::min<uint>(caps->getMaxSupportedAnisotropy(), mMaxAniso)));

        OGRE_CHECK_GL_ERROR(
            glSamplerParameteri(mSamplerId, GL_TEXTURE_COMPARE_MODE,
                                mCompareEnabled ? GL_COMPARE_REF_TO_TEXTURE : GL_NONE));

        auto cmpFunc = mCompareFunc;
        if(reversedZ)
            cmpFunc = GL3PlusRenderSystem::reverseCompareFunction(cmpFunc);

        OGRE_CHECK_GL_ERROR(
            glSamplerParameteri(mSamplerId, GL_TEXTURE_COMPARE_FUNC,
                                GL3PlusRenderSystem::convertCompareFunction(cmpFunc)));

        // Combine with existing mip filter
        OGRE_CHECK_GL_ERROR(glSamplerParameteri(mSamplerId, GL_TEXTURE_MIN_FILTER,
                                                getCombinedMinMipFilter(mMinFilter, mMipFilter)));

        switch (mMagFilter)
        {
        case FO_ANISOTROPIC: // GL treats linear and aniso the same
        case FO_LINEAR:
            OGRE_CHECK_GL_ERROR(glSamplerParameteri(mSamplerId, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
            break;
        case FO_POINT:
        case FO_NONE:
            OGRE_CHECK_GL_ERROR(glSamplerParameteri(mSamplerId, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
            break;
        }

        mDirty = false;
    }

    GL3PlusTextureManager::GL3PlusTextureManager(GL3PlusRenderSystem* renderSystem)
        : TextureManager(), mRenderSystem(renderSystem)
    {
        // Register with group manager
        ResourceGroupManager::getSingleton()._registerResourceManager(mResourceType, this);
    }

    GL3PlusTextureManager::~GL3PlusTextureManager()
    {
        // Unregister with group manager
        ResourceGroupManager::getSingleton()._unregisterResourceManager(mResourceType);
    }

    Resource* GL3PlusTextureManager::createImpl(const String& name, ResourceHandle handle,
                                                const String& group, bool isManual,
                                                ManualResourceLoader* loader,
                                                const NameValuePairList* createParams)
    {
        return new GL3PlusTexture(this, name, handle, group, isManual, loader, mRenderSystem);
    }

    SamplerPtr GL3PlusTextureManager::_createSamplerImpl()
    {
        return std::make_shared<GL3PlusSampler>(mRenderSystem);
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


    PixelFormat GL3PlusTextureManager::getNativeFormat(TextureType ttype, PixelFormat format, int usage)
    {
        // Adjust requested parameters to capabilities
        const RenderSystemCapabilities *caps = Root::getSingleton().getRenderSystem()->getCapabilities();

        // Check compressed texture support
        // if a compressed format not supported, revert to PF_A8R8G8B8
        if(PixelUtil::isCompressed(format) &&
           !caps->hasCapability( RSC_TEXTURE_COMPRESSION_DXT ))
        {
            return PF_BYTE_RGBA;
        }
        // if floating point textures not supported, revert to PF_A8R8G8B8
        if (PixelUtil::isFloatingPoint(format) &&
            !caps->hasCapability(RSC_TEXTURE_FLOAT))
        {
            return PF_BYTE_RGBA;
        }

        if(GL3PlusPixelUtil::getGLInternalFormat(format) == GL_NONE)
        {
            return PF_BYTE_RGBA;
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
