// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreRoot.h"
#include "OgreStringConverter.h"
#include "OgreViewport.h"

#include "OgreWaylandEGLSupport.h"
#include "OgreWaylandEGLWindow.h"

#include <thread>
#include <chrono>

namespace Ogre
{

WaylandEGLWindow::WaylandEGLWindow(WaylandEGLSupport* glsupport) : EGLWindow(glsupport)
{
    mGLSupport = glsupport;
    mNativeDisplay = nullptr; //glsupport->getNativeDisplay();
    mXdgSurface = nullptr;
    mXdgToplevel = nullptr;
}

WaylandEGLWindow::~WaylandEGLWindow()
{
    //mNativeDisplay = mGLSupport->getNativeDisplay();

    if (mWindow && mIsTopLevel)
    {
        if (!mIsExternal)
            wl_egl_window_destroy(mWindow);
    }
    mWindow = nullptr;
}

void WaylandEGLWindow::getCustomAttribute(const String& name, void* pData)
{
    EGLWindow::getCustomAttribute(name, pData);
    // #TODO add support for wayland-related variables;
    if (name == "WL_DISPLAY")
    {
        *static_cast<NativeDisplayType*>(pData) = mGLSupport->getNativeDisplay();
        return;
    }
    else if (name == "WL_WINDOW")
    {
        *static_cast<NativeWindowType*>(pData) = mWindow;
        return;
    }
    // ADD XDG stuff also
}

void WaylandEGLWindow::initNativeCreatedWindow(const NameValuePairList* miscParams)
{
    //mNativeDisplay = mGLSupport->getNativeDisplay();

    if (miscParams)
    {
        NameValuePairList::const_iterator opt;
        NameValuePairList::const_iterator end = miscParams->end();

        if ((opt = miscParams->find("externalDisplay")) != end)
        {
          StringVector tokens = StringUtil::split(opt->second, " :");
          LogManager::getSingleton().logWarning("GOT DISPLAY POINTER:" + tokens[0]);

          auto tmp = (wl_display*)StringConverter::parseSizeT(tokens[0]);
          LogManager::getSingleton().stream() << "EXTERNAL DISPLAY POINTER:" << tmp;
          LogManager::getSingleton().stream() << "EXISTING DISPLAY POINTER:" << mNativeDisplay;
          mNativeDisplay = tmp;
          mGLSupport->setNativeDisplay(mNativeDisplay);

        }
        if ((opt = miscParams->find("parentWindowHandle")) != end ||
            (opt = miscParams->find("externalWindowHandle")) != end)
        {
            StringVector tokens = StringUtil::split(opt->second, " :");

            LogManager::getSingleton().logWarning("GOT WINDOW HANDLE:" + tokens[0]);

            mWindow = (wl_egl_window*)StringConverter::parseSizeT(tokens[0]);
            mIsExternal = true;
            mIsTopLevel = true;
        }
        if ((opt = miscParams->find("externalSurface")) != end)
        {
            StringVector tokens = StringUtil::split(opt->second, " :");
            LogManager::getSingleton().logWarning("GOT SURFACE HANDLE:" + tokens[0]);

            mGLSupport->mWlSurface = (wl_surface*)StringConverter::parseSizeT(tokens[0]);
        }
        if ((opt = miscParams->find("externalXdgSurface")) != end)
        {
            StringVector tokens = StringUtil::split(opt->second, " :");
            LogManager::getSingleton().logWarning("GOT XDG SURFACE HANDLE:" + tokens[0]);

            mXdgSurface = (xdg_surface*)StringConverter::parseSizeT(tokens[0]);
        }
        if ((opt = miscParams->find("externalXdgToplevel")) != end)
        {
            StringVector tokens = StringUtil::split(opt->second, " :");
            LogManager::getSingleton().logWarning("GOT XDG TOPLEVEL HANDLE:" + tokens[0]);

            mXdgToplevel = (xdg_toplevel*)StringConverter::parseSizeT(tokens[0]);
        }
    }
    mNativeDisplay = mGLSupport->getNativeDisplay();
}

void WaylandEGLWindow::createNativeWindow(uint& width, uint& height, String& title)
{
    mEglDisplay = mGLSupport->getGLDisplay();

    if (!mGLSupport->mWlSurface)
    {
        LogManager::getSingleton().logWarning("NO WAYLAND SURFACE, CREATING IT");
        mGLSupport->mWlSurface = wl_compositor_create_surface(mGLSupport->mWlCompositor);
    }
    else
    {
        LogManager::getSingleton().logWarning("WAYLAND SURFACE EXISTS");
    }

    if (mGLSupport->mWlSurface == nullptr)
    {
        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Couldn't create Wayland surface");
    }

    mGLSupport->mWlRegion = wl_compositor_create_region(mGLSupport->mWlCompositor);
    wl_region_add(mGLSupport->mWlRegion, 0, 0, width, height);
    wl_surface_set_opaque_region(mGLSupport->mWlSurface, mGLSupport->mWlRegion);

    if (!mWindow)
    {
        LogManager::getSingleton().logWarning("NO WAYLAND WINDOW, CREATING IT");
        mWindow = wl_egl_window_create(mGLSupport->mWlSurface, width, height);
    }
    else
    {
        LogManager::getSingleton().logWarning("WAYLAND WINDOW EXISTS");
    }

    if (mWindow == EGL_NO_SURFACE)
    {
        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Could not create EGL window");
    }

    if (mIsFullScreen)
    {
        switchFullScreen(true);
    }

    wl_surface_commit(mGLSupport->mWlSurface);
    wl_display_dispatch_pending(mNativeDisplay);
    wl_display_flush(mNativeDisplay);
}

void WaylandEGLWindow::setFullscreen(bool fullscreen, uint width, uint height)
{

    if (mXdgToplevel)
    {
        if (fullscreen)
            xdg_toplevel_set_fullscreen(mXdgToplevel, nullptr);
        else
            xdg_toplevel_unset_fullscreen(mXdgToplevel);

        wl_surface_damage(mGLSupport->mWlSurface, 0, 0, mWidth, mHeight);
        wl_surface_commit(mGLSupport->mWlSurface);
        wl_display_dispatch_pending(mNativeDisplay);
        wl_display_flush(mNativeDisplay);
    }
}

void WaylandEGLWindow::resize(uint width, uint height)
{

    if (mClosed)
    {
        return;
    }

    if (mWidth == width && mHeight == height)
    {
        return;
    }

    if (width != 0 && height != 0)
    {

        if (mWindow)
        {
            wl_egl_window_resize(mWindow, width, height, 0, 0);
            mWidth = width;
            mHeight = height;

            for (auto& it : mViewportList)
                it.second->_updateDimensions();

            wl_surface_damage(mGLSupport->mWlSurface, 0, 0, mWidth, mHeight);
            wl_surface_commit(mGLSupport->mWlSurface);
            wl_display_dispatch_pending(mNativeDisplay);
            wl_display_flush(mNativeDisplay);
        }
    }
}

void WaylandEGLWindow::windowMovedOrResized()
{
    if (mClosed || !mWindow)
        return;

    int width, height;
    wl_egl_window_get_attached_size(mWindow, &width, &height);
    resize(width, height);
}

void WaylandEGLWindow::switchFullScreen(bool fullscreen)
{
    if (mXdgToplevel)
    {
        if (fullscreen)
            xdg_toplevel_set_fullscreen(mXdgToplevel, nullptr);
        else
            xdg_toplevel_unset_fullscreen(mXdgToplevel);

        wl_surface_damage(mGLSupport->mWlSurface, 0, 0, mWidth, mHeight);
        wl_surface_commit(mGLSupport->mWlSurface);
        wl_display_dispatch_pending(mNativeDisplay);
        wl_display_flush(mNativeDisplay);
    }
}

void WaylandEGLWindow::create(const String& name, uint width, uint height, bool fullScreen,
                              const NameValuePairList* miscParams)
{
    String title = name;
    int samples = 0;
    short frequency = 0;
    bool vsync = false;
    ::EGLContext eglContext = NULL;

    unsigned int vsyncInterval = 1;

    //mNativeDisplay = mGLSupport->getNativeDisplay();
    mIsFullScreen = fullScreen;

    if (miscParams)
    {
        NameValuePairList::const_iterator opt;
        NameValuePairList::const_iterator end = miscParams->end();

        if ((opt = miscParams->find("currentGLContext")) != end && StringConverter::parseBool(opt->second))
        {
            eglContext = eglGetCurrentContext();
            EGL_CHECK_ERROR
            if (!eglContext)
            {
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                            "currentGLContext was specified with no current GL context", "EGLWindow::create");
            }

            mEglDisplay = eglGetCurrentDisplay();
            EGL_CHECK_ERROR
        }

        if ((opt = miscParams->find("FSAA")) != end)
        {
            samples = StringConverter::parseUnsignedInt(opt->second);
        }

        if ((opt = miscParams->find("displayFrequency")) != end)
        {
            frequency = (short)StringConverter::parseInt(opt->second);
        }

        if ((opt = miscParams->find("vsync")) != end)
        {
            vsync = StringConverter::parseBool(opt->second);
        }

        if ((opt = miscParams->find("vsyncInterval")) != end)
            vsyncInterval = StringConverter::parseUnsignedInt(opt->second);

        if ((opt = miscParams->find("gamma")) != end)
        {
            mHwGamma = StringConverter::parseBool(opt->second);
        }

        if ((opt = miscParams->find("title")) != end)
        {
            title = opt->second;
        }

        if ((opt = miscParams->find("externalGLControl")) != end)
        {
            mIsExternalGLControl = StringConverter::parseBool(opt->second);
        }
    }

