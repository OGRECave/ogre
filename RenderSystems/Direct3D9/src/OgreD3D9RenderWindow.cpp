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
#include "OgreD3D9RenderWindow.h"
#include "OgreLogManager.h"
#include "OgreException.h"
#include "OgreD3D9RenderSystem.h"
#include "OgreRenderSystem.h"
#include "OgreBitwise.h"
#include "OgreStringConverter.h"
#include "OgreRoot.h"
#include "OgreD3D9DeviceManager.h"
#include "OgreDepthBuffer.h"

#if OGRE_NO_QUAD_BUFFER_STEREO == 0
#include "OgreD3D9StereoDriverBridge.h"
#endif

namespace Ogre
{
    D3D9RenderWindow::D3D9RenderWindow(HINSTANCE instance)
        : mInstance(instance)        
    {
        mDevice = NULL;
        mIsFullScreen = false;      
        mIsExternal = false;
        mHWnd = 0;
        mActive = false;
        mHidden = false;
        mSwitchingFullscreen = false;
        mDisplayFrequency = 0;
        mDeviceValid = false;
        mUseNVPerfHUD = false;
        mWindowedWinStyle = 0;
        mFullscreenWinStyle = 0;
		mDesiredWidth = 0;
		mDesiredHeight = 0;
    }

    D3D9RenderWindow::~D3D9RenderWindow()
    {
        destroy();
    }

