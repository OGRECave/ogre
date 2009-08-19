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
#ifndef __D3D9PIXELBUFFER_H__
#define __D3D9PIXELBUFFER_H__

#include "OgreD3D9Prerequisites.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreD3D9Resource.h"

#include <d3d9.h>
#include <d3dx9.h>
namespace Ogre {

	class D3D9Texture;
	class D3D9RenderTexture;

	class D3D9HardwarePixelBuffer: public HardwarePixelBuffer, public D3D9Resource
	{
	protected:		
		struct BufferResources
		{			
			/// Surface abstracted by this buffer
			IDirect3DSurface9* surface;
			/// AA Surface abstracted by this buffer
			IDirect3DSurface9* fSAASurface;
			/// Volume abstracted by this buffer
			IDirect3DVolume9* volume;
			/// Temporary surface in main memory if direct locking of mSurface is not possible
			IDirect3DSurface9* tempSurface;
			/// Temporary volume in main memory if direct locking of mVolume is not possible
			IDirect3DVolume9* tempVolume;
			/// Mip map texture.
			IDirect3DBaseTexture9 *mipTex;			
		};

		typedef map<IDirect3DDevice9*, BufferResources*>::type	DeviceToBufferResourcesMap;
		typedef DeviceToBufferResourcesMap::iterator			DeviceToBufferResourcesIterator;

		/// Map between device to buffer resources.
		DeviceToBufferResourcesMap	mMapDeviceToBufferResources;
				
		/// Mipmapping
		bool mDoMipmapGen;
		bool mHWMipmaps;
		
		/// Render target
		D3D9RenderTexture* mRenderTexture;

		// The owner texture if exists.
		D3D9Texture* mOwnerTexture;
		
		// The current lock flags of this surface.
		DWORD mLockFlags;

	protected:
		/// Lock a box
		PixelBox lockImpl(const Image::Box lockBox,  LockOptions options);
		PixelBox lockBuffer(BufferResources* bufferResources, const Image::Box &lockBox, DWORD flags);

		/// Unlock a box
		void unlockImpl(void);
		void unlockBuffer(BufferResources* bufferResources);

		BufferResources* getBufferResources(IDirect3DDevice9* d3d9Device);
		BufferResources* createBufferResources();
	
		/// updates render texture.
		void updateRenderTexture(bool writeGamma, uint fsaa, const String& srcName);
		/// destroy render texture.
		void destroyRenderTexture();

		void blit(const HardwarePixelBufferSharedPtr &src,
				const Image::Box &srcBox, const Image::Box &dstBox, 
				BufferResources* srcBufferResources, 
				BufferResources* dstBufferResources);
		void blitFromMemory(const PixelBox &src, const Image::Box &dstBox, BufferResources* dstBufferResources);

		void blitToMemory(const Image::Box &srcBox, const PixelBox &dst, BufferResources* srcBufferResources, IDirect3DDevice9* d3d9Device);
	
		/// Destroy resources associated with the given device.
		void destroyBufferResources(IDirect3DDevice9* d3d9Device);

	public:
		D3D9HardwarePixelBuffer(HardwareBuffer::Usage usage, 
			D3D9Texture* ownerTexture);
		~D3D9HardwarePixelBuffer();

		/// Call this to associate a D3D surface or volume with this pixel buffer
		void bind(IDirect3DDevice9 *dev, IDirect3DSurface9 *mSurface, IDirect3DSurface9* fsaaSurface,
				  bool writeGamma, uint fsaa, const String& srcName, IDirect3DBaseTexture9 *mipTex);
		void bind(IDirect3DDevice9 *dev, IDirect3DVolume9 *mVolume, IDirect3DBaseTexture9 *mipTex);
		
		/// @copydoc HardwarePixelBuffer::blit
        void blit(const HardwarePixelBufferSharedPtr &src, const Image::Box &srcBox, const Image::Box &dstBox);
		
		/// @copydoc HardwarePixelBuffer::blitFromMemory
		void blitFromMemory(const PixelBox &src, const Image::Box &dstBox);
	
		/// @copydoc HardwarePixelBuffer::blitToMemory
		void blitToMemory(const Image::Box &srcBox, const PixelBox &dst);
		
		/// Internal function to update mipmaps on update of level 0
		void _genMipmaps(IDirect3DBaseTexture9* mipTex);
		
		/// Function to set mipmap generation
		void _setMipmapping(bool doMipmapGen, bool HWMipmaps);
		
		
		/// Get rendertarget for z slice
		RenderTexture *getRenderTarget(size_t zoffset);

		/// Accessor for surface
		IDirect3DSurface9 *getSurface(IDirect3DDevice9* d3d9Device);
		
		/// Accessor for AA surface
		IDirect3DSurface9 *getFSAASurface(IDirect3DDevice9* d3d9Device);

		/// Notify TextureBuffer of destruction of render target
        virtual void _clearSliceRTT(size_t zoffset);

		// Called before the Direct3D device is going to be destroyed.
		virtual void notifyOnDeviceDestroy(IDirect3DDevice9* d3d9Device);

		/// Release surfaces held by this pixel buffer.
		void releaseSurfaces(IDirect3DDevice9* d3d9Device);

	};
};
#endif
