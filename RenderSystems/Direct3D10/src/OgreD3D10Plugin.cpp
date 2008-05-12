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

#include "OgreD3D10Plugin.h"
#include "OgreRoot.h"

namespace Ogre 
{
	const String sPluginName = "D3D10 RenderSystem";
	//---------------------------------------------------------------------
	D3D10Plugin::D3D10Plugin()
		: mRenderSystem(0)
	{

	}
	//---------------------------------------------------------------------
	const String& D3D10Plugin::getName() const
	{
		return sPluginName;
	}
	//---------------------------------------------------------------------
	void D3D10Plugin::install()
	{
		// Create the DirectX 10 rendering api
#ifdef OGRE_STATIC_LIB
		HINSTANCE hInst = GetModuleHandle( NULL );
#else
		HINSTANCE hInst = GetModuleHandle( "RenderSystem_Direct3D10.dll" );
#endif
		mRenderSystem = new D3D10RenderSystem( hInst );
		// Register the render system
		Root::getSingleton().addRenderSystem( mRenderSystem );
	}
	//---------------------------------------------------------------------
	void D3D10Plugin::initialise()
	{
		// nothing to do
	}
	//---------------------------------------------------------------------
	void D3D10Plugin::shutdown()
	{
		// nothing to do
	}
	//---------------------------------------------------------------------
	void D3D10Plugin::uninstall()
	{
		delete mRenderSystem;
		mRenderSystem = 0;
	}


}
