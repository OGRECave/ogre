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
#include "OgreLogManager.h"
#include "OgreStringConverter.h"

#include "OgreX11EGLSupport.h"
#include "OgreX11EGLWindow.h"
#include "OgreX11.h"

namespace Ogre {
    X11EGLWindow::X11EGLWindow(X11EGLSupport *glsupport)
        : EGLWindow(glsupport) 
        //, mParentWindow(glsupport)   todo
    {
        mGLSupport = glsupport;
        mNativeDisplay = glsupport->getNativeDisplay();
    }

    X11EGLWindow::~X11EGLWindow()
    {
        mNativeDisplay = mGLSupport->getNativeDisplay();

        if (mWindow && mIsTopLevel)
        {
            destroyXWindow(mNativeDisplay, mWindow);
        }

        mWindow = 0;
    }

    void X11EGLWindow::getCustomAttribute( const String& name, void* pData )
    {
        EGLWindow::getCustomAttribute(name, pData);
        if (name == "ATOM")
        {
            *static_cast< ::Atom* >(pData) = mGLSupport->mAtomDeleteWindow;
            return;
        } 
        else if (name == "XDISPLAY")
        {
            *static_cast<NativeDisplayType*>(pData) = mGLSupport->getNativeDisplay();
            return;
        }
        else if (name == "XWINDOW")
        {
            *static_cast<NativeWindowType*>(pData) = mWindow;
            return;
        }

    }

    void X11EGLWindow::getLeftAndTopFromNativeWindow( int & left, int & top, uint width, uint height )
    {
        left = DisplayWidth((Display*)mNativeDisplay, DefaultScreen(mNativeDisplay))/2 - width/2;
        top  = DisplayHeight((Display*)mNativeDisplay, DefaultScreen(mNativeDisplay))/2 - height/2;
    }

    void X11EGLWindow::initNativeCreatedWindow(const NameValuePairList *miscParams)
    {
        mNativeDisplay = mGLSupport->getNativeDisplay();
        mParentWindow = DefaultRootWindow((Display*)mNativeDisplay);

        if (miscParams)
        {
            NameValuePairList::const_iterator opt;
            NameValuePairList::const_iterator end = miscParams->end();

            OgreAssert(miscParams->find("externalWlDisplay") == end, "Recompile with OGRE_USE_WAYLAND=ON");

            if ((opt = miscParams->find("parentWindowHandle")) != end ||
                (opt = miscParams->find("externalWindowHandle")) != end)
            {
                StringVector tokens = StringUtil::split(opt->second, " :");

                if (tokens.size() >= 3)
                {
                    // deprecated display:screen:xid:visualinfo format
                    mParentWindow = (Window)StringConverter::parseSizeT(tokens[2]);
                }
                else
                {
                    // xid format
                    mParentWindow = (Window)StringConverter::parseSizeT(tokens[0]);
                }
            }
        }

        validateParentWindow(mNativeDisplay, mParentWindow);

        mIsTopLevel = (!mIsExternal && mParentWindow == DefaultRootWindow((Display*)mNativeDisplay));

    }

    void X11EGLWindow::createNativeWindow( int &left, int &top, uint &width, uint &height, String &title )
    {
        mEglDisplay = mGLSupport->getGLDisplay();//todo
        XVisualInfo *visualInfo = mGLSupport->getVisualFromFBConfig(mEglConfig);

        // Create window on server
        mWindow = createXWindow(mNativeDisplay, mParentWindow, visualInfo, left, top, width, height,
                                mGLSupport->mAtomFullScreen, mIsFullScreen);

        if (mIsTopLevel)
        {
            finaliseTopLevel(mNativeDisplay, mWindow, left, top, width, height, title, mGLSupport->mAtomDeleteWindow);
        }

        mEglSurface = createSurfaceFromWindow(mGLSupport->getGLDisplay(), mWindow);

        XMapWindow((Display*)mNativeDisplay, (Window)mWindow);

        if (mIsFullScreen)
        {
            switchFullScreen(true);
        }

        XFlush((Display*)mNativeDisplay);
    }

    void X11EGLWindow::setFullscreen( bool fullscreen, uint width, uint height )
    {
        if (mIsFullScreen != fullscreen && mGLSupport->mAtomFullScreen == None)
        {
            // Without WM support it is best to give up.
            LogManager::getSingleton().logMessage("EGLWindow::switchFullScreen: Your WM has no fullscreen support");
            return;
        }
        EGLWindow::setFullscreen(fullscreen, width, height);
    }

