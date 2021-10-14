/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#include "OgreD3D11TextureManager.h"
#include "OgreD3D11Texture.h"
#include "OgreRoot.h"
#include "OgreLogManager.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreD3D11RenderSystem.h"
#include "OgreD3D11Device.h"

namespace Ogre 
{
    ID3D11SamplerState* D3D11Sampler::getState()
    {
        if(!mDirty)
            return mState.Get();

        D3D11_SAMPLER_DESC  desc;

        desc.Filter = D3D11Mappings::get(mMinFilter, mMagFilter, mMipFilter, mCompareEnabled);
        desc.MaxAnisotropy = mMaxAniso;
        desc.MipLODBias = mMipmapBias;
        desc.MinLOD = 0;
        desc.MaxLOD = mMipFilter == FO_NONE ? 0 : D3D11_FLOAT32_MAX;

        bool reversedZ = Root::getSingleton().getRenderSystem()->isReverseDepthBufferEnabled();
        auto cmpFunc = mCompareFunc;
        if(reversedZ)
            cmpFunc = D3D11RenderSystem::reverseCompareFunction(cmpFunc);
        desc.ComparisonFunc = D3D11Mappings::get(cmpFunc);

        desc.AddressU = D3D11Mappings::get(mAddressMode.u);
        desc.AddressV = D3D11Mappings::get(mAddressMode.v);
        desc.AddressW = D3D11Mappings::get(mAddressMode.w);

        if (mAddressMode.u == TAM_BORDER || mAddressMode.v == TAM_BORDER || mAddressMode.w == TAM_BORDER)
        {
            auto borderColour =
                (reversedZ && mCompareEnabled) ? ColourValue::White - mBorderColour : mBorderColour;
            memcpy(desc.BorderColor, borderColour.ptr(), sizeof(float)*4);
        }

        OGRE_CHECK_DX_ERROR(mDevice->CreateSamplerState(&desc, mState.ReleaseAndGetAddressOf()));

        mDirty = false;

        return mState.Get();
    }
    //---------------------------------------------------------------------
    D3D11TextureManager::D3D11TextureManager( D3D11Device & device ) : TextureManager(), mDevice (device)
    {
        if( mDevice.isNull())
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "Invalid Direct3DDevice passed", "D3D11TextureManager::D3D11TextureManager" );
        // register with group manager
        ResourceGroupManager::getSingleton()._registerResourceManager(mResourceType, this);
    }
    //---------------------------------------------------------------------
    D3D11TextureManager::~D3D11TextureManager()
    {
        // unregister with group manager
        ResourceGroupManager::getSingleton()._unregisterResourceManager(mResourceType);

    }
    //---------------------------------------------------------------------
    Resource* D3D11TextureManager::createImpl(const String& name, 
        ResourceHandle handle, const String& group, bool isManual, 
        ManualResourceLoader* loader, const NameValuePairList* createParams)
    {
        return new D3D11Texture(this, name, handle, group, isManual, loader, mDevice); 
    }
    SamplerPtr D3D11TextureManager::_createSamplerImpl()
    {
        return std::make_shared<D3D11Sampler>(mDevice);
    }
    //---------------------------------------------------------------------
    PixelFormat D3D11TextureManager::getNativeFormat(TextureType ttype, PixelFormat format, int usage)
    {
        // Basic filtering
        DXGI_FORMAT d3dPF = D3D11Mappings::_getPF(D3D11Mappings::_getClosestSupportedPF(format));

        return D3D11Mappings::_getPF(d3dPF);
    }
}
