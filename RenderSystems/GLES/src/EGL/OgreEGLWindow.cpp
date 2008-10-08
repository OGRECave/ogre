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
--------------------------------------------------------------------------*/

#include "OgreRoot.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreWindowEventUtilities.h"

#include "OgreGLESPrerequisites.h"
#include "OgreGLESRenderSystem.h"

#include "OgreEGLSupport.h"
#include "OgreEGLWindow.h"
#include "OgreEGLContext.h"

#include <iostream>
#include <algorithm>
#include <sys/time.h>
#include <climits>

#include <X11/Xlib.h>
#include <X11/keysym.h>

#include <X11/extensions/Xrandr.h>

extern "C"
{
    int safeXErrorHandler(Display *display, XErrorEvent *event)
    {
        // Ignore all XErrorEvents
        return 0;
    }

    int (*oldXErrorHandler)(Display *, XErrorEvent*);
}

namespace Ogre {
    EGLWindow::EGLWindow(EGLSupport *glsupport)
        : mGLSupport(glsupport),
          mContext(0)
    {
        mWindow = 0;

        mIsTopLevel = false;
        mIsFullScreen = false;
        mClosed = false;
        mActive = false;
        mIsExternalGLControl = false;
    }

    EGLWindow::~EGLWindow()
    {
        Display* xDisplay = mGLSupport->getXDisplay();

        destroy();

        // Ignore fatal XErrorEvents from stale handles.
        oldXErrorHandler = XSetErrorHandler(safeXErrorHandler);

        if (mWindow)
        {
            XDestroyWindow(xDisplay, mWindow);
        }

        if (mContext)
        {
            delete mContext;
        }

        XSetErrorHandler(oldXErrorHandler);

        mContext = 0;
        mWindow = 0;
    }

    void EGLWindow::create(const String& name, uint width, uint height,
                           bool fullScreen, const NameValuePairList *miscParams)
    {
        Display *xDisplay = mGLSupport->getXDisplay();
        ::EGLDisplay glDisplay = mGLSupport->getGLDisplay();
        String title = name;
        uint samples = 0;
        int gamma;
        short frequency = 0;
        bool vsync = false;
        ::EGLContext eglContext = 0;
        ::EGLSurface eglDrawable = 0;
        Window externalWindow = 0;
        Window parentWindow = DefaultRootWindow(xDisplay);
        int left = DisplayWidth(xDisplay, DefaultScreen(xDisplay))/2 - width/2;
        int top  = DisplayHeight(xDisplay, DefaultScreen(xDisplay))/2 - height/2;

        mIsFullScreen = fullScreen;

        if (miscParams)
        {
            NameValuePairList::const_iterator opt;
            NameValuePairList::const_iterator end = miscParams->end();

            if ((opt = miscParams->find("currentGLContext")) != end &&
                StringConverter::parseBool(opt->second))
            {
                eglContext = eglGetCurrentContext();
                if (eglContext)
                {
                    OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                                "currentGLContext was specified with no current GL context",
                                "EGLWindow::create");
                }

                eglContext = eglGetCurrentContext();
                eglDrawable = eglGetCurrentSurface(EGL_DRAW);
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

            if ((opt = miscParams->find("gamma")) != end)
            {
                gamma = StringConverter::parseBool(opt->second);
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

            if ((opt = miscParams->find("parentWindowHandle")) != end)
            {
                std::vector<String> tokens = StringUtil::split(opt->second, " :");

                if (tokens.size() == 3)
                {
                    // deprecated display:screen:xid format
                    parentWindow = StringConverter::parseUnsignedLong(tokens[2]);
                }
                else
                {
                    // xid format
                    parentWindow = StringConverter::parseUnsignedLong(tokens[0]);
                }
            }
            else if ((opt = miscParams->find("externalWindowHandle")) != end)
            {
                std::vector<String> tokens = StringUtil::split(opt->second, " :");

                LogManager::getSingleton().logMessage(
                                                      "EGLWindow::create: The externalWindowHandle parameter is deprecated.\n"
                                                      "Use the parentWindowHandle or currentGLContext parameter instead.");

                if (tokens.size() == 3)
                {
                    // Old display:screen:xid format
                    // The old EGL code always created a "parent" window in this case:
                    parentWindow = StringConverter::parseUnsignedLong(tokens[2]);
                }
                else if (tokens.size() == 4)
                {
                    // Old display:screen:xid:visualinfo format
                    externalWindow = StringConverter::parseUnsignedLong(tokens[2]);
                }
                else
                {
                    // xid format
                    externalWindow = StringConverter::parseUnsignedLong(tokens[0]);
                }
            }
        }

        // Ignore fatal XErrorEvents during parameter validation:
        oldXErrorHandler = XSetErrorHandler(safeXErrorHandler);

        // Validate parentWindowHandle
        if (parentWindow != DefaultRootWindow(xDisplay))
        {
            XWindowAttributes windowAttrib;

            if (!XGetWindowAttributes(xDisplay, parentWindow, &windowAttrib) ||
                windowAttrib.root != DefaultRootWindow(xDisplay))
            {
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                            "Invalid parentWindowHandle (wrong server or screen)",
                            "EGLWindow::create");
            }
        }

        // Validate externalWindowHandle
        if (externalWindow != 0)
        {
            XWindowAttributes windowAttrib;

            if (!XGetWindowAttributes(xDisplay, externalWindow, &windowAttrib) ||
                windowAttrib.root != DefaultRootWindow(xDisplay))
            {
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                            "Invalid externalWindowHandle (wrong server or screen)",
                            "EGLWindow::create");
            }

            eglDrawable = createSurfaceFromWindow(glDisplay, externalWindow, None);
        }

