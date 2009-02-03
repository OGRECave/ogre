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
#include "OgreD3D9Device.h"
#include "OgreD3D9DeviceManager.h"
#include "OgreD3D9Driver.h"
#include "OgreD3D9RenderSystem.h"
#include "OgreD3D9ResourceManager.h"
#include "OgreD3D9RenderWindow.h"
#include "OgreRoot.h"
#include "OgreHardwareBufferManager.h"

namespace Ogre
{
	HWND D3D9Device::msSharedFocusWindow = NULL;

	//---------------------------------------------------------------------
	D3D9Device::D3D9Device(D3D9DeviceManager* deviceManager,
		UINT adapterNumber, 
		HMONITOR hMonitor, 
		D3DDEVTYPE devType, 
		DWORD behaviorFlags)
	{
		mpDeviceManager				= deviceManager;
		mpDevice					= NULL;		
		mAdapterNumber				= adapterNumber;
		mMonitor					= hMonitor;
		mDeviceType					= devType;
		mFocusWindow				= NULL;
		mBehaviorFlags				= behaviorFlags;	
		mD3D9DeviceCapsValid		= false;
		mPresentationParamsCount 	= 0;
		mPresentationParams		 	= NULL;	
		mDeviceInvalidated			= false;
		mDeviceValid				= false;
		memset(&mD3D9DeviceCaps, 0, sizeof(mD3D9DeviceCaps));
		memset(&mCreationParams, 0, sizeof(mCreationParams));		
	}

	//---------------------------------------------------------------------
	D3D9Device::~D3D9Device()
	{

	}

	//---------------------------------------------------------------------
	D3D9Device::RenderWindowToResorucesIterator D3D9Device::getRenderWindowIterator(D3D9RenderWindow* renderWindow)
	{
		RenderWindowToResorucesIterator it = mMapRenderWindowToResoruces.find(renderWindow);

		if (it == mMapRenderWindowToResoruces.end())
		{
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"Render window was not attached to this device !!", 
				"D3D9Device::getRenderWindowIterator" );
		}

