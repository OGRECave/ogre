/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/
#include "OgreD3D10Texture.h"
#include "OgreD3D10HardwarePixelBuffer.h"
#include "OgreD3D10Mappings.h"
#include "OgreD3D10Device.h"
#include "OgreException.h"

namespace Ogre 
{
	//---------------------------------------------------------------------
	D3D10Texture::D3D10Texture(ResourceManager* creator, const String& name, 
		ResourceHandle handle, const String& group, bool isManual, 
		ManualResourceLoader* loader, D3D10Device & device)
		:Texture(creator, name, handle, group, isManual, loader),
		mDevice(device), 
		// mpD3D(NULL), 
		mpTex(NULL),
		mpShaderResourceView(NULL),
		mp1DTex(NULL),
		mp2DTex(NULL),
		mp3DTex(NULL),
		mDynamicTextures(false)
	{
	}
	//---------------------------------------------------------------------
	D3D10Texture::~D3D10Texture()
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
	void D3D10Texture::copyToTexture(TexturePtr& target)
	{
		// check if this & target are the same format and type
		// blitting from or to cube textures is not supported yet
		if (target->getUsage() != this->getUsage() ||
			target->getTextureType() != this->getTextureType())
		{
			OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, 
				"Src. and dest. textures must be of same type and must have the same usage !!!", 
				"D3D10Texture::copyToTexture" );
		}


		D3D10Texture *other;
		// get the target
		other = reinterpret_cast< D3D10Texture * >( target.get() );

