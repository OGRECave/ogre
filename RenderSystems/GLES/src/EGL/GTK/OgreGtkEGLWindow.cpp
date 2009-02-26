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

#include "OgreGtkEGLSupport.h"
#include "OgreGtkEGLWindow.h"
#include "OgreGtkEGLContext.h"

#include <iostream>
#include <algorithm>
#include <climits>

#include <sys/time.h>
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
	GtkEGLWindow::GtkEGLWindow(EGLSupport *glsupport)
		: EGLWindow(glsupport) , 
		  mParentWindow(glsupport)  
	{
	}

	GtkEGLWindow::~GtkEGLWindow()
	{

		Display* xDisplay = mGLSupport->getXDisplay();
		// Ignore fatal XErrorEvents from stale handles.
		oldXErrorHandler = XSetErrorHandler(safeXErrorHandler);

		if (mWindow)
		{
			XDestroyWindow(xDisplay, mWindow);
		}

		XSetErrorHandler(oldXErrorHandler);
		mWindow = 0;

	}

	void GtkEGLWindow::getCustomAttribute( const String& name, void* pData )
	{
		EGLWindow::getCustomAttribute(name, pData);
		if (name == "ATOM")
		{
			*static_cast< ::Atom* >(pData) = mGLSupport->mAtomDeleteWindow;
			return;
		} 
		else if (name == "XDISPLAY")
		{
			*static_cast<Display**>(pData) = mGLSupport->getXDisplay();
			return;
		}
		else if (name == "XWINDOW")
		{
			*static_cast<Window*>(pData) = mWindow;
			return;
		}

	}

	EGLContext * GtkEGLWindow::createEGLContext() const
	{
		return new gtkEGLContext(mGLSupport, mEglConfig, mEglSurface);
	}

	void GtkEGLWindow::getLeftAndTopFromNativeWindow( int & left, int & top )
	{
		Display *xDisplay = mGLSupport->getXDisplay();
		left = DisplayWidth(xDisplay, DefaultScreen(xDisplay))/2 - width/2;
		top  = DisplayHeight(xDisplay, DefaultScreen(xDisplay))/2 - height/2;
	}

	void GtkEGLWindow::initNativeCreatedWindow()
	{
		if (miscParams)
		{
			NameValuePairList::const_iterator opt;
			NameValuePairList::const_iterator end = miscParams->end();

			Window externalWindow = 0;
			mParentWindow = DefaultRootWindow(xDisplay);

			if ((opt = miscParams->find("parentWindowHandle")) != end)
			{
				vector<String>::type tokens = StringUtil::split(opt->second, " :");

				if (tokens.size() == 3)
				{
					// deprecated display:screen:xid format
					mParentWindow = StringConverter::parseUnsignedLong(tokens[2]);
				}
				else
				{
					// xid format
					mParentWindow = StringConverter::parseUnsignedLong(tokens[0]);
				}
			}
			else if ((opt = miscParams->find("externalWindowHandle")) != end)
			{
				vector<String>::type tokens = StringUtil::split(opt->second, " :");

				LogManager::getSingleton().logMessage(
					"EGLWindow::create: The externalWindowHandle parameter is deprecated.\n"
					"Use the parentWindowHandle or currentGLContext parameter instead.");
				if (tokens.size() == 3)
				{
					// Old display:screen:xid format
					// The old EGL code always created a "parent" window in this case:
					mParentWindow = StringConverter::parseUnsignedLong(tokens[2]);
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
		if (mParentWindow != DefaultRootWindow(xDisplay))
		{
			XWindowAttributes windowAttrib;

			if (!XGetWindowAttributes(xDisplay, mParentWindow, &windowAttrib) ||
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

			mEglConfig = 0;
			mEglSurface = createSurfaceFromWindow(glDisplay, externalWindow);
		}

		XSetErrorHandler(oldXErrorHandler);

		mIsTopLevel = (!mIsExternal && mParentWindow == DefaultRootWindow(xDisplay));

	}

	void GtkEGLWindow::createNativeWindow( int &left, int &top, uint &width, uint &height, String &title )
	{
		mEglDisplay = mGLSupport->getGLDisplay();//todo
		XSetWindowAttributes attr;
		ulong mask;
		XVisualInfo *visualInfo = mGLSupport->getVisualFromFBConfig(mEglConfig);

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
			mParentWindow,
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

		mEglSurface = createSurfaceFromWindow(mGLSupport->getGLDisplay(), mWindow);

		XMapWindow(xDisplay, mWindow);

		if (mIsFullScreen)
		{
			switchFullScreen(true);
		}

		XFlush(xDisplay);

		WindowEventUtilities::_addRenderWindow(this);
	}

	void GtkEGLWindow::setFullscreen( bool fullscreen, uint width, uint height )
	{
		if (mIsFullScreen != fullscreen && &mGLSupport->mAtomFullScreen == None)
		{
			// Without WM support it is best to give up.
			LogManager::getSingleton().logMessage("EGLWindow::switchFullScreen: Your WM has no fullscreen support");
			return;
		}
		EGLWindow::setFullscreen(fullscreen, width, height);
	}

	void GtkEGLWindow::reposition( int left, int top )
	{
		if (mClosed || ! mIsTopLevel)
		{
			return;
		}

		XMoveWindow(mGLSupport->getXDisplay(), mWindow, left, top);
	}

	void GtkEGLWindow::resize(uint width, uint height)
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

	void GtkEGLWindow::windowMovedOrResized()
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
	void GtkEGLWindow::switchFullScreen(bool fullscreen)
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



}