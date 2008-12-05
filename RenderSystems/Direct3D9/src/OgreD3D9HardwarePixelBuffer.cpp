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

#include <d3dx9.h>
#include <dxerr9.h>

namespace Ogre {

//-----------------------------------------------------------------------------  

D3D9HardwarePixelBuffer::D3D9HardwarePixelBuffer(HardwareBuffer::Usage usage):
	HardwarePixelBuffer(0, 0, 0, PF_UNKNOWN, usage, false, false),
	mpDev(0),
	mSurface(0), mFSAASurface(0), mVolume(0), mTempSurface(0), mTempVolume(0),
	mDoMipmapGen(0), mHWMipmaps(0), mMipTex(0)
{
}
D3D9HardwarePixelBuffer::~D3D9HardwarePixelBuffer()
{
	destroyRenderTextures();
}
//-----------------------------------------------------------------------------  
void D3D9HardwarePixelBuffer::bind(IDirect3DDevice9 *dev, IDirect3DSurface9 *surface, 
								   bool update, bool writeGamma, uint fsaa, const String& fsaaHint,  
								   IDirect3DSurface9* fsaaSurface, const String& srcName)
{
	mpDev = dev;
	mSurface = surface;
	mFSAASurface = fsaaSurface;
	
	D3DSURFACE_DESC desc;
	if(mSurface->GetDesc(&desc) != D3D_OK)
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
		createRenderTextures(update, writeGamma, fsaa, fsaaHint, srcName);
}
//-----------------------------------------------------------------------------
void D3D9HardwarePixelBuffer::bind(IDirect3DDevice9 *dev, IDirect3DVolume9 *volume, 
								   bool update, bool writeGamma, const String& srcName)
{
	mpDev = dev;
	mVolume = volume;
	
	D3DVOLUME_DESC desc;
	if(mVolume->GetDesc(&desc) != D3D_OK)
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

	if(mUsage & TU_RENDERTARGET)
		createRenderTextures(update, writeGamma, 0, StringUtil::BLANK, srcName);
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
	prect.left = lockBox.left;
	prect.right = lockBox.right;
	prect.top = lockBox.top;
	prect.bottom = lockBox.bottom;
	return prect;
}
// Convert Ogre integer Box to D3D box
D3DBOX toD3DBOX(const Box &lockBox)
{
	D3DBOX pbox;
	pbox.Left = lockBox.left;
	pbox.Right = lockBox.right;
	pbox.Top = lockBox.top;
	pbox.Bottom = lockBox.bottom;
	pbox.Front = lockBox.front;
	pbox.Back = lockBox.back;
	return pbox;
}
// Convert Ogre pixelbox extent to D3D rectangle
RECT toD3DRECTExtent(const PixelBox &lockBox)
{
	RECT prect;
	assert(lockBox.getDepth() == 1);
	prect.left = 0;
	prect.right = lockBox.getWidth();
	prect.top = 0;
	prect.bottom = lockBox.getHeight();
	return prect;
}
// Convert Ogre pixelbox extent to D3D box
D3DBOX toD3DBOXExtent(const PixelBox &lockBox)
{
	D3DBOX pbox;
	pbox.Left = 0;
	pbox.Right = lockBox.getWidth();
	pbox.Top = 0;
	pbox.Bottom = lockBox.getHeight();
	pbox.Front = 0;
	pbox.Back = lockBox.getDepth();
	return pbox;
}
//-----------------------------------------------------------------------------  
PixelBox D3D9HardwarePixelBuffer::lockImpl(const Image::Box lockBox,  LockOptions options)
{
	// Check for misuse
	if(mUsage & TU_RENDERTARGET)
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "DirectX does not allow locking of or directly writing to RenderTargets. Use blitFromMemory if you need the contents.",
		 	"D3D9HardwarePixelBuffer::lockImpl");	
	// Set extents and format
	PixelBox rval(lockBox, mFormat);
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
	
