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

#include "OgreVulkanCache.h"

#include "OgreVulkanDevice.h"
#include "OgreVulkanUtils.h"

#include "OgreException.h"

namespace Ogre
{
    VulkanCache::VkRenderPassCreateInfoCmp::CmpResult  //
    VulkanCache::VkRenderPassCreateInfoCmp::cmp( const VkAttachmentDescription &a,
                                                 const VkAttachmentDescription &b ) const
    {
        if( a.flags != b.flags )
            return a.flags < b.flags ? CmpResultLess : CmpResultGreater;
        if( a.format != b.format )
            return a.format < b.format ? CmpResultLess : CmpResultGreater;
        if( a.samples != b.samples )
            return a.samples < b.samples ? CmpResultLess : CmpResultGreater;
        if( a.loadOp != b.loadOp )
            return a.loadOp < b.loadOp ? CmpResultLess : CmpResultGreater;
        if( a.storeOp != b.storeOp )
            return a.storeOp < b.storeOp ? CmpResultLess : CmpResultGreater;
        if( a.stencilLoadOp != b.stencilLoadOp )
            return a.stencilLoadOp < b.stencilLoadOp ? CmpResultLess : CmpResultGreater;
        if( a.stencilStoreOp != b.stencilStoreOp )
            return a.stencilStoreOp < b.stencilStoreOp ? CmpResultLess : CmpResultGreater;
        if( a.initialLayout != b.initialLayout )
            return a.initialLayout < b.initialLayout ? CmpResultLess : CmpResultGreater;
        if( a.finalLayout != b.finalLayout )
            return a.finalLayout < b.finalLayout ? CmpResultLess : CmpResultGreater;
        return CmpResultEqual;
    }
    //-------------------------------------------------------------------------
    VulkanCache::VkRenderPassCreateInfoCmp::CmpResult  //
    VulkanCache::VkRenderPassCreateInfoCmp::cmp( const VkAttachmentReference &a,
                                                 const VkAttachmentReference &b ) const
    {
        if( a.attachment != b.attachment )
            return a.attachment < b.attachment ? CmpResultLess : CmpResultGreater;
        if( a.layout != b.layout )
            return a.layout < b.layout ? CmpResultLess : CmpResultGreater;
        return CmpResultEqual;
    }
    //-------------------------------------------------------------------------
    VulkanCache::VkRenderPassCreateInfoCmp::CmpResult  //
    VulkanCache::VkRenderPassCreateInfoCmp::cmp( const VkSubpassDescription &a,
                                                 const VkSubpassDescription &b ) const
    {
        if( a.flags != b.flags )
            return a.flags < b.flags ? CmpResultLess : CmpResultGreater;
        if( a.pipelineBindPoint != b.pipelineBindPoint )
            return a.pipelineBindPoint < b.pipelineBindPoint ? CmpResultLess : CmpResultGreater;
        if( a.inputAttachmentCount != b.inputAttachmentCount )
            return a.inputAttachmentCount < b.inputAttachmentCount ? CmpResultLess : CmpResultGreater;
        if( a.colorAttachmentCount != b.colorAttachmentCount )
            return a.colorAttachmentCount < b.colorAttachmentCount ? CmpResultLess : CmpResultGreater;
        if( a.preserveAttachmentCount != b.preserveAttachmentCount )
        {
            return a.preserveAttachmentCount < b.preserveAttachmentCount ? CmpResultLess
                                                                         : CmpResultGreater;
        }
        for( size_t i = 0u; i < a.preserveAttachmentCount; ++i )
        {
            if( a.pPreserveAttachments[i] != b.pPreserveAttachments[i] )
                return a.pPreserveAttachments[i] < b.pPreserveAttachments[i] ? CmpResultLess
                                                                             : CmpResultGreater;
        }

        const bool isResolveAttachmPresentA = a.pResolveAttachments != 0;
        const bool isResolveAttachmPresentB = b.pResolveAttachments != 0;
        if( isResolveAttachmPresentA != isResolveAttachmPresentB )
        {
            return isResolveAttachmPresentA < isResolveAttachmPresentB ? CmpResultLess
                                                                       : CmpResultGreater;
        }

        const bool isDepthAttachmPresentA = a.pDepthStencilAttachment != 0;
        const bool isDepthAttachmPresentB = b.pDepthStencilAttachment != 0;
        if( isDepthAttachmPresentA != isDepthAttachmPresentB )
            return isDepthAttachmPresentA < isDepthAttachmPresentB ? CmpResultLess : CmpResultGreater;

        for( size_t i = 0u; i < a.inputAttachmentCount; ++i )
        {
            CmpResult cmpResult = cmp( a.pInputAttachments[i], b.pInputAttachments[i] );
            if( cmpResult != CmpResultEqual )
                return cmpResult;
        }
        for( size_t i = 0u; i < a.colorAttachmentCount; ++i )
        {
            CmpResult cmpResult = cmp( a.pColorAttachments[i], b.pColorAttachments[i] );
            if( cmpResult != CmpResultEqual )
                return cmpResult;
            if( a.pResolveAttachments )
            {
                cmpResult = cmp( a.pResolveAttachments[i], b.pResolveAttachments[i] );
                if( cmpResult != CmpResultEqual )
                    return cmpResult;
            }
        }

        if( a.pDepthStencilAttachment )
        {
            CmpResult cmpResult = cmp( *a.pDepthStencilAttachment, *b.pDepthStencilAttachment );
            if( cmpResult != CmpResultEqual )
                return cmpResult;
        }

        return CmpResultEqual;
    }
    VulkanCache::VkRenderPassCreateInfoCmp::CmpResult VulkanCache::VkRenderPassCreateInfoCmp::cmp(
        const VkSubpassDependency &a, const VkSubpassDependency &b ) const
    {
        return CmpResult();
    }
    //-------------------------------------------------------------------------
    bool VulkanCache::VkRenderPassCreateInfoCmp::operator()( const VkRenderPassCreateInfo &a,
                                                             const VkRenderPassCreateInfo &b ) const
    {
        CmpResult cmpResult;

        if( a.flags != b.flags )
            return a.flags < b.flags;
        if( a.attachmentCount != b.attachmentCount )
            return a.attachmentCount < b.attachmentCount;
        for( size_t i = 0u; i < a.attachmentCount; ++i )
        {
            cmpResult = cmp( a.pAttachments[i], b.pAttachments[i] );
            if( cmpResult != CmpResultEqual )
                return cmpResult == CmpResultLess;
        }

        if( a.subpassCount != b.subpassCount )
            return a.subpassCount < b.subpassCount;
        for( size_t i = 0u; i < a.subpassCount; ++i )
        {
            cmpResult = cmp( a.pSubpasses[i], b.pSubpasses[i] );
            if( cmpResult != CmpResultEqual )
                return cmpResult == CmpResultLess;
        }
        if( a.dependencyCount != b.dependencyCount )
            return a.dependencyCount < b.dependencyCount;

        for( size_t i = 0u; i < a.dependencyCount; ++i )
        {
            cmpResult = cmp( a.pDependencies[i], b.pDependencies[i] );
            if( cmpResult != CmpResultEqual )
                return cmpResult == CmpResultLess;
        }

        return false;
    }
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    VulkanCache::VulkanCache( VulkanDevice *device ) : mDevice( device ) {}
    //-------------------------------------------------------------------------
    VulkanCache::~VulkanCache()
    {
        VkRenderPassMap::const_iterator itor = mRenderPassCache.begin();
        VkRenderPassMap::const_iterator endt = mRenderPassCache.end();

        while( itor != endt )
        {
            vkDestroyRenderPass( mDevice->mDevice, itor->second, 0 );
            delete[] itor->first.pAttachments;

            const size_t subpassCount = itor->first.subpassCount;
            for( size_t i = 0u; i < subpassCount; ++i )
            {
                // Attachments are all contiguous, so we just have to delete the first
                // entry we see, which points to the beginning of the array
                if( itor->first.pSubpasses[i].pInputAttachments )
                {
                    delete[] itor->first.pSubpasses[i].pInputAttachments;
                    break;
                }
                else if( itor->first.pSubpasses[i].pColorAttachments )
                {
                    delete[] itor->first.pSubpasses[i].pColorAttachments;
                    break;
                }
                else if( itor->first.pSubpasses[i].pResolveAttachments )
                {
                    delete[] itor->first.pSubpasses[i].pResolveAttachments;
                    break;
                }
                else if( itor->first.pSubpasses[i].pDepthStencilAttachment )
                {
                    delete[] itor->first.pSubpasses[i].pDepthStencilAttachment;
                    break;
                }
            }
            delete[] itor->first.pSubpasses;
            delete[] itor->first.pDependencies;

            ++itor;
        }
        mRenderPassCache.clear();
    }
    //-------------------------------------------------------------------------
    void VulkanCache::copySubpass( VkAttachmentReference const **dstStruct,
                                   const VkAttachmentReference *src, uint32_t attachmentCount,
                                   VkAttachmentReference *memoryBuffer, size_t &assignedIdx )
    {
        if( attachmentCount > 0u )
        {
            *dstStruct = &memoryBuffer[assignedIdx];
            memcpy( &memoryBuffer[assignedIdx], src, attachmentCount * sizeof( VkAttachmentReference ) );
            assignedIdx += attachmentCount;
        }
        else
        {
            *dstStruct = 0;
        }
    }
    //-------------------------------------------------------------------------
    VkRenderPass VulkanCache::getRenderPass( const VkRenderPassCreateInfo &renderPassCi )
    {
        VkRenderPass retVal = 0;
        VkRenderPassMap::const_iterator itor = mRenderPassCache.find( renderPassCi );
        if( itor == mRenderPassCache.end() )
        {
            VkRenderPassCreateInfo rpciCopy = renderPassCi;
            if( rpciCopy.attachmentCount > 0u )
            {
                VkAttachmentDescription *attachments =
                    new VkAttachmentDescription[rpciCopy.attachmentCount];
                rpciCopy.pAttachments = attachments;
                memcpy( attachments, renderPassCi.pAttachments,
                        rpciCopy.attachmentCount * sizeof( VkAttachmentDescription ) );
            }
            if( rpciCopy.subpassCount > 0u )
            {
                VkSubpassDescription *subpasses = new VkSubpassDescription[rpciCopy.subpassCount];
                rpciCopy.pSubpasses = subpasses;
                memcpy( subpasses, renderPassCi.pSubpasses,
                        rpciCopy.subpassCount * sizeof( VkSubpassDescription ) );

                size_t numTotalAttachments = 0u;
                for( size_t i = 0u; i < rpciCopy.subpassCount; ++i )
                {
                    numTotalAttachments += subpasses[i].inputAttachmentCount;
                    numTotalAttachments += subpasses[i].colorAttachmentCount;
                    if( subpasses[i].pResolveAttachments )
                        numTotalAttachments += subpasses[i].colorAttachmentCount;
                    if( subpasses[i].pDepthStencilAttachment )
                        numTotalAttachments += 1u;
                }

                // Create all attachments contiguously
                size_t assignedIdx = 0u;
                VkAttachmentReference *attachments = new VkAttachmentReference[numTotalAttachments];
                for( size_t i = 0u; i < rpciCopy.subpassCount; ++i )
                {
                    copySubpass( &subpasses[i].pInputAttachments,
                                 renderPassCi.pSubpasses[i].pInputAttachments,
                                 subpasses[i].inputAttachmentCount, attachments, assignedIdx );
                    copySubpass( &subpasses[i].pColorAttachments,
                                 renderPassCi.pSubpasses[i].pColorAttachments,
                                 subpasses[i].colorAttachmentCount, attachments, assignedIdx );
                    if( renderPassCi.pSubpasses[i].pResolveAttachments )
                    {
                        copySubpass( &subpasses[i].pResolveAttachments,
                                     renderPassCi.pSubpasses[i].pResolveAttachments,
                                     subpasses[i].colorAttachmentCount, attachments, assignedIdx );
                    }
                    if( subpasses[i].pDepthStencilAttachment )
                    {
                        copySubpass( &subpasses[i].pDepthStencilAttachment,
                                     renderPassCi.pSubpasses[i].pDepthStencilAttachment, 1u, attachments,
                                     assignedIdx );
                    }
                }

                OGRE_ASSERT_MEDIUM( assignedIdx == numTotalAttachments );
            }
            if( rpciCopy.dependencyCount > 0u )
            {
                VkSubpassDependency *dependencies = new VkSubpassDependency[rpciCopy.subpassCount];
                rpciCopy.pDependencies = dependencies;
                memcpy( dependencies, renderPassCi.pDependencies,
                        rpciCopy.dependencyCount * sizeof( VkSubpassDependency ) );
            }

            VkResult result =
                vkCreateRenderPass( mDevice->mGraphicsQueue.mDevice, &rpciCopy, 0, &retVal );
            checkVkResult( result, "vkCreateRenderPass" );

            mRenderPassCache[rpciCopy] = retVal;
        }
        else
            retVal = itor->second;

        return retVal;
    }
}  // namespace Ogre
