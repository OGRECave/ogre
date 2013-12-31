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
#include "OgreD3D11VideoModeList.h"
#include "OgreException.h"
#include "OgreD3D11Driver.h"
#include "OgreD3D11VideoMode.h"

namespace Ogre 
{
	//---------------------------------------------------------------------
	D3D11VideoModeList::D3D11VideoModeList( D3D11Driver* pDriver )
	{
		if( NULL == pDriver )
			OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "pDriver parameter is NULL", "D3D11VideoModeList::D3D11VideoModeList" );

		mDriver = pDriver;
		enumerate();
	}
	//---------------------------------------------------------------------
	D3D11VideoModeList::~D3D11VideoModeList()
	{
		mDriver = NULL;
		mModeList.clear();
	}
	//---------------------------------------------------------------------
	BOOL D3D11VideoModeList::enumerate()
	{
		//		int pD3D = mDriver->getD3D();
		UINT adapter = mDriver->getAdapterNumber();
		HRESULT hr;
		IDXGIOutput *pOutput;
		for( int iOutput = 0; ; ++iOutput )
		{
			//AIZTODO: one output for a single monitor ,to be handled for mulimon	    
			hr = mDriver->getDeviceAdapter()->EnumOutputs( iOutput, &pOutput );
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
				DXGI_OUTPUT_DESC OutputDesc;
				pOutput->GetDesc(&OutputDesc);

				UINT NumModes = 0;
                hr = pOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM,
                    0,
                    &NumModes,
                    NULL );

                DXGI_MODE_DESC *pDesc = new DXGI_MODE_DESC[ NumModes ];
                ZeroMemory(pDesc, sizeof(DXGI_MODE_DESC) * NumModes);
				hr = pOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM,
					0,
					&NumModes,
					pDesc );
				
				SAFE_RELEASE(pOutput);

				// display mode list can not be obtained when working over terminal session
				if(FAILED(hr))
				{
					NumModes = 0;

					if(hr == DXGI_ERROR_NOT_CURRENTLY_AVAILABLE)
					{
						pDesc[0].Width = 800;
						pDesc[0].Height = 600;
						pDesc[0].RefreshRate.Numerator = 60;
						pDesc[0].RefreshRate.Denominator = 1;
						pDesc[0].Format = DXGI_FORMAT_R8G8B8A8_UNORM;
						pDesc[0].ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
						pDesc[0].Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

						NumModes = 1;
					}
				}

				for( UINT m=0; m<NumModes; m++ )
				{
					DXGI_MODE_DESC displayMode=pDesc[m];
					// Filter out low-resolutions
					if( displayMode.Width < 640 || displayMode.Height < 400 )
						continue;

					// Check to see if it is already in the list (to filter out refresh rates)
					BOOL found = FALSE;
					vector<D3D11VideoMode>::type::iterator it;
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
						mModeList.push_back( D3D11VideoMode( OutputDesc,displayMode ) );

				}
                delete [] pDesc;
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
		vector<D3D11VideoMode>::type::iterator it;
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
		mModeList.push_back( D3D11VideoMode( displayMode ) );
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
		vector<D3D11VideoMode>::type::iterator it;
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
		mModeList.push_back( D3D11VideoMode( displayMode ) );
		}
		*/
		return TRUE;
	}
	//---------------------------------------------------------------------
	size_t D3D11VideoModeList::count()
	{
		return mModeList.size();
	}
	//---------------------------------------------------------------------
	D3D11VideoMode* D3D11VideoModeList::item( size_t index )
	{
		vector<D3D11VideoMode>::type::iterator p = mModeList.begin();

		return &p[index];
	}
	//---------------------------------------------------------------------
	D3D11VideoMode* D3D11VideoModeList::item( const String &name )
	{
		vector<D3D11VideoMode>::type::iterator it = mModeList.begin();
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
