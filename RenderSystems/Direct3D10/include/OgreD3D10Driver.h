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
#ifndef __D3D10DRIVER_H__
#define __D3D10DRIVER_H__

#include "OgreD3D10Prerequisites.h"

namespace Ogre
{

	class D3D10VideoModeList;
	class D3D10VideoMode;

	class D3D10Driver
	{
	private:
		// D3D only allows one device per adapter, so it can safely be stored
		// here as well.
		D3D10Device & mDevice;
		unsigned int mAdapterNumber;
		DXGI_ADAPTER_DESC mAdapterIdentifier;
		DXGI_MODE_DESC mDesktopDisplayMode;
		D3D10VideoModeList* mpVideoModeList;
		unsigned int tempNo;
		static unsigned int driverCount;
		IDXGIAdapter*	mpDXGIAdapter;
		//DXGI_ADAPTER_DESC mAdapterDesc;


	public:
		// Constructors
		D3D10Driver(D3D10Device & device);		// Default
		D3D10Driver( const D3D10Driver &ob );	// Copy
		D3D10Driver(D3D10Device & device,  unsigned int adapterNumber,  IDXGIAdapter* pDXGIAdapter );
		~D3D10Driver();

		// Information accessors
		String DriverName() const;
		String DriverDescription() const;

		// change the device
		void setDevice(D3D10Device & device);
		unsigned int getAdapterNumber() const;
		const DXGI_ADAPTER_DESC& getAdapterIdentifier() const;
		const DXGI_MODE_DESC& getDesktopMode() const;
		IDXGIAdapter* getDeviceAdapter() const;
		D3D10VideoModeList* getVideoModeList();
	};
}
#endif
