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
#include "OgreD3D11Texture.h"
#include "OgreD3D11Mappings.h"
#include "OgreD3D11RenderWindow.h"

namespace Ogre
{
    D3D11DepthBuffer::D3D11DepthBuffer( uint16 poolId 
                                        , D3D11RenderSystem *renderSystem
                                        , RenderTarget *renderTarget
                                        , bool dontBindToShader
                                        , uint32 width 
                                        , uint32 height 
                                        , uint32 depth
                                        , uint32 fsaa
                                        , uint32 multiSampleQuality
                                        , bool isManual ) :
                                         DepthBuffer(poolId, 0, width, height, depth, fsaa, "", isManual)
                                        , mMultiSampleQuality( multiSampleQuality)
                                        , mRenderSystem(renderSystem)
                                        , mDontBindToShader(dontBindToShader)
                                        , mDepthStencilView(NULL)
                                        , mDepthStencilTexture(NULL)
    {
        init(renderTarget);
    }
//-----------------------------------------------------------------------
    D3D11DepthBuffer::D3D11DepthBuffer(uint16 poolId
        , D3D11RenderSystem *renderSystem
        , RenderTarget *renderTarget
        , bool dontBindToShader 
        , bool isManual) :
        DepthBuffer(poolId, 0, /*width*/-1, /*height*/-1, /*depth*/-1, /*fsaa*/-1, "", isManual)
        , mRenderSystem(renderSystem)
        , mMultiSampleQuality(-1)
        , mDontBindToShader(dontBindToShader)
        , mDepthStencilView(NULL)
        , mDepthStencilTexture(NULL)
        , mDepthStencilShaderResouceView(NULL)
    {
        init(renderTarget);
    }
//-----------------------------------------------------------------------
    void D3D11DepthBuffer::init(RenderTarget* renderTarget)
    {
        mDepthStencilView = NULL;
        mDepthStencilTexture = NULL;
        mDepthStencilShaderResouceView = NULL;
        createDepthStencilTexture(renderTarget);
        updateDepthStencilView(renderTarget);
    }
//-----------------------------------------------------------------------
    
    
    D3D11DepthBuffer::~D3D11DepthBuffer()
    {
        SAFE_RELEASE(mDepthStencilShaderResouceView);
        SAFE_RELEASE(mDepthStencilView);
        SAFE_RELEASE(mDepthStencilTexture);
        
    }
//-----------------------------------------------------------------------
    ID3D11ShaderResourceView* D3D11DepthBuffer::createShaderResourceView()
    {
        //
        // Create the View of the texture
        // If MSAA is used, we cannot do this
        //
        if (mDepthStencilShaderResouceView != NULL)
        {
            SAFE_RELEASE(mDepthStencilShaderResouceView);
        }

        if (mUseAsShaderResource)
        {
            D3D11Device &device = mRenderSystem->_getDevice();

            D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
            viewDesc.Format = DXGI_FORMAT_R32_FLOAT;
            viewDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
            viewDesc.Texture2D.MostDetailedMip = 0;
            viewDesc.Texture2D.MipLevels = 1;
            
            HRESULT hr = device->CreateShaderResourceView(mDepthStencilTexture, &viewDesc, &mDepthStencilShaderResouceView);
            if (FAILED(hr) || device.isError())
            {
                String errorDescription = device.getErrorDescription(hr);
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                    "Unable to create the view of the depth texture \nError Description:" + errorDescription,
                    "D3D11RenderSystem::_createDepthBufferFor");
            }
        }

        return mDepthStencilShaderResouceView;
    }
//-----------------------------------------------------------------------

    ID3D11ShaderResourceView* D3D11DepthBuffer::getShaderResourceView()
    {
        if (mDepthStencilShaderResouceView == NULL)
            createShaderResourceView();
        return mDepthStencilShaderResouceView;
    }
