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
-----------------------------------------------------------------------------
*/

#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreRoot.h"

#include "OgreGLESPrerequisites.h"
#include "OgreGLESRenderSystem.h"

#include "OgreGtkEGLSupport.h"
#include "OgreGtkEGLWindow.h"
#include "OgreGtkEGLRenderTexture.h"
#include "OgreGtkEGLContext.h"


#if (OGRE_PLATFORM != OGRE_PLATFORM_LINUX)
	void XStringListToTextProperty(char ** prop, int num, XTextProperty * textProp){};
	Window DefaultRootWindow(Display* nativeDisplayType){return Window();};
	bool XQueryExtension(Display* nativeDisplayType, char * name, int * dummy0, int * dummy2, int * dummy3){return 0;}
	XRRScreenConfiguration * XRRGetScreenInfo(Display* nativeDisplayType, Window window ){return 0;};
	int XRRConfigCurrentConfiguration(XRRScreenConfiguration * config, Rotation * rotation){return 0;};
	XRRScreenSize * XRRConfigSizes(XRRScreenConfiguration * config, int * nSizes){return 0;};
	int XRRConfigCurrentRate(XRRScreenConfiguration * config){return 0;};
	short * XRRConfigRates(XRRScreenConfiguration * config, int sizeID, int * nRates){return 0;};
	void XRRFreeScreenConfigInfo(XRRScreenConfiguration * config){}
	int DefaultScreen(NativeDisplayType nativeDisplayType){return 0;};
	int DisplayWidth(Display* nativeDisplayType, int screen){return 0;};
	int DisplayHeight(Display* nativeDisplayType, int screen){return 0;};
	Display* XOpenDisplay(int num){return NULL;};
	void XCloseDisplay(Display* nativeDisplayType){};
	Atom XInternAtom(Display* nativeDisplayType, char * name, GtkBool isTrue) {return Atom();};
	char * DisplayString(NativeDisplayType nativeDisplayType){return 0;};
	const char * XDisplayName(char * name){return 0;};
	Visual * DefaultVisual(Display* nativeDisplayType,  int screen){return 0;};
	int XVisualIDFromVisual(Visual *v){return 0;};
	void XRRSetScreenConfigAndRate(Display* nativeDisplayType, XRRScreenConfiguration * config, Window window, int size, Rotation rotation, int mode, int currentTime ){};
	XVisualInfo * XGetVisualInfo(Display* nativeDisplayType,  int mask, XVisualInfo * info, int * n){return 0;};
	typedef int (*XErrorHandler)(Display *, XErrorEvent*);
	XErrorHandler XSetErrorHandler(XErrorHandler xErrorHandler){return 0;};
	void XDestroyWindow(Display* nativeDisplayType, Window nativeWindowType){};
	bool XGetWindowAttributes(Display* nativeDisplayType, Window nativeWindowType, XWindowAttributes * xWindowAttributes){return 0;};
	int XCreateColormap(Display* nativeDisplayType, Window nativeWindowType, int visual, int allocNone){return 0;};
	Window XCreateWindow(Display* nativeDisplayType, Window nativeWindowType, int left, int top, int width, int height, int dummy1, int depth, int inputOutput, int visual, int mask, XSetWindowAttributes * xSetWindowAttributes){return Window();};
	void XFree(void *data){};
	XWMHints * XAllocWMHints(){return 0;};
	XSizeHints * XAllocSizeHints(){return 0;};
	void XSetWMProperties(Display* nativeDisplayType, Window nativeWindowType,XTextProperty * titleprop, char * dummy1, char * dummy2, int num, XSizeHints *sizeHints, XWMHints *wmHints, char * dummy3){};
	void XSetWMProtocols(Display* nativeDisplayType, Window nativeWindowType, Atom * atom, int num){};
	void XMapWindow(Display* nativeDisplayType, Window nativeWindowType){};
	void XFlush(Display* nativeDisplayType){};
	void XMoveWindow(Display* nativeDisplayType, Window nativeWindowType, int left, int top){};
	void XResizeWindow(Display* nativeDisplayType, Window nativeWindowType, int left, int top){};
	void XQueryTree(Display* nativeDisplayType, Window nativeWindowType, Window * root, Window *parent, Window **children, unsigned int * nChildren){};
	void XSendEvent(Display* nativeDisplayType, Window nativeWindowType, int dummy1, int mask, XEvent* xevent){};
