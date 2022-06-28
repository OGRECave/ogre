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
#include "OgreD3D11Texture.h"
#include "OgreD3D11HardwarePixelBuffer.h"
#include "OgreD3D11Mappings.h"
#include "OgreD3D11Device.h"
#include "OgreD3D11RenderSystem.h"
#include "OgreD3D11DepthBuffer.h"
#include "OgreRoot.h"
#include "OgreLogManager.h"
#include "OgreException.h"

namespace Ogre 
{
    //---------------------------------------------------------------------
    D3D11Texture::D3D11Texture(ResourceManager* creator, const String& name, 
        ResourceHandle handle, const String& group, bool isManual, 
        ManualResourceLoader* loader, D3D11Device & device)
        :Texture(creator, name, handle, group, isManual, loader),
        mDevice(device), 
        mAutoMipMapGeneration(false)
    {
        mFSAAType.Count = 1;
        mFSAAType.Quality = 0;
    }
    //---------------------------------------------------------------------
    D3D11Texture::~D3D11Texture()
    {
        // have to call this here reather than in Resource destructor
        // since calling virtual methods in base destructors causes crash
        if (isLoaded())
        {
            unload(); 
        }
        else
        {
            freeInternalResources();
        }
    }
    //---------------------------------------------------------------------
    void D3D11Texture::notifyDeviceLost(D3D11Device* device)
    {
        unloadImpl();
    }
    //---------------------------------------------------------------------
    void D3D11Texture::notifyDeviceRestored(D3D11Device* device)
    {
        if(isManuallyLoaded())
        {
            preLoadImpl();
            createInternalResourcesImpl();
            if (mLoader != NULL)
                mLoader->loadResource(this);
            postLoadImpl();
        }
        else
        {
            preLoadImpl();
            loadImpl();
            postLoadImpl();
        }
    }
    //---------------------------------------------------------------------
    void D3D11Texture::copyToTexture(TexturePtr& target)
    {
        // check if this & target are the same format and type
        // blitting from or to cube textures is not supported yet
        if (target->getUsage() != this->getUsage() ||
            target->getTextureType() != this->getTextureType())
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, 
                "Src. and dest. textures must be of same type and must have the same usage !!!", 
                "D3D11Texture::copyToTexture" );
        }


        D3D11Texture *other;
        // get the target
        other = static_cast< D3D11Texture * >( target.get() );

        mDevice.GetImmediateContext()->CopyResource(other->getTextureResource(), mpTex.Get());
        if (mDevice.isError())
        {
            String errorDescription = mDevice.getErrorDescription();
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                "D3D11 device cannot copy resource\nError Description:" + errorDescription,
                "D3D11Texture::copyToTexture");
        }

    }
    //---------------------------------------------------------------------
    void D3D11Texture::loadImpl()
    {
        Texture::loadImpl();

        if (mUsage & TU_RENDERTARGET)
        {
            return;
        }

        _setSrcAttributes(mWidth, mHeight, mDepth, mFormat);
    }
    //---------------------------------------------------------------------
    void D3D11Texture::freeInternalResourcesImpl()
    {
        mpTex.Reset();
        mpShaderResourceView.Reset();
        mp1DTex.Reset();
        mp2DTex.Reset();
        mp3DTex.Reset();
    }

    //---------------------------------------------------------------------
    void D3D11Texture::createInternalResourcesImpl(void)
    {
        // If mSrcWidth and mSrcHeight are zero, the requested extents have probably been set
        // through setWidth and setHeight, which set mWidth and mHeight. Take those values.
        if(mSrcWidth == 0 || mSrcHeight == 0) {
            mSrcWidth = mWidth;
            mSrcHeight = mHeight;
        }

        mFormat = D3D11Mappings::_getClosestSupportedPF(mFormat);

        // Choose closest supported D3D format
        mD3DFormat = D3D11Mappings::_getGammaFormat(D3D11Mappings::_getPF(mFormat), isHardwareGammaEnabled());

        mFSAAType.Count = 1;
        mFSAAType.Quality = 0;
        if((mUsage & TU_RENDERTARGET) != 0 && (mUsage & TU_DYNAMIC) == 0)
        {
            D3D11RenderSystem* rsys = static_cast<D3D11RenderSystem*>(Root::getSingleton().getRenderSystem());
            // http://msdn.microsoft.com/en-us/library/windows/desktop/ff476150%28v=vs.85%29.aspx#ID3D11Device_CreateTexture2D
            // 10Level9, When using D3D11_BIND_SHADER_RESOURCE, SampleDesc.Count must be 1.
            if(rsys->_getFeatureLevel() >= D3D_FEATURE_LEVEL_10_0 || (mUsage & TU_NOT_SRV))
                rsys->determineFSAASettings(mFSAA, mFSAAHint, mD3DFormat, &mFSAAType);
        }

        // load based on tex.type
        switch (this->getTextureType())
        {
        case TEX_TYPE_1D:
            {
                D3D11RenderSystem* rs = (D3D11RenderSystem*)Root::getSingleton().getRenderSystem();
                if(rs->_getFeatureLevel() >= D3D_FEATURE_LEVEL_10_0)
                {
                    this->_create1DTex();
                    break; // For Feature levels that do not support 1D textures, revert to creating a 2D texture.
                }
            }
        case TEX_TYPE_2D:
        case TEX_TYPE_CUBE_MAP:
        case TEX_TYPE_2D_ARRAY:
            this->_create2DTex();
            break;
        case TEX_TYPE_3D:
            this->_create3DTex();
            break;
        default:
            this->freeInternalResources();
            OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, "Unknown texture type", "D3D11Texture::createInternalResources" );
        }
    }
    //---------------------------------------------------------------------
    void D3D11Texture::_create1DTex()
    {
        // we must have those defined here
        assert(mSrcWidth > 0 || mSrcHeight > 0);

        // determine total number of mipmaps including main one (d3d11 convention)
        UINT numMips = (mNumMipmaps == MIP_UNLIMITED || (1U << mNumMipmaps) > mSrcWidth) ? 0 : mNumMipmaps + 1;

        D3D11_TEXTURE1D_DESC desc;
        desc.Width          = static_cast<UINT>(mSrcWidth);
        desc.MipLevels      = numMips;
        desc.ArraySize      = 1;
        desc.Format         = mD3DFormat;
		desc.Usage			= D3D11Mappings::_getUsage(_getTextureUsage());
		desc.BindFlags		= D3D11Mappings::_getTextureBindFlags(mD3DFormat, _getTextureUsage());
		desc.CPUAccessFlags = D3D11Mappings::_getAccessFlags(_getTextureUsage());
        desc.MiscFlags      = D3D11Mappings::_getTextureMiscFlags(desc.BindFlags, getTextureType(), _getTextureUsage());

        // create the texture
        HRESULT hr = mDevice->CreateTexture1D(  
            &desc,
            NULL,
            mp1DTex.ReleaseAndGetAddressOf());                      // data pointer
        // check result and except if failed
        if (FAILED(hr) || mDevice.isError())
        {
            this->freeInternalResources();
			String errorDescription = mDevice.getErrorDescription(hr);
			OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
				"Error creating texture\nError Description:" + errorDescription,
                "D3D11Texture::_create1DTex" );
        }

        _queryInterface<ID3D11Texture1D, ID3D11Resource>(mp1DTex, &mpTex);
        _create1DResourceView();
    }
    //---------------------------------------------------------------------
    void D3D11Texture::_create1DResourceView()
    {
        // set final tex. attributes from tex. description
        // they may differ from the source image !!!
        HRESULT hr;
        D3D11_TEXTURE1D_DESC desc;

        // set final tex. attributes from tex. description
        // they may differ from the source image !!!
        mp1DTex->GetDesc(&desc);
        mNumMipmaps = desc.MipLevels - 1;

        ZeroMemory( &mSRVDesc, sizeof(mSRVDesc) );
        mSRVDesc.Format = desc.Format;
        mSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
        mSRVDesc.Texture1D.MipLevels = desc.MipLevels;
        OGRE_CHECK_DX_ERROR(
            (desc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
                ? mDevice->CreateShaderResourceView(mp1DTex.Get(), &mSRVDesc,
                                                    mpShaderResourceView.ReleaseAndGetAddressOf())
                : S_FALSE);

        this->_setFinalAttributes(desc.Width, 1, 1, D3D11Mappings::_getPF(desc.Format), desc.MiscFlags);
    }
    //---------------------------------------------------------------------
    inline bool IsPowerOfTwo(unsigned int n)                { return ((n&(n-1))==0);                    }
    //---------------------------------------------------------------------
    void D3D11Texture::_create2DTex()
    {
        // we must have those defined here
        assert(mSrcWidth > 0 || mSrcHeight > 0);

        // determine total number of mipmaps including main one (d3d11 convention)
        UINT numMips = (mNumMipmaps == MIP_UNLIMITED || (1U << mNumMipmaps) > std::max(mSrcWidth, mSrcHeight)) ? 0 : mNumMipmaps + 1;
        if(D3D11Mappings::_isBinaryCompressedFormat(mD3DFormat) && numMips > 1)
            numMips = std::max(1U, numMips - 2);

        D3D11_TEXTURE2D_DESC desc;
        desc.Width          = static_cast<UINT>(mSrcWidth);
        desc.Height         = static_cast<UINT>(mSrcHeight);
        desc.MipLevels      = numMips;
        desc.ArraySize      = mDepth == 0 ? 1 : mDepth;
        desc.Format         = mD3DFormat;
        desc.SampleDesc     = mFSAAType;
        desc.Usage          = D3D11Mappings::_getUsage(_getTextureUsage());
        desc.BindFlags      = D3D11Mappings::_getTextureBindFlags(mD3DFormat, _getTextureUsage());
        desc.CPUAccessFlags = D3D11Mappings::_getAccessFlags(_getTextureUsage());
        desc.MiscFlags      = D3D11Mappings::_getTextureMiscFlags(desc.BindFlags, getTextureType(), _getTextureUsage());

        if (PixelUtil::isDepth(mFormat))
        {
            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.CPUAccessFlags        = 0;
            desc.MiscFlags             = 0;
        }

        if (this->getTextureType() == TEX_TYPE_CUBE_MAP)
        {
                desc.ArraySize          = 6;
        }

        D3D11RenderSystem* rs = (D3D11RenderSystem*)Root::getSingleton().getRenderSystem();
        if(rs->_getFeatureLevel() < D3D_FEATURE_LEVEL_10_0)
        {
            // http://msdn.microsoft.com/en-us/library/windows/desktop/ff476150%28v=vs.85%29.aspx#ID3D11Device_CreateTexture2D
            // ...If MipCount > 1, Dimensions must be integral power of two...
            if(!IsPowerOfTwo(desc.Width) || !IsPowerOfTwo(desc.Height))
            {
                desc.MipLevels = 1;
            }

#if 0
           // there seems to be a Microsoft bug that crash if you do GenerateMips in a level less then D3D_FEATURE_LEVEL_10_0
           // is this still true or addressed by the code above?
           desc.MipLevels = 1;
#endif
        }

        // create the texture
        HRESULT hr = mDevice->CreateTexture2D(  
            &desc,
            NULL,// data pointer
            mp2DTex.ReleaseAndGetAddressOf());
        // check result and except if failed
        if (FAILED(hr) || mDevice.isError())
        {
            this->freeInternalResources();
            String errorDescription = mDevice.getErrorDescription(hr);
			OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
                "Error creating texture\nError Description:" + errorDescription, 
                "D3D11Texture::_create2DTex" );
        }

        //set the base texture we'll use in the render system
        _queryInterface<ID3D11Texture2D, ID3D11Resource>(mp2DTex, &mpTex);

        _create2DResourceView();
    }
    //----------------------------------------------------------------------------
    void D3D11Texture::_create2DResourceView()
    {
        // set final tex. attributes from tex. description
        // they may differ from the source image !!!
        HRESULT hr;
        D3D11_TEXTURE2D_DESC desc;
        mp2DTex->GetDesc(&desc);
        mNumMipmaps = desc.MipLevels - 1;
        
        ZeroMemory( &mSRVDesc, sizeof(mSRVDesc) );
        mSRVDesc.Format = desc.Format == DXGI_FORMAT_R32_TYPELESS ? DXGI_FORMAT_R32_FLOAT : desc.Format;
        
        switch(this->getTextureType())
        {
        case TEX_TYPE_CUBE_MAP:
            mSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
            mSRVDesc.TextureCube.MipLevels = desc.MipLevels;
            mSRVDesc.TextureCube.MostDetailedMip = 0;
            break;

        case TEX_TYPE_2D_ARRAY:
            if (mFSAAType.Count > 1)
            {
                mSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
                mSRVDesc.Texture2DMSArray.FirstArraySlice = 0;
                mSRVDesc.Texture2DMSArray.ArraySize = desc.ArraySize;
            }
            else
            {
                mSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
                mSRVDesc.Texture2DArray.MostDetailedMip = 0;
                mSRVDesc.Texture2DArray.MipLevels = desc.MipLevels;
                mSRVDesc.Texture2DArray.FirstArraySlice = 0;
                mSRVDesc.Texture2DArray.ArraySize = desc.ArraySize;
            }
            break;

        case TEX_TYPE_2D:
        case TEX_TYPE_1D:  // For Feature levels that do not support 1D textures, revert to creating a 2D texture.
            if (mFSAAType.Count > 1)
            {
                mSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
            }
            else
            {
                mSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                mSRVDesc.Texture2D.MostDetailedMip = 0;
                mSRVDesc.Texture2D.MipLevels = desc.MipLevels;
            }
            break;
        }

        OGRE_CHECK_DX_ERROR(
            (desc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
                ? mDevice->CreateShaderResourceView(mp2DTex.Get(), &mSRVDesc,
                                                    mpShaderResourceView.ReleaseAndGetAddressOf())
                : S_FALSE);

        this->_setFinalAttributes(desc.Width, desc.Height, desc.ArraySize / getNumFaces(), D3D11Mappings::_getPF(desc.Format), desc.MiscFlags);
    }
    //---------------------------------------------------------------------
    void D3D11Texture::_create3DTex()
    {
        // we must have those defined here
        assert(mWidth > 0 && mHeight > 0 && mDepth>0);

        // determine total number of mipmaps including main one (d3d11 convention)
        UINT numMips = (mNumMipmaps == MIP_UNLIMITED || (1U << mNumMipmaps) > std::max(std::max(mSrcWidth, mSrcHeight), mDepth)) ? 0 : mNumMipmaps + 1;

        D3D11_TEXTURE3D_DESC desc;
        desc.Width          = static_cast<UINT>(mSrcWidth);
        desc.Height         = static_cast<UINT>(mSrcHeight);
        desc.Depth          = static_cast<UINT>(mDepth);
        desc.MipLevels      = numMips;
        desc.Format         = mD3DFormat;
		desc.Usage			= D3D11Mappings::_getUsage(_getTextureUsage());
        desc.BindFlags      = D3D11Mappings::_getTextureBindFlags(mD3DFormat, _getTextureUsage());

        D3D11RenderSystem* rsys = static_cast<D3D11RenderSystem*>(Root::getSingleton().getRenderSystem());
        if (rsys->_getFeatureLevel() < D3D_FEATURE_LEVEL_10_0)
           desc.BindFlags      &= ~D3D11_BIND_RENDER_TARGET;

		desc.CPUAccessFlags = D3D11Mappings::_getAccessFlags(_getTextureUsage());
        desc.MiscFlags      = 0;

        // create the texture
        HRESULT hr = mDevice->CreateTexture3D(  
            &desc,
            NULL,
            mp3DTex.ReleaseAndGetAddressOf());                      // data pointer
        // check result and except if failed
        if (FAILED(hr) || mDevice.isError())
        {
            this->freeInternalResources();
            String errorDescription = mDevice.getErrorDescription(hr);
			OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
                "Error creating texture\nError Description:" + errorDescription, 
                "D3D11Texture::_create3DTex" );
        }

        _queryInterface<ID3D11Texture3D, ID3D11Resource>(mp3DTex, &mpTex);
        _create3DResourceView();
    }
    //-------------------------------------------------------------------------------
    void D3D11Texture::_create3DResourceView()
    {
        // set final tex. attributes from tex. description
        // they may differ from the source image !!!
        HRESULT hr;
        D3D11_TEXTURE3D_DESC desc;
        mp3DTex->GetDesc(&desc);
        mNumMipmaps = desc.MipLevels - 1;

        ZeroMemory( &mSRVDesc, sizeof(mSRVDesc) );
        mSRVDesc.Format = desc.Format;
        mSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
        mSRVDesc.Texture3D.MostDetailedMip = 0;
        mSRVDesc.Texture3D.MipLevels = desc.MipLevels;
        OGRE_CHECK_DX_ERROR(
            (desc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
                ? mDevice->CreateShaderResourceView(mp3DTex.Get(), &mSRVDesc,
                                                    mpShaderResourceView.ReleaseAndGetAddressOf())
                : S_FALSE);

        this->_setFinalAttributes(desc.Width, desc.Height, desc.Depth, D3D11Mappings::_getPF(desc.Format), desc.MiscFlags);
    }
    //-------------------------------------------------------------------------------
    void D3D11Texture::_setFinalAttributes(unsigned long width, unsigned long height, 
        unsigned long depth, PixelFormat format, UINT miscflags)
    { 
        // set target texture attributes
        mHeight = height; 
        mWidth = width; 
        mDepth = depth;
        mFormat = format;
        mAutoMipMapGeneration = miscflags & D3D11_RESOURCE_MISC_GENERATE_MIPS;

        // Update size (the final size, including temp space because in consumed memory)
        // this is needed in Resource class
        mSize = calculateSize();

        // say to the world what we are doing
        if (mWidth != mSrcWidth ||
            mHeight != mSrcHeight)
        {
            LogManager::getSingleton().logMessage("D3D11: ***** Dimensions altered by the render system");
            LogManager::getSingleton().logMessage("D3D11: ***** Source image dimensions : " + StringConverter::toString(mSrcWidth) + "x" + StringConverter::toString(mSrcHeight));
            LogManager::getSingleton().logMessage("D3D11: ***** Texture dimensions : " + StringConverter::toString(mWidth) + "x" + StringConverter::toString(mHeight));
        }

        // Create list of subsurfaces for getBuffer()
        _createSurfaceList();
    }
    //---------------------------------------------------------------------
    void D3D11Texture::_setSrcAttributes(unsigned long width, unsigned long height, 
        unsigned long depth, PixelFormat format)
    { 
        // set source image attributes
        mSrcWidth = width; 
        mSrcHeight = height; 
        mSrcDepth = depth;
        mSrcFormat = format;
    }
    //---------------------------------------------------------------------
    void D3D11Texture::_createSurfaceList(void)
    {
        // Create new list of surfaces
        mSurfaceList.clear();
        size_t depth = mDepth;

        for(size_t face=0; face<getNumFaces(); ++face)
        {
            size_t width = mWidth;
            size_t height = mHeight;
            for(size_t mip=0; mip<=mNumMipmaps; ++mip)
            { 

                D3D11HardwarePixelBuffer *buffer;
                buffer = new D3D11HardwarePixelBuffer(
                    this, // parentTexture
                    mDevice, // device
                    mip, 
                    width, 
                    height, 
                    depth,
                    face,
                    mFormat,
                    (HardwareBuffer::Usage)mUsage
                    ); 

                mSurfaceList.push_back(HardwarePixelBufferSharedPtr(buffer));

                if(width > 1) width /= 2;
                if(height > 1) height /= 2;
                if(depth > 1 && getTextureType() != TEX_TYPE_2D_ARRAY) depth /= 2;
            }
        }
    }
    //---------------------------------------------------------------------
    // D3D11RenderTexture
    //---------------------------------------------------------------------
    void D3D11RenderTexture::rebind( D3D11HardwarePixelBuffer *buffer )
    {
        mBuffer = buffer;
        mWidth = (unsigned int) mBuffer->getWidth();
        mHeight = (unsigned int) mBuffer->getHeight();
        
        ID3D11Resource * pBackBuffer = buffer->getParentTexture()->getTextureResource();

        D3D11_RENDER_TARGET_VIEW_DESC RTVDesc;
        ZeroMemory( &RTVDesc, sizeof(RTVDesc) );

        RTVDesc.Format = buffer->getParentTexture()->getShaderResourceViewDesc().Format;
        switch(buffer->getParentTexture()->getShaderResourceViewDesc().ViewDimension)
        {
        case D3D11_SRV_DIMENSION_BUFFER:
            RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_BUFFER;
            break;
        case D3D11_SRV_DIMENSION_TEXTURE1D:
            RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1D;
            break;
        case D3D11_SRV_DIMENSION_TEXTURE1DARRAY:
            RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1DARRAY;
            break;
        case D3D11_SRV_DIMENSION_TEXTURECUBE:
            RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
            RTVDesc.Texture2DArray.FirstArraySlice = buffer->getFace();
            RTVDesc.Texture2DArray.ArraySize = 1;
            break;
        case D3D11_SRV_DIMENSION_TEXTURE2D:
            RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            break;
        case D3D11_SRV_DIMENSION_TEXTURE2DARRAY:
            RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
            RTVDesc.Texture2DArray.FirstArraySlice = mZOffset;
            RTVDesc.Texture2DArray.ArraySize = 1;
            break;
        case D3D11_SRV_DIMENSION_TEXTURE2DMS:
            RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
            break;
        case D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY:
            RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
            RTVDesc.Texture2DArray.FirstArraySlice = mZOffset;
            RTVDesc.Texture2DArray.ArraySize = 1;
            break;
        case D3D11_SRV_DIMENSION_TEXTURE3D:
            RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
            RTVDesc.Texture3D.FirstWSlice = mZOffset;
            RTVDesc.Texture3D.WSize = 1;
            break;
        default:
            assert(false);
        }

        if (!PixelUtil::isDepth(mBuffer->getFormat()))
        {
            OGRE_CHECK_DX_ERROR(mDevice->CreateRenderTargetView(pBackBuffer, &RTVDesc,
                                                                mRenderTargetView.ReleaseAndGetAddressOf()));
            return;
        }

        // also create DSV for depth textures
        D3D11_TEXTURE2D_DESC BBDesc;
        getSurface()->GetDesc(&BBDesc);

        // Create the depth stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
        descDSV.Format = DXGI_FORMAT_D32_FLOAT;
        descDSV.ViewDimension = (BBDesc.SampleDesc.Count > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
        descDSV.Flags = 0 /* D3D11_DSV_READ_ONLY_DEPTH | D3D11_DSV_READ_ONLY_STENCIL */;    // TODO: Allows bind depth buffer as depth view AND texture simultaneously.
        descDSV.Texture2D.MipSlice = 0;

        ID3D11DepthStencilView      *depthStencilView;
        OGRE_CHECK_DX_ERROR(mDevice->CreateDepthStencilView(pBackBuffer, &descDSV, &depthStencilView ));

        D3D11RenderSystem* rs = (D3D11RenderSystem*)Root::getSingleton().getRenderSystem();
        mDepthBuffer =
            new D3D11DepthBuffer(DepthBuffer::POOL_NO_DEPTH, rs, depthStencilView, mWidth, mHeight,
                                 BBDesc.SampleDesc.Count, BBDesc.SampleDesc.Quality, false);
        mDepthBuffer->_notifyRenderTargetAttached(this);
    }

    uint D3D11RenderTexture::getNumberOfViews() const { return PixelUtil::isDepth(mBuffer->getFormat()) ? 0 : 1; }

    ID3D11Texture2D* D3D11RenderTexture::getSurface(uint index) const
    {
        return index == 0 ? static_cast<D3D11HardwarePixelBuffer*>(mBuffer)->getParentTexture()->GetTex2D()
                          : NULL;
    }

    ID3D11RenderTargetView* D3D11RenderTexture::getRenderTargetView(uint index) const
    {
        return index == 0 ? mRenderTargetView.Get() : NULL;
    }

    D3D11RenderTexture::D3D11RenderTexture( const String &name, D3D11HardwarePixelBuffer *buffer, uint32 zoffset, D3D11Device & device )
        : RenderTexture(buffer, zoffset)
        , mDevice(device)
    {
        mName = name;
        rebind(buffer);
    }
    //---------------------------------------------------------------------
    D3D11RenderTexture::~D3D11RenderTexture()
    {
        if (mDepthBuffer && PixelUtil::isDepth (mBuffer->getFormat ()))
            delete mDepthBuffer;
    }

    //---------------------------------------------------------------------
    void D3D11RenderTexture::notifyDeviceLost(D3D11Device* device)
    {
    }
    //---------------------------------------------------------------------
    void D3D11RenderTexture::notifyDeviceRestored(D3D11Device* device)
    {
        rebind(static_cast<D3D11HardwarePixelBuffer*>(mBuffer));
    }
}
