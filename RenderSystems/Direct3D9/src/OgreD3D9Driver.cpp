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
#include "OgreD3D9Driver.h"
#include "OgreD3D9VideoModeList.h"
#include "OgreD3D9VideoMode.h"

namespace Ogre
{
    unsigned int D3D9Driver::driverCount = 0;

	D3D9Driver::D3D9Driver()
	{
		tempNo = ++driverCount;
		mpD3D = NULL;
		// initialise device member
		mpD3DDevice = NULL;
		ZeroMemory( &mAdapterIdentifier, sizeof(mAdapterIdentifier) );
		ZeroMemory( &mDesktopDisplayMode, sizeof(mDesktopDisplayMode) );
		mpVideoModeList = NULL;
	}

	D3D9Driver::D3D9Driver( const D3D9Driver &ob )
	{
		tempNo = ++driverCount;
		mpD3D = ob.mpD3D;
		// copy device member
		mpD3DDevice = ob.mpD3DDevice;
		mAdapterNumber = ob.mAdapterNumber;
		mAdapterIdentifier = ob.mAdapterIdentifier;
		mDesktopDisplayMode = ob.mDesktopDisplayMode;
		mpVideoModeList = NULL;
	}

	D3D9Driver::D3D9Driver( LPDIRECT3D9 pD3D, unsigned int adapterNumber, const D3DADAPTER_IDENTIFIER9& adapterIdentifier, const D3DDISPLAYMODE& desktopDisplayMode )
	{
		tempNo = ++driverCount;
		mpD3D = pD3D;
		// initialise device member
		mpD3DDevice = NULL;
		mAdapterNumber = adapterNumber;
		mAdapterIdentifier = adapterIdentifier;
		mDesktopDisplayMode = desktopDisplayMode;
		mpVideoModeList = NULL;
	}

	D3D9Driver::~D3D9Driver()
	{
		SAFE_DELETE( mpVideoModeList );
		driverCount--;
	}

	String D3D9Driver::DriverName() const
	{
		return String(mAdapterIdentifier.Driver);
	}

	String D3D9Driver::DriverDescription() const
	{
        String driverDescription(mAdapterIdentifier.Description);
        StringUtil::trim(driverDescription);

        return  driverDescription;
	}

	D3D9VideoModeList* D3D9Driver::getVideoModeList()
	{
		if( !mpVideoModeList )
			mpVideoModeList = new D3D9VideoModeList( this );

		return mpVideoModeList;
	}
}
