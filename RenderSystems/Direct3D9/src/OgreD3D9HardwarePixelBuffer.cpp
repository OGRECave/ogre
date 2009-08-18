/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
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

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#include "OgreD3D9HardwarePixelBuffer.h"
#include "OgreD3D9Texture.h"
#include "OgreD3D9Mappings.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreBitwise.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"

namespace Ogre {

//-----------------------------------------------------------------------------  

D3D9HardwarePixelBuffer::D3D9HardwarePixelBuffer(HardwareBuffer::Usage usage, 
												 D3D9Texture* ownerTexture):
	HardwarePixelBuffer(0, 0, 0, PF_UNKNOWN, usage, false, false),
	mDoMipmapGen(0), mHWMipmaps(0), mOwnerTexture(ownerTexture), 
	mRenderTexture(NULL), mDeviceAccessLockCount(0)
{
}
D3D9HardwarePixelBuffer::~D3D9HardwarePixelBuffer()
{
	OGRE_LOCK_MUTEX(mDeviceAccessMutex)

	destroyRenderTexture();
	
	DeviceToBufferResourcesIterator it = mMapDeviceToBufferResources.begin();

	while (it != mMapDeviceToBufferResources.end())
	{
		SAFE_RELEASE(it->second->surface);
		SAFE_RELEASE(it->second->volume);
		SAFE_DELETE(it->second);
		it = mMapDeviceToBufferResources.erase(it);
	}
}
//-----------------------------------------------------------------------------  
void D3D9HardwarePixelBuffer::bind(IDirect3DDevice9 *dev, IDirect3DSurface9 *surface, 
								   IDirect3DSurface9* fsaaSurface,
								   bool writeGamma, uint fsaa, const String& srcName,
								   IDirect3DBaseTexture9 *mipTex)
{
	OGRE_LOCK_MUTEX(mDeviceAccessMutex)

	BufferResources* bufferResources = getBufferResources(dev);
	bool isNewBuffer = false;

	if (bufferResources == NULL)
	{
		bufferResources = createBufferResources();		
		mMapDeviceToBufferResources[dev] = bufferResources;
		isNewBuffer = true;
	}
		
	bufferResources->mipTex = mipTex;
	bufferResources->surface = surface;
	bufferResources->surface->AddRef();
	bufferResources->fSAASurface = fsaaSurface;

	D3DSURFACE_DESC desc;
	if(surface->GetDesc(&desc) != D3D_OK)
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Could not get surface information",
		 "D3D9HardwarePixelBuffer::D3D9HardwarePixelBuffer");

	mWidth = desc.Width;
	mHeight = desc.Height;
	mDepth = 1;
	mFormat = D3D9Mappings::_getPF(desc.Format);
	// Default
	mRowPitch = mWidth;
	mSlicePitch = mHeight*mWidth;
	mSizeInBytes = PixelUtil::getMemorySize(mWidth, mHeight, mDepth, mFormat);	
	
	if(mUsage & TU_RENDERTARGET)
		updateRenderTexture(writeGamma, fsaa, srcName);