	if(mSurface) 
	{
		// Surface
		D3DLOCKED_RECT lrect; // Filled in by D3D
		HRESULT hr;

		if (lockBox.left == 0 && lockBox.top == 0 
			&& lockBox.right == mWidth && lockBox.bottom == mHeight)
		{
			// Lock whole surface
			hr = mSurface->LockRect(&lrect, NULL, flags);
		}
		else
		{
			RECT prect = toD3DRECT(lockBox); // specify range to lock
			hr = mSurface->LockRect(&lrect, &prect, flags);
		}
		if (FAILED(hr))		
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Surface locking failed",
		 		"D3D9HardwarePixelBuffer::lockImpl");
		fromD3DLock(rval, lrect);
	} 
	else 
	{
		// Volume
		D3DBOX pbox = toD3DBOX(lockBox); // specify range to lock
		D3DLOCKED_BOX lbox; // Filled in by D3D
		
		if(mVolume->LockBox(&lbox, &pbox, flags) != D3D_OK)
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Volume locking failed",
		 		"D3D9HardwarePixelBuffer::lockImpl");
		fromD3DLock(rval, lbox);
	}

	return rval;
}
//-----------------------------------------------------------------------------  
void D3D9HardwarePixelBuffer::unlockImpl(void)
{
	if(mSurface) 
	{
		// Surface
		mSurface->UnlockRect();
	} else {
		// Volume
		mVolume->UnlockBox();
	}
	if(mDoMipmapGen)
		_genMipmaps();
}
//-----------------------------------------------------------------------------  
void D3D9HardwarePixelBuffer::blit(const HardwarePixelBufferSharedPtr &rsrc, const Image::Box &srcBox, const Image::Box &dstBox)
{
	D3D9HardwarePixelBuffer *src = static_cast<D3D9HardwarePixelBuffer*>(rsrc.getPointer());
	if(mSurface && src->mSurface)
	{
		// Surface-to-surface
		RECT dsrcRect = toD3DRECT(srcBox);
		RECT ddestRect = toD3DRECT(dstBox);
		// D3DXLoadSurfaceFromSurface
		if(D3DXLoadSurfaceFromSurface(
			mSurface, NULL, &ddestRect, 
			src->mSurface, NULL, &dsrcRect,
			 D3DX_DEFAULT, 0) != D3D_OK)
		{
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "D3DXLoadSurfaceFromSurface failed",
		 		"D3D9HardwarePixelBuffer::blit");
		}
	}
	else if(mVolume && src->mVolume)
	{
		// Volume-to-volume
		D3DBOX dsrcBox = toD3DBOX(srcBox);
		D3DBOX ddestBox = toD3DBOX(dstBox);
		
		// D3DXLoadVolumeFromVolume
		if(D3DXLoadVolumeFromVolume(
			mVolume, NULL, &ddestBox, 
			src->mVolume, NULL, &dsrcBox,
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
	if(mSurface)
	{
		RECT destRect, srcRect;
		srcRect = toD3DRECT(converted);
		destRect = toD3DRECT(dstBox);
		
		if(D3DXLoadSurfaceFromMemory(mSurface, NULL, &destRect, 
			converted.data, D3D9Mappings::_getPF(converted.format),
			rowWidth,
			NULL, &srcRect, D3DX_DEFAULT, 0) != D3D_OK)
		{
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "D3DXLoadSurfaceFromMemory failed",
		 		"D3D9HardwarePixelBuffer::blitFromMemory");
		}
	}
	else
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
		
		if(D3DXLoadVolumeFromMemory(mVolume, NULL, &destBox, 
			converted.data, D3D9Mappings::_getPF(converted.format),
			rowWidth, sliceWidth,
			NULL, &srcBox, D3DX_DEFAULT, 0) != D3D_OK)
		{
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "D3DXLoadSurfaceFromMemory failed",
		 		"D3D9HardwarePixelBuffer::blitFromMemory");
		}
	}
	if(mDoMipmapGen)
		_genMipmaps();
}
//-----------------------------------------------------------------------------  
void D3D9HardwarePixelBuffer::blitToMemory(const Image::Box &srcBox, const PixelBox &dst)
{
	// Decide on pixel format of temp surface
	PixelFormat tmpFormat = mFormat; 
	if(D3D9Mappings::_getPF(dst.format) != D3DFMT_UNKNOWN)
	{
		tmpFormat = dst.format;
	}


	if(mSurface)
	{
		assert(srcBox.getDepth() == 1 && dst.getDepth() == 1);
		// Create temp texture
		IDirect3DTexture9 *tmp;
		IDirect3DSurface9 *surface;

		D3DSURFACE_DESC srcDesc;
		if(mSurface->GetDesc(&srcDesc) != D3D_OK)
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
			mpDev,
			dst.getWidth(), dst.getHeight(), 
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
            if(mpDev->GetRenderTargetData(mSurface, surface) == D3D_OK)
			{
				fastLoadSuccess = true;
			}
        }
		if (!fastLoadSuccess)
        {
            if(D3DXLoadSurfaceFromSurface(
                surface, NULL, &destRect, 
                mSurface, NULL, &srcRect,
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
	else
	{
		// Create temp texture
		IDirect3DVolumeTexture9 *tmp;
		IDirect3DVolume9 *surface;
	
		if(D3DXCreateVolumeTexture(
			mpDev,
			dst.getWidth(), dst.getHeight(), dst.getDepth(), 0,
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
			mVolume, NULL, &dsrcBox,
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
void D3D9HardwarePixelBuffer::_genMipmaps()
{
	assert(mMipTex);
	// Mipmapping
	if (mHWMipmaps)
	{
		// Hardware mipmaps
		mMipTex->GenerateMipSubLevels();
	}
	else
	{
		// Software mipmaps
		if( D3DXFilterTexture( mMipTex, NULL, D3DX_DEFAULT, D3DX_DEFAULT ) != D3D_OK )
		{
			OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR, 
			"Failed to filter texture (generate mipmaps)",
			 "D3D9HardwarePixelBuffer::_genMipmaps" );
		}
	}

}
//----------------------------------------------------------------------------- 
void D3D9HardwarePixelBuffer::_setMipmapping(bool doMipmapGen, bool HWMipmaps, IDirect3DBaseTexture9 *mipTex)
{
	mDoMipmapGen = doMipmapGen;
	mHWMipmaps = HWMipmaps;
	mMipTex = mipTex;
}
//-----------------------------------------------------------------------------    
RenderTexture *D3D9HardwarePixelBuffer::getRenderTarget(size_t zoffset)
{
    assert(mUsage & TU_RENDERTARGET);
    assert(zoffset < mDepth);
    return mSliceTRT[zoffset];
}
//-----------------------------------------------------------------------------    
void D3D9HardwarePixelBuffer::createRenderTextures(bool update, bool writeGamma, 
	uint fsaa, const String& fsaaHint, const String& srcName)
{
    if (update)
    {
        assert(mSliceTRT.size() == mDepth);
        for (SliceTRT::const_iterator it = mSliceTRT.begin(); it != mSliceTRT.end(); ++it)
        {
            D3D9RenderTexture *trt = static_cast<D3D9RenderTexture*>(*it);
            trt->rebind(this);
        }
        return;
    }

	destroyRenderTextures();
	if(!mSurface)
	{
		OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR, 
			"Rendering to 3D slices not supported yet for Direct3D",
			 "D3D9HardwarePixelBuffer::createRenderTexture");
	}
	// Create render target for each slice
    mSliceTRT.reserve(mDepth);
	assert(mDepth==1);
    for(size_t zoffset=0; zoffset<mDepth; ++zoffset)
    {
        String name;
		name = "rtt/"+Ogre::StringConverter::toString((size_t)mSurface) + "/" + srcName;
		
        RenderTexture *trt = new D3D9RenderTexture(name, this, writeGamma, fsaa, fsaaHint);
        mSliceTRT.push_back(trt);
        Root::getSingleton().getRenderSystem()->attachRenderTarget(*trt);
    }
}
//-----------------------------------------------------------------------------    
void D3D9HardwarePixelBuffer::destroyRenderTextures()
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

};
