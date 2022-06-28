/*
  -----------------------------------------------------------------------------
  This source file is part of OGRE
  (Object-oriented Graphics Rendering Engine)
  For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2016 Torus Knot Software Ltd

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

#include "OgreMetalTextureManager.h"
#include "OgreMetalTexture.h"
#include "OgreMetalMappings.h"
#include "OgreMetalDevice.h"

namespace Ogre
{
    id <MTLSamplerState> MetalSampler::getState()
    {
        if(!mDirty)
            return mSampler;

        MTLSamplerDescriptor *samplerDescriptor = [MTLSamplerDescriptor new];
        samplerDescriptor.minFilter = MetalMappings::get( mMinFilter );
        samplerDescriptor.magFilter = MetalMappings::get( mMagFilter );
        samplerDescriptor.mipFilter = MetalMappings::getMipFilter( mMipFilter );
        samplerDescriptor.maxAnisotropy = mMaxAniso;
        samplerDescriptor.sAddressMode  = MetalMappings::get( mAddressMode.u );
        samplerDescriptor.tAddressMode  = MetalMappings::get( mAddressMode.v );
        samplerDescriptor.rAddressMode  = MetalMappings::get( mAddressMode.w );
        samplerDescriptor.normalizedCoordinates = YES;
        //samplerDescriptor.lodMinClamp   = newBlock->mMinLod;
        //samplerDescriptor.lodMaxClamp   = newBlock->mMaxLod;

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        const bool supportsCompareFunction =
                [mDevice->mDevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily3_v1];
#else
        const bool supportsCompareFunction = true;
#endif

        if( supportsCompareFunction && mCompareEnabled )
        {
            samplerDescriptor.compareFunction = MetalMappings::get( mCompareFunc );
        }

        mSampler = [mDevice->mDevice newSamplerStateWithDescriptor:samplerDescriptor];

        mDirty = false;

        return mSampler;
    }

    MetalTextureManager::MetalTextureManager( MetalDevice *device ) :
        TextureManager(),
        mDevice( device )
    {
        ResourceGroupManager::getSingleton()._registerResourceManager(mResourceType, this);
    }

    MetalTextureManager::~MetalTextureManager()
    {
        // unregister with group manager
        ResourceGroupManager::getSingleton()._unregisterResourceManager(mResourceType);
    }

    Resource* MetalTextureManager::createImpl(const String& name,
        ResourceHandle handle, const String& group, bool isManual,
        ManualResourceLoader* loader, const NameValuePairList* createParams)
    {
        return new MetalTexture( this, name, handle, group, isManual, loader, mDevice );
    }

    SamplerPtr MetalTextureManager::_createSamplerImpl()
    {
        return std::make_shared<MetalSampler>(mDevice);
    }

    PixelFormat MetalTextureManager::getNativeFormat(TextureType ttype, PixelFormat format, int usage)
    {
        if( format == PF_R8G8B8 )
            return PF_X8R8G8B8;
        if( format == PF_B8G8R8 )
            return PF_X8B8G8R8;

        if(MetalMappings::getPixelFormat( format, false ) != MTLPixelFormatInvalid)
            return format;

        return PF_BYTE_RGBA;
    }
}
