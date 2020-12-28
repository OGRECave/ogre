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

#include "OgreVulkanDescriptorSets.h"

#include "OgreVulkanDelayedFuncs.h"
#include "OgreVulkanTextureGpu.h"
#include "OgreVulkanTextureGpuManager.h"
#include "OgreVulkanUtils.h"
#include "Vao/OgreVulkanReadOnlyBufferPacked.h"
#include "Vao/OgreVulkanTexBufferPacked.h"
#include "Vao/OgreVulkanUavBufferPacked.h"

#include "OgreDescriptorSetSampler.h"
#include "OgreDescriptorSetUav.h"
#include "OgreHlmsSamplerblock.h"

namespace Ogre
{
    VulkanDescriptorSetSampler::VulkanDescriptorSetSampler( const DescriptorSetSampler &descSet,
                                                            VkSampler dummySampler )
    {
        mSamplers.reserve( descSet.mSamplers.size() );

        FastArray<const HlmsSamplerblock *>::const_iterator itor = descSet.mSamplers.begin();
        FastArray<const HlmsSamplerblock *>::const_iterator endt = descSet.mSamplers.end();

        while( itor != endt )
        {
            const HlmsSamplerblock *samplerblock = *itor;
            VkSampler sampler = dummySampler;
            if( samplerblock )
                sampler = reinterpret_cast<VkSampler>( samplerblock->mRsData );
            VkDescriptorImageInfo imageInfo;
            imageInfo.sampler = sampler;
            imageInfo.imageView = 0;
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            mSamplers.push_back( imageInfo );
            ++itor;
        }

        makeVkStruct( mWriteDescSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET );
        mWriteDescSet.descriptorCount = static_cast<uint32>( mSamplers.size() );
        mWriteDescSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        mWriteDescSet.pImageInfo = mSamplers.begin();
    }
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    VulkanDescriptorSetTexture::VulkanDescriptorSetTexture( const DescriptorSetTexture &descSet ) :
        mLastHazardousTex( std::numeric_limits<uint32>::max() )
    {
        if( descSet.mTextures.empty() )
        {
            memset( &mWriteDescSet, 0, sizeof( mWriteDescSet ) );
            return;
        }

        FastArray<const TextureGpu *>::const_iterator itor = descSet.mTextures.begin();
        FastArray<const TextureGpu *>::const_iterator endt = descSet.mTextures.end();

        mTextures.reserve( descSet.mTextures.size() );

        while( itor != endt )
        {
            OGRE_ASSERT_HIGH( dynamic_cast<const VulkanTextureGpu *>( *itor ) );
            const VulkanTextureGpu *vulkanTexture = static_cast<const VulkanTextureGpu *>( *itor );

            VkDescriptorImageInfo imageInfo;
            imageInfo.sampler = 0;
            imageInfo.imageView = vulkanTexture->getDefaultDisplaySrv();
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            mTextures.push_back( imageInfo );

            ++itor;
        }

        makeVkStruct( mWriteDescSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET );

        mWriteDescSet.descriptorCount = static_cast<uint32>( mTextures.size() );
        mWriteDescSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        mWriteDescSet.pImageInfo = mTextures.begin();

        mWriteDescSetHazardous = mWriteDescSet;
    }
    //-------------------------------------------------------------------------
    void VulkanDescriptorSetTexture::setHazardousTex( const DescriptorSetTexture &descSet,
                                                      const uint32 hazardousTexIdx,
                                                      VulkanTextureGpuManager *textureManager )
    {
        if( mLastHazardousTex != hazardousTexIdx )
        {
            const size_t realNumTextures = descSet.mTextures.size();
            mTextures.resize( realNumTextures );
            mTextures.appendPOD( mTextures.begin(), mTextures.end() );
            mWriteDescSetHazardous.pImageInfo = mTextures.begin() + realNumTextures;
            mTextures[realNumTextures + hazardousTexIdx].imageView = textureManager->getBlankTextureView(
                descSet.mTextures[hazardousTexIdx]->getInternalTextureType() );
            mLastHazardousTex = hazardousTexIdx;
        }
    }
    //-------------------------------------------------------------------------
    VulkanDescriptorSetTexture2::VulkanDescriptorSetTexture2( const DescriptorSetTexture2 &descSet )
    {
        if( descSet.mTextures.empty() )
        {
            memset( mWriteDescSets, 0, sizeof( mWriteDescSets ) );
            return;
        }

        size_t numTextures = 0u;
        size_t numTexBuffers = 0u;
        size_t numROBuffers = 0u;

        FastArray<DescriptorSetTexture2::Slot>::const_iterator itor = descSet.mTextures.begin();
        FastArray<DescriptorSetTexture2::Slot>::const_iterator endt = descSet.mTextures.end();

        while( itor != endt )
        {
            if( itor->isBuffer() )
            {
                if( itor->getBuffer().buffer->getBufferPackedType() == BP_TYPE_TEX )
                    ++numTexBuffers;
                else
                    ++numROBuffers;
            }
            else
                ++numTextures;
            ++itor;
        }

        mTextures.reserve( numTextures );
        mBuffers.resizePOD( numTexBuffers );
        mReadOnlyBuffers.resizePOD( numROBuffers );
        numTexBuffers = 0u;
        numROBuffers = 0u;

        itor = descSet.mTextures.begin();

        while( itor != endt )
        {
            if( itor->isBuffer() )
            {
                const DescriptorSetTexture2::BufferSlot &bufferSlot = itor->getBuffer();

                if( itor->getBuffer().buffer->getBufferPackedType() == BP_TYPE_TEX )
                {
                    OGRE_ASSERT_HIGH( dynamic_cast<VulkanTexBufferPacked *>( bufferSlot.buffer ) );
                    VulkanTexBufferPacked *vulkanBuffer =
                        static_cast<VulkanTexBufferPacked *>( bufferSlot.buffer );

                    // No caching of VkBufferView. Unlike VkImageViews, setting lots of TexBuffers
                    // via DescriptorSetTextureN is not common enough to warrant it.
                    mBuffers[numTexBuffers] =
                        vulkanBuffer->createBufferView( bufferSlot.offset, bufferSlot.sizeBytes );
                    ++numTexBuffers;
                }
                else
                {
                    OGRE_ASSERT_HIGH( dynamic_cast<VulkanReadOnlyBufferPacked *>( bufferSlot.buffer ) );
                    VulkanReadOnlyBufferPacked *vulkanBuffer =
                        static_cast<VulkanReadOnlyBufferPacked *>( bufferSlot.buffer );

                    vulkanBuffer->setupBufferInfo( mReadOnlyBuffers[numROBuffers], bufferSlot.offset,
                                                   bufferSlot.sizeBytes );
                    ++numROBuffers;
                }
            }
            else
            {
                const DescriptorSetTexture2::TextureSlot &texSlot = itor->getTexture();
                OGRE_ASSERT_HIGH( dynamic_cast<VulkanTextureGpu *>( texSlot.texture ) );
                VulkanTextureGpu *vulkanTexture = static_cast<VulkanTextureGpu *>( texSlot.texture );

                VkDescriptorImageInfo imageInfo;
                imageInfo.sampler = 0;
                imageInfo.imageView = vulkanTexture->createView( texSlot );
                imageInfo.imageLayout = texSlot.generalReadWrite
                                            ? VK_IMAGE_LAYOUT_GENERAL
                                            : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                mTextures.push_back( imageInfo );
            }

            ++itor;
        }

        if( numROBuffers != 0u )
        {
            VkWriteDescriptorSet *writeDescSet = &mWriteDescSets[0];
            makeVkStruct( *writeDescSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET );

            writeDescSet->descriptorCount = static_cast<uint32>( numROBuffers );
            writeDescSet->descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            writeDescSet->pBufferInfo = mReadOnlyBuffers.begin();
        }
        else
        {
            memset( &mWriteDescSets[0], 0, sizeof( mWriteDescSets[0] ) );
        }

        if( numTexBuffers != 0u )
        {
            VkWriteDescriptorSet *writeDescSet = &mWriteDescSets[1];
            makeVkStruct( *writeDescSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET );

            writeDescSet->descriptorCount = static_cast<uint32>( numTexBuffers );
            writeDescSet->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
            writeDescSet->pTexelBufferView = mBuffers.begin();
        }
        else
        {
            memset( &mWriteDescSets[1], 0, sizeof( mWriteDescSets[1] ) );
        }

        if( numTextures != 0u )
        {
            VkWriteDescriptorSet *writeDescSet = &mWriteDescSets[2];
            makeVkStruct( *writeDescSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET );

            writeDescSet->descriptorCount = static_cast<uint32>( numTextures );
            writeDescSet->descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            writeDescSet->pImageInfo = mTextures.begin();
        }
        else
        {
            memset( &mWriteDescSets[2], 0, sizeof( mWriteDescSets[2] ) );
        }
    }
    //-------------------------------------------------------------------------
    void VulkanDescriptorSetTexture2::destroy( VaoManager *vaoManager, VkDevice device,
                                               const DescriptorSetTexture2 &descSet )
    {
        {
            FastArray<VkBufferView>::const_iterator itor = mBuffers.begin();
            FastArray<VkBufferView>::const_iterator endt = mBuffers.end();

            while( itor != endt )
            {
                delayed_vkDestroyBufferView( vaoManager, device, *itor, 0 );
                ++itor;
            }
        }
        FastArray<VkDescriptorImageInfo>::const_iterator imgInfoIt = mTextures.begin();

        FastArray<DescriptorSetTexture2::Slot>::const_iterator itor = descSet.mTextures.begin();
        FastArray<DescriptorSetTexture2::Slot>::const_iterator endt = descSet.mTextures.end();

        while( itor != endt )
        {
            if( itor->isTexture() )
            {
                const DescriptorSetTexture2::TextureSlot &texSlot = itor->getTexture();
                OGRE_ASSERT_HIGH( dynamic_cast<VulkanTextureGpu *>( texSlot.texture ) );
                VulkanTextureGpu *vulkanTexture = static_cast<VulkanTextureGpu *>( texSlot.texture );
                vulkanTexture->destroyView( texSlot, imgInfoIt->imageView );
                ++imgInfoIt;
            }
            ++itor;
        }
    }
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    VulkanDescriptorSetUav::VulkanDescriptorSetUav( const DescriptorSetUav &descSetUav )
    {
        if( descSetUav.mUavs.empty() )
        {
            memset( mWriteDescSets, 0, sizeof( mWriteDescSets ) );
            return;
        }

        size_t numTextures = 0u;
        size_t numBuffers = 0u;

        FastArray<DescriptorSetUav::Slot>::const_iterator itor = descSetUav.mUavs.begin();
        FastArray<DescriptorSetUav::Slot>::const_iterator endt = descSetUav.mUavs.end();

        while( itor != endt )
        {
            if( itor->isBuffer() )
                ++numBuffers;
            else
                ++numTextures;
            ++itor;
        }

        mTextures.reserve( numTextures );
        mBuffers.resize( numBuffers );
        numBuffers = 0u;

        itor = descSetUav.mUavs.begin();

        while( itor != endt )
        {
            if( itor->isBuffer() )
            {
                const DescriptorSetUav::BufferSlot &bufferSlot = itor->getBuffer();
                OGRE_ASSERT_HIGH( dynamic_cast<VulkanUavBufferPacked *>( bufferSlot.buffer ) );
                VulkanUavBufferPacked *vulkanBuffer =
                    static_cast<VulkanUavBufferPacked *>( bufferSlot.buffer );

                vulkanBuffer->setupBufferInfo( mBuffers[numBuffers], bufferSlot.offset,
                                               bufferSlot.sizeBytes );
                ++numBuffers;
            }
            else
            {
                const DescriptorSetUav::TextureSlot &texSlot = itor->getTexture();
                OGRE_ASSERT_HIGH( dynamic_cast<VulkanTextureGpu *>( texSlot.texture ) );
                VulkanTextureGpu *vulkanTexture = static_cast<VulkanTextureGpu *>( texSlot.texture );

                VkDescriptorImageInfo imageInfo;
                imageInfo.sampler = 0;
                imageInfo.imageView = vulkanTexture->createView( texSlot );
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                mTextures.push_back( imageInfo );
            }

            ++itor;
        }

        if( numBuffers != 0u )
        {
            VkWriteDescriptorSet *writeDescSet = &mWriteDescSets[0];
            makeVkStruct( *writeDescSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET );

            writeDescSet->descriptorCount = static_cast<uint32>( numBuffers );
            writeDescSet->descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            writeDescSet->pBufferInfo = mBuffers.begin();
        }
        else
        {
            memset( &mWriteDescSets[0], 0, sizeof( mWriteDescSets[0] ) );
        }

        if( numTextures != 0u )
        {
            VkWriteDescriptorSet *writeDescSet = &mWriteDescSets[1];
            makeVkStruct( *writeDescSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET );

            writeDescSet->descriptorCount = static_cast<uint32>( numTextures );
            writeDescSet->descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            writeDescSet->pImageInfo = mTextures.begin();
        }
        else
        {
            memset( &mWriteDescSets[1], 0, sizeof( mWriteDescSets[1] ) );
        }
    }
    //-------------------------------------------------------------------------
    void VulkanDescriptorSetUav::destroy( const DescriptorSetUav &descSetUav )
    {
        FastArray<VkDescriptorImageInfo>::const_iterator imgInfoIt = mTextures.begin();

        FastArray<DescriptorSetUav::Slot>::const_iterator itor = descSetUav.mUavs.begin();
        FastArray<DescriptorSetUav::Slot>::const_iterator endt = descSetUav.mUavs.end();

        while( itor != endt )
        {
            if( itor->isTexture() )
            {
                const DescriptorSetUav::TextureSlot &texSlot = itor->getTexture();
                OGRE_ASSERT_HIGH( dynamic_cast<VulkanTextureGpu *>( texSlot.texture ) );
                VulkanTextureGpu *vulkanTexture = static_cast<VulkanTextureGpu *>( texSlot.texture );
                vulkanTexture->destroyView( texSlot, imgInfoIt->imageView );
                ++imgInfoIt;
            }
            ++itor;
        }
    }
}  // namespace Ogre
