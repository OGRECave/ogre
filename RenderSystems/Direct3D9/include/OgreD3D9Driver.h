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
#ifndef __D3D9DRIVER_H__
#define __D3D9DRIVER_H__

#include "OgreD3D9Prerequisites.h"
#include "OgreString.h"

namespace Ogre
{

	class D3D9VideoModeList;
	class D3D9VideoMode;

	class D3D9Driver
	{
	
	public:
		// Constructors
		D3D9Driver();						// Default
		D3D9Driver( const D3D9Driver &ob );	// Copy
		D3D9Driver( unsigned int adapterNumber, 
			const D3DCAPS9& deviceCaps,
			const D3DADAPTER_IDENTIFIER9& adapterIdentifer, 
			const D3DDISPLAYMODE& desktopDisplayMode);
		~D3D9Driver();

				
		const D3DCAPS9&		getD3D9DeviceCaps	() const { return mD3D9DeviceCaps; }
		String				DriverName			() const;
		String				DriverDescription	() const;
				
		unsigned int					getAdapterNumber	() const { return mAdapterNumber; }
		const D3DADAPTER_IDENTIFIER9&	getAdapterIdentifier() const { return mAdapterIdentifier; }
		const D3DDISPLAYMODE&			getDesktopMode		() const { return mDesktopDisplayMode; }
		D3D9VideoModeList*				getVideoModeList	();
			
	private:				
		// Adapter number.
		unsigned int			mAdapterNumber;
		
		// Device caps.
		D3DCAPS9				mD3D9DeviceCaps;		
		
		// Adapter identifier
		D3DADAPTER_IDENTIFIER9	mAdapterIdentifier;

		// Desktop display mode.
		D3DDISPLAYMODE			mDesktopDisplayMode;

		// Video modes list.
		D3D9VideoModeList*		mpVideoModeList;	
	};
}
#endif