//-----------------------------------------------------------------------
    DXGI_FORMAT D3D11DepthBuffer::getFormatFor(RenderTarget* renderTarget) const
    {
        DXGI_FORMAT result = DXGI_FORMAT_R32_TYPELESS;

        if (false
            || renderTarget->getIsStencilBufferRequired()
            || dynamic_cast<D3D11RenderWindowBase*>(renderTarget) != NULL
            || mRenderSystem->_getFeatureLevel() < D3D_FEATURE_LEVEL_10_0)
        {
            result = DXGI_FORMAT_D24_UNORM_S8_UINT;;
        }
        
        return result;
    }
//-----------------------------------------------------------------------

	ID3D11Texture2D* D3D11DepthBuffer::getTextureFor(RenderTarget* renderTarget) const
	{
		ID3D11Texture2D* parentTexture = NULL;

		bool isTexture = false;
		renderTarget->getCustomAttribute("isTexture", &isTexture);

		if (isTexture)
		{
			
			D3D11HardwarePixelBuffer *pBuffer;
			renderTarget->getCustomAttribute("BUFFER", &pBuffer);
			parentTexture = static_cast<ID3D11Texture2D*>(pBuffer->getParentTexture()->getTextureResource());
		}
		else
		{
			ID3D11Texture2D* pBack[OGRE_MAX_MULTIPLE_RENDER_TARGETS];
			memset(pBack, 0, sizeof(pBack));
			
				
				renderTarget->getCustomAttribute("DDBACKBUFFER", &pBack);
				parentTexture = pBack[0];
				
				if (parentTexture == NULL)
					renderTarget->getCustomAttribute("ID3D11Texture2D", &parentTexture);
		}

	
		// If not found raise an exception
		if (parentTexture == NULL)
		{
			OGRE_EXCEPT(Exception::ERR_INVALID_STATE,
				"Could not find underlying D3D11 texture for render target",
				"D3D11DepthBuffer::getTextureFor");
		}

		return parentTexture;
	}

	
	
	

    void D3D11DepthBuffer::createDepthStencilTexture(RenderTarget* renderTarget)
    {

        DXGI_FORMAT format = getFormatFor(renderTarget);

        bool useStencil = format == DXGI_FORMAT_D24_UNORM_S8_UINT;

        D3D11_TEXTURE2D_DESC BBDesc;
		getTextureDescpritionFor(renderTarget,BBDesc);
		
        
        mis2DArrayTextureCube = (BBDesc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE) == D3D11_RESOURCE_MISC_TEXTURECUBE;

        //check that we actually can use ShaderResoruceView
        mUseAsShaderResource = true
            && !mDontBindToShader
            && mRenderSystem->_getFeatureLevel() >= D3D_FEATURE_LEVEL_10_0
            && BBDesc.SampleDesc.Count == 1
            && !useStencil;
        
        D3D11Device &device = mRenderSystem->_getDevice();


        D3D11_TEXTURE2D_DESC descDepth;

        descDepth.Width = renderTarget->getWidth();
        descDepth.Height = renderTarget->getHeight();
        descDepth.MipLevels = 1;
        descDepth.ArraySize = BBDesc.ArraySize;
        descDepth.Format = format;
    
        descDepth.SampleDesc.Count = BBDesc.SampleDesc.Count;
        descDepth.SampleDesc.Quality = BBDesc.SampleDesc.Quality;
        descDepth.Usage = D3D11_USAGE_DEFAULT;
        descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;

        // If we tell we want to use it as a Shader Resource when in MSAA, we will fail
        // This is a recommendation from NVidia.
        if (mUseAsShaderResource)
            descDepth.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

        descDepth.CPUAccessFlags = 0;
        descDepth.MiscFlags = 0;

        if (mis2DArrayTextureCube)
        {
            descDepth.MiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
        }

        SAFE_RELEASE(mDepthStencilTexture);
        HRESULT hr = device->CreateTexture2D(&descDepth, NULL, &mDepthStencilTexture);
        if (FAILED(hr) || device.isError())
        {
            String errorDescription = device.getErrorDescription(hr);
            OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
                "Unable to create depth texture\nError Description:" + errorDescription,
                "D3D11RenderSystem::_createDepthBufferFor");
        }

        //Update parent fields
        mWidth = descDepth.Width;
        mHeight = descDepth.Height;
        mDepth = descDepth.ArraySize;
        mFsaa = descDepth.SampleDesc.Count;
        
        mMultiSampleQuality = descDepth.SampleDesc.Quality;
    }

