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
--------------------------------------------------------------------------*/

#include "OgreRoot.h"
#include "OgreException.h"
#include "OgreStringConverter.h"

#include "OgreGLRenderSystemCommon.h"

#include "OgreEGLSupport.h"
#include "OgreEGLWindow.h"
#include "OgreEGLContext.h"

#include <EGL/eglext.h>

namespace Ogre {
    EGLWindow::EGLWindow(EGLSupport *glsupport)
        : GLWindow(), mGLSupport(glsupport),
          mWindow(0),
          mNativeDisplay(EGL_DEFAULT_DISPLAY),
          mEglDisplay(EGL_NO_DISPLAY),
          mEglConfig(0),
          mEglSurface(0)
    {
        mActive = true;//todo
    }

    EGLWindow::~EGLWindow()
    {
        destroy();

        if (mContext)
        {
            delete mContext;
        }

        mContext = 0;
    }

    void EGLWindow::destroy(void)
    {
        if (mClosed)
        {
            return;
        }

        mClosed = true;
        mActive = false;

        if (mIsFullScreen)
        {
            mGLSupport->switchMode();
            switchFullScreen(false);
        }
    }

    void EGLWindow::setFullscreen(bool fullscreen, uint width, uint height)
    {
        short frequency = 0;

        if (mClosed || !mIsTopLevel)
        {
            return;
        }

        if (fullscreen == mIsFullScreen && width == mWidth && height == mHeight)
        {
            return;
        }
        if (fullscreen)
        {
            mGLSupport->switchMode(width, height, frequency);
        }
        else
        {
            mGLSupport->switchMode();
        }

        if (mIsFullScreen != fullscreen)
        {
            switchFullScreen(fullscreen);
        }

        if (!mIsFullScreen)
        {
            resize(width, height);
            reposition(mLeft, mTop);
        }
    }

    void EGLWindow::swapBuffers()
    {
        if (mClosed || mIsExternalGLControl)
        {
            return;
        }

        if (eglSwapBuffers(mEglDisplay, mEglSurface) == EGL_FALSE)
        {
            EGL_CHECK_ERROR
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Fail to SwapBuffers");
        }
    }

    void EGLWindow::getCustomAttribute( const String& name, void* pData )
    {
        if (name == "DISPLAYNAME")
        {
            *static_cast<String*>(pData) = mGLSupport->getDisplayName();
            return;
        }
        else if (name == "DISPLAY")
        {
            *static_cast<EGLDisplay*>(pData) = mEglDisplay;
            return;
        }
        else if (name == "GLCONTEXT")
        {
            *static_cast<GLContext**>(pData) = mContext;
            return;
        } 
        else if (name == "WINDOW")
        {
            *static_cast<NativeWindowType*>(pData) = mWindow;
            return;
        } 
    }

    PixelFormat EGLWindow::suggestPixelFormat() const
    {
        return mGLSupport->getContextProfile() == GLNativeSupport::CONTEXT_ES ? PF_BYTE_RGBA : PF_BYTE_RGB;
    }

    void EGLWindow::create(const String& name, unsigned int width, unsigned int height, bool fullScreen,
                           const NameValuePairList* miscParams)
    {
        int samples = 0;

        if (miscParams)
        {
            NameValuePairList::const_iterator opt;
            if ((opt = miscParams->find("FSAA")) != miscParams->end())
            {
                samples = StringConverter::parseUnsignedInt(opt->second);
            }
        }

        int minAttribs[] = {
            EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
            EGL_BLUE_SIZE, 5,
            EGL_GREEN_SIZE, 6,
            EGL_RED_SIZE, 5,
            EGL_DEPTH_SIZE, 16,
            EGL_NONE
        };

        int maxAttribs[] = {
            EGL_RED_SIZE,       8,
            EGL_GREEN_SIZE,     8,
            EGL_BLUE_SIZE,      8,
            EGL_DEPTH_SIZE,     24,
            EGL_ALPHA_SIZE,     8,
            EGL_STENCIL_SIZE,   8,
            EGL_SAMPLE_BUFFERS, 1,
            EGL_SAMPLES, samples,
            EGL_NONE
        };

        mEglConfig = mGLSupport->selectGLConfig(minAttribs, maxAttribs);
        mEglDisplay = mGLSupport->getGLDisplay();

        int pbufferAttribs[] = {
                EGL_WIDTH, int(width),
                EGL_HEIGHT, int(height),
                EGL_NONE,
        };

        mEglSurface = eglCreatePbufferSurface(mEglDisplay, mEglConfig, pbufferAttribs);
        mContext = createEGLContext(NULL);
        mIsExternalGLControl = true; // dont want swapBuffers
        mName = name;
        mWidth = width;
        mHeight = height;

        finaliseWindow();
    }

    ::EGLSurface EGLWindow::createSurfaceFromWindow(::EGLDisplay display,
                                                    NativeWindowType win)
    {
        ::EGLSurface surface;

#if OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
        int* gamma_attribs = NULL;
#else
        int gamma_attribs[] = {EGL_GL_COLORSPACE_KHR, EGL_GL_COLORSPACE_SRGB_KHR, EGL_NONE};
#endif
        mHwGamma = mHwGamma && mGLSupport->checkExtension("EGL_KHR_gl_colorspace");

        surface = eglCreateWindowSurface(display, mEglConfig, (EGLNativeWindowType)win, mHwGamma ? gamma_attribs : NULL);
        EGL_CHECK_ERROR

        if (surface == EGL_NO_SURFACE)
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        "Fail to create EGLSurface based on NativeWindowType");
        }
        return surface;
    }

    void EGLWindow::setVSyncEnabled(bool vsync) {
        mVSync = vsync;
        // we need to make our context current to set vsync
        // store previous context to restore when finished.
        ::EGLSurface oldRead = eglGetCurrentSurface(EGL_READ);
        EGL_CHECK_ERROR
        ::EGLSurface oldDraw = eglGetCurrentSurface(EGL_DRAW);
        EGL_CHECK_ERROR
        ::EGLContext  oldContext  = eglGetCurrentContext();
        EGL_CHECK_ERROR
        ::EGLDisplay dpy = mGLSupport->getGLDisplay();

        mContext->setCurrent();

        if (! mIsExternalGLControl )
        {
            eglSwapInterval(dpy, vsync ? mVSyncInterval : 0);
            EGL_CHECK_ERROR
        }

        mContext->endCurrent();

        eglMakeCurrent (dpy, oldDraw, oldRead, oldContext);
        EGL_CHECK_ERROR
    }

    void EGLWindow::finaliseWindow()
    {
        // query selected config
        int Rsz, Gsz, Bsz, Asz, fsaa;
        mGLSupport->getGLConfigAttrib(mEglConfig, EGL_RED_SIZE, &Rsz);
        mGLSupport->getGLConfigAttrib(mEglConfig, EGL_BLUE_SIZE, &Gsz);
        mGLSupport->getGLConfigAttrib(mEglConfig, EGL_GREEN_SIZE, &Bsz);
        mGLSupport->getGLConfigAttrib(mEglConfig, EGL_ALPHA_SIZE, &Asz);
        mGLSupport->getGLConfigAttrib(mEglConfig, EGL_SAMPLES, &fsaa);

        LogManager::getSingleton().logMessage(
            StringUtil::format("EGLWindow: colourBufferSize=%d/%d/%d/%d gamma=%d FSAA=%d", Rsz, Bsz, Gsz,
                               Asz, mHwGamma, fsaa));

        mActive = true;
        mVisible = true;
        mFSAA = fsaa;
        mClosed = false;
    }
}
