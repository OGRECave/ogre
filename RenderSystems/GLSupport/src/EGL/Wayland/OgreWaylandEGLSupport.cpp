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

const struct wl_registry_listener WaylandEGLSupport::mRegistryListener = {
    .global = WaylandEGLSupport::globalRegistryHandler, .global_remove = WaylandEGLSupport::globalRegistryRemover};

void WaylandEGLSupport::globalRegistryHandler(void* data, struct wl_registry* registry, uint32_t id,
                                              const char* interface, uint32_t version)
{
    LogManager::getSingleton().logMessage("Got a registry event for " + std::string(interface) + " " +
                                          std::to_string(id)); //, LogMessageLevel::LML_TRIVIAL);

    auto ptr = static_cast<WaylandEGLSupport*>(data);

    if (std::string(interface) == "wl_compositor")
    {
        ptr->mWlCompositor = static_cast<wl_compositor*>(wl_registry_bind(registry, id, &wl_compositor_interface, 4));
    }
}

void WaylandEGLSupport::globalRegistryRemover(void* data, struct wl_registry* registry, uint32_t id)
{
    // auto ptr = static_cast<WaylandEGLSupport*>(data);
    LogManager::getSingleton().logMessage("Got a registry losing event for " + std::to_string(id));
}

WaylandEGLSupport::WaylandEGLSupport(int profile) : EGLSupport(profile)
{
    mWlCompositor = nullptr;
    mWlSurface = nullptr;
    mWlRegion = nullptr;
    mIsExternalDisplay = false;

    // doInit();
}

void WaylandEGLSupport::doInit()
{
    // A connection that is NOT shared to enable independent event processing:
    // mNativeDisplay = getNativeDisplay();

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
    else
    {
        // TODO
        // getWaylandVideoModes(mNativeDisplay, mCurrentMode, mVideoModes);
    }

    if (mVideoModes.empty()) // none of the above worked
    {
        // TODO fix this
        mCurrentMode.width = 640;  // DisplayWidth(mNativeDisplay, DefaultScreen(mNativeDisplay));
        mCurrentMode.height = 480; // DisplayHeight(mNativeDisplay, DefaultScreen(mNativeDisplay));
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

    window->create(name, width, height, fullScreen, miscParams);

    return window;
}

// WaylandEGLSupport::getGLDisplay sets up the native variable
// then calls EGLSupport::getGLDisplay
EGLDisplay WaylandEGLSupport::getGLDisplay()
{
    if (!mNativeDisplay)
    {
        //
        return nullptr;
    }
    if (!mGLDisplay)
    {
        if (!mNativeDisplay)
            mNativeDisplay = getNativeDisplay();
        struct wl_registry* registry = wl_display_get_registry(mNativeDisplay);
        //
        wl_registry_add_listener(registry, &WaylandEGLSupport::mRegistryListener, this);
        wl_display_dispatch(mNativeDisplay);
        wl_display_roundtrip(mNativeDisplay);

        if (mWlCompositor == nullptr)
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Couldn't find Wayland compositor or shell display");
            return nullptr;
        }

        return EGLSupport::getGLDisplay();
    }
    return mGLDisplay;
}

} // namespace Ogre