	if (isNewBuffer && mOwnerTexture->isManuallyLoaded())
	{
		DeviceToBufferResourcesIterator it = mMapDeviceToBufferResources.begin();

		while (it != mMapDeviceToBufferResources.end())
		{
			if (it->second != bufferResources && 
				it->second->surface != NULL &&
				it->first->TestCooperativeLevel() == D3D_OK &&
				dev->TestCooperativeLevel() == D3D_OK)
			{
				Box fullBufferBox(0,0,0,mWidth,mHeight,mDepth);
				PixelBox dstBox(fullBufferBox, mFormat);

				dstBox.data = new char[getSizeInBytes()];
				blitToMemory(fullBufferBox, dstBox, it->second, it->first);
				blitFromMemory(dstBox, fullBufferBox, bufferResources);
				SAFE_DELETE_ARRAY(dstBox.data);
				break;
			}
			++it;			
		}				
	}
}
//-----------------------------------------------------------------------------
void D3D9HardwarePixelBuffer::bind(IDirect3DDevice9 *dev, IDirect3DVolume9 *volume, IDirect3DBaseTexture9 *mipTex)
{
	OGRE_LOCK_MUTEX(mDeviceAccessMutex)

	BufferResources* bufferResources = getBufferResources(dev);
	bool isNewBuffer = false;

	if (bufferResources == NULL)
	{
		bufferResources = createBufferResources();
		mMapDeviceToBufferResources[dev] = bufferResources;
		isNewBuffer = true;
	}

	bufferResources->mipTex = mipTex;
	bufferResources->volume = volume;
	bufferResources->volume->AddRef();
	
	D3DVOLUME_DESC desc;
	if(volume->GetDesc(&desc) != D3D_OK)
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Could not get volume information",
		 "D3D9HardwarePixelBuffer::D3D9HardwarePixelBuffer");
	mWidth = desc.Width;
	mHeight = desc.Height;
	mDepth = desc.Depth;
	mFormat = D3D9Mappings::_getPF(desc.Format);
	// Default
	mRowPitch = mWidth;
	mSlicePitch = mHeight*mWidth;
	mSizeInBytes = PixelUtil::getMemorySize(mWidth, mHeight, mDepth, mFormat);

	if (isNewBuffer && mOwnerTexture->isManuallyLoaded())
	{
		DeviceToBufferResourcesIterator it = mMapDeviceToBufferResources.begin();
		
		while (it != mMapDeviceToBufferResources.end())
		{
			if (it->second != bufferResources &&
				it->second->volume != NULL &&
				it->first->TestCooperativeLevel() == D3D_OK &&
				dev->TestCooperativeLevel() == D3D_OK)
			{
				Box fullBufferBox(0,0,0,mWidth,mHeight,mDepth);
				PixelBox dstBox(fullBufferBox, mFormat);

				dstBox.data = new char[getSizeInBytes()];
				blitToMemory(fullBufferBox, dstBox, it->second, it->first);
				blitFromMemory(dstBox, fullBufferBox, bufferResources);
				SAFE_DELETE(dstBox.data);
				break;
			}
			++it;			
		}				
	}
}

//-----------------------------------------------------------------------------  
D3D9HardwarePixelBuffer::BufferResources* D3D9HardwarePixelBuffer::getBufferResources(IDirect3DDevice9* d3d9Device)
{
	DeviceToBufferResourcesIterator it = mMapDeviceToBufferResources.find(d3d9Device);

	if (it != mMapDeviceToBufferResources.end())	
		return it->second;
	
	return NULL;
}

//-----------------------------------------------------------------------------  
D3D9HardwarePixelBuffer::BufferResources* D3D9HardwarePixelBuffer::createBufferResources()
{
	BufferResources* newResources = new BufferResources;

	memset(newResources, 0, sizeof(BufferResources));

	return newResources;
}

//-----------------------------------------------------------------------------  
void D3D9HardwarePixelBuffer::destroyBufferResources(IDirect3DDevice9* d3d9Device)
{
	OGRE_LOCK_MUTEX(mDeviceAccessMutex)

	DeviceToBufferResourcesIterator it = mMapDeviceToBufferResources.find(d3d9Device);

	if (it != mMapDeviceToBufferResources.end())
	{
		SAFE_RELEASE(it->second->surface);
		SAFE_RELEASE(it->second->volume);	
		SAFE_DELETE(it->second);
		mMapDeviceToBufferResources.erase(it);
	}
}

//-----------------------------------------------------------------------------  
void D3D9HardwarePixelBuffer::lockDeviceAccess()
{
	assert(mDeviceAccessLockCount >= 0);
	mDeviceAccessLockCount++;
	if (mDeviceAccessLockCount == 1)
	{
		OGRE_LOCK_RECURSIVE_MUTEX(mDeviceAccessMutex);		
	}
		
}

//-----------------------------------------------------------------------------  
void D3D9HardwarePixelBuffer::unlockDeviceAccess()
{
	if (mDeviceAccessLockCount > 0)
	{
		mDeviceAccessLockCount--;
		if (mDeviceAccessLockCount == 0)
		{
			OGRE_UNLOCK_RECURSIVE_MUTEX(mDeviceAccessMutex);			
		}
	}		
}

