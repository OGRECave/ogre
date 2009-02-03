/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#ifndef __D3D9Device_H__
#define __D3D9Device_H__

#include "OgreD3D9Prerequisites.h"

namespace Ogre {

	class D3D9RenderWindow;
	class D3D9DeviceManager;

	/** High level interface of Direct3D9 Device.
	Provide useful methods for device handling.
	*/
	class D3D9Device
	{

	// Interface.
	public:
		void					attachRenderWindow		(D3D9RenderWindow* renderWindow);
		void					detachRenderWindow		(D3D9RenderWindow* renderWindow);
	
		bool					acquire					();
		
		void					release					();		
		void					destroy					();		
		
		bool					isDeviceLost			();				
		IDirect3DDevice9*		getD3D9Device			();
					
		UINT					getAdapterNumber		() const;
		D3DDEVTYPE				getDeviceType			() const;
		bool					isMultihead				() const;					
		bool					isAutoDepthStencil		() const;
		
		const D3DCAPS9&			getD3D9DeviceCaps		() const;
		D3DFORMAT				getBackBufferFormat		() const;

		bool					validate				(D3D9RenderWindow* renderWindow);
		void					invalidate				();

		void					present					(D3D9RenderWindow* renderWindow);
		
		IDirect3DSurface9*		getDepthBuffer			(D3D9RenderWindow* renderWindow);
		IDirect3DSurface9*		getBackBuffer			(D3D9RenderWindow* renderWindow);

		uint					getRenderWindowCount	() const;
		D3D9RenderWindow*		getRenderWindow			(uint index);
		uint					getLastPresentFrame		() const { return mLastPresentFrame; }

		void					setAdapterOrdinalIndex  (D3D9RenderWindow* renderWindow, uint adapterOrdinalInGroupIndex);
	
	public:
		D3D9Device	(D3D9DeviceManager* deviceManager,
					 UINT adapterNumber, 
					 HMONITOR hMonitor, 
					 D3DDEVTYPE devType, 
					 DWORD behaviorFlags);
		~D3D9Device	();

	protected:		
		D3D9DeviceManager*				mpDeviceManager;			// The manager of this device instance.
		IDirect3DDevice9*				mpDevice;					// Will hold the device interface.				
		UINT							mAdapterNumber;				// The adapter that this device belongs to.	
		HMONITOR						mMonitor;					// The monitor that this device belongs to.
		D3DDEVTYPE						mDeviceType;				// Device type.	
		static HWND						msSharedFocusWindow;		// The shared focus window in case of multiple full screen render windows.
		HWND							mFocusWindow;				// The focus window this device attached to.			
		DWORD							mBehaviorFlags;				// The behavior of this device.		
		D3DPRESENT_PARAMETERS*			mPresentationParams;		// Presentation parameters which the device was created with. May be
																	// an array of presentation parameters in case of multi-head device.				
		UINT							mPresentationParamsCount;	// Number of presentation parameters elements.		
		D3DCAPS9						mD3D9DeviceCaps;			// Device caps.	
		bool							mD3D9DeviceCapsValid;		// True if device caps initialized.
		bool							mDeviceInvalidated;			// True if device was invalidated.
		bool							mDeviceValid;				// True if device in valid rendering state.			
		D3DDEVICE_CREATION_PARAMETERS	mCreationParams;			// Creation parameters.
		uint							mLastPresentFrame;			// Last frame that this device present method called.

		struct RenderWindowResources
		{
			IDirect3DSwapChain9* 	swapChain;						
			uint					adapterOrdinalInGroupIndex;
			uint					presentParametersIndex;			
			IDirect3DSurface9*	 	backBuffer;
			IDirect3DSurface9*	 	depthBuffer;
			D3DPRESENT_PARAMETERS	presentParameters;				
		};		
		typedef std::map<D3D9RenderWindow*, RenderWindowResources*>	RenderWindowToResorucesMap;
		typedef RenderWindowToResorucesMap::iterator				RenderWindowToResorucesIterator;

		RenderWindowToResorucesMap mMapRenderWindowToResoruces;		// Map between render window to resources.


	protected:
		RenderWindowToResorucesIterator getRenderWindowIterator (D3D9RenderWindow* renderWindow);

		bool					acquire							(D3D9RenderWindow* renderWindow);
		bool					reset							();
		void					updatePresentationParameters	();
		void					updateRenderWindowsIndices		();
		
		void					createD3D9Device				();
		void					releaseD3D9Device				();
		void					releaseRenderWindowResources	(RenderWindowResources* renderWindowResources);
		void					clearDeviceStreams				();
		void					acquireRenderWindowResources	(RenderWindowToResorucesIterator it);		
		void					setupDeviceStates				();
		void					notifyDeviceLost				();

		void					validateFocusWindow				();
		void					validateBackBufferSize			(D3D9RenderWindow* renderWindow);
		bool					validateDisplayMonitor			(D3D9RenderWindow* renderWindow);
		bool					validateDeviceState				(D3D9RenderWindow* renderWindow);
		bool					isSwapChainWindow				(D3D9RenderWindow* renderWindow);
		D3D9RenderWindow*		getPrimaryWindow				();
		void					setSharedWindowHandle			(HWND hSharedHWND);

	private:
		friend class D3D9DeviceManager;
		friend class D3D9RenderSystem;
	};
}
#endif
