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
#include "OgreD3D10DriverList.h"
#include "OgreLogManager.h"
#include "OgreD3D10Device.h"
#include "OgreD3D10Driver.h"

namespace Ogre 
{
	//-----------------------------------------------------------------------
	D3D10DriverList::D3D10DriverList(  ) 
	{
		mpDXGIFactory = NULL;
		enumerate();
	}
	//-----------------------------------------------------------------------
	D3D10DriverList::~D3D10DriverList(void)
	{
		for(int i=0;i<0;i++)
		{
			delete (mDriverList[i]);
		}
		mDriverList.clear();
		SAFE_RELEASE(mpDXGIFactory)

	}
	//-----------------------------------------------------------------------
	BOOL D3D10DriverList::enumerate()
	{
		LogManager::getSingleton().logMessage( "D3D10: Driver Detection Starts" );
		// Create the DXGI Factory
		HRESULT hr;
		hr = CreateDXGIFactory( IID_IDXGIFactory, (void**)&mpDXGIFactory );
		if( FAILED(hr) )
			return false;

		for( UINT iAdapter=0; ; iAdapter++ )
		{
			IDXGIAdapter*					pDXGIAdapter;
			hr = mpDXGIFactory->EnumAdapters( iAdapter, &pDXGIAdapter );
			if( DXGI_ERROR_NOT_FOUND == hr )
			{
				hr = S_OK;
				break;
			}
			if( FAILED(hr) )
			{
				delete pDXGIAdapter;
				return false;
			}

			// we don't want NVIDIA PerfHUD in the list - so - here we filter it out
			DXGI_ADAPTER_DESC adaptDesc;
			if ( SUCCEEDED( pDXGIAdapter->GetDesc( &adaptDesc ) ) )
			{
				const bool isPerfHUD = wcscmp( adaptDesc.Description, L"NVIDIA PerfHUD" ) == 0;

				if (isPerfHUD)
				{
					continue;
				}
			}
			
			D3D10Device dev;
			mDriverList.push_back(new D3D10Driver( dev,  iAdapter,pDXGIAdapter) );

		}

		LogManager::getSingleton().logMessage( "D3D10: Driver Detection Ends" );

		return TRUE;
	}
	//-----------------------------------------------------------------------
	size_t D3D10DriverList::count() const 
	{
		return mDriverList.size();
	}
	//-----------------------------------------------------------------------
	D3D10Driver* D3D10DriverList::item( size_t index )
	{
		return mDriverList.at( index );
	}
	//-----------------------------------------------------------------------
	D3D10Driver* D3D10DriverList::item( const String &name )
	{
		vector<D3D10Driver*>::type::iterator it = mDriverList.begin();
		if (it == mDriverList.end())
			return NULL;

		for (;it != mDriverList.end(); ++it)
		{
			if ((*it)->DriverDescription() == name)
				return (*it);
		}

		return NULL;
	}
	//-----------------------------------------------------------------------
}
