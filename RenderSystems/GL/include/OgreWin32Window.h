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

#ifndef __Win32Window_H__
#define __Win32Window_H__

#include "OgreWin32Prerequisites.h"
#include "OgreRenderWindow.h"

namespace Ogre {
    class _OgrePrivate Win32Window : public RenderWindow
    {
    public:
        Win32Window(Win32GLSupport &glsupport);
        ~Win32Window();

       void create(const String& name, unsigned int width, unsigned int height,
	            bool fullScreen, const NameValuePairList *miscParams);
	   void setFullscreen(bool fullScreen, unsigned int width, unsigned int height);
        void destroy(void);
		bool isActive(void) const;
        bool isVisible() const;
        bool isClosed(void) const;
        void reposition(int left, int top);
        void resize(unsigned int width, unsigned int height);
        void swapBuffers(bool waitForVSync);

		/** Overridden - see RenderTarget. */
		virtual void copyContentsToMemory(const PixelBox &dst, FrameBuffer buffer);

		bool requiresTextureFlipping() const { return false; }

		HWND getWindowHandle() const { return mHWnd; }
		HDC getHDC() const { return mHDC; }
		
		// Method for dealing with resize / move & 3d library
		virtual void windowMovedOrResized(void);

		void getCustomAttribute( const String& name, void* pData );

        /** Used to set the active state of the render target.
        */
        virtual void setActive( bool state );

	protected:
		Win32GLSupport &mGLSupport;
		HWND	mHWnd;					// Win32 Window handle
		HDC		mHDC;
		HGLRC	mGlrc;
        bool    mIsExternal;
		char*   mDeviceName;
		bool    mIsExternalGLControl;
		bool	mIsExternalGLContext;
        bool    mSizing;
		bool	mClosed;
        int     mDisplayFrequency;      // fullscreen only, to restore display
        Win32Context *mContext;
    };
}

#endif
