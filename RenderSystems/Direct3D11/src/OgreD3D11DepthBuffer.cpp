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
#include "OgreD3D11DepthBuffer.h"
#include "OgreD3D11HardwarePixelBuffer.h"
#include "OgreD3D11DepthTexture.h"
#include "OgreD3D11Texture.h"
#include "OgreD3D11Mappings.h"
#include "OgreViewport.h"

namespace Ogre
{
    D3D11DepthBuffer::D3D11DepthBuffer( uint16 poolId, D3D11RenderSystem *renderSystem,
                                        ID3D11Texture2D *depthStencilResource,
                                        ID3D11DepthStencilView *depthBufferView,
                                        ID3D11ShaderResourceView *depthTextureView,
                                        uint32 width, uint32 height, uint32 fsaa,
                                        uint32 multiSampleQuality, PixelFormat pixelFormat,
                                        bool isDepthTexture, bool isManual ) :
                DepthBuffer( poolId, 0, width, height, fsaa, "", pixelFormat,
                             isDepthTexture, isManual, renderSystem ),
                mDepthTextureView( depthTextureView ),
                mMultiSampleQuality( multiSampleQuality ),
                mDepthStencilResource( depthStencilResource )
    {
        mDepthStencilView[0] = depthBufferView;
        mDepthStencilView[1] = 0;

        D3D11_DEPTH_STENCIL_VIEW_DESC pDesc;
        mDepthStencilView[0]->GetDesc( &pDesc );
        // Unknown PixelFormat at the moment
        PixelFormat format = D3D11Mappings::_getPF(pDesc.Format);
		mBitDepth = PixelUtil::getNumElemBytes(format) * 8;
    }

    D3D11DepthBuffer::~D3D11DepthBuffer()
    {
        if( !mManual )
        {
            mDepthStencilView[0]->Release();

            if( mDepthTextureView )
                mDepthTextureView->Release();

            mDepthStencilResource->Release();
        }

        if( mDepthStencilView[1] )
            mDepthStencilView[1]->Release();
        mDepthStencilView[0]    = 0;
        mDepthStencilView[1]    = 0;
        mDepthTextureView       = 0;
        mDepthStencilResource   = 0;
    }
    //---------------------------------------------------------------------
    bool D3D11DepthBuffer::isCompatible( RenderTarget *renderTarget, bool exactFormatMatch ) const
    {
        ID3D11Texture2D *D3D11texture = NULL;
        renderTarget->getCustomAttribute( "First_ID3D11Texture2D", &D3D11texture );
        D3D11_TEXTURE2D_DESC BBDesc;

        ZeroMemory( &BBDesc, sizeof(D3D11_TEXTURE2D_DESC) );
        if( D3D11texture )
        {
            D3D11texture->GetDesc( &BBDesc );
        }
        else
        {
            //Depth textures.
            assert( dynamic_cast<D3D11DepthTextureTarget*>(renderTarget) );
            //BBDesc.ArraySize = renderTarget;
            BBDesc.SampleDesc.Count     = std::max( 1u, renderTarget->getFSAA() );
            BBDesc.SampleDesc.Quality   = atoi( renderTarget->getFSAAHint().c_str() );
        }

        //RenderSystem will determine if bitdepths match (i.e. 32 bit RT don't like 16 bit Depth)
        //This is the same function used to create them. Note results are usually cached so this should
        //be quick

        if( mFsaa == BBDesc.SampleDesc.Count &&
            mMultiSampleQuality == BBDesc.SampleDesc.Quality &&
            this->getWidth() == renderTarget->getWidth() &&
            this->getHeight() == renderTarget->getHeight() &&
            mDepthTexture == renderTarget->prefersDepthTexture() &&
            ((!exactFormatMatch && mFormat == PF_D24_UNORM_S8_UINT) ||
             mFormat == renderTarget->getDesiredDepthBufferFormat()) )
        {
            return true;
        }

        return false;
    }
    //---------------------------------------------------------------------
    bool D3D11DepthBuffer::copyToImpl( DepthBuffer *_destination )
    {
        bool retVal = false;
        D3D11RenderSystem *renderSystem = static_cast<D3D11RenderSystem*>( mRenderSystem );

        if( renderSystem->_getFeatureLevel() >= D3D_FEATURE_LEVEL_10_1 )
        {
            D3D11DepthBuffer *destination   = static_cast<D3D11DepthBuffer*>( _destination );

            D3D11Device &device = renderSystem->_getDevice();
            ID3D11DeviceContextN *deviceContext = device.GetImmediateContext();

            deviceContext->CopyResource( destination->mDepthStencilResource,
                                         this->mDepthStencilResource );

            retVal = true;
        }

        return retVal;
    }
    //---------------------------------------------------------------------
    void D3D11DepthBuffer::createReadOnlySRV(void)
    {
        if( mDepthStencilView[1] )
        {
            mDepthStencilView[1]->Release();
            mDepthStencilView[1] = 0;
        }

        D3D11_DEPTH_STENCIL_VIEW_DESC pDesc;
        mDepthStencilView[0]->GetDesc( &pDesc );
        pDesc.Flags = D3D11_DSV_READ_ONLY_DEPTH;

        if( pDesc.Format == DXGI_FORMAT_D24_UNORM_S8_UINT ||
            pDesc.Format == DXGI_FORMAT_D32_FLOAT_S8X24_UINT )
        {
            pDesc.Flags |= D3D11_DSV_READ_ONLY_STENCIL;
        }

        D3D11RenderSystem *renderSystem = static_cast<D3D11RenderSystem*>( mRenderSystem );
        D3D11Device &device = renderSystem->_getDevice();

        HRESULT hr;
        hr = device->CreateDepthStencilView( mDepthStencilResource, &pDesc, &mDepthStencilView[1] );
        if( FAILED(hr) )
        {
            String errorDescription = device.getErrorDescription(hr);
            OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
                "Unable to create depth stencil view\nError Description:" + errorDescription,
                "D3D11DepthBuffer::createReadOnlySRV");
        }
    }
    //---------------------------------------------------------------------
    ID3D11DepthStencilView* D3D11DepthBuffer::getDepthStencilView( uint8 viewportRenderTargetFlags )
    {
        if( viewportRenderTargetFlags & (VP_RTT_READ_ONLY_DEPTH|VP_RTT_READ_ONLY_STENCIL) )
        {
            if( !mDepthStencilView[1] )
                createReadOnlySRV();
            return mDepthStencilView[1];
        }

        return mDepthStencilView[0];
    }
    //---------------------------------------------------------------------
    ID3D11ShaderResourceView* D3D11DepthBuffer::getDepthTextureView() const
    {
        return mDepthTextureView;
    }
    //---------------------------------------------------------------------
    void D3D11DepthBuffer::_resized(ID3D11DepthStencilView *depthBufferView, uint32 width, uint32 height)
    {
        mHeight = height;
        mWidth = width;

        mDepthStencilView[0] = depthBufferView;
        if( mDepthStencilView[1] )
            createReadOnlySRV();
    }
}
