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

#ifndef _OgreVulkanTextureGpuManager_H_
#define _OgreVulkanTextureGpuManager_H_

#include "OgreVulkanPrerequisites.h"

#include "OgreTextureGpuManager.h"

#include "OgreVulkanDevice.h"
#include "OgreVulkanRenderSystem.h"

#include "OgreDescriptorSetTexture.h"
#include "OgreDescriptorSetUav.h"
#include "OgreTextureGpu.h"

#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Core
     *  @{
     */
    /** \addtogroup Resources
     *  @{
     */
    class _OgreVulkanExport VulkanTextureGpuManager : public TextureGpuManager
    {
    protected:
        struct BlankTexture
        {
            VkImage vkImage;
            VkImageView defaultView;
            size_t vboPoolIdx;
            size_t internalBufferStart;
        };

        struct CachedView
        {
            uint32 refCount;
            VkImageView imageView;
        };

        typedef map<DescriptorSetTexture2::TextureSlot, CachedView>::type CachedTex2ImageViewMap;
        typedef map<DescriptorSetUav::TextureSlot, CachedView>::type CachedUavImageViewMap;

        CachedTex2ImageViewMap mCachedTex;
        CachedUavImageViewMap mCachedUavs;

        /// 4x4 texture for when we have nothing to display.
        BlankTexture mBlankTexture[TextureTypes::Type3D + 1u];

        VulkanDevice *mDevice;

        bool mCanRestrictImageViewUsage;

        virtual TextureGpu *createTextureImpl( GpuPageOutStrategy::GpuPageOutStrategy pageOutStrategy,
                                               IdString name, uint32 textureFlags,
                                               TextureTypes::TextureTypes initialType );
        virtual StagingTexture *createStagingTextureImpl( uint32 width, uint32 height, uint32 depth,
                                                          uint32 slices, PixelFormatGpu pixelFormat );
        virtual void destroyStagingTextureImpl( StagingTexture *stagingTexture );

        virtual AsyncTextureTicket *createAsyncTextureTicketImpl( uint32 width, uint32 height,
                                                                  uint32 depthOrSlices,
                                                                  TextureTypes::TextureTypes textureType,
                                                                  PixelFormatGpu pixelFormatFamily );

    public:
        VulkanTextureGpuManager( VulkanVaoManager *vaoManager, RenderSystem *renderSystem,
                                 VulkanDevice *device, bool bCanRestrictImageViewUsage );
        virtual ~VulkanTextureGpuManager();

        TextureGpu *createTextureGpuWindow( VulkanWindow *window );
        TextureGpu *createWindowDepthBuffer( void );

        VkImage getBlankTextureVulkanName( TextureTypes::TextureTypes textureType ) const;
        VkImageView getBlankTextureView( TextureTypes::TextureTypes textureType ) const;

        VkImageView createView( const DescriptorSetTexture2::TextureSlot &texSlot );
        void destroyView( DescriptorSetTexture2::TextureSlot texSlot, VkImageView imageView );

        VkImageView createView( const DescriptorSetUav::TextureSlot &texSlot );
        void destroyView( DescriptorSetUav::TextureSlot texSlot, VkImageView imageView );

        VulkanDevice *getDevice() const { return mDevice; }

        bool canRestrictImageViewUsage( void ) const { return mCanRestrictImageViewUsage; }

        virtual bool checkSupport( PixelFormatGpu format, uint32 textureFlags ) const;
    };

    /** @} */
    /** @} */
}  // namespace Ogre

#include "OgreHeaderSuffix.h"

#endif
