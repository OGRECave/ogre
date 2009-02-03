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
