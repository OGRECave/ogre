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
#ifndef __D3D9MULTIRENDERTARGET_H__
#define __D3D9MULTIRENDERTARGET_H__

#include "OgreD3D9Prerequisites.h"
#include "OgreTexture.h"
#include "OgreRenderTexture.h"
#include "OgreImage.h"
#include "OgreException.h"
#include "OgreD3D9HardwarePixelBuffer.h"

#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr9.h>

namespace Ogre {
	class D3D9MultiRenderTarget : public MultiRenderTarget
	{
	public:
		D3D9MultiRenderTarget(const String &name);
		~D3D9MultiRenderTarget();

        virtual void update(bool swapBuffers);

		virtual void getCustomAttribute( const String& name, void *pData );

		bool requiresTextureFlipping() const { return false; }
	private:
		D3D9HardwarePixelBuffer *mRenderTargets[OGRE_MAX_MULTIPLE_RENDER_TARGETS];
		virtual void bindSurfaceImpl(size_t attachment, RenderTexture *target);
		virtual void unbindSurfaceImpl(size_t attachment);

		/** Check surfaces and update RenderTarget extent */
		void checkAndUpdate();
	};
};

#endif