//-----------------------------------------------------------------------------  
// Util functions to convert a D3D locked box to a pixel box
void fromD3DLock(PixelBox &rval, const D3DLOCKED_RECT &lrect)
{
	size_t bpp = PixelUtil::getNumElemBytes(rval.format);
	if (bpp != 0)
	{
		rval.rowPitch = lrect.Pitch / bpp;
		rval.slicePitch = rval.rowPitch * rval.getHeight();
		assert((lrect.Pitch % bpp)==0);
	}
	else if (PixelUtil::isCompressed(rval.format))
	{
		rval.rowPitch = rval.getWidth();
		rval.slicePitch = rval.getWidth() * rval.getHeight();
	}
	else
	{
		OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
			"Invalid pixel format", "fromD3DLock");
	}

	rval.data = lrect.pBits;
}
void fromD3DLock(PixelBox &rval, const D3DLOCKED_BOX &lbox)
{
	size_t bpp = PixelUtil::getNumElemBytes(rval.format);
	if (bpp != 0)
	{
		rval.rowPitch = lbox.RowPitch / bpp;
		rval.slicePitch = lbox.SlicePitch / bpp;
		assert((lbox.RowPitch % bpp)==0);
		assert((lbox.SlicePitch % bpp)==0);
	}
	else if (PixelUtil::isCompressed(rval.format))
	{
		rval.rowPitch = rval.getWidth();
		rval.slicePitch = rval.getWidth() * rval.getHeight();
	}
	else
	{
		OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
			"Invalid pixel format", "fromD3DLock");
	}
	rval.data = lbox.pBits;
}
// Convert Ogre integer Box to D3D rectangle
RECT toD3DRECT(const Box &lockBox)
{
	RECT prect;
	assert(lockBox.getDepth() == 1);
	prect.left = static_cast<LONG>(lockBox.left);
	prect.right = static_cast<LONG>(lockBox.right);
	prect.top = static_cast<LONG>(lockBox.top);
	prect.bottom = static_cast<LONG>(lockBox.bottom);
	return prect;
}
// Convert Ogre integer Box to D3D box
D3DBOX toD3DBOX(const Box &lockBox)
{
	D3DBOX pbox;
	
	pbox.Left = static_cast<UINT>(lockBox.left);
	pbox.Right = static_cast<UINT>(lockBox.right);
	pbox.Top = static_cast<UINT>(lockBox.top);
	pbox.Bottom = static_cast<UINT>(lockBox.bottom);
	pbox.Front = static_cast<UINT>(lockBox.front);
	pbox.Back = static_cast<UINT>(lockBox.back);
	return pbox;
}
// Convert Ogre pixelbox extent to D3D rectangle
RECT toD3DRECTExtent(const PixelBox &lockBox)
{
	RECT prect;
	assert(lockBox.getDepth() == 1);
	prect.left = 0;
	prect.right = static_cast<LONG>(lockBox.getWidth());
	prect.top = 0;
	prect.bottom = static_cast<LONG>(lockBox.getHeight());
	return prect;
}
// Convert Ogre pixelbox extent to D3D box
D3DBOX toD3DBOXExtent(const PixelBox &lockBox)
{
	D3DBOX pbox;
	pbox.Left = 0;
	pbox.Right = static_cast<UINT>(lockBox.getWidth());
	pbox.Top = 0;
	pbox.Bottom = static_cast<UINT>(lockBox.getHeight());
	pbox.Front = 0;
	pbox.Back = static_cast<UINT>(lockBox.getDepth());
	return pbox;
}
//-----------------------------------------------------------------------------  
PixelBox D3D9HardwarePixelBuffer::lockImpl(const Image::Box lockBox,  LockOptions options)
{	
	OGRE_LOCK_MUTEX(mDeviceAccessMutex)

	// Check for misuse
	if(mUsage & TU_RENDERTARGET)
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "DirectX does not allow locking of or directly writing to RenderTargets. Use blitFromMemory if you need the contents.",
		 	"D3D9HardwarePixelBuffer::lockImpl");		
	// Set locking flags according to options
	DWORD flags = 0;
	switch(options)
	{
	case HBL_DISCARD:
		// D3D only likes D3DLOCK_DISCARD if you created the texture with D3DUSAGE_DYNAMIC
		// debug runtime flags this up, could cause problems on some drivers
		if (mUsage & HBU_DYNAMIC)
			flags |= D3DLOCK_DISCARD;
		break;
	case HBL_READ_ONLY:
		flags |= D3DLOCK_READONLY;
		break;
	default: 
		break;
	};

	if (mMapDeviceToBufferResources.size() == 0)
	{
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "There are no resources attached to this pixel buffer !!",
			"D3D9HardwarePixelBuffer::lockImpl");	
	}
	
	mLockedBox = lockBox;
	mLockFlags = flags;

	BufferResources* bufferResources = mMapDeviceToBufferResources.begin()->second;
	
	// Lock the source buffer.
	return lockBuffer(bufferResources, lockBox, flags);
}

