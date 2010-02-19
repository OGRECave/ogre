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
#ifndef __D3D9VIDEOMODE_H__
#define __D3D9VIDEOMODE_H__

#include "OgreD3D9Prerequisites.h"
#include "OgreString.h"

namespace Ogre 
{
	static unsigned int modeCount = 0;

	class _OgreD3D9Export D3D9VideoMode
	{
	private:
		D3DDISPLAYMODE mDisplayMode;
		unsigned int modeNumber;

	public:
		D3D9VideoMode() { modeNumber = ++modeCount; ZeroMemory( &mDisplayMode, sizeof(D3DDISPLAYMODE) ); }
		D3D9VideoMode( const D3D9VideoMode &ob ) { modeNumber = ++modeCount; mDisplayMode = ob.mDisplayMode; }
		D3D9VideoMode( D3DDISPLAYMODE d3ddm ) { modeNumber = ++modeCount; mDisplayMode = d3ddm; }
		~D3D9VideoMode()
		{
			modeCount--;
		}

		unsigned int getWidth() const { return mDisplayMode.Width; }
		unsigned int getHeight() const { return mDisplayMode.Height; }
		D3DFORMAT getFormat() const { return mDisplayMode.Format; }
		unsigned int getRefreshRate() const { return mDisplayMode.RefreshRate; }
		unsigned int getColourDepth() const;
		D3DDISPLAYMODE getDisplayMode() const { return mDisplayMode; }
		void increaseRefreshRate(unsigned int rr) { mDisplayMode.RefreshRate = rr; } 
		String getDescription() const;
	};
}
#endif
