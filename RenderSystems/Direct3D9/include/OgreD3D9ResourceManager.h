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
#ifndef __D3D9ResourceManager_H__
#define __D3D9ResourceManager_H__

#include "OgreD3D9Prerequisites.h"

namespace Ogre {

	enum D3D9ResourceCreationPolicy
	{
		// Creates the rendering resource on the current active device that needs it.
		// This policy should be used when video memory consumption should be minimized but
		// it might cause some performance issues when using loader resource thread since
		// a resource that was not created on specified device will be created on demand during
		// the rendering process and might cause the FPS to drop down. 
		RCP_CREATE_ON_ACTIVE_DEVICE,

		// Create the rendering resource on every existing device. 		
		// This policy should be used when working intensively with a background loader thread.
		// In that case when multiple devices exist, the resource will be created on each device
		// and will be ready to use in the rendering thread. 
		// The draw back can be some video memory waste in case that each device render different
		// scene and doesn't really need all the resources.
		RCP_CREATE_ON_ALL_DEVICES
	};
	
	class D3D9ResourceManager
	{

	// Interface.
	public:

		// Called immediately after the Direct3D device has been created.
		void notifyOnDeviceCreate	(IDirect3DDevice9* d3d9Device);

		// Called before the Direct3D device is going to be destroyed.
		void notifyOnDeviceDestroy	(IDirect3DDevice9* d3d9Device);

		// Called immediately after the Direct3D device has entered a lost state.
		void notifyOnDeviceLost		(IDirect3DDevice9* d3d9Device);

		// Called immediately after the Direct3D device has been reset.
		void notifyOnDeviceReset	(IDirect3DDevice9* d3d9Device);

		// Called when device state is changing. Access to any device should be locked.
		// Relevant for multi thread application.
		void lockDeviceAccess		();

		// Called when device state change completed. Access to any device is allowed.
		// Relevant for multi thread application.
		void unlockDeviceAccess		();
		
		D3D9ResourceManager			();
		~D3D9ResourceManager		();		

		void						setCreationPolicy	(D3D9ResourceCreationPolicy creationPolicy);
		D3D9ResourceCreationPolicy	getCreationPolicy	() const;
		
	// Friends.
	protected:
		friend class D3D9Resource;
	
	// Types.
	protected:
		typedef vector<D3D9Resource*>::type		ResourceContainer;
		typedef ResourceContainer::iterator		ResourceContainerIterator;

	// Protected methods.
	protected:
		
		// Called when new resource created.
		void _notifyResourceCreated		(D3D9Resource* pResource);

		// Called when resource is about to be destroyed.
		void _notifyResourceDestroyed	(D3D9Resource* pResource);
				

	// Attributes.
	protected:		
		OGRE_MUTEX(mResourcesMutex)
		ResourceContainer			mResources;
		D3D9ResourceCreationPolicy	mResourceCreationPolicy;
		long						mDeviceAccessLockCount;		
	};
}

#endif