    void D3D9RenderWindow::create(const String& name, unsigned int width, unsigned int height,
        bool fullScreen, const NameValuePairList *miscParams)
    {
        HINSTANCE hInst = mInstance;
    
        WNDPROC windowProc = DefWindowProc;
        HWND externalHandle = 0;
        mFSAAType = D3DMULTISAMPLE_NONE;
        mFSAAQuality = 0;
        mFSAA = 0;
        mVSync = false;
        mVSyncInterval = 1;
        String title = name;
        unsigned int colourDepth = 32;
        int left = INT_MAX; // Defaults to screen center
        int top = INT_MAX;  // Defaults to screen center
        bool depthBuffer = true;
        String border = "";
        bool outerSize = false;
        mUseNVPerfHUD = false;
        size_t fsaaSamples = 0;
        String fsaaHint;
        bool enableDoubleClick = false;

        D3D9RenderSystem* rsys = static_cast<D3D9RenderSystem*>(Root::getSingleton().getRenderSystem());
        int monitorIndex = rsys->getAdapterNumber();  // default to whatever was set in "Rendering Device" config option

        if(miscParams)
        {
            // Get variable-length params
            NameValuePairList::const_iterator opt;
            // left (x)
            opt = miscParams->find("left");
            if(opt != miscParams->end())
                left = StringConverter::parseInt(opt->second);
            // top (y)
            opt = miscParams->find("top");
            if(opt != miscParams->end())
                top = StringConverter::parseInt(opt->second);
            // Window title
            opt = miscParams->find("title");
            if(opt != miscParams->end())
                title = opt->second;
            opt = miscParams->find("windowProc");
            if (opt != miscParams->end())
                windowProc = reinterpret_cast<WNDPROC>(StringConverter::parseSizeT(opt->second));
            // externalWindowHandle     -> externalHandle
            opt = miscParams->find("externalWindowHandle");
            if (opt == miscParams->end())
                opt = miscParams->find("parentWindowHandle");
            if(opt != miscParams->end())
                externalHandle = (HWND)StringConverter::parseSizeT(opt->second);
            // vsync    [parseBool]
            opt = miscParams->find("vsync");
            if(opt != miscParams->end())
                mVSync = StringConverter::parseBool(opt->second);
            // hidden   [parseBool]
            opt = miscParams->find("hidden");
            if(opt != miscParams->end())
                mHidden = StringConverter::parseBool(opt->second);
            // vsyncInterval    [parseUnsignedInt]
            opt = miscParams->find("vsyncInterval");
            if(opt != miscParams->end())
                mVSyncInterval = StringConverter::parseUnsignedInt(opt->second);
            // displayFrequency
            opt = miscParams->find("displayFrequency");
            if(opt != miscParams->end())
                mDisplayFrequency = StringConverter::parseUnsignedInt(opt->second);
            // colourDepth
            opt = miscParams->find("colourDepth");
            if(opt != miscParams->end())
                colourDepth = StringConverter::parseUnsignedInt(opt->second);
            // depthBuffer [parseBool]
            opt = miscParams->find("depthBuffer");
            if(opt != miscParams->end())
                depthBuffer = StringConverter::parseBool(opt->second);
            // FSAA settings
            opt = miscParams->find("FSAA");
            if(opt != miscParams->end())
            {
                mFSAA = StringConverter::parseUnsignedInt(opt->second);
            }
            opt = miscParams->find("FSAAHint");
            if(opt != miscParams->end())
            {
                mFSAAHint = opt->second;
            }

            // window border style
            opt = miscParams->find("border");
            if(opt != miscParams->end())
                border = opt->second;
            // set outer dimensions?
            opt = miscParams->find("outerDimensions");
            if(opt != miscParams->end())
                outerSize = StringConverter::parseBool(opt->second);
            // NV perf HUD?
            opt = miscParams->find("useNVPerfHUD");
            if(opt != miscParams->end())
                mUseNVPerfHUD = StringConverter::parseBool(opt->second);
            // sRGB?
            opt = miscParams->find("gamma");
            if(opt != miscParams->end())
                mHwGamma = StringConverter::parseBool(opt->second);
            // monitor index
            opt = miscParams->find("monitorIndex");
            if(opt != miscParams->end())
                monitorIndex = StringConverter::parseInt(opt->second);
            opt = miscParams->find("show");
            if(opt != miscParams->end())
                mHidden = !StringConverter::parseBool(opt->second);
            // enable double click messages
            opt = miscParams->find("enableDoubleClick");
            if(opt != miscParams->end())
                enableDoubleClick = StringConverter::parseBool(opt->second);

        }
        mIsFullScreen = fullScreen;

        // Destroy current window if any
        if( mHWnd )
            destroy();

        if (!externalHandle)
        {
            DWORD       dwStyleEx = 0;
            HMONITOR    hMonitor = NULL;        
            MONITORINFO monitorInfo;
            RECT        rc;

            // If we specified which adapter we want to use - find it's monitor.
            if (monitorIndex != -1)
            {
                IDirect3D9* direct3D9 = D3D9RenderSystem::getDirect3D9();

                for (uint i=0; i < direct3D9->GetAdapterCount(); ++i)
                {
                    if (i == monitorIndex)
                    {
                        hMonitor = direct3D9->GetAdapterMonitor(i);
                        break;
                    }
                }               
            }

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
            memset(&monitorInfo, 0, sizeof(MONITORINFO));
            monitorInfo.cbSize = sizeof(MONITORINFO);
            GetMonitorInfo(hMonitor, &monitorInfo);

            // Update window style flags.
            mFullscreenWinStyle = WS_CLIPCHILDREN | WS_POPUP;
            mWindowedWinStyle   = WS_CLIPCHILDREN;

            if (!mHidden)
            {
                mFullscreenWinStyle |= WS_VISIBLE;
                mWindowedWinStyle |= WS_VISIBLE;
            }

            if (border == "none")
                mWindowedWinStyle |= WS_POPUP;
            else if (border == "fixed")
                mWindowedWinStyle |= WS_OVERLAPPED | WS_BORDER | WS_CAPTION |
                WS_SYSMENU | WS_MINIMIZEBOX;
            else
                mWindowedWinStyle |= WS_OVERLAPPEDWINDOW;
                    
            unsigned int winWidth, winHeight;
            winWidth = width;
            winHeight = height;


            // No specified top left -> Center the window in the middle of the monitor
            if (left == INT_MAX || top == INT_MAX)
            {               
                uint32 screenw = monitorInfo.rcWork.right  - monitorInfo.rcWork.left;
                uint32 screenh = monitorInfo.rcWork.bottom - monitorInfo.rcWork.top;

                // clamp window dimensions to screen size
                uint32 outerw = (winWidth < screenw)? winWidth : screenw;
                uint32 outerh = (winHeight < screenh)? winHeight : screenh;

                if (left == INT_MAX)
                    left = monitorInfo.rcWork.left + (screenw - outerw) / 2;
                else if (monitorIndex != -1)
                    left += monitorInfo.rcWork.left;

                if (top == INT_MAX)
                    top = monitorInfo.rcWork.top + (screenh - outerh) / 2;
                else if (monitorIndex != -1)
                    top += monitorInfo.rcWork.top;
            }
            else if (monitorIndex != -1)
            {
                left += monitorInfo.rcWork.left;
                top += monitorInfo.rcWork.top;
            }

            mWidth = mDesiredWidth = width;
            mHeight = mDesiredHeight = height;
            mTop = top;
            mLeft = left;

            if (fullScreen)
            {
                dwStyleEx |= WS_EX_TOPMOST;             
                mTop = monitorInfo.rcMonitor.top;
                mLeft = monitorInfo.rcMonitor.left;     
            }
            else
            {               
                adjustWindow(width, height, &winWidth, &winHeight);

                if (!outerSize)
                {
                    // Calculate window dimensions required
                    // to get the requested client area
                    SetRect(&rc, 0, 0, mWidth, mHeight);
                    AdjustWindowRect(&rc, getWindowStyle(fullScreen), false);
                    mWidth = rc.right - rc.left;
                    mHeight = rc.bottom - rc.top;

                    // Clamp window rect to the nearest display monitor.
                    if (mLeft < monitorInfo.rcWork.left)
                        mLeft = monitorInfo.rcWork.left;        

                    if (mTop < monitorInfo.rcWork.top)                  
                        mTop = monitorInfo.rcWork.top;                  
                
                    if (static_cast<int>(winWidth) > monitorInfo.rcWork.right - mLeft)                  
                        winWidth = monitorInfo.rcWork.right - mLeft;    

                    if (static_cast<int>(winHeight) > monitorInfo.rcWork.bottom - mTop)                 
                        winHeight = monitorInfo.rcWork.bottom - mTop;                                       
                }
            }
            
            UINT classStyle = 0;
            if (enableDoubleClick)
                classStyle |= CS_DBLCLKS;


            // Register the window class
            // NB allow 4 bytes of window data for D3D9RenderWindow pointer
            WNDCLASS wc = { classStyle, windowProc, 0, 0, hInst,
                LoadIcon(0, IDI_APPLICATION), LoadCursor(NULL, IDC_ARROW),
                (HBRUSH)GetStockObject(BLACK_BRUSH), 0, "OgreD3D9Wnd" };
            RegisterClass(&wc);

            // Create our main window
            // Pass pointer to self
            mIsExternal = false;
            mHWnd = CreateWindowEx(dwStyleEx, "OgreD3D9Wnd", title.c_str(), getWindowStyle(fullScreen),
                mLeft, mTop, winWidth, winHeight, 0, 0, hInst, this);
        }
        else
        {
            mHWnd = externalHandle;
            mIsExternal = true;
        }

        RECT rc;
        // top and left represent outer window coordinates
        GetWindowRect(mHWnd, &rc);
        mTop = rc.top;
        mLeft = rc.left;
        // width and height represent interior drawable area
        GetClientRect(mHWnd, &rc);
        mWidth = rc.right;
        mHeight = rc.bottom;

        mName = name;
        mDepthBufferPoolId = depthBuffer ? DepthBuffer::POOL_DEFAULT : DepthBuffer::POOL_NO_DEPTH;
        mDepthBuffer = 0;
        mColourDepth = colourDepth;

        LogManager::getSingleton().stream()
            << "D3D9 : Created D3D9 Rendering Window '"
            << mName << "' : " << mWidth << "x" << mHeight 
            << ", " << mColourDepth << "bpp";
                                    
        mActive = true;
        mClosed = false;
        setHidden(mHidden);
    }

