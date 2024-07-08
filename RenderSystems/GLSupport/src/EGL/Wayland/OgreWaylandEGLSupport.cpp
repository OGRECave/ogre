// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreRoot.h"
#include "OgreStringConverter.h"

#include "OgreWaylandEGLSupport.h"
#include "OgreWaylandEGLWindow.h"

#include "OgreGLUtil.h"

namespace Ogre
{

GLNativeSupport* getGLSupport(int profile) { return new WaylandEGLSupport(profile); }

WaylandEGLSupport::WaylandEGLSupport(int profile) : EGLSupport(profile)
{
    mIsExternalDisplay = false;
}

void WaylandEGLSupport::doInit()
{
    // A connection that is NOT shared to enable independent event processing:
    mNativeDisplay = getNativeDisplay();

    // A connection that might be shared with the application for GL rendering:
    mGLDisplay = getGLDisplay();

    if (mNativeDisplay == EGL_DEFAULT_DISPLAY)
    {
        // fake video mode
        mCurrentMode.width = 0;
        mCurrentMode.height = 0;
        mCurrentMode.refreshRate = 0;
        mVideoModes.push_back(mCurrentMode);
    }

    mOriginalMode = mCurrentMode;

    EGLConfig* glConfigs;
    int config, nConfigs = 0;

    glConfigs = chooseGLConfig(nullptr, &nConfigs);

    for (config = 0; config < nConfigs; config++)
    {
        int caveat, samples;

        getGLConfigAttrib(glConfigs[config], EGL_CONFIG_CAVEAT, &caveat);

        if (caveat != EGL_SLOW_CONFIG)
        {
            getGLConfigAttrib(glConfigs[config], EGL_SAMPLES, &samples);
            mFSAALevels.push_back(samples);
        }
    }

    free(glConfigs);

    initialiseExtensions();
}

WaylandEGLSupport::~WaylandEGLSupport()
{
    if (mNativeDisplay)
    {
        if (!mIsExternalDisplay)
        {
            wl_display_disconnect(mNativeDisplay);
        }
    }

    if (mGLDisplay)
    {
        eglTerminate(mGLDisplay);
    }
}

NativeDisplayType WaylandEGLSupport::getNativeDisplay()
{
    if (!mNativeDisplay)
    {
        mNativeDisplay = wl_display_connect(nullptr);
        if (mNativeDisplay == nullptr)
        {
            LogManager::getSingleton().logWarning("Couldn't connect to Wayland display");
            return mNativeDisplay;
        }

        if (mNativeDisplay == EGL_DEFAULT_DISPLAY)
        {
            LogManager::getSingleton().logWarning("Couldn't open Wayland display");
            return mNativeDisplay;
        }
    }

    return mNativeDisplay;
}

RenderWindow* WaylandEGLSupport::newWindow(const String& name, unsigned int width, unsigned int height, bool fullScreen,
                                           const NameValuePairList* miscParams)
{
    EGLWindow* window = new WaylandEGLWindow(this);

    if (!mInitialWindow)
    {
        mInitialWindow = window;

        NameValuePairList::const_iterator opt;
        NameValuePairList::const_iterator end = miscParams->end();

        if ((opt = miscParams->find("externalWlDisplay")) != end)
        {
            mNativeDisplay = (wl_display*)StringConverter::parseSizeT(opt->second);
            mIsExternalDisplay = true;
        }

        doInit();
    }

    window->create(name, width, height, fullScreen, miscParams);

    return window;
}

void WaylandEGLSupport::start()
{
    LogManager::getSingleton().logMessage(
        "******************************\n"
        "*** Starting EGL Subsystem ***\n"
        "******************************");
}

// WaylandEGLSupport::getGLDisplay sets up the native variable
// then calls EGLSupport::getGLDisplay
EGLDisplay WaylandEGLSupport::getGLDisplay()
{
    if (!mNativeDisplay)
    {
        mNativeDisplay = getNativeDisplay();
    }
    if (!mGLDisplay)
    {
        return EGLSupport::getGLDisplay();
    }
    return mGLDisplay;
}

} // namespace Ogre
