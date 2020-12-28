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

#include "OgreVulkanRenderPassDescriptor.h"

#include "OgreVulkanDevice.h"
#include "OgreVulkanRenderSystem.h"
#include "OgreVulkanTextureGpu.h"
#include "OgreVulkanTextureGpuWindow.h"
#include "OgreVulkanWindow.h"

#include "OgrePixelFormatGpuUtils.h"
#include "Vao/OgreVulkanVaoManager.h"

#include "OgreVulkanDelayedFuncs.h"
#include "OgreVulkanMappings.h"
#include "OgreVulkanUtils.h"

#if OGRE_DEBUG_MODE && OGRE_PLATFORM == OGRE_PLATFORM_LINUX
#    include <execinfo.h>  //backtrace
#endif

#define TODO_use_render_pass_that_can_load

namespace Ogre
{
    VulkanRenderPassDescriptor::VulkanRenderPassDescriptor( VulkanQueue *graphicsQueue,
                                                            VulkanRenderSystem *renderSystem ) :
        mSharedFboItor( renderSystem->_getFrameBufferDescMap().end() ),
        mSharedFboFlushItor( renderSystem->_getFlushOnlyDescMap().end() ),
        mTargetWidth( 0u ),
        mTargetHeight( 0u ),
        mQueue( graphicsQueue ),
        mRenderSystem( renderSystem )
#if OGRE_DEBUG_MODE && OGRE_PLATFORM == OGRE_PLATFORM_LINUX
        ,
        mNumCallstackEntries( 0 )
#endif
    {
    }
    //-----------------------------------------------------------------------------------
    VulkanRenderPassDescriptor::~VulkanRenderPassDescriptor() { releaseFbo(); }
    //-----------------------------------------------------------------------------------
    void VulkanRenderPassDescriptor::checkRenderWindowStatus( void )
    {
        if( ( mNumColourEntries > 0 && mColour[0].texture->isRenderWindowSpecific() ) ||
            ( mDepth.texture && mDepth.texture->isRenderWindowSpecific() ) ||
            ( mStencil.texture && mStencil.texture->isRenderWindowSpecific() ) )
        {
            if( mNumColourEntries > 1u )
            {
                OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                             "Cannot use RenderWindow as MRT with other colour textures",
                             "VulkanRenderPassDescriptor::colourEntriesModified" );
            }

            if( ( mNumColourEntries > 0 && !mColour[0].texture->isRenderWindowSpecific() ) ||
                ( mDepth.texture && !mDepth.texture->isRenderWindowSpecific() ) ||
                ( mStencil.texture && !mStencil.texture->isRenderWindowSpecific() ) )
            {
                OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                             "Cannot mix RenderWindow colour texture with depth or stencil buffer "
                             "that aren't for RenderWindows, or viceversa",
                             "VulkanRenderPassDescriptor::checkRenderWindowStatus" );
            }
        }

        calculateSharedKey();
    }
    //-----------------------------------------------------------------------------------
    void VulkanRenderPassDescriptor::calculateSharedKey( void )
    {
        VulkanFrameBufferDescKey key( *this );
        VulkanFrameBufferDescMap &frameBufferDescMap = mRenderSystem->_getFrameBufferDescMap();
        VulkanFrameBufferDescMap::iterator newItor = frameBufferDescMap.find( key );

        if( newItor == frameBufferDescMap.end() )
        {
            VulkanFrameBufferDescValue value;
            value.refCount = 0;
            frameBufferDescMap[key] = value;
            newItor = frameBufferDescMap.find( key );
        }

        ++newItor->second.refCount;

        releaseFbo();

        mSharedFboItor = newItor;
        calculateSharedFlushOnlyKey();
    }
    //-----------------------------------------------------------------------------------
    void VulkanRenderPassDescriptor::calculateSharedFlushOnlyKey( void )
    {
        FrameBufferDescKey key( *this );
        VulkanFlushOnlyDescMap &frameBufferDescMap = mRenderSystem->_getFlushOnlyDescMap();
        VulkanFlushOnlyDescMap::iterator newItor = frameBufferDescMap.find( key );

        if( newItor == frameBufferDescMap.end() )
        {
            VulkanFlushOnlyDescValue value;
            value.refCount = 0;
            frameBufferDescMap[key] = value;
            newItor = frameBufferDescMap.find( key );
        }

        ++newItor->second.refCount;

        mSharedFboFlushItor = newItor;
    }
    //-----------------------------------------------------------------------------------
    VkAttachmentLoadOp VulkanRenderPassDescriptor::get( LoadAction::LoadAction action )
    {
        switch( action )
        {
        case LoadAction::DontCare:
            return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        case LoadAction::Clear:
            return VK_ATTACHMENT_LOAD_OP_CLEAR;
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
        case LoadAction::ClearOnTilers:
            return VK_ATTACHMENT_LOAD_OP_CLEAR;
#else
        case LoadAction::ClearOnTilers:
            return VK_ATTACHMENT_LOAD_OP_LOAD;
#endif
        case LoadAction::Load:
            return VK_ATTACHMENT_LOAD_OP_LOAD;
        }

        return VK_ATTACHMENT_LOAD_OP_LOAD;
    }
    //-----------------------------------------------------------------------------------
    VkAttachmentStoreOp VulkanRenderPassDescriptor::get( StoreAction::StoreAction action,
                                                         bool bResolveTarget )
    {
        switch( action )
        {
        case StoreAction::DontCare:
            return VK_ATTACHMENT_STORE_OP_DONT_CARE;
        case StoreAction::Store:
            OGRE_ASSERT_LOW( !bResolveTarget &&
                             "We shouldn't add a resolve attachment if we aren't resolving" );
            return VK_ATTACHMENT_STORE_OP_STORE;
        case StoreAction::MultisampleResolve:
            return bResolveTarget ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
        case StoreAction::StoreAndMultisampleResolve:
            return VK_ATTACHMENT_STORE_OP_STORE;
        case StoreAction::StoreOrResolve:
            OGRE_ASSERT_LOW( false &&
                             "StoreOrResolve is invalid. "
                             "Compositor should've set one or the other already!" );
            return VK_ATTACHMENT_STORE_OP_STORE;
        }

        return VK_ATTACHMENT_STORE_OP_STORE;
    }
    //-----------------------------------------------------------------------------------
    VkClearColorValue VulkanRenderPassDescriptor::getClearColour( const ColourValue &clearColour,
                                                                  PixelFormatGpu pixelFormat )
    {
        const bool isInteger = PixelFormatGpuUtils::isInteger( pixelFormat );
        const bool isSigned = PixelFormatGpuUtils::isSigned( pixelFormat );
        VkClearColorValue retVal;
        if( !isInteger )
        {
            for( size_t i = 0u; i < 4u; ++i )
                retVal.float32[i] = static_cast<float>( clearColour[i] );
        }
        else
        {
            if( !isSigned )
            {
                for( size_t i = 0u; i < 4u; ++i )
                    retVal.uint32[i] = static_cast<uint32>( clearColour[i] );
            }
            else
            {
                for( size_t i = 0u; i < 4u; ++i )
                    retVal.int32[i] = static_cast<int32>( clearColour[i] );
            }
        }
        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void VulkanRenderPassDescriptor::sanitizeMsaaResolve( size_t colourIdx )
    {
#if VULKAN_DISABLED
        const size_t i = colourIdx;

        // iOS_GPUFamily3_v2, OSX_GPUFamily1_v2
        if( mColour[i].storeAction == StoreAction::StoreAndMultisampleResolve &&
            !mRenderSystem->hasStoreAndMultisampleResolve() &&
            ( mColour[i].texture->getMsaa() > 1u && mColour[i].resolveTexture ) )
        {
            // Must emulate the behavior (slower)
            mColourAttachment[i].storeAction = MTLStoreActionStore;
            mColourAttachment[i].resolveTexture = nil;
            mResolveColourAttachm[i] = [mColourAttachment[i] copy];
            mResolveColourAttachm[i].loadAction = MTLLoadActionLoad;
            mResolveColourAttachm[i].storeAction = MTLStoreActionMultisampleResolve;

            mRequiresManualResolve = true;
        }
        else if( mColour[i].storeAction == StoreAction::DontCare ||
                 mColour[i].storeAction == StoreAction::Store )
        {
            mColourAttachment[i].resolveTexture = nil;
        }
#endif
    }
    //-----------------------------------------------------------------------------------
    /**
    @brief VulkanRenderPassDescriptor::setupColourAttachment
        This will setup:
            attachments[currAttachmIdx]
            colourAttachRefs[vkIdx]
            resolveAttachRefs[vkIdx]
            fboDesc.mImageViews[currAttachmIdx]
            fboDesc.mWindowImageViews

        Except mWindowImageViews, all the other variables are *always* written to.
    @param idx [in]
        idx to mColour[idx]
    @param fboDesc [in/out]
    @param attachments [out]
        A pointer to setup VkAttachmentDescription
    @param currAttachmIdx [in/out]
        A value to index attachments[currAttachmIdx]
    @param colourAttachRefs [out]
        A pointer to setup VkAttachmentReference
    @param resolveAttachRefs [out]
        A pointer to setup VkAttachmentReference
    @param vkIdx [in]
        A value to index both colourAttachRefs[vkIdx] & resolveAttachRefs[vkIdx]
        Very often idx == vkIdx except when we skip a colour entry due to being PFG_NULL
    @param resolveTex
        False if we're setting up the main target
        True if we're setting up the resolve target
    */
    void VulkanRenderPassDescriptor::setupColourAttachment(
        const size_t idx, VulkanFrameBufferDescValue &fboDesc, VkAttachmentDescription *attachments,
        uint32 &currAttachmIdx, VkAttachmentReference *colourAttachRefs,
        VkAttachmentReference *resolveAttachRefs, const size_t vkIdx, const bool bResolveTex )
    {
        const RenderPassColourTarget &colour = mColour[idx];

        if( ( !colour.texture->getSampleDescription().isMultisample() || !colour.resolveTexture ||
              ( colour.storeAction != StoreAction::MultisampleResolve &&
                colour.storeAction != StoreAction::StoreAndMultisampleResolve ) ) &&
            bResolveTex )
        {
            // There's no resolve texture to setup
            resolveAttachRefs[vkIdx].attachment = VK_ATTACHMENT_UNUSED;
            resolveAttachRefs[vkIdx].layout = VK_IMAGE_LAYOUT_UNDEFINED;
            return;
        }

        VkImage texName = 0;
        VulkanTextureGpu *texture = 0;

        if( !bResolveTex )
        {
            OGRE_ASSERT_HIGH( dynamic_cast<VulkanTextureGpu *>( colour.texture ) );
            texture = static_cast<VulkanTextureGpu *>( colour.texture );
            if( colour.texture->getSampleDescription().isMultisample() )
            {
                if( !colour.texture->hasMsaaExplicitResolves() )
                    texName = texture->getMsaaFramebufferName();
                else
                    texName = texture->getFinalTextureName();
            }
            else
                texName = texture->getFinalTextureName();
        }
        else
        {
            OGRE_ASSERT_HIGH( dynamic_cast<VulkanTextureGpu *>( colour.resolveTexture ) );
            texture = static_cast<VulkanTextureGpu *>( colour.resolveTexture );
            texName = texture->getFinalTextureName();
        }

        VkAttachmentDescription &attachment = attachments[currAttachmIdx];
        attachment.format = VulkanMappings::get( texture->getPixelFormat() );
        attachment.samples = bResolveTex ? VK_SAMPLE_COUNT_1_BIT
                                         : static_cast<VkSampleCountFlagBits>(
                                               texture->getSampleDescription().getColourSamples() );
        attachment.loadOp = bResolveTex ? VK_ATTACHMENT_LOAD_OP_DONT_CARE : get( colour.loadAction );
        attachment.storeOp = get( colour.storeAction, bResolveTex );
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        if( !bResolveTex )
        {
            if( texture->isRenderWindowSpecific() && !texture->isMultisample() &&
                mReadyWindowForPresent )
            {
                attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            }
            else
            {
                attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            }
        }
        else
        {
            attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            if( texture->isRenderWindowSpecific() && mReadyWindowForPresent )
                attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            else
                attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }

        const uint8 mipLevel = bResolveTex ? colour.resolveMipLevel : colour.mipLevel;
        const uint16 slice = bResolveTex ? colour.resolveSlice : colour.slice;

        if( !texture->isRenderWindowSpecific() || ( texture->isMultisample() && !bResolveTex ) )
        {
            fboDesc.mImageViews[currAttachmIdx] = texture->_createView(
                texture->getPixelFormat(), mipLevel, 1u, slice, false, false, 1u, texName );
        }
        else
        {
            fboDesc.mImageViews[currAttachmIdx] = 0;  // Set to null (will be set later, 1 for each FBO)

            OGRE_ASSERT_HIGH( dynamic_cast<VulkanTextureGpuWindow *>( texture ) );
            VulkanTextureGpuWindow *textureVulkan = static_cast<VulkanTextureGpuWindow *>( texture );

            OGRE_ASSERT_LOW( fboDesc.mWindowImageViews.empty() &&
                             "Only one window can be used as target" );
            const size_t numSurfaces = textureVulkan->getWindowNumSurfaces();
            fboDesc.mWindowImageViews.resize( numSurfaces );
            for( size_t surfIdx = 0u; surfIdx < numSurfaces; ++surfIdx )
            {
                texName = textureVulkan->getWindowFinalTextureName( surfIdx );
                fboDesc.mWindowImageViews[surfIdx] = texture->_createView(
                    texture->getPixelFormat(), mipLevel, 1u, slice, false, false, 1u, texName );
            }
        }

        if( bResolveTex )
        {
            resolveAttachRefs[vkIdx].attachment = currAttachmIdx;
            resolveAttachRefs[vkIdx].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            ++currAttachmIdx;
        }
        else
        {
            colourAttachRefs[vkIdx].attachment = currAttachmIdx;
            colourAttachRefs[vkIdx].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            ++currAttachmIdx;

            // Now repeat with the resolve texture (if applies)
            setupColourAttachment( idx, fboDesc, attachments, currAttachmIdx, colourAttachRefs,
                                   resolveAttachRefs, vkIdx, true );
        }
    }
    //-----------------------------------------------------------------------------------
    VkImageView VulkanRenderPassDescriptor::setupDepthAttachment( VkAttachmentDescription &attachment )
    {
        attachment.format = VulkanMappings::get( mDepth.texture->getPixelFormat() );
        attachment.samples = static_cast<VkSampleCountFlagBits>(
            mDepth.texture->getSampleDescription().getColourSamples() );
        attachment.loadOp = get( mDepth.loadAction );
        attachment.storeOp = get( mDepth.storeAction, false );
        if( mStencil.texture )
        {
            attachment.stencilLoadOp = get( mStencil.loadAction );
            attachment.stencilStoreOp = get( mStencil.storeAction, false );
        }
        else
        {
            attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        }

        if( mDepth.readOnly )
        {
            attachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
            attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        }
        else
        {
            attachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

        OGRE_ASSERT_HIGH( dynamic_cast<VulkanTextureGpu *>( mDepth.texture ) );
        VulkanTextureGpu *texture = static_cast<VulkanTextureGpu *>( mDepth.texture );
        VkImage texName = texture->getFinalTextureName();
        return texture->_createView( texture->getPixelFormat(), mDepth.mipLevel, 1u, mDepth.slice, false,
                                     false, 1u, texName );
    }
    //-----------------------------------------------------------------------------------
    void VulkanRenderPassDescriptor::setupFbo( VulkanFrameBufferDescValue &fboDesc )
    {
        if( fboDesc.mRenderPass )
            return;  // Already initialized

        if( mDepth.texture && mDepth.texture->getResidencyStatus() != GpuResidency::Resident )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "RenderTexture '" + mDepth.texture->getNameStr() + "' must be resident!",
                         "VulkanRenderPassDescriptor::updateFbo" );
        }

        if( mStencil.texture && mStencil.texture->getResidencyStatus() != GpuResidency::Resident )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "RenderTexture '" + mStencil.texture->getNameStr() + "' must be resident!",
                         "VulkanRenderPassDescriptor::updateFbo" );
        }

        if( !mDepth.texture )
        {
            if( mStencil.texture )
            {
                OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                             "Stencil without depth (RenderTexture '" + mStencil.texture->getNameStr() +
                                 "'). This is not supported by Vulkan",
                             "VulkanRenderPassDescriptor::updateFbo" );
            }
        }

        bool hasRenderWindow = false;

        uint32 attachmentIdx = 0u;
        uint32 numColourAttachments = 0u;
        uint32 windowAttachmentIdx = std::numeric_limits<uint32>::max();
        bool usesResolveAttachments = false;

        // 1 per MRT
        // 1 per MRT MSAA resolve
        // 1 for Depth buffer
        // 1 for Stencil buffer
        VkAttachmentDescription attachments[OGRE_MAX_MULTIPLE_RENDER_TARGETS * 2u + 2u];
        memset( &attachments, 0, sizeof( attachments ) );

        VkAttachmentReference colourAttachRefs[OGRE_MAX_MULTIPLE_RENDER_TARGETS];
        VkAttachmentReference resolveAttachRefs[OGRE_MAX_MULTIPLE_RENDER_TARGETS];
        VkAttachmentReference depthAttachRef;

        for( size_t i = 0; i < mNumColourEntries; ++i )
        {
            if( mColour[i].texture->getResidencyStatus() != GpuResidency::Resident )
            {
                OGRE_EXCEPT(
                    Exception::ERR_INVALIDPARAMS,
                    "RenderTexture '" + mColour[i].texture->getNameStr() + "' must be resident!",
                    "VulkanRenderPassDescriptor::updateFbo" );
            }
            if( i > 0 && hasRenderWindow != mColour[i].texture->isRenderWindowSpecific() )
            {
                // This is a GL restriction actually, which we mimic for consistency
                OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                             "Cannot use RenderWindow as MRT with other colour textures",
                             "VulkanRenderPassDescriptor::updateFbo" );
            }

            hasRenderWindow |= mColour[i].texture->isRenderWindowSpecific();

            if( mColour[i].texture->getPixelFormat() == PFG_NULL )
                continue;

            OGRE_ASSERT_HIGH( dynamic_cast<VulkanTextureGpu *>( mColour[i].texture ) );
            VulkanTextureGpu *textureVulkan = static_cast<VulkanTextureGpu *>( mColour[i].texture );

            if( textureVulkan->isRenderWindowSpecific() )
            {
                // If the window is MSAA but not being resolved,
                // then just behave like a regular RenderTexture
                if( !textureVulkan->isMultisample() )
                    windowAttachmentIdx = attachmentIdx;
                else if( textureVulkan == mColour[i].resolveTexture )
                    windowAttachmentIdx = attachmentIdx + 1u;
            }

            mClearValues[attachmentIdx].color =
                getClearColour( mColour[i].clearColour, mColour[i].texture->getPixelFormat() );

            setupColourAttachment( i, fboDesc, attachments, attachmentIdx, colourAttachRefs,
                                   resolveAttachRefs, numColourAttachments, false );
            if( resolveAttachRefs[numColourAttachments].attachment != VK_ATTACHMENT_UNUSED )
                usesResolveAttachments = true;
            ++numColourAttachments;

            sanitizeMsaaResolve( i );
        }

        if( mDepth.texture )
        {
            if( !mRenderSystem->isReverseDepth() )
                mClearValues[attachmentIdx].depthStencil.depth = static_cast<float>( mDepth.clearDepth );
            else
            {
                mClearValues[attachmentIdx].depthStencil.depth =
                    static_cast<float>( Real( 1.0 ) - mDepth.clearDepth );
            }
            mClearValues[attachmentIdx].depthStencil.stencil = mStencil.clearStencil;

            fboDesc.mImageViews[attachmentIdx] = setupDepthAttachment( attachments[attachmentIdx] );
            depthAttachRef.attachment = attachmentIdx;
            if( mDepth.readOnly )
                depthAttachRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
            else
                depthAttachRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            ++attachmentIdx;
        }

        VkSubpassDescription subpass;
        memset( &subpass, 0, sizeof( subpass ) );
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.inputAttachmentCount = 0u;
        subpass.colorAttachmentCount = numColourAttachments;
        subpass.pColorAttachments = colourAttachRefs;
        subpass.pResolveAttachments = usesResolveAttachments ? resolveAttachRefs : 0;
        subpass.pDepthStencilAttachment = mDepth.texture ? &depthAttachRef : 0;

        fboDesc.mNumImageViews = attachmentIdx;

        VkRenderPassCreateInfo renderPassCreateInfo;
        makeVkStruct( renderPassCreateInfo, VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO );
        renderPassCreateInfo.attachmentCount = attachmentIdx;
        renderPassCreateInfo.pAttachments = attachments;
        renderPassCreateInfo.subpassCount = 1u;
        renderPassCreateInfo.pSubpasses = &subpass;
        VkResult result =
            vkCreateRenderPass( mQueue->mDevice, &renderPassCreateInfo, 0, &fboDesc.mRenderPass );
        checkVkResult( result, "vkCreateRenderPass" );

        VkFramebufferCreateInfo fbCreateInfo;
        makeVkStruct( fbCreateInfo, VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO );
        fbCreateInfo.renderPass = fboDesc.mRenderPass;
        fbCreateInfo.attachmentCount = attachmentIdx;
        fbCreateInfo.pAttachments = fboDesc.mImageViews;
        fbCreateInfo.width = mTargetWidth;
        fbCreateInfo.height = mTargetHeight;
        fbCreateInfo.layers = 1u;

        const size_t numFramebuffers = std::max<size_t>( fboDesc.mWindowImageViews.size(), 1u );
        fboDesc.mFramebuffers.resize( numFramebuffers );
        for( size_t i = 0u; i < numFramebuffers; ++i )
        {
            if( !fboDesc.mWindowImageViews.empty() )
                fboDesc.mImageViews[windowAttachmentIdx] = fboDesc.mWindowImageViews[i];
            result = vkCreateFramebuffer( mQueue->mDevice, &fbCreateInfo, 0, &fboDesc.mFramebuffers[i] );
            checkVkResult( result, "vkCreateFramebuffer" );
            if( !fboDesc.mWindowImageViews.empty() )
                fboDesc.mImageViews[windowAttachmentIdx] = 0;
        }
    }
    //-----------------------------------------------------------------------------------
    void VulkanRenderPassDescriptor::releaseFbo( void )
    {
        {
            VulkanFrameBufferDescMap &frameBufferDescMap = mRenderSystem->_getFrameBufferDescMap();
            if( mSharedFboItor != frameBufferDescMap.end() )
            {
                --mSharedFboItor->second.refCount;
                if( !mSharedFboItor->second.refCount )
                {
                    destroyFbo( mQueue, mSharedFboItor->second );
                    frameBufferDescMap.erase( mSharedFboItor );
                }
                mSharedFboItor = frameBufferDescMap.end();
            }
        }
        {
            VulkanFlushOnlyDescMap &frameBufferDescMap = mRenderSystem->_getFlushOnlyDescMap();
            if( mSharedFboFlushItor != frameBufferDescMap.end() )
            {
                --mSharedFboFlushItor->second.refCount;
                if( !mSharedFboFlushItor->second.refCount )
                    frameBufferDescMap.erase( mSharedFboFlushItor );
                mSharedFboFlushItor = frameBufferDescMap.end();
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void VulkanRenderPassDescriptor::destroyFbo( VulkanQueue *queue,
                                                 VulkanFrameBufferDescValue &fboDesc )
    {
        VaoManager *vaoManager = queue->getVaoManager();

        {
            FastArray<VkFramebuffer>::const_iterator itor = fboDesc.mFramebuffers.begin();
            FastArray<VkFramebuffer>::const_iterator endt = fboDesc.mFramebuffers.end();
            while( itor != endt )
                delayed_vkDestroyFramebuffer( vaoManager, queue->mDevice, *itor++, 0 );
            fboDesc.mFramebuffers.clear();
        }

        {
            FastArray<VkImageView>::const_iterator itor = fboDesc.mWindowImageViews.begin();
            FastArray<VkImageView>::const_iterator endt = fboDesc.mWindowImageViews.end();

            while( itor != endt )
                delayed_vkDestroyImageView( vaoManager, queue->mDevice, *itor++, 0 );
            fboDesc.mWindowImageViews.clear();
        }

        for( size_t i = 0u; i < fboDesc.mNumImageViews; ++i )
        {
            if( fboDesc.mImageViews[i] )
            {
                delayed_vkDestroyImageView( vaoManager, queue->mDevice, fboDesc.mImageViews[i], 0 );
                fboDesc.mImageViews[i] = 0;
            }
        }
        fboDesc.mNumImageViews = 0u;

        delayed_vkDestroyRenderPass( vaoManager, queue->mDevice, fboDesc.mRenderPass, 0 );
        fboDesc.mRenderPass = 0;
    }
    //-----------------------------------------------------------------------------------
    void VulkanRenderPassDescriptor::notifySwapchainCreated( VulkanWindow *window )
    {
        if( mNumColourEntries > 0 && mColour[0].texture->isRenderWindowSpecific() &&
            mColour[0].texture == window->getTexture() )
        {
            entriesModified( RenderPassDescriptor::All );
        }
    }
    //-----------------------------------------------------------------------------------
    void VulkanRenderPassDescriptor::notifySwapchainDestroyed( VulkanWindow *window )
    {
        if( mNumColourEntries > 0 && mColour[0].texture->isRenderWindowSpecific() &&
            mColour[0].texture == window->getTexture() )
        {
            releaseFbo();
        }
    }
    //-----------------------------------------------------------------------------------
    void VulkanRenderPassDescriptor::entriesModified( uint32 entryTypes )
    {
        RenderPassDescriptor::entriesModified( entryTypes );

        checkRenderWindowStatus();

        TextureGpu *anyTargetTexture = 0;
        const uint8 numColourEntries = mNumColourEntries;
        for( int i = 0; i < numColourEntries && !anyTargetTexture; ++i )
            anyTargetTexture = mColour[i].texture;
        if( !anyTargetTexture )
            anyTargetTexture = mDepth.texture;
        if( !anyTargetTexture )
            anyTargetTexture = mStencil.texture;

        mTargetWidth = 0u;
        mTargetHeight = 0u;
        if( anyTargetTexture )
        {
            mTargetWidth = anyTargetTexture->getInternalWidth();
            mTargetHeight = anyTargetTexture->getInternalHeight();
        }

        if( entryTypes & RenderPassDescriptor::All )
            setupFbo( mSharedFboItor->second );
    }
    //-----------------------------------------------------------------------------------
    void VulkanRenderPassDescriptor::setClearColour( uint8 idx, const ColourValue &clearColour )
    {
        RenderPassDescriptor::setClearColour( idx, clearColour );

        size_t attachmentIdx = 0u;
        for( size_t i = 0u; i < idx; ++i )
        {
            ++attachmentIdx;
            if( mColour->resolveTexture )
                ++attachmentIdx;
        }

        mClearValues[attachmentIdx].color =
            getClearColour( clearColour, mColour[idx].texture->getPixelFormat() );
    }
    //-----------------------------------------------------------------------------------
    void VulkanRenderPassDescriptor::setClearDepth( Real clearDepth )
    {
        RenderPassDescriptor::setClearDepth( clearDepth );
        if( mDepth.texture && mSharedFboItor != mRenderSystem->_getFrameBufferDescMap().end() )
        {
            size_t attachmentIdx = mSharedFboItor->second.mNumImageViews - 1u;
            if( !mRenderSystem->isReverseDepth() )
                mClearValues[attachmentIdx].depthStencil.depth = static_cast<float>( mDepth.clearDepth );
            else
            {
                mClearValues[attachmentIdx].depthStencil.depth =
                    static_cast<float>( Real( 1.0 ) - mDepth.clearDepth );
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void VulkanRenderPassDescriptor::setClearStencil( uint32 clearStencil )
    {
        RenderPassDescriptor::setClearStencil( clearStencil );
        if( ( mDepth.texture || mStencil.texture ) &&
            mSharedFboItor != mRenderSystem->_getFrameBufferDescMap().end() )
        {
            size_t attachmentIdx = mSharedFboItor->second.mNumImageViews - 1u;
            mClearValues[attachmentIdx].depthStencil.stencil = clearStencil;
        }
    }
    //-----------------------------------------------------------------------------------
    void VulkanRenderPassDescriptor::setClearColour( const ColourValue &clearColour )
    {
        const size_t numColourEntries = mNumColourEntries;
        size_t attachmentIdx = 0u;
        for( size_t i = 0u; i < numColourEntries; ++i )
        {
            mColour[i].clearColour = clearColour;
            mClearValues[attachmentIdx].color =
                getClearColour( clearColour, mColour[i].texture->getPixelFormat() );
            ++attachmentIdx;
            if( mColour->resolveTexture )
                ++attachmentIdx;
        }
    }
    //-----------------------------------------------------------------------------------
    uint32 VulkanRenderPassDescriptor::checkForClearActions( VulkanRenderPassDescriptor *other ) const
    {
        uint32 entriesToFlush = 0;

        assert( this->mSharedFboFlushItor == other->mSharedFboFlushItor );
        assert( this->mNumColourEntries == other->mNumColourEntries );

        const RenderSystemCapabilities *capabilities = mRenderSystem->getCapabilities();
        const bool isTiler = capabilities->hasCapability( RSC_IS_TILER );

        for( size_t i = 0; i < mNumColourEntries; ++i )
        {
            // this->mColour[i].allLayers doesn't need to be analyzed
            // because it requires a different FBO.
            if( other->mColour[i].loadAction == LoadAction::Clear ||
                ( isTiler && mColour[i].loadAction == LoadAction::ClearOnTilers ) )
            {
                entriesToFlush |= RenderPassDescriptor::Colour0 << i;
            }
        }

        if( other->mDepth.loadAction == LoadAction::Clear ||
            ( isTiler && mDepth.loadAction == LoadAction::ClearOnTilers ) )
        {
            entriesToFlush |= RenderPassDescriptor::Depth;
        }

        if( other->mStencil.loadAction == LoadAction::Clear ||
            ( isTiler && mStencil.loadAction == LoadAction::ClearOnTilers ) )
        {
            entriesToFlush |= RenderPassDescriptor::Stencil;
        }

        return entriesToFlush;
    }
    //-----------------------------------------------------------------------------------
    uint32 VulkanRenderPassDescriptor::willSwitchTo( VulkanRenderPassDescriptor *newDesc,
                                                     bool warnIfRtvWasFlushed ) const
    {
        uint32 entriesToFlush = 0;

        if( !newDesc ||                                                   //
            this->mSharedFboFlushItor != newDesc->mSharedFboFlushItor ||  //
            this->mInformationOnly || newDesc->mInformationOnly )
        {
            entriesToFlush = RenderPassDescriptor::All;
        }
        else
            entriesToFlush |= checkForClearActions( newDesc );

        if( warnIfRtvWasFlushed )
            newDesc->checkWarnIfRtvWasFlushed( entriesToFlush );

        return entriesToFlush;
    }
    //-----------------------------------------------------------------------------------
    bool VulkanRenderPassDescriptor::cannotInterruptRendering( void ) const
    {
        bool cannotInterrupt = false;

        for( size_t i = 0; i < mNumColourEntries && !cannotInterrupt; ++i )
        {
            if( mColour[i].storeAction != StoreAction::Store &&
                mColour[i].storeAction != StoreAction::StoreAndMultisampleResolve )
            {
                cannotInterrupt = true;
            }
        }

        cannotInterrupt |= ( mDepth.texture && mDepth.storeAction != StoreAction::Store &&
                             mDepth.storeAction != StoreAction::StoreAndMultisampleResolve ) ||
                           ( mStencil.texture && mStencil.storeAction != StoreAction::Store &&
                             mStencil.storeAction != StoreAction::StoreAndMultisampleResolve );

        return cannotInterrupt;
    }
    //-----------------------------------------------------------------------------------
    void VulkanRenderPassDescriptor::performLoadActions( bool renderingWasInterrupted )
    {
        if( mInformationOnly )
            return;

        VkCommandBuffer cmdBuffer = mQueue->mCurrentCmdBuffer;

        const VulkanFrameBufferDescValue &fboDesc = mSharedFboItor->second;

        size_t fboIdx = 0u;
        if( !fboDesc.mWindowImageViews.empty() )
        {
            VulkanTextureGpuWindow *textureVulkan =
                static_cast<VulkanTextureGpuWindow *>( mColour[0].texture );
            fboIdx = textureVulkan->getCurrentSwapchainIdx();

            VkSemaphore semaphore = textureVulkan->getImageAcquiredSemaphore();
            if( semaphore )
            {
                // We cannot start executing color attachment commands until the semaphore says so
                mQueue->addWindowToWaitFor( semaphore );
            }
        }

        VkRenderPassBeginInfo passBeginInfo;
        makeVkStruct( passBeginInfo, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO );
        passBeginInfo.renderPass = fboDesc.mRenderPass;
        passBeginInfo.framebuffer = fboDesc.mFramebuffers[fboIdx];
        passBeginInfo.renderArea.offset.x = 0;
        passBeginInfo.renderArea.offset.y = 0;
        passBeginInfo.renderArea.extent.width = mTargetWidth;
        passBeginInfo.renderArea.extent.height = mTargetHeight;
        passBeginInfo.clearValueCount = sizeof( mClearValues ) / sizeof( mClearValues[0] );
        passBeginInfo.pClearValues = mClearValues;

        if( renderingWasInterrupted )
        {
            TODO_use_render_pass_that_can_load;
            OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, "TODO_use_render_pass_that_can_load",
                         "VulkanRenderPassDescriptor::performLoadActions" );
        }

        vkCmdBeginRenderPass( cmdBuffer, &passBeginInfo, VK_SUBPASS_CONTENTS_INLINE );
    }
    //-----------------------------------------------------------------------------------
    void VulkanRenderPassDescriptor::performStoreActions( bool isInterruptingRendering )
    {
        if( mInformationOnly )
            return;

        if( mQueue->getEncoderState() != VulkanQueue::EncoderGraphicsOpen )
            return;

        vkCmdEndRenderPass( mQueue->mCurrentCmdBuffer );

        if( isInterruptingRendering )
        {
#if OGRE_DEBUG_MODE && OGRE_PLATFORM == OGRE_PLATFORM_LINUX
            // Save the backtrace to report it later
            const bool cannotInterrupt = cannotInterruptRendering();
            static bool warnedOnce = false;
            if( !warnedOnce || cannotInterrupt )
            {
                mNumCallstackEntries = backtrace( mCallstackBacktrace, 32 );
                warnedOnce = true;
            }
#endif
            return;
        }

        // End (if exists) the render command encoder tied to this RenderPassDesc.
        // Another encoder will have to be created, and don't let ours linger
        // since mCurrentRenderPassDescriptor probably doesn't even point to 'this'
        mQueue->endAllEncoders( false );
    }
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    VulkanFrameBufferDescKey::VulkanFrameBufferDescKey() : FrameBufferDescKey() {}
    //-----------------------------------------------------------------------------------
    VulkanFrameBufferDescKey::VulkanFrameBufferDescKey( const RenderPassDescriptor &desc ) :
        FrameBufferDescKey( desc )
    {
        // Base class ignores these. We can't.
        for( size_t i = 0; i < numColourEntries; ++i )
        {
            colour[i].loadAction = desc.mColour[i].loadAction;
            colour[i].storeAction = desc.mColour[i].storeAction;
        }

        depth.loadAction = desc.mDepth.loadAction;
        depth.storeAction = desc.mDepth.storeAction;
        stencil.loadAction = desc.mStencil.loadAction;
        stencil.storeAction = desc.mStencil.storeAction;
    }
    //-----------------------------------------------------------------------------------
    bool VulkanFrameBufferDescKey::operator<( const VulkanFrameBufferDescKey &other ) const
    {
        return FrameBufferDescKey::operator<( other );
    }
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    VulkanFlushOnlyDescValue::VulkanFlushOnlyDescValue() : refCount( 0 ) {}
    //-----------------------------------------------------------------------------------
    VulkanFrameBufferDescValue::VulkanFrameBufferDescValue() :
        refCount( 0u ),
        mNumImageViews( 0u ),
        mRenderPass( 0 )
    {
        memset( mImageViews, 0, sizeof( mImageViews ) );
    }
}  // namespace Ogre
