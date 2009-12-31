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
#ifndef __D3D9DeviceManager_H__
#define __D3D9DeviceManager_H__

#include "OgreD3D9Prerequisites.h"

namespace Ogre {

	class D3D9Device;
	class D3D9RenderWindow;

	/** Device manager interface.
	*/
	class D3D9DeviceManager
	{	

	// Interface.
	public:		
		void				setActiveDevice					(D3D9Device* device);
		D3D9Device*			getActiveDevice					();
		void				setActiveRenderTargetDevice		(D3D9Device* device);
		D3D9Device*			getActiveRenderTargetDevice		();		
		UINT				getDeviceCount					();
		D3D9Device*			getDevice						(UINT index);			
		void				linkRenderWindow				(D3D9RenderWindow* renderWindow);
		void				destroyInactiveRenderDevices	();
		void				notifyOnDeviceDestroy			(D3D9Device* device);
		D3D9Device*			getDeviceFromD3D9Device			(IDirect3DDevice9* d3d9Device);
		
	public:
		D3D9DeviceManager	();
		~D3D9DeviceManager	();

	protected:		
		typedef vector<D3D9Device*>::type		 DeviceList;
		typedef DeviceList::iterator			 DeviceIterator;
		typedef DeviceList::const_iterator		 ConstDeviceIterator;
		typedef vector<D3D9RenderWindow*>::type  D3D9RenderWindowList;

	protected:
		D3D9Device*			selectDevice		(D3D9RenderWindow* renderWindow, D3D9RenderWindowList& renderWindowsGroup);
		D3D9Driver*			findDriver			(D3D9RenderWindow* renderWindow);

		
		DeviceList								mRenderDevices;		
		D3D9Device*								mActiveDevice;
		D3D9Device*								mActiveRenderWindowDevice;		
	};
}
#endif