#endif

namespace Ogre {

    GtkEGLSupport::GtkEGLSupport()
    {
	mNativeDisplay = getNativeDisplay();
        mGLDisplay = getGLDisplay();

		int dummy;

        if (XQueryExtension((Display*)mNativeDisplay, "RANDR", &dummy, &dummy, &dummy))
        {
            XRRScreenConfiguration *screenConfig;

            mRandr = true;
            screenConfig = XRRGetScreenInfo((Display*)mNativeDisplay, DefaultRootWindow((Display*)mNativeDisplay));

            if (screenConfig)
            {
                XRRScreenSize *screenSizes;
                int nSizes = 0;
                Rotation currentRotation;
                int currentSizeID = XRRConfigCurrentConfiguration(screenConfig, &currentRotation);

                screenSizes = XRRConfigSizes(screenConfig, &nSizes);
                mCurrentMode.first.first = screenSizes[currentSizeID].width;
                mCurrentMode.first.second = screenSizes[currentSizeID].height;
                mCurrentMode.second = XRRConfigCurrentRate(screenConfig);
                mOriginalMode = mCurrentMode;

                for (int sizeID = 0; sizeID < nSizes; sizeID++)
                {
                    short *rates;
                    int nRates = 0;

                    rates = XRRConfigRates(screenConfig, sizeID, &nRates);
                    for (int rate = 0; rate < nRates; rate++)
                    {
                        VideoMode mode;

                        mode.first.first = screenSizes[sizeID].width;
                        mode.first.second = screenSizes[sizeID].height;
                        mode.second = rates[rate];

                        mVideoModes.push_back(mode);
                    }
                }
                XRRFreeScreenConfigInfo(screenConfig);
            }
        }
        else
        {
            mCurrentMode.first.first = DisplayWidth((Display*)mNativeDisplay, DefaultScreen(mNativeDisplay));
            mCurrentMode.first.second = DisplayHeight((Display*)mNativeDisplay, DefaultScreen(mNativeDisplay));
            mCurrentMode.second = 0;
            mOriginalMode = mCurrentMode;
            mVideoModes.push_back(mCurrentMode);
        }

		EGLConfig *glConfigs;
        int config, nConfigs = 0;

        glConfigs = chooseGLConfig(NULL, &nConfigs);

        for (config = 0; config < nConfigs; config++)
        {
            int caveat, samples;

            getGLConfigAttrib(glConfigs[config], EGL_CONFIG_CAVEAT, &caveat);

            if (caveat != EGL_SLOW_CONFIG)
            {
                getGLConfigAttrib(glConfigs[config], EGL_SAMPLES, &samples);
                mSampleLevels.push_back(StringConverter::toString(samples));
            }
        }

        free(glConfigs);

        removeDuplicates(mSampleLevels);
    }


	//Removed this because it was easier to call new GtkEGLWindow directly
	//to get the native version
//	EGLWindow* GtkEGLSupport::createEGLWindow( EGLSupport * support )
//	{
//		return new GtkEGLWindow((GtkEGLSupport*)support);
//	}

	GLESPBuffer* GtkEGLSupport::createPBuffer( PixelComponentType format, size_t width, size_t height )
	{
		return new GtkEGLPBuffer(this, format, width, height);
	}

    GtkEGLSupport::~GtkEGLSupport()
    {
        if (mNativeDisplay)
        {
            XCloseDisplay((Display*)mNativeDisplay);
        }

        if (mGLDisplay)
        {
            eglTerminate(mGLDisplay);
        }
	}




	NativeDisplayType GtkEGLSupport::getNativeDisplay()
	{
        if (!mNativeDisplay)
        {
	    mNativeDisplay = (NativeDisplayType)XOpenDisplay(0);

            if (!mNativeDisplay)
            {
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                            "Couldn`t open X display",
                            "EGLSupport::getXDisplay");
            }

            mAtomDeleteWindow = XInternAtom((Display*)mNativeDisplay, "WM_DELETE_WINDOW", True);
            mAtomFullScreen = XInternAtom((Display*)mNativeDisplay, "_NET_WM_STATE_FULLSCREEN", True);
            mAtomState = XInternAtom((Display*)mNativeDisplay, "_NET_WM_STATE", True);
        }