//-----------------------------------------------------------------------
    void D3D11DepthBuffer::updateDepthStencilView(RenderTarget* renderTarget)
    {
        D3D11RenderTexture* renderTexture = dynamic_cast<D3D11RenderTexture*>(renderTarget);
        D3D11RenderWindowBase* d3dwindow = dynamic_cast<D3D11RenderWindowBase*>(renderTarget);
		MultiRenderTarget* multiRenderTarget = dynamic_cast<MultiRenderTarget*>(renderTarget);

        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
        ZeroMemory(&descDSV, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
        
        if (d3dwindow != NULL)
        {
            D3D11_TEXTURE2D_DESC BBDesc;
            mDepthStencilTexture->GetDesc(&BBDesc);

            descDSV.Format = BBDesc.Format;
            descDSV.ViewDimension = d3dwindow->getFSAA() ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
            descDSV.Texture2D.MipSlice = 0;
        }
		else if (renderTexture != NULL || multiRenderTarget != NULL)
        {
            //create depth buffer for render texture
            D3D11HardwarePixelBuffer *pBuffer;
            renderTarget->getCustomAttribute("BUFFER", &pBuffer);

            static_cast<D3D11Texture*>(pBuffer->getParentTexture())->fillDSVDescription(mDepthStencilTexture, descDSV);

            if (descDSV.ViewDimension == D3D11_DSV_DIMENSION_TEXTURE2DARRAY)
            {
                if (mis2DArrayTextureCube)
                {
                    descDSV.Texture2DArray.FirstArraySlice = pBuffer->getFace();
                }
            }
            descDSV.Flags = 0 /* D3D11_DSV_READ_ONLY_DEPTH | D3D11_DSV_READ_ONLY_STENCIL */;    // TODO: Allows bind depth buffer as depth view AND texture simultaneously.

        }
        else
        {
            OGRE_EXCEPT(Exception::ERR_INVALID_STATE, "depth buffer are valid only for render texture and render windows",
                "D3D11DepthBuffer::updateDepthStencilView");
        }


        SAFE_RELEASE(mDepthStencilView);
        HRESULT hr = mRenderSystem->_getDevice()->CreateDepthStencilView(mDepthStencilTexture, &descDSV, &mDepthStencilView);
        
        if (FAILED(hr))
        {
            String errorDescription = mRenderSystem->_getDevice().getErrorDescription(hr);
            OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
                "Unable to create depth stencil view\nError Description:" + errorDescription,
                "D3D11RenderSystem::_createDepthBufferFor");
        }
        
        mBitDepth = PixelUtil::getNumElemBytes(D3D11Mappings::_getPF(descDSV.Format)) * 8;
    }
    //---------------------------------------------------------------------
    bool D3D11DepthBuffer::isCompatible( RenderTarget *renderTarget ) const
    {

		D3D11_TEXTURE2D_DESC BBDesc;
		getTextureDescpritionFor(renderTarget, BBDesc);
        
        D3D11_TEXTURE2D_DESC DepthDesc;
        mDepthStencilTexture->GetDesc(&DepthDesc);
        
        if (mFsaa == BBDesc.SampleDesc.Count &&
            mMultiSampleQuality == BBDesc.SampleDesc.Quality &&
            this->getFormatFor(renderTarget) == DepthDesc.Format &&
            this->getWidth() == renderTarget->getWidth() &&
            this->getHeight() == renderTarget->getHeight()  && 
            this->getDepth() == renderTarget->getDepth()
            )
        {
            return true;
        }

        return false;
    }
    //---------------------------------------------------------------------
    ID3D11DepthStencilView* D3D11DepthBuffer::getDepthStencilView() const
    {
        return mDepthStencilView;
    }
    
	//---------------------------------------------------------------------
	void D3D11DepthBuffer::getTextureDescpritionFor(RenderTarget * renderTarget, D3D11_TEXTURE2D_DESC& BBDesc) const
	{
		getTextureFor(renderTarget)->GetDesc(&BBDesc);
	}

    //---------------------------------------------------------------------
}