    initNativeCreatedWindow(miscParams);

    mGLSupport->doInit();


    if (!mEglConfig)
    {
        int minAttribs[] = {EGL_RED_SIZE,
                            5,
                            EGL_GREEN_SIZE,
                            6,
                            EGL_BLUE_SIZE,
                            5,
                            EGL_DEPTH_SIZE,
                            16,
                            EGL_SAMPLES,
                            0,
                            EGL_ALPHA_SIZE,
                            EGL_DONT_CARE,
                            EGL_STENCIL_SIZE,
                            EGL_DONT_CARE,
                            EGL_SAMPLE_BUFFERS,
                            0,
                            EGL_NONE};

        int maxAttribs[] = {EGL_RED_SIZE,       8,  EGL_GREEN_SIZE, 8,       EGL_BLUE_SIZE,    8,
                            EGL_DEPTH_SIZE,     24, EGL_ALPHA_SIZE, 8,       EGL_STENCIL_SIZE, 8,
                            EGL_SAMPLE_BUFFERS, 1,  EGL_SAMPLES,    samples, EGL_NONE};

        mEglConfig = mGLSupport->selectGLConfig(minAttribs, maxAttribs);
    }

    if (!mIsTopLevel)
    {
        mIsFullScreen = false;
    }

    if (mIsFullScreen)
    {
        mGLSupport->switchMode(width, height, frequency);
    }

    createNativeWindow(width, height, title);

    LogManager::getSingleton().logWarning("Created native window");
    // https://github.com/libsdl-org/SDL/issues/5386
    // https://github.com/libsdl-org/SDL/pull/8789
    // See ApplicationContextSDL
    mEglSurface = createSurfaceFromWindow(mGLSupport->getGLDisplay(), mWindow);

    LogManager::getSingleton().logWarning("Created egl surface");

    mContext = createEGLContext(eglContext);

    // apply vsync settings. call setVSyncInterval first to avoid
    // setting vsync more than once.
    setVSyncInterval(vsyncInterval);
    setVSyncEnabled(vsync);

    mName = name;
    mWidth = width;
    mHeight = height;

    finaliseWindow();
    wl_surface_commit(mGLSupport->mWlSurface);
}

} // namespace Ogre