    void X11EGLWindow::reposition( int left, int top )
    {
        if (mClosed || ! mIsTopLevel)
        {
            return;
        }

        XMoveWindow((Display*)mGLSupport->getNativeDisplay(), (Window)mWindow, left, top);
    }

    void X11EGLWindow::resize(uint width, uint height)
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
            if (!mIsTopLevel)
            {
                XResizeWindow(mGLSupport->getNativeDisplay(), mWindow, width, height);
                XFlush(mGLSupport->getNativeDisplay());
            }

            RenderWindow::resize(width, height);
        }
    }

    void X11EGLWindow::windowMovedOrResized()
    {
        if (mClosed || !mWindow)
            return;

        uint width, height;
        queryRect(mNativeDisplay, mWindow, mLeft, mTop, width, height, mIsTopLevel && !mIsFullScreen);
        resize(width, height);
    }
    void X11EGLWindow::switchFullScreen(bool fullscreen)
    { 
        if (mGLSupport->mAtomFullScreen != None)
        {
            XClientMessageEvent xMessage;

            xMessage.type = ClientMessage;
            xMessage.serial = 0;
            xMessage.send_event = True;
            xMessage.window = (Window)mWindow;
            xMessage.message_type = mGLSupport->mAtomState;
            xMessage.format = 32;
            xMessage.data.l[0] = (fullscreen ? 1 : 0);
            xMessage.data.l[1] = mGLSupport->mAtomFullScreen;
            xMessage.data.l[2] = 0;

            XSendEvent((Display*)mNativeDisplay, DefaultRootWindow((Display*)mNativeDisplay), False,
                SubstructureRedirectMask | SubstructureNotifyMask,
                (XEvent*)&xMessage);

            mIsFullScreen = fullscreen;
        }
    }


    //Moved EGLWindow::create to native source because it has native calls in it
    void X11EGLWindow::create(const String& name, uint width, uint height,
                           bool fullScreen, const NameValuePairList *miscParams)
    {
        String title = name;
        int samples = 0;
        short frequency = 0;
        bool vsync = false;
        ::EGLContext eglContext = NULL;
        int left = 0;
        int top  = 0;

        unsigned int vsyncInterval = 1;

        mNativeDisplay = mGLSupport->getNativeDisplay();
        getLeftAndTopFromNativeWindow(left, top, width, height);

        mIsFullScreen = fullScreen;

        if (miscParams)
        {
            NameValuePairList::const_iterator opt;
            NameValuePairList::const_iterator end = miscParams->end();

            if ((opt = miscParams->find("currentGLContext")) != end &&
                StringConverter::parseBool(opt->second))
            {
                eglContext = eglGetCurrentContext();
                EGL_CHECK_ERROR
                if (!eglContext)
                {
                    OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                                "currentGLContext was specified with no current GL context",
                                "EGLWindow::create");
                }

                mEglDisplay = eglGetCurrentDisplay();
                EGL_CHECK_ERROR
            }

            // Note: Some platforms support AA inside ordinary windows
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

            if((opt = miscParams->find("vsyncInterval")) != end)
                vsyncInterval = StringConverter::parseUnsignedInt(opt->second);

            if ((opt = miscParams->find("gamma")) != end)
            {
                mHwGamma = StringConverter::parseBool(opt->second);
            }

            if ((opt = miscParams->find("left")) != end)
            {
                left = StringConverter::parseInt(opt->second);
            }

            if ((opt = miscParams->find("top")) != end)
            {
                top = StringConverter::parseInt(opt->second);
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

        if (!mEglConfig)
        {
            int minAttribs[] = {
                EGL_RED_SIZE,       5,
                EGL_GREEN_SIZE,     6,
                EGL_BLUE_SIZE,      5,
                EGL_DEPTH_SIZE,     16,
                EGL_SAMPLES,        0,
                EGL_ALPHA_SIZE,     EGL_DONT_CARE,
                EGL_STENCIL_SIZE,   EGL_DONT_CARE,
                EGL_SAMPLE_BUFFERS,  0,
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
        }

        if (!mIsTopLevel)
        {
            mIsFullScreen = false;
            left = top = 0;
        }

        if (mIsFullScreen)
        {
            mGLSupport->switchMode (width, height, frequency);
        }

        createNativeWindow(left, top, width, height, title);

        mContext = createEGLContext(eglContext);

        // apply vsync settings. call setVSyncInterval first to avoid
        // setting vsync more than once.
        setVSyncInterval(vsyncInterval);
        setVSyncEnabled(vsync);

        mName = name;
        mWidth = width;
        mHeight = height;
        mLeft = left;
        mTop = top;

        finaliseWindow();
    }

}

