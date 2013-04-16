/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

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

