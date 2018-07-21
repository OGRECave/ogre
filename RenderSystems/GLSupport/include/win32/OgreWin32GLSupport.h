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
#ifndef __OgreWin32GLSupport_H__
#define __OgreWin32GLSupport_H__

#include "OgreWin32Prerequisites.h"
#include "OgreGLNativeSupport.h"

namespace Ogre
{
    
    class _OgreGLExport Win32GLSupport : public GLNativeSupport
    {
    public:
        Win32GLSupport(int profile);
        
        ConfigOptionMap getConfigOptions();
        
        /// @copydoc RenderSystem::_createRenderWindow
        virtual RenderWindow* newWindow(const String &name, unsigned int width, unsigned int height, 
            bool fullScreen, const NameValuePairList *miscParams = 0);

        
        /**
        * Start anything special
        */
        void start();
        /**
        * Stop anything special
        */
        void stop();

        /**
        * Get the address of a function
        */
        void* getProcAddress(const char* procname) const;

        /**
         * Initialise extensions
         */
        virtual void initialiseExtensions();
        

        bool selectPixelFormat(HDC hdc, int colourDepth, int multisample, bool hwGamma);

        virtual unsigned int getDisplayMonitorCount() const;
    private:
        Win32Window *mInitialWindow;
        bool mHasPixelFormatARB;
        bool mHasMultisample;
        bool mHasHardwareGamma;
        PFNWGLCHOOSEPIXELFORMATARBPROC mWglChoosePixelFormat;

        struct DisplayMonitorInfo
        {
            HMONITOR        hMonitor;
            MONITORINFOEX   monitorInfoEx;
        };

        typedef std::vector<DisplayMonitorInfo> DisplayMonitorInfoList;
        typedef DisplayMonitorInfoList::iterator DisplayMonitorInfoIterator;

        DisplayMonitorInfoList mMonitorInfoList;

        void initialiseWGL();
        static LRESULT CALLBACK dummyWndProc(HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp);
        static BOOL CALLBACK sCreateMonitorsInfoEnumProc(HMONITOR hMonitor, HDC hdcMonitor, 
            LPRECT lprcMonitor, LPARAM dwData);
    };

}

#endif
