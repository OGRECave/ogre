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
#include "OgreRoot.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"

#include <algorithm>

#include "OgreWin32GLSupport.h"
#include "OgreWin32Window.h"
#include "OgreGLUtil.h"

#include <GL/gl.h>
#include <GL/wglext.h>

static PFNWGLCREATECONTEXTATTRIBSARBPROC _wglCreateContextAttribsARB = 0;

static int glMajorMax = 0;
static int glMinorMax = 0;

namespace Ogre {
    GLNativeSupport* getGLSupport(int profile)
    {
        return new Win32GLSupport(profile);
    }

    Win32GLSupport::Win32GLSupport(int profile)
        : GLNativeSupport(profile)
        , mInitialWindow(0)
        , mHasPixelFormatARB(false)
        , mHasMultisample(false)
        , mHasHardwareGamma(false)
        , mWglChoosePixelFormat(0)
    {
        // immediately test WGL_ARB_pixel_format and FSAA support
        // so we can set configuration options appropriately
        initialiseWGL();
#if OGRE_NO_QUAD_BUFFER_STEREO == 0
		mStereoMode = SMT_NONE;
#endif
    } 

    ConfigOptionMap Win32GLSupport::getConfigOptions()
    {
        ConfigOptionMap mOptions;
        
        //TODO: EnumDisplayDevices http://msdn.microsoft.com/library/en-us/gdi/devcons_2303.asp
        /*vector<string> DisplayDevices;
        DISPLAY_DEVICE DisplayDevice;
        DisplayDevice.cb = sizeof(DISPLAY_DEVICE);
        DWORD i=0;
        while (EnumDisplayDevices(NULL, i++, &DisplayDevice, 0) {
            DisplayDevices.push_back(DisplayDevice.DeviceName);
        }*/

        // Video mode possibilities
        DEVMODE DevMode;
        DevMode.dmSize = sizeof(DEVMODE);
        for (DWORD i = 0; EnumDisplaySettings(NULL, i, &DevMode); ++i)
        {
            if (DevMode.dmBitsPerPel < 16 || DevMode.dmPelsHeight < 480)
                continue;
            mVideoModes.push_back({DevMode.dmPelsWidth, DevMode.dmPelsHeight,
                                   short(DevMode.dmDisplayFrequency), uint8(DevMode.dmBitsPerPel)});
        }

        ConfigOption optColourDepth;
        optColourDepth.name = "Colour Depth";
        optColourDepth.immutable = false;

        mOptions[optColourDepth.name] = optColourDepth;

        return mOptions;
    }

    BOOL CALLBACK Win32GLSupport::sCreateMonitorsInfoEnumProc(
        HMONITOR hMonitor,  // handle to display monitor
        HDC hdcMonitor,     // handle to monitor DC
        LPRECT lprcMonitor, // monitor intersection rectangle
        LPARAM dwData       // data
        )
    {
        DisplayMonitorInfoList* pArrMonitorsInfo = (DisplayMonitorInfoList*)dwData;

        // Get monitor info
        DisplayMonitorInfo displayMonitorInfo;

        displayMonitorInfo.hMonitor = hMonitor;

        memset(&displayMonitorInfo.monitorInfoEx, 0, sizeof(MONITORINFOEX));
        displayMonitorInfo.monitorInfoEx.cbSize = sizeof(MONITORINFOEX);
        GetMonitorInfo(hMonitor, &displayMonitorInfo.monitorInfoEx);

        pArrMonitorsInfo->push_back(displayMonitorInfo);

        return TRUE;
    }