		mDevice->CopyResource(other->getTextureResource(), mpTex);
		if (mDevice.isError())
		{
			String errorDescription = mDevice.getErrorDescription();
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"D3D10 device cannot copy resource\nError Description:" + errorDescription,
				"D3D10Texture::copyToTexture");
		}

	}
	//---------------------------------------------------------------------
	void D3D10Texture::loadImage( const Image &img )
	{
		// Use OGRE its own codecs
		vector<const Image*>::type imagePtrs;
		imagePtrs.push_back(&img);
		_loadImages( imagePtrs );
	}
	//---------------------------------------------------------------------
	void D3D10Texture::loadImpl()
	{
		if (mUsage & TU_RENDERTARGET)
		{
			createInternalResources();
			return;
		}

		this->_loadTex();

	}
	//---------------------------------------------------------------------
	void D3D10Texture::freeInternalResourcesImpl()
	{
		SAFE_RELEASE(mpTex);
		SAFE_RELEASE(mp1DTex);
		SAFE_RELEASE(mp2DTex);
		SAFE_RELEASE(mp3DTex);
	}
	//---------------------------------------------------------------------
	void D3D10Texture::_loadTex()
	{
		size_t pos = mName.find_last_of(".");
		String ext = mName.substr(pos+1);
		String baseName = mName.substr(0, pos);
		if(this->getTextureType() == TEX_TYPE_CUBE_MAP)
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

				images[i].load(dstream, ext);

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
			Image img;
			DataStreamPtr dstream ;
			// find & load resource data intro stream to allow resource
			// group changes if required
			if(ResourceGroupManager::getSingleton().resourceExists(mGroup,mName))
			{
				dstream = 
					ResourceGroupManager::getSingleton().openResource(
					mName, mGroup, true, this);
			}
			else
			{
				LogManager::getSingleton().logMessage("D3D10 : File "+mName+ " doesn't Exist ,Loading Missing.png instead ");
				mName="Missing.png";
				dstream =
					ResourceGroupManager::getSingleton().openResource(
					mName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, true, this);
			}

		
			if(ext=="dds")
			{
				HRESULT hr;

				MemoryDataStreamPtr memoryptr=MemoryDataStreamPtr(new MemoryDataStream(dstream));
				// Load the Texture
				hr = D3DX10CreateShaderResourceViewFromMemory( mDevice.get(),
										memoryptr->getPtr(),
										memoryptr->size(),
										NULL,
										NULL, 
										&mpShaderResourceView, 
										NULL );
				if( FAILED( hr ) )
				{
					LogManager::getSingleton().logMessage("D3D10 : "+mName+" Could not be loaded");
				}
				D3D10_SHADER_RESOURCE_VIEW_DESC pDesc;
				mpShaderResourceView->GetDesc(&pDesc);
				int a=0;
			}
			else
			{
				img.load(dstream, ext);
				loadImage(img);
			}
		}
	}
	//---------------------------------------------------------------------
	void D3D10Texture::createInternalResourcesImpl(void)
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
			this->_create1DTex();
			break;
		case TEX_TYPE_2D:
		case TEX_TYPE_CUBE_MAP:
			this->_create2DTex();
			break;
		case TEX_TYPE_3D:
			this->_create3DTex();
			break;
		default:
			this->freeInternalResources();
			OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, "Unknown texture type", "D3D10Texture::createInternalResources" );
		}
	}
	//---------------------------------------------------------------------
	void D3D10Texture::_create1DTex()
	{
		// we must have those defined here
		assert(mSrcWidth > 0 || mSrcHeight > 0);

		// determine which D3D10 pixel format we'll use
		HRESULT hr;
		DXGI_FORMAT d3dPF = this->_chooseD3DFormat();

		// Use D3DX to help us create the texture, this way it can adjust any relevant sizes
		UINT numMips = static_cast<UINT>(mNumRequestedMipmaps );




		D3D10_TEXTURE1D_DESC desc;
		desc.Width			= static_cast<UINT>(mSrcWidth);
		desc.MipLevels		= numMips;
		desc.ArraySize		= 1;
		desc.Format			= d3dPF;
		desc.Usage			= D3D10Mappings::_getUsage(mUsage);
		desc.BindFlags		= D3D10_BIND_SHADER_RESOURCE | D3D10_BIND_RENDER_TARGET;
		desc.CPUAccessFlags = D3D10Mappings::_getAccessFlags(mUsage);
		desc.MiscFlags		= D3D10_RESOURCE_MISC_GENERATE_MIPS;

		if (mIsDynamic)
		{
			desc.Usage			= D3D10_USAGE_DYNAMIC;
			desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
			desc.MiscFlags = 0;
			desc.BindFlags		= D3D10_BIND_SHADER_RESOURCE ;
		}

		// create the texture
		hr = mDevice->CreateTexture1D(	
			&desc,
			NULL,
			&mp1DTex);						// data pointer
		// check result and except if failed
		if (FAILED(hr) || mDevice.isError())
		{
			this->freeInternalResources();
			String errorDescription = mDevice.getErrorDescription();
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error creating texture\nError Description:" + errorDescription, 
				"D3D10Texture::_create1DTex" );
		}

		// set the base texture we'll use in the render system
		hr = mp1DTex->QueryInterface(IID_ID3D10Resource, (void **)&mpTex);
		if (FAILED(hr) || mDevice.isError())
		{
			this->freeInternalResources();
			String errorDescription = mDevice.getErrorDescription();
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Can't get base texture\nError Description:" + errorDescription, 
				"D3D10Texture::_create1DTex" );
		}

		// set final tex. attributes from tex. description
		// they may differ from the source image !!!

		mp1DTex->GetDesc(&desc);

		ZeroMemory( &mSRVDesc, sizeof(mSRVDesc) );
		mSRVDesc.Format = desc.Format;
		mSRVDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE1D;
		mSRVDesc.Texture1D.MipLevels = desc.MipLevels;
		hr = mDevice->CreateShaderResourceView( mp1DTex, &mSRVDesc, &mpShaderResourceView );
		if (FAILED(hr) || mDevice.isError())
		{
			String errorDescription = mDevice.getErrorDescription(hr);
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"D3D10 device can't create shader resource view.\nError Description:" + errorDescription,
				"D3D10Texture::_create1DTex");

		}

		this->_setFinalAttributes(desc.Width, 1, 1, D3D10Mappings::_getPF(desc.Format));

	}
	//---------------------------------------------------------------------
	void D3D10Texture::_create2DTex()
	{
		// we must have those defined here
		assert(mSrcWidth > 0 || mSrcHeight > 0);

		// determine which D3D10 pixel format we'll use
		HRESULT hr;
		DXGI_FORMAT d3dPF = this->_chooseD3DFormat();

		// Use D3DX to help us create the texture, this way it can adjust any relevant sizes
		UINT numMips = 0;
		
		if (mNumRequestedMipmaps == MIP_UNLIMITED)
		{
			numMips = 0;
			mNumMipmaps = 0; // TODO - get this value from the created texture
		}
		else
		{
			numMips = static_cast<UINT>(mNumRequestedMipmaps + 1);
		}

		D3D10_TEXTURE2D_DESC desc;
		desc.Width			= static_cast<UINT>(mSrcWidth);
		desc.Height			= static_cast<UINT>(mSrcHeight);
		desc.MipLevels		= numMips;
		desc.ArraySize		= 1;
		desc.Format			= d3dPF;
		DXGI_SAMPLE_DESC sampleDesc;
		sampleDesc.Count = 1;
		sampleDesc.Quality = 0;
		desc.SampleDesc		= sampleDesc;
		desc.Usage			= D3D10_USAGE_DEFAULT;//D3D10Mappings::_getUsage(mUsage);

		desc.BindFlags		= D3D10_BIND_SHADER_RESOURCE | D3D10_BIND_RENDER_TARGET;


		desc.CPUAccessFlags = 0;//D3D10_CPU_ACCESS_WRITE;//D3D10Mappings::_getAccessFlags(mUsage);
		desc.MiscFlags		= 0;//D3D10_RESOURCE_MISC_GENERATE_MIPS;
		//if (mMipmapsHardwareGenerated)
		{
			desc.MiscFlags		|= D3D10_RESOURCE_MISC_GENERATE_MIPS;
		}

		if (mIsDynamic)
		{
			desc.Usage			= D3D10_USAGE_DYNAMIC;
			desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
			desc.MiscFlags = 0;
			desc.BindFlags		= D3D10_BIND_SHADER_RESOURCE ;
		}

		if (this->getTextureType() == TEX_TYPE_CUBE_MAP)
		{
			desc.MiscFlags		|= D3D10_RESOURCE_MISC_TEXTURECUBE;
			desc.ArraySize		= 6;
		}

		if( desc.Format == DXGI_FORMAT_BC1_UNORM || 
			desc.Format == DXGI_FORMAT_BC2_UNORM ||
			desc.Format == DXGI_FORMAT_BC3_UNORM ||
			desc.Format == DXGI_FORMAT_BC4_UNORM ||
			desc.Format == DXGI_FORMAT_BC5_UNORM 
			)
		{
			desc.BindFlags		= D3D10_BIND_SHADER_RESOURCE;
			desc.MiscFlags = 0;
			mNumRequestedMipmaps -= 2;
			desc.MipLevels -= 2;
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
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
				"Error creating texture\nError Description:" + errorDescription, 
				"D3D10Texture::_create2DTex" );
		}

		// set the base texture we'll use in the render system
		hr = mp2DTex->QueryInterface(IID_ID3D10Resource, (void **)&mpTex);
		if (FAILED(hr) || mDevice.isError())
		{
			this->freeInternalResources();
			String errorDescription = mDevice.getErrorDescription(hr);
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"Can't get base texture\nError Description:" + errorDescription, 
				"D3D10Texture::_create2DTex" );
		}

		// set final tex. attributes from tex. description
		// they may differ from the source image !!!
		mp2DTex->GetDesc(&desc);
		
		ZeroMemory( &mSRVDesc, sizeof(mSRVDesc) );
		mSRVDesc.Format = desc.Format;
		mSRVDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
		mSRVDesc.Texture2D.MipLevels = desc.MipLevels;

		if (this->getTextureType() == TEX_TYPE_CUBE_MAP)
		{
			mSRVDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURECUBE;
			mSRVDesc.TextureCube.MipLevels = desc.MipLevels;
			mSRVDesc.TextureCube.MostDetailedMip = 0;

		}
		
		hr = mDevice->CreateShaderResourceView( mp2DTex, &mSRVDesc, &mpShaderResourceView );
		if (FAILED(hr) || mDevice.isError())
		{
			String errorDescription = mDevice.getErrorDescription(hr);
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"D3D10 device can't create shader resource view.\nError Description:" + errorDescription,
				"D3D10Texture::_create2DTex");
		}

		this->_setFinalAttributes(desc.Width, desc.Height, 1, D3D10Mappings::_getPF(desc.Format));
	}
	//---------------------------------------------------------------------
	void D3D10Texture::_create3DTex()
	{
		// we must have those defined here
		assert(mWidth > 0 && mHeight > 0 && mDepth>0);

		// determine which D3D10 pixel format we'll use
		HRESULT hr;
		DXGI_FORMAT d3dPF = this->_chooseD3DFormat();

		// Use D3DX to help us create the texture, this way it can adjust any relevant sizes
		UINT numMips = static_cast<UINT>(mNumRequestedMipmaps + 1);

		D3D10_TEXTURE3D_DESC desc;
		desc.Width			= static_cast<UINT>(mSrcWidth);
		desc.Height			= static_cast<UINT>(mSrcHeight);
		desc.Depth			= static_cast<UINT>(mDepth);
		desc.MipLevels		= numMips;
		desc.Format			= d3dPF;
		desc.Usage			= D3D10Mappings::_getUsage(mUsage);
		desc.BindFlags		= D3D10_BIND_SHADER_RESOURCE | D3D10_BIND_RENDER_TARGET;
		desc.CPUAccessFlags = D3D10Mappings::_getAccessFlags(mUsage);
		desc.MiscFlags		= 0;

		if (mIsDynamic)
		{
			desc.Usage			= D3D10_USAGE_DYNAMIC;
			desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
			desc.BindFlags		= D3D10_BIND_SHADER_RESOURCE ;
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
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"Error creating texture\nError Description:" + errorDescription, 
				"D3D10Texture::_create3DTex" );
		}

		// set the base texture we'll use in the render system
		hr = mp3DTex->QueryInterface(IID_ID3D10Resource, (void **)&mpTex);
		if (FAILED(hr) || mDevice.isError())
		{
			this->freeInternalResources();
			String errorDescription = mDevice.getErrorDescription(hr);
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"Can't get base texture\nError Description:" + errorDescription, 
				"D3D10Texture::_create3DTex" );
		}

		// set final tex. attributes from tex. description
		// they may differ from the source image !!!
		mp3DTex->GetDesc(&desc);

	
		ZeroMemory( &mSRVDesc, sizeof(mSRVDesc) );
		mSRVDesc.Format = desc.Format;
		mSRVDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE3D;
		mSRVDesc.Texture3D.MipLevels = desc.MipLevels;
		hr = mDevice->CreateShaderResourceView( mp3DTex, &mSRVDesc, &mpShaderResourceView );
		if (FAILED(hr) || mDevice.isError())
		{
			String errorDescription = mDevice.getErrorDescription(hr);
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"D3D10 device can't create shader resource view.\nError Description:" + errorDescription,
				"D3D10Texture::_create3DTex");
		}

		this->_setFinalAttributes(desc.Width, desc.Height, desc.Depth, D3D10Mappings::_getPF(desc.Format));
	}
	//---------------------------------------------------------------------
	void D3D10Texture::_setFinalAttributes(unsigned long width, unsigned long height, 
		unsigned long depth, PixelFormat format)
	{ 
		// set target texture attributes
		mHeight = height; 
		mWidth = width; 
		mDepth = depth;
		mFormat = format; 

		// Update size (the final size, including temp space because in consumed memory)
		// this is needed in Resource class
		mSize = calculateSize();

		// say to the world what we are doing
		if (mWidth != mSrcWidth ||
			mHeight != mSrcHeight)
		{
			LogManager::getSingleton().logMessage("D3D10 : ***** Dimensions altered by the render system");
			LogManager::getSingleton().logMessage("D3D10 : ***** Source image dimensions : " + StringConverter::toString(mSrcWidth) + "x" + StringConverter::toString(mSrcHeight));
			LogManager::getSingleton().logMessage("D3D10 : ***** Texture dimensions : " + StringConverter::toString(mWidth) + "x" + StringConverter::toString(mHeight));
		}

		// Create list of subsurfaces for getBuffer()
		_createSurfaceList();
	}
	//---------------------------------------------------------------------
	void D3D10Texture::_setSrcAttributes(unsigned long width, unsigned long height, 
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
				LogManager::getSingleton().logMessage("D3D10 : Creating 1D RenderTarget, name : '" + this->getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			else
				LogManager::getSingleton().logMessage("D3D10 : Loading 1D Texture, image name : '" + this->getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			break;
		case TEX_TYPE_2D:
			if (mUsage & TU_RENDERTARGET)
				LogManager::getSingleton().logMessage("D3D10 : Creating 2D RenderTarget, name : '" + this->getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			else
				LogManager::getSingleton().logMessage("D3D10 : Loading 2D Texture, image name : '" + this->getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			break;
		case TEX_TYPE_3D:
			if (mUsage & TU_RENDERTARGET)
				LogManager::getSingleton().logMessage("D3D10 : Creating 3D RenderTarget, name : '" + this->getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			else
				LogManager::getSingleton().logMessage("D3D10 : Loading 3D Texture, image name : '" + this->getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			break;
		case TEX_TYPE_CUBE_MAP:
			if (mUsage & TU_RENDERTARGET)
				LogManager::getSingleton().logMessage("D3D10 : Creating Cube map RenderTarget, name : '" + this->getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			else
				LogManager::getSingleton().logMessage("D3D10 : Loading Cube Texture, base image name : '" + this->getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			break;
		default:
			this->freeInternalResources();
			OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, "Unknown texture type", "D3D10Texture::_setSrcAttributes" );
		}
	}
	//---------------------------------------------------------------------
	DXGI_FORMAT D3D10Texture::_chooseD3DFormat()
	{
		// Choose frame buffer pixel format in case PF_UNKNOWN was requested
		if(mFormat == PF_UNKNOWN)
			return mBBPixelFormat;
		// Choose closest supported D3D format as a D3D format
		return D3D10Mappings::_getPF(D3D10Mappings::_getClosestSupportedPF(mFormat));

	}
	//---------------------------------------------------------------------
	void D3D10Texture::_createSurfaceList(void)
	{
		unsigned int bufusage;
		if ((mUsage & TU_DYNAMIC) && mDynamicTextures)
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
			PixelFormat format = D3D10Mappings::_getClosestSupportedPF(mFormat);
			size_t depth = mDepth;

			for(size_t face=0; face<getNumFaces(); ++face)
			{
				size_t width = mWidth;
				size_t height = mHeight;
				for(size_t mip=0; mip<=mNumMipmaps; ++mip)
				{ 

					D3D10HardwarePixelBuffer *buffer;
					size_t subresourceIndex = mip + face * mNumMipmaps;
					if (getNumFaces() > 0)
					{
						subresourceIndex = mip;

					}
					buffer = new D3D10HardwarePixelBuffer(
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
	HardwarePixelBufferSharedPtr D3D10Texture::getBuffer(size_t face, size_t mipmap) 
	{
		if(face >= getNumFaces())
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "A three dimensional cube has six faces",
			"D3D10Texture::getBuffer");
		if(mipmap > mNumMipmaps)
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Mipmap index out of range",
			"D3D10Texture::getBuffer");
		size_t idx = face*(mNumMipmaps+1) + mipmap;
		assert(idx < mSurfaceList.size());
		return mSurfaceList[idx];
	}
	//---------------------------------------------------------------------
	bool D3D10Texture::releaseIfDefaultPool(void)
	{
		if(mIsDynamic)
		{
			LogManager::getSingleton().logMessage(
				"Releasing D3D10 default pool texture: " + mName);
			// Just free any internal resources, don't call unload() here
			// because we want the un-touched resource to keep its unloaded status
			// after device reset.
			freeInternalResources();
			LogManager::getSingleton().logMessage(
				"Released D3D10 default pool texture: " + mName);
			return true;
		}
		return false;
	}
	//---------------------------------------------------------------------
	bool D3D10Texture::recreateIfDefaultPool(D3D10Device &  device)
	{
		bool ret = false;
		if(mIsDynamic)
		{
			ret = true;
			LogManager::getSingleton().logMessage(
				"Recreating D3D10 default pool texture: " + mName);
			// We just want to create the texture resources if:
			// 1. This is a render texture, or
			// 2. This is a manual texture with no loader, or
			// 3. This was an unloaded regular texture (preserve unloaded state)
			if ((mIsManual && !mLoader) || (mUsage & TU_RENDERTARGET) || !isLoaded())
			{
				// just recreate any internal resources
				createInternalResources();
			}
			// Otherwise, this is a regular loaded texture, or a manual texture with a loader
			else
			{
				// The internal resources already freed, need unload/load here:
				// 1. Make sure resource memory usage statistic correction.
				// 2. Don't call unload() in releaseIfDefaultPool() because we want
				//    the un-touched resource keep unload status after device reset.
				unload();
				// if manual, we need to recreate internal resources since load() won't do that
				if (mIsManual)
					createInternalResources();
				load();
			}
			LogManager::getSingleton().logMessage(
				"Recreated D3D10 default pool texture: " + mName);
		}

		return ret;

	}
	//---------------------------------------------------------------------
	D3D10_SHADER_RESOURCE_VIEW_DESC D3D10Texture::getShaderResourceViewDesc() const
	{
		return mSRVDesc;
	}
	//---------------------------------------------------------------------
	// D3D10RenderTexture
	//---------------------------------------------------------------------
	void D3D10RenderTexture::rebind( D3D10HardwarePixelBuffer *buffer )
	{
		mBuffer = buffer;
		mWidth = (unsigned int) mBuffer->getWidth();
		mHeight = (unsigned int) mBuffer->getHeight();
		mColourDepth = (unsigned int) PixelUtil::getNumElemBits(mBuffer->getFormat());
		
		ID3D10Resource * pBackBuffer = buffer->getParentTexture()->getTextureResource();

		D3D10_RENDER_TARGET_VIEW_DESC RTVDesc;
		ZeroMemory( &RTVDesc, sizeof(RTVDesc) );

		RTVDesc.Format = buffer->getParentTexture()->getShaderResourceViewDesc().Format;
		switch(buffer->getParentTexture()->getShaderResourceViewDesc().ViewDimension)
		{
		case D3D10_SRV_DIMENSION_BUFFER:
			RTVDesc.ViewDimension = D3D10_RTV_DIMENSION_BUFFER;
			break;
		case D3D10_SRV_DIMENSION_TEXTURE1D:
			RTVDesc.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE1D;
				break;
		case D3D10_SRV_DIMENSION_TEXTURE1DARRAY:
			RTVDesc.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE1DARRAY;
				break;
		case D3D10_SRV_DIMENSION_TEXTURE2D:
			RTVDesc.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2D;
				break;
		case D3D10_SRV_DIMENSION_TEXTURE2DARRAY:
			RTVDesc.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2DARRAY;
				break;
		case D3D10_SRV_DIMENSION_TEXTURE2DMS:
			RTVDesc.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2DMS;
			break;
		case D3D10_SRV_DIMENSION_TEXTURE2DMSARRAY:
			RTVDesc.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2DMSARRAY;
			break;
		case D3D10_SRV_DIMENSION_TEXTURE3D:
			RTVDesc.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE3D;
			break;
		default:
			assert(false);
		}
		RTVDesc.Texture2D.MipSlice = static_cast<uint>(buffer->getSubresourceIndex());
		HRESULT hr = mDevice->CreateRenderTargetView( pBackBuffer, &RTVDesc, &mRenderTargetView );

		if (FAILED(hr) || mDevice.isError())
		{
			String errorDescription = mDevice.getErrorDescription();
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error creating Render Target View\nError Description:" + errorDescription, 
				"D3D10RenderTexture::rebind" );
		}
		// Create depth stencil texture
		ID3D10Texture2D* pDepthStencil = NULL;
		D3D10_TEXTURE2D_DESC descDepth;

		descDepth.Width = mWidth;
		descDepth.Height = mHeight;
		descDepth.MipLevels = 1;
		descDepth.ArraySize = 1;
		descDepth.Format = DXGI_FORMAT_R32_TYPELESS;
		descDepth.SampleDesc.Count = 1;
		descDepth.SampleDesc.Quality = 0;
		descDepth.Usage = D3D10_USAGE_DEFAULT;
		descDepth.BindFlags = D3D10_BIND_DEPTH_STENCIL;
		descDepth.CPUAccessFlags = 0;
		descDepth.MiscFlags = 0;

		hr = mDevice->CreateTexture2D( &descDepth, NULL, &pDepthStencil );
		if( FAILED(hr) || mDevice.isError())
		{
			String errorDescription = mDevice.getErrorDescription(hr);
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"Unable to create depth texture\nError Description:" + errorDescription,
				"D3D10RenderTexture::rebind");
		}

		// Create the depth stencil view
		D3D10_DEPTH_STENCIL_VIEW_DESC descDSV;
		descDSV.Format = DXGI_FORMAT_D32_FLOAT;
		descDSV.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
		descDSV.Texture2D.MipSlice = 0;
		hr = mDevice->CreateDepthStencilView( pDepthStencil, &descDSV, &mDepthStencilView );
		SAFE_RELEASE( pDepthStencil );
		if( FAILED(hr) )
		{
			String errorDescription = mDevice.getErrorDescription();
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"Unable to create depth stencil view\nError Description:" + errorDescription,
				"D3D10RenderTexture::rebind");
		}

	}
	//---------------------------------------------------------------------
	void D3D10RenderTexture::getCustomAttribute( const String& name, void *pData )
	{
		if(name == "DDBACKBUFFER")
		{
			// IDXGISurface ** pSurf = (IDXGISurface **)pData;
			//*pSurf = static_cast<D3D10HardwarePixelBuffer*>(mBuffer)->getSurface();
			return;
		}
		else if(name == "HWND")
		{
			HWND *pHwnd = (HWND*)pData;
			*pHwnd = NULL;
			return;
		}
		else if(name == "BUFFER")
		{
			*static_cast<HardwarePixelBuffer**>(pData) = mBuffer;
			return;
		}
		else if(name == "ID3D10RenderTargetView")
		{
			*static_cast<ID3D10RenderTargetView**>(pData) = mRenderTargetView;
			return;
		}		
		else if(name == "ID3D10DepthStencilView")
		{
			*static_cast<ID3D10DepthStencilView**>(pData) = mDepthStencilView;
			return;
		}


	}
	//---------------------------------------------------------------------
	D3D10RenderTexture::D3D10RenderTexture( const String &name, D3D10HardwarePixelBuffer *buffer,  D3D10Device & device ) : mDevice(device),
	RenderTexture(buffer, 0)
	{

		mName = name;

		rebind(buffer);
	}

	//---------------------------------------------------------------------

	D3D10RenderTexture::~D3D10RenderTexture()
	{

	}
}