//-----------------------------------------------------------------------------  
Ogre::PixelBox D3D9HardwarePixelBuffer::lockBuffer(BufferResources* bufferResources, 
												   const Image::Box &lockBox, 
												   DWORD flags)
{
	// Set extents and format
	// Note that we do not carry over the left/top/front here, since the returned
	// PixelBox will be re-based from the locking point onwards
	PixelBox rval(lockBox.getWidth(), lockBox.getHeight(), lockBox.getDepth(), mFormat);


	if (bufferResources->surface != NULL) 
	{
		// Surface
		D3DLOCKED_RECT lrect; // Filled in by D3D
		HRESULT hr;

		if (lockBox.left == 0 && lockBox.top == 0 
			&& lockBox.right == mWidth && lockBox.bottom == mHeight)
		{
			// Lock whole surface
			hr = bufferResources->surface->LockRect(&lrect, NULL, flags);
		}
		else
		{
			RECT prect = toD3DRECT(lockBox); // specify range to lock
			hr = bufferResources->surface->LockRect(&lrect, &prect, flags);
		}
		if (FAILED(hr))		
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Surface locking failed",
			"D3D9HardwarePixelBuffer::lockImpl");
		fromD3DLock(rval, lrect);
	} 
	else if(bufferResources->volume) 
	{
		// Volume
		D3DBOX pbox = toD3DBOX(lockBox); // specify range to lock
		D3DLOCKED_BOX lbox; // Filled in by D3D

		if(bufferResources->volume->LockBox(&lbox, &pbox, flags) != D3D_OK)
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Volume locking failed",
			"D3D9HardwarePixelBuffer::lockImpl");
		fromD3DLock(rval, lbox);
	}


	return rval;
}

//-----------------------------------------------------------------------------  
void D3D9HardwarePixelBuffer::unlockImpl(void)
{
	OGRE_LOCK_MUTEX(mDeviceAccessMutex)

	if (mMapDeviceToBufferResources.size() == 0)
	{
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "There are no resources attached to this pixel buffer !!",
			"D3D9HardwarePixelBuffer::lockImpl");	
	}

	DeviceToBufferResourcesIterator it;
	
	// 1. Update duplicates buffers.
	it = mMapDeviceToBufferResources.begin();
	++it;
	while (it != mMapDeviceToBufferResources.end())
	{			
		BufferResources* bufferResources = it->second;
		
		// Update duplicated buffer from the from the locked buffer content.					
		blitFromMemory(mCurrentLock, mLockedBox, bufferResources);										
		++it;			
	}

	// 2. Unlock the locked buffer.
	it = mMapDeviceToBufferResources.begin();							
	unlockBuffer( it->second);		
	if(mDoMipmapGen)
		_genMipmaps(it->second->mipTex);	
}

//-----------------------------------------------------------------------------  
void D3D9HardwarePixelBuffer::unlockBuffer(BufferResources* bufferResources)
{
	if(bufferResources->surface) 
	{
		// Surface
		bufferResources->surface->UnlockRect();
	} 
	else if(bufferResources->volume) 
	{
		// Volume
		bufferResources->volume->UnlockBox();
	}
}