    RenderWindow* Win32GLSupport::newWindow(const String &name, unsigned int width, 
        unsigned int height, bool fullScreen, const NameValuePairList *miscParams)
    {       
        Win32Window* window = OGRE_NEW Win32Window(*this);
        NameValuePairList newParams;
    
        if (miscParams != NULL)
        {   
            newParams = *miscParams;
            miscParams = &newParams;

            NameValuePairList::const_iterator monitorIndexIt = miscParams->find("monitorIndex");            
            HMONITOR hMonitor = NULL;
            int monitorIndex = -1;
        
            // If monitor index found, try to assign the monitor handle based on it.
            if (monitorIndexIt != miscParams->end())
            {               
                if (mMonitorInfoList.empty())       
                    EnumDisplayMonitors(NULL, NULL, sCreateMonitorsInfoEnumProc, (LPARAM)&mMonitorInfoList);            

                monitorIndex = StringConverter::parseInt(monitorIndexIt->second);
                if (monitorIndex < (int)mMonitorInfoList.size())
                {                       
                    hMonitor = mMonitorInfoList[monitorIndex].hMonitor;                 
                }
            }
            // If we didn't specified the monitor index, or if it didn't find it
            if (hMonitor == NULL)
            {
                POINT windowAnchorPoint;
        
                NameValuePairList::const_iterator opt;
                int left = -1;
                int top  = -1;

                if ((opt = newParams.find("left")) != newParams.end())
                    left = StringConverter::parseInt(opt->second);

                if ((opt = newParams.find("top")) != newParams.end())
                    top = StringConverter::parseInt(opt->second);

                // Fill in anchor point.
                windowAnchorPoint.x = left;
                windowAnchorPoint.y = top;


                // Get the nearest monitor to this window.
                hMonitor = MonitorFromPoint(windowAnchorPoint, MONITOR_DEFAULTTOPRIMARY);               
            }

            newParams["monitorHandle"] = StringConverter::toString((size_t)hMonitor);                                                               
        }

        window->create(name, width, height, fullScreen, miscParams);

        if(!mInitialWindow) {
            mInitialWindow = window;
            initialiseExtensions();
        }

        return window;
    }

    void Win32GLSupport::start()
    {
        LogManager::getSingleton().logMessage("*** Starting Win32GL Subsystem ***");
    }

    void Win32GLSupport::stop()
    {
        LogManager::getSingleton().logMessage("*** Stopping Win32GL Subsystem ***");
        mInitialWindow = 0; // Since there is no removeWindow, although there should be...
    }

    void Win32GLSupport::initialiseExtensions()
    {
        assert(mInitialWindow);

        // Check for W32 specific extensions probe function
        PFNWGLGETEXTENSIONSSTRINGARBPROC _wglGetExtensionsStringARB = 
            (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
        if(!_wglGetExtensionsStringARB)
            return;
        const char *wgl_extensions = _wglGetExtensionsStringARB(mInitialWindow->getHDC());
        LogManager::getSingleton().stream()
            << "Supported WGL extensions: " << wgl_extensions;
        // Parse them, and add them to the main list
        StringStream ext;
        String instr;
        ext << wgl_extensions;
        while(ext >> instr)
        {
            extensionList.insert(instr);
        }
    }


    void* Win32GLSupport::getProcAddress(const char* procname) const
    {
        void* res = (void*)wglGetProcAddress(procname);

        if (!res) {
            static HMODULE libgl = LoadLibraryA("opengl32.dll");
            res = (void*)GetProcAddress(libgl, procname);
        }

        return res;
    }
    void Win32GLSupport::initialiseWGL()
    {
        // wglGetProcAddress does not work without an active OpenGL context,
        // but we need wglChoosePixelFormatARB's address before we can
        // create our main window.  Thank you very much, Microsoft!
        //
        // The solution is to create a dummy OpenGL window first, and then
        // test for WGL_ARB_pixel_format support.  If it is not supported,
        // we make sure to never call the ARB pixel format functions.
        //
        // If is is supported, we call the pixel format functions at least once
        // to initialise them (pointers are stored by glprocs.h).  We can also
        // take this opportunity to enumerate the valid FSAA modes.
        
        LPCSTR dummyText = "OgreWglDummy";

		HINSTANCE hinst = NULL;
		#ifdef __MINGW32__
			#ifdef OGRE_STATIC_LIB
        		hinst = GetModuleHandle( NULL );
			#else
				#if OGRE_DEBUG_MODE == 1
					hinst = GetModuleHandle("RenderSystem_GL3Plus_d.dll");
				#else
					hinst = GetModuleHandle("RenderSystem_GL3Plus.dll");
				#endif
			#endif
		#else
			static const TCHAR staticVar;
			GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, &staticVar, &hinst);
		#endif
        
        WNDCLASS dummyClass;
        memset(&dummyClass, 0, sizeof(WNDCLASS));
        dummyClass.style = CS_OWNDC;
        dummyClass.hInstance = hinst;
        dummyClass.lpfnWndProc = dummyWndProc;
        dummyClass.lpszClassName = dummyText;
        RegisterClass(&dummyClass);

        HWND hwnd = CreateWindow(dummyText, dummyText,
            WS_POPUP | WS_CLIPCHILDREN,
            0, 0, 32, 32, 0, 0, hinst, 0);

        // if a simple CreateWindow fails, then boy are we in trouble...
        if (hwnd == NULL)
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "CreateWindow() failed", "Win32GLSupport::initializeWGL");


        // no chance of failure and no need to release thanks to CS_OWNDC
        HDC hdc = GetDC(hwnd); 

