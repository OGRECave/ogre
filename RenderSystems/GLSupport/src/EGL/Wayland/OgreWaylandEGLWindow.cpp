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

#include <chrono>
#include <thread>

namespace Ogre
{

WaylandEGLWindow::WaylandEGLWindow(WaylandEGLSupport* glsupport) : EGLWindow(glsupport)
{
    mGLSupport = glsupport;
    mWlSurface = nullptr;
    mNativeDisplay = nullptr;
}

WaylandEGLWindow::~WaylandEGLWindow()
{
    if (mWindow && mIsTopLevel)
    {
        if (!mIsExternal)
            wl_egl_window_destroy(mWindow);
    }
    mWindow = nullptr;
}

void WaylandEGLWindow::initNativeCreatedWindow(const NameValuePairList* miscParams)
{
    if (miscParams)
    {
        NameValuePairList::const_iterator opt;
        NameValuePairList::const_iterator end = miscParams->end();

        if ((opt = miscParams->find("externalWlSurface")) != end)
        {
            mWlSurface = (wl_surface*)StringConverter::parseSizeT(opt->second);
        }
    }
    OgreAssert(mWlSurface, "externalWlSurface required");
}

void WaylandEGLWindow::createNativeWindow(uint& width, uint& height)
{
    mEglDisplay = mGLSupport->getGLDisplay();

    if (!mWindow)
    {
        mWindow = wl_egl_window_create(mWlSurface, width, height);
    }

    if (mWindow == EGL_NO_SURFACE)
    {
        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Could not create EGL window");
    }

    if (mIsFullScreen)
    {
        switchFullScreen(true);
    }

    wl_surface_commit(mWlSurface);
    wl_display_dispatch_pending(mNativeDisplay);
    wl_display_flush(mNativeDisplay);
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

            RenderWindow::resize(width, height);

            wl_surface_damage(mWlSurface, 0, 0, mWidth, mHeight);
            wl_surface_commit(mWlSurface);
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

void WaylandEGLWindow::create(const String& name, uint width, uint height, bool fullScreen,
                              const NameValuePairList* miscParams)
{
    int samples = 0;
    short frequency = 0;
    int maxBufferSize(24), minBufferSize(16), maxDepthSize(16), maxStencilSize(0);
    bool vsync = false;
    ::EGLContext eglContext = nullptr;
    unsigned int vsyncInterval = 1;

    mNativeDisplay = mGLSupport->getNativeDisplay();

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

        if ((opt = miscParams->find("maxColourBufferSize")) != end)
        {
            maxBufferSize = Ogre::StringConverter::parseInt(opt->second);
        }

        if ((opt = miscParams->find("maxDepthBufferSize")) != end)
        {
            maxDepthSize = Ogre::StringConverter::parseInt(opt->second);
        }

        if ((opt = miscParams->find("maxStencilBufferSize")) != end)
        {
            maxStencilSize = Ogre::StringConverter::parseInt(opt->second);
        }

        if ((opt = miscParams->find("minColourBufferSize")) != end)
        {
            minBufferSize = Ogre::StringConverter::parseInt(opt->second);
            if (minBufferSize > maxBufferSize)
                minBufferSize = maxBufferSize;
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

        if ((opt = miscParams->find("externalGLControl")) != end)
        {
            mIsExternalGLControl = StringConverter::parseBool(opt->second);
        }

        OgreAssert(miscParams->find("parentWindowHandle") == end && miscParams->find("externalWindowHandle") == end,
                   "Recompile with OGRE_USE_WAYLAND=OFF");
    }

    initNativeCreatedWindow(miscParams);

    if (!mEglConfig)
    {
        int minAttribs[] = {
            EGL_BUFFER_SIZE, minBufferSize,
            EGL_DEPTH_SIZE, 16,
            EGL_SAMPLE_BUFFERS, 0,
            EGL_SAMPLES, samples,
            EGL_NONE
        };
        int maxAttribs[] = {
            EGL_BUFFER_SIZE, maxBufferSize,
            EGL_DEPTH_SIZE, maxDepthSize,
            EGL_STENCIL_SIZE, maxStencilSize,
            EGL_SAMPLE_BUFFERS, 1,
            EGL_SAMPLES, samples,
            EGL_NONE
        };

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

    createNativeWindow(width, height);
    mEglSurface = createSurfaceFromWindow(mGLSupport->getGLDisplay(), mWindow);
    mContext = createEGLContext(eglContext);

    // apply vsync settings. call setVSyncInterval first to avoid
    // setting vsync more than once.
    setVSyncInterval(vsyncInterval);
    setVSyncEnabled(vsync);

    mName = name;
    mWidth = width;
    mHeight = height;

    finaliseWindow();
    wl_surface_commit(mWlSurface);
}

} // namespace Ogre
