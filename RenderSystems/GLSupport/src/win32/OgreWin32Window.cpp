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

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0502
#endif
#include "OgreWin32Window.h"
#include "OgreRoot.h"
#include "OgreViewport.h"
#include "OgreLogManager.h"
#include "OgreRenderSystem.h"
#include "OgreImageCodec.h"
#include "OgreStringConverter.h"
#include "OgreException.h"
#include "OgreWin32GLSupport.h"
#include "OgreWin32Context.h"

#include "OgreDepthBuffer.h"

#include "OgreGLRenderSystemCommon.h"

namespace Ogre {

    #define _MAX_CLASS_NAME_ 128

    Win32Window::Win32Window(Win32GLSupport &glsupport): GLWindow(),
        mGLSupport(glsupport)
    {
        mHWnd = 0;
        mGlrc = 0;
        mOwnsGLContext = true;
        mSizing = false;
        mDisplayFrequency = 0;
        mDeviceName = NULL;
        mWindowedWinStyle = 0;
        mFullscreenWinStyle = 0;
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

		HINSTANCE hInst = NULL;
		#ifdef __MINGW32__
			#ifdef OGRE_STATIC_LIB
        		hInst = GetModuleHandle( NULL );
			#else
				#if OGRE_DEBUG_MODE == 1
					hInst = GetModuleHandle("GLSupport_d.dll");
				#else
					hInst = GetModuleHandle("GLSupport.dll");
				#endif
			#endif
		#else
			static const TCHAR staticVar;
			GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, &staticVar, &hInst);
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
        WNDPROC windowProc = DefWindowProc;
        String title = name;
        bool hidden = false;
        String border;
        bool outerSize = false;
        bool hwGamma = false;
        bool enableDoubleClick = false;
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
                mVSync = StringConverter::parseBool(opt->second);

            if ((opt = miscParams->find("hidden")) != end)
                hidden = StringConverter::parseBool(opt->second);

            if ((opt = miscParams->find("vsyncInterval")) != end)
                mVSyncInterval = StringConverter::parseUnsignedInt(opt->second);

            if ((opt = miscParams->find("FSAA")) != end)
                mFSAA = StringConverter::parseUnsignedInt(opt->second);

            if ((opt = miscParams->find("FSAAHint")) != end)
                mFSAAHint = opt->second;

            if ((opt = miscParams->find("gamma")) != end)
                hwGamma = StringConverter::parseBool(opt->second);

#if OGRE_NO_QUAD_BUFFER_STEREO == 0
			if ((opt = miscParams->find("stereoMode")) != end)
			{
				mStereoEnabled = StringConverter::parseBool(opt->second);
			}
#endif



            if ((opt = miscParams->find("parentWindowHandle")) != end ||
                (opt = miscParams->find("externalWindowHandle")) != end)
            {
                mHWnd = (HWND)StringConverter::parseSizeT(opt->second);
                if (mHWnd)
                {
                    mIsExternal = true;
                    mIsFullScreen = false;
                }

                if ((opt = miscParams->find("externalGLControl")) != end) {
                  mIsExternalGLControl = StringConverter::parseBool(opt->second);
                }
            }

            if ((opt = miscParams->find("currentGLContext")) != end )
            {
                if( StringConverter::parseBool(opt->second) )
                {
                    mGlrc = wglGetCurrentContext();

                    if ( mGlrc )
                    {
                        mOwnsGLContext = false;
                    }
                    else
                    {
                        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        "currentGLContext was specified with no current GL context", "Win32Window::create");
                    }
                }
            }
            if ((opt = miscParams->find("externalGLContext")) != end)
            {
                mGlrc = (HGLRC)StringConverter::parseSizeT(opt->second);

                if( mGlrc )
                {
                    mOwnsGLContext = false;
                }
                else
                {
                    OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                    "parsing the value of 'externalGLContext' failed: " + translateWGLError(), "Win32Window::create");
                }
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

            if ((opt = miscParams->find("windowProc")) != end)
                windowProc = reinterpret_cast<WNDPROC>(StringConverter::parseSizeT(opt->second));

            // monitor index
            if ((opt = miscParams->find("monitorIndex")) != end)
                monitorIndex = StringConverter::parseInt(opt->second);
            
            // monitor handle
            if ((opt = miscParams->find("monitorHandle")) != end)
                hMonitor = (HMONITOR)StringConverter::parseSizeT(opt->second);

            // enable double click messages
            if ((opt = miscParams->find("enableDoubleClick")) != end)
                enableDoubleClick = StringConverter::parseBool(opt->second);

        }

        if (!mIsExternal)
        {           
            DWORD         dwStyleEx = 0;                    
            MONITORINFOEX monitorInfoEx;
            RECT          rc;
            
            // If we didn't specified the adapter index, or if it didn't find it
            if (hMonitor == NULL)
            {
                POINT windowAnchorPoint;

                // Fill in anchor point.
                windowAnchorPoint.x = left;
                windowAnchorPoint.y = top;


                // Get the nearest monitor to this window.
                hMonitor = MonitorFromPoint(windowAnchorPoint, MONITOR_DEFAULTTOPRIMARY);
            }

            // Get the target monitor info      
            memset(&monitorInfoEx, 0, sizeof(MONITORINFOEX));
            monitorInfoEx.cbSize = sizeof(MONITORINFOEX);
            GetMonitorInfo(hMonitor, &monitorInfoEx);

            size_t devNameLen = strlen(monitorInfoEx.szDevice);
            mDeviceName = new char[devNameLen + 1];

            strcpy(mDeviceName, monitorInfoEx.szDevice);

            // Update window style flags.
            mFullscreenWinStyle = (hidden ? 0 : WS_VISIBLE) | WS_CLIPCHILDREN | WS_POPUP;
            mWindowedWinStyle   = (hidden ? 0 : WS_VISIBLE) | WS_CLIPCHILDREN;
            
            if (border == "none")
                mWindowedWinStyle |= WS_POPUP;
            else if (border == "fixed")
                mWindowedWinStyle |= WS_OVERLAPPED | WS_BORDER | WS_CAPTION |
                WS_SYSMENU | WS_MINIMIZEBOX;
            else
                mWindowedWinStyle |= WS_OVERLAPPEDWINDOW;

            // No specified top left -> Center the window in the middle of the monitor
            if (left == -1 || top == -1)
            {               
                uint screenw = monitorInfoEx.rcWork.right  - monitorInfoEx.rcWork.left;
                uint screenh = monitorInfoEx.rcWork.bottom - monitorInfoEx.rcWork.top;

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
                dwStyleEx |= WS_EX_TOPMOST;
                mTop = monitorInfoEx.rcMonitor.top;
                mLeft = monitorInfoEx.rcMonitor.left;                                           
            }
            else
            {
                int screenw = GetSystemMetrics(SM_CXSCREEN);
                int screenh = GetSystemMetrics(SM_CYSCREEN);

                if (!outerSize)
                {
                    // Calculate window dimensions required
                    // to get the requested client area
                    SetRect(&rc, 0, 0, mWidth, mHeight);
                    AdjustWindowRect(&rc, getWindowStyle(fullScreen), false);
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
 
            UINT classStyle = CS_OWNDC;
            if (enableDoubleClick)
                classStyle |= CS_DBLCLKS;

            // register class and create window
            WNDCLASS wc = { classStyle, windowProc, 0, 0, hInst,
                LoadIcon(NULL, IDI_APPLICATION), LoadCursor(NULL, IDC_ARROW),
                (HBRUSH)GetStockObject(BLACK_BRUSH), NULL, "OgreGLWindow" };
            RegisterClass(&wc);

            if (mIsFullScreen)
            {
                switchMode(mWidth, mHeight, mDisplayFrequency);
            }

            // Pass pointer to self as WM_CREATE parameter
            mHWnd = CreateWindowEx(dwStyleEx, "OgreGLWindow", title.c_str(),
                getWindowStyle(fullScreen), mLeft, mTop, mWidth, mHeight, 0, 0, hInst, this);

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
            bool formatOk = mGLSupport.selectPixelFormat(mHDC, mColourDepth, testFsaa, testHwGamma, mStereoEnabled);
            if (!formatOk)
            {
                if (mFSAA > 0)
                {
                    // try without FSAA
                    testFsaa = 0;
                    formatOk = mGLSupport.selectPixelFormat(mHDC, mColourDepth, testFsaa, testHwGamma, mStereoEnabled);
                }

                if (!formatOk && hwGamma)
                {
                    // try without sRGB
                    testHwGamma = false;
                    testFsaa = mFSAA;
                    formatOk = mGLSupport.selectPixelFormat(mHDC, mColourDepth, testFsaa, testHwGamma, mStereoEnabled);
                }

                if (!formatOk && hwGamma && (mFSAA > 0))
                {
                    // try without both
                    testHwGamma = false;
                    testFsaa = 0;
                    formatOk = mGLSupport.selectPixelFormat(mHDC, mColourDepth, testFsaa, testHwGamma, mStereoEnabled);
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
        if (mOwnsGLContext)
        {
            // New context is shared with previous one
            mGlrc = mGLSupport.createNewContext(mHDC, old_context);
        }

        if (!wglMakeCurrent(mHDC, mGlrc))
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "wglMakeCurrent", "Win32Window::create");

        // Do not change vsync if the external window has the OpenGL control
        if (!mIsExternalGLControl) {
            // Don't use wglew as if this is the first window, we won't have initialised yet
            PFNWGLSWAPINTERVALEXTPROC _wglSwapIntervalEXT = 
                (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
            if (_wglSwapIntervalEXT)
                _wglSwapIntervalEXT(mVSync? mVSyncInterval : 0);
        }

        if (old_context && old_context != mGlrc)
        {
            // Restore old context
            if (!wglMakeCurrent(old_hdc, old_context))
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "wglMakeCurrent() failed", "Win32Window::create");
        }

        // Create RenderSystem context
        mContext = new Win32Context(mHDC, mGlrc, mGLSupport);

        mActive = true;
        setHidden(hidden);
    }

    void Win32Window::adjustWindow(unsigned int clientWidth, unsigned int clientHeight, 
        unsigned int* winWidth, unsigned int* winHeight)
    {
        // NB only call this for non full screen
        RECT rc;
        SetRect(&rc, 0, 0, clientWidth, clientHeight);
        AdjustWindowRect(&rc, getWindowStyle(mIsFullScreen), false);
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

    void Win32Window::switchMode(uint width, uint height, uint frequency)
    {
        DEVMODE displayDeviceMode = {};

        displayDeviceMode.dmSize = sizeof(DEVMODE);
        displayDeviceMode.dmBitsPerPel = mColourDepth;
        displayDeviceMode.dmPelsWidth = width;
        displayDeviceMode.dmPelsHeight = height;
        displayDeviceMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

        if (frequency)
        {
            displayDeviceMode.dmDisplayFrequency = frequency;
            displayDeviceMode.dmFields |= DM_DISPLAYFREQUENCY;
            if (ChangeDisplaySettingsEx(mDeviceName, &displayDeviceMode, NULL, CDS_FULLSCREEN | CDS_TEST, NULL) != DISP_CHANGE_SUCCESSFUL)
            {
                LogManager::getSingleton().logWarning("ChangeDisplaySettings with user display frequency failed");
                displayDeviceMode.dmFields ^= DM_DISPLAYFREQUENCY;
            }
        }
        if (ChangeDisplaySettingsEx(mDeviceName, &displayDeviceMode, NULL, CDS_FULLSCREEN, NULL) != DISP_CHANGE_SUCCESSFUL)
            LogManager::getSingleton().logError("ChangeDisplaySettings failed");
    }

    void Win32Window::setFullscreen(bool fullScreen, unsigned int width, unsigned int height)
    {
        if(mIsExternal)
            return;

        if (mIsFullScreen != fullScreen || width != mWidth || height != mHeight)
        {
            mIsFullScreen = fullScreen;
            
            if (mIsFullScreen)
            {
                switchMode(width, height, mDisplayFrequency);

                // Get the nearest monitor to this window.
                HMONITOR hMonitor = MonitorFromWindow(mHWnd, MONITOR_DEFAULTTONEAREST);

                // Get monitor info 
                MONITORINFO monitorInfo;

                memset(&monitorInfo, 0, sizeof(MONITORINFO));
                monitorInfo.cbSize = sizeof(MONITORINFO);
                GetMonitorInfo(hMonitor, &monitorInfo);

                mTop = monitorInfo.rcMonitor.top;
                mLeft = monitorInfo.rcMonitor.left;

                SetWindowLong(mHWnd, GWL_STYLE, getWindowStyle(mIsFullScreen));
                SetWindowPos(mHWnd, HWND_TOPMOST, mLeft, mTop, width, height,
                    SWP_NOACTIVATE);
                mWidth = width;
                mHeight = height;


            }
            else
            {               
                // drop out of fullscreen
                ChangeDisplaySettingsEx(mDeviceName, NULL, NULL, 0, NULL);

                // calculate overall dimensions for requested client area
                unsigned int winWidth, winHeight;
                adjustWindow(width, height, &winWidth, &winHeight);

                // deal with centering when switching down to smaller resolution

                HMONITOR hMonitor = MonitorFromWindow(mHWnd, MONITOR_DEFAULTTONEAREST);
                MONITORINFO monitorInfo;
                memset(&monitorInfo, 0, sizeof(MONITORINFO));
                monitorInfo.cbSize = sizeof(MONITORINFO);
                GetMonitorInfo(hMonitor, &monitorInfo);

                uint screenw = monitorInfo.rcWork.right  - monitorInfo.rcWork.left;
                uint screenh = monitorInfo.rcWork.bottom - monitorInfo.rcWork.top;


                int left = screenw > winWidth ? ((screenw - winWidth) / 2) : 0;
                int top = screenh > winHeight ? ((screenh - winHeight) / 2) : 0;

                SetWindowLong(mHWnd, GWL_STYLE, getWindowStyle(mIsFullScreen));
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
        mContext = 0;

        if (mOwnsGLContext)
        {
            wglDeleteContext(mGlrc);
            mGlrc = 0;
        }
        if (!mIsExternal)
        {
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

    void Win32Window::setHidden(bool hidden)
    {
        mHidden = hidden;
        if (!mIsExternal)
        {
            if (hidden)
                ShowWindow(mHWnd, SW_HIDE);
            else
                ShowWindow(mHWnd, SW_SHOWNORMAL);
        }
    }

    void Win32Window::setVSyncEnabled(bool vsync)
    {
        mVSync = vsync;
        HDC old_hdc = wglGetCurrentDC();
        HGLRC old_context = wglGetCurrentContext();
        if (!wglMakeCurrent(mHDC, mGlrc))
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "wglMakeCurrent", "Win32Window::setVSyncEnabled");

        // Do not change vsync if the external window has the OpenGL control
        if (!mIsExternalGLControl) {
            // Don't use wglew as if this is the first window, we won't have initialised yet
            PFNWGLSWAPINTERVALEXTPROC _wglSwapIntervalEXT = 
                (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
            if (_wglSwapIntervalEXT)
                _wglSwapIntervalEXT(mVSync? mVSyncInterval : 0);
        }

        if (old_context && old_context != mGlrc)
        {
            // Restore old context
            if (!wglMakeCurrent(old_hdc, old_context))
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "wglMakeCurrent() failed", "Win32Window::setVSyncEnabled");
        }
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
        if (!isVisible())
            return;

        if (!mIsExternal && !mIsFullScreen && width > 0 && height > 0)
        {
            RECT rc = { 0, 0, int(width), int(height) };
            AdjustWindowRect(&rc, getWindowStyle(mIsFullScreen), false);
            width = rc.right - rc.left;
            height = rc.bottom - rc.top;
            SetWindowPos(mHWnd, 0, 0, 0, width, height,
                SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
            return;
        }

        updateWindowRect();
    }

    void Win32Window::windowMovedOrResized()
    {
        if (!isVisible())
            return;

        updateWindowRect();     
    }


    void Win32Window::updateWindowRect()
    {
        RECT rc;
        BOOL result;

        // Update top left parameters
        result = GetWindowRect(mHWnd, &rc);
        if (result == FALSE)
        {
            mTop = 0;
            mLeft = 0;
            mWidth = 0;
            mHeight = 0;
            return;
        }

        mTop = rc.top;
        mLeft = rc.left;

        // width and height represent drawable area only
        result = GetClientRect(mHWnd, &rc);
        if (result == FALSE)
        {
            mTop = 0;
            mLeft = 0;
            mWidth = 0;
            mHeight = 0;
            return;
        }
        unsigned int width = rc.right - rc.left;
        unsigned int height = rc.bottom - rc.top;

        // Case window resized.
        if (width != mWidth || height != mHeight)
        {
            mWidth  = rc.right - rc.left;
            mHeight = rc.bottom - rc.top;

            // Notify viewports of resize
            ViewportList::iterator it = mViewportList.begin();
            while( it != mViewportList.end() )
                (*it++).second->_updateDimensions();            
        }
    }


    void Win32Window::swapBuffers()
    {
      if (!mIsExternalGLControl) {
        SwapBuffers(mHDC);
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
            {   //Restore Desktop
                ChangeDisplaySettingsEx(mDeviceName, NULL, NULL, 0, NULL);
                ShowWindow(mHWnd, SW_SHOWMINNOACTIVE);
            }
            else
            {
                switchMode(mWidth, mHeight, mDisplayFrequency);
            }
        }
    }
}
