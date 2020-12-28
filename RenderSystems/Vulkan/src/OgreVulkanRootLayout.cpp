/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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

#include "OgreVulkanRootLayout.h"

#include "OgreVulkanDescriptorPool.h"
#include "OgreVulkanDevice.h"
#include "OgreVulkanGlobalBindingTable.h"
#include "OgreVulkanGpuProgramManager.h"
#include "OgreVulkanUtils.h"
#include "Vao/OgreVulkanVaoManager.h"

#include "OgreException.h"
#include "OgreLwString.h"
#include "OgreStringConverter.h"

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

#include "vulkan/vulkan_core.h"

#define TODO_limit_NUM_BIND_TEXTURES  // and co.

namespace Ogre
{
    static const char c_bufferTypes[] = "PBRTtsUu";
    static const char c_HLSLBufferTypesMap[] = "ccuttsuu";
    //-------------------------------------------------------------------------
    uint32 toVkDescriptorType( DescBindingTypes::DescBindingTypes type )
    {
        switch( type )
        {
            // clang-format off
        case DescBindingTypes::ParamBuffer:       return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case DescBindingTypes::ConstBuffer:       return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case DescBindingTypes::ReadOnlyBuffer:    return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case DescBindingTypes::TexBuffer:         return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
        case DescBindingTypes::Texture:           return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case DescBindingTypes::Sampler:           return VK_DESCRIPTOR_TYPE_SAMPLER;
        case DescBindingTypes::UavTexture:        return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        case DescBindingTypes::UavBuffer:         return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case DescBindingTypes::NumDescBindingTypes:   return VK_DESCRIPTOR_TYPE_MAX_ENUM;
            // clang-format on
        }
        return VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }
    //-------------------------------------------------------------------------
    VulkanRootLayout::VulkanRootLayout( VulkanGpuProgramManager *programManager ) :
        mRootLayout( 0 ),
        mProgramManager( programManager )
    {
    }
    //-------------------------------------------------------------------------
    VulkanRootLayout::~VulkanRootLayout()
    {
        if( mRootLayout )
        {
            // This should not need to be delayed, since VulkanRootLayouts persists until shutdown
            vkDestroyPipelineLayout( mProgramManager->getDevice()->mDevice, mRootLayout, 0 );
            mRootLayout = 0;
        }
    }
    //-------------------------------------------------------------------------
    void VulkanRootLayout::copyFrom( const RootLayout &rootLayout )
    {
        OGRE_ASSERT_LOW( !mRootLayout && "Cannot call parseRootLayout after createVulkanHandles" );
        RootLayout::copyFrom( rootLayout );
    }
    //-------------------------------------------------------------------------
    void VulkanRootLayout::parseRootLayout( const char *rootLayout, const bool bCompute,
                                            const String &filename )
    {
        OGRE_ASSERT_LOW( !mRootLayout && "Cannot call parseRootLayout after createVulkanHandles" );
        RootLayout::parseRootLayout( rootLayout, bCompute, filename );
    }
    //-------------------------------------------------------------------------
    void VulkanRootLayout::generateRootLayoutMacros( uint32 shaderStage, ShaderSyntax shaderType,
                                                     String &inOutString ) const
    {
        String macroStr;
        macroStr.swap( inOutString );

        char tmpBuffer[256];
        LwString textStr( LwString::FromEmptyPointer( tmpBuffer, sizeof( tmpBuffer ) ) );

        textStr.a( "#define ogre_" );
        const size_t prefixSize0 = textStr.size();

        for( size_t i = 0u; i < OGRE_MAX_NUM_BOUND_DESCRIPTOR_SETS; ++i )
        {
            uint32 bindingIdx = 0u;
            for( size_t j = 0u; j < DescBindingTypes::NumDescBindingTypes; ++j )
            {
                textStr.resize( prefixSize0 );
                textStr.aChar( c_bufferTypes[j] );
                const size_t prefixSize1 = textStr.size();

                if( j == DescBindingTypes::ParamBuffer )
                {
                    // ParamBuffer is special because it's always ogre_P0, but we still
                    // must still account all the other stages' binding indices while counting
                    if( mDescBindingRanges[i][j].isInUse() )
                    {
                        if( mParamsBuffStages & ( 1u << shaderStage ) )
                        {
                            textStr.resize( prefixSize1 );  // #define ogre_P
                            uint32 numPrevStagesWithParams = 0u;
                            if( !mCompute )
                            {
                                for( size_t k = 0u; k < shaderStage; ++k )
                                {
                                    if( mParamsBuffStages & ( 1u << k ) )
                                        ++numPrevStagesWithParams;
                                }
                            }

                            if( shaderType == GLSL )
                            {
                                // #define ogre_P0 set = 1, binding = 6
                                textStr.a( "0", " set = ", (uint32)i,
                                           ", binding = ", numPrevStagesWithParams, "\n" );
                            }
                            else
                            {
                                // #define ogre_B3 c3
                                textStr.a( "0 " );
                                textStr.aChar( c_HLSLBufferTypesMap[j] );
                                textStr.a( numPrevStagesWithParams, "\n" );
                            }

                            macroStr += textStr.c_str();
                        }

                        const size_t numSlots = mDescBindingRanges[i][j].getNumUsedSlots();
                        bindingIdx += numSlots;
                    }
                }
                else
                {
                    uint32 emulatedSlot = mDescBindingRanges[i][j].start;
                    const size_t numSlots = mDescBindingRanges[i][j].getNumUsedSlots();
                    for( size_t k = 0u; k < numSlots; ++k )
                    {
                        textStr.resize( prefixSize1 );  // #define ogre_B
                        if( shaderType == GLSL )
                        {
                            // #define ogre_B3 set = 1, binding = 6
                            textStr.a( emulatedSlot, " set = ", (uint32)i, ", binding = ", bindingIdx,
                                       "\n" );
                        }
                        else
                        {
                            // #define ogre_B3 c3
                            textStr.a( emulatedSlot, " " );
                            textStr.aChar( c_HLSLBufferTypesMap[j] );
                            textStr.a( bindingIdx, "\n" );
                        }

                        ++bindingIdx;
                        ++emulatedSlot;

                        macroStr += textStr.c_str();
                    }
                }
            }
        }
        macroStr.swap( inOutString );
    }
    //-------------------------------------------------------------------------
    VkPipelineLayout VulkanRootLayout::createVulkanHandles( void )
    {
        if( mRootLayout )
            return mRootLayout;

        FastArray<FastArray<VkDescriptorSetLayoutBinding> > rootLayoutDesc;

        const size_t numSets = calculateNumUsedSets();
        rootLayoutDesc.resize( numSets );
        mSets.resize( numSets );
        mPools.resize( numSets );

        for( size_t i = 0u; i < numSets; ++i )
        {
            rootLayoutDesc[i].resize( calculateNumBindings( i ) );

            size_t bindingIdx = 0u;

            for( size_t j = 0u; j < DescBindingTypes::NumDescBindingTypes; ++j )
            {
                const size_t numSlots = mDescBindingRanges[i][j].getNumUsedSlots();
                for( size_t k = 0u; k < numSlots; ++k )
                {
                    rootLayoutDesc[i][bindingIdx].binding = static_cast<uint32_t>( bindingIdx );
                    rootLayoutDesc[i][bindingIdx].descriptorType = static_cast<VkDescriptorType>(
                        toVkDescriptorType( static_cast<DescBindingTypes::DescBindingTypes>( j ) ) );
                    rootLayoutDesc[i][bindingIdx].descriptorCount = 1u;
                    rootLayoutDesc[i][bindingIdx].stageFlags =
                        mCompute ? VK_SHADER_STAGE_COMPUTE_BIT : VK_SHADER_STAGE_ALL_GRAPHICS;

                    if( !mCompute && j == DescBindingTypes::ParamBuffer &&
                        ( mParamsBuffStages & c_allGraphicStagesMask ) != c_allGraphicStagesMask )
                    {
                        if( mParamsBuffStages & ( 1u << VertexShader ) )
                            rootLayoutDesc[i][bindingIdx].stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
                        if( mParamsBuffStages & ( 1u << PixelShader ) )
                            rootLayoutDesc[i][bindingIdx].stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
                        if( mParamsBuffStages & ( 1u << GeometryShader ) )
                            rootLayoutDesc[i][bindingIdx].stageFlags |= VK_SHADER_STAGE_GEOMETRY_BIT;
                        if( mParamsBuffStages & ( 1u << HullShader ) )
                        {
                            rootLayoutDesc[i][bindingIdx].stageFlags |=
                                VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
                        }
                        if( mParamsBuffStages & ( 1u << DomainShader ) )
                        {
                            rootLayoutDesc[i][bindingIdx].stageFlags |=
                                VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
                        }
                    }
                    rootLayoutDesc[i][bindingIdx].pImmutableSamplers = 0;

                    ++bindingIdx;
                }
            }

            mSets[i] = mProgramManager->getCachedSet( rootLayoutDesc[i] );
        }

        VulkanDevice *device = mProgramManager->getDevice();

        VkPipelineLayoutCreateInfo pipelineLayoutCi;
        makeVkStruct( pipelineLayoutCi, VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO );
        pipelineLayoutCi.setLayoutCount = static_cast<uint32>( numSets );
        pipelineLayoutCi.pSetLayouts = mSets.begin();

        VkResult result = vkCreatePipelineLayout( device->mDevice, &pipelineLayoutCi, 0, &mRootLayout );
        checkVkResult( result, "vkCreatePipelineLayout" );

        return mRootLayout;
    }
    //-------------------------------------------------------------------------
    inline void VulkanRootLayout::bindCommon( VkWriteDescriptorSet &writeDescSet,
                                              size_t &numWriteDescSets, uint32 &currBinding,
                                              VkDescriptorSet descSet,
                                              const DescBindingRange &bindRanges )
    {
        makeVkStruct( writeDescSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET );
        writeDescSet.dstSet = descSet;
        writeDescSet.dstBinding = currBinding;
        writeDescSet.dstArrayElement = 0u;
        writeDescSet.descriptorCount = static_cast<uint32_t>( bindRanges.getNumUsedSlots() );
        currBinding += bindRanges.getNumUsedSlots();
        ++numWriteDescSets;
    }
    //-------------------------------------------------------------------------
    inline void VulkanRootLayout::bindParamsBuffer( VkWriteDescriptorSet *writeDescSets,
                                                    size_t &numWriteDescSets, uint32 &currBinding,
                                                    VkDescriptorSet descSet,
                                                    const DescBindingRange *descBindingRanges,
                                                    const VulkanGlobalBindingTable &table )
    {
        const DescBindingRange &bindRanges = descBindingRanges[DescBindingTypes::ParamBuffer];

        if( !bindRanges.isInUse() )
            return;

        if( mCompute )
        {
            VkWriteDescriptorSet &writeDescSet = writeDescSets[numWriteDescSets];
            bindCommon( writeDescSet, numWriteDescSets, currBinding, descSet, bindRanges );
            writeDescSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writeDescSet.pBufferInfo = &table.paramsBuffer[GPT_COMPUTE_PROGRAM];
        }
        else
        {
            for( size_t i = 0u; i < NumShaderTypes; ++i )
            {
                if( mParamsBuffStages & ( 1u << i ) )
                {
                    VkWriteDescriptorSet &writeDescSet = writeDescSets[numWriteDescSets];

                    makeVkStruct( writeDescSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET );
                    writeDescSet.dstSet = descSet;
                    writeDescSet.dstBinding = currBinding;
                    writeDescSet.dstArrayElement = 0u;
                    writeDescSet.descriptorCount = 1u;
                    ++currBinding;
                    ++numWriteDescSets;

                    writeDescSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    writeDescSet.pBufferInfo = &table.paramsBuffer[i];
                }
            }
        }
    }
    //-------------------------------------------------------------------------
    inline void VulkanRootLayout::bindConstBuffers( VkWriteDescriptorSet *writeDescSets,
                                                    size_t &numWriteDescSets, uint32 &currBinding,
                                                    VkDescriptorSet descSet,
                                                    const DescBindingRange *descBindingRanges,
                                                    const VulkanGlobalBindingTable &table )
    {
        const DescBindingRange &bindRanges = descBindingRanges[DescBindingTypes::ConstBuffer];

        if( !bindRanges.isInUse() )
            return;

        VkWriteDescriptorSet &writeDescSet = writeDescSets[numWriteDescSets];
        bindCommon( writeDescSet, numWriteDescSets, currBinding, descSet, bindRanges );
        writeDescSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescSet.pBufferInfo = &table.constBuffers[bindRanges.start];
    }
    //-------------------------------------------------------------------------
    inline void VulkanRootLayout::bindReadOnlyBuffers( VkWriteDescriptorSet *writeDescSets,
                                                       size_t &numWriteDescSets, uint32 &currBinding,
                                                       VkDescriptorSet descSet,
                                                       const DescBindingRange *descBindingRanges,
                                                       const VulkanGlobalBindingTable &table )
    {
        const DescBindingRange &bindRanges = descBindingRanges[DescBindingTypes::ReadOnlyBuffer];

        if( !bindRanges.isInUse() )
            return;

        VkWriteDescriptorSet &writeDescSet = writeDescSets[numWriteDescSets];
        bindCommon( writeDescSet, numWriteDescSets, currBinding, descSet, bindRanges );
        writeDescSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        writeDescSet.pBufferInfo = &table.readOnlyBuffers[bindRanges.start];
    }
    //-------------------------------------------------------------------------
    inline void VulkanRootLayout::bindTexBuffers( VkWriteDescriptorSet *writeDescSets,
                                                  size_t &numWriteDescSets, uint32 &currBinding,
                                                  VkDescriptorSet descSet,
                                                  const DescBindingRange *descBindingRanges,
                                                  const VulkanGlobalBindingTable &table )
    {
        const DescBindingRange &bindRanges = descBindingRanges[DescBindingTypes::TexBuffer];

        if( !bindRanges.isInUse() )
            return;

        VkWriteDescriptorSet &writeDescSet = writeDescSets[numWriteDescSets];
        bindCommon( writeDescSet, numWriteDescSets, currBinding, descSet, bindRanges );
        writeDescSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
        writeDescSet.pTexelBufferView = &table.texBuffers[bindRanges.start];
    }
    //-------------------------------------------------------------------------
    inline void VulkanRootLayout::bindTextures( VkWriteDescriptorSet *writeDescSets,
                                                size_t &numWriteDescSets, uint32 &currBinding,
                                                VkDescriptorSet descSet,
                                                const DescBindingRange *descBindingRanges,
                                                const VulkanGlobalBindingTable &table )
    {
        const DescBindingRange &bindRanges = descBindingRanges[DescBindingTypes::Texture];

        if( !bindRanges.isInUse() )
            return;

        VkWriteDescriptorSet &writeDescSet = writeDescSets[numWriteDescSets];
        bindCommon( writeDescSet, numWriteDescSets, currBinding, descSet, bindRanges );
        writeDescSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        writeDescSet.pImageInfo = &table.textures[bindRanges.start];
    }
    //-------------------------------------------------------------------------
    inline void VulkanRootLayout::bindSamplers( VkWriteDescriptorSet *writeDescSets,
                                                size_t &numWriteDescSets, uint32 &currBinding,
                                                VkDescriptorSet descSet,
                                                const DescBindingRange *descBindingRanges,
                                                const VulkanGlobalBindingTable &table )
    {
        const DescBindingRange &bindRanges = descBindingRanges[DescBindingTypes::Sampler];

        if( !bindRanges.isInUse() )
            return;

        VkWriteDescriptorSet &writeDescSet = writeDescSets[numWriteDescSets];
        bindCommon( writeDescSet, numWriteDescSets, currBinding, descSet, bindRanges );
        writeDescSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        writeDescSet.pImageInfo = &table.samplers[bindRanges.start];
    }
    //-------------------------------------------------------------------------
    uint32 VulkanRootLayout::calculateFirstDirtySet( const VulkanGlobalBindingTable &table ) const
    {
        uint32 firstDirtySet = 0u;
        const size_t numSets = mSets.size();
        for( size_t i = 0u; i < numSets; ++i )
        {
            bool bDirty = false;

            const DescBindingRange *ranges = mDescBindingRanges[i];

            if( !mBaked[i] )
            {
                bDirty |= table.dirtyParamsBuffer & ranges[DescBindingTypes::ParamBuffer].isInUse();
                bDirty |= ranges[DescBindingTypes::ConstBuffer].isDirty( table.minDirtySlotConst );
                bDirty |= ranges[DescBindingTypes::TexBuffer].isDirty( table.minDirtySlotTexBuffer );
                bDirty |= ranges[DescBindingTypes::Texture].isDirty( table.minDirtySlotTextures );
                bDirty |= ranges[DescBindingTypes::Sampler].isDirty( table.minDirtySlotSamplers );
                bDirty |=
                    ranges[DescBindingTypes::ReadOnlyBuffer].isDirty( table.minDirtySlotReadOnlyBuffer );
            }
            else
            {
                bDirty |=
                    table.dirtyBakedTextures & ( ranges[DescBindingTypes::ReadOnlyBuffer].isInUse() |
                                                 ranges[DescBindingTypes::TexBuffer].isInUse() |
                                                 ranges[DescBindingTypes::Texture].isInUse() );
                bDirty |= table.dirtyBakedSamplers & ranges[DescBindingTypes::Sampler].isInUse();
                bDirty |= table.dirtyBakedUavs & ( ranges[DescBindingTypes::UavBuffer].isInUse() |
                                                   ranges[DescBindingTypes::UavTexture].isInUse() );
            }

            if( bDirty )
                return firstDirtySet;

            ++firstDirtySet;
        }

        return firstDirtySet;
    }
    //-------------------------------------------------------------------------
    void VulkanRootLayout::bind( VulkanDevice *device, VulkanVaoManager *vaoManager,
                                 const VulkanGlobalBindingTable &table )
    {
        VkDescriptorSet descSets[OGRE_MAX_NUM_BOUND_DESCRIPTOR_SETS];

        const size_t numSets = mSets.size();

        const uint32 firstDirtySet = calculateFirstDirtySet( table );

        for( size_t i = firstDirtySet; i < numSets; ++i )
        {
            if( !mPools[i] || !mPools[i]->isAvailableInCurrentFrame() )
                mPools[i] = vaoManager->getDescriptorPool( this, i, mSets[i] );

            VkDescriptorSet descSet = mPools[i]->allocate( device, mSets[i] );

            const DescBindingRange *descBindingRanges = mDescBindingRanges[i];

            size_t numWriteDescSets = 0u;
            uint32 currBinding = 0u;
            // ParamBuffer consumes up to 1 per stage (not just 1)
            VkWriteDescriptorSet writeDescSets[DescBindingTypes::Sampler + NumShaderTypes];

            // Note: We must bind in the same order as DescBindingTypes
            if( !mBaked[i] )
            {
                bindParamsBuffer( writeDescSets, numWriteDescSets, currBinding, descSet,
                                  descBindingRanges, table );
                bindConstBuffers( writeDescSets, numWriteDescSets, currBinding, descSet,
                                  descBindingRanges, table );
                bindReadOnlyBuffers( writeDescSets, numWriteDescSets, currBinding, descSet,
                                     descBindingRanges, table );
                bindTexBuffers( writeDescSets, numWriteDescSets, currBinding, descSet, descBindingRanges,
                                table );
                bindTextures( writeDescSets, numWriteDescSets, currBinding, descSet, descBindingRanges,
                              table );
                bindSamplers( writeDescSets, numWriteDescSets, currBinding, descSet, descBindingRanges,
                              table );
            }
            else
            {
                for( size_t i = 0u; i < BakedDescriptorSets::NumBakedDescriptorSets; ++i )
                {
                    const DescBindingRange &bindRanges =
                        descBindingRanges[i + DescBindingTypes::ReadOnlyBuffer];
                    if( bindRanges.isInUse() )
                    {
                        OGRE_ASSERT_MEDIUM( table.bakedDescriptorSets[i] &&
                                            "No DescriptorSetTexture/Sampler/Uav bound when expected!" );
                        OGRE_ASSERT_MEDIUM( table.bakedDescriptorSets[i]->descriptorCount ==
                                                bindRanges.getNumUsedSlots() &&
                                            "DescriptorSetTexture/Sampler/Uav provided is incompatible "
                                            "with active RootLayout. e.g. you can't bind a set of "
                                            "7 textures when the shader expects 4" );

                        VkWriteDescriptorSet &writeDescSet = writeDescSets[numWriteDescSets];
                        writeDescSet = *table.bakedDescriptorSets[i];
                        writeDescSet.dstSet = descSet;
                        writeDescSet.dstBinding = currBinding;
                        currBinding += writeDescSet.descriptorCount;
                        ++numWriteDescSets;
                    }
                }
            }

            vkUpdateDescriptorSets( device->mDevice, static_cast<uint32_t>( numWriteDescSets ),
                                    writeDescSets, 0u, 0 );
            descSets[i] = descSet;
        }

        // The table may have been dirty, but nothing the root layout actually uses was dirty
        if( firstDirtySet < mSets.size() )
        {
            vkCmdBindDescriptorSets(
                device->mGraphicsQueue.mCurrentCmdBuffer,
                mCompute ? VK_PIPELINE_BIND_POINT_COMPUTE : VK_PIPELINE_BIND_POINT_GRAPHICS, mRootLayout,
                firstDirtySet, static_cast<uint32_t>( mSets.size() ) - firstDirtySet,
                &descSets[firstDirtySet], 0u, 0 );
        }
    }
    //-------------------------------------------------------------------------
    VulkanRootLayout *VulkanRootLayout::findBest( VulkanRootLayout *a, VulkanRootLayout *b )
    {
        if( !b )
            return a;
        if( !a )
            return b;
        if( a == b )
            return a;

        VulkanRootLayout *best = 0;

        for( size_t i = 0u; i < OGRE_MAX_NUM_BOUND_DESCRIPTOR_SETS; ++i )
        {
            VulkanRootLayout *other = 0;

            bool bDiverged = false;
            size_t numSlotsA = 0u;
            size_t numSlotsB = 0u;
            for( size_t j = 0u; j < DescBindingTypes::NumDescBindingTypes; ++j )
            {
                numSlotsA += a->mDescBindingRanges[i][j].getNumUsedSlots();
                numSlotsB += b->mDescBindingRanges[i][j].getNumUsedSlots();

                if( !bDiverged )
                {
                    if( numSlotsA != numSlotsB )
                    {
                        VulkanRootLayout *newBest = 0;
                        if( numSlotsA > numSlotsB )
                        {
                            newBest = a;
                            other = b;
                        }
                        else
                        {
                            newBest = b;
                            other = a;
                        }

                        if( best && best != newBest )
                        {
                            // This is the first divergence within this set idx
                            // However a previous set diverged; and the 'best' one
                            // is not this time's winner.
                            //
                            // a and b are incompatible
                            return 0;
                        }

                        best = newBest;
                        bDiverged = true;
                    }
                }
                else
                {
                    // a and b were already on a path to being incompatible
                    if( other->mDescBindingRanges[i][j].isInUse() )
                        return 0;  // a and b are incompatible
                }
            }
        }

        if( !best )
        {
            // If we're here then a and b are equivalent? We should not arrive here due to
            // VulkanGpuProgramManager::getRootLayout always returning the same layout.
            // Anyway, pick any just in case
            best = a;
        }

        return best;
    }
    //-------------------------------------------------------------------------
    bool VulkanRootLayout::operator<( const VulkanRootLayout &other ) const
    {
        if( this->mCompute != other.mCompute )
            return this->mCompute < other.mCompute;
        if( this->mParamsBuffStages != other.mParamsBuffStages )
            return this->mParamsBuffStages < other.mParamsBuffStages;

        for( size_t i = 0u; i < OGRE_MAX_NUM_BOUND_DESCRIPTOR_SETS; ++i )
        {
            if( this->mBaked[i] != other.mBaked[i] )
                return this->mBaked[i] < other.mBaked[i];

            for( size_t j = 0u; j < DescBindingTypes::NumDescBindingTypes; ++j )
            {
                if( this->mDescBindingRanges[i][j].start != other.mDescBindingRanges[i][j].start )
                    return this->mDescBindingRanges[i][j].start < other.mDescBindingRanges[i][j].start;
                if( this->mDescBindingRanges[i][j].end != other.mDescBindingRanges[i][j].end )
                    return this->mDescBindingRanges[i][j].end < other.mDescBindingRanges[i][j].end;
            }
        }

        // If we're here then a and b are equals, thus a < b returns false
        return false;
    }
}  // namespace Ogre
