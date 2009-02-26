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


#include <X11/extensions/Xrandr.h>
#include <X11/Xutil.h>

namespace Ogre {
    template<class C> void removeDuplicates(C& c)
    {
        std::sort(c.begin(), c.end());
        typename C::iterator p = std::unique(c.begin(), c.end());
        c.erase(p, c.end());
    }


    GtkEGLSupport::GtkEGLSupport()
    {
        mGLDisplay = getGLDisplay();
		mNativeDisplay = getNativeDisplayType();

		int dummy;

        if (XQueryExtension(mNativeDisplay, "RANDR", &dummy, &dummy, &dummy))
        {
            XRRScreenConfiguration *screenConfig;

            mRandr = true;
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
        }
        else
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

	EGLWindow* GtkEGLSupport::createEGLWindow( EGLSupport * support )
	{
		return new GtkEGLWindow(support);
	}

	GLESPBuffer* GtkEGLSupport::createPBuffer( PixelComponentType format, size_t width, size_t height )
	{
		return new GtkEGLPBuffer(this, format, width, height);
	}

    EGLSupport::~EGLSupport()
    {
        if (!mNativeDisplay)
        {
            XCloseDisplay(mNativeDisplay);
        }

        if (!mGLDisplay)
        {
            eglTerminate(mGLDisplay);
        }
	}




	NativeDisplayType GtkEGLSupport::getNativeDisplayType()
	{
		return (NativeDisplayType) getXDisplay();
	}

 
    Display* GtkEGLSupport::getXDisplay(void)
    {
        if (!mNativeDisplay)
        {
            mNativeDisplay = XOpenDisplay(0);

            if (!mNativeDisplay)
            {
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                            "Couldn`t open X display",
                            "EGLSupport::getXDisplay");
            }

            mAtomDeleteWindow = XInternAtom(mNativeDisplay, "WM_DELETE_WINDOW", True);
            mAtomFullScreen = XInternAtom(mNativeDisplay, "_NET_WM_STATE_FULLSCREEN", True);
            mAtomState = XInternAtom(mNativeDisplay, "_NET_WM_STATE", True);
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
                XRRGetScreenInfo(mNativeDisplay, DefaultRootWindow(mNativeDisplay));
            if (screenConfig)
            {
                Rotation currentRotation;

                XRRConfigCurrentConfiguration (screenConfig, &currentRotation);
                XRRSetScreenConfigAndRate(mNativeDisplay, screenConfig, DefaultRootWindow(mNativeDisplay),
                                          newSize, currentRotation, newMode->second, CurrentTime);
                XRRFreeScreenConfigInfo(screenConfig);
                mCurrentMode = *newMode;
                LogManager::getSingleton().logMessage("Entered video mode " + StringConverter::toString(mCurrentMode.first.first) + "x" + StringConverter::toString(mCurrentMode.first.second) + " @ " + StringConverter::toString(mCurrentMode.second) + "MHz");
            }
        }
	}

	void GtkEGLSupport::switchMode( uint& width, uint& height, short& frequency )
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
		// todo

	}


    GLESPBuffer *GtkEGLSupport::createPBuffer(PixelComponentType format,
                                           size_t width, size_t height)
    {
        return new GtkEGLPBuffer(this, format, width, height);
    }

    XVisualInfo *GtkEGLSupport::getVisualFromFBConfig(::EGLConfig glConfig)
    {
        XVisualInfo *vi, tmp;
        int vid, n;
        ::EGLDisplay glDisplay;
        Display *xDisplay;

        glDisplay = getGLDisplay();
        xDisplay = getXDisplay();

        if (eglGetConfigAttrib(glDisplay, glConfig, EGL_NATIVE_VISUAL_ID, &vid) == EGL_FALSE)
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        "Fail to get VISUAL_ID from glConfig",
                        __FUNCTION__);
            return 0;
        }

        if (vid == 0)
        {
            const int screen_number = DefaultScreen(xDisplay);
            Visual *v = DefaultVisual(xDisplay, screen_number);
            vid = XVisualIDFromVisual(v);
        }

        tmp.visualid = vid;
        vi = 0;
        vi = XGetVisualInfo(xDisplay,
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

}
