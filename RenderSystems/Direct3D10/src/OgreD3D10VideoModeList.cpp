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
#include "OgreD3D10VideoModeList.h"
#include "OgreException.h"
#include "OgreD3D10Driver.h"
#include "OgreD3D10VideoMode.h"

namespace Ogre 
{
	//---------------------------------------------------------------------
	D3D10VideoModeList::D3D10VideoModeList( D3D10Driver* pDriver )
	{
		if( NULL == pDriver )
			OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "pDriver parameter is NULL", "D3D10VideoModeList::D3D10VideoModeList" );

		mpDriver = pDriver;
		enumerate();
	}
	//---------------------------------------------------------------------
	D3D10VideoModeList::~D3D10VideoModeList()
	{
		mpDriver = NULL;
		mModeList.clear();
	}
	//---------------------------------------------------------------------
	BOOL D3D10VideoModeList::enumerate()
	{
		//		int pD3D = mpDriver->getD3D();
		UINT adapter = mpDriver->getAdapterNumber();
		HRESULT hr;
		IDXGIOutput *pOutput;
		for( int iOutput = 0; ; ++iOutput )
		{
			//AIZTODO: one output for a single monitor ,to be handled for mulimon	    
			hr = mpDriver->getDeviceAdapter()->EnumOutputs( iOutput, &pOutput );
			if( DXGI_ERROR_NOT_FOUND == hr )
			{
				return false;
			}
			else if (FAILED(hr))
			{
				return false;	//Something bad happened.
			}
			else //Success!
			{
				const DXGI_FORMAT allowedAdapterFormatArray[] = 
				{
					DXGI_FORMAT_R8G8B8A8_UNORM,			//This is DXUT's preferred mode

					//DXGI_FORMAT_R16G16B16A16_FLOAT,
					//DXGI_FORMAT_R10G10B10A2_UNORM,
					//DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
				};
				int allowedAdapterFormatArrayCount  = sizeof(allowedAdapterFormatArray) / sizeof(allowedAdapterFormatArray[0]);

				UINT NumModes = 512;
				DXGI_MODE_DESC *pDesc = new DXGI_MODE_DESC[ NumModes ];
				hr = pOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM,//allowedAdapterFormatArray[f],
					0,
					&NumModes,
					pDesc );
				DXGI_OUTPUT_DESC OutputDesc;
				pOutput->GetDesc(&OutputDesc);
				for( UINT m=0; m<NumModes; m++ )
				{
					DXGI_MODE_DESC displayMode=pDesc[m];
					// Filter out low-resolutions
					if( displayMode.Width < 640 || displayMode.Height < 400 )
						continue;

					// Check to see if it is already in the list (to filter out refresh rates)
					BOOL found = FALSE;
					vector<D3D10VideoMode>::type::iterator it;
					for( it = mModeList.begin(); it != mModeList.end(); it++ )
					{
						DXGI_OUTPUT_DESC oldOutput= it->getDisplayMode();
						DXGI_MODE_DESC oldDisp = it->getModeDesc();
						if(//oldOutput.Monitor==OutputDesc.Monitor &&
							oldDisp.Width == displayMode.Width &&
							oldDisp.Height == displayMode.Height// &&
							//oldDisp.Format == displayMode.Format
							)
						{
							// Check refresh rate and favour higher if poss
							//if (oldDisp.RefreshRate < displayMode.RefreshRate)
							//	it->increaseRefreshRate(displayMode.RefreshRate);
							found = TRUE;
							break;
						}
					}

					if( !found )
						mModeList.push_back( D3D10VideoMode( OutputDesc,displayMode ) );

				}

			}
		}
		/*	
		UINT iMode;
		for( iMode=0; iMode < pD3D->GetAdapterModeCount( adapter, D3DFMT_R5G6B5 ); iMode++ )
		{
		DXGI_OUTPUT_DESC displayMode;
		pD3D->EnumAdapterModes( adapter, D3DFMT_R5G6B5, iMode, &displayMode );

		// Filter out low-resolutions
		if( displayMode.Width < 640 || displayMode.Height < 400 )
		continue;

		// Check to see if it is already in the list (to filter out refresh rates)
		BOOL found = FALSE;
		vector<D3D10VideoMode>::type::iterator it;
		for( it = mModeList.begin(); it != mModeList.end(); it++ )
		{
		DXGI_OUTPUT_DESC oldDisp = it->getDisplayMode();
		if( oldDisp.Width == displayMode.Width &&
		oldDisp.Height == displayMode.Height &&
		oldDisp.Format == displayMode.Format )
		{
		// Check refresh rate and favour higher if poss
		if (oldDisp.RefreshRate < displayMode.RefreshRate)
		it->increaseRefreshRate(displayMode.RefreshRate);
		found = TRUE;
		break;
		}
		}

		if( !found )
		mModeList.push_back( D3D10VideoMode( displayMode ) );
		}

		for( iMode=0; iMode < pD3D->GetAdapterModeCount( adapter, D3DFMT_X8R8G8B8 ); iMode++ )
		{
		DXGI_OUTPUT_DESC displayMode;
		pD3D->EnumAdapterModes( adapter, D3DFMT_X8R8G8B8, iMode, &displayMode );

		// Filter out low-resolutions
		if( displayMode.Width < 640 || displayMode.Height < 400 )
		continue;

		// Check to see if it is already in the list (to filter out refresh rates)
		BOOL found = FALSE;
		vector<D3D10VideoMode>::type::iterator it;
		for( it = mModeList.begin(); it != mModeList.end(); it++ )
		{
		DXGI_OUTPUT_DESC oldDisp = it->getDisplayMode();
		if( oldDisp.Width == displayMode.Width &&
		oldDisp.Height == displayMode.Height &&
		oldDisp.Format == displayMode.Format )
		{
		// Check refresh rate and favour higher if poss
		if (oldDisp.RefreshRate < displayMode.RefreshRate)
		it->increaseRefreshRate(displayMode.RefreshRate);
		found = TRUE;
		break;
		}
		}

		if( !found )
		mModeList.push_back( D3D10VideoMode( displayMode ) );
		}
		*/
		return TRUE;
	}
	//---------------------------------------------------------------------
	size_t D3D10VideoModeList::count()
	{
		return mModeList.size();
	}
	//---------------------------------------------------------------------
	D3D10VideoMode* D3D10VideoModeList::item( size_t index )
	{
		vector<D3D10VideoMode>::type::iterator p = mModeList.begin();

		return &p[index];
	}
	//---------------------------------------------------------------------
	D3D10VideoMode* D3D10VideoModeList::item( const String &name )
	{
		vector<D3D10VideoMode>::type::iterator it = mModeList.begin();
		if (it == mModeList.end())
			return NULL;

		for (;it != mModeList.end(); ++it)
		{
			if (it->getDescription() == name)
				return &(*it);
		}

		return NULL;
	}
	//---------------------------------------------------------------------
}
