/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
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
--------------------------------------------------------------------------*/

#include "OgreRoot.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreWindowEventUtilities.h"

#include "OgreGLESPrerequisites.h"
#include "OgreGLESRenderSystem.h"

#include "OgreWin32EGLSupport.h"
#include "OgreWin32EGLWindow.h"
#include "OgreWin32EGLContext.h"

#include <iostream>
#include <algorithm>
#include <climits>


namespace Ogre {
    Win32EGLWindow::Win32EGLWindow(Win32EGLSupport *glsupport)
		: EGLWindow(glsupport)
    {
		mGLSupport = glsupport;
		mNativeDisplay = glsupport->getNativeDisplay();
    }

    Win32EGLWindow::~Win32EGLWindow()
    {
    }

	EGLContext * Win32EGLWindow::createEGLContext() const
	{
		return new Win32EGLContext(mEglDisplay, mGLSupport, mEglConfig, mEglSurface);
	}

	void Win32EGLWindow::getLeftAndTopFromNativeWindow( int & left, int & top, uint width, uint height )
	{

	}

	void Win32EGLWindow::initNativeCreatedWindow(const NameValuePairList *miscParams)
	{

	}

	void Win32EGLWindow::createNativeWindow( int &left, int &top, uint &width, uint &height, String &title )
	{
		// destroy current window, if any
		if (mWindow)
			destroy();

#ifdef OGRE_STATIC_LIB
		HINSTANCE hInst = GetModuleHandle( NULL );
#else
#  if OGRE_DEBUG_MODE == 1
		HINSTANCE hInst = GetModuleHandle("RenderSystem_GLES_d.dll");
#  else
		HINSTANCE hInst = GetModuleHandle("RenderSystem_GLES.dll");
#  endif
#endif

		mWindow = 0;
		mClosed = false;		
		mIsDepthBuffered = true;
		mColourDepth = mIsFullScreen? 32 : GetDeviceCaps(GetDC(0), BITSPIXEL);
		HWND parent = 0;
		bool vsync = false;
		String border;
		bool outerSize = false;
		bool hwGamma = false;
		int monitorIndex = -1;
		HMONITOR hMonitor = NULL;


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

			//size_t devNameLen = strlen(monitorInfoEx.szDevice);
			//mDeviceName = new char[devNameLen + 1];

			//strcpy(mDeviceName, monitorInfoEx.szDevice);			


			// No specified top left -> Center the window in the middle of the monitor
			if (left == -1 || top == -1)
			{				
				int screenw = monitorInfoEx.rcMonitor.right  - monitorInfoEx.rcMonitor.left;
				int screenh = monitorInfoEx.rcMonitor.bottom - monitorInfoEx.rcMonitor.top;

				SetRect(&rc, 0, 0, width, height);
				AdjustWindowRect(&rc, dwStyle, false);

				// clamp window dimensions to screen size
				int outerw = (rc.right-rc.left < screenw)? rc.right-rc.left : screenw;
				int outerh = (rc.bottom-rc.top < screenh)? rc.bottom-rc.top : screenh;

				if (left == -1)
					left = monitorInfoEx.rcMonitor.left + (screenw - outerw) / 2;
				else if (monitorIndex != -1)
					left += monitorInfoEx.rcMonitor.left;

				if (top == -1)
					top = monitorInfoEx.rcMonitor.top + (screenh - outerh) / 2;
				else if (monitorIndex != -1)
					top += monitorInfoEx.rcMonitor.top;
			}
			else if (monitorIndex != -1)
			{
				left += monitorInfoEx.rcMonitor.left;
				top += monitorInfoEx.rcMonitor.top;
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
					if (mLeft < monitorInfoEx.rcMonitor.left)
						mLeft = monitorInfoEx.rcMonitor.left;		

					if (mTop < monitorInfoEx.rcMonitor.top)					
						mTop = monitorInfoEx.rcMonitor.top;					

					if ((int)mWidth > monitorInfoEx.rcMonitor.right - mLeft)					
						mWidth = monitorInfoEx.rcMonitor.right - mLeft;	

					if ((int)mHeight > monitorInfoEx.rcMonitor.bottom - mTop)					
						mHeight = monitorInfoEx.rcMonitor.bottom - mTop;		
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
/*
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
*/

			}
			// Pass pointer to self as WM_CREATE parameter
			mWindow = CreateWindowEx(dwStyleEx, "OgreGLWindow", title.c_str(),
				dwStyle, mLeft, mTop, mWidth, mHeight, parent, 0, hInst, this);

			WindowEventUtilities::_addRenderWindow(this);

			LogManager::getSingleton().stream()
				<< "Created Win32Window '"
				<< mName << "' : " << mWidth << "x" << mHeight
				<< ", " << mColourDepth << "bpp";

		}

