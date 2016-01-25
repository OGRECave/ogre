/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
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

#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreRoot.h"

#include "OgreX11EGLSupport.h"
#include "OgreX11EGLWindow.h"
#include "OgreX11EGLRenderTexture.h"

#include "OgreGLUtil.h"

#include <X11/extensions/Xrandr.h>

namespace Ogre {
    GLNativeSupport* getGLSupport(int profile)
    {
        return new X11EGLSupport(profile);
    }

    X11EGLSupport::X11EGLSupport(int profile) : EGLSupport(profile)
    {
        // A connection that might be shared with the application for GL rendering:
        mGLDisplay = getGLDisplay();

        // A connection that is NOT shared to enable independent event processing:
        mNativeDisplay = getNativeDisplay();

        int dummy;

        if (XQueryExtension(mNativeDisplay, "RANDR", &dummy, &dummy, &dummy))
        {
            XRRScreenConfiguration *screenConfig;

            screenConfig = XRRGetScreenInfo(mNativeDisplay, DefaultRootWindow(mNativeDisplay));

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
        } else
        {
            mCurrentMode.first.first = DisplayWidth(mNativeDisplay, DefaultScreen(mNativeDisplay));
            mCurrentMode.first.second = DisplayHeight(mNativeDisplay, DefaultScreen(mNativeDisplay));
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

    X11EGLSupport::~X11EGLSupport()
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

    NativeDisplayType X11EGLSupport::getNativeDisplay()
    {
        if (!mNativeDisplay)
        {
        mNativeDisplay = (NativeDisplayType)XOpenDisplay(NULL);

            if (!mNativeDisplay)
            {
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                            "Couldn`t open X display",
                            "X11EGLSupport::getXDisplay");
            }

            mAtomDeleteWindow = XInternAtom((Display*)mNativeDisplay, "WM_DELETE_WINDOW", True);
            mAtomFullScreen = XInternAtom((Display*)mNativeDisplay, "_NET_WM_STATE_FULLSCREEN", True);
            mAtomState = XInternAtom((Display*)mNativeDisplay, "_NET_WM_STATE", True);
        }

        return mNativeDisplay;
    }

    String X11EGLSupport::getDisplayName(void)
    {
        return String((const char*)XDisplayName(DisplayString(mNativeDisplay)));
    }


    void X11EGLSupport::switchMode(uint& width, uint& height, short& frequency)
    {
//        if (!mRandr)
//            return;

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
            XWindowAttributes winAtt;
            newMode->first.first = DisplayWidth(mNativeDisplay, 0);
            newMode->first.second = DisplayHeight(mNativeDisplay, 0);
            newMode->second = 0; // TODO: Hardcoding refresh rate for LCD's
            mCurrentMode = *newMode;
        }
    }

    XVisualInfo *X11EGLSupport::getVisualFromFBConfig(::EGLConfig glConfig)
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
        EGL_CHECK_ERROR

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

    RenderWindow* X11EGLSupport::newWindow(const String &name,
                                        unsigned int width, unsigned int height,
                                        bool fullScreen,
                                        const NameValuePairList *miscParams)
    {
        EGLWindow* window = new X11EGLWindow(this);
        window->create(name, width, height, fullScreen, miscParams);

        return window;
    }

    //X11EGLSupport::getGLDisplay sets up the native variable
    //then calls EGLSupport::getGLDisplay
    EGLDisplay X11EGLSupport::getGLDisplay()
    {
        if (!mGLDisplay)
        {
            if(!mNativeDisplay)
                mNativeDisplay = getNativeDisplay();
            return EGLSupport::getGLDisplay();
        }
        return mGLDisplay;
    }

}