        ::EGLConfig glConfig = 0;

        if (eglDrawable)
        {
            glConfig = mGLSupport->getGLConfigFromDrawable (eglDrawable, &width, &height);
        }

        if (!glConfig && eglContext)
        {
            glConfig = mGLSupport->getGLConfigFromContext(eglContext);

            if (!glConfig)
            {
                // This should never happen.
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                            "Unexpected failure to determine a EGLFBConfig",
                            "EGLWindow::create");
            }
        }

        mIsExternal = (eglDrawable != 0);
        XSetErrorHandler(oldXErrorHandler);

        if (!glConfig)
        {
            int minAttribs[] = {
                EGL_LEVEL, 0,
                EGL_DEPTH_SIZE, 16,
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                EGL_NONE
            };

            int maxAttribs[] = {
                EGL_SAMPLES, samples,
                EGL_STENCIL_SIZE, INT_MAX,
                EGL_NONE
            };

            glConfig = mGLSupport->selectGLConfig(minAttribs, maxAttribs);
            mHwGamma = false;
        }

        mIsTopLevel = (!mIsExternal && parentWindow == DefaultRootWindow(xDisplay));

        if (!mIsTopLevel)
        {
            mIsFullScreen = false;
            left = top = 0;
        }

        if (mIsFullScreen)
        {
            mGLSupport->switchMode (width, height, frequency);
        }

        if (!mIsExternal)
        {
            XSetWindowAttributes attr;
            ulong mask;
            XVisualInfo *visualInfo = mGLSupport->getVisualFromFBConfig(glConfig);

            attr.background_pixel = 0;
            attr.border_pixel = 0;
            attr.colormap = XCreateColormap(xDisplay,
                                            DefaultRootWindow(xDisplay),
                                            visualInfo->visual,
                                            AllocNone);
            attr.event_mask = StructureNotifyMask | VisibilityChangeMask | FocusChangeMask;
            mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

            if(mIsFullScreen && mGLSupport->mAtomFullScreen == None) 
            {
                LogManager::getSingleton().logMessage("EGLWindow::switchFullScreen: Your WM has no fullscreen support");

                // A second best approach for outdated window managers
                attr.backing_store = NotUseful;
                attr.save_under = False;
                attr.override_redirect = True;
                mask |= CWSaveUnder | CWBackingStore | CWOverrideRedirect;
                left = top = 0;
            }

            // Create window on server
            mWindow = XCreateWindow(xDisplay,
                                    parentWindow,
                                    left, top, width, height,
                                    0, visualInfo->depth,
                                    InputOutput,
                                    visualInfo->visual, mask, &attr);
            XFree(visualInfo);

            if(!mWindow)
            {
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                            "Unable to create an X Window",
                            "EGLWindow::create");
            }

            if (mIsTopLevel)
            {
                XWMHints *wmHints;
                XSizeHints *sizeHints;

                // Is this really necessary ? Which broken WM might need it?
                if ((wmHints = XAllocWMHints()) != NULL)
                {
                    wmHints->initial_state = NormalState;
                    wmHints->input = True;
                    wmHints->flags = StateHint | InputHint;
                }

                // Is this really necessary ? Which broken WM might need it?
                if ((sizeHints = XAllocSizeHints()) != NULL)
                {
                    sizeHints->flags = USPosition;
                }

                XTextProperty titleprop;
                char *lst = (char*)title.c_str();
                XStringListToTextProperty((char **)&lst, 1, &titleprop);
                XSetWMProperties(xDisplay, mWindow, &titleprop,
                                 NULL, NULL, 0, sizeHints, wmHints, NULL);

                XFree(titleprop.value);
                XFree(wmHints);
                XFree(sizeHints);

                XSetWMProtocols(xDisplay, mWindow, &mGLSupport->mAtomDeleteWindow, 1);

                XWindowAttributes windowAttrib;

                XGetWindowAttributes(xDisplay, mWindow, &windowAttrib);

                left = windowAttrib.x;
                top = windowAttrib.y;
                width = windowAttrib.width;
                height = windowAttrib.height;
            }

            eglDrawable = createSurfaceFromWindow(glDisplay, mWindow, glConfig);

            XMapWindow(xDisplay, mWindow);

            if (mIsFullScreen)
            {
                switchFullScreen(true);
            }

            XFlush(xDisplay);

            WindowEventUtilities::_addRenderWindow(this);

        }

        mContext = new EGLContext(mGLSupport, glConfig, eglDrawable);

        ::EGLSurface oldDrawableDraw = eglGetCurrentSurface(EGL_DRAW);
        ::EGLSurface oldDrawableRead = eglGetCurrentSurface(EGL_READ);
        ::EGLContext oldContext  = eglGetCurrentContext();

        int glConfigID;

        mGLSupport->getGLConfigAttrib(glConfig, EGL_CONFIG_ID, &glConfigID);
        LogManager::getSingleton().logMessage("EGLWindow::create used FBConfigID = " + StringConverter::toString(glConfigID));

        mName = name;
        mWidth = width;
        mHeight = height;
        mLeft = left;
        mTop = top;
        mActive = true;
        mClosed = false;
    }

    void EGLWindow::destroy(void)
    {
        if (mClosed)
        {
            return;
        }

        mClosed = true;
        mActive = false;

        if (!mIsExternal)
        {
            WindowEventUtilities::_removeRenderWindow(this);
        }

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

        if (mIsFullScreen != fullscreen && &mGLSupport->mAtomFullScreen == None)
        {
            // Without WM support it is best to give up.
            LogManager::getSingleton().logMessage("EGLWindow::switchFullScreen: Your WM has no fullscreen support");
            return;
        }
        else if (fullscreen)
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

    bool EGLWindow::isClosed() const
    {
        return mClosed;
    }

    bool EGLWindow::isVisible() const
    {
        return mVisible;
    }

    void EGLWindow::setVisible(bool visible)
    {
        mVisible = visible;
    }

    void EGLWindow::reposition(int left, int top)
    {
        if (mClosed || ! mIsTopLevel)
        {
            return;
        }

        XMoveWindow(mGLSupport->getXDisplay(), mWindow, left, top);
    }

    void EGLWindow::resize(uint width, uint height)
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
            if (mIsTopLevel)
            {
                XResizeWindow(mGLSupport->getXDisplay(), mWindow, width, height);
            }
            else
            {
                mWidth = width;
                mHeight = height;

                for (ViewportList::iterator it = mViewportList.begin(); it != mViewportList.end(); ++it)
                {
                    (*it).second->_updateDimensions();
                }
            }
        }
    }

    void EGLWindow::windowMovedOrResized()
    {
        if (mClosed || !mWindow)
        {
            return;
        }

        Display* xDisplay = mGLSupport->getXDisplay();
        XWindowAttributes windowAttrib;

        if (mIsTopLevel && !mIsFullScreen)
        {
            Window parent, root, *children;
            uint nChildren;

            XQueryTree(xDisplay, mWindow, &root, &parent, &children, &nChildren);

            if (children)
            {
                XFree(children);
            }

            XGetWindowAttributes(xDisplay, parent, &windowAttrib);
            mLeft = windowAttrib.x;
            mTop = windowAttrib.y;
        }

        XGetWindowAttributes(xDisplay, mWindow, &windowAttrib);

        if (mWidth == windowAttrib.width && mHeight == windowAttrib.height)
        {
            return;
        }

        mWidth = windowAttrib.width;
        mHeight = windowAttrib.height;

        for (ViewportList::iterator it = mViewportList.begin(); it != mViewportList.end(); ++it)
        {
            (*it).second->_updateDimensions();
        }
    }

    void EGLWindow::swapBuffers(bool waitForVSync)
    {
        if (mClosed || mIsExternalGLControl)
        {
            return;
        }

        glFlush();
        if (eglSwapBuffers(mGLSupport->getGLDisplay(), mContext->mDrawable) == EGL_FALSE)
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        "Fail to SwapBuffers",
                        __FUNCTION__);
        }
    }

    void EGLWindow::getCustomAttribute( const String& name, void* pData )
    {
        if (name == "DISPLAY NAME")
        {
            *static_cast<String*>(pData) = mGLSupport->getDisplayName();
            return;
        }
        else if (name == "DISPLAY")
        {
            *static_cast<EGLDisplay*>(pData) = mGLSupport->getGLDisplay();
            return;
        }
        else if (name == "GLCONTEXT")
        {
            *static_cast<EGLContext**>(pData) = mContext;
            return;
        } 
        else if (name == "XDISPLAY")
        {
            *static_cast<Display**>(pData) = mGLSupport->getXDisplay();
            return;
        }
        else if (name == "ATOM")
        {
            *static_cast< ::Atom* >(pData) = mGLSupport->mAtomDeleteWindow;
            return;
        } 
        else if (name == "WINDOW")
        {
            *static_cast<Window*>(pData) = mWindow;
            return;
        }
    }

    void EGLWindow::copyContentsToMemory(const PixelBox &dst, FrameBuffer buffer)
    {
        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                    "Not implemented.",
                    "");
    }

    void EGLWindow::switchFullScreen(bool fullscreen)
    {
        if (&mGLSupport->mAtomFullScreen != None)
        {
            Display* xDisplay = mGLSupport->getXDisplay();
            XClientMessageEvent xMessage;

            xMessage.type = ClientMessage;
            xMessage.serial = 0;
            xMessage.send_event = True;
            xMessage.window = mWindow;
            xMessage.message_type = mGLSupport->mAtomState;
            xMessage.format = 32;
            xMessage.data.l[0] = (fullscreen ? 1 : 0);
            xMessage.data.l[1] = mGLSupport->mAtomFullScreen;
            xMessage.data.l[2] = 0;

            XSendEvent(xDisplay, DefaultRootWindow(xDisplay), False,
                       SubstructureRedirectMask | SubstructureNotifyMask,
                       (XEvent*)&xMessage);

            mIsFullScreen = fullscreen;
        }
    }

    ::EGLSurface EGLWindow::createSurfaceFromWindow(::EGLDisplay display,
                                                    Window win,
                                                    ::EGLConfig config)
    {
        ::EGLSurface surface;

        surface = eglCreateWindowSurface(display, config,
                                         (NativeWindowType) win, NULL);

        if (surface == EGL_NO_SURFACE)
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        "Fail to create EGLSurface based on X Window",
                        __FUNCTION__);
        }
        return surface;
    }
}
