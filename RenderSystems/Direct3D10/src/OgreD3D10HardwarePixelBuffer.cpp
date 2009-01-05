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
#include "OgreD3D10HardwarePixelBuffer.h"
#include "OgreD3D10Texture.h"
#include "OgreD3D10Mappings.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreBitwise.h"

#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreD3D10Texture.h"
#include "OgreD3D10Device.h"

namespace Ogre {

	//-----------------------------------------------------------------------------  

	D3D10HardwarePixelBuffer::D3D10HardwarePixelBuffer(D3D10Texture * parentTexture, D3D10Device & device, size_t subresourceIndex,
		size_t width, size_t height, size_t depth, size_t face, PixelFormat format, HardwareBuffer::Usage usage):
	HardwarePixelBuffer(width, height, depth, format, usage, false, false),
		mParentTexture(parentTexture),
		mDevice(device),
		mSubresourceIndex(subresourceIndex),
		mFace(face)
	{
		if(mUsage & TU_RENDERTARGET)
		{
			// Create render target for each slice
			mSliceTRT.reserve(mDepth);
			assert(mDepth==1);
			for(size_t zoffset=0; zoffset<mDepth; ++zoffset)
			{
				String name;
				name = "rtt/"+StringConverter::toString((size_t)mParentTexture) + "/" + StringConverter::toString(mSubresourceIndex) + "/" + parentTexture->getName();

				RenderTexture *trt = new D3D10RenderTexture(name, this, mDevice);
				mSliceTRT.push_back(trt);
				Root::getSingleton().getRenderSystem()->attachRenderTarget(*trt);
			}
		}
	}
	D3D10HardwarePixelBuffer::~D3D10HardwarePixelBuffer()
	{
		if(mSliceTRT.empty())
			return;
		// Delete all render targets that are not yet deleted via _clearSliceRTT
		for(size_t zoffset=0; zoffset<mDepth; ++zoffset)
		{
			if(mSliceTRT[zoffset])
				Root::getSingleton().getRenderSystem()->destroyRenderTarget(mSliceTRT[zoffset]->getName());
		}
	}
	//-----------------------------------------------------------------------------  
	// Util functions to convert a D3D locked box to a pixel box
	void fromD3DLock(PixelBox &rval, const DXGI_MAPPED_RECT &lrect)
	{
		rval.rowPitch = lrect.Pitch / PixelUtil::getNumElemBytes(rval.format);
		rval.slicePitch = rval.rowPitch * rval.getHeight();
		assert((lrect.Pitch % PixelUtil::getNumElemBytes(rval.format))==0);
		rval.data = lrect.pBits;
	}
	//-----------------------------------------------------------------------------  
	PixelBox D3D10HardwarePixelBuffer::lockImpl(const Image::Box lockBox,  LockOptions options)
	{
		// Check for misuse
		if(mUsage & TU_RENDERTARGET)
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "DirectX does not allow locking of or directly writing to RenderTargets. Use blitFromMemory if you need the contents.",
			"D3D10HardwarePixelBuffer::lockImpl");	
		// Set extents and format
		PixelBox rval(lockBox, mFormat);
		// Set locking flags according to options
		D3D10_MAP  flags = D3D10_MAP_WRITE_DISCARD ;
		switch(options)
		{
		case HBL_DISCARD:
			// D3D only likes D3DLOCK_DISCARD if you created the texture with D3DUSAGE_DYNAMIC
			// debug runtime flags this up, could cause problems on some drivers
			if (mUsage & HBU_DYNAMIC)
				flags = D3D10_MAP_WRITE_DISCARD;
			break;
		case HBL_READ_ONLY:
			flags = D3D10_MAP_READ;
			break;
		default: 
			break;
		};

		mDevice.clearStoredErrorMessages();

