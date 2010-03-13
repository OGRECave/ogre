/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
Copyright (c) 2000-2009 Torus Knot Software Ltd

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
#include "OgreWindowEventUtilities.h"

#include "OgreGLES2Prerequisites.h"
#include "OgreGLES2RenderSystem.h"

#include "OgreX11EGLSupport.h"
#include "OgreX11EGLWindow.h"
#include "OgreX11EGLContext.h"

#include <iostream>
#include <algorithm>
#include <climits>

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
		// Ignore fatal XErrorEvents from stale handles.
		oldXErrorHandler = XSetErrorHandler(safeXErrorHandler);

		if (mWindow)
		{
			XDestroyWindow((Display*)mNativeDisplay, (Window)mWindow);
		}

		XSetErrorHandler(oldXErrorHandler);
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

	EGLContext * X11EGLWindow::createEGLContext() const
	{
		return new X11EGLContext(mEglDisplay, mGLSupport, mEglConfig, mEglSurface);
	}

	void X11EGLWindow::getLeftAndTopFromNativeWindow( int & left, int & top, uint width, uint height )
	{
		NativeDisplayType mNativeDisplay = mGLSupport->getNativeDisplay();
		left = DisplayWidth((Display*)mNativeDisplay, DefaultScreen(mNativeDisplay))/2 - width/2;
		top  = DisplayHeight((Display*)mNativeDisplay, DefaultScreen(mNativeDisplay))/2 - height/2;
	}

	void X11EGLWindow::initNativeCreatedWindow(const NameValuePairList *miscParams)
	{
		if (miscParams)
		{
			NameValuePairList::const_iterator opt;
			NameValuePairList::const_iterator end = miscParams->end();

			mExternalWindow = 0;
			mNativeDisplay = mGLSupport->getNativeDisplay();
			mParentWindow = DefaultRootWindow((Display*)mNativeDisplay);

			if ((opt = miscParams->find("parentWindowHandle")) != end)
			{
				//vector<String>::type tokens = StringUtil::split(opt->second, " :");
		                StringVector tokens = StringUtil::split(opt->second, " :");

				if (tokens.size() == 3)
				{
					// deprecated display:screen:xid format
					mParentWindow = (Window)StringConverter::parseUnsignedLong(tokens[2]);
				}
				else
				{
					// xid format
					mParentWindow = (Window)StringConverter::parseUnsignedLong(tokens[0]);
				}
			}
			else if ((opt = miscParams->find("externalWindowHandle")) != end)
			{
				//vector<String>::type tokens = StringUtil::split(opt->second, " :");
		                StringVector tokens = StringUtil::split(opt->second, " :");

				LogManager::getSingleton().logMessage(
					"EGLWindow::create: The externalWindowHandle parameter is deprecated.\n"
					"Use the parentWindowHandle or currentGLContext parameter instead.");
				if (tokens.size() == 3)
				{
					// Old display:screen:xid format
					// The old EGL code always created a "parent" window in this case:
					mParentWindow = (Window)StringConverter::parseUnsignedLong(tokens[2]);
				}
				else if (tokens.size() == 4)
				{
					// Old display:screen:xid:visualinfo format
					mExternalWindow = (Window)StringConverter::parseUnsignedLong(tokens[2]);
				}
				else
				{
					// xid format
					mExternalWindow = (Window)StringConverter::parseUnsignedLong(tokens[0]);
				}
			}

		}

		// Ignore fatal XErrorEvents during parameter validation:
		oldXErrorHandler = XSetErrorHandler(safeXErrorHandler);

		// Validate parentWindowHandle
		if (mParentWindow != DefaultRootWindow((Display*)mNativeDisplay))
		{
			XWindowAttributes windowAttrib;

			if (!XGetWindowAttributes((Display*)mNativeDisplay, mParentWindow, &windowAttrib) ||
				windowAttrib.root != DefaultRootWindow((Display*)mNativeDisplay))
			{
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
					"Invalid parentWindowHandle (wrong server or screen)",
					"EGLWindow::create");
			}
		}

		// Validate externalWindowHandle
		if (mExternalWindow != 0)
		{
			XWindowAttributes windowAttrib;

			if (!XGetWindowAttributes((Display*)mNativeDisplay, mExternalWindow, &windowAttrib) ||
				windowAttrib.root != DefaultRootWindow((Display*)mNativeDisplay))
			{
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
					"Invalid externalWindowHandle (wrong server or screen)",
					"EGLWindow::create");
			}

			mEglConfig = 0;
			mEglSurface = createSurfaceFromWindow(mEglDisplay, (NativeWindowType)mExternalWindow);
		}

		XSetErrorHandler(oldXErrorHandler);

		mIsTopLevel = (!mIsExternal && mParentWindow == DefaultRootWindow((Display*)mNativeDisplay));

	}

	void X11EGLWindow::createNativeWindow( int &left, int &top, uint &width, uint &height, String &title )
	{
		mEglDisplay = mGLSupport->getGLDisplay();//todo
		XSetWindowAttributes attr;
		ulong mask;
		XVisualInfo *visualInfo = mGLSupport->getVisualFromFBConfig(mEglConfig);

		attr.background_pixel = 0;
		attr.border_pixel = 0;
		attr.colormap = XCreateColormap((Display*)mNativeDisplay,
			DefaultRootWindow((Display*)mNativeDisplay),
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
		mWindow = (NativeWindowType)XCreateWindow((Display*)mNativeDisplay,
			mParentWindow,
			left, top, width, height,
			0, visualInfo->depth,
			InputOutput,
			visualInfo->visual, mask, &attr);
		XFree(visualInfo);

		if(!mWindow)
		{
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
				"Unable to create an X NativeWindowType",
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
			XSetWMProperties((Display*)mNativeDisplay, (Window)mWindow, &titleprop,
				NULL, NULL, 0, sizeHints, wmHints, NULL);

			XFree(titleprop.value);
			XFree(wmHints);
			XFree(sizeHints);

			XSetWMProtocols((Display*)mNativeDisplay, (Window)mWindow, &mGLSupport->mAtomDeleteWindow, 1);

			XWindowAttributes windowAttrib;

			XGetWindowAttributes((Display*)mNativeDisplay, (Window)mWindow, &windowAttrib);

			left = windowAttrib.x;
			top = windowAttrib.y;
			width = windowAttrib.width;
			height = windowAttrib.height;
		}

		mEglSurface = createSurfaceFromWindow(mGLSupport->getGLDisplay(), mWindow);

		XMapWindow((Display*)mNativeDisplay, (Window)mWindow);

		if (mIsFullScreen)
		{
			switchFullScreen(true);
		}

		XFlush((Display*)mNativeDisplay);

		WindowEventUtilities::_addRenderWindow(this);
	}

	void X11EGLWindow::setFullscreen( bool fullscreen, uint width, uint height )
	{
		if (mIsFullScreen != fullscreen && &mGLSupport->mAtomFullScreen == None)
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
			if (mIsTopLevel)
			{ 
				XResizeWindow((Display*)mGLSupport->getNativeDisplay(), (Window)mWindow, width, height);
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

	void X11EGLWindow::windowMovedOrResized()
	{
		if (mClosed || !mWindow)
		{
			return;
		}

		NativeDisplayType mNativeDisplay = mGLSupport->getNativeDisplay();
		XWindowAttributes windowAttrib;

		if (mIsTopLevel && !mIsFullScreen)
		{
			Window parent, root, *children;
			uint nChildren;

			XQueryTree((Display*)mNativeDisplay, (Window)mWindow, &root, &parent, &children, &nChildren);

			if (children)
			{
				XFree(children);
			}

			XGetWindowAttributes((Display*)mNativeDisplay, parent, &windowAttrib);
			mLeft = windowAttrib.x;
			mTop = windowAttrib.y;
		}

		XGetWindowAttributes((Display*)mNativeDisplay, (Window)mWindow, &windowAttrib);

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
	void X11EGLWindow::switchFullScreen(bool fullscreen)
	{ 
		if (&mGLSupport->mAtomFullScreen != None)
		{
			NativeDisplayType mNativeDisplay = mGLSupport->getNativeDisplay();
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
        uint samples = 0;
        int gamma;
        short frequency = 0;
        bool vsync = false;
        ::EGLContext eglContext = 0;
		int left = 0;
		int top  = 0;

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
                if (eglContext)
                {
                    OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                                "currentGLContext was specified with no current GL context",
                                "EGLWindow::create");
                }

                eglContext = eglGetCurrentContext();
                mEglSurface = eglGetCurrentSurface(EGL_DRAW);
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
	}

	initNativeCreatedWindow(miscParams);

        if (mEglSurface)
        {
            mEglConfig = mGLSupport->getGLConfigFromDrawable (mEglSurface, &width, &height);
        }

        if (!mEglConfig && eglContext)
        {
            mEglConfig = mGLSupport->getGLConfigFromContext(eglContext);

            if (!mEglConfig)
            {
                // This should never happen.
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                            "Unexpected failure to determine a EGLFBConfig",
                            "EGLWindow::create");
            }
        }

        mIsExternal = (mEglSurface != 0);



        if (!mEglConfig)
        {
            int minAttribs[] = {
                EGL_DEPTH_SIZE,     16,
                EGL_SAMPLE_BUFFERS,  0,
                EGL_SAMPLES,         0,
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                EGL_NONE
            };

            int maxAttribs[] = {
                EGL_SAMPLES, samples,
                EGL_STENCIL_SIZE, INT_MAX,
                EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                EGL_NONE
            };

            mEglConfig = mGLSupport->selectGLConfig(minAttribs, maxAttribs);
            mHwGamma = false;
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

		if (!mIsExternal)
        {
			createNativeWindow(left, top, width, height, title);
		}

		mContext = createEGLContext();

        ::EGLSurface oldDrawableDraw = eglGetCurrentSurface(EGL_DRAW);
        ::EGLSurface oldDrawableRead = eglGetCurrentSurface(EGL_READ);
        ::EGLContext oldContext  = eglGetCurrentContext();

        int glConfigID;

        mGLSupport->getGLConfigAttrib(mEglConfig, EGL_CONFIG_ID, &glConfigID);
        LogManager::getSingleton().logMessage("EGLWindow::create used FBConfigID = " + StringConverter::toString(glConfigID));

        mName = name;
        mWidth = width;
        mHeight = height;
        mLeft = left;
        mTop = top;
        mActive = true;
		mVisible = true;

        mClosed = false;
	}



}