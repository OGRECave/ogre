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
#ifndef __D3D9DRIVER_H__
#define __D3D9DRIVER_H__

#include "OgreD3D9Prerequisites.h"
#include "OgreString.h"

namespace Ogre
{

	class D3D9VideoModeList;
	class D3D9VideoMode;

	class _OgreD3D9Export D3D9Driver : public ResourceAlloc
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
		D3D9VideoModeList*		mVideoModeList;	
	};
}
#endif
