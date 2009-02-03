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
#include "OgreD3D9VideoModeList.h"
#include "OgreException.h"
#include "OgreD3D9RenderSystem.h"

namespace Ogre 
{
	D3D9VideoModeList::D3D9VideoModeList( D3D9Driver* pDriver )
	{
		if( NULL == pDriver )
			OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "pDriver parameter is NULL", "D3D9VideoModeList::D3D9VideoModeList" );

		mpDriver = pDriver;
		enumerate();
	}

	D3D9VideoModeList::~D3D9VideoModeList()
	{
		mpDriver = NULL;
		mModeList.clear();
	}

	BOOL D3D9VideoModeList::enumerate()
	{
		UINT iMode;
		IDirect3D9* pD3D = D3D9RenderSystem::getDirect3D9();
		UINT adapter = mpDriver->getAdapterNumber();

		for( iMode=0; iMode < pD3D->GetAdapterModeCount( adapter, D3DFMT_R5G6B5 ); iMode++ )
		{
			D3DDISPLAYMODE displayMode;
			pD3D->EnumAdapterModes( adapter, D3DFMT_R5G6B5, iMode, &displayMode );

			// Filter out low-resolutions
			if( displayMode.Width < 640 || displayMode.Height < 400 )
				continue;

			// Check to see if it is already in the list (to filter out refresh rates)
			BOOL found = FALSE;
			vector<D3D9VideoMode>::type::iterator it;
			for( it = mModeList.begin(); it != mModeList.end(); it++ )
			{
				D3DDISPLAYMODE oldDisp = it->getDisplayMode();
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
				mModeList.push_back( D3D9VideoMode( displayMode ) );
		}

		for( iMode=0; iMode < pD3D->GetAdapterModeCount( adapter, D3DFMT_X8R8G8B8 ); iMode++ )
		{
			D3DDISPLAYMODE displayMode;
			pD3D->EnumAdapterModes( adapter, D3DFMT_X8R8G8B8, iMode, &displayMode );

			// Filter out low-resolutions
			if( displayMode.Width < 640 || displayMode.Height < 400 )
				continue;

			// Check to see if it is already in the list (to filter out refresh rates)
			BOOL found = FALSE;
			vector<D3D9VideoMode>::type::iterator it;
			for( it = mModeList.begin(); it != mModeList.end(); it++ )
			{
				D3DDISPLAYMODE oldDisp = it->getDisplayMode();
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
				mModeList.push_back( D3D9VideoMode( displayMode ) );
		}

		return TRUE;
	}

	size_t D3D9VideoModeList::count()
	{
		return mModeList.size();
	}

	D3D9VideoMode* D3D9VideoModeList::item( size_t index )
	{
		vector<D3D9VideoMode>::type::iterator p = mModeList.begin();

		return &p[index];
	}

	D3D9VideoMode* D3D9VideoModeList::item( const String &name )
	{
		vector<D3D9VideoMode>::type::iterator it = mModeList.begin();
		if (it == mModeList.end())
			return NULL;

		for (;it != mModeList.end(); ++it)
		{
			if (it->getDescription() == name)
				return &(*it);
		}

		return NULL;
	}
}