        // assign a simple OpenGL pixel format that everyone supports
        PIXELFORMATDESCRIPTOR pfd;
        memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
        pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
        pfd.nVersion = 1;
        pfd.cColorBits = 16;
        pfd.cDepthBits = 15;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        
        // if these fail, wglCreateContext will also quietly fail
        int format;
        if ((format = ChoosePixelFormat(hdc, &pfd)) != 0)
            SetPixelFormat(hdc, format, &pfd);

        HGLRC hrc = wglCreateContext(hdc);
        if (hrc)
        {
            HGLRC oldrc = wglGetCurrentContext();
            HDC oldhdc = wglGetCurrentDC();
            // if wglMakeCurrent fails, wglGetProcAddress will return null
            wglMakeCurrent(hdc, hrc);

            // the default context is created with maximum version
            // use GL3 query type, as it is the only consumer
            glGetIntegerv(0x821B, &glMajorMax);
            glGetIntegerv(0x821C, &glMinorMax);

            _wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)
                wglGetProcAddress("wglCreateContextAttribsARB");

            PFNWGLGETEXTENSIONSSTRINGARBPROC _wglGetExtensionsStringARB =
                (PFNWGLGETEXTENSIONSSTRINGARBPROC)
                wglGetProcAddress("wglGetExtensionsStringARB");
            
            // check for pixel format and multisampling support
            if (_wglGetExtensionsStringARB)
            {
                std::istringstream wglexts(_wglGetExtensionsStringARB(hdc));
                std::string ext;
                while (wglexts >> ext)
                {
                    if (ext == "WGL_ARB_pixel_format")
                        mHasPixelFormatARB = true;
                    else if (ext == "WGL_ARB_multisample")
                        mHasMultisample = true;
                    else if (ext == "WGL_EXT_framebuffer_sRGB")
                        mHasHardwareGamma = true;
                }
            }

            if (mHasPixelFormatARB && mHasMultisample)
            {
                // enumerate all formats w/ multisampling
                static const int iattr[] = {
                    WGL_DRAW_TO_WINDOW_ARB, true,
                    WGL_SUPPORT_OPENGL_ARB, true,
                    WGL_DOUBLE_BUFFER_ARB, true,
                    WGL_SAMPLE_BUFFERS_ARB, true,
                    WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
                    /* We are no matter about the colour, depth and stencil buffers here
                    WGL_COLOR_BITS_ARB, 24,
                    WGL_ALPHA_BITS_ARB, 8,
                    WGL_DEPTH_BITS_ARB, 24,
                    WGL_STENCIL_BITS_ARB, 8,
                    */
                    WGL_SAMPLES_ARB, 2,
                    0
                };
                int formats[256] = {0};
                unsigned int count = 0;
                // cheating here.  wglChoosePixelFormatARB procc address needed later on
                // when a valid GL context does not exist and glew is not initialized yet.
                PFNWGLGETPIXELFORMATATTRIBIVARBPROC wglGetPixelFormatAttribiv = (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)wglGetProcAddress("wglGetPixelFormatAttribivARB");

                mWglChoosePixelFormat = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");


                if (mWglChoosePixelFormat(hdc, iattr, 0, 256, formats, &count))
                {
                    // determine what multisampling levels are offered
                    int query = WGL_SAMPLES_ARB, samples;
                    for (unsigned int i = 0; i < count; ++i)
                    {
                        if (wglGetPixelFormatAttribiv(hdc, formats[i], 0, 1, &query, &samples))
                        {
                            mFSAALevels.push_back(samples);
                        }
                    }
                    mFSAALevels.push_back(0);
                }
            }
            
