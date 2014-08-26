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
#include "OgreRoot.h"
#include "OgreException.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 && !defined(_WIN32_WINNT_WIN8)
#define USE_D3DX11_LIBRARY
#endif

#ifdef USE_D3DX11_LIBRARY
// D3DX11CreateShaderResourceViewFromMemory
#include <d3dx11.h>
#endif

namespace Ogre 
{
	//---------------------------------------------------------------------
	D3D11Texture::D3D11Texture(ResourceManager* creator, const String& name, 
		ResourceHandle handle, const String& group, bool isManual, 
		ManualResourceLoader* loader, D3D11Device & device)
		:Texture(creator, name, handle, group, isManual, loader),
		mDevice(device), 
		mpTex(NULL),
		mpShaderResourceView(NULL),
		mp1DTex(NULL),
		mp2DTex(NULL),
		mp3DTex(NULL),
		mDynamicTextures(false),
		mAutoMipMapGeneration(false)
	{
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
		other = reinterpret_cast< D3D11Texture * >( target.get() );

		mDevice.GetImmediateContext()->CopyResource(other->getTextureResource(), mpTex);
		if (mDevice.isError())
		{
			String errorDescription = mDevice.getErrorDescription();
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"D3D11 device cannot copy resource\nError Description:" + errorDescription,
				"D3D11Texture::copyToTexture");
		}

	}
	//---------------------------------------------------------------------
	void D3D11Texture::loadImage( const Image &img )
	{
		// Use OGRE its own codecs
		vector<const Image*>::type imagePtrs;
		imagePtrs.push_back(&img);
		_loadImages( imagePtrs );
	}
	//---------------------------------------------------------------------
	void D3D11Texture::loadImpl()
	{
		if (mUsage & TU_RENDERTARGET)
		{
			createInternalResources();
			return;
		}

		// Make sure streams prepared.
		if (mLoadedStreams.isNull())
		{
			prepareImpl();
		}

		// Set reading positions of loaded streams to the beginning.
		for (uint i = 0; i < mLoadedStreams->size(); ++i)
		{
			MemoryDataStreamPtr curDataStream = (*mLoadedStreams)[i];

			curDataStream->seek(0);
		}

		// only copy is on the stack so well-behaved if exception thrown
		LoadedStreams loadedStreams = mLoadedStreams;

		this->_loadTex(loadedStreams);

	}
	//---------------------------------------------------------------------
	void D3D11Texture::freeInternalResources(void)
	{
		freeInternalResourcesImpl();
	}
	//---------------------------------------------------------------------
	void D3D11Texture::freeInternalResourcesImpl()
	{
		SAFE_RELEASE(mpTex);
        SAFE_RELEASE(mpShaderResourceView);
		SAFE_RELEASE(mp1DTex);
		SAFE_RELEASE(mp2DTex);
		SAFE_RELEASE(mp3DTex);
	}
	//---------------------------------------------------------------------
	void D3D11Texture::_loadTex(LoadedStreams & loadedStreams)
	{
		size_t pos = mName.find_last_of(".");
		String ext = mName.substr(pos+1);
		String baseName = mName.substr(0, pos);
		if((getSourceFileType() != "dds") && (this->getTextureType() == TEX_TYPE_CUBE_MAP))
		{
			// Load from 6 separate files
			// Use OGRE its own codecs
			//	String baseName;
			//	size_t pos = mName.find_last_of(".");
			
			//	if ( pos != String::npos )
			//		ext = mName.substr(pos+1);
			vector<Image>::type images(6);
			ConstImagePtrList imagePtrs;
			static const String suffixes[6] = {"_rt", "_lf", "_up", "_dn", "_fr", "_bk"};

            assert(loadedStreams->size()==6);
			for(size_t i = 0; i < 6; i++)
			{
				String fullName = baseName + suffixes[i];
				if (!ext.empty())
					fullName = fullName + "." + ext;

				// find & load resource data intro stream to allow resource
				// group changes if required
				DataStreamPtr stream((*loadedStreams)[i]);

				images[i].load(stream, ext);

				size_t imageMips = images[i].getNumMipmaps();

				if(imageMips < mNumMipmaps) {
					mNumMipmaps = imageMips;
				}


				imagePtrs.push_back(&images[i]);
			}

			_loadImages( imagePtrs );

		}
		else
		{
            assert(loadedStreams->size()==1);

			Image img;
			DataStreamPtr dstream((*loadedStreams)[0]);
#ifdef USE_D3DX11_LIBRARY		
			if(ext=="dds")
			{
				_loadDDS(dstream);
			}
			else
#endif
			{
				img.load(dstream, ext);
				loadImage(img);
			}
		}
	}
	//---------------------------------------------------------------------