//-----------------------------------------------------------------------------  
void D3D9HardwarePixelBuffer::blit(const HardwarePixelBufferSharedPtr &rsrc, 
								   const Image::Box &srcBox, 
								   const Image::Box &dstBox)
{
	OGRE_LOCK_MUTEX(mDeviceAccessMutex)

	D3D9HardwarePixelBuffer *src = static_cast<D3D9HardwarePixelBuffer*>(rsrc.getPointer());
	DeviceToBufferResourcesIterator it = mMapDeviceToBufferResources.begin();

	// Update all the buffer copies.
	while (it != mMapDeviceToBufferResources.end())
	{
		BufferResources* srcBufferResources = src->getBufferResources(it->first);
		BufferResources* dstBufferResources = it->second;

		if (srcBufferResources == NULL)
		{
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "There are no matching resources attached to the source pixel buffer !!",
				"D3D9HardwarePixelBuffer::blit");	
		}

		blit(rsrc, srcBox, dstBox, srcBufferResources, dstBufferResources);
		++it;
	}
}

//-----------------------------------------------------------------------------  
void D3D9HardwarePixelBuffer::blit(const HardwarePixelBufferSharedPtr &rsrc, 
								   const Image::Box &srcBox, 
								   const Image::Box &dstBox,
								   BufferResources* srcBufferResources, 
								   BufferResources* dstBufferResources)
{
	if(dstBufferResources->surface && srcBufferResources->surface)
	{
		// Surface-to-surface
		RECT dsrcRect = toD3DRECT(srcBox);
		RECT ddestRect = toD3DRECT(dstBox);
		// D3DXLoadSurfaceFromSurface
		if(D3DXLoadSurfaceFromSurface(
			dstBufferResources->surface, NULL, &ddestRect, 
			srcBufferResources->surface, NULL, &dsrcRect,
			D3DX_DEFAULT, 0) != D3D_OK)
		{
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "D3DXLoadSurfaceFromSurface failed",
				"D3D9HardwarePixelBuffer::blit");
		}
	}
	else if(dstBufferResources->volume && srcBufferResources->volume)
	{
		// Volume-to-volume
		D3DBOX dsrcBox = toD3DBOX(srcBox);
		D3DBOX ddestBox = toD3DBOX(dstBox);

		// D3DXLoadVolumeFromVolume
		if(D3DXLoadVolumeFromVolume(
			dstBufferResources->volume, NULL, &ddestBox, 
			srcBufferResources->volume, NULL, &dsrcBox,
			D3DX_DEFAULT, 0) != D3D_OK)
		{
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "D3DXLoadVolumeFromVolume failed",
				"D3D9HardwarePixelBuffer::blit");
		}
	}
	else
	{
		// Software fallback   
		HardwarePixelBuffer::blit(rsrc, srcBox, dstBox);
	}
}

//-----------------------------------------------------------------------------  
void D3D9HardwarePixelBuffer::blitFromMemory(const PixelBox &src, const Image::Box &dstBox)
{	
	OGRE_LOCK_MUTEX(mDeviceAccessMutex)

	DeviceToBufferResourcesIterator it = mMapDeviceToBufferResources.begin();

	while (it != mMapDeviceToBufferResources.end())
	{		
		BufferResources* dstBufferResources = it->second;
		
		blitFromMemory(src, dstBox, dstBufferResources);	
		++it;
	}
}

