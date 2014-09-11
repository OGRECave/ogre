/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

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
#include "OgreD3D9DeviceManager.h"
#include "OgreD3D9Device.h"
#include "OgreD3D9RenderSystem.h"
#include "OgreD3D9RenderWindow.h"
#include "OgreD3D9Driver.h"
#include "OgreD3D9DriverList.h"
#include "OgreRoot.h"

namespace Ogre
{
	//---------------------------------------------------------------------
	D3D9DeviceManager::D3D9DeviceManager()
	{
		mActiveDevice = NULL;		
		mActiveRenderWindowDevice = NULL;
	}

	//---------------------------------------------------------------------
	D3D9DeviceManager::~D3D9DeviceManager()
	{
		DeviceIterator itDevice = mRenderDevices.begin();
		while (mRenderDevices.size() > 0)
		{			
			mRenderDevices[0]->destroy();						
		}		

		mActiveDevice = NULL;
		mActiveRenderWindowDevice = NULL;
	}

	//---------------------------------------------------------------------
	void D3D9DeviceManager::setActiveDevice(D3D9Device* device)
	{
		if (mActiveDevice != device)
		{
			mActiveDevice = device;

			D3D9RenderSystem*	renderSystem = static_cast<D3D9RenderSystem*>(Root::getSingleton().getRenderSystem());
			D3D9DriverList*		driverList	 = renderSystem->getDirect3DDrivers();

			// Update the active driver member.
			for (uint i=0; i < driverList->count(); ++i)
			{
				D3D9Driver* currDriver = driverList->item(i);
				if (currDriver->getAdapterNumber() == mActiveDevice->getAdapterNumber())
				{
					renderSystem->mActiveD3DDriver = currDriver;
					break;
				}				
			}	

			// Invalidate active view port.
			renderSystem->mActiveViewport = NULL;
		}						
	}

	//---------------------------------------------------------------------
	D3D9Device* D3D9DeviceManager::getActiveDevice()
	{	
		if (mActiveDevice == NULL)
		{
			OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, 
				"Current active device is NULL !!!", 
				"D3D9RenderSystem::getActiveDevice" );
		}

