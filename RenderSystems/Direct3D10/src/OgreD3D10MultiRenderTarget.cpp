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
#include "OgreD3D10MultiRenderTarget.h"
#include "OgreD3D10RenderSystem.h"
#include "OgreRoot.h"
#include "OgreD3D10Texture.h"
#include "OgreD3D10HardwarePixelBuffer.h"

namespace Ogre 
{
	//---------------------------------------------------------------------
	D3D10MultiRenderTarget::D3D10MultiRenderTarget(const String &name) :
	//---------------------------------------------------------------------
	MultiRenderTarget(name)
	{
		/// Clear targets
		for(size_t x=0; x<OGRE_MAX_MULTIPLE_RENDER_TARGETS; ++x)
		{
			targets[x] = 0;
		}
	}
	//---------------------------------------------------------------------
	D3D10MultiRenderTarget::~D3D10MultiRenderTarget()
	{
	}
	//---------------------------------------------------------------------
	void D3D10MultiRenderTarget::bindSurfaceImpl(size_t attachment, RenderTexture *target)
	{
		assert(attachment<OGRE_MAX_MULTIPLE_RENDER_TARGETS);
		/// Get buffer and surface to bind to
		D3D10HardwarePixelBuffer *buffer = 0;
		target->getCustomAttribute("BUFFER", &buffer);
		assert(buffer);

		/// Find first non-null target
		int y;
		for(y=0; y<OGRE_MAX_MULTIPLE_RENDER_TARGETS && !targets[y]; ++y) ;

		if(y!=OGRE_MAX_MULTIPLE_RENDER_TARGETS)
		{
			/// If there is another target bound, compare sizes
			if(targets[y]->getWidth() != buffer->getWidth() ||
				targets[y]->getHeight() != buffer->getHeight() ||
				PixelUtil::getNumElemBits(targets[y]->getFormat()) != 
				PixelUtil::getNumElemBits(buffer->getFormat()))
			{
				OGRE_EXCEPT(
					Exception::ERR_INVALIDPARAMS, 
					"MultiRenderTarget surfaces are not of same size or bit depth", 
					"D3D10MultiRenderTarget::bindSurface"
					);
			}
		}

		targets[attachment] = buffer;
		checkAndUpdate();
	}
	//---------------------------------------------------------------------
	void D3D10MultiRenderTarget::unbindSurfaceImpl(size_t attachment)
	{
		assert(attachment<OGRE_MAX_MULTIPLE_RENDER_TARGETS);
		targets[attachment] = 0;
		checkAndUpdate();
	}
	//---------------------------------------------------------------------
	void D3D10MultiRenderTarget::update(void)
	{
		D3D10RenderSystem* rs = static_cast<D3D10RenderSystem*>(
			Root::getSingleton().getRenderSystem());
		if (rs->isDeviceLost())
			return;

		MultiRenderTarget::update();
	}
	//---------------------------------------------------------------------
	void D3D10MultiRenderTarget::getCustomAttribute(const String& name, void *pData)
	{
		if(name == "DDBACKBUFFER")
		{
			ID3D10Texture2D ** pSurf = (ID3D10Texture2D **)pData;
			/// Transfer surfaces
			for(size_t x=0; x<OGRE_MAX_MULTIPLE_RENDER_TARGETS; ++x)
			{
				if(targets[x])
					pSurf[x] = targets[x]->getParentTexture()->GetTex2D();
			}
			return;
		}
	}
	//---------------------------------------------------------------------
	void D3D10MultiRenderTarget::checkAndUpdate()
	{
		if(targets[0])
		{
			mWidth = static_cast<unsigned int>(targets[0]->getWidth());
			mHeight = static_cast<unsigned int>(targets[0]->getHeight());
		}
		else
		{
			mWidth = 0;
			mHeight = 0;
		}
	}
	//---------------------------------------------------------------------


}