//-----------------------------------------------------------------------------  
void D3D9HardwarePixelBuffer::blitFromMemory(const PixelBox &src, const Image::Box &dstBox, BufferResources* dstBufferResources)
{
	// for scoped deletion of conversion buffer
	MemoryDataStreamPtr buf;
	PixelBox converted = src;

	// convert to pixelbuffer's native format if necessary
	if (D3D9Mappings::_getPF(src.format) == D3DFMT_UNKNOWN)
	{
		buf.bind(new MemoryDataStream(
			PixelUtil::getMemorySize(src.getWidth(), src.getHeight(), src.getDepth(),
			mFormat)));
		converted = PixelBox(src.getWidth(), src.getHeight(), src.getDepth(), mFormat, buf->getPtr());
		PixelUtil::bulkPixelConversion(src, converted);
	}

	size_t rowWidth;
	if (PixelUtil::isCompressed(converted.format))
	{
		// D3D wants the width of one row of cells in bytes
		if (converted.format == PF_DXT1)
		{
			// 64 bits (8 bytes) per 4x4 block
			rowWidth = (converted.rowPitch / 4) * 8;
		}
		else
		{
			// 128 bits (16 bytes) per 4x4 block
			rowWidth = (converted.rowPitch / 4) * 16;
		}

	}
	else
	{
		rowWidth = converted.rowPitch * PixelUtil::getNumElemBytes(converted.format);
	}

	if (dstBufferResources->surface)
	{
		RECT destRect, srcRect;
		srcRect = toD3DRECT(converted);
		destRect = toD3DRECT(dstBox);

		if(D3DXLoadSurfaceFromMemory(dstBufferResources->surface, NULL, &destRect, 
			converted.data, D3D9Mappings::_getPF(converted.format),
			static_cast<UINT>(rowWidth),
			NULL, &srcRect, D3DX_DEFAULT, 0) != D3D_OK)
		{
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "D3DXLoadSurfaceFromMemory failed",
				"D3D9HardwarePixelBuffer::blitFromMemory");
		}
	}
	else if (dstBufferResources->volume)
	{
		D3DBOX destBox, srcBox;
		srcBox = toD3DBOX(converted);
		destBox = toD3DBOX(dstBox);
		size_t sliceWidth;
		if (PixelUtil::isCompressed(converted.format))
		{
			// D3D wants the width of one slice of cells in bytes
			if (converted.format == PF_DXT1)
			{
				// 64 bits (8 bytes) per 4x4 block
				sliceWidth = (converted.slicePitch / 16) * 8;
			}
			else
			{
				// 128 bits (16 bytes) per 4x4 block
				sliceWidth = (converted.slicePitch / 16) * 16;
			}

		}
		else
		{
			sliceWidth = converted.slicePitch * PixelUtil::getNumElemBytes(converted.format);
		}

		if(D3DXLoadVolumeFromMemory(dstBufferResources->volume, NULL, &destBox, 
			converted.data, D3D9Mappings::_getPF(converted.format),
			static_cast<UINT>(rowWidth), static_cast<UINT>(sliceWidth),
			NULL, &srcBox, D3DX_DEFAULT, 0) != D3D_OK)
		{
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "D3DXLoadSurfaceFromMemory failed",
				"D3D9HardwarePixelBuffer::blitFromMemory");
		}
	}

	if(mDoMipmapGen)
		_genMipmaps(dstBufferResources->mipTex);

}

//-----------------------------------------------------------------------------  
void D3D9HardwarePixelBuffer::blitToMemory(const Image::Box &srcBox, const PixelBox &dst)
{
	OGRE_LOCK_MUTEX(mDeviceAccessMutex)

	DeviceToBufferResourcesIterator it = mMapDeviceToBufferResources.begin();
	BufferResources* bufferResources = it->second;

	blitToMemory(srcBox, dst, bufferResources, it->first);
}

