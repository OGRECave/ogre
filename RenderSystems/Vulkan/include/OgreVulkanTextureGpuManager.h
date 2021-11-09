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

#include "OgreTextureManager.h"

#include "OgreVulkanDevice.h"
#include "OgreVulkanRenderSystem.h"

#include "OgreVulkanTextureGpu.h"

#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Core
     *  @{
     */
    /** \addtogroup Resources
     *  @{
     */

    struct StagingTexture;
    struct AsyncTextureTicket;

    class VulkanSampler : public Sampler
    {
    public:
        VulkanSampler(VkDevice device);
        ~VulkanSampler();
        VkSampler bind();
    private:
        VkDevice mDevice;
        VkSampler mVkSampler;
    };

    class VulkanTextureGpuManager : public TextureManager
    {
    protected:
        VulkanDevice *mDevice;

        bool mCanRestrictImageViewUsage;

        SamplerPtr _createSamplerImpl() override;

        Resource* createImpl(const String& name, ResourceHandle handle, const String& group, bool isManual,
                             ManualResourceLoader* loader, const NameValuePairList* createParams);

        PixelFormat getNativeFormat(TextureType ttype, PixelFormat format, int usage);
    public:
        VulkanTextureGpuManager(RenderSystem* renderSystem, VulkanDevice* device, bool bCanRestrictImageViewUsage);
        virtual ~VulkanTextureGpuManager();

        VulkanDevice *getDevice() const { return mDevice; }

        bool canRestrictImageViewUsage( void ) const { return mCanRestrictImageViewUsage; }

        virtual bool checkSupport( PixelFormatGpu format, uint32 textureFlags ) const;
    };

    /** @} */
    /** @} */
}  // namespace Ogre

#include "OgreHeaderSuffix.h"

#endif
