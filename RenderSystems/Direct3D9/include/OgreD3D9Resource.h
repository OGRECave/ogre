/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
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
#ifndef __D3D9Resource_H__
#define __D3D9Resource_H__

#include "OgreD3D9Prerequisites.h"

namespace Ogre {

	/** Represents a Direct3D rendering resource.
	Provide unified interface to
	handle various device states.
	*/
	class D3D9Resource
	{

	// Interface.
	public:

		// Called immediately after the Direct3D device has been created.
		virtual void notifyOnDeviceCreate(IDirect3DDevice9* d3d9Device) {}

		// Called before the Direct3D device is going to be destroyed.
		virtual void notifyOnDeviceDestroy(IDirect3DDevice9* d3d9Device) {}

		// Called immediately after the Direct3D device has entered a lost state.
		// This is the place to release non-managed resources.
		virtual void notifyOnDeviceLost(IDirect3DDevice9* d3d9Device) {}

		// Called immediately after the Direct3D device has been reset.
		// This is the place to create non-managed resources.
		virtual void notifyOnDeviceReset(IDirect3DDevice9* d3d9Device) {}

		// Called when device state is changing. Access to any device should be locked.
		// Relevant for multi thread application.
		static void lockDeviceAccess();

		// Called when device state change completed. Access to any device is allowed.
		// Relevant for multi thread application.
		static void unlockDeviceAccess();


	public:
		D3D9Resource			();
		virtual ~D3D9Resource	();

	protected:
		OGRE_STATIC_MUTEX(msDeviceAccessMutex)		
	};
}
#endif
