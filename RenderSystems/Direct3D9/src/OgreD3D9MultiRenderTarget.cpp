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
#include "OgreD3D9MultiRenderTarget.h"
#include "OgreD3D9HardwarePixelBuffer.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreBitwise.h"
#include "OgreD3D9RenderSystem.h"
#include "OgreRoot.h"
#include "OgreD3D9Device.h"
#include "OgreD3D9DeviceManager.h"

namespace Ogre 
{
	D3D9MultiRenderTarget::D3D9MultiRenderTarget(const String &name):
		MultiRenderTarget(name)
	{
		/// Clear targets
		for(size_t x=0; x<OGRE_MAX_MULTIPLE_RENDER_TARGETS; ++x)
		{
			mRenderTargets[x] = 0;
		}
	}
	D3D9MultiRenderTarget::~D3D9MultiRenderTarget()
	{
	}

	void D3D9MultiRenderTarget::bindSurfaceImpl(size_t attachment, RenderTexture *target)
	{
		assert(attachment<OGRE_MAX_MULTIPLE_RENDER_TARGETS);
		/// Get buffer and surface to bind to
		D3D9HardwarePixelBuffer *buffer = 0;
		target->getCustomAttribute("BUFFER", &buffer);
		assert(buffer);

		/// Find first non-null target
		int y;
		for(y=0; y<OGRE_MAX_MULTIPLE_RENDER_TARGETS && !mRenderTargets[y]; ++y) ;
		
		if(y!=OGRE_MAX_MULTIPLE_RENDER_TARGETS)
		{
			/// If there is another target bound, compare sizes
			if (mRenderTargets[y]->getWidth() != buffer->getWidth() ||
				mRenderTargets[y]->getHeight() != buffer->getHeight())
			{
				OGRE_EXCEPT(
					Exception::ERR_INVALIDPARAMS, 
					"MultiRenderTarget surfaces are not of same size", 
					"D3D9MultiRenderTarget::bindSurface");
			}

			if (!Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_MRT_DIFFERENT_BIT_DEPTHS)
				&& (PixelUtil::getNumElemBits(mRenderTargets[y]->getFormat()) != 
					PixelUtil::getNumElemBits(buffer->getFormat())))
			{
				OGRE_EXCEPT(
					Exception::ERR_INVALIDPARAMS, 
					"MultiRenderTarget surfaces are not of same bit depth and hardware requires it", 
					"D3D9MultiRenderTarget::bindSurface"
				);
			}
		}

		mRenderTargets[attachment] = buffer;
		checkAndUpdate();
	}

	void D3D9MultiRenderTarget::unbindSurfaceImpl(size_t attachment)
	{
		assert(attachment<OGRE_MAX_MULTIPLE_RENDER_TARGETS);
		mRenderTargets[attachment] = 0;
		checkAndUpdate();
	}

    void D3D9MultiRenderTarget::update(bool swapBuffers)
    {     
		D3D9DeviceManager* deviceManager = D3D9RenderSystem::getDeviceManager();       	
		D3D9Device* currRenderWindowDevice = deviceManager->getActiveRenderTargetDevice();

		if (currRenderWindowDevice != NULL)
		{
			if (currRenderWindowDevice->isDeviceLost() == false)
				MultiRenderTarget::update(swapBuffers);
		}
		else
		{
			for (UINT i=0; i < deviceManager->getDeviceCount(); ++i)
			{
				D3D9Device* device = deviceManager->getDevice(i);

				if (device->isDeviceLost() == false)
				{
					deviceManager->setActiveRenderTargetDevice(device);
					MultiRenderTarget::update(swapBuffers);
					deviceManager->setActiveRenderTargetDevice(NULL);
				}								
			}
		}		
    }

	void D3D9MultiRenderTarget::getCustomAttribute(const String& name, void *pData)
    {
		if(name == "DDBACKBUFFER")
        {
            IDirect3DSurface9 ** pSurf = (IDirect3DSurface9 **)pData;
			/// Transfer surfaces
			for(size_t x=0; x<OGRE_MAX_MULTIPLE_RENDER_TARGETS; ++x)
			{
				if(mRenderTargets[x] != NULL)								
					pSurf[x] = mRenderTargets[x]->getSurface(D3D9RenderSystem::getActiveD3D9Device());			
			}
			return;
        }
	}

	void D3D9MultiRenderTarget::checkAndUpdate()
	{
		if(mRenderTargets[0])
		{
			mWidth  = (unsigned int)mRenderTargets[0]->getWidth();
			mHeight = (unsigned int)mRenderTargets[0]->getHeight();
		}
		else
		{
			mWidth = 0;
			mHeight = 0;
		}
	}
}

