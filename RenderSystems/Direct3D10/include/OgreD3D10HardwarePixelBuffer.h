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
#ifndef __D3D10PIXELBUFFER_H__
#define __D3D10PIXELBUFFER_H__

#include "OgreD3D10Prerequisites.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreD3D10Driver.h"

namespace Ogre {

	class D3D10HardwarePixelBuffer: public HardwarePixelBuffer
	{
	protected:
		/// Lock a box
		PixelBox lockImpl(const Image::Box lockBox,  LockOptions options);

		/// Unlock a box
		void unlockImpl(void);

		/// D3DDevice pointer
		D3D10Device & mDevice;

		D3D10Texture * mParentTexture;
		size_t mSubresourceIndex;

		size_t mFace;

		D3D10_BOX OgreImageBoxToDx10Box(const Image::Box &inBox) const;

		/// Render targets
		typedef vector<RenderTexture*>::type SliceTRT;
		SliceTRT mSliceTRT;
	public:
		D3D10HardwarePixelBuffer(D3D10Texture * parentTexture, D3D10Device & device, size_t subresourceIndex,
			size_t width, size_t height, size_t depth, size_t face, PixelFormat format, HardwareBuffer::Usage usage);

		/// @copydoc HardwarePixelBuffer::blit
		void blit(const HardwarePixelBufferSharedPtr &src, const Image::Box &srcBox, const Image::Box &dstBox);

		/// @copydoc HardwarePixelBuffer::blitFromMemory
		void blitFromMemory(const PixelBox &src, const Image::Box &dstBox);

		/// @copydoc HardwarePixelBuffer::blitToMemory
		void blitToMemory(const Image::Box &srcBox, const PixelBox &dst);

		/// Internal function to update mipmaps on update of level 0
		void _genMipmaps();


		~D3D10HardwarePixelBuffer();

		/// Get rendertarget for z slice
		RenderTexture *getRenderTarget(size_t zoffset);

		/// Notify TextureBuffer of destruction of render target
		virtual void _clearSliceRTT(size_t zoffset)
		{
			mSliceTRT[zoffset] = 0;
		}

		D3D10Texture * getParentTexture() const;
		size_t getSubresourceIndex() const;
	};
};
#endif