//-----------------------------------------------------------------------------  
void D3D9HardwarePixelBuffer::blitToMemory(const Image::Box &srcBox, const PixelBox &dst, 
										   BufferResources* srcBufferResources,
										   IDirect3DDevice9* d3d9Device)
{
	// Decide on pixel format of temp surface
	PixelFormat tmpFormat = mFormat; 
	if(D3D9Mappings::_getPF(dst.format) != D3DFMT_UNKNOWN)
	{
		tmpFormat = dst.format;
	}

	if (srcBufferResources->surface)
	{
		assert(srcBox.getDepth() == 1 && dst.getDepth() == 1);
		// Create temp texture
		IDirect3DTexture9 *tmp;
		IDirect3DSurface9 *surface;

		D3DSURFACE_DESC srcDesc;
		if(srcBufferResources->surface->GetDesc(&srcDesc) != D3D_OK)
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Could not get surface information",
			"D3D9HardwarePixelBuffer::blitToMemory");

		D3DPOOL temppool = D3DPOOL_SCRATCH;
		// if we're going to try to use GetRenderTargetData, need to use system mem pool
		bool tryGetRenderTargetData = false;
		if (((srcDesc.Usage & D3DUSAGE_RENDERTARGET) != 0) &&
			(srcBox.getWidth() == dst.getWidth()) && (srcBox.getHeight() == dst.getHeight()) &&
			(srcBox.getWidth() == getWidth()) && (srcBox.getHeight() == getHeight()) &&
			(mFormat == tmpFormat))
		{
			tryGetRenderTargetData = true;
			temppool = D3DPOOL_SYSTEMMEM;
		}

		if(D3DXCreateTexture(
			d3d9Device,
			static_cast<UINT>(dst.getWidth()), static_cast<UINT>(dst.getHeight()), 
			1, // 1 mip level ie topmost, generate no mipmaps
			0, D3D9Mappings::_getPF(tmpFormat), temppool,
			&tmp
			) != D3D_OK)
		{
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Create temporary texture failed",
				"D3D9HardwarePixelBuffer::blitToMemory");
		}
		if(tmp->GetSurfaceLevel(0, &surface) != D3D_OK)
		{
			tmp->Release();
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Get surface level failed",
				"D3D9HardwarePixelBuffer::blitToMemory");
		}
		// Copy texture to this temp surface
		RECT destRect, srcRect;
		srcRect = toD3DRECT(srcBox);
		destRect = toD3DRECTExtent(dst);

		// Get the real temp surface format
		D3DSURFACE_DESC dstDesc;
		if(surface->GetDesc(&dstDesc) != D3D_OK)
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Could not get surface information",
			"D3D9HardwarePixelBuffer::blitToMemory");
		tmpFormat = D3D9Mappings::_getPF(dstDesc.Format);

		// Use fast GetRenderTargetData if we are in its usage conditions
		bool fastLoadSuccess = false;
		if (tryGetRenderTargetData)
		{
			if(d3d9Device->GetRenderTargetData(srcBufferResources->surface, surface) == D3D_OK)
			{
				fastLoadSuccess = true;
			}
		}
		if (!fastLoadSuccess)
		{
			if(D3DXLoadSurfaceFromSurface(
				surface, NULL, &destRect, 
				srcBufferResources->surface, NULL, &srcRect,
				D3DX_DEFAULT, 0) != D3D_OK)
			{
				surface->Release();
				tmp->Release();
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "D3DXLoadSurfaceFromSurface failed",
					"D3D9HardwarePixelBuffer::blitToMemory");
			}
		}

		// Lock temp surface and copy it to memory
		D3DLOCKED_RECT lrect; // Filled in by D3D
		if(surface->LockRect(&lrect, NULL,  D3DLOCK_READONLY) != D3D_OK)
		{
			surface->Release();
			tmp->Release();
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "surface->LockRect",
				"D3D9HardwarePixelBuffer::blitToMemory");
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
	else if (srcBufferResources->volume)
	{
		// Create temp texture
		IDirect3DVolumeTexture9 *tmp;
		IDirect3DVolume9 *surface;

		if(D3DXCreateVolumeTexture(
			d3d9Device,
			static_cast<UINT>(dst.getWidth()), 
			static_cast<UINT>(dst.getHeight()), 
			static_cast<UINT>(dst.getDepth()), 0,
			0, D3D9Mappings::_getPF(tmpFormat), D3DPOOL_SCRATCH,
			&tmp
			) != D3D_OK)
		{
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Create temporary texture failed",
				"D3D9HardwarePixelBuffer::blitToMemory");
		}
		if(tmp->GetVolumeLevel(0, &surface) != D3D_OK)
		{
			tmp->Release();
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Get volume level failed",
				"D3D9HardwarePixelBuffer::blitToMemory");
		}
		// Volume
		D3DBOX ddestBox, dsrcBox;
		ddestBox = toD3DBOXExtent(dst);
		dsrcBox = toD3DBOX(srcBox);

		if(D3DXLoadVolumeFromVolume(
			surface, NULL, &ddestBox, 
			srcBufferResources->volume, NULL, &dsrcBox,
			D3DX_DEFAULT, 0) != D3D_OK)
		{
			surface->Release();
			tmp->Release();
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "D3DXLoadVolumeFromVolume failed",
				"D3D9HardwarePixelBuffer::blitToMemory");
		}
		// Lock temp surface and copy it to memory
		D3DLOCKED_BOX lbox; // Filled in by D3D
		if(surface->LockBox(&lbox, NULL,  D3DLOCK_READONLY) != D3D_OK)
		{
			surface->Release();
			tmp->Release();
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "surface->LockBox",
				"D3D9HardwarePixelBuffer::blitToMemory");
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
}