    void D3D9RenderWindow::setFullscreen(bool fullScreen, unsigned int width, unsigned int height)
    {
        if (fullScreen != mIsFullScreen || width != mWidth || height != mHeight)
        {
            if (fullScreen != mIsFullScreen)
                mSwitchingFullscreen = true;

            bool oldFullscreen = mIsFullScreen;
            mIsFullScreen = fullScreen;
            mWidth = mDesiredWidth = width;
            mHeight = mDesiredHeight = height;

            if (fullScreen)
            {
                // Get the nearest monitor to this window.
                HMONITOR hMonitor = MonitorFromWindow(mHWnd, MONITOR_DEFAULTTONEAREST);

                // Get monitor info 
                MONITORINFO monitorInfo;

                memset(&monitorInfo, 0, sizeof(MONITORINFO));
                monitorInfo.cbSize = sizeof(MONITORINFO);
                GetMonitorInfo(hMonitor, &monitorInfo);

                mTop = monitorInfo.rcMonitor.top;
                mLeft = monitorInfo.rcMonitor.left;             
                
                // need different ordering here

                if (oldFullscreen)
                {
                    // was previously fullscreen, just changing the resolution
                    SetWindowPos(mHWnd, HWND_TOPMOST, mLeft, mTop, width, height, SWP_NOACTIVATE);
                }
                else
                {
                    SetWindowPos(mHWnd, HWND_TOPMOST, mLeft, mTop, width, height, SWP_NOACTIVATE);
                    SetWindowLong(mHWnd, GWL_STYLE, getWindowStyle(mIsFullScreen));
                    SetWindowPos(mHWnd, 0, 0,0, 0,0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
                }
            }
            else
            {
                // Calculate window dimensions required
                // to get the requested client area
                unsigned int winWidth, winHeight;
                winWidth = mWidth;
                winHeight = mHeight;
                
                adjustWindow(mWidth, mHeight, &winWidth, &winHeight);

                SetWindowLong(mHWnd, GWL_STYLE, getWindowStyle(mIsFullScreen));
                SetWindowPos(mHWnd, HWND_NOTOPMOST, 0, 0, winWidth, winHeight,
                    SWP_DRAWFRAME | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOACTIVATE);
                // Note that we also set the position in the restoreLostDevice method
                // via _finishSwitchingFullScreen

                // Update the current rect.
                updateWindowRect();
            }
                                
            // Have to release & trigger device reset
            // NB don't use windowMovedOrResized since Win32 doesn't know
            // about the size change yet                
            mDevice->invalidate(this);

            RenderWindow::resize(mWidth, mHeight);
        }
    } 

    void D3D9RenderWindow::adjustWindow(unsigned int clientWidth, unsigned int clientHeight, 
        unsigned int* winWidth, unsigned int* winHeight)
    {
        // NB only call this for non full screen
        RECT rc;
        SetRect(&rc, 0, 0, clientWidth, clientHeight);
        AdjustWindowRect(&rc, getWindowStyle(mIsFullScreen), false);
        *winWidth = rc.right - rc.left;
        *winHeight = rc.bottom - rc.top;
    }

    void D3D9RenderWindow::_finishSwitchingFullscreen()
    {       
        if(mIsFullScreen)
        {
            // Need to reset the region on the window sometimes, when the 
            // windowed mode was constrained by desktop 
            HRGN hRgn = CreateRectRgn(0,0,mWidth, mHeight);
            SetWindowRgn(mHWnd, hRgn, FALSE);
        }
        else
        {           
            // When switching back to windowed mode, need to reset window size 
            // after device has been restored
            // We may have had a resize event which polluted our desired sizes
            if (mWidth != mDesiredWidth ||
                mHeight != mDesiredHeight)
            {
                mWidth = mDesiredWidth;
                mHeight = mDesiredHeight;               
            }
            unsigned int winWidth, winHeight;
            adjustWindow(mWidth, mHeight, &winWidth, &winHeight);

            // deal with centering when switching down to smaller resolution
            HMONITOR hMonitor = MonitorFromWindow(mHWnd, MONITOR_DEFAULTTONEAREST);
            MONITORINFO monitorInfo;
            memset(&monitorInfo, 0, sizeof(MONITORINFO));
            monitorInfo.cbSize = sizeof(MONITORINFO);
            GetMonitorInfo(hMonitor, &monitorInfo);

            ULONG screenw = monitorInfo.rcWork.right  - monitorInfo.rcWork.left;
            ULONG screenh = monitorInfo.rcWork.bottom - monitorInfo.rcWork.top;

            int left = screenw > winWidth ? ((screenw - winWidth) / 2) : 0;
            int top = screenh > winHeight ? ((screenh - winHeight) / 2) : 0;
            SetWindowPos(mHWnd, HWND_NOTOPMOST, left, top, winWidth, winHeight,
                SWP_DRAWFRAME | SWP_FRAMECHANGED | SWP_NOACTIVATE);

            updateWindowRect();
        }
        mSwitchingFullscreen = false;
    }
    
    void D3D9RenderWindow::buildPresentParameters(D3DPRESENT_PARAMETERS* presentParams)
    {       
        // Set up the presentation parameters       
        IDirect3D9* pD3D = D3D9RenderSystem::getDirect3D9();
        D3DDEVTYPE devType = D3DDEVTYPE_HAL;

        if (mDevice != NULL)        
            devType = mDevice->getDeviceType();     
    

        ZeroMemory( presentParams, sizeof(D3DPRESENT_PARAMETERS) );
        presentParams->Windowed                 = !mIsFullScreen;
        
        DWORD version = GetVersion();
        DWORD major = (DWORD) (LOBYTE(LOWORD(version)));
        DWORD minor = (DWORD) (HIBYTE(LOWORD(version)));
        bool isWindows7 = (major > 6) || ((major == 6) && (minor >= 1));

        // http://msdn.microsoft.com/en-us/library/windows/desktop/bb172574%28v=vs.85%29.aspx
        // Multisampling is valid only on a swap chain that is being created or reset with the D3DSWAPEFFECT_DISCARD swap effect.       
        bool useFlipSwap =  D3D9RenderSystem::isDirectX9Ex() && isWindows7 && (isAA() == false);
            
        presentParams->SwapEffect               = useFlipSwap ? D3DSWAPEFFECT_FLIPEX : D3DSWAPEFFECT_DISCARD;
        // triple buffer if VSync is on or if flip swap is used. Otherwise we may get a performance penalty.
        presentParams->BackBufferCount          = mVSync || useFlipSwap ? 2 : 1;
        presentParams->EnableAutoDepthStencil   = (mDepthBufferPoolId != DepthBuffer::POOL_NO_DEPTH);
        presentParams->hDeviceWindow            = mHWnd;
        presentParams->BackBufferWidth          = mWidth;
        presentParams->BackBufferHeight         = mHeight;
        presentParams->FullScreen_RefreshRateInHz = mIsFullScreen ? mDisplayFrequency : 0;
        
        if (presentParams->BackBufferWidth == 0)        
            presentParams->BackBufferWidth = 1;                 

        if (presentParams->BackBufferHeight == 0)   
            presentParams->BackBufferHeight = 1;                    


        if (mVSync)
        {
            // D3D9 only seems to support 2-4 presentation intervals in fullscreen
            if (mIsFullScreen)
            {
                switch(mVSyncInterval)
                {
                case 1:
                default:
                    presentParams->PresentationInterval = D3DPRESENT_INTERVAL_ONE;
                    break;
                case 2:
                    presentParams->PresentationInterval = D3DPRESENT_INTERVAL_TWO;
                    break;
                case 3:
                    presentParams->PresentationInterval = D3DPRESENT_INTERVAL_THREE;
                    break;
                case 4:
                    presentParams->PresentationInterval = D3DPRESENT_INTERVAL_FOUR;
                    break;
                };
                // check that the interval was supported, revert to 1 to be safe otherwise
                D3DCAPS9 caps;
                pD3D->GetDeviceCaps(mDevice->getAdapterNumber(), devType, &caps);
                if (!(caps.PresentationIntervals & presentParams->PresentationInterval))
                    presentParams->PresentationInterval = D3DPRESENT_INTERVAL_ONE;

            }
            else
            {
                presentParams->PresentationInterval = D3DPRESENT_INTERVAL_ONE;
            }

        }
        else
        {
            // NB not using vsync in windowed mode in D3D9 can cause jerking at low 
            // frame rates no matter what buffering modes are used (odd - perhaps a
            // timer issue in D3D9 since GL doesn't suffer from this) 
            // low is < 200fps in this context
            if (!mIsFullScreen)
            {
                LogManager::getSingleton().logWarning(
                    "D3D9: disabling VSync in windowed mode can cause timing issues at lower "
                    "frame rates, turn VSync on if you observe this problem.");
            }
            presentParams->PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
        }

        presentParams->BackBufferFormat     = D3DFMT_R5G6B5;
        if( mColourDepth > 16 )
            presentParams->BackBufferFormat = D3DFMT_X8R8G8B8;

        if (mColourDepth > 16 )
        {
            // Try to create a 32-bit depth, 8-bit stencil
            if( FAILED( pD3D->CheckDeviceFormat(mDevice->getAdapterNumber(),
                devType,  presentParams->BackBufferFormat,  D3DUSAGE_DEPTHSTENCIL, 
                D3DRTYPE_SURFACE, D3DFMT_D24S8 )))
            {
                // Bugger, no 8-bit hardware stencil, just try 32-bit zbuffer 
                if( FAILED( pD3D->CheckDeviceFormat(mDevice->getAdapterNumber(),
                    devType,  presentParams->BackBufferFormat,  D3DUSAGE_DEPTHSTENCIL, 
                    D3DRTYPE_SURFACE, D3DFMT_D32 )))
                {
                    // Jeez, what a naff card. Fall back on 16-bit depth buffering
                    presentParams->AutoDepthStencilFormat = D3DFMT_D16;
                }
                else
                    presentParams->AutoDepthStencilFormat = D3DFMT_D32;
            }
            else
            {
                // Woohoo!
                if( SUCCEEDED( pD3D->CheckDepthStencilMatch( mDevice->getAdapterNumber(), devType,
                    presentParams->BackBufferFormat, presentParams->BackBufferFormat, D3DFMT_D24S8 ) ) )
                {
                    presentParams->AutoDepthStencilFormat = D3DFMT_D24S8; 
                } 
                else 
                    presentParams->AutoDepthStencilFormat = D3DFMT_D24X8; 
            }
        }
        else
            // 16-bit depth, software stencil
            presentParams->AutoDepthStencilFormat   = D3DFMT_D16;


        D3D9RenderSystem* rsys = static_cast<D3D9RenderSystem*>(Root::getSingleton().getRenderSystem());
        
        rsys->determineFSAASettings(mDevice->getD3D9Device(),
            mFSAA, mFSAAHint, presentParams->BackBufferFormat, mIsFullScreen, 
            &mFSAAType, &mFSAAQuality);

        presentParams->MultiSampleType = mFSAAType;
        presentParams->MultiSampleQuality = (mFSAAQuality == 0) ? 0 : mFSAAQuality;

        // Check sRGB
        if (mHwGamma)
        {
            /* hmm, this never succeeds even when device does support??
            if(FAILED(pD3D->CheckDeviceFormat(mDriver->getAdapterNumber(),
                devType, presentParams->BackBufferFormat, D3DUSAGE_QUERY_SRGBWRITE, 
                D3DRTYPE_SURFACE, presentParams->BackBufferFormat )))
            {
                // disable - not supported
                mHwGamma = false;
            }
            */

        }
    }
    
    void D3D9RenderWindow::destroy()
    {
        if (mDevice != NULL)
        {
            mDevice->detachRenderWindow(this);
            mDevice = NULL;
        }
        
        if (mHWnd && !mIsExternal)
        {
            DestroyWindow(mHWnd);
        }

        mHWnd = 0;
        mActive = false;
        mClosed = true;
    }

    bool D3D9RenderWindow::isActive() const
    {
        if (isFullScreen())
            return isVisible();

        return mActive && isVisible();
    }

    bool D3D9RenderWindow::isVisible() const
    {
        HWND currentWindowHandle = mHWnd;
        bool visible;
        while ((visible = (IsIconic(currentWindowHandle) == false)) &&
            (GetWindowLong(currentWindowHandle, GWL_STYLE) & WS_CHILD) != 0)
        {
            currentWindowHandle = GetParent(currentWindowHandle);
        }
        return visible;
    }

    void D3D9RenderWindow::setHidden(bool hidden)
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

    void D3D9RenderWindow::setVSyncEnabled(bool vsync)
    {
        mVSync = vsync;
        if (!mIsExternal)
        {
            // we need to reset the device with new vsync params
            // invalidate the window to trigger this
            mDevice->invalidate(this);
        }
    }

    bool D3D9RenderWindow::isVSyncEnabled() const
    {
        return mVSync;
    }

    void D3D9RenderWindow::setVSyncInterval(unsigned int interval)
    {
        mVSyncInterval = interval;
        if (mVSync)
            setVSyncEnabled(true);
    }

    void D3D9RenderWindow::reposition(int top, int left)
    {
        if (mHWnd && !mIsFullScreen)
        {
            SetWindowPos(mHWnd, 0, top, left, 0, 0,
                SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        }
    }

    void D3D9RenderWindow::resize(unsigned int width, unsigned int height)
    {
        if (!mIsExternal)
        {
            if (mHWnd && !mIsFullScreen)
            {
                unsigned int winWidth, winHeight;
                adjustWindow(width, height, &winWidth, &winHeight);
                SetWindowPos(mHWnd, 0, 0, 0, winWidth, winHeight,
                    SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
            }
        }
        else
            updateWindowRect();
    }

    void D3D9RenderWindow::windowMovedOrResized()
    {
        if (!mHWnd || IsIconic(mHWnd))
            return;
    
        updateWindowRect();
    }

    void D3D9RenderWindow::swapBuffers( )
    {
        if (mDeviceValid)
            mDevice->present(this);     
    }

    void D3D9RenderWindow::getCustomAttribute( const String& name, void* pData )
    {
        // Valid attributes and their equvalent native functions:
        // D3DDEVICE            : getD3DDevice
        // WINDOW               : getWindowHandle

        if( name == "D3DDEVICE" )
        {
            *(IDirect3DDevice9**)pData = getD3D9Device();
        }       
        else if( name == "WINDOW" )
        {
            *(HWND*)pData = getWindowHandle();
        }
        else if( name == "isTexture" )
        {
            *(bool*)pData = false;
        }
        else if( name == "D3DZBUFFER" )
        {
            *(IDirect3DSurface9**)pData = mDevice->getDepthBuffer(this);
        }
        else if( name == "DDBACKBUFFER" )
        {
            *(IDirect3DSurface9**)pData = mDevice->getBackBuffer(this);
        }
        else if( name == "DDFRONTBUFFER" )
        {
            *(IDirect3DSurface9**)pData = mDevice->getBackBuffer(this);
        }
    }

    PixelFormat D3D9RenderWindow::suggestPixelFormat() const
    {
        return mColourDepth == 16 ? PF_R5G6B5 : PF_X8R8G8B8;
    }

    void D3D9RenderWindow::copyContentsToMemory(const Box& src, const PixelBox &dst, FrameBuffer buffer)
    {
        mDevice->copyContentsToMemory(this, src, dst, buffer);
    }
    //-----------------------------------------------------------------------------
    void D3D9RenderWindow::_beginUpdate()
    {       
        // External windows should update per frame
        // since it dosen't get the window resize/move messages.
        if (mIsExternal)
        {       
            updateWindowRect();
        }

        if (mWidth == 0 || mHeight == 0)
        {
            mDeviceValid = false;
            return;
        }

        D3D9RenderSystem::getDeviceManager()->setActiveRenderTargetDevice(mDevice);

        // Check that device can be used for rendering operations.
        mDeviceValid = mDevice->validate(this);
        if (mDeviceValid)
        {
            // Finish window / fullscreen mode switch.
            if (_getSwitchingFullscreen())
            {
                _finishSwitchingFullscreen();       
                // have to re-validate since this may have altered dimensions
                mDeviceValid = mDevice->validate(this);
            }
        }

        RenderWindow::_beginUpdate();
    }
    //---------------------------------------------------------------------
    void D3D9RenderWindow::_updateViewport(Viewport* viewport, bool updateStatistics)
    {
        if (mDeviceValid)
        {
            RenderWindow::_updateViewport(viewport, updateStatistics);
        }
    }
    //---------------------------------------------------------------------
    void D3D9RenderWindow::_endUpdate()
    {
        RenderWindow::_endUpdate();

        D3D9RenderSystem::getDeviceManager()->setActiveRenderTargetDevice(NULL);    

    }
    //-----------------------------------------------------------------------------
    IDirect3DDevice9* D3D9RenderWindow::getD3D9Device()
    {
        return mDevice->getD3D9Device();
    }

    //-----------------------------------------------------------------------------
    IDirect3DSurface9* D3D9RenderWindow::getRenderSurface()
    {
        return mDevice->getBackBuffer(this);
    }

    //-----------------------------------------------------------------------------
    bool D3D9RenderWindow::_getSwitchingFullscreen() const
    {
        return mSwitchingFullscreen;
    }

    //-----------------------------------------------------------------------------
    D3D9Device* D3D9RenderWindow::getDevice()
    {
        return mDevice;
    }

    //-----------------------------------------------------------------------------
    void D3D9RenderWindow::setDevice(D3D9Device* device)
    {
        mDevice = device;
        mDeviceValid = false;
    }

    //-----------------------------------------------------------------------------
    bool D3D9RenderWindow::isDepthBuffered() const
    {
        return (mDepthBufferPoolId != DepthBuffer::POOL_NO_DEPTH);
    }

    //-----------------------------------------------------------------------------
    void D3D9RenderWindow::updateWindowRect()
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

        if (width != mWidth || height != mHeight)
        {
            RenderWindow::resize(rc.right - rc.left, rc.bottom - rc.top);
        }
    }
    //-----------------------------------------------------------------------------
    void D3D9RenderWindow::updateStats( void )
    {
        RenderTarget::updateStats();
        mStats.vBlankMissCount = mDevice->getVBlankMissCount(this);
    }
    //-----------------------------------------------------------------------------
    bool D3D9RenderWindow::isNvPerfHUDEnable() const
    {
        return mUseNVPerfHUD;
    }
    //---------------------------------------------------------------------
    bool D3D9RenderWindow::_validateDevice()
    {
        mDeviceValid = mDevice->validate(this);
        return mDeviceValid;
    }
	//---------------------------------------------------------------------
#if OGRE_NO_QUAD_BUFFER_STEREO == 0
	void D3D9RenderWindow::_validateStereo()
	{
		mStereoEnabled = D3D9StereoDriverBridge::getSingleton().isStereoEnabled(this->getName());
	}
#endif
	//---------------------------------------------------------------------
}
