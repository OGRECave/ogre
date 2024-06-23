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

void WaylandEGLWindow::initNativeCreatedWindow(const NameValuePairList* miscParams)
{
    //mNativeDisplay = mGLSupport->getNativeDisplay();

    if (miscParams)
    {
        NameValuePairList::const_iterator opt;
        NameValuePairList::const_iterator end = miscParams->end();

        if ((opt = miscParams->find("externalWlDisplay")) != end)
        {
          StringVector tokens = StringUtil::split(opt->second, " :");

          auto tmp = (wl_display*)StringConverter::parseSizeT(tokens[0]);
          mNativeDisplay = tmp;
          mGLSupport->setNativeDisplay(mNativeDisplay);

        }
        if ((opt = miscParams->find("externalSurface")) != end)
        {
            StringVector tokens = StringUtil::split(opt->second, " :");

            mGLSupport->mWlSurface = (wl_surface*)StringConverter::parseSizeT(tokens[0]);
        }
    }
    mNativeDisplay = mGLSupport->getNativeDisplay();
}

void WaylandEGLWindow::createNativeWindow(uint& width, uint& height, String& title)
{
    mEglDisplay = mGLSupport->getGLDisplay();

    mGLSupport->mWlRegion = wl_compositor_create_region(mGLSupport->mWlCompositor);
    wl_region_add(mGLSupport->mWlRegion, 0, 0, width, height);
    wl_surface_set_opaque_region(mGLSupport->mWlSurface, mGLSupport->mWlRegion);

    if (!mWindow)
    {
        mWindow = wl_egl_window_create(mGLSupport->mWlSurface, width, height);
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

void WaylandEGLWindow::create(const String& name, uint width, uint height, bool fullScreen,
                              const NameValuePairList* miscParams)
{
    String title = name;
    int samples = 0;
    short frequency = 0;
    int maxBufferSize(32), minBufferSize(16), maxDepthSize(16), maxStencilSize(0);
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


        if((opt = miscParams->find("maxColourBufferSize")) != end)
        {
          maxBufferSize = Ogre::StringConverter::parseInt(opt->second);
        }

        if((opt = miscParams->find("maxDepthBufferSize")) != end)
        {
          maxDepthSize = Ogre::StringConverter::parseInt(opt->second);
        }

        if((opt = miscParams->find("maxStencilBufferSize")) != end)
        {
          maxStencilSize = Ogre::StringConverter::parseInt(opt->second);
        }

        if((opt = miscParams->find("minColourBufferSize")) != end)
        {
          minBufferSize = Ogre::StringConverter::parseInt(opt->second);
          if (minBufferSize > maxBufferSize) minBufferSize = maxBufferSize;
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
      int MSAAminAttribs[] = {
        //EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_BUFFER_SIZE, minBufferSize,
        EGL_DEPTH_SIZE, 16,
        EGL_SAMPLE_BUFFERS, 1,
        EGL_SAMPLES, samples,
        EGL_NONE
      };
      int MSAAmaxAttribs[] = {
        //EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_BUFFER_SIZE, maxBufferSize,
        EGL_DEPTH_SIZE, maxDepthSize,
        EGL_STENCIL_SIZE, maxStencilSize,
        EGL_SAMPLE_BUFFERS, 1,
        EGL_SAMPLES, samples,
        EGL_NONE
      };

      mEglConfig = mGLSupport->selectGLConfig(MSAAminAttribs, MSAAmaxAttribs);
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
    EGLDisplay disp = mGLSupport->getGLDisplay();
    LogManager::getSingleton().logWarning("Got EGLDisplay");
    // Qt problem:
    // By default, mWindow is WId, can get QWindow* using QWindow::fromWinId(id), but there are limitations on using this
    // How to get wl_egl_window from QWindow?
    // This function call will fail for Qt with wayland
    mEglSurface = createSurfaceFromWindow(disp, mWindow);

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
