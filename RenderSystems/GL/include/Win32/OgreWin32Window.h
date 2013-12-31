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

#ifndef __Win32Window_H__
#define __Win32Window_H__

#include "OgreWin32Prerequisites.h"
#include "OgreRenderWindow.h"

namespace Ogre {
    class _OgreGLExport Win32Window : public RenderWindow
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
		bool isHidden() const { return mHidden; }
		void setHidden(bool hidden);
		void setVSyncEnabled(bool vsync);
		bool isVSyncEnabled() const;
		void setVSyncInterval(unsigned int interval);
		unsigned int getVSyncInterval() const;
        bool isClosed(void) const;
        void reposition(int left, int top);
        void resize(unsigned int width, unsigned int height);
        void swapBuffers();

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

		void adjustWindow(unsigned int clientWidth, unsigned int clientHeight, 
			unsigned int* winWidth, unsigned int* winHeight);

	protected:
		
		/** Update the window rect. */ 
		void updateWindowRect();

		/** Return the target window style depending on the fullscreen parameter. */
		DWORD getWindowStyle(bool fullScreen) const { if (fullScreen) return mFullscreenWinStyle; return mWindowedWinStyle; }

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
		bool	mHidden;
		bool	mVSync;
		unsigned int mVSyncInterval;
        int     mDisplayFrequency;      // fullscreen only, to restore display
        Win32Context *mContext;
		DWORD	mWindowedWinStyle;		// Windowed mode window style flags.
		DWORD	mFullscreenWinStyle;	// Fullscreen mode window style flags.
    };
}

#endif
