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
