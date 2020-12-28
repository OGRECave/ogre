/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-present Torus Knot Software Ltd

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

#ifndef _OgreVulkanDescriptorSetTexture_H_
#define _OgreVulkanDescriptorSetTexture_H_

#include "OgreVulkanPrerequisites.h"

#include "OgreHeaderPrefix.h"

#include "vulkan/vulkan_core.h"

namespace Ogre
{
    struct VulkanDescriptorSetSampler
    {
        FastArray<VkDescriptorImageInfo> mSamplers;
        VkWriteDescriptorSet mWriteDescSet;
        VulkanDescriptorSetSampler( const DescriptorSetSampler &descSet, VkSampler dummySampler );
    };

    struct VulkanDescriptorSetTexture
    {
        // Note: mTextures.size() may be twice its real size
        // due to an extra copy for hazardous-free version
        FastArray<VkDescriptorImageInfo> mTextures;
        VkWriteDescriptorSet mWriteDescSet;
        VkWriteDescriptorSet mWriteDescSetHazardous;
        uint32 mLastHazardousTex;

        VulkanDescriptorSetTexture( const DescriptorSetTexture &descSet );

        void setHazardousTex( const DescriptorSetTexture &descSet, const uint32 hazardousTexIdx,
                              VulkanTextureGpuManager *textureManager );
    };

    struct VulkanDescriptorSetTexture2
    {
        FastArray<VkBufferView> mBuffers;
        FastArray<VkDescriptorBufferInfo> mReadOnlyBuffers;
        FastArray<VkDescriptorImageInfo> mTextures;
        VkWriteDescriptorSet mWriteDescSets[3];

        VulkanDescriptorSetTexture2( const DescriptorSetTexture2 &descSet );

        void destroy( VaoManager *vaoManager, VkDevice device, const DescriptorSetTexture2 &descSetUav );
    };

    struct VulkanDescriptorSetUav
    {
        FastArray<VkDescriptorBufferInfo> mBuffers;
        FastArray<VkDescriptorImageInfo> mTextures;
        VkWriteDescriptorSet mWriteDescSets[2];

        VulkanDescriptorSetUav( const DescriptorSetUav &descSetUav );

        void destroy( const DescriptorSetUav &descSetUav );
    };
}  // namespace Ogre

#include "OgreHeaderSuffix.h"

#endif  // #ifndef _OgreVulkanDescriptorSetTexture_H_