//-----------------------------------------------------------------------------  
void D3D9HardwarePixelBuffer::_genMipmaps(IDirect3DBaseTexture9* mipTex)
{
	assert(mipTex);

	// Mipmapping
	if (mHWMipmaps)
	{
		// Hardware mipmaps
		mipTex->GenerateMipSubLevels();
	}
	else
	{
		// Software mipmaps
		if( D3DXFilterTexture( mipTex, NULL, D3DX_DEFAULT, D3DX_DEFAULT ) != D3D_OK )
		{
			OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR, 
			"Failed to filter texture (generate mipmaps)",
			 "D3D9HardwarePixelBuffer::_genMipmaps" );
		}
	}

}
//----------------------------------------------------------------------------- 
void D3D9HardwarePixelBuffer::_setMipmapping(bool doMipmapGen, 
											 bool HWMipmaps)
{	
	mDoMipmapGen = doMipmapGen;
	mHWMipmaps = HWMipmaps;	
}
//-----------------------------------------------------------------------------   
void D3D9HardwarePixelBuffer::_clearSliceRTT(size_t zoffset)
{
	mRenderTexture = NULL;
}

//-----------------------------------------------------------------------------  
void D3D9HardwarePixelBuffer::releaseSurfaces(IDirect3DDevice9* d3d9Device)
{
	BufferResources* bufferResources = getBufferResources(d3d9Device);

	if (bufferResources != NULL)
	{
		SAFE_RELEASE(bufferResources->surface);
		SAFE_RELEASE(bufferResources->volume);
	}
}
//-----------------------------------------------------------------------------   
IDirect3DSurface9* D3D9HardwarePixelBuffer::getSurface(IDirect3DDevice9* d3d9Device)
{
	BufferResources* bufferResources = getBufferResources(d3d9Device);

	if (bufferResources	== NULL)
	{
		mOwnerTexture->createTextureResources(d3d9Device);
		bufferResources = getBufferResources(d3d9Device);
	}

	return bufferResources->surface;
}
//-----------------------------------------------------------------------------   
IDirect3DSurface9* D3D9HardwarePixelBuffer::getFSAASurface(IDirect3DDevice9* d3d9Device)
{
	BufferResources* bufferResources = getBufferResources(d3d9Device);

	if (bufferResources	== NULL)
	{
		mOwnerTexture->createTextureResources(d3d9Device);
		bufferResources = getBufferResources(d3d9Device);
	}
	
	return bufferResources->fSAASurface;
}
//-----------------------------------------------------------------------------    
RenderTexture *D3D9HardwarePixelBuffer::getRenderTarget(size_t zoffset)
{
    assert(mUsage & TU_RENDERTARGET);
	assert(mRenderTexture != NULL);   
	return mRenderTexture;
}
//-----------------------------------------------------------------------------    
void D3D9HardwarePixelBuffer::updateRenderTexture(bool writeGamma, uint fsaa, const String& srcName)
{
	if (mRenderTexture == NULL)
	{
		String name;
		name = "rtt/" +Ogre::StringConverter::toString((size_t)this) + "/" + srcName;

		mRenderTexture = new D3D9RenderTexture(name, this, writeGamma, fsaa);		
		Root::getSingleton().getRenderSystem()->attachRenderTarget(*mRenderTexture);
	}
}
//-----------------------------------------------------------------------------    
void D3D9HardwarePixelBuffer::destroyRenderTexture()
{
	if (mRenderTexture != NULL)
	{
		Root::getSingleton().getRenderSystem()->destroyRenderTarget(mRenderTexture->getName());
		mRenderTexture = NULL;
	}
}

};
