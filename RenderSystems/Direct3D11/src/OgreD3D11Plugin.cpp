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
#include "OgreD3D11Plugin.h"
#include "OgreRoot.h"

namespace Ogre 
{
	const String sPluginName = "D3D11 RenderSystem";
	//---------------------------------------------------------------------
	D3D11Plugin::D3D11Plugin()
		: mRenderSystem(0)
	{

	}
	//---------------------------------------------------------------------
	const String& D3D11Plugin::getName() const
	{
		return sPluginName;
	}
	//---------------------------------------------------------------------
	void D3D11Plugin::install()
	{
		// Create the DirectX 10 rendering api
#ifdef OGRE_STATIC_LIB
		HINSTANCE hInst = GetModuleHandle( NULL );
#else
		HINSTANCE hInst = GetModuleHandle( "RenderSystem_Direct3D11.dll" );
#endif
		mRenderSystem = new D3D11RenderSystem( hInst );
		// Register the render system
		Root::getSingleton().addRenderSystem( mRenderSystem );
	}
	//---------------------------------------------------------------------
	void D3D11Plugin::initialise()
	{
		// nothing to do
	}
	//---------------------------------------------------------------------
	void D3D11Plugin::shutdown()
	{
		// nothing to do
	}
	//---------------------------------------------------------------------
	void D3D11Plugin::uninstall()
	{
		delete mRenderSystem;
		mRenderSystem = 0;
	}


}
