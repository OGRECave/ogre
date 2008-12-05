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

#include "OgreWin32Window.h"
#include "OgreRoot.h"
#include "OgreLogManager.h"
#include "OgreRenderSystem.h"
#include "OgreImageCodec.h"
#include "OgreStringConverter.h"
#include "OgreException.h"
#include "OgreWin32GLSupport.h"
#include "OgreWin32Context.h"
#include "OgreWindowEventUtilities.h"
#include "OgreGLPixelFormat.h"

namespace Ogre {

	Win32Window::Win32Window(Win32GLSupport &glsupport):
		mGLSupport(glsupport),
		mContext(0)
	{
		mIsFullScreen = false;
		mHWnd = 0;
		mGlrc = 0;
		mIsExternal = false;
		mIsExternalGLControl = false;
		mIsExternalGLContext = false;
		mSizing = false;
		mClosed = false;
		mDisplayFrequency = 0;
		mActive = false;
	}

	Win32Window::~Win32Window()
	{
		destroy();
	}

	void Win32Window::create(const String& name, unsigned int width, unsigned int height,
							bool fullScreen, const NameValuePairList *miscParams)
	{
		// destroy current window, if any
		if (mHWnd)
			destroy();

#ifdef OGRE_STATIC_LIB
		HINSTANCE hInst = GetModuleHandle( NULL );
#else
#  if OGRE_DEBUG_MODE == 1
		HINSTANCE hInst = GetModuleHandle("RenderSystem_GL_d.dll");
#  else
		HINSTANCE hInst = GetModuleHandle("RenderSystem_GL.dll");
#  endif
#endif

		mHWnd = 0;
		mName = name;
		mIsFullScreen = fullScreen;
		mClosed = false;

		// load window defaults
		mLeft = mTop = -1; // centered
		mWidth = width;
		mHeight = height;
		mDisplayFrequency = 0;
		mIsDepthBuffered = true;
		mColourDepth = mIsFullScreen? 32 : GetDeviceCaps(GetDC(0), BITSPIXEL);

		HWND parent = 0;
		String title = name;
		bool vsync = false;
		String border;
		bool outerSize = false;
		bool hwGamma = false;

		if(miscParams)
		{
			// Get variable-length params
			NameValuePairList::const_iterator opt;
			NameValuePairList::const_iterator end = miscParams->end();

			if ((opt = miscParams->find("title")) != end)
				title = opt->second;

			if ((opt = miscParams->find("left")) != end)
				mLeft = StringConverter::parseInt(opt->second);

			if ((opt = miscParams->find("top")) != end)
				mTop = StringConverter::parseInt(opt->second);

			if ((opt = miscParams->find("depthBuffer")) != end)
				mIsDepthBuffered = StringConverter::parseBool(opt->second);

			if ((opt = miscParams->find("vsync")) != end)
				vsync = StringConverter::parseBool(opt->second);

			if ((opt = miscParams->find("FSAA")) != end)
				mFSAA = StringConverter::parseUnsignedInt(opt->second);

			if ((opt = miscParams->find("FSAAHint")) != end)
				mFSAAHint = opt->second;

			if ((opt = miscParams->find("gamma")) != end)
				hwGamma = StringConverter::parseBool(opt->second);

			if ((opt = miscParams->find("externalWindowHandle")) != end)
			{
				mHWnd = (HWND)StringConverter::parseUnsignedInt(opt->second);
				if (mHWnd)
				{
					mIsExternal = true;
					mIsFullScreen = false;
				}

				if ((opt = miscParams->find("externalGLControl")) != end) {
				  mIsExternalGLControl = StringConverter::parseBool(opt->second);
				}
			}
			if ((opt = miscParams->find("externalGLContext")) != end)
			{
				mGlrc = (HGLRC)StringConverter::parseUnsignedLong(opt->second);
				if( mGlrc )
					mIsExternalGLContext = true;
			}

			// window border style
			opt = miscParams->find("border");
			if(opt != miscParams->end())
				border = opt->second;
			// set outer dimensions?
			opt = miscParams->find("outerDimensions");
			if(opt != miscParams->end())
				outerSize = StringConverter::parseBool(opt->second);

			// only available with fullscreen
			if ((opt = miscParams->find("displayFrequency")) != end)
				mDisplayFrequency = StringConverter::parseUnsignedInt(opt->second);
			if ((opt = miscParams->find("colourDepth")) != end)
			{
				mColourDepth = StringConverter::parseUnsignedInt(opt->second);
				if (!mIsFullScreen)
				{
					// make sure we don't exceed desktop colour depth
					if (mColourDepth > GetDeviceCaps(GetDC(0), BITSPIXEL))
						mColourDepth = GetDeviceCaps(GetDC(0), BITSPIXEL);
				}
			}

			// incompatible with fullscreen
			if ((opt = miscParams->find("parentWindowHandle")) != end)
				parent = (HWND)StringConverter::parseUnsignedInt(opt->second);
		}

		if (!mIsExternal)
		{
			DWORD dwStyle = WS_VISIBLE | WS_CLIPCHILDREN;
			DWORD dwStyleEx = 0;
			int outerw, outerh;

			if (mIsFullScreen)
			{
				dwStyle |= WS_POPUP;
				dwStyleEx |= WS_EX_TOPMOST;
				outerw = mWidth;
				outerh = mHeight;
				mLeft = mTop = 0;
			}
			else
			{
				if (parent)
				{
					dwStyle |= WS_CHILD;
				}
				else
				{
					if (border == "none")
						dwStyle |= WS_POPUP;
					else if (border == "fixed")
						dwStyle |= WS_OVERLAPPED | WS_BORDER | WS_CAPTION |
						WS_SYSMENU | WS_MINIMIZEBOX;
					else
						dwStyle |= WS_OVERLAPPEDWINDOW;
				}

				int screenw = GetSystemMetrics(SM_CXSCREEN);
				int screenh = GetSystemMetrics(SM_CYSCREEN);

				if (!outerSize)
				{
					// calculate overall dimensions for requested client area
					RECT rc = { 0, 0, mWidth, mHeight };
					AdjustWindowRect(&rc, dwStyle, false);

					// clamp window dimensions to screen size
					outerw = (rc.right-rc.left < screenw)? rc.right-rc.left : screenw;
					outerh = (rc.bottom-rc.top < screenh)? rc.bottom-rc.top : screenh;
				}

				// center window if given negative coordinates
				if (mLeft < 0)
					mLeft = (screenw - outerw) / 2;
				if (mTop < 0)
					mTop = (screenh - outerh) / 2;

				// keep window contained in visible screen area
				if (mLeft > screenw - outerw)
					mLeft = screenw - outerw;
				if (mTop > screenh - outerh)
					mTop = screenh - outerh;
			}

			// register class and create window
			WNDCLASS wc = { CS_OWNDC, WindowEventUtilities::_WndProc, 0, 0, hInst,
				LoadIcon(NULL, IDI_APPLICATION), LoadCursor(NULL, IDC_ARROW),
				(HBRUSH)GetStockObject(BLACK_BRUSH), NULL, "OgreGLWindow" };
			RegisterClass(&wc);

			// Pass pointer to self as WM_CREATE parameter
			mHWnd = CreateWindowEx(dwStyleEx, "OgreGLWindow", title.c_str(),
				dwStyle, mLeft, mTop, outerw, outerh, parent, 0, hInst, this);

			WindowEventUtilities::_addRenderWindow(this);

			LogManager::getSingleton().stream()
				<< "Created Win32Window '"
				<< mName << "' : " << mWidth << "x" << mHeight
				<< ", " << mColourDepth << "bpp";

			if (mIsFullScreen)
			{
				DEVMODE dm;
				dm.dmSize = sizeof(DEVMODE);
				dm.dmBitsPerPel = mColourDepth;
				dm.dmPelsWidth = mWidth;
				dm.dmPelsHeight = mHeight;
				dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
				if (mDisplayFrequency)
				{
					dm.dmDisplayFrequency = mDisplayFrequency;
					dm.dmFields |= DM_DISPLAYFREQUENCY;
					if (ChangeDisplaySettings(&dm, CDS_FULLSCREEN | CDS_TEST) != DISP_CHANGE_SUCCESSFUL)
					{
						LogManager::getSingleton().logMessage(LML_NORMAL, "ChangeDisplaySettings with user display frequency failed");
						dm.dmFields ^= DM_DISPLAYFREQUENCY;
					}
				}
				if (ChangeDisplaySettings(&dm, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
					LogManager::getSingleton().logMessage(LML_CRITICAL, "ChangeDisplaySettings failed");
			}
		}

		HDC old_hdc = wglGetCurrentDC();
		HGLRC old_context = wglGetCurrentContext();

		RECT rc;
		// top and left represent outer window position
		GetWindowRect(mHWnd, &rc);
		mTop = rc.top;
		mLeft = rc.left;
		// width and height represent drawable area only
		GetClientRect(mHWnd, &rc);
		mWidth = rc.right;
		mHeight = rc.bottom;

		mHDC = GetDC(mHWnd);

		if (!mIsExternalGLControl)
		{
			int testFsaa = mFSAA;
			bool testHwGamma = hwGamma;
			bool formatOk = mGLSupport.selectPixelFormat(mHDC, mColourDepth, testFsaa, testHwGamma);
			if (!formatOk)
			{
				if (mFSAA > 0)
				{
					// try without FSAA
					testFsaa = 0;
					formatOk = mGLSupport.selectPixelFormat(mHDC, mColourDepth, testFsaa, testHwGamma);
				}

				if (!formatOk && hwGamma)
				{
					// try without sRGB
					testHwGamma = false;
					testFsaa = mFSAA;
					formatOk = mGLSupport.selectPixelFormat(mHDC, mColourDepth, testFsaa, testHwGamma);
				}

				if (!formatOk && hwGamma && (mFSAA > 0))
				{
					// try without both
					testHwGamma = false;
					testFsaa = 0;
					formatOk = mGLSupport.selectPixelFormat(mHDC, mColourDepth, testFsaa, testHwGamma);
				}

				if (!formatOk)
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
						"selectPixelFormat failed", "Win32Window::create");

			}
			// record what gamma option we used in the end
			// this will control enabling of sRGB state flags when used
			mHwGamma = testHwGamma;
			mFSAA = testFsaa;
		}
		if (!mIsExternalGLContext)
		{
			mGlrc = wglCreateContext(mHDC);
			if (!mGlrc)
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"wglCreateContext failed: " + translateWGLError(), "Win32Window::create");
		}
		if (!wglMakeCurrent(mHDC, mGlrc))
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "wglMakeCurrent", "Win32Window::create");

