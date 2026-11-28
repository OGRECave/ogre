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
}

void WaylandEGLSupport::doInit()
{
    // the window provider supplies the connection; without one EGLSupport picks
    // a device display instead, which is what an offscreen window renders into
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
    // EGL keeps using the Wayland connection the display was created from.
    releaseGLDisplay();
}

NativeDisplayType WaylandEGLSupport::getNativeDisplay()
{
    // the connection belongs to the window provider; a surface created from it
    // cannot be presented on a connection of our own
    return mNativeDisplay;
}

RenderWindow* WaylandEGLSupport::newWindow(const String& name, unsigned int width, unsigned int height, bool fullScreen,
                                           const NameValuePairList* miscParams)
{
    if (!mInitialWindow)
    {
        if (miscParams)
        {
            NameValuePairList::const_iterator opt;
            NameValuePairList::const_iterator end = miscParams->end();

            if ((opt = miscParams->find("externalWlDisplay")) != end)
            {
                mNativeDisplay = (wl_display*)StringConverter::parseSizeT(opt->second);
            }
            else if (miscParams->find("externalWlSurface") != end)
            {
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "externalWlDisplay required with externalWlSurface");
            }
        }

        doInit();
    }

    // Without the window provider's display there is nothing to create a surface
    // from, so render offscreen into a pbuffer like the X11 backend does when
    // DISPLAY is missing.
    bool offscreen = mNativeDisplay == EGL_DEFAULT_DISPLAY;
    EGLWindow* window = offscreen ? new EGLWindow(this) : new WaylandEGLWindow(this);

    if (!mInitialWindow)
        mInitialWindow = window;

    window->create(name, width, height, fullScreen, miscParams);

    return window;
}

EGLDisplay WaylandEGLSupport::getGLDisplay()
{
    // the display is process-wide once created; do not initialize it again
    return mGLDisplay != EGL_NO_DISPLAY ? mGLDisplay : EGLSupport::getGLDisplay();
}

void WaylandEGLSupport::start()
{
    LogManager::getSingleton().logMessage(
        "******************************\n"
        "*** Starting EGL Subsystem ***\n"
        "******************************");
}

} // namespace Ogre