            wglMakeCurrent(oldhdc, oldrc);
            wglDeleteContext(hrc);
        }

        // clean up our dummy window and class
        DestroyWindow(hwnd);
        UnregisterClass(dummyText, hinst);
    }

    LRESULT Win32GLSupport::dummyWndProc(HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp)
    {
        return DefWindowProc(hwnd, umsg, wp, lp);
    }

    bool Win32GLSupport::selectPixelFormat(HDC hdc, int colourDepth, int multisample, bool hwGamma)
    {
        PIXELFORMATDESCRIPTOR pfd;
        memset(&pfd, 0, sizeof(pfd));
        pfd.nSize = sizeof(pfd);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = (colourDepth > 16)? 24 : colourDepth;
        pfd.cAlphaBits = (colourDepth > 16)? 8 : 0;
        pfd.cDepthBits = 24;
        pfd.cStencilBits = 8;

#if OGRE_NO_QUAD_BUFFER_STEREO == 0
		if (SMT_FRAME_SEQUENTIAL == mStereoMode)
			pfd.dwFlags |= PFD_STEREO;
#endif

        int format = 0;

        int useHwGamma = hwGamma;

        if (multisample && (!mHasMultisample || !mHasPixelFormatARB))
            return false;

        if (hwGamma && !mHasHardwareGamma)
            return false;
        
        if ((multisample || hwGamma) && mWglChoosePixelFormat)
        {

            // Use WGL to test extended caps (multisample, sRGB)
            std::vector<int> attribList;
            attribList.push_back(WGL_DRAW_TO_WINDOW_ARB); attribList.push_back(true);
            attribList.push_back(WGL_SUPPORT_OPENGL_ARB); attribList.push_back(true);
            attribList.push_back(WGL_DOUBLE_BUFFER_ARB); attribList.push_back(true);
            attribList.push_back(WGL_SAMPLE_BUFFERS_ARB); attribList.push_back(true);
            attribList.push_back(WGL_ACCELERATION_ARB); attribList.push_back(WGL_FULL_ACCELERATION_ARB);
            attribList.push_back(WGL_COLOR_BITS_ARB); attribList.push_back(pfd.cColorBits);
            attribList.push_back(WGL_ALPHA_BITS_ARB); attribList.push_back(pfd.cAlphaBits);
            attribList.push_back(WGL_DEPTH_BITS_ARB); attribList.push_back(24);
            attribList.push_back(WGL_STENCIL_BITS_ARB); attribList.push_back(8);
            attribList.push_back(WGL_SAMPLES_ARB); attribList.push_back(multisample);
			
#if OGRE_NO_QUAD_BUFFER_STEREO == 0
			if (SMT_FRAME_SEQUENTIAL == mStereoMode)
				attribList.push_back(WGL_STEREO_ARB); attribList.push_back(true);
#endif

            if (useHwGamma && mHasHardwareGamma)
            {
                attribList.push_back(WGL_FRAMEBUFFER_SRGB_CAPABLE_EXT); attribList.push_back(true);
            }
            // terminator
            attribList.push_back(0);


            UINT nformats;
            // ChoosePixelFormatARB proc address was obtained when setting up a dummy GL context in initialiseWGL()
            // since glew hasn't been initialized yet, we have to cheat and use the previously obtained address
            if (!mWglChoosePixelFormat(hdc, &(attribList[0]), NULL, 1, &format, &nformats) || nformats <= 0)
                return false;
        }
        else
        {
            format = ChoosePixelFormat(hdc, &pfd);
        }


        return (format && SetPixelFormat(hdc, format, &pfd));
    }

    HGLRC Win32GLSupport::createNewContext(HDC hdc, HGLRC shareList)
    {
        HGLRC glrc = NULL;

        int profile;
        int majorVersion;
        int minorVersion = 0;

        switch(mContextProfile) {
        case CONTEXT_COMPATIBILITY:
            profile = WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
            majorVersion = 1;
            break;
        case CONTEXT_ES:
            profile = WGL_CONTEXT_ES2_PROFILE_BIT_EXT;
            majorVersion = 2;
            break;
        default:
            profile = WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
            majorVersion = std::max(glMajorMax, 3);
            minorVersion = std::max(glMinorMax, 3); // 3.1 would be sufficient per spec, but we need 3.3 anyway..
            break;
        }

        int context_attribs[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, majorVersion,
            WGL_CONTEXT_MINOR_VERSION_ARB, minorVersion,
            WGL_CONTEXT_PROFILE_MASK_ARB, profile,
            0
        };

        glrc = _wglCreateContextAttribsARB ? _wglCreateContextAttribsARB(hdc, shareList, context_attribs) : wglCreateContext(hdc);

        if (!glrc)
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "wglCreateContext failed: " + translateWGLError());

        if(!_wglCreateContextAttribsARB && shareList && shareList != glrc)
        {
            // Share lists with old context
            if (!wglShareLists(shareList, glrc))
            {
                wglDeleteContext(glrc);
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "wglShareLists() failed: "+ translateWGLError());
            }
        }

        return glrc;
    }

    unsigned int Win32GLSupport::getDisplayMonitorCount() const
    {
        if (mMonitorInfoList.empty())       
            EnumDisplayMonitors(NULL, NULL, sCreateMonitorsInfoEnumProc, (LPARAM)&mMonitorInfoList);

        return (unsigned int)mMonitorInfoList.size();
    }

    String translateWGLError()
    {
        int winError = GetLastError();
        char* errDesc;
        int i;

        errDesc = new char[255];
        // Try windows errors first
        i = FormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            winError,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
            (LPTSTR) errDesc,
            255,
            NULL
            );

        return String(errDesc);
    }

}
