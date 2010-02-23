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

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
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
#include "OgreDepthBuffer.h"

namespace Ogre {

	#define _MAX_CLASS_NAME_ 128

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
		mDeviceName = NULL;
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
		mDisplayFrequency = 0;
		mDepthBufferPoolId = DepthBuffer::POOL_DEFAULT;
		mColourDepth = mIsFullScreen? 32 : GetDeviceCaps(GetDC(0), BITSPIXEL);
		int left = -1; // Defaults to screen center
		int top = -1; // Defaults to screen center
		HWND parent = 0;
		String title = name;
		bool vsync = false;
		unsigned int vsyncInterval = 1;
		String border;
		bool outerSize = false;
		bool hwGamma = false;
		int monitorIndex = -1;
		HMONITOR hMonitor = NULL;

		if(miscParams)
		{
			// Get variable-length params
			NameValuePairList::const_iterator opt;
			NameValuePairList::const_iterator end = miscParams->end();

			if ((opt = miscParams->find("title")) != end)
				title = opt->second;

			if ((opt = miscParams->find("left")) != end)
				left = StringConverter::parseInt(opt->second);

			if ((opt = miscParams->find("top")) != end)
				top = StringConverter::parseInt(opt->second);

			if ((opt = miscParams->find("depthBuffer")) != end)
			{
				mDepthBufferPoolId = StringConverter::parseBool(opt->second) ?
												DepthBuffer::POOL_DEFAULT : DepthBuffer::POOL_NO_DEPTH;
			}

			if ((opt = miscParams->find("vsync")) != end)
				vsync = StringConverter::parseBool(opt->second);

			if ((opt = miscParams->find("vsyncInterval")) != end)
				vsyncInterval = StringConverter::parseUnsignedInt(opt->second);

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
					if ((int)mColourDepth > GetDeviceCaps(GetDC(0), BITSPIXEL))
						mColourDepth = GetDeviceCaps(GetDC(0), BITSPIXEL);
				}
			}

			// incompatible with fullscreen
			if ((opt = miscParams->find("parentWindowHandle")) != end)
				parent = (HWND)StringConverter::parseUnsignedInt(opt->second);


			// monitor index
			if ((opt = miscParams->find("monitorIndex")) != end)
				monitorIndex = StringConverter::parseInt(opt->second);
			
			// monitor handle
			if ((opt = miscParams->find("monitorHandle")) != end)
				hMonitor = (HMONITOR)StringConverter::parseInt(opt->second);			
		}

		if (!mIsExternal)
		{
			DWORD		  dwStyle = WS_VISIBLE | WS_CLIPCHILDREN;
			DWORD		  dwStyleEx = 0;					
			MONITORINFOEX monitorInfoEx;
			RECT		  rc;
			
			// If we didn't specified the adapter index, or if it didn't find it
			if (hMonitor == NULL)
			{
				POINT windowAnchorPoint;

				// Fill in anchor point.
				windowAnchorPoint.x = left;
				windowAnchorPoint.y = top;


				// Get the nearest monitor to this window.
				hMonitor = MonitorFromPoint(windowAnchorPoint, MONITOR_DEFAULTTONEAREST);
			}

			// Get the target monitor info		
			memset(&monitorInfoEx, 0, sizeof(MONITORINFOEX));
			monitorInfoEx.cbSize = sizeof(MONITORINFOEX);
			GetMonitorInfo(hMonitor, &monitorInfoEx);

			size_t devNameLen = strlen(monitorInfoEx.szDevice);
			mDeviceName = new char[devNameLen + 1];

			strcpy(mDeviceName, monitorInfoEx.szDevice);			


			// No specified top left -> Center the window in the middle of the monitor
			if (left == -1 || top == -1)
			{				
				int screenw = monitorInfoEx.rcWork.right  - monitorInfoEx.rcWork.left;
				int screenh = monitorInfoEx.rcWork.bottom - monitorInfoEx.rcWork.top;

				unsigned int winWidth, winHeight;
				adjustWindow(width, height, &winWidth, &winHeight);

				// clamp window dimensions to screen size
				int outerw = (winWidth < screenw)? winWidth : screenw;
				int outerh = (winHeight < screenh)? winHeight : screenh;

				if (left == -1)
					left = monitorInfoEx.rcWork.left + (screenw - outerw) / 2;
				else if (monitorIndex != -1)
					left += monitorInfoEx.rcWork.left;

				if (top == -1)
					top = monitorInfoEx.rcWork.top + (screenh - outerh) / 2;
				else if (monitorIndex != -1)
					top += monitorInfoEx.rcWork.top;
			}
			else if (monitorIndex != -1)
			{
				left += monitorInfoEx.rcWork.left;
				top += monitorInfoEx.rcWork.top;
			}

			mWidth = width;
			mHeight = height;
			mTop = top;
			mLeft = left;

			if (mIsFullScreen)
			{
				dwStyle |= WS_POPUP;
				dwStyleEx |= WS_EX_TOPMOST;
				mTop = monitorInfoEx.rcMonitor.top;
				mLeft = monitorInfoEx.rcMonitor.left;											
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
					// Calculate window dimensions required
					// to get the requested client area
					SetRect(&rc, 0, 0, mWidth, mHeight);
					AdjustWindowRect(&rc, dwStyle, false);
					mWidth = rc.right - rc.left;
					mHeight = rc.bottom - rc.top;

					// Clamp window rect to the nearest display monitor.
					if (mLeft < monitorInfoEx.rcWork.left)
						mLeft = monitorInfoEx.rcWork.left;		

					if (mTop < monitorInfoEx.rcWork.top)					
						mTop = monitorInfoEx.rcWork.top;					

					if ((int)mWidth > monitorInfoEx.rcWork.right - mLeft)					
						mWidth = monitorInfoEx.rcWork.right - mLeft;	

					if ((int)mHeight > monitorInfoEx.rcWork.bottom - mTop)					
						mHeight = monitorInfoEx.rcWork.bottom - mTop;		
				}			
			}

			// register class and create window
			WNDCLASS wc = { CS_OWNDC, WindowEventUtilities::_WndProc, 0, 0, hInst,
				LoadIcon(NULL, IDI_APPLICATION), LoadCursor(NULL, IDC_ARROW),
				(HBRUSH)GetStockObject(BLACK_BRUSH), NULL, "OgreGLWindow" };
			RegisterClass(&wc);

			if (mIsFullScreen)
			{
				DEVMODE displayDeviceMode;

				memset(&displayDeviceMode, 0, sizeof(displayDeviceMode));
				displayDeviceMode.dmSize = sizeof(DEVMODE);
				displayDeviceMode.dmBitsPerPel = mColourDepth;
				displayDeviceMode.dmPelsWidth = mWidth;
				displayDeviceMode.dmPelsHeight = mHeight;
				displayDeviceMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

				if (mDisplayFrequency)
				{
					displayDeviceMode.dmDisplayFrequency = mDisplayFrequency;
					displayDeviceMode.dmFields |= DM_DISPLAYFREQUENCY;
					if (ChangeDisplaySettingsEx(mDeviceName, &displayDeviceMode, NULL, CDS_FULLSCREEN | CDS_TEST, NULL) != DISP_CHANGE_SUCCESSFUL)
					{
						LogManager::getSingleton().logMessage(LML_NORMAL, "ChangeDisplaySettings with user display frequency failed");
						displayDeviceMode.dmFields ^= DM_DISPLAYFREQUENCY;
					}
				}
				if (ChangeDisplaySettingsEx(mDeviceName, &displayDeviceMode, NULL, CDS_FULLSCREEN, NULL) != DISP_CHANGE_SUCCESSFUL)								
					LogManager::getSingleton().logMessage(LML_CRITICAL, "ChangeDisplaySettings failed");
			}

			// Pass pointer to self as WM_CREATE parameter
			mHWnd = CreateWindowEx(dwStyleEx, "OgreGLWindow", title.c_str(),
				dwStyle, mLeft, mTop, mWidth, mHeight, parent, 0, hInst, this);

			WindowEventUtilities::_addRenderWindow(this);

			LogManager::getSingleton().stream()
				<< "Created Win32Window '"
				<< mName << "' : " << mWidth << "x" << mHeight
				<< ", " << mColourDepth << "bpp";
			
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
				_wglSwapIntervalEXT(vsync? vsyncInterval : 0);
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

	void Win32Window::adjustWindow(unsigned int clientWidth, unsigned int clientHeight, 
		unsigned int* winWidth, unsigned int* winHeight)
	{
		// NB only call this for non full screen
		RECT rc;
		SetRect(&rc, 0, 0, clientWidth, clientHeight);
		AdjustWindowRect(&rc, WS_VISIBLE | WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW, false);
		*winWidth = rc.right - rc.left;
		*winHeight = rc.bottom - rc.top;

		// adjust to monitor
		HMONITOR hMonitor = MonitorFromWindow(mHWnd, MONITOR_DEFAULTTONEAREST);

		// Get monitor info	
		MONITORINFO monitorInfo;

		memset(&monitorInfo, 0, sizeof(MONITORINFO));
		monitorInfo.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo(hMonitor, &monitorInfo);

		LONG maxW = monitorInfo.rcWork.right  - monitorInfo.rcWork.left;
		LONG maxH = monitorInfo.rcWork.bottom - monitorInfo.rcWork.top;

		if (*winWidth > (unsigned int)maxW)
			*winWidth = maxW;
		if (*winHeight > (unsigned int)maxH)
			*winHeight = maxH;

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

				DEVMODE displayDeviceMode;

				memset(&displayDeviceMode, 0, sizeof(displayDeviceMode));
				displayDeviceMode.dmSize = sizeof(DEVMODE);
				displayDeviceMode.dmBitsPerPel = mColourDepth;
				displayDeviceMode.dmPelsWidth = width;
				displayDeviceMode.dmPelsHeight = height;
				displayDeviceMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
				if (mDisplayFrequency)
				{
					displayDeviceMode.dmDisplayFrequency = mDisplayFrequency;
					displayDeviceMode.dmFields |= DM_DISPLAYFREQUENCY;

					if (ChangeDisplaySettingsEx(mDeviceName, &displayDeviceMode, NULL, 
						CDS_FULLSCREEN | CDS_TEST, NULL) != DISP_CHANGE_SUCCESSFUL)					
					{
						LogManager::getSingleton().logMessage(LML_NORMAL, "ChangeDisplaySettings with user display frequency failed");
						displayDeviceMode.dmFields ^= DM_DISPLAYFREQUENCY;
					}
				}
				else
				{
					// try a few
					displayDeviceMode.dmDisplayFrequency = 100;
					displayDeviceMode.dmFields |= DM_DISPLAYFREQUENCY;
					if (ChangeDisplaySettingsEx(mDeviceName, &displayDeviceMode, NULL, 
						CDS_FULLSCREEN | CDS_TEST, NULL) != DISP_CHANGE_SUCCESSFUL)		
					{
						displayDeviceMode.dmDisplayFrequency = 75;
						if (ChangeDisplaySettingsEx(mDeviceName, &displayDeviceMode, NULL, 
							CDS_FULLSCREEN | CDS_TEST, NULL) != DISP_CHANGE_SUCCESSFUL)		
						{
							displayDeviceMode.dmFields ^= DM_DISPLAYFREQUENCY;
						}
					}

				}
				// move window to 0,0 before display switch
				SetWindowPos(mHWnd, HWND_TOPMOST, 0, 0, mWidth, mHeight, SWP_NOACTIVATE);

				if (ChangeDisplaySettingsEx(mDeviceName, &displayDeviceMode, NULL, CDS_FULLSCREEN, NULL) != DISP_CHANGE_SUCCESSFUL)				
					LogManager::getSingleton().logMessage(LML_CRITICAL, "ChangeDisplaySettings failed");

				// Get the nearest monitor to this window.
				HMONITOR hMonitor = MonitorFromWindow(mHWnd, MONITOR_DEFAULTTONEAREST);

				// Get monitor info	
				MONITORINFO monitorInfo;

				memset(&monitorInfo, 0, sizeof(MONITORINFO));
				monitorInfo.cbSize = sizeof(MONITORINFO);
				GetMonitorInfo(hMonitor, &monitorInfo);

				mTop = monitorInfo.rcMonitor.top;
				mLeft = monitorInfo.rcMonitor.left;

				SetWindowLong(mHWnd, GWL_STYLE, dwStyle);
				SetWindowPos(mHWnd, HWND_TOPMOST, mLeft, mTop, width, height,
					SWP_NOACTIVATE);
				mWidth = width;
				mHeight = height;


			}
			else
			{
				dwStyle |= WS_OVERLAPPEDWINDOW;

				// drop out of fullscreen
				ChangeDisplaySettingsEx(mDeviceName, NULL, NULL, 0, NULL);

				// calculate overall dimensions for requested client area
				unsigned int winWidth, winHeight;
				adjustWindow(width, height, &winWidth, &winHeight);

				// deal with centreing when switching down to smaller resolution

				HMONITOR hMonitor = MonitorFromWindow(mHWnd, MONITOR_DEFAULTTONEAREST);
				MONITORINFO monitorInfo;
				memset(&monitorInfo, 0, sizeof(MONITORINFO));
				monitorInfo.cbSize = sizeof(MONITORINFO);
				GetMonitorInfo(hMonitor, &monitorInfo);

				LONG screenw = monitorInfo.rcWork.right  - monitorInfo.rcWork.left;
				LONG screenh = monitorInfo.rcWork.bottom - monitorInfo.rcWork.top;


				int left = screenw > winWidth ? ((screenw - winWidth) / 2) : 0;
				int top = screenh > winHeight ? ((screenh - winHeight) / 2) : 0;

				SetWindowLong(mHWnd, GWL_STYLE, dwStyle);
				SetWindowPos(mHWnd, HWND_NOTOPMOST, left, top, winWidth, winHeight,
					SWP_DRAWFRAME | SWP_FRAMECHANGED | SWP_NOACTIVATE);
				mWidth = width;
				mHeight = height;

				windowMovedOrResized();

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
				ChangeDisplaySettingsEx(mDeviceName, NULL, NULL, 0, NULL);
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

		if (mDeviceName != NULL)
		{
			delete[] mDeviceName;
			mDeviceName = NULL;
		}
		
	}


	bool Win32Window::isActive(void) const
	{
		if (isFullScreen())
			return isVisible();

		return mActive && isVisible();
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
		if (!mHWnd || IsIconic(mHWnd))
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

		mWidth = rc.right - rc.left;
		mHeight = rc.bottom - rc.top;

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
		if (mDeviceName != NULL && state == false)
		{
			HWND hActiveWindow = GetActiveWindow();
			char classNameSrc[_MAX_CLASS_NAME_ + 1];
			char classNameDst[_MAX_CLASS_NAME_ + 1];

			GetClassName(mHWnd, classNameSrc, _MAX_CLASS_NAME_);
			GetClassName(hActiveWindow, classNameDst, _MAX_CLASS_NAME_);

			if (strcmp(classNameDst, classNameSrc) == 0)
			{
				state = true;
			}						
		}
		
		mActive = state;

		if( mIsFullScreen )
		{
			if( state == false )
			{	//Restore Desktop
				ChangeDisplaySettingsEx(mDeviceName, NULL, NULL, 0, NULL);
				ShowWindow(mHWnd, SW_SHOWMINNOACTIVE);
			}
			else
			{	//Restore App
				ShowWindow(mHWnd, SW_SHOWNORMAL);

				DEVMODE displayDeviceMode;

				memset(&displayDeviceMode, 0, sizeof(displayDeviceMode));
				displayDeviceMode.dmSize = sizeof(DEVMODE);
				displayDeviceMode.dmBitsPerPel = mColourDepth;
				displayDeviceMode.dmPelsWidth = mWidth;
				displayDeviceMode.dmPelsHeight = mHeight;
				displayDeviceMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
				if (mDisplayFrequency)
				{
					displayDeviceMode.dmDisplayFrequency = mDisplayFrequency;
					displayDeviceMode.dmFields |= DM_DISPLAYFREQUENCY;
				}
				ChangeDisplaySettingsEx(mDeviceName, &displayDeviceMode, NULL, CDS_FULLSCREEN, NULL);
			}
		}
	}
}
