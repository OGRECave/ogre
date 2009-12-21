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
#ifndef __D3D11VIDEOMODE_H__
#define __D3D11VIDEOMODE_H__

#include "OgreD3D11Prerequisites.h"

namespace Ogre 
{
	static unsigned int modeCount = 0;

	class D3D11VideoMode
	{
	private:
		DXGI_OUTPUT_DESC mDisplayMode;
		DXGI_MODE_DESC mModeDesc;
		unsigned int modeNumber;

	public:
		D3D11VideoMode();
		D3D11VideoMode( const D3D11VideoMode &ob );
		D3D11VideoMode( DXGI_OUTPUT_DESC d3ddm,DXGI_MODE_DESC ModeDesc );
		~D3D11VideoMode();

		unsigned int getWidth() const;
		unsigned int getHeight() const;
		DXGI_FORMAT getFormat() const;
		DXGI_RATIONAL getRefreshRate() const;
		unsigned int getColourDepth() const;
		DXGI_OUTPUT_DESC getDisplayMode() const;
		DXGI_MODE_DESC getModeDesc() const;
		void increaseRefreshRate(DXGI_RATIONAL rr); 
		String getDescription() const;
	};
}
#endif
