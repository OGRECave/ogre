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
#include "OgreD3D10Driver.h"
#include "OgreD3D10VideoModeList.h"
#include "OgreD3D10VideoMode.h"
#include "OgreD3D10Device.h"
#include "OgreString.h"
namespace Ogre
{
	//---------------------------------------------------------------------
	unsigned int D3D10Driver::driverCount = 0;
	//---------------------------------------------------------------------
	D3D10Driver::D3D10Driver(D3D10Device & device) : mDevice(device)
	{
		tempNo = ++driverCount;
		ZeroMemory( &mAdapterIdentifier, sizeof(mAdapterIdentifier) );
		ZeroMemory( &mDesktopDisplayMode, sizeof(mDesktopDisplayMode) );
		mpVideoModeList = NULL;
		mpDXGIAdapter=NULL;
	}
	//---------------------------------------------------------------------
	D3D10Driver::D3D10Driver( const D3D10Driver &ob ) : mDevice(ob.mDevice)
	{
		tempNo = ++driverCount;
		mAdapterNumber = ob.mAdapterNumber;
		mAdapterIdentifier = ob.mAdapterIdentifier;
		mDesktopDisplayMode = ob.mDesktopDisplayMode;
		mpVideoModeList = NULL;
		mpDXGIAdapter=ob.mpDXGIAdapter;

	}
	//---------------------------------------------------------------------
	D3D10Driver::D3D10Driver( D3D10Device & device, unsigned int adapterNumber, IDXGIAdapter* pDXGIAdapter) : mDevice(device)
	{
		tempNo = ++driverCount;
		mAdapterNumber = adapterNumber;
		mpVideoModeList = NULL;
		mpDXGIAdapter=pDXGIAdapter;

		// get the description of the adapter
		pDXGIAdapter->GetDesc( &mAdapterIdentifier );

	}
	//---------------------------------------------------------------------
	D3D10Driver::~D3D10Driver()
	{
		SAFE_DELETE( mpVideoModeList );
		driverCount--;
	}
	//---------------------------------------------------------------------
	String D3D10Driver::DriverName() const
	{
		size_t size=wcslen(mAdapterIdentifier.Description);
		char * str=new char[size+1];

		wcstombs(str, mAdapterIdentifier.Description,size);
		str[size]='\0';
		String Description=str;
		delete str;
		return String(Description );
	}
	//---------------------------------------------------------------------
	String D3D10Driver::DriverDescription() const
	{
		size_t size=wcslen(mAdapterIdentifier.Description);
		char * str=new char[size+1];

		wcstombs(str, mAdapterIdentifier.Description,size);
		str[size]='\0';
		String driverDescription=str;
		delete [] str;
		StringUtil::trim(driverDescription);

		return  driverDescription;
	}
	//---------------------------------------------------------------------
	D3D10VideoModeList* D3D10Driver::getVideoModeList()
	{
		if( !mpVideoModeList )
			mpVideoModeList = new D3D10VideoModeList( this );

		return mpVideoModeList;
	}
	//---------------------------------------------------------------------
	void D3D10Driver::setDevice( D3D10Device & device )
	{
		mDevice = device;
	}
	//---------------------------------------------------------------------
	unsigned int D3D10Driver::getAdapterNumber() const
	{
		return mAdapterNumber;
	}
	//---------------------------------------------------------------------
	const DXGI_ADAPTER_DESC& D3D10Driver::getAdapterIdentifier() const
	{
		return mAdapterIdentifier;
	}
	//---------------------------------------------------------------------
	const DXGI_MODE_DESC& D3D10Driver::getDesktopMode() const
	{
		return mDesktopDisplayMode;
	}
	//---------------------------------------------------------------------
	IDXGIAdapter* D3D10Driver::getDeviceAdapter() const
	{
		return mpDXGIAdapter;
	}
	//---------------------------------------------------------------------
}
