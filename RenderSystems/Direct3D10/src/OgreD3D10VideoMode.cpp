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
#include "OgreD3D10VideoMode.h"

namespace Ogre 
{
	//---------------------------------------------------------------------
	String D3D10VideoMode::getDescription() const
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
	unsigned int D3D10VideoMode::getColourDepth() const
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
	unsigned int D3D10VideoMode::getWidth() const
	{
		return mModeDesc.Width;
	}
	//---------------------------------------------------------------------
	unsigned int D3D10VideoMode::getHeight() const
	{
		return mModeDesc.Height;
	}
	//---------------------------------------------------------------------
	DXGI_FORMAT D3D10VideoMode::getFormat() const
	{
		return mModeDesc.Format;
	}
	//---------------------------------------------------------------------
	DXGI_RATIONAL D3D10VideoMode::getRefreshRate() const
	{
		return mModeDesc.RefreshRate;
	}
	//---------------------------------------------------------------------
	DXGI_OUTPUT_DESC D3D10VideoMode::getDisplayMode() const
	{
		return mDisplayMode;
	}
	//---------------------------------------------------------------------
	DXGI_MODE_DESC D3D10VideoMode::getModeDesc() const
	{
		return mModeDesc;
	}
	//---------------------------------------------------------------------
	void D3D10VideoMode::increaseRefreshRate( DXGI_RATIONAL rr )
	{
		mModeDesc.RefreshRate = rr;
	}
	//---------------------------------------------------------------------
	D3D10VideoMode::D3D10VideoMode( DXGI_OUTPUT_DESC d3ddm,DXGI_MODE_DESC ModeDesc )
	{
		modeNumber = ++modeCount; mDisplayMode = d3ddm;mModeDesc=ModeDesc;
	}
	//---------------------------------------------------------------------
	D3D10VideoMode::D3D10VideoMode( const D3D10VideoMode &ob )
	{
		modeNumber = ++modeCount; mDisplayMode = ob.mDisplayMode;mModeDesc=ob.mModeDesc;
	}
	//---------------------------------------------------------------------
	D3D10VideoMode::D3D10VideoMode()
	{
		modeNumber = ++modeCount; ZeroMemory( &mDisplayMode, sizeof(DXGI_OUTPUT_DESC) );ZeroMemory( &mModeDesc, sizeof(DXGI_MODE_DESC) );
	}
	//---------------------------------------------------------------------
	D3D10VideoMode::~D3D10VideoMode()
	{
		modeCount--;
	}
	//---------------------------------------------------------------------
}