		RECT rc;
		// top and left represent outer window position
		GetWindowRect(mWindow, &rc);
		mTop = rc.top;
		mLeft = rc.left;
		// width and height represent drawable area only
		GetClientRect(mWindow, &rc);
		mWidth = rc.right;
		mHeight = rc.bottom;

		mNativeDisplay = GetDC(mWindow);
		mEglDisplay = eglGetDisplay(mNativeDisplay);
		mGLSupport->setGLDisplay(mEglDisplay);
		mEglSurface = createSurfaceFromWindow(mEglDisplay, mWindow);


	}

	void Win32EGLWindow::reposition( int left, int top )
	{

	}

	void Win32EGLWindow::resize( unsigned int width, unsigned int height )
	{

	}

	void Win32EGLWindow::windowMovedOrResized()
	{

	}

	void Win32EGLWindow::switchFullScreen( bool fullscreen )
	{

	}

    void Win32EGLWindow::create(const String& name, uint width, uint height,
                                bool fullScreen, const NameValuePairList *miscParams)
    {
        String title = name;
        uint samples = 0;
        int gamma;
        short frequency = 0;
        bool vsync = false;
        ::EGLContext eglContext = 0;
		int left = 0;
		int top  = 0;

		getLeftAndTopFromNativeWindow(left, top, width, height);

        mIsFullScreen = fullScreen;

        if (miscParams)
        {
            NameValuePairList::const_iterator opt;
            NameValuePairList::const_iterator end = miscParams->end();

            if ((opt = miscParams->find("currentGLContext")) != end &&
                StringConverter::parseBool(opt->second))
            {
                eglContext = eglGetCurrentContext();
                if (eglContext)
                {
                    OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                                "currentGLContext was specified with no current GL context",
                                "EGLWindow::create");
                }

                eglContext = eglGetCurrentContext();
                mEglSurface = eglGetCurrentSurface(EGL_DRAW);
            }

            // Note: Some platforms support AA inside ordinary windows
            if ((opt = miscParams->find("FSAA")) != end)
            {
                samples = StringConverter::parseUnsignedInt(opt->second);
            }

            if ((opt = miscParams->find("displayFrequency")) != end)
            {
                frequency = (short)StringConverter::parseInt(opt->second);
            }

            if ((opt = miscParams->find("vsync")) != end)
            {
                vsync = StringConverter::parseBool(opt->second);
            }

            if ((opt = miscParams->find("gamma")) != end)
            {
                gamma = StringConverter::parseBool(opt->second);
            }

            if ((opt = miscParams->find("left")) != end)
            {
                left = StringConverter::parseInt(opt->second);
            }

            if ((opt = miscParams->find("top")) != end)
            {
                top = StringConverter::parseInt(opt->second);
            }

            if ((opt = miscParams->find("title")) != end)
            {
                title = opt->second;
            }

            if ((opt = miscParams->find("externalGLControl")) != end)
            {
                mIsExternalGLControl = StringConverter::parseBool(opt->second);
            }
		}

		initNativeCreatedWindow(miscParams);

        if (mEglSurface)
        {
            mEglConfig = mGLSupport->getGLConfigFromDrawable (mEglSurface, &width, &height);
        }

        if (!mEglConfig && eglContext)
        {
            mEglConfig = mGLSupport->getGLConfigFromContext(eglContext);

            if (!mEglConfig)
            {
                // This should never happen.
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                            "Unexpected failure to determine a EGLFBConfig",
                            "EGLWindow::create");
            }
        }

        mIsExternal = (mEglSurface != 0);



        if (!mEglConfig)
        {
            int minAttribs[] = {
                EGL_LEVEL, 0,
                EGL_DEPTH_SIZE, 16,
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                EGL_NONE
            };

            int maxAttribs[] = {
                EGL_SAMPLES, samples,
                EGL_STENCIL_SIZE, INT_MAX,
                EGL_NONE
            };

            mEglConfig = mGLSupport->selectGLConfig(minAttribs, maxAttribs);
            mHwGamma = false;
        }

        if (!mIsTopLevel)
        {
            mIsFullScreen = false;
            left = top = 0;
        }

        if (mIsFullScreen)
        {
            mGLSupport->switchMode (width, height, frequency);
        }

		if (!mIsExternal)
        {
			createNativeWindow(left, top, width, height, title);
		}

		mContext = createEGLContext();

        ::EGLSurface oldDrawableDraw = eglGetCurrentSurface(EGL_DRAW);
        ::EGLSurface oldDrawableRead = eglGetCurrentSurface(EGL_READ);
        ::EGLContext oldContext  = eglGetCurrentContext();

        int glConfigID;

        mGLSupport->getGLConfigAttrib(mEglConfig, EGL_CONFIG_ID, &glConfigID);
        LogManager::getSingleton().logMessage("EGLWindow::create used FBConfigID = " + StringConverter::toString(glConfigID));

        mName = name;
        mWidth = width;
        mHeight = height;
        mLeft = left;
        mTop = top;
        mActive = true;
		mVisible = true;

        mClosed = false;
	}

}
