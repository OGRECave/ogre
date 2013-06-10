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

#include "OgreD3D9Plugin.h"
#include "OgreRoot.h"

#ifdef __MINGW32__
extern "C" {   
#include "WIN32/OgreMinGWSupport.h"
void _chkstk();
void _fastcall __security_check_cookie(intptr_t i);
} 	
#endif

namespace Ogre 
{
	const String sPluginName = "D3D9 RenderSystem";
	//---------------------------------------------------------------------
	D3D9Plugin::D3D9Plugin()
		: mRenderSystem(0)
	{

	}
	//---------------------------------------------------------------------
	const String& D3D9Plugin::getName() const
	{
		return sPluginName;
	}
	
	//---------------------------------------------------------------------
	void D3D9Plugin::install()
	{
		// When building with MinGW, we need to call dummy implementations
		// of missing MinGW DirectX functions to make sure they are carried over
		// in dynamic AND static builds
#ifdef __MINGW32__
    _chkstk();
    __security_check_cookie((intptr_t)NULL);    
#endif
    
		// Create the DirectX 9 rendering api
#ifdef OGRE_STATIC_LIB
		HINSTANCE hInst = GetModuleHandle( NULL );
#else
#  if OGRE_DEBUG_MODE == 1
		HINSTANCE hInst = GetModuleHandle( "RenderSystem_Direct3D9_d.dll" );
#  else
		HINSTANCE hInst = GetModuleHandle( "RenderSystem_Direct3D9.dll" );
#  endif
#endif
		mRenderSystem = OGRE_NEW D3D9RenderSystem( hInst );
		// Register the render system
		Root::getSingleton().addRenderSystem( mRenderSystem );
	}
	//---------------------------------------------------------------------
	void D3D9Plugin::initialise()
	{
		// nothing to do
	}
	//---------------------------------------------------------------------
	void D3D9Plugin::shutdown()
	{
		// nothing to do
	}
	//---------------------------------------------------------------------
	void D3D9Plugin::uninstall()
	{
		if (mRenderSystem != NULL)
		{
			OGRE_DELETE mRenderSystem;
			mRenderSystem = NULL;
		}				
	}


}
