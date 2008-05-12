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
#ifndef __RenderTexture_H__
#define __RenderTexture_H__

#include "OgrePrerequisites.h"

#include "OgreRenderTarget.h"

namespace Ogre
{    
    /** This class represents a RenderTarget that renders to a Texture. There is no 1 on 1
        relation between Textures and RenderTextures, as there can be multiple 
        RenderTargets rendering to different mipmaps, faces (for cubemaps) or slices (for 3D textures)
        of the same Texture.
    */
    class _OgreExport RenderTexture: public RenderTarget
    {
    public:
        RenderTexture(HardwarePixelBuffer *buffer, size_t zoffset);
        virtual ~RenderTexture();

		virtual void copyContentsToMemory(const PixelBox &dst, FrameBuffer buffer);
		PixelFormat suggestPixelFormat() const;

	protected:
		HardwarePixelBuffer *mBuffer;
		size_t mZOffset;
    };

	/** This class represents a render target that renders to multiple RenderTextures
		at once. Surfaces can be bound and unbound at will, as long as the following constraints
		are met:
		- All bound surfaces have the same size
		- All bound surfaces have the same bit depth
		- Target 0 is bound
	*/
	class _OgreExport MultiRenderTarget: public RenderTarget
	{
	public:
		MultiRenderTarget(const String &name);

		/** Bind a surface to a certain attachment point.
            @param attachment	0 .. mCapabilities->getNumMultiRenderTargets()-1
			@param target		RenderTexture to bind.

			It does not bind the surface and fails with an exception (ERR_INVALIDPARAMS) if:
			- Not all bound surfaces have the same size
			- Not all bound surfaces have the same internal format 
		*/

		virtual void bindSurface(size_t attachment, RenderTexture *target)
		{
			for (size_t i = mBoundSurfaces.size(); i <= attachment; ++i)
			{
				mBoundSurfaces.push_back(0);
			}
			mBoundSurfaces[attachment] = target;

			bindSurfaceImpl(attachment, target);
		}



		/** Unbind attachment.
		*/

		virtual void unbindSurface(size_t attachment)
		{
			if (attachment < mBoundSurfaces.size())
				mBoundSurfaces[attachment] = 0;
			unbindSurfaceImpl(attachment);
		}

		/** Error throwing implementation, it's not possible to write a MultiRenderTarget
			to disk. 
		*/
		virtual void copyContentsToMemory(const PixelBox &dst, FrameBuffer buffer);

		/// Irrelevant implementation since cannot copy
		PixelFormat suggestPixelFormat() const { return PF_UNKNOWN; }

		typedef std::vector<RenderTexture*> BoundSufaceList;
		/// Get a list of the surfaces which have been bound
		const BoundSufaceList& getBoundSurfaceList() const { return mBoundSurfaces; }

		/** Get a pointer to a bound surface */
		RenderTexture* getBoundSurface(size_t index)
		{
			assert (index < mBoundSurfaces.size());
			return mBoundSurfaces[index];
		}


	protected:
		BoundSufaceList mBoundSurfaces;

		/// implementation of bindSurface, must be provided
		virtual void bindSurfaceImpl(size_t attachment, RenderTexture *target) = 0;
		/// implementation of unbindSurface, must be provided
		virtual void unbindSurfaceImpl(size_t attachment) = 0;


	};
}

#endif