#ifdef USE_D3DX11_LIBRARY		
	void D3D11Texture::_loadDDS(DataStreamPtr &dstream)
	{
		HRESULT hr;

		MemoryDataStreamPtr memoryptr=MemoryDataStreamPtr(new MemoryDataStream(dstream));
		// Load the Texture
		hr = D3DX11CreateTextureFromMemory( mDevice.get(), 
			memoryptr->getPtr(),
			memoryptr->size(),
			NULL,
			NULL, 
			&mpTex, 
			NULL );
		if( FAILED( hr ) )
		{
			LogManager::getSingleton().logMessage("D3D11 : " + mName + " Could not be loaded");
			return;
		}	

		D3D11_RESOURCE_DIMENSION dimension;
		mpTex->GetType(&dimension);

		switch (dimension)
		{
		case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
			{
				_queryInterface<ID3D11Resource, ID3D11Texture1D>(mpTex, &mp1DTex);

				D3D11_TEXTURE1D_DESC desc;
				mp1DTex->GetDesc(&desc);
				
				mFormat = D3D11Mappings::_getPF(desc.Format);
				mTextureType = TEX_TYPE_1D;

				_create1DResourceView();
			} 					
			break;
		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
			{
				_queryInterface<ID3D11Resource, ID3D11Texture2D>(mpTex, &mp2DTex);

				D3D11_TEXTURE2D_DESC desc;
				mp2DTex->GetDesc(&desc);
				
				mFormat = D3D11Mappings::_getPF(desc.Format);
				
				if(desc.ArraySize % 6 == 0 && desc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE)
					mTextureType = TEX_TYPE_CUBE_MAP; //2darray cubemap
				else if(desc.ArraySize > 1)
					mTextureType = TEX_TYPE_2D_ARRAY;
				else
					mTextureType = TEX_TYPE_2D;

				_create2DResourceView();
			}
			break;
		case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
			{
				_queryInterface<ID3D11Resource, ID3D11Texture3D>(mpTex, &mp3DTex);

				D3D11_TEXTURE3D_DESC desc;
				mp3DTex->GetDesc(&desc);

				mFormat = D3D11Mappings::_getPF(desc.Format);
				mTextureType = TEX_TYPE_3D;

				_create3DResourceView();
			}
			break;
		}
	}
