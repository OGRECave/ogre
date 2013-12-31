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
#include "OgreD3D11Driver.h"
#include "OgreD3D11VideoModeList.h"
#include "OgreD3D11VideoMode.h"
#include "OgreD3D11Device.h"
#include "OgreString.h"
namespace Ogre
{
	//---------------------------------------------------------------------
	unsigned int D3D11Driver::driverCount = 0;
	//---------------------------------------------------------------------
	D3D11Driver::D3D11Driver() 
	{
		tempNo = ++driverCount;
		ZeroMemory( &mAdapterIdentifier, sizeof(mAdapterIdentifier) );
		ZeroMemory( &mDesktopDisplayMode, sizeof(mDesktopDisplayMode) );
		mVideoModeList = NULL;
		mDXGIAdapter=NULL;
	}
	//---------------------------------------------------------------------
	D3D11Driver::D3D11Driver( const D3D11Driver &ob ) 
	{
		tempNo = ++driverCount;
		mAdapterNumber = ob.mAdapterNumber;
		mAdapterIdentifier = ob.mAdapterIdentifier;
		mDesktopDisplayMode = ob.mDesktopDisplayMode;
		mVideoModeList = NULL;
		mDXGIAdapter=ob.mDXGIAdapter;

		if(mDXGIAdapter)
			mDXGIAdapter->AddRef();
	}
	//---------------------------------------------------------------------
	D3D11Driver::D3D11Driver( unsigned int adapterNumber, IDXGIAdapterN* pDXGIAdapter)
	{
		tempNo = ++driverCount;
		mAdapterNumber = adapterNumber;
		mVideoModeList = NULL;
		mDXGIAdapter=pDXGIAdapter;
		if(mDXGIAdapter)
			mDXGIAdapter->AddRef();

		// get the description of the adapter
		pDXGIAdapter->GetDesc1( &mAdapterIdentifier );

	}
	//---------------------------------------------------------------------
	D3D11Driver::~D3D11Driver()
	{
		SAFE_DELETE( mVideoModeList );
		SAFE_RELEASE(mDXGIAdapter);
		driverCount--;
	}
	//---------------------------------------------------------------------
	D3D11Driver& D3D11Driver::operator=(const D3D11Driver& ob)
	{
		tempNo = ++driverCount;
		mAdapterNumber = ob.mAdapterNumber;
		mAdapterIdentifier = ob.mAdapterIdentifier;
		mDesktopDisplayMode = ob.mDesktopDisplayMode;
		mVideoModeList = NULL;
		if(ob.mDXGIAdapter)
			ob.mDXGIAdapter->AddRef();
		SAFE_RELEASE(mDXGIAdapter);
		mDXGIAdapter=ob.mDXGIAdapter;

		return *this;
	}
	//---------------------------------------------------------------------	
	String D3D11Driver::DriverName() const
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
	String D3D11Driver::DriverDescription() const
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
	D3D11VideoModeList* D3D11Driver::getVideoModeList()
	{
		if( !mVideoModeList )
			mVideoModeList = new D3D11VideoModeList( this );

		return mVideoModeList;
	}
	//---------------------------------------------------------------------
	unsigned int D3D11Driver::getAdapterNumber() const
	{
		return mAdapterNumber;
	}
	//---------------------------------------------------------------------
	const DXGI_ADAPTER_DESC1& D3D11Driver::getAdapterIdentifier() const
	{
		return mAdapterIdentifier;
	}
	//---------------------------------------------------------------------
	const DXGI_MODE_DESC& D3D11Driver::getDesktopMode() const
	{
		return mDesktopDisplayMode;
	}
	//---------------------------------------------------------------------
	IDXGIAdapterN* D3D11Driver::getDeviceAdapter() const
	{
		return mDXGIAdapter;
	}
	//---------------------------------------------------------------------
}