		// TODO - check return values here
		switch(mParentTexture->getTextureType()) {
		case TEX_TYPE_1D:
			{

				mParentTexture->GetTex1D()->Map(static_cast<UINT>(mSubresourceIndex), flags, 0, &rval.data);
				if (mDevice.isError())
				{
					String errorDescription = mDevice.getErrorDescription();
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
						"D3D10 device cannot map 1D texture\nError Description:" + errorDescription,
						"D3D10HardwarePixelBuffer::lockImpl");
				}
			}
			break;
		case TEX_TYPE_CUBE_MAP:
		case TEX_TYPE_2D:
			{
				D3D10_MAPPED_TEXTURE2D mappedTex2D;
				mParentTexture->GetTex2D()->Map(static_cast<UINT>(mSubresourceIndex), flags, 0, &mappedTex2D);
				rval.data = mappedTex2D.pData;
				if (mDevice.isError())
				{
					String errorDescription = mDevice.getErrorDescription();
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
						"D3D10 device cannot map 1D texture\nError Description:" + errorDescription,
						"D3D10HardwarePixelBuffer::lockImpl");
				}
			}
			break;
		case TEX_TYPE_3D:
			{
				D3D10_MAPPED_TEXTURE3D mappedTex3D;
				mParentTexture->GetTex3D()->Map(static_cast<UINT>(mSubresourceIndex), flags, 0, &mappedTex3D);
				rval.data = mappedTex3D.pData;
				if (mDevice.isError())
				{
					String errorDescription = mDevice.getErrorDescription();
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
						"D3D10 device cannot map 1D texture\nError Description:" + errorDescription,
						"D3D10HardwarePixelBuffer::lockImpl");
				}
			}
			break;
		}




		return rval;
	}
	//-----------------------------------------------------------------------------  
	void D3D10HardwarePixelBuffer::unlockImpl(void)
	{
		switch(mParentTexture->getTextureType()) {
		case TEX_TYPE_1D:
			{
				mParentTexture->GetTex1D()->Unmap(static_cast<UINT>(mSubresourceIndex));
			}
			break;
		case TEX_TYPE_CUBE_MAP:
		case TEX_TYPE_2D:
			{
				mParentTexture->GetTex2D()->Unmap(static_cast<UINT>(mSubresourceIndex));
			}
			break;
		case TEX_TYPE_3D:
			{
				mParentTexture->GetTex3D()->Unmap(static_cast<UINT>(mSubresourceIndex));
			}
			break;
		}
	}

	//-----------------------------------------------------------------------------  

	D3D10_BOX D3D10HardwarePixelBuffer::OgreImageBoxToDx10Box(const Image::Box &inBox) const
	{
		D3D10_BOX res;
		res.left	= static_cast<UINT>(inBox.left);
		res.top		= static_cast<UINT>(inBox.top);
		res.front	= static_cast<UINT>(inBox.front);
		res.right	= static_cast<UINT>(inBox.right);
		res.bottom	= static_cast<UINT>(inBox.bottom);
		res.back	= static_cast<UINT>(inBox.back);

		return res;
	}

	//-----------------------------------------------------------------------------  

	void D3D10HardwarePixelBuffer::blit(const HardwarePixelBufferSharedPtr &rsrc, const Image::Box &srcBox, const Image::Box &dstBox)
	{
		if (
			(srcBox.getWidth() != dstBox.getWidth())
			|| (srcBox.getHeight() != dstBox.getHeight())
			|| (srcBox.getDepth() != dstBox.getDepth())
			)
		{
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"D3D10 device cannot copy a subresource - source and dest size are not the same and they have to be the same in DX10.",
				"D3D10HardwarePixelBuffer::blit");
		}

		D3D10_BOX srcBoxDx10 = OgreImageBoxToDx10Box(srcBox);


		D3D10HardwarePixelBuffer * rsrcDx10 = static_cast<D3D10HardwarePixelBuffer *>(rsrc.get());

		switch(mParentTexture->getTextureType()) {
		case TEX_TYPE_1D:
			{

				mDevice->CopySubresourceRegion(
					mParentTexture->GetTex1D(), 
					static_cast<UINT>(mSubresourceIndex),
					static_cast<UINT>(dstBox.left),
					0,
					0,
					rsrcDx10->mParentTexture->GetTex1D(),
					static_cast<UINT>(rsrcDx10->mSubresourceIndex),
					&srcBoxDx10);
				if (mDevice.isError())
				{
					String errorDescription = mDevice.getErrorDescription();
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
						"D3D10 device cannot copy 1d subresource Region\nError Description:" + errorDescription,
						"D3D10HardwarePixelBuffer::blit");
				}			
			}
			break;
		case TEX_TYPE_CUBE_MAP:
		case TEX_TYPE_2D:
			{
				mDevice->CopySubresourceRegion(
					mParentTexture->GetTex2D(), 
					static_cast<UINT>(mSubresourceIndex),
					static_cast<UINT>(dstBox.left),
					static_cast<UINT>(dstBox.top),
					mFace,
					rsrcDx10->mParentTexture->GetTex2D(),
					static_cast<UINT>(rsrcDx10->mSubresourceIndex),
					&srcBoxDx10);
				if (mDevice.isError())
				{
					String errorDescription = mDevice.getErrorDescription();
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
						"D3D10 device cannot copy 2d subresource Region\nError Description:" + errorDescription,
						"D3D10HardwarePixelBuffer::blit");
				}
			}
			break;
		case TEX_TYPE_3D:
			{
				mDevice->CopySubresourceRegion(
					mParentTexture->GetTex2D(), 
					static_cast<UINT>(mSubresourceIndex),
					static_cast<UINT>(dstBox.left),
					static_cast<UINT>(dstBox.top),
					static_cast<UINT>(dstBox.front),
					rsrcDx10->mParentTexture->GetTex2D(),
					static_cast<UINT>(rsrcDx10->mSubresourceIndex),
					&srcBoxDx10);
				if (mDevice.isError())
				{
					String errorDescription = mDevice.getErrorDescription();
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
						"D3D10 device cannot copy 3d subresource Region\nError Description:" + errorDescription,
						"D3D10HardwarePixelBuffer::blit");
				}
			}
			break;
		}


		_genMipmaps();

	}
	//-----------------------------------------------------------------------------  
	void D3D10HardwarePixelBuffer::blitFromMemory(const PixelBox &src, const Image::Box &dstBox)
	{
		bool isDds = false;
		switch(mFormat)
		{
		case PF_DXT1:
		case PF_DXT2:
		case PF_DXT3:
		case PF_DXT4:
		case PF_DXT5:
			isDds = true;
			break;
		default:

			break;
		}

		if (isDds && (dstBox.getWidth() % 4 != 0 || dstBox.getHeight() % 4 != 0 ))
		{
			return;
		}


		// for scoped deletion of conversion buffer
		MemoryDataStreamPtr buf;
		PixelBox converted = src;

		D3D10_BOX dstBoxDx10 = OgreImageBoxToDx10Box(dstBox);

		// convert to pixelbuffer's native format if necessary
		if (src.format != mFormat)
		{
			buf.bind(new MemoryDataStream(
				PixelUtil::getMemorySize(src.getWidth(), src.getHeight(), src.getDepth(),
				mFormat)));
			converted = PixelBox(src.getWidth(), src.getHeight(), src.getDepth(), mFormat, buf->getPtr());
			PixelUtil::bulkPixelConversion(src, converted);
		}

		// In d3d10 the Row Pitch is defined as: "The size of one row of the source data" and not 
		// the same as the OGRE row pitch - meaning that we need to multiple the OGRE row pitch 
		// with the size in bytes of the element to get the d3d10 row pitch. 
		UINT d3dRowPitch = static_cast<UINT>(converted.rowPitch) * static_cast<UINT>(PixelUtil::getNumElemBytes(mFormat));


		switch(mParentTexture->getTextureType()) {
		case TEX_TYPE_1D:
			{

				mDevice->UpdateSubresource( 
					mParentTexture->GetTex1D(), 
					0,
					&dstBoxDx10,
					converted.data,
					0,
					0 );
				if (mDevice.isError())
				{
					String errorDescription = mDevice.getErrorDescription();
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
						"D3D10 device cannot update 1d subresource\nError Description:" + errorDescription,
						"D3D10HardwarePixelBuffer::blitFromMemory");
				}
			}
			break;
		case TEX_TYPE_CUBE_MAP:
		case TEX_TYPE_2D:
			{
				mDevice->UpdateSubresource( 
					mParentTexture->GetTex2D(), 
					static_cast<UINT>(mSubresourceIndex),
					&dstBoxDx10,
					converted.data,
					d3dRowPitch,
					mFace );
				if (mDevice.isError())
				{
					String errorDescription = mDevice.getErrorDescription();
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
						"D3D10 device cannot update 2d subresource\nError Description:" + errorDescription,
						"D3D10HardwarePixelBuffer::blitFromMemory");
				}
			}
			break;
		case TEX_TYPE_3D:
			{
				mDevice->UpdateSubresource( 
					mParentTexture->GetTex2D(), 
					static_cast<UINT>(mSubresourceIndex),
					&dstBoxDx10,
					converted.data,
					d3dRowPitch,
					static_cast<UINT>(converted.slicePitch)
					);
				if (mDevice.isError())
				{
					String errorDescription = mDevice.getErrorDescription();
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
						"D3D10 device cannot update 3d subresource\nError Description:" + errorDescription,
						"D3D10HardwarePixelBuffer::blitFromMemory");
				}
			}
			break;
		}


		if (!isDds)
		{
			_genMipmaps();
		}

	}
	//-----------------------------------------------------------------------------  
	void D3D10HardwarePixelBuffer::blitToMemory(const Image::Box &srcBox, const PixelBox &dst)
	{
		/*
		// Decide on pixel format of temp surface
		PixelFormat tmpFormat = mFormat; 
		if(D3D10Mappings::_getPF(dst.format) != D3DFMT_UNKNOWN)
		{
		tmpFormat = dst.format;
		}
		if(mSurface)
		{
		assert(srcBox.getDepth() == 1 && dst.getDepth() == 1);
		// Create temp texture
		ID3D10Resource  *tmp;
		IDirect3DSurface9 *surface;

		if(D3DXCreateTexture(
		mDevice,
		dst.getWidth(), dst.getHeight(), 
		1, // 1 mip level ie topmost, generate no mipmaps
		0, D3D10Mappings::_getPF(tmpFormat), D3DPOOL_SCRATCH,
		&tmp
		) != D3D_OK)
		{
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Create temporary texture failed",
		"D3D10HardwarePixelBuffer::blitToMemory");
		}
		if(tmp->GetSurfaceLevel(0, &surface) != D3D_OK)
		{
		tmp->Release();
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Get surface level failed",
		"D3D10HardwarePixelBuffer::blitToMemory");
		}
		// Copy texture to this temp surface
		RECT destRect, srcRect;
		srcRect = toD3DRECT(srcBox);
		destRect = toD3DRECTExtent(dst);

		if(D3DXLoadSurfaceFromSurface(
		surface, NULL, &destRect, 
		mSurface, NULL, &srcRect,
		D3DX_DEFAULT, 0) != D3D_OK)
		{
		surface->Release();
		tmp->Release();
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "D3DXLoadSurfaceFromSurface failed",
		"D3D10HardwarePixelBuffer::blitToMemory");
		}
		// Lock temp surface and copy it to memory
		DXGI_MAPPED_RECT lrect; // Filled in by D3D
		if(surface->LockRect(&lrect, NULL,  D3DLOCK_READONLY) != D3D_OK)
		{
		surface->Release();
		tmp->Release();
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "surface->LockRect",
		"D3D10HardwarePixelBuffer::blitToMemory");
		}
		// Copy it
		PixelBox locked(dst.getWidth(), dst.getHeight(), dst.getDepth(), tmpFormat);
		fromD3DLock(locked, lrect);
		PixelUtil::bulkPixelConversion(locked, dst);
		surface->UnlockRect();
		// Release temporary surface and texture
		surface->Release();
		tmp->Release();
		}
		else
		{
		// Create temp texture
		IDirect3DVolumeTexture9 *tmp;
		IDirect3DVolume9 *surface;

		if(D3DXCreateVolumeTexture(
		mDevice,
		dst.getWidth(), dst.getHeight(), dst.getDepth(), 0,
		0, D3D10Mappings::_getPF(tmpFormat), D3DPOOL_SCRATCH,
		&tmp
		) != D3D_OK)
		{
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Create temporary texture failed",
		"D3D10HardwarePixelBuffer::blitToMemory");
		}
		if(tmp->GetVolumeLevel(0, &surface) != D3D_OK)
		{
		tmp->Release();
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Get volume level failed",
		"D3D10HardwarePixelBuffer::blitToMemory");
		}
		// Volume
		D3D10_BOX ddestBox, dsrcBox;
		ddestBox = toD3DBOXExtent(dst);
		dsrcBox = toD3DBOX(srcBox);

		if(D3DXLoadVolumeFromVolume(
		surface, NULL, &ddestBox, 
		mVolume, NULL, &dsrcBox,
		D3DX_DEFAULT, 0) != D3D_OK)
		{
		surface->Release();
		tmp->Release();
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "D3DXLoadVolumeFromVolume failed",
		"D3D10HardwarePixelBuffer::blitToMemory");
		}
		// Lock temp surface and copy it to memory
		D3DLOCKED_BOX lbox; // Filled in by D3D
		if(surface->LockBox(&lbox, NULL,  D3DLOCK_READONLY) != D3D_OK)
		{
		surface->Release();
		tmp->Release();
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "surface->LockBox",
		"D3D10HardwarePixelBuffer::blitToMemory");
		}
		// Copy it
		PixelBox locked(dst.getWidth(), dst.getHeight(), dst.getDepth(), tmpFormat);
		fromD3DLock(locked, lbox);
		PixelUtil::bulkPixelConversion(locked, dst);
		surface->UnlockBox();
		// Release temporary surface and texture
		surface->Release();
		tmp->Release();
		}
		*/
	}

	//-----------------------------------------------------------------------------  
	void D3D10HardwarePixelBuffer::_genMipmaps()
	{
		mDevice->GenerateMips(mParentTexture->getTexture());
		if (mDevice.isError())
		{
			String errorDescription = mDevice.getErrorDescription();
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"D3D10 device cannot generate mips\nError Description:" + errorDescription,
				"D3D10HardwarePixelBuffer::_genMipmaps");
		}	


	}
	//-----------------------------------------------------------------------------    
	RenderTexture *D3D10HardwarePixelBuffer::getRenderTarget(size_t zoffset)
	{
		assert(mUsage & TU_RENDERTARGET);
		assert(zoffset < mDepth);
		return mSliceTRT[zoffset];
	}
	//-----------------------------------------------------------------------------    
	D3D10Texture * D3D10HardwarePixelBuffer::getParentTexture() const
	{
		return mParentTexture;
	}
	//-----------------------------------------------------------------------------    
	size_t D3D10HardwarePixelBuffer::getSubresourceIndex() const
	{
		return mSubresourceIndex;
	}
};