		return it;
	}


	//---------------------------------------------------------------------
	void D3D9Device::attachRenderWindow(D3D9RenderWindow* renderWindow)
	{
		RenderWindowToResorucesIterator it = mMapRenderWindowToResoruces.find(renderWindow);

		if (it == mMapRenderWindowToResoruces.end())
		{
			RenderWindowResources* renderWindowResources = new RenderWindowResources;

			memset(renderWindowResources, 0, sizeof(RenderWindowResources));
						
			renderWindowResources->adapterOrdinalInGroupIndex = 0;					
			mMapRenderWindowToResoruces[renderWindow] = renderWindowResources;

			// invalidate this device.
			invalidate();
		}
		updateRenderWindowsIndices();
	}

	//---------------------------------------------------------------------
	void D3D9Device::detachRenderWindow(D3D9RenderWindow* renderWindow)
	{
		RenderWindowToResorucesIterator it = mMapRenderWindowToResoruces.find(renderWindow);

		if (it != mMapRenderWindowToResoruces.end())
		{		
			// The focus window in which the d3d9 device created on is detached.
			// resources must be acquired again.
			if (mFocusWindow == renderWindow->getWindowHandle())
			{
				mFocusWindow = NULL;				
			}

			// Case this is the shared focus window.
			if (renderWindow->getWindowHandle() == msSharedFocusWindow)			
				setSharedWindowHandle(NULL);		
			
			RenderWindowResources* renderWindowResources = it->second;

			releaseRenderWindowResources(renderWindowResources);

			SAFE_DELETE(renderWindowResources);
			
			mMapRenderWindowToResoruces.erase(it);		
		}
		updateRenderWindowsIndices();
	}

	//---------------------------------------------------------------------
	bool D3D9Device::acquire()
	{	
		updatePresentationParameters();
			
		// Create device if need to.
		if (mpDevice == NULL)
		{			
			createD3D9Device();
		}
			
		// Update resources of each window.
		RenderWindowToResorucesIterator it = mMapRenderWindowToResoruces.begin();

		while (it != mMapRenderWindowToResoruces.end())
		{
			acquireRenderWindowResources(it);
			++it;
		}
		
		return true;
	}

	//---------------------------------------------------------------------
	void D3D9Device::release()
	{
		if (mpDevice != NULL)
		{
			D3D9RenderSystem* renderSystem = static_cast<D3D9RenderSystem*>(Root::getSingleton().getRenderSystem());

			// Clean up depth stencil surfaces
			renderSystem->_cleanupDepthStencils(mpDevice);	

			RenderWindowToResorucesIterator it = mMapRenderWindowToResoruces.begin();

			while (it != mMapRenderWindowToResoruces.end())
			{
				RenderWindowResources* renderWindowResources = it->second;

				releaseRenderWindowResources(renderWindowResources);
				++it;
			}

			releaseD3D9Device();
		}				
	}

	//---------------------------------------------------------------------
	bool D3D9Device::acquire(D3D9RenderWindow* renderWindow)
	{
		RenderWindowToResorucesIterator it = getRenderWindowIterator(renderWindow);
		
		acquireRenderWindowResources(it);

		return true;
	}

	//---------------------------------------------------------------------	
	void D3D9Device::notifyDeviceLost()
	{
		// Case this device is already in invalid rendering state.
		if (mDeviceValid == false)
			return;

		// Case we just moved from valid state to invalid state.
		mDeviceValid = false;	
		
		D3D9RenderSystem* renderSystem = static_cast<D3D9RenderSystem*>(Root::getSingleton().getRenderSystem());
		
		renderSystem->notifyOnDeviceLost(this);
	}	

	//---------------------------------------------------------------------
	IDirect3DSurface9* D3D9Device::getDepthBuffer(D3D9RenderWindow* renderWindow)
	{
		RenderWindowToResorucesIterator it = getRenderWindowIterator(renderWindow);		

		return it->second->depthBuffer;
	}

	//---------------------------------------------------------------------
	IDirect3DSurface9* D3D9Device::getBackBuffer(D3D9RenderWindow* renderWindow)
	{
		RenderWindowToResorucesIterator it = getRenderWindowIterator(renderWindow);
	
		return it->second->backBuffer;		
	}

	//---------------------------------------------------------------------
	uint D3D9Device::getRenderWindowCount() const
	{
		return static_cast<uint>(mMapRenderWindowToResoruces.size());
	}

	//---------------------------------------------------------------------
	D3D9RenderWindow* D3D9Device::getRenderWindow(uint index)
	{
		if (index >= mMapRenderWindowToResoruces.size())
		{
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"Index of render window is out of bounds!", 
				"D3D9RenderWindow::getRenderWindow");
		}
		
		RenderWindowToResorucesIterator it = mMapRenderWindowToResoruces.begin();

		while (it != mMapRenderWindowToResoruces.end())
		{
			if (index == 0)			
				break;			
			
			--index;
			++it;
		}
		
		return it->first;
	}

	//---------------------------------------------------------------------
	void D3D9Device::setAdapterOrdinalIndex(D3D9RenderWindow* renderWindow, uint adapterOrdinalInGroupIndex)
	{
		RenderWindowToResorucesIterator it = getRenderWindowIterator(renderWindow);

		it->second->adapterOrdinalInGroupIndex = adapterOrdinalInGroupIndex;

		updateRenderWindowsIndices();
	}
	
	//---------------------------------------------------------------------
	void D3D9Device::destroy()
	{	
		release();
		
		RenderWindowToResorucesIterator it = mMapRenderWindowToResoruces.begin();

		if (it != mMapRenderWindowToResoruces.end())
		{	
			if (it->first->getWindowHandle() == msSharedFocusWindow)
				setSharedWindowHandle(NULL);

			SAFE_DELETE(it->second);
			++it;
		}
		mMapRenderWindowToResoruces.clear();
		
		// Reset dynamic attributes.		
		mFocusWindow			= NULL;		
		mD3D9DeviceCapsValid	= false;
		SAFE_DELETE_ARRAY(mPresentationParams);
		mPresentationParamsCount = 0;

		// Notify the device manager on this instance destruction.	
		mpDeviceManager->notifyOnDeviceDestroy(this);
	}	
	
	//---------------------------------------------------------------------
	bool D3D9Device::isDeviceLost()
	{		
		HRESULT hr;

		hr = mpDevice->TestCooperativeLevel();

		if (hr == D3DERR_DEVICELOST ||
			hr == D3DERR_DEVICENOTRESET)
		{
			return true;
		}
		
		return false;
	}

	//---------------------------------------------------------------------
	bool D3D9Device::reset()
	{
		updatePresentationParameters();

		RenderWindowToResorucesIterator it = mMapRenderWindowToResoruces.begin();

		while (it != mMapRenderWindowToResoruces.end())
		{
			RenderWindowResources* renderWindowResources = it->second;

			releaseRenderWindowResources(renderWindowResources);
			++it;
		}

		clearDeviceStreams();

		// Reset the device
		HRESULT hr;

		
		// Check that device is in valid state for reset.
		hr = mpDevice->TestCooperativeLevel();
		if (hr == D3DERR_DEVICELOST ||
			hr == D3DERR_DRIVERINTERNALERROR)
		{
			return false;
		}

		D3D9RenderSystem* renderSystem = static_cast<D3D9RenderSystem*>(Root::getSingleton().getRenderSystem());


		// Release all automatic temporary buffers and free unused
		// temporary buffers, so we doesn't need to recreate them,
		// and they will reallocate on demand. This save a lot of
		// release/recreate of non-managed vertex buffers which
		// wasn't need at all.
		HardwareBufferManager::getSingleton()._releaseBufferCopies(true);


		// Cleanup depth stencils surfaces.
		renderSystem->_cleanupDepthStencils(mpDevice);



		// Inform all resources that device lost.
		D3D9RenderSystem::getResourceManager()->notifyOnDeviceLost(mpDevice);
		

		// Reset the device using the presentation parameters.
		hr = mpDevice->Reset(mPresentationParams);
	
		if (hr == D3DERR_DEVICELOST)
		{
			// Don't continue
			return false;
		}
		else if (FAILED(hr))
		{
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"Cannot reset device!", 
				"D3D9RenderWindow::reset");
		}

		mDeviceInvalidated = false;
		mDeviceValid = true;

		// Initialize device states.
		setupDeviceStates();

		// Update resources of each window.
		it = mMapRenderWindowToResoruces.begin();

		while (it != mMapRenderWindowToResoruces.end())
		{
			acquireRenderWindowResources(it);
			++it;
		}				

		// Inform all resources that device has been reset.
		D3D9RenderSystem::getResourceManager()->notifyOnDeviceReset(mpDevice);

	
		renderSystem->notifyOnDeviceReset(this);
	
		return true;
	}

	//---------------------------------------------------------------------
	bool D3D9Device::isAutoDepthStencil() const
	{
		const D3DPRESENT_PARAMETERS& primaryPresentationParams = mPresentationParams[0];

		// Check if auto depth stencil can be used.
		for (unsigned int i = 1; i < mPresentationParamsCount; i++)
		{			
			// disable AutoDepthStencil if these parameters are not all the same.
			if(primaryPresentationParams.BackBufferHeight != mPresentationParams[i].BackBufferHeight || 
				primaryPresentationParams.BackBufferWidth != mPresentationParams[i].BackBufferWidth	|| 
				primaryPresentationParams.BackBufferFormat != mPresentationParams[i].BackBufferFormat || 
				primaryPresentationParams.AutoDepthStencilFormat != mPresentationParams[i].AutoDepthStencilFormat)
			{
				return false;
			}
		}

		return true;
	}

	//---------------------------------------------------------------------
	const D3DCAPS9& D3D9Device::getD3D9DeviceCaps() const
	{
		if (mD3D9DeviceCapsValid == false)
		{
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"Device caps are invalid!", 
				"D3D9Device::getD3D9DeviceCaps" );
		}

		return mD3D9DeviceCaps;
	}

	//---------------------------------------------------------------------
	D3DFORMAT D3D9Device::getBackBufferFormat() const
	{
		if (mPresentationParams == NULL || mPresentationParamsCount == 0)
		{
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"Presentation parameters are invalid!", 
				"D3D9Device::getBackBufferFormat" );
		}

		return mPresentationParams[0].BackBufferFormat;
	}

	//---------------------------------------------------------------------
	IDirect3DDevice9* D3D9Device::getD3D9Device()
	{
		return mpDevice;
	}

	//---------------------------------------------------------------------
	void D3D9Device::updatePresentationParameters()
	{
		// Clear old presentation parameters.
		SAFE_DELETE_ARRAY(mPresentationParams);
		mPresentationParamsCount = 0;		

		if (mMapRenderWindowToResoruces.size() > 0)
		{
			mPresentationParams = new D3DPRESENT_PARAMETERS[mMapRenderWindowToResoruces.size()];

			RenderWindowToResorucesIterator it = mMapRenderWindowToResoruces.begin();

			while (it != mMapRenderWindowToResoruces.end())
			{
				D3D9RenderWindow* renderWindow = it->first;
				RenderWindowResources* renderWindowResources = it->second;

				// Ask the render window to build it's own parameters.
				renderWindow->buildPresentParameters(&renderWindowResources->presentParameters);
				

				// Update shared focus window handle.
				if (renderWindow->isFullScreen() && 
					renderWindowResources->presentParametersIndex == 0 &&
					msSharedFocusWindow == NULL)
					setSharedWindowHandle(renderWindow->getWindowHandle());					

				// This is the primary window or a full screen window that is part of multi head device.
				if (renderWindowResources->presentParametersIndex == 0 ||
					renderWindow->isFullScreen())
				{
					mPresentationParams[renderWindowResources->presentParametersIndex] = renderWindowResources->presentParameters;
					mPresentationParamsCount++;
				}
															
				++it;
			}
		}

		// Case we have to cancel auto depth stencil.
		if (isMultihead() && isAutoDepthStencil() == false)
		{
			for(unsigned int i = 0; i < mPresentationParamsCount; i++)
			{
				mPresentationParams[i].EnableAutoDepthStencil = false;
			}
		}
	}	

	//---------------------------------------------------------------------
	UINT D3D9Device::getAdapterNumber() const
	{
		return mAdapterNumber;
	}

	//---------------------------------------------------------------------
	D3DDEVTYPE D3D9Device::getDeviceType() const
	{
		return mDeviceType;
	}

	//---------------------------------------------------------------------
	bool D3D9Device::isMultihead() const
	{
		RenderWindowToResorucesMap::const_iterator it = mMapRenderWindowToResoruces.begin();

		while (it != mMapRenderWindowToResoruces.end())				
		{
			RenderWindowResources* renderWindowResources = it->second;
			
			if (renderWindowResources->adapterOrdinalInGroupIndex > 0 &&
				it->first->isFullScreen())
			{
				return true;
			}
			
			++it;		
		}

		return false;
	}

	//---------------------------------------------------------------------
	void D3D9Device::clearDeviceStreams()
	{
		// Set all texture units to nothing to release texture surfaces
		for (DWORD stage = 0; stage < mD3D9DeviceCaps.MaxSimultaneousTextures; ++stage)
		{
			mpDevice->SetTexture(stage, NULL);
		}

		// Unbind any vertex streams to avoid memory leaks				
		for (unsigned int i = 0; i < mD3D9DeviceCaps.MaxStreams; ++i)
		{
			mpDevice->SetStreamSource(i, NULL, 0, 0);
		}
	}

	//---------------------------------------------------------------------
	void D3D9Device::createD3D9Device()
	{		
		// Update focus window.
		D3D9RenderWindow* primaryRenderWindow = getPrimaryWindow();

		// Case we have to share the same focus window.
		if (msSharedFocusWindow != NULL)
			mFocusWindow = msSharedFocusWindow;
		else
			mFocusWindow = primaryRenderWindow->getWindowHandle();		

		IDirect3D9* pD3D9 = D3D9RenderSystem::getDirect3D9();
		HRESULT     hr;


		if (isMultihead())
		{
			mBehaviorFlags |= D3DCREATE_ADAPTERGROUP_DEVICE;
		}		
		else
		{
			mBehaviorFlags &= ~D3DCREATE_ADAPTERGROUP_DEVICE;
		}

		// Try to create the device with hardware vertex processing. 
		mBehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
		hr = pD3D9->CreateDevice(mAdapterNumber, mDeviceType, mFocusWindow,
			mBehaviorFlags, mPresentationParams, &mpDevice);

		if (FAILED(hr))
		{
			// Try a second time, may fail the first time due to back buffer count,
			// which will be corrected down to 1 by the runtime
			hr = pD3D9->CreateDevice(mAdapterNumber, mDeviceType, mFocusWindow,
				mBehaviorFlags, mPresentationParams, &mpDevice);
		}

		// Case hardware vertex processing failed.
		if( FAILED( hr ) )
		{
			// Try to create the device with mixed vertex processing.
			mBehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
			mBehaviorFlags |= D3DCREATE_MIXED_VERTEXPROCESSING;

			hr = pD3D9->CreateDevice(mAdapterNumber, mDeviceType, mFocusWindow,
				mBehaviorFlags, mPresentationParams, &mpDevice);

			if( FAILED( hr ) )
			{
				// Last thing we try to create the device with software vertex processing.
				mBehaviorFlags &= ~D3DCREATE_MIXED_VERTEXPROCESSING;
				mBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
				hr = pD3D9->CreateDevice(mAdapterNumber, mDeviceType, mFocusWindow,
					mBehaviorFlags, mPresentationParams, &mpDevice);

				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
					"Cannot create device!", 
					"D3D9Device::createD3D9Device" );
			}
		}

		// Get current device caps.
		hr = mpDevice->GetDeviceCaps(&mD3D9DeviceCaps);
		if( FAILED( hr ) )
		{
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"Cannot get device caps!", 
				"D3D9Device::createD3D9Device" );
		}

		// Get current creation parameters caps.
		hr = mpDevice->GetCreationParameters(&mCreationParams);
		if ( FAILED(hr) )
		{
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"Error Get Creation Parameters", 
				"D3D9Device:createD3D9Device" );
		}

		mD3D9DeviceCapsValid = true;
		mDeviceValid = true;

		// Inform all resources that new device created.
		D3D9RenderSystem::getResourceManager()->notifyOnDeviceCreate(mpDevice);

		// Initialize device states.
		setupDeviceStates();
	}

	//---------------------------------------------------------------------
	void D3D9Device::releaseD3D9Device()
	{
		if (mpDevice != NULL)
		{
			// Inform all resources that device is going to be destroyed.
			D3D9RenderSystem::getResourceManager()->notifyOnDeviceDestroy(mpDevice);

			clearDeviceStreams();

			// Release device.
			SAFE_RELEASE(mpDevice);	
		}
	}

	//---------------------------------------------------------------------
	void D3D9Device::releaseRenderWindowResources(RenderWindowResources* renderWindowResources)
	{
		SAFE_RELEASE(renderWindowResources->backBuffer);
		SAFE_RELEASE(renderWindowResources->depthBuffer);
		SAFE_RELEASE(renderWindowResources->swapChain);
	}

	//---------------------------------------------------------------------
	void D3D9Device::invalidate()
	{
		mDeviceInvalidated = true;
	}

	//---------------------------------------------------------------------
	bool D3D9Device::validate(D3D9RenderWindow* renderWindow)
	{
		// Validate that this device created on the correct target focus window handle		
		validateFocusWindow();		
		
		// Validate that the render window dimensions matches to back buffer dimensions.
		validateBackBufferSize(renderWindow);

		// Validate that the render window should run on this device.
		if (false == validateDisplayMonitor(renderWindow))
			return false;
			
		// Validate that this device is in valid rendering state.
		if (false == validateDeviceState(renderWindow))
			return false;

		return true;
	}

	//---------------------------------------------------------------------
	void D3D9Device::validateFocusWindow()
	{
		if ((msSharedFocusWindow != NULL && mCreationParams.hFocusWindow != msSharedFocusWindow) ||
			(msSharedFocusWindow == NULL && mCreationParams.hFocusWindow != getPrimaryWindow()->getWindowHandle()))
		{
			release();
			acquire();
		}
	}

	//---------------------------------------------------------------------
	bool D3D9Device::validateDeviceState(D3D9RenderWindow* renderWindow)
	{
		HRESULT hr;

		hr = mpDevice->TestCooperativeLevel();	

		// Case device is not valid for rendering. 
		if (mDeviceInvalidated || FAILED(hr))
		{					
			RenderWindowToResorucesIterator it = getRenderWindowIterator(renderWindow);		
			RenderWindowResources* renderWindowResources =  it->second;

			// device lost, and we can't reset
			// can't do anything about it here, wait until we get 
			// D3DERR_DEVICENOTRESET; rendering calls will silently fail until 
			// then (except Present, but we ignore device lost there too)
			if (hr == D3DERR_DEVICELOST)
			{			
				SAFE_RELEASE(renderWindowResources->backBuffer);
				SAFE_RELEASE (renderWindowResources->depthBuffer);
				Sleep(50);

				mDeviceValid = false;

				return false;
			}

			// device lost, and we can reset
			else if (hr == D3DERR_DEVICENOTRESET || mDeviceInvalidated)
			{												
				bool deviceRestored = reset();

				// Still lost?
				if (deviceRestored == false)
				{
					// Wait a while
					Sleep(50);

					mDeviceValid = false;

					return false;
				}				
			}
		}

		return true;
	}
		

	//---------------------------------------------------------------------
	void D3D9Device::validateBackBufferSize(D3D9RenderWindow* renderWindow)
	{
		RenderWindowToResorucesIterator it = getRenderWindowIterator(renderWindow);
		RenderWindowResources*	renderWindowResources = it->second;
	

		// Case size has been changed.
		if (renderWindow->getWidth() != renderWindowResources->presentParameters.BackBufferWidth ||
			renderWindow->getHeight() != renderWindowResources->presentParameters.BackBufferHeight)
		{			
			if (renderWindow->getWidth() > 0)
				renderWindowResources->presentParameters.BackBufferWidth = renderWindow->getWidth();

			if (renderWindow->getHeight() > 0)
				renderWindowResources->presentParameters.BackBufferHeight = renderWindow->getHeight();

			invalidate();
		}				
	}

	//---------------------------------------------------------------------
	bool D3D9Device::validateDisplayMonitor(D3D9RenderWindow* renderWindow)
	{
		// Ignore full screen since it doesn't really move and it is possible 
		// that it created using multi-head adapter so for a subordinate the
		// native monitor handle and this device handle will be different.
		if (renderWindow->isFullScreen())
			return true;

		RenderWindowToResorucesIterator it = getRenderWindowIterator(renderWindow);
		HMONITOR	hRenderWindowMonitor = NULL;

		// Find the monitor this render window belongs to.
		hRenderWindowMonitor = MonitorFromWindow(renderWindow->getWindowHandle(), MONITOR_DEFAULTTONULL);

		// This window doesn't intersect with any of the display monitor
		if (hRenderWindowMonitor == NULL)		
			return false;		
		

		// Case this window changed monitor.
		if (hRenderWindowMonitor != mMonitor)
		{			
			mpDeviceManager->linkRenderWindow(renderWindow);
			return false;
		}

		return true;
	}

	//---------------------------------------------------------------------
	void D3D9Device::present(D3D9RenderWindow* renderWindow)
	{
		if (mDeviceValid == false || isDeviceLost())		
			return;		
		
		RenderWindowToResorucesIterator it = getRenderWindowIterator(renderWindow);
		RenderWindowResources*	renderWindowResources = it->second;					
		HRESULT hr;

		if (isMultihead())
		{
			// Only the master will call present method results in synchronized
			// buffer swap for the rest of the implicit swap chain.
			if (getPrimaryWindow() == renderWindow)
				hr = mpDevice->Present( NULL, NULL, 0, NULL );
			else
				hr = S_OK;
		}
		else
		{
			hr = renderWindowResources->swapChain->Present(NULL, NULL, NULL, NULL, 0);			
		}


		if( D3DERR_DEVICELOST == hr || mDeviceInvalidated)
		{
			SAFE_RELEASE(renderWindowResources->depthBuffer);
			SAFE_RELEASE(renderWindowResources->backBuffer);
			SAFE_RELEASE(renderWindowResources->swapChain);

			notifyDeviceLost();
		}
		else if( FAILED(hr) )
		{
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"Error Presenting surfaces", 
				"D3D9Device::present" );
		}
		else
			mLastPresentFrame = Root::getSingleton().getNextFrameNumber();
	}

	//---------------------------------------------------------------------
	void D3D9Device::acquireRenderWindowResources(RenderWindowToResorucesIterator it)
	{
		RenderWindowResources*	renderWindowResources = it->second;
		D3D9RenderWindow*		renderWindow = it->first;			
		
		releaseRenderWindowResources(renderWindowResources);

		// No need to create resources for invisible windows.
		if (renderWindow->getWidth() == 0 || 
			renderWindow->getHeight() == 0)
		{
			return;
		}

		// Create additional swap chain
		if (isSwapChainWindow(renderWindow) && !isMultihead())
		{
			// Create swap chain
			HRESULT hr = mpDevice->CreateAdditionalSwapChain(&renderWindowResources->presentParameters, 
				&renderWindowResources->swapChain);

			if (FAILED(hr))
			{
				// Try a second time, may fail the first time due to back buffer count,
				// which will be corrected by the runtime
				hr = mpDevice->CreateAdditionalSwapChain(&renderWindowResources->presentParameters, 
					&renderWindowResources->swapChain);
			}

			if (FAILED(hr))
			{
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
					"Unable to create an additional swap chain",
					"D3D9RenderWindow::acquireRenderWindowResources");
			}
		}
		else
		{
			// The swap chain is already created by the device
			HRESULT hr = mpDevice->GetSwapChain(renderWindowResources->presentParametersIndex, 
				&renderWindowResources->swapChain);
			if (FAILED(hr)) 
			{
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
					"Unable to get the swap chain",
					"D3D9RenderWindow::acquireRenderWindowResources");
			}
		}

		// Store references to buffers for convenience		
		renderWindowResources->swapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, 
			&renderWindowResources->backBuffer);

		// Additional swap chains need their own depth buffer
		// to support resizing them
		if (renderWindow->isDepthBuffered()) 
		{
			// if multihead is enabled, depth buffer can be created automatically for 
			// all the adapters. if multihead is not enabled, depth buffer is just
			// created for the main swap chain
			if (isMultihead() && isAutoDepthStencil() || 
			    isMultihead() == false && isSwapChainWindow(renderWindow) == false)
			{
				mpDevice->GetDepthStencilSurface(&renderWindowResources->depthBuffer);
			}
			else
			{
				HRESULT hr = mpDevice->CreateDepthStencilSurface(
					renderWindow->getWidth(), renderWindow->getHeight(),
					renderWindowResources->presentParameters.AutoDepthStencilFormat,
					renderWindowResources->presentParameters.MultiSampleType,
					renderWindowResources->presentParameters.MultiSampleQuality, 
					(renderWindowResources->presentParameters.Flags & D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL),
					&renderWindowResources->depthBuffer, NULL
					);

				if (FAILED(hr)) 
				{
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
						"Unable to create a depth buffer for the swap chain",
						"D3D9RenderWindow::acquireRenderWindowResources");
				}

				if (isSwapChainWindow(renderWindow) == false)
				{
					mpDevice->SetDepthStencilSurface(renderWindowResources->depthBuffer);
				}
			}
		}		
	}

	//---------------------------------------------------------------------
	void D3D9Device::setupDeviceStates()
	{
		HRESULT hr = mpDevice->SetRenderState(D3DRS_SPECULARENABLE, TRUE);
		
		if (FAILED(hr)) 
		{
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"Unable to apply render state: D3DRS_SPECULARENABLE <- TRUE",
				"D3D9Device::setupDeviceStates");
		}		
	}

	//---------------------------------------------------------------------
	bool D3D9Device::isSwapChainWindow(D3D9RenderWindow* renderWindow)
	{
		RenderWindowToResorucesIterator it = getRenderWindowIterator(renderWindow);
		
		if (it->second->presentParametersIndex == 0 || renderWindow->isFullScreen())			
			return false;
			
		return true;
	}

	//---------------------------------------------------------------------
	D3D9RenderWindow* D3D9Device::getPrimaryWindow()
	{		
		RenderWindowToResorucesIterator it = mMapRenderWindowToResoruces.begin();
	
		while (it != mMapRenderWindowToResoruces.end() && it->second->presentParametersIndex != 0)				
			++it;		

		assert(it != mMapRenderWindowToResoruces.end());

		return it->first;
	}

	//---------------------------------------------------------------------
	void D3D9Device::setSharedWindowHandle(HWND hSharedHWND)
	{
		if (hSharedHWND != msSharedFocusWindow)					
			msSharedFocusWindow = hSharedHWND;					
	}

	//---------------------------------------------------------------------
	void D3D9Device::updateRenderWindowsIndices()
	{
		// Update present parameters index attribute per render window.
		if (isMultihead())
		{
			RenderWindowToResorucesIterator it = mMapRenderWindowToResoruces.begin();

			// Multi head device case -  
			// Present parameter index is based on adapter ordinal in group index.
			while (it != mMapRenderWindowToResoruces.end())			
			{
				it->second->presentParametersIndex = it->second->adapterOrdinalInGroupIndex;
				++it;
			}
		}
		else
		{
			// Single head device case - 
			// Just assign index in incremental order - 
			// NOTE: It suppose to cover both cases that possible here:
			// 1. Single full screen window - only one allowed per device (this is not multi-head case).
			// 2. Multiple window mode windows.

			uint nextPresParamIndex = 0;

			RenderWindowToResorucesIterator it;
			D3D9RenderWindow* deviceFocusWindow = NULL;

			// In case a d3d9 device exists - try to keep the present parameters order
			// so that the window that the device is focused on will stay the same and we
			// will avoid device re-creation.
			if (mpDevice != NULL)
			{
				it = mMapRenderWindowToResoruces.begin();
				while (it != mMapRenderWindowToResoruces.end())			
				{
					if (it->first->getWindowHandle() == mCreationParams.hFocusWindow)
					{
						deviceFocusWindow = it->first;
						it->second->presentParametersIndex = nextPresParamIndex;
						++nextPresParamIndex;
						break;
					}					
					++it;
				}
			}
			
		

			it = mMapRenderWindowToResoruces.begin();
			while (it != mMapRenderWindowToResoruces.end())			
			{
				if (it->first != deviceFocusWindow)
				{
					it->second->presentParametersIndex = nextPresParamIndex;
					++nextPresParamIndex;
				}								
				++it;
			}
		}
	}
}