		return mActiveDevice;		
	}

	//---------------------------------------------------------------------
	void D3D9DeviceManager::setActiveRenderTargetDevice(D3D9Device* device)
	{
		mActiveRenderWindowDevice = device;
		if (mActiveRenderWindowDevice != NULL)		
			setActiveDevice(mActiveRenderWindowDevice);			
	}

	//---------------------------------------------------------------------
	D3D9Device* D3D9DeviceManager::getActiveRenderTargetDevice()
	{
		return mActiveRenderWindowDevice;
	}

	//---------------------------------------------------------------------
	UINT D3D9DeviceManager::getDeviceCount()
	{
		return static_cast<UINT>(mRenderDevices.size());
	}

	//---------------------------------------------------------------------
	D3D9Device* D3D9DeviceManager::getDevice(UINT index)
	{
		return mRenderDevices[index];
	}

	//---------------------------------------------------------------------
	void D3D9DeviceManager::linkRenderWindow(D3D9RenderWindow* renderWindow)
	{		
		D3D9Device* renderDevice;

		// Detach from previous device.
		renderDevice = renderWindow->getDevice();		
		if (renderDevice != NULL)		
			renderDevice->detachRenderWindow(renderWindow);						

		D3D9RenderWindowList renderWindowsGroup;

		// Select new device for this window.		
		renderDevice = selectDevice(renderWindow, renderWindowsGroup);

		// Link the windows group to the new device.
		for (uint i = 0; i < renderWindowsGroup.size(); ++i)
		{
			D3D9RenderWindow* currWindow = renderWindowsGroup[i];

			currWindow->setDevice(renderDevice);
			renderDevice->attachRenderWindow(currWindow);
			renderDevice->setAdapterOrdinalIndex(currWindow, i);
		}
				
		renderDevice->acquire();
		if (mActiveDevice == NULL)			
			setActiveDevice(renderDevice);		
	}

	//---------------------------------------------------------------------
	D3D9Device* D3D9DeviceManager::selectDevice(D3D9RenderWindow* renderWindow, D3D9RenderWindowList& renderWindowsGroup)
	{
		D3D9RenderSystem*		renderSystem	 = static_cast<D3D9RenderSystem*>(Root::getSingleton().getRenderSystem());
		D3D9Device*				renderDevice	 = NULL;	
		IDirect3D9*				direct3D9	     = D3D9RenderSystem::getDirect3D9();
		UINT					nAdapterOrdinal  = D3DADAPTER_DEFAULT;
		D3DDEVTYPE				devType			 = D3DDEVTYPE_HAL;						
		DWORD					extraFlags		 = 0;					
		D3D9DriverList*			driverList = renderSystem->getDirect3DDrivers();
		bool					nvAdapterFound = false;


		// Default group includes at least the given render window.
		renderWindowsGroup.push_back(renderWindow);

		// Case we use nvidia performance HUD, override the device settings. 
		if (renderWindow->isNvPerfHUDEnable())
		{
			// Look for 'NVIDIA NVPerfHUD' adapter (<= v4)
			// or 'NVIDIA PerfHUD' (v5)
			// If it is present, override default settings
			for (UINT adapter=0; adapter < direct3D9->GetAdapterCount(); ++adapter)
			{
				D3D9Driver* currDriver = driverList->item(adapter);
				const D3DADAPTER_IDENTIFIER9& currAdapterIdentifier = currDriver->getAdapterIdentifier();

				if(strstr(currAdapterIdentifier.Description, "PerfHUD") != NULL)
				{
					renderDevice = NULL;
					nAdapterOrdinal = adapter;
					renderSystem->mActiveD3DDriver = currDriver;
					devType = D3DDEVTYPE_REF;
					nvAdapterFound = true;
					break;
				}
			}		
		}

		// No special adapter should be used.
		if (nvAdapterFound == false)
		{
			renderSystem->mActiveD3DDriver = findDriver(renderWindow);
			nAdapterOrdinal = renderSystem->mActiveD3DDriver->getAdapterNumber();

			bool bTryUsingMultiheadDevice = false;

			if (renderWindow->isFullScreen())
			{
				bTryUsingMultiheadDevice = true;
				if (renderSystem->getMultiheadUse() == D3D9RenderSystem::mutAuto)
				{
					OSVERSIONINFO osVersionInfo;
					
					osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
					
					// Case version info failed -> assume we run on XP.
					if (FALSE == GetVersionEx(&osVersionInfo))
					{
						osVersionInfo.dwMajorVersion = 5;
					}

					// XP and below - multi-head will cause artifacts when vsync is on.
					if (osVersionInfo.dwMajorVersion <= 5 && renderWindow->isVSync())
					{
						bTryUsingMultiheadDevice = false;
						LogManager::getSingleton().logMessage("D3D9 : Multi head disabled. It causes horizontal line when used in XP + VSync combination");
					}		

					// Vista and SP1 or SP2 - multi-head device can not be reset - it causes memory corruption.
					if (osVersionInfo.dwMajorVersion == 6 &&
						(_stricmp(osVersionInfo.szCSDVersion, "Service Pack 1") == 0 ||
						 _stricmp(osVersionInfo.szCSDVersion, "Service Pack 2") == 0))

					{
						bTryUsingMultiheadDevice = false;
						LogManager::getSingleton().logMessage("D3D9 : Multi head disabled. It causes application run time crashes when used in Vista + SP 1 or 2 combination");
					}	
				}
				else
				{
					bTryUsingMultiheadDevice = renderSystem->getMultiheadUse() == D3D9RenderSystem::mutYes ? true : false;
				}
			}
			
			
			// Check if we can create a group of render windows 
			// on the same device using the multi-head feature.
			if (bTryUsingMultiheadDevice)
			{
				const D3DCAPS9& targetAdapterCaps = renderSystem->mActiveD3DDriver->getD3D9DeviceCaps();
				D3DCAPS9        masterAdapterCaps;

				// Find the master device caps.
				if (targetAdapterCaps.MasterAdapterOrdinal == targetAdapterCaps.AdapterOrdinal)
				{
					masterAdapterCaps = targetAdapterCaps;
				}
				else
				{
					for (uint i = 0; i < driverList->count(); ++i)
					{
						D3D9Driver* currDriver = driverList->item(i);
						const D3DCAPS9& currDeviceCaps = currDriver->getD3D9DeviceCaps();

						if (currDeviceCaps.AdapterOrdinal == targetAdapterCaps.MasterAdapterOrdinal)
						{
							masterAdapterCaps = currDeviceCaps;
							break;
						}					
					}
				}

				// Case the master adapter can handle multiple adapters.
				if (masterAdapterCaps.NumberOfAdaptersInGroup > 1)
				{				
					// Create empty list of render windows composing this group.
					renderWindowsGroup.resize(masterAdapterCaps.NumberOfAdaptersInGroup);
					for (uint i = 0; i < renderWindowsGroup.size(); ++i)
						renderWindowsGroup[i] = NULL;


					// Assign the current render window to it's place in the group.
					renderWindowsGroup[targetAdapterCaps.AdapterOrdinalInGroup] = renderWindow;


					// For each existing window - check if it belongs to the group.
					for (uint i = 0; i < renderSystem->mRenderWindows.size(); ++i)
					{
						D3D9RenderWindow* currRenderWindow = renderSystem->mRenderWindows[i];

						if (currRenderWindow->isFullScreen())
						{
							D3D9Driver* currDriver = findDriver(currRenderWindow);
							const D3DCAPS9& currDeviceCaps = currDriver->getD3D9DeviceCaps();

							if (currDeviceCaps.MasterAdapterOrdinal == masterAdapterCaps.AdapterOrdinal)
							{
								renderWindowsGroup[currDeviceCaps.AdapterOrdinalInGroup] = currRenderWindow;
								break;
							}
						}									
					}

					bool bDeviceGroupFull = true;


					// Check if render windows group is full and ready to be driven by
					// the master device.
					for (uint i = 0; i < renderWindowsGroup.size(); ++i)
					{
						// This group misses required window -> go back to default.
						if (renderWindowsGroup[i] == NULL)
						{
							bDeviceGroupFull = false;
							renderWindowsGroup.clear();
							renderWindowsGroup.push_back(renderWindow);
							break;
						}					
					}

					// Case device group is full -> we can use multi head device.
					if (bDeviceGroupFull)
					{
						bool validateAllDevices = false;

						for (uint i = 0; i < renderWindowsGroup.size(); ++i)
						{
							D3D9RenderWindow* currRenderWindow = renderWindowsGroup[i];
							D3D9Device* currDevice = currRenderWindow->getDevice();

							// This is the master window
							if (i == 0)
							{
								// If master device exists - just release it.
								if (currDevice != NULL)
								{
									renderDevice = currDevice;
									renderDevice->release();
								}							
							}

							// This is subordinate window.
							else
							{						
								// If subordinate device exists - destroy it.
								if (currDevice != NULL)
								{
									currDevice->destroy();
									validateAllDevices = true;
								}							
							}						
						}

						// In case some device was destroyed - make sure all other devices are valid.
						// A possible scenario is that full screen window has been destroyed and it's handle
						// was used and the shared focus handle. All other devices used this handle and must be
						// recreated using other handles otherwise create device will fail. 
						if (validateAllDevices)
						{
							for (uint i = 0; i < mRenderDevices.size(); ++i)
								mRenderDevices[i]->validateFocusWindow();
						}	
					}				
				}
			}		
		}
		
		

		// Do we want to preserve the FPU mode? Might be useful for scientific apps
		ConfigOptionMap& options = renderSystem->getConfigOptions();
		ConfigOptionMap::iterator opti = options.find("Floating-point mode");
		if (opti != options.end() && opti->second.currentValue == "Consistent")
			extraFlags |= D3DCREATE_FPU_PRESERVE;

#if OGRE_THREAD_SUPPORT == 1
		extraFlags |= D3DCREATE_MULTITHREADED;
#endif


		// Try to find a matching device from current device list.
		if (renderDevice == NULL)
		{
			for (uint i = 0; i < mRenderDevices.size(); ++i)
			{
				D3D9Device* currDevice = mRenderDevices[i];

				if (currDevice->getAdapterNumber() == nAdapterOrdinal &&
					currDevice->getDeviceType() == devType &&
					currDevice->isFullScreen() == renderWindow->isFullScreen())
				{
					renderDevice = currDevice;
					break;
				}			
			}
		}

		// No matching device found -> try reference device type (might have been 
		// previously created as a fallback, but don't change devType because HAL
		// should be preferred on creation)
		if (renderDevice == NULL)
		{
			for (uint i = 0; i < mRenderDevices.size(); ++i)
			{
				D3D9Device* currDevice = mRenderDevices[i];

				if (currDevice->getAdapterNumber() == nAdapterOrdinal &&
					currDevice->getDeviceType() == D3DDEVTYPE_REF)
				{
					renderDevice = currDevice;
					break;
				}			
			}
		}


		// No matching device found -> create new one.
		if (renderDevice == NULL)
		{
			renderDevice = OGRE_NEW D3D9Device(this, nAdapterOrdinal, direct3D9->GetAdapterMonitor(nAdapterOrdinal), devType, extraFlags);
			mRenderDevices.push_back(renderDevice);
			if (mActiveDevice == NULL)			
				setActiveDevice(renderDevice);											
		}				

		return renderDevice;		
	}

	//-----------------------------------------------------------------------
	D3D9Driver* D3D9DeviceManager::findDriver(D3D9RenderWindow* renderWindow)
	{
		D3D9RenderSystem*		renderSystem	 = static_cast<D3D9RenderSystem*>(Root::getSingleton().getRenderSystem());		
		IDirect3D9*				direct3D9	     = D3D9RenderSystem::getDirect3D9();
		UINT					nAdapterOrdinal  = D3DADAPTER_DEFAULT;						
		HMONITOR				hRenderWindowMonitor = NULL;			
		D3D9DriverList*			driverList = renderSystem->getDirect3DDrivers();

		// Find the monitor this render window belongs to.
		hRenderWindowMonitor = MonitorFromWindow(renderWindow->getWindowHandle(), MONITOR_DEFAULTTONEAREST);


		// Find the matching driver using window monitor handle.
		for (uint i = 0; i < driverList->count(); ++i)
		{
			D3D9Driver* currDriver       = driverList->item(i);
			HMONITOR hCurrAdpaterMonitor = direct3D9->GetAdapterMonitor(currDriver->getAdapterNumber());

			if (hCurrAdpaterMonitor == hRenderWindowMonitor)
			{
				return currDriver;				
			}
		}

		return NULL;
	}

	//-----------------------------------------------------------------------
	void D3D9DeviceManager::notifyOnDeviceDestroy(D3D9Device* device)
	{
		if (device != NULL)
		{						
			if (device == mActiveDevice)			
				mActiveDevice = NULL;			

			DeviceIterator itDevice = mRenderDevices.begin();
			while (itDevice != mRenderDevices.end())
			{			
				if (*itDevice == device)
				{					
					OGRE_DELETE device;
					mRenderDevices.erase(itDevice);
					break;
				}												
				++itDevice;
			}

			if (mActiveDevice == NULL)
			{
				DeviceIterator itDevice = mRenderDevices.begin();
				if (itDevice != mRenderDevices.end())				
					mActiveDevice = (*itDevice);
			}
		}
	}

	//---------------------------------------------------------------------
	D3D9Device*	D3D9DeviceManager::getDeviceFromD3D9Device(IDirect3DDevice9* d3d9Device)
	{
		DeviceIterator itDevice = mRenderDevices.begin();
		while (itDevice != mRenderDevices.end())
		{			
			if ((*itDevice)->getD3D9Device() == d3d9Device)
			{					
				return *itDevice;
			}												
			++itDevice;
		}

		return NULL;
	}

	//---------------------------------------------------------------------
	void D3D9DeviceManager::destroyInactiveRenderDevices()
	{
		DeviceIterator itDevice = mRenderDevices.begin();
		while (itDevice != mRenderDevices.end())
		{			
			if ((*itDevice)->getRenderWindowCount() == 0 &&
				(*itDevice)->getLastPresentFrame() + 1 < Root::getSingleton().getNextFrameNumber())
			{		
				if (*itDevice == mActiveRenderWindowDevice)
					setActiveRenderTargetDevice(NULL);
				(*itDevice)->destroy();
				break;
			}												
			++itDevice;
		}
	}

}
