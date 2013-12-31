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
#include "OgreD3D11VideoMode.h"

namespace Ogre 
{
	//---------------------------------------------------------------------
	String D3D11VideoMode::getDescription() const
	{
		char tmp[128];
		unsigned int colourDepth = 16;
		if( mModeDesc.Format == DXGI_FORMAT_R8G8B8A8_UNORM ||
			mModeDesc.Format == DXGI_FORMAT_R32G32B32A32_TYPELESS ||
			mModeDesc.Format == DXGI_FORMAT_R32G32B32A32_FLOAT ||
			mModeDesc.Format == DXGI_FORMAT_R32G32B32A32_UINT ||
			mModeDesc.Format == DXGI_FORMAT_R32G32B32A32_SINT ||
			mModeDesc.Format == DXGI_FORMAT_R32G32B32_TYPELESS ||
			mModeDesc.Format == DXGI_FORMAT_R32G32B32_FLOAT ||
			mModeDesc.Format == DXGI_FORMAT_R32G32B32_UINT ||
			mModeDesc.Format == DXGI_FORMAT_R32G32B32_SINT 
			)
			colourDepth = 32;

		sprintf( tmp, "%d x %d @ %d-bit colour", mModeDesc.Width, mModeDesc.Height, colourDepth );
		return String(tmp);
	}
	//---------------------------------------------------------------------
	unsigned int D3D11VideoMode::getColourDepth() const
	{
		unsigned int colourDepth = 16;
		if( mModeDesc.Format == DXGI_FORMAT_R32G32B32A32_TYPELESS ||
			mModeDesc.Format == DXGI_FORMAT_R32G32B32A32_FLOAT ||
			mModeDesc.Format == DXGI_FORMAT_R32G32B32A32_UINT ||
			mModeDesc.Format == DXGI_FORMAT_R32G32B32A32_SINT ||
			mModeDesc.Format == DXGI_FORMAT_R32G32B32_TYPELESS ||
			mModeDesc.Format == DXGI_FORMAT_R32G32B32_FLOAT ||
			mModeDesc.Format == DXGI_FORMAT_R32G32B32_UINT ||
			mModeDesc.Format == DXGI_FORMAT_R32G32B32_SINT 
			)
			colourDepth = 32;

		return colourDepth;
	}
	//---------------------------------------------------------------------
	unsigned int D3D11VideoMode::getWidth() const
	{
		return mModeDesc.Width;
	}
	//---------------------------------------------------------------------
	unsigned int D3D11VideoMode::getHeight() const
	{
		return mModeDesc.Height;
	}
	//---------------------------------------------------------------------
	DXGI_FORMAT D3D11VideoMode::getFormat() const
	{
		return mModeDesc.Format;
	}
	//---------------------------------------------------------------------
	DXGI_RATIONAL D3D11VideoMode::getRefreshRate() const
	{
		return mModeDesc.RefreshRate;
	}
	//---------------------------------------------------------------------
	DXGI_OUTPUT_DESC D3D11VideoMode::getDisplayMode() const
	{
		return mDisplayMode;
	}
	//---------------------------------------------------------------------
	DXGI_MODE_DESC D3D11VideoMode::getModeDesc() const
	{
		return mModeDesc;
	}
	//---------------------------------------------------------------------
	void D3D11VideoMode::increaseRefreshRate( DXGI_RATIONAL rr )
	{
		mModeDesc.RefreshRate = rr;
	}
	//---------------------------------------------------------------------
	D3D11VideoMode::D3D11VideoMode( DXGI_OUTPUT_DESC d3ddm,DXGI_MODE_DESC ModeDesc )
	{
		modeNumber = ++modeCount; mDisplayMode = d3ddm;mModeDesc=ModeDesc;
	}
	//---------------------------------------------------------------------
	D3D11VideoMode::D3D11VideoMode( const D3D11VideoMode &ob )
	{
		modeNumber = ++modeCount; mDisplayMode = ob.mDisplayMode;mModeDesc=ob.mModeDesc;
	}
	//---------------------------------------------------------------------
	D3D11VideoMode::D3D11VideoMode()
	{
		modeNumber = ++modeCount; ZeroMemory( &mDisplayMode, sizeof(DXGI_OUTPUT_DESC) );ZeroMemory( &mModeDesc, sizeof(DXGI_MODE_DESC) );
	}
	//---------------------------------------------------------------------
	D3D11VideoMode::~D3D11VideoMode()
	{
		modeCount--;
	}
	//---------------------------------------------------------------------
}
