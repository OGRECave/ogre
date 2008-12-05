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

#include <d3d9.h>
#include <d3dx9.h>
namespace Ogre {

	class D3D9HardwarePixelBuffer: public HardwarePixelBuffer
	{
	protected:
		/// Lock a box
		PixelBox lockImpl(const Image::Box lockBox,  LockOptions options);

		/// Unlock a box
		void unlockImpl(void);

		/// Create (or update) render textures for slices
		void createRenderTextures(bool update, bool writeGamma, uint fsaa, const String& fsaaHint, const String& srcName);
		/// Destroy render textures for slices
		void destroyRenderTextures();
		
		/// D3DDevice pointer
		IDirect3DDevice9 *mpDev;
		
		/// Surface abstracted by this buffer
		IDirect3DSurface9 *mSurface;
		/// AA Surface abstracted by this buffer
		IDirect3DSurface9 *mFSAASurface;
		/// Volume abstracted by this buffer
		IDirect3DVolume9 *mVolume;
		/// Temporary surface in main memory if direct locking of mSurface is not possible
		IDirect3DSurface9 *mTempSurface;
		/// Temporary volume in main memory if direct locking of mVolume is not possible
		IDirect3DVolume9 *mTempVolume;
		
		/// Mipmapping
		bool mDoMipmapGen;
		bool mHWMipmaps;
		IDirect3DBaseTexture9 *mMipTex;

		/// Render targets
		typedef std::vector<RenderTexture*> SliceTRT;
        SliceTRT mSliceTRT;
	public:
		D3D9HardwarePixelBuffer(HardwareBuffer::Usage usage);
		
		/// Call this to associate a D3D surface or volume with this pixel buffer
		void bind(IDirect3DDevice9 *dev, IDirect3DSurface9 *mSurface, bool update, 
			bool writeGamma, uint fsaa, const String& fsaaHint, IDirect3DSurface9* fsaaSurface, const String& srcName);
		void bind(IDirect3DDevice9 *dev, IDirect3DVolume9 *mVolume, bool update, 
			bool writeGamma, const String& srcName);
		
		/// @copydoc HardwarePixelBuffer::blit
        void blit(const HardwarePixelBufferSharedPtr &src, const Image::Box &srcBox, const Image::Box &dstBox);
		
		/// @copydoc HardwarePixelBuffer::blitFromMemory
		void blitFromMemory(const PixelBox &src, const Image::Box &dstBox);
		
		/// @copydoc HardwarePixelBuffer::blitToMemory
		void blitToMemory(const Image::Box &srcBox, const PixelBox &dst);
		
		/// Internal function to update mipmaps on update of level 0
		void _genMipmaps();
		
		/// Function to set mipmap generation
		void _setMipmapping(bool doMipmapGen, bool HWMipmaps, IDirect3DBaseTexture9 *mipTex);
		
		~D3D9HardwarePixelBuffer();

		/// Get rendertarget for z slice
		RenderTexture *getRenderTarget(size_t zoffset);

		/// Accessor for surface
		IDirect3DSurface9 *getSurface() { return mSurface; }
		/// Accessor for AA surface
		IDirect3DSurface9 *getFSAASurface() { return mFSAASurface; }

		/// Notify TextureBuffer of destruction of render target
        virtual void _clearSliceRTT(size_t zoffset)
        {
            mSliceTRT[zoffset] = 0;
        }
	};
};
#endif