        return mNativeDisplay;
    }

    String GtkEGLSupport::getDisplayName(void)
    {
		return String((const char*)XDisplayName(DisplayString(mNativeDisplay)));
	}


    void GtkEGLSupport::switchMode(uint& width, uint& height, short& frequency)
    {
        if (!mRandr)
            return;

        int size = 0;
        int newSize = -1;

        VideoModes::iterator mode;
        VideoModes::iterator end = mVideoModes.end();
        VideoMode *newMode = 0;

        for(mode = mVideoModes.begin(); mode != end; size++)
        {
            if (mode->first.first >= static_cast<int>(width) &&
                mode->first.second >= static_cast<int>(height))
            {
                if (!newMode ||
                    mode->first.first < newMode->first.first ||
                    mode->first.second < newMode->first.second)
                {
                    newSize = size;
                    newMode = &(*mode);
                }
            }

            VideoMode* lastMode = &(*mode);

            while (++mode != end && mode->first == lastMode->first)
            {
                if (lastMode == newMode && mode->second == frequency)
                {
                    newMode = &(*mode);
                }
            }
        }

        if (newMode && *newMode != mCurrentMode)
        {
            XRRScreenConfiguration *screenConfig =
                XRRGetScreenInfo((Display*)mNativeDisplay, DefaultRootWindow((Display*)mNativeDisplay));
            if (screenConfig)
            {
                Rotation currentRotation;

                XRRConfigCurrentConfiguration (screenConfig, &currentRotation);
                XRRSetScreenConfigAndRate((Display*)mNativeDisplay, screenConfig, DefaultRootWindow((Display*)mNativeDisplay),
                                          newSize, currentRotation, newMode->second, CurrentTime);
                XRRFreeScreenConfigInfo(screenConfig);
                mCurrentMode = *newMode;
                LogManager::getSingleton().logMessage("Entered video mode " + StringConverter::toString(mCurrentMode.first.first) + "x" + StringConverter::toString(mCurrentMode.first.second) + " @ " + StringConverter::toString(mCurrentMode.second) + "MHz");
            }
        }
	}



    XVisualInfo *GtkEGLSupport::getVisualFromFBConfig(::EGLConfig glConfig)
    {
        XVisualInfo *vi, tmp;
        int vid, n;
        ::EGLDisplay glDisplay;

        glDisplay = getGLDisplay();
        mNativeDisplay = getNativeDisplay();

        if (eglGetConfigAttrib(glDisplay, glConfig, EGL_NATIVE_VISUAL_ID, &vid) == EGL_FALSE)
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        "Fail to get VISUAL_ID from glConfig",
                        __FUNCTION__);
            return 0;
        }

        if (vid == 0)
        {
            const int screen_number = DefaultScreen(mNativeDisplay);
            Visual *v = DefaultVisual((Display*)mNativeDisplay, screen_number);
            vid = XVisualIDFromVisual(v);
        }

        tmp.visualid = vid;
        vi = 0;
        vi = XGetVisualInfo((Display*)mNativeDisplay,
                            VisualIDMask,
                            &tmp, &n);
        if (vi == 0)
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        "Fail to get X11 VISUAL",
                        __FUNCTION__);
            return 0;
        }

        return vi;
    }

    RenderWindow* GtkEGLSupport::newWindow(const String &name,
                                        unsigned int width, unsigned int height,
                                        bool fullScreen,
                                        const NameValuePairList *miscParams)
    {
//        EGLWindow* window = createEGLWindow(this);
	EGLWindow* window = new GtkEGLWindow(this);
        window->create(name, width, height, fullScreen, miscParams);

        return window;
    }

	//GtkEGLSupport::getGLDisplay sets up the native variable
	//then calls EGLSupport::getGLDisplay
    EGLDisplay GtkEGLSupport::getGLDisplay()
    {
	if (!mGLDisplay)
	{
		if(!mNativeDisplay)
			mNativeDisplay = getNativeDisplay();
		return  EGLSupport::getGLDisplay();
	}
	return mGLDisplay;
    }


}
