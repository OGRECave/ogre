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

			mDriverList.push_back(new D3D10Driver( D3D10Device(),  iAdapter,pDXGIAdapter) );

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
