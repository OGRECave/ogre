/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#include "OgreD3D11MultiRenderTarget.h"
#include "OgreD3D11RenderSystem.h"
#include "OgreRoot.h"
#include "OgreD3D11Texture.h"
#include "OgreD3D11HardwarePixelBuffer.h"

namespace Ogre 
{
	//---------------------------------------------------------------------
	D3D11MultiRenderTarget::D3D11MultiRenderTarget(const String &name) :
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
	D3D11MultiRenderTarget::~D3D11MultiRenderTarget()
	{
	}
	//---------------------------------------------------------------------
	void D3D11MultiRenderTarget::bindSurfaceImpl(size_t attachment, RenderTexture *target)
	{
		assert(attachment<OGRE_MAX_MULTIPLE_RENDER_TARGETS);
		/// Get buffer and surface to bind to
		D3D11HardwarePixelBuffer *buffer = 0;
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
					"D3D11MultiRenderTarget::bindSurface"
					);
			}
		}

		targets[attachment] = buffer;
		checkAndUpdate();
	}
	//---------------------------------------------------------------------
	void D3D11MultiRenderTarget::unbindSurfaceImpl(size_t attachment)
	{
		assert(attachment<OGRE_MAX_MULTIPLE_RENDER_TARGETS);
		targets[attachment] = 0;
		checkAndUpdate();
	}
	//---------------------------------------------------------------------
	void D3D11MultiRenderTarget::update(void)
	{
		D3D11RenderSystem* rs = static_cast<D3D11RenderSystem*>(
			Root::getSingleton().getRenderSystem());

		MultiRenderTarget::update();
	}
	//---------------------------------------------------------------------
	void D3D11MultiRenderTarget::getCustomAttribute(const String& name, void *pData)
	{
		if(name == "DDBACKBUFFER")
		{
			ID3D11Texture2D ** pSurf = (ID3D11Texture2D **)pData;
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
	void D3D11MultiRenderTarget::checkAndUpdate()
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

