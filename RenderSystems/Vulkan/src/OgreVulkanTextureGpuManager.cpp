/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2017 Torus Knot Software Ltd

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

#include "OgreVulkanTextureGpuManager.h"

#include "OgreVulkanMappings.h"
#include "OgreVulkanTextureGpu.h"
#include "OgreVulkanTextureGpuWindow.h"
#include "OgreVulkanUtils.h"

#include "OgrePixelFormat.h"
#include "OgreVector.h"
#include "OgreRoot.h"

#include "OgreException.h"

namespace Ogre
{
    VulkanSampler::VulkanSampler(VkDevice device) : mDevice(device), mVkSampler(VK_NULL_HANDLE) {}
    VulkanSampler::~VulkanSampler() { vkDestroySampler(mDevice, mVkSampler, nullptr); }
    VkSampler VulkanSampler::bind()
    {
        if(!mDirty)
            return mVkSampler;

        vkDestroySampler(mDevice, mVkSampler, nullptr);

        VkSamplerCreateInfo samplerCi = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
        samplerCi.minFilter = VulkanMappings::get(mMinFilter);
        samplerCi.magFilter = VulkanMappings::get(mMagFilter);
        samplerCi.mipmapMode = VulkanMappings::getMipFilter(mMipFilter);
        samplerCi.mipLodBias = mMipmapBias;

        auto caps = Root::getSingleton().getRenderSystem()->getCapabilities();
        if (caps->hasCapability(RSC_ANISOTROPY))
        {
            samplerCi.anisotropyEnable = VK_TRUE;
            samplerCi.maxAnisotropy = std::min<uint>(caps->getMaxSupportedAnisotropy(), mMaxAniso);
        }

        samplerCi.addressModeU = VulkanMappings::get(mAddressMode.u);
        samplerCi.addressModeV = VulkanMappings::get(mAddressMode.v);
        samplerCi.addressModeW = VulkanMappings::get(mAddressMode.w);
        samplerCi.unnormalizedCoordinates = VK_FALSE;
        samplerCi.maxLod = mMipFilter == FO_NONE ? 0 : VK_LOD_CLAMP_NONE;

        bool reversedZ = Root::getSingleton().getRenderSystem()->isReverseDepthBufferEnabled();

        auto borderBlack = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        auto borderWhite = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

        if(reversedZ)
            std::swap(borderBlack, borderWhite);

        samplerCi.borderColor = mBorderColour.getAsRGBA() == 0x0000FF ? borderBlack : borderWhite;

        if (mCompareEnabled)
        {
            auto cmpFunc = mCompareFunc;
            if(reversedZ)
                cmpFunc = VulkanRenderSystem::reverseCompareFunction(cmpFunc);
            samplerCi.compareEnable = VK_TRUE;
            samplerCi.compareOp = VulkanMappings::get(cmpFunc);
        }

        OGRE_VK_CHECK(vkCreateSampler(mDevice, &samplerCi, 0, &mVkSampler));
        mDirty = false;
        return mVkSampler;
    }

    VulkanTextureGpuManager::VulkanTextureGpuManager(RenderSystem *renderSystem, VulkanDevice *device,
                                                      bool bCanRestrictImageViewUsage ) :
        mDevice( device ),
        mCanRestrictImageViewUsage( bCanRestrictImageViewUsage )
    {
    }
    //-----------------------------------------------------------------------------------
    VulkanTextureGpuManager::~VulkanTextureGpuManager()
    {
        removeAll();
    }
    SamplerPtr VulkanTextureGpuManager::_createSamplerImpl()
    {
        return std::make_shared<VulkanSampler>(mDevice->mDevice);
    }
    //-----------------------------------------------------------------------------------
    Resource* VulkanTextureGpuManager::createImpl(const String& name, ResourceHandle handle,
                                                  const String& group, bool isManual,
                                                  ManualResourceLoader* loader,
                                                  const NameValuePairList* createParams)
    {
        return OGRE_NEW VulkanTextureGpu(this, name, handle, group, isManual, loader);
    }
    PixelFormat VulkanTextureGpuManager::getNativeFormat(TextureType ttype, PixelFormat format, int usage)
    {
        if( format == PF_R8G8B8 )
            return PF_X8R8G8B8;
        if( format == PF_B8G8R8 )
            return PF_X8B8G8R8;

#ifdef OGRE_VK_WORKAROUND_ADRENO_D32_FLOAT
        if( Workarounds::mAdrenoD32FloatBug && isRenderToTexture() )
        {
            if( pixelFormat == PF_DEPTH32F )
                return PFG_D24_UNORM;
            else if( pixelFormat == PFG_D32_FLOAT_S8X24_UINT )
                return PFG_D24_UNORM_S8_UINT;
        }
#endif

        if (VulkanMappings::get(format))
            return format;

        return PF_BYTE_RGBA;
    }
    //-----------------------------------------------------------------------------------
    bool VulkanTextureGpuManager::checkSupport( PixelFormatGpu format, uint32 textureFlags ) const
    {
        OGRE_ASSERT_LOW(
            textureFlags != TU_NOT_SRV &&
            "Invalid textureFlags combination. Asking to check if format is supported to do nothing" );

        const VkFormat vkFormat = VulkanMappings::get( format );

        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties( mDevice->mPhysicalDevice, vkFormat, &props );

        uint32 features = 0;

        if( !( textureFlags & TU_NOT_SRV ) )
            features |= VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;

        if( textureFlags & TU_UAV )
            features |= VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT;

        if( textureFlags & TU_RENDERTARGET )
        {
            if( PixelUtil::isDepth( format ) )
            {
                features |= VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
            }
            else
            {
                features |= VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT |
                            VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT;
            }
        }

        if( textureFlags & TU_AUTOMIPMAP )
        {
            features |= VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT;
            //if( !PixelUtil::supportsHwMipmaps( format ) )
                return false;
        }

        if( ( props.optimalTilingFeatures & features ) == features )
            return true;

#ifdef OGRE_VK_WORKAROUND_ADRENO_5XX_6XX_MINCAPS
        if( Workarounds::mAdreno5xx6xxMinCaps &&
            ( textureFlags & ( TextureFlags::Uav | TextureFlags::AllowAutomipmaps ) ) == 0u )
        {
            switch( format )
            {
            case PFG_R16_UNORM:
            case PFG_R16_SNORM:
            case PFG_RG16_UNORM:
            case PFG_RG16_SNORM:
            case PFG_RGBA16_UNORM:
            case PFG_RGBA16_SNORM:
                return true;
            default:
                break;
            }
        }
#endif

        return false;
    }
}  // namespace Ogre