#endif
    //---------------------------------------------------------------------
	void D3D11Texture::createInternalResources(void)
	{
		createInternalResourcesImpl();
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

		// Determine D3D pool to use
		// Use managed unless we're a render target or user has asked for 
		// a dynamic texture
		if (//(mUsage & TU_RENDERTARGET) ||
			(mUsage & TU_DYNAMIC))
		{
			mIsDynamic = true;
		}
		else
		{
			mIsDynamic = false;
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

		// determine which D3D11 pixel format we'll use
		HRESULT hr;
		DXGI_FORMAT d3dPF = this->_chooseD3DFormat();

		// determine total number of mipmaps including main one (d3d11 convention)
		UINT numMips = (mNumRequestedMipmaps == MIP_UNLIMITED || (1U << mNumRequestedMipmaps) > mSrcWidth) ? 0 : mNumRequestedMipmaps + 1;

		D3D11_TEXTURE1D_DESC desc;
		desc.Width			= static_cast<UINT>(mSrcWidth);
		desc.MipLevels		= numMips;
		desc.ArraySize		= 1;
		desc.Format			= d3dPF;
		desc.Usage			= D3D11Mappings::_getUsage(_getTextureUsage());
		desc.BindFlags		= D3D11Mappings::_getTextureBindFlags(d3dPF, _getTextureUsage());
		desc.CPUAccessFlags = D3D11Mappings::_getAccessFlags(_getTextureUsage());
		desc.MiscFlags		= D3D11Mappings::_getTextureMiscFlags(desc.BindFlags, getTextureType(), mIsDynamic);

		// create the texture
		hr = mDevice->CreateTexture1D(	
			&desc,
			NULL,
			&mp1DTex);						// data pointer
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
		hr = mDevice->CreateShaderResourceView( mp1DTex, &mSRVDesc, &mpShaderResourceView );
		if (FAILED(hr) || mDevice.isError())
		{
			String errorDescription = mDevice.getErrorDescription(hr);
			OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
				"D3D11 device can't create shader resource view.\nError Description:" + errorDescription,
				"D3D11Texture::_create1DTex");
		}

		this->_setFinalAttributes(desc.Width, 1, 1, D3D11Mappings::_getPF(desc.Format), desc.MiscFlags);

	}
    //---------------------------------------------------------------------
    inline bool	IsPowerOfTwo(unsigned int n)				{ return ((n&(n-1))==0);					}
	//---------------------------------------------------------------------
	void D3D11Texture::_create2DTex()
	{
		// we must have those defined here
		assert(mSrcWidth > 0 || mSrcHeight > 0);

		// determine which D3D11 pixel format we'll use
		HRESULT hr;
		DXGI_FORMAT d3dPF = this->_chooseD3DFormat();

		bool isBinaryCompressedFormat = 
			d3dPF == DXGI_FORMAT_BC1_TYPELESS || d3dPF == DXGI_FORMAT_BC1_UNORM || d3dPF == DXGI_FORMAT_BC1_UNORM_SRGB ||
			d3dPF == DXGI_FORMAT_BC2_TYPELESS || d3dPF == DXGI_FORMAT_BC2_UNORM || d3dPF == DXGI_FORMAT_BC2_UNORM_SRGB ||
			d3dPF == DXGI_FORMAT_BC3_TYPELESS || d3dPF == DXGI_FORMAT_BC3_UNORM || d3dPF == DXGI_FORMAT_BC3_UNORM_SRGB ||
			d3dPF == DXGI_FORMAT_BC4_TYPELESS || d3dPF == DXGI_FORMAT_BC4_UNORM || d3dPF == DXGI_FORMAT_BC4_SNORM ||
			d3dPF == DXGI_FORMAT_BC5_TYPELESS || d3dPF == DXGI_FORMAT_BC5_UNORM || d3dPF == DXGI_FORMAT_BC5_SNORM ||
#if OGRE_PLATFORM == OGRE_PLATFORM_WINRT
			d3dPF == DXGI_FORMAT_BC6H_TYPELESS || d3dPF == DXGI_FORMAT_BC6H_UF16 || d3dPF == DXGI_FORMAT_BC6H_SF16 || 
			d3dPF == DXGI_FORMAT_BC7_TYPELESS || d3dPF == DXGI_FORMAT_BC7_UNORM || d3dPF == DXGI_FORMAT_BC7_UNORM_SRGB ||
#endif
			0;

		// determine total number of mipmaps including main one (d3d11 convention)
		UINT numMips = (mNumRequestedMipmaps == MIP_UNLIMITED || (1U << mNumRequestedMipmaps) > std::max(mSrcWidth, mSrcHeight)) ? 0 : mNumRequestedMipmaps + 1;
		if(isBinaryCompressedFormat && numMips > 1)
			numMips = std::max(1U, numMips - 2);

		D3D11_TEXTURE2D_DESC desc;
		desc.Width			= static_cast<UINT>(mSrcWidth);
		desc.Height			= static_cast<UINT>(mSrcHeight);
		desc.MipLevels		= numMips;
		desc.ArraySize		= mDepth == 0 ? 1 : mDepth;
		desc.Format			= d3dPF;

		// Handle multisampled render target
        if (mUsage & TU_RENDERTARGET && (mFSAA > 1 || atoi(mFSAAHint.c_str()) > 0))
        {
                desc.SampleDesc.Count = mFSAA;
                desc.SampleDesc.Quality = atoi(mFSAAHint.c_str());
        }
        else
        {
                desc.SampleDesc.Count = 1;
                desc.SampleDesc.Quality = 0;
        }

        desc.Usage          = D3D11Mappings::_getUsage(_getTextureUsage());
        desc.BindFlags      = D3D11Mappings::_getTextureBindFlags(d3dPF, _getTextureUsage());
        desc.CPUAccessFlags = D3D11Mappings::_getAccessFlags(_getTextureUsage());
        desc.MiscFlags      = D3D11Mappings::_getTextureMiscFlags(desc.BindFlags, getTextureType(), mIsDynamic);

        if (mIsDynamic)
        {
                desc.SampleDesc.Count = 1;
                desc.SampleDesc.Quality = 0;
        }

        if (this->getTextureType() == TEX_TYPE_CUBE_MAP)
        {
                desc.ArraySize          = 6;
        }

		if( isBinaryCompressedFormat )
        {
                desc.SampleDesc.Count = 1;
                desc.SampleDesc.Quality = 0;
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
		hr = mDevice->CreateTexture2D(	
			&desc,
			NULL,// data pointer
			&mp2DTex);						
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
		mSRVDesc.Format = desc.Format;
		
		switch(this->getTextureType())
		{
		case TEX_TYPE_CUBE_MAP:
			mSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
			mSRVDesc.TextureCube.MipLevels = desc.MipLevels;
			mSRVDesc.TextureCube.MostDetailedMip = 0;
			break;

		case TEX_TYPE_2D_ARRAY:
			if (mUsage & TU_RENDERTARGET && (mFSAA > 1 || atoi(mFSAAHint.c_str()) > 0))
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
			if (mUsage & TU_RENDERTARGET && (mFSAA > 1 || atoi(mFSAAHint.c_str()) > 0))
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

		hr = mDevice->CreateShaderResourceView( mp2DTex, &mSRVDesc, &mpShaderResourceView );
		if (FAILED(hr) || mDevice.isError())
		{
			String errorDescription = mDevice.getErrorDescription(hr);
			OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
				"D3D11 device can't create shader resource view.\nError Description:" + errorDescription,
				"D3D11Texture::_create2DTex");
		}

		this->_setFinalAttributes(desc.Width, desc.Height, desc.ArraySize, D3D11Mappings::_getPF(desc.Format), desc.MiscFlags);
	}
	//---------------------------------------------------------------------
	void D3D11Texture::_create3DTex()
	{
		// we must have those defined here
		assert(mWidth > 0 && mHeight > 0 && mDepth>0);

		// determine which D3D11 pixel format we'll use
		HRESULT hr;
		DXGI_FORMAT d3dPF = this->_chooseD3DFormat();

		// determine total number of mipmaps including main one (d3d11 convention)
		UINT numMips = (mNumRequestedMipmaps == MIP_UNLIMITED || (1U << mNumRequestedMipmaps) > std::max(std::max(mSrcWidth, mSrcHeight), mDepth)) ? 0 : mNumRequestedMipmaps + 1;

		D3D11_TEXTURE3D_DESC desc;
		desc.Width			= static_cast<UINT>(mSrcWidth);
		desc.Height			= static_cast<UINT>(mSrcHeight);
		desc.Depth			= static_cast<UINT>(mDepth);
		desc.MipLevels		= numMips;
		desc.Format			= d3dPF;
		desc.Usage			= D3D11Mappings::_getUsage(_getTextureUsage());
		desc.BindFlags		= D3D11_BIND_SHADER_RESOURCE;

		D3D11RenderSystem* rsys = reinterpret_cast<D3D11RenderSystem*>(Root::getSingleton().getRenderSystem());
		if (rsys->_getFeatureLevel() >= D3D_FEATURE_LEVEL_10_0)
		   desc.BindFlags		|= D3D11_BIND_RENDER_TARGET;

		desc.CPUAccessFlags = D3D11Mappings::_getAccessFlags(_getTextureUsage());
		desc.MiscFlags		= 0;
		if (mIsDynamic)
		{
			desc.Usage			= D3D11_USAGE_DYNAMIC;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.BindFlags		= D3D11_BIND_SHADER_RESOURCE ;
		}

		// create the texture
		hr = mDevice->CreateTexture3D(	
			&desc,
			NULL,
			&mp3DTex);						// data pointer
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
		hr = mDevice->CreateShaderResourceView( mp3DTex, &mSRVDesc, &mpShaderResourceView );
		if (FAILED(hr) || mDevice.isError())
		{
			String errorDescription = mDevice.getErrorDescription(hr);
			OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
				"D3D11 device can't create shader resource view.\nError Description:" + errorDescription,
				"D3D11Texture::_create3DTex");
		}

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
			LogManager::getSingleton().logMessage("D3D11 : ***** Dimensions altered by the render system");
			LogManager::getSingleton().logMessage("D3D11 : ***** Source image dimensions : " + StringConverter::toString(mSrcWidth) + "x" + StringConverter::toString(mSrcHeight));
			LogManager::getSingleton().logMessage("D3D11 : ***** Texture dimensions : " + StringConverter::toString(mWidth) + "x" + StringConverter::toString(mHeight));
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
		// say to the world what we are doing
		switch (this->getTextureType())
		{
		case TEX_TYPE_1D:
			if (mUsage & TU_RENDERTARGET)
				LogManager::getSingleton().logMessage("D3D11 : Creating 1D RenderTarget, name : '" + this->getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			else
				LogManager::getSingleton().logMessage("D3D11 : Loading 1D Texture, image name : '" + this->getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			break;
		case TEX_TYPE_2D:
			if (mUsage & TU_RENDERTARGET)
				LogManager::getSingleton().logMessage("D3D11 : Creating 2D RenderTarget, name : '" + this->getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			else
				LogManager::getSingleton().logMessage("D3D11 : Loading 2D Texture, image name : '" + this->getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			break;
		case TEX_TYPE_2D_ARRAY:
			if (mUsage & TU_RENDERTARGET)
				LogManager::getSingleton().logMessage("D3D11 : Creating 2D array RenderTarget, name : '" + this->getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			else
				LogManager::getSingleton().logMessage("D3D11 : Loading 2D Texture array, image name : '" + this->getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			break;
		case TEX_TYPE_3D:
			if (mUsage & TU_RENDERTARGET)
				LogManager::getSingleton().logMessage("D3D11 : Creating 3D RenderTarget, name : '" + this->getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			else
				LogManager::getSingleton().logMessage("D3D11 : Loading 3D Texture, image name : '" + this->getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			break;
		case TEX_TYPE_CUBE_MAP:
			if (mUsage & TU_RENDERTARGET)
				LogManager::getSingleton().logMessage("D3D11 : Creating Cube map RenderTarget, name : '" + this->getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			else
				LogManager::getSingleton().logMessage("D3D11 : Loading Cube Texture, base image name : '" + this->getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			break;
		default:
			this->freeInternalResources();
			OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, "Unknown texture type", "D3D11Texture::_setSrcAttributes" );
		}
	}
	//---------------------------------------------------------------------
	DXGI_FORMAT D3D11Texture::_chooseD3DFormat()
	{
		// Choose frame buffer pixel format in case PF_UNKNOWN was requested
		if(mFormat == PF_UNKNOWN)
			return mBBPixelFormat;

		D3D11RenderSystem* rsys = reinterpret_cast<D3D11RenderSystem*>(Root::getSingleton().getRenderSystem());
		if (rsys->_getFeatureLevel() < D3D_FEATURE_LEVEL_10_0 && mFormat == PF_L8)
		{
			// For 3D textures, PF_L8, which maps to DXGI_FORMAT_R8_UNORM, is not supported but PF_A8, which maps to DXGI_FORMAT_R8_UNORM is supported.
			mFormat = PF_A8; 
			mNumRequestedMipmaps = 0;
		}

		// Choose closest supported D3D format as a D3D format
		return D3D11Mappings::_getPF(D3D11Mappings::_getClosestSupportedPF(mFormat));

	}
	//---------------------------------------------------------------------
	void D3D11Texture::_createSurfaceList(void)
	{
		unsigned int bufusage;
		if ((mUsage & TU_DYNAMIC))
		{
			bufusage = HardwareBuffer::HBU_DYNAMIC;
		}
		else
		{
			bufusage = HardwareBuffer::HBU_STATIC;
		}
		if (mUsage & TU_RENDERTARGET)
		{
			bufusage |= TU_RENDERTARGET;
		}

		bool updateOldList = mSurfaceList.size() == (getNumFaces() * (mNumMipmaps + 1));
		if(!updateOldList)
		{	
			// Create new list of surfaces
			mSurfaceList.clear();
			PixelFormat format = D3D11Mappings::_getClosestSupportedPF(mSrcFormat);
			size_t depth = mDepth;

			for(size_t face=0; face<getNumFaces(); ++face)
			{
				size_t width = mWidth;
				size_t height = mHeight;
				for(size_t mip=0; mip<=mNumMipmaps; ++mip)
				{ 

					D3D11HardwarePixelBuffer *buffer;
					size_t subresourceIndex = D3D11CalcSubresource(mip, face, mNumMipmaps);
					if (getNumFaces() > 0)
					{
						subresourceIndex = mip;

					}
					buffer = new D3D11HardwarePixelBuffer(
						this, // parentTexture
						mDevice, // device
						subresourceIndex, // subresourceIndex
						width, 
						height, 
						depth,
						face,
						format,
						(HardwareBuffer::Usage)bufusage // usage
						); 

					mSurfaceList.push_back(
						HardwarePixelBufferSharedPtr(buffer)
						);
					width /= 2;
					height /= 2;
				}
			}
		}

		// do we need to bind?

	}
	//---------------------------------------------------------------------
	HardwarePixelBufferSharedPtr D3D11Texture::getBuffer(size_t face, size_t mipmap) 
	{
		if(face >= getNumFaces())
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "A three dimensional cube has six faces",
			"D3D11Texture::getBuffer");
		if(mipmap > mNumMipmaps)
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Mipmap index out of range",
			"D3D11Texture::getBuffer");
		size_t idx = face*(mNumMipmaps+1) + mipmap;
		assert(idx < mSurfaceList.size());
		return mSurfaceList[idx];
	}
	//---------------------------------------------------------------------
	void D3D11Texture::prepareImpl( void )
	{
		if (mUsage & TU_RENDERTARGET || isManuallyLoaded())
		{
			return;
		}

		//D3D11_DEVICE_ACCESS_CRITICAL_SECTION
		
        LoadedStreams loadedStreams;

		// prepare load based on tex.type
		switch (getTextureType())
		{
		case TEX_TYPE_1D:
		case TEX_TYPE_2D:
		case TEX_TYPE_2D_ARRAY:
			loadedStreams = _prepareNormTex();
			break;
		case TEX_TYPE_3D:
			loadedStreams = _prepareVolumeTex();
			break;
		case TEX_TYPE_CUBE_MAP:
			loadedStreams = _prepareCubeTex();
			break;
		default:
			OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, "Unknown texture type", "D3D11Texture::prepareImpl" );
		}

		mLoadedStreams = loadedStreams;		
	}
	//---------------------------------------------------------------------
	D3D11Texture::LoadedStreams D3D11Texture::_prepareCubeTex()
	{
		assert(getTextureType() == TEX_TYPE_CUBE_MAP);

        LoadedStreams loadedStreams = LoadedStreams(OGRE_NEW_T (vector<MemoryDataStreamPtr>::type, MEMCATEGORY_GENERAL), SPFM_DELETE_T );
        // DDS load?
		if (getSourceFileType() == "dds")
		{
            // find & load resource data
			DataStreamPtr dstream = 
				ResourceGroupManager::getSingleton().openResource(
					mName, mGroup, true, this);
            loadedStreams->push_back(MemoryDataStreamPtr(OGRE_NEW MemoryDataStream(dstream)));
        }
        else
        {
			// Load from 6 separate files
			// Use OGRE its own codecs
			String baseName, ext;
			size_t pos = mName.find_last_of(".");
			baseName = mName.substr(0, pos);
			if ( pos != String::npos )
				ext = mName.substr(pos+1);
			static const String suffixes[6] = {"_rt", "_lf", "_up", "_dn", "_fr", "_bk"};

			for(size_t i = 0; i < 6; i++)
			{
				String fullName = baseName + suffixes[i];
				if (!ext.empty())
					fullName = fullName + "." + ext;

            	// find & load resource data intro stream to allow resource
				// group changes if required
				DataStreamPtr dstream = 
					ResourceGroupManager::getSingleton().openResource(
						fullName, mGroup, true, this);

                loadedStreams->push_back(MemoryDataStreamPtr(OGRE_NEW MemoryDataStream(dstream)));
			}
        }

        return loadedStreams;
	}
	//---------------------------------------------------------------------
	D3D11Texture::LoadedStreams D3D11Texture::_prepareVolumeTex()
	{
		assert(getTextureType() == TEX_TYPE_3D);

		// find & load resource data
		DataStreamPtr dstream = 
			ResourceGroupManager::getSingleton().openResource(
				mName, mGroup, true, this);

        LoadedStreams loadedStreams = LoadedStreams(OGRE_NEW_T (vector<MemoryDataStreamPtr>::type, MEMCATEGORY_GENERAL), SPFM_DELETE_T);
        loadedStreams->push_back(MemoryDataStreamPtr(OGRE_NEW MemoryDataStream(dstream)));
        return loadedStreams;
    }
	//---------------------------------------------------------------------
	D3D11Texture::LoadedStreams D3D11Texture::_prepareNormTex()
	{
		assert(getTextureType() == TEX_TYPE_1D || getTextureType() == TEX_TYPE_2D || getTextureType() == TEX_TYPE_2D_ARRAY);

		// find & load resource data
		DataStreamPtr dstream = 
			ResourceGroupManager::getSingleton().openResource(
				mName, mGroup, true, this);

        LoadedStreams loadedStreams = LoadedStreams(OGRE_NEW_T (vector<MemoryDataStreamPtr>::type, MEMCATEGORY_GENERAL), SPFM_DELETE_T);
        loadedStreams->push_back(MemoryDataStreamPtr(OGRE_NEW MemoryDataStream(dstream)));
        return loadedStreams;
	}
	//---------------------------------------------------------------------
	void D3D11Texture::unprepareImpl( void )
	{
		if (mUsage & TU_RENDERTARGET || isManuallyLoaded())
		{
			return;
		}   
	}
	//---------------------------------------------------------------------
	void D3D11Texture::postLoadImpl()
	{
		//D3D11_DEVICE_ACCESS_CRITICAL_SECTION
		mLoadedStreams.setNull();	
	}
	//---------------------------------------------------------------------
	// D3D11RenderTexture
	//---------------------------------------------------------------------
	void D3D11RenderTexture::rebind( D3D11HardwarePixelBuffer *buffer )
	{
		mBuffer = buffer;
		mWidth = (unsigned int) mBuffer->getWidth();
		mHeight = (unsigned int) mBuffer->getHeight();
		mColourDepth = (unsigned int) PixelUtil::getNumElemBits(mBuffer->getFormat());
		
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
			RTVDesc.Texture2DArray.MipSlice = 0;
			break;
		case D3D11_SRV_DIMENSION_TEXTURE2D:
			RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			break;
		case D3D11_SRV_DIMENSION_TEXTURE2DARRAY:
			RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
			break;
		case D3D11_SRV_DIMENSION_TEXTURE2DMS:
			RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
			break;
		case D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY:
			RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
			break;
		case D3D11_SRV_DIMENSION_TEXTURE3D:
			RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
			break;
		default:
			assert(false);
		}
		HRESULT hr = mDevice->CreateRenderTargetView( pBackBuffer, &RTVDesc, &mRenderTargetView );

		if (FAILED(hr) || mDevice.isError())
		{
			String errorDescription = mDevice.getErrorDescription(hr);
			OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
				"Error creating Render Target View\nError Description:" + errorDescription,
				"D3D11RenderTexture::rebind" );
		}
	}
	//---------------------------------------------------------------------
	void D3D11RenderTexture::getCustomAttribute( const String& name, void *pData )
	{
		if(name == "DDBACKBUFFER")
		{
			//IDXGISurface ** pSurf = (IDXGISurface **)pData;
			//*pSurf = static_cast<D3D11HardwarePixelBuffer*>(mBuffer)->getSurface();
			*static_cast<HardwarePixelBuffer**>(pData) = mBuffer;
			return;
		}
		else if(name == "HWND")
		{
			HWND *pHwnd = (HWND*)pData;
			*pHwnd = NULL;
			return;
		}
		else if(name == "isTexture")
		{
			bool *b = reinterpret_cast< bool * >( pData );
			*b = true;
			return;
		}
		else if(name == "BUFFER")
		{
			*static_cast<HardwarePixelBuffer**>(pData) = mBuffer;
			return;
		}
		else if( name == "ID3D11Texture2D" )
		{
			ID3D11Texture2D **pBackBuffer = (ID3D11Texture2D**)pData;
			*pBackBuffer = static_cast<D3D11HardwarePixelBuffer*>(mBuffer)->getParentTexture()->GetTex2D();
			return;
		}
		else if(name == "ID3D11RenderTargetView")
		{
			*static_cast<ID3D11RenderTargetView**>(pData) = mRenderTargetView;
			//*static_cast<ID3D11RenderTargetView***>(pData) = &mRenderTargetView;
			return;
		}
		else if( name == "numberOfViews" )
		{
			unsigned int* n = reinterpret_cast<unsigned int*>(pData);
			*n = 1;
			return;
		}

		RenderTexture::getCustomAttribute(name, pData);
	}
	//---------------------------------------------------------------------
	D3D11RenderTexture::D3D11RenderTexture( const String &name, D3D11HardwarePixelBuffer *buffer,  D3D11Device & device ) : mDevice(device),
	RenderTexture(buffer, 0)
	{
		mName = name;

		rebind(buffer);
	}

	//---------------------------------------------------------------------

	D3D11RenderTexture::~D3D11RenderTexture()
	{

	}
}