		// Do not change vsync if the external window has the OpenGL control
		if (!mIsExternalGLControl) {
			// Don't use wglew as if this is the first window, we won't have initialised yet
			PFNWGLSWAPINTERVALEXTPROC _wglSwapIntervalEXT = 
				(PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
			if (_wglSwapIntervalEXT)
				_wglSwapIntervalEXT(vsync? 1 : 0);
		}

        if (old_context && old_context != mGlrc)
        {
            // Restore old context
		    if (!wglMakeCurrent(old_hdc, old_context))
			    OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "wglMakeCurrent() failed", "Win32Window::create");

            // Share lists with old context
		    if (!wglShareLists(old_context, mGlrc))
			    OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "wglShareLists() failed", " Win32Window::create");
        }

		// Create RenderSystem context
		mContext = new Win32Context(mHDC, mGlrc);

		mActive = true;
	}

	void Win32Window::setFullscreen(bool fullScreen, unsigned int width, unsigned int height)
	{
		if (mIsFullScreen != fullScreen || width != mWidth || height != mHeight)
		{
			mIsFullScreen = fullScreen;
			DWORD dwStyle = WS_VISIBLE | WS_CLIPCHILDREN;

			if (mIsFullScreen)
			{
				dwStyle |= WS_POPUP;

				DEVMODE dm;
				dm.dmSize = sizeof(DEVMODE);
				dm.dmBitsPerPel = mColourDepth;
				dm.dmPelsWidth = width;
				dm.dmPelsHeight = height;
				dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
				if (mDisplayFrequency)
				{
					dm.dmDisplayFrequency = mDisplayFrequency;
					dm.dmFields |= DM_DISPLAYFREQUENCY;
					if (ChangeDisplaySettings(&dm, CDS_FULLSCREEN | CDS_TEST) != DISP_CHANGE_SUCCESSFUL)
					{
						LogManager::getSingleton().logMessage(LML_NORMAL, "ChangeDisplaySettings with user display frequency failed");
						dm.dmFields ^= DM_DISPLAYFREQUENCY;
					}
				}
				else
				{
					// try a few
					dm.dmDisplayFrequency = 100;
					dm.dmFields |= DM_DISPLAYFREQUENCY;
					if (ChangeDisplaySettings(&dm, CDS_FULLSCREEN | CDS_TEST) != DISP_CHANGE_SUCCESSFUL)
					{
						dm.dmDisplayFrequency = 75;
						if (ChangeDisplaySettings(&dm, CDS_FULLSCREEN | CDS_TEST) != DISP_CHANGE_SUCCESSFUL)
						{
							dm.dmFields ^= DM_DISPLAYFREQUENCY;
						}
					}

				}
				if (ChangeDisplaySettings(&dm, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
					LogManager::getSingleton().logMessage(LML_CRITICAL, "ChangeDisplaySettings failed");

				SetWindowLong(mHWnd, GWL_STYLE, dwStyle);
				SetWindowPos(mHWnd, HWND_TOPMOST, 0, 0, width, height,
					SWP_NOACTIVATE);
				mWidth = width;
				mHeight = height;


			}
			else
			{
				dwStyle |= WS_OVERLAPPEDWINDOW;

				// drop out of fullscreen
				ChangeDisplaySettings(NULL, 0);

				// calculate overall dimensions for requested client area
				RECT rc = { 0, 0, width, height };
				AdjustWindowRect(&rc, dwStyle, false);
				unsigned int winWidth = rc.right - rc.left;
				unsigned int winHeight = rc.bottom - rc.top;

				int screenw = GetSystemMetrics(SM_CXSCREEN);
				int screenh = GetSystemMetrics(SM_CYSCREEN);
				int left = (screenw - winWidth) / 2;
				int top = (screenh - winHeight) / 2;


				SetWindowLong(mHWnd, GWL_STYLE, dwStyle);
				SetWindowPos(mHWnd, HWND_NOTOPMOST, left, top, winWidth, winHeight,
					SWP_DRAWFRAME | SWP_FRAMECHANGED | SWP_NOACTIVATE);
				mWidth = width;
				mHeight = height;

			}

		}
	}

	void Win32Window::destroy(void)
	{
		if (!mHWnd)
			return;

		// Unregister and destroy OGRE GLContext
		delete mContext;

		if (!mIsExternalGLContext && mGlrc)
		{
			wglDeleteContext(mGlrc);
			mGlrc = 0;
		}
		if (!mIsExternal)
		{
			WindowEventUtilities::_removeRenderWindow(this);

			if (mIsFullScreen)
				ChangeDisplaySettings(NULL, 0);
			DestroyWindow(mHWnd);
		}
		else
		{
			// just release the DC
			ReleaseDC(mHWnd, mHDC);
		}

		mActive = false;
		mClosed = true;
		mHDC = 0; // no release thanks to CS_OWNDC wndclass style
		mHWnd = 0;
	}

	bool Win32Window::isVisible() const
	{
		return (mHWnd && !IsIconic(mHWnd));
	}

	bool Win32Window::isClosed() const
	{
		return mClosed;
	}

	void Win32Window::reposition(int left, int top)
	{
		if (mHWnd && !mIsFullScreen)
		{
			SetWindowPos(mHWnd, 0, left, top, 0, 0,
				SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}

	void Win32Window::resize(unsigned int width, unsigned int height)
	{
		if (mHWnd && !mIsFullScreen)
		{
			RECT rc = { 0, 0, width, height };
			AdjustWindowRect(&rc, GetWindowLong(mHWnd, GWL_STYLE), false);
			width = rc.right - rc.left;
			height = rc.bottom - rc.top;
			SetWindowPos(mHWnd, 0, 0, 0, width, height,
				SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}

	void Win32Window::windowMovedOrResized()
	{
		if (!isVisible())
			return;

		RECT rc;
		// top and left represent outer window position
		GetWindowRect(mHWnd, &rc);
		mTop = rc.top;
		mLeft = rc.left;
		// width and height represent drawable area only
		GetClientRect(mHWnd, &rc);

		if (mWidth == rc.right && mHeight == rc.bottom)
			return;

		mWidth = rc.right;
		mHeight = rc.bottom;

		// Notify viewports of resize
		ViewportList::iterator it, itend;
		itend = mViewportList.end();
		for( it = mViewportList.begin(); it != itend; ++it )
			(*it).second->_updateDimensions();
	}

	void Win32Window::swapBuffers(bool waitForVSync)
	{
	  if (!mIsExternalGLControl) {
	  	SwapBuffers(mHDC);
	  }
	}

	void Win32Window::copyContentsToMemory(const PixelBox &dst, FrameBuffer buffer)
	{
		if ((dst.left < 0) || (dst.right > mWidth) ||
			(dst.top < 0) || (dst.bottom > mHeight) ||
			(dst.front != 0) || (dst.back != 1))
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
						"Invalid box.",
						"Win32Window::copyContentsToMemory" );
		}

		if (buffer == FB_AUTO)
		{
			buffer = mIsFullScreen? FB_FRONT : FB_BACK;
		}

		GLenum format = Ogre::GLPixelUtil::getGLOriginFormat(dst.format);
		GLenum type = Ogre::GLPixelUtil::getGLOriginDataType(dst.format);

		if ((format == GL_NONE) || (type == 0))
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
						"Unsupported format.",
						"Win32Window::copyContentsToMemory" );
		}


		// Switch context if different from current one
		RenderSystem* rsys = Root::getSingleton().getRenderSystem();
		rsys->_setViewport(this->getViewport(0));

		// Must change the packing to ensure no overruns!
		glPixelStorei(GL_PACK_ALIGNMENT, 1);

		glReadBuffer((buffer == FB_FRONT)? GL_FRONT : GL_BACK);
		glReadPixels((GLint)dst.left, (GLint)dst.top,
					 (GLsizei)dst.getWidth(), (GLsizei)dst.getHeight(),
					 format, type, dst.data);

		// restore default alignment
		glPixelStorei(GL_PACK_ALIGNMENT, 4);

		//vertical flip
		{
			size_t rowSpan = dst.getWidth() * PixelUtil::getNumElemBytes(dst.format);
			size_t height = dst.getHeight();
			uchar *tmpData = new uchar[rowSpan * height];
			uchar *srcRow = (uchar *)dst.data, *tmpRow = tmpData + (height - 1) * rowSpan;

			while (tmpRow >= tmpData)
			{
				memcpy(tmpRow, srcRow, rowSpan);
				srcRow += rowSpan;
				tmpRow -= rowSpan;
			}
			memcpy(dst.data, tmpData, rowSpan * height);

			delete [] tmpData;
		}
	}

	void Win32Window::getCustomAttribute( const String& name, void* pData )
	{
		if( name == "GLCONTEXT" ) {
			*static_cast<GLContext**>(pData) = mContext;
			return;
		} else if( name == "WINDOW" )
		{
			HWND *pHwnd = (HWND*)pData;
			*pHwnd = getWindowHandle();
			return;
		} 
	}

	void Win32Window::setActive( bool state )
	{
		mActive = state;

		if( mIsFullScreen )
		{
			if( state == false )
			{	//Restore Desktop
				ChangeDisplaySettings(NULL, 0);
				ShowWindow(mHWnd, SW_SHOWMINNOACTIVE);
			}
			else
			{	//Restore App
				ShowWindow(mHWnd, SW_SHOWNORMAL);

				DEVMODE dm;
				dm.dmSize = sizeof(DEVMODE);
				dm.dmBitsPerPel = mColourDepth;
				dm.dmPelsWidth = mWidth;
				dm.dmPelsHeight = mHeight;
				dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
				if (mDisplayFrequency)
				{
					dm.dmDisplayFrequency = mDisplayFrequency;
					dm.dmFields |= DM_DISPLAYFREQUENCY;
				}
				ChangeDisplaySettings(&dm, CDS_FULLSCREEN);
			}
		}
	}
}
