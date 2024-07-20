/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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

#include "OgreMetalTexture.h"
#include "OgreMetalRenderTexture.h"
#include "OgreMetalMappings.h"
#include "OgreMetalDevice.h"
#include "OgreMetalRenderSystem.h"
#include "OgreTextureManager.h"
#include "OgreStringConverter.h"
#include "OgreMetalDepthBuffer.h"
#include "OgreBitwise.h"

#import "Metal/MTLBlitCommandEncoder.h"

namespace Ogre
{
    MetalTexture::MetalTexture( ResourceManager* creator, const String& name, ResourceHandle handle,
                                const String& group, bool isManual, ManualResourceLoader* loader,
                                MetalDevice *device ) :
        Texture(creator, name, handle, group, isManual, loader),
        mTexture( 0 ),
        mDevice( device )
    {
        mMipmapsHardwareGenerated = true;
    }
    //-----------------------------------------------------------------------------------
    MetalTexture::~MetalTexture()
    {
        // have to call this here rather than in Resource destructor
        // since calling virtual methods in base destructors causes crash
        unload();
    }
    //-----------------------------------------------------------------------------------
    MTLTextureType MetalTexture::getMetalTextureTarget(void) const
    {
        switch( mTextureType )
        {
            case TEX_TYPE_1D:
                return MTLTextureType1D;
            case TEX_TYPE_2D:
                return MTLTextureType2D;
            case TEX_TYPE_CUBE_MAP:
                return MTLTextureTypeCube;
            case TEX_TYPE_3D:
                return MTLTextureType3D;
            case TEX_TYPE_2D_ARRAY:
                return MTLTextureType2DArray;
            default:
                return (MTLTextureType)999;
        };
    }
    //-----------------------------------------------------------------------------------
    void MetalTexture::createMetalTexResource(void)
    {
        // Adjust format if required
        mFormat = TextureManager::getSingleton().getNativeFormat( mTextureType, mFormat, mUsage );
        const MTLTextureType texTarget = getMetalTextureTarget();

        if(mTextureType == TEX_TYPE_1D )
            mNumMipmaps = mNumRequestedMipmaps = 0;

        // If we can do automip generation and the user desires this, do so
        mMipmapsHardwareGenerated = !PixelUtil::isCompressed( mFormat );

        MTLTextureDescriptor *desc = [MTLTextureDescriptor new];
        desc.mipmapLevelCount   = mNumMipmaps + 1u;
        desc.textureType        = texTarget;
        desc.width              = mWidth;
        desc.height             = mHeight;
        if( mTextureType != TEX_TYPE_CUBE_MAP )
        {
                desc.depth      = mTextureType == TEX_TYPE_2D_ARRAY ? 1u : mDepth;
                desc.arrayLength= mTextureType == TEX_TYPE_2D_ARRAY ? mDepth : 1u;
        }
        else
        {
                desc.depth      = 1u;
                desc.arrayLength= 1u;
        }
        desc.pixelFormat        = MetalMappings::getPixelFormat( mFormat, mHwGamma );
        desc.sampleCount        = 1u;

        desc.usage = MTLTextureUsageShaderRead;
        if( mUsage & TU_RENDERTARGET )
            desc.usage |= MTLTextureUsageRenderTarget;
        if( mUsage & TU_UNORDERED_ACCESS )
            desc.usage |= MTLTextureUsageShaderWrite;

        if( mUsage & (TU_UNORDERED_ACCESS|TU_NOT_SAMPLED|TU_RENDERTARGET) )
            desc.storageMode = MTLStorageModePrivate;

        mTexture = [mDevice->mDevice newTextureWithDescriptor:desc];
        mTexture.label = [NSString stringWithUTF8String:mName.c_str()];

        if( mFSAA > 1u )
        {
            desc.textureType    = MTLTextureType2DMultisample;
            desc.depth          = 1u;
            desc.arrayLength    = 1u;
            desc.sampleCount    = mFSAA;
            mMsaaTexture = [mDevice->mDevice newTextureWithDescriptor:desc];
            mMsaaTexture.label = [NSString stringWithUTF8String:(mName + "_MSAA").c_str()];
        }
    }
    //-----------------------------------------------------------------------------------
    // Creation / loading methods
    void MetalTexture::createInternalResourcesImpl(void)
    {
        if( mFSAA < 1u )
            mFSAA = 1u;

        createMetalTexResource();
        createSurfaceList();

        if( (mUsage & TU_AUTOMIPMAP) &&
            mNumMipmaps && mMipmapsHardwareGenerated )
        {
            __unsafe_unretained id<MTLBlitCommandEncoder> blitEncoder = mDevice->getBlitEncoder();
            [blitEncoder generateMipmapsForTexture:mTexture];
        }
    }
    void MetalTexture::freeInternalResourcesImpl(void)
    {
        mTexture = 0;
    }
    //-----------------------------------------------------------------------------------
    void MetalTexture::createSurfaceList(void)
    {
        mSurfaceList.clear();

        __unsafe_unretained id<MTLTexture> renderTexture = mTexture;
        __unsafe_unretained id<MTLTexture> resolveTexture = 0;

        if( mMsaaTexture )
        {
            renderTexture   = mMsaaTexture;
            resolveTexture  = mTexture;
        }

        for (size_t face = 0; face < getNumFaces(); face++)
        {
            uint32 width = mWidth;
            uint32 height = mHeight;
            uint32 depth = mDepth;

            for (uint8 mip = 0; mip <= getNumMipmaps(); mip++)
            {
                MetalHardwarePixelBuffer *buf = OGRE_NEW MetalTextureBuffer(
                            renderTexture, resolveTexture, mDevice, mName,
                            getMetalTextureTarget(), width, height, depth, mFormat,
                            static_cast<int32>(face), mip,
                            static_cast<HardwareBuffer::Usage>(mUsage),
                            mHwGamma, mFSAA );

                mSurfaceList.push_back( HardwarePixelBufferSharedPtr(buf) );
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void MetalTexture::_autogenerateMipmaps(void)
    {
        __unsafe_unretained id<MTLBlitCommandEncoder> blitEncoder = mDevice->getBlitEncoder();
        [blitEncoder generateMipmapsForTexture: mTexture];

        // mSurfaceList[0]->getRenderTarget()->_setMipmapsUpdated();
    }
    //-----------------------------------------------------------------------------------
    id<MTLTexture> MetalTexture::getTextureForSampling( MetalRenderSystem *renderSystem )
    {
        __unsafe_unretained id<MTLTexture> retVal = mTexture;

        if( mUsage & TU_RENDERTARGET )
        {
        #if OGRE_DEBUG_MODE
            RenderTarget *renderTarget = mSurfaceList[0]->getRenderTarget();
            assert( dynamic_cast<MetalRenderTexture*>( renderTarget ) );
            if( PixelUtil::isDepth( renderTarget->suggestPixelFormat() ) )
            {
                MetalDepthBuffer *depthBuffer = static_cast<MetalDepthBuffer*>(
                            renderTarget->getDepthBuffer() );
                assert( depthBuffer->mDepthAttachmentDesc.loadAction != MTLLoadActionClear );
            }
            else
            {
                MetalRenderTexture *renderTexture = static_cast<MetalRenderTexture*>( renderTarget );
                assert( renderTexture->mColourAttachmentDesc.loadAction != MTLLoadActionClear );
            }
        #endif

            if( mFSAA > 1u )
            {
                //RenderTarget *renderTarget = mSurfaceList[0]->getRenderTarget();
                //In metal, implicitly resolved textures are immediately resolved
                //via MTLStoreActionMultisampleResolve, so we don't need to do
                //anything for implicit resolves (it's already resolved and the
                //contents of the msaa texture are left undefined).
//                if( !mFsaaExplicitResolve )
//                {
//                    for( size_t face=0; face<getNumFaces(); ++face )
//                    {
//                        renderTarget = mSurfaceList[face * (mNumMipmaps+1)]->getRenderTarget();
//                        if( renderTarget->isFsaaResolveDirty() )
//                            renderTarget->swapBuffers();
//                    }
//                }
//                else if( renderTarget->isFsaaResolveDirty() )
                //if( !mFsaaExplicitResolve && renderTarget->isFsaaResolveDirty() )
                {
                    //Explicit resolves. Only use the
                    //Fsaa texture before it has been resolved
                    retVal = mMsaaTexture;
                }
            }

            if( (mUsage & (TU_AUTOMIPMAP|TU_RENDERTARGET)) ==
                    (TU_AUTOMIPMAP|TU_RENDERTARGET) )
            {
                // RenderTarget *renderTarget = mSurfaceList[0]->getRenderTarget();
                // if( renderTarget->isMipmapsDirty() )
                    this->_autogenerateMipmaps();
            }
        }

        return retVal;
    }
}
