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

#include "OgreWin32Context.h"
#include "OgreException.h"
#include "OgreGLRenderSystem.h"
#include "OgreRoot.h"

namespace Ogre {

    Win32Context::Win32Context(HDC     HDC,
                 HGLRC   Glrc):
        mHDC(HDC),
        mGlrc(Glrc)
    {
    }
    
    Win32Context::~Win32Context()
    {
		// NB have to do this is subclass to ensure any methods called back
		// are on this subclass and not half-destructed superclass
		GLRenderSystem *rs = static_cast<GLRenderSystem*>(Root::getSingleton().getRenderSystem());
		rs->_unregisterContext(this);
    }
        
    void Win32Context::setCurrent()
    {
         wglMakeCurrent(mHDC, mGlrc);      
    }
	void Win32Context::endCurrent()
	{
		wglMakeCurrent(NULL, NULL);
	}

	GLContext* Win32Context::clone() const
	{
		// Create new context based on own HDC
		HGLRC newCtx = wglCreateContext(mHDC);
		
		if (!newCtx)
		{
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
				"Error calling wglCreateContext", "Win32Context::clone");
		}

		HGLRC oldrc = wglGetCurrentContext();
		HDC oldhdc = wglGetCurrentDC();
		wglMakeCurrent(NULL, NULL);
		// Share lists with old context
	    if (!wglShareLists(mGlrc, newCtx))
		{
			String errorMsg = translateWGLError();
			wglDeleteContext(newCtx);
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, String("wglShareLists() failed: ") + errorMsg, "Win32Context::clone");
		}
		// restore old context
		wglMakeCurrent(oldhdc, oldrc);
		

		return new Win32Context(mHDC, newCtx);
	}

	void Win32Context::releaseContext()
	{
		if (mGlrc != NULL)
		{
			wglDeleteContext(mGlrc);
			mGlrc = NULL;
			mHDC  = NULL;
		}		
	}
}

#if OGRE_THREAD_SUPPORT == 1

// declared in OgreGLPrerequisites.h 
WGLEWContext * wglewGetContext()
{
	using namespace Ogre;
	static OGRE_THREAD_POINTER_VAR(WGLEWContext, WGLEWContextsPtr);

	WGLEWContext * currentWGLEWContextsPtr = OGRE_THREAD_POINTER_GET(WGLEWContextsPtr);
	if (currentWGLEWContextsPtr == NULL)
	{
		currentWGLEWContextsPtr = new WGLEWContext();
		OGRE_THREAD_POINTER_SET(WGLEWContextsPtr, currentWGLEWContextsPtr);
		ZeroMemory(currentWGLEWContextsPtr, sizeof(WGLEWContext));
		wglewInit();

	}
	return currentWGLEWContextsPtr;
}

#endif