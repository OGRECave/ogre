/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

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
#include "OgreD3D9Driver.h"
#include "OgreD3D9VideoModeList.h"
#include "OgreD3D9VideoMode.h"

namespace Ogre
{   
	D3D9Driver::D3D9Driver()
	{						
		mAdapterNumber	= 0;
		ZeroMemory( &mD3D9DeviceCaps, sizeof(mD3D9DeviceCaps) );
		ZeroMemory( &mAdapterIdentifier, sizeof(mAdapterIdentifier) );
		ZeroMemory( &mDesktopDisplayMode, sizeof(mDesktopDisplayMode) );		
		mVideoModeList = NULL;				
	}

	D3D9Driver::D3D9Driver( const D3D9Driver &ob )
	{			
		mAdapterNumber		= ob.mAdapterNumber;
		mD3D9DeviceCaps		= ob.mD3D9DeviceCaps;
		mAdapterIdentifier	= ob.mAdapterIdentifier;
		mDesktopDisplayMode = ob.mDesktopDisplayMode;
		mVideoModeList		= NULL;				
	}

	D3D9Driver::D3D9Driver( unsigned int adapterNumber,
		const D3DCAPS9& deviceCaps,
		const D3DADAPTER_IDENTIFIER9& adapterIdentifier, 
		const D3DDISPLAYMODE& desktopDisplayMode )
	{				
		mAdapterNumber		= adapterNumber;
		mD3D9DeviceCaps		= deviceCaps;
		mAdapterIdentifier	= adapterIdentifier;
		mDesktopDisplayMode = desktopDisplayMode;
		mVideoModeList		= NULL;			
	}

	D3D9Driver::~D3D9Driver()
	{
		if (mVideoModeList != NULL)
		{
			OGRE_DELETE mVideoModeList;
			mVideoModeList = NULL;
		}
	}

	String D3D9Driver::DriverName() const
	{
		return String(mAdapterIdentifier.Driver);
	}

	String D3D9Driver::DriverDescription() const
	{       
		StringUtil::StrStreamType str;
		str << "Monitor-" << (mAdapterNumber+1) << "-" << mAdapterIdentifier.Description;
		String driverDescription(str.str());
		StringUtil::trim(driverDescription);

        return  driverDescription;
	}

	D3D9VideoModeList* D3D9Driver::getVideoModeList()
	{
		if( !mVideoModeList )
			mVideoModeList = OGRE_NEW D3D9VideoModeList( this );

		return mVideoModeList;
	}	
}
