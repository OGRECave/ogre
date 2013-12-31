/*
-----------------------------------------------------------------------------
This source file is part of OGRE
	(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
-----------------------------------------------------------------------------
*/

#include "OgreGLXWindow.h"
#include "OgreRoot.h"
#include "OgreGLRenderSystem.h"
#include "OgreImageCodec.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreGLXUtils.h"
#include "OgreGLXGLSupport.h"
#include "OgreGLPixelFormat.h"
#include "OgreWindowEventUtilities.h"

#include <iostream>
#include <algorithm>
#include <sys/time.h>
#include <climits>

#include <X11/Xlib.h>
#include <X11/keysym.h>

#include <X11/extensions/Xrandr.h>

extern "C"
{
	int 
	safeXErrorHandler (Display *display, XErrorEvent *event)
	{
		// Ignore all XErrorEvents
		return 0;
	}
	int (*oldXErrorHandler)(Display *, XErrorEvent*);
}

namespace Ogre
{
	//-------------------------------------------------------------------------------------------------//
	GLXWindow::GLXWindow(GLXGLSupport *glsupport) :
		mGLSupport(glsupport), mContext(0)
	{
		mWindow = 0;
		
		mIsTopLevel = false;
		mIsFullScreen = false;
		mIsExternal = false;
		mIsExternalGLControl = false;
		mClosed = false;
		mActive = false;
		mHidden = false;
		mVSync = false;
		mVSyncInterval = 1;
	}
	
	//-------------------------------------------------------------------------------------------------//
	GLXWindow::~GLXWindow() 
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
	
	//-------------------------------------------------------------------------------------------------//
	void GLXWindow::create(const String& name, uint width, uint height,
			   bool fullScreen, const NameValuePairList *miscParams)
	{
		Display *xDisplay = mGLSupport->getXDisplay();
		String title = name;
		uint samples = 0;
		short frequency = 0;
		bool vsync = false;
		bool hidden = false;
		unsigned int vsyncInterval = 1;
		int gamma = 0;
		::GLXContext glxContext = 0;
		::GLXDrawable glxDrawable = 0;
		Window externalWindow = 0;
		Window parentWindow = DefaultRootWindow(xDisplay);
		int left = DisplayWidth(xDisplay, DefaultScreen(xDisplay))/2 - width/2;
		int top  = DisplayHeight(xDisplay, DefaultScreen(xDisplay))/2 - height/2;
		String border;
		
		mIsFullScreen = fullScreen;
		
		if(miscParams)
		{
			NameValuePairList::const_iterator opt;
			NameValuePairList::const_iterator end = miscParams->end();
			
			// NB: Do not try to implement the externalGLContext option.
			//
			//	 Accepting a non-current context would expose us to the 
			//	 risk of segfaults when we made it current. Since the
			//	 application programmers would be responsible for these
			//	 segfaults, they are better discovering them in their code.
		
			if ((opt = miscParams->find("currentGLContext")) != end &&
				StringConverter::parseBool(opt->second))
			{
				if (! glXGetCurrentContext())
				{
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "currentGLContext was specified with no current GL context", "GLXWindow::create");
				}
				
				glxContext = glXGetCurrentContext();
				glxDrawable = glXGetCurrentDrawable();
			}
			
			// Note: Some platforms support AA inside ordinary windows
			if((opt = miscParams->find("FSAA")) != end) 
				samples = StringConverter::parseUnsignedInt(opt->second);
			
			if((opt = miscParams->find("displayFrequency")) != end) 
				frequency = (short)StringConverter::parseInt(opt->second);
			
			if((opt = miscParams->find("vsync")) != end) 
				vsync = StringConverter::parseBool(opt->second);

			if((opt = miscParams->find("hidden")) != end)
				hidden = StringConverter::parseBool(opt->second);

			if((opt = miscParams->find("vsyncInterval")) != end) 
				vsyncInterval = StringConverter::parseUnsignedInt(opt->second);

			if ((opt = miscParams->find("gamma")) != end)
				gamma = StringConverter::parseBool(opt->second);

			if((opt = miscParams->find("left")) != end) 
				left = StringConverter::parseInt(opt->second);
			
			if((opt = miscParams->find("top")) != end) 
				top = StringConverter::parseInt(opt->second);
			
			if((opt = miscParams->find("title")) != end) 
				title = opt->second;
			
			if ((opt = miscParams->find("externalGLControl")) != end)
				mIsExternalGLControl = StringConverter::parseBool(opt->second);
			
			if((opt = miscParams->find("parentWindowHandle")) != end) 
			{
				vector<String>::type tokens = StringUtil::split(opt->second, " :");
				
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
			else if((opt = miscParams->find("externalWindowHandle")) != end) 
			{
				vector<String>::type tokens = StringUtil::split(opt->second, " :");
				
				LogManager::getSingleton().logMessage(
													  "GLXWindow::create: The externalWindowHandle parameter is deprecated.\n"
													  "Use the parentWindowHandle or currentGLContext parameter instead.");
				
				if (tokens.size() == 3)
				{
					// Old display:screen:xid format
					// The old GLX code always created a "parent" window in this case:
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

			if ((opt = miscParams->find("border")) != end)
				border = opt->second;
		}
		
		// Ignore fatal XErrorEvents during parameter validation:
		oldXErrorHandler = XSetErrorHandler(safeXErrorHandler);
		// Validate parentWindowHandle
		
		if (parentWindow != DefaultRootWindow(xDisplay))
		{
			XWindowAttributes windowAttrib;
			
			if (! XGetWindowAttributes(xDisplay, parentWindow, &windowAttrib) ||
				windowAttrib.root != DefaultRootWindow(xDisplay))
			{
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Invalid parentWindowHandle (wrong server or screen)", "GLXWindow::create");
			}
		}
		
		// Validate externalWindowHandle
		
		if (externalWindow != 0)
		{
			XWindowAttributes windowAttrib;
			
			if (! XGetWindowAttributes(xDisplay, externalWindow, &windowAttrib) ||
				windowAttrib.root != DefaultRootWindow(xDisplay))
			{
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Invalid externalWindowHandle (wrong server or screen)", "GLXWindow::create");
			}
			glxDrawable = externalWindow;
		}

		// Derive fbConfig
		
		::GLXFBConfig fbConfig = 0;
		
		if (glxDrawable)
		{
			fbConfig = mGLSupport->getFBConfigFromDrawable (glxDrawable, &width, &height);
		}

		if (! fbConfig && glxContext)
		{
			fbConfig = mGLSupport->getFBConfigFromContext (glxContext);
		}
			
		mIsExternal = (glxDrawable != 0);
		
		XSetErrorHandler(oldXErrorHandler);
		
		if (! fbConfig)
		{
			int minAttribs[] = {
				GLX_DRAWABLE_TYPE,  GLX_WINDOW_BIT,
				GLX_RENDER_TYPE,	GLX_RGBA_BIT,
				GLX_RED_SIZE,	   1,
				GLX_BLUE_SIZE,	  1,
				GLX_GREEN_SIZE,	 1,
				None
			};
			
			int maxAttribs[] = {
				GLX_SAMPLES,		static_cast<int>(samples),
				GLX_DOUBLEBUFFER,   1,
				GLX_STENCIL_SIZE,   INT_MAX,
				GLX_FRAMEBUFFER_SRGB_CAPABLE_EXT, 1,
				None
			};

			fbConfig = mGLSupport->selectFBConfig(minAttribs, maxAttribs);

			// Now check the actual supported fsaa value
			GLint maxSamples;
			mGLSupport->getFBConfigAttrib(fbConfig, GLX_SAMPLES, &maxSamples);
			mFSAA = maxSamples;

			if (gamma != 0)
			{
				mGLSupport->getFBConfigAttrib(fbConfig, GL_FRAMEBUFFER_SRGB_CAPABLE_EXT, &gamma);
			}

			mHwGamma = (gamma != 0);
		}
		
    if (! fbConfig)
    {
      // This should never happen.
      OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unexpected failure to determine a GLXFBConfig","GLXWindow::create");
    }
		
		mIsTopLevel = (! mIsExternal && parentWindow == DefaultRootWindow(xDisplay));
		
		if (! mIsTopLevel)
		{
			mIsFullScreen = false;
			left = top = 0;
		}
		
		if (mIsFullScreen) 
		{
			mGLSupport->switchMode (width, height, frequency);
		}
		
		if (! mIsExternal)
		{
			XSetWindowAttributes attr;
			ulong mask;
			XVisualInfo *visualInfo = mGLSupport->getVisualFromFBConfig (fbConfig);
			
			attr.background_pixel = 0;
			attr.border_pixel = 0;
			attr.colormap = XCreateColormap(xDisplay, DefaultRootWindow(xDisplay), visualInfo->visual, AllocNone);
			attr.event_mask = StructureNotifyMask | VisibilityChangeMask | FocusChangeMask;
			mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;
			
			if(mIsFullScreen && mGLSupport->mAtomFullScreen == None) 
			{
				LogManager::getSingleton().logMessage("GLXWindow::switchFullScreen: Your WM has no fullscreen support");
				
				// A second best approach for outdated window managers
				attr.backing_store = NotUseful;
				attr.save_under = False;
				attr.override_redirect = True;
				mask |= CWSaveUnder | CWBackingStore | CWOverrideRedirect;
				left = top = 0;
			} 
			
			// Create window on server
			mWindow = XCreateWindow(xDisplay, parentWindow, left, top, width, height, 0, visualInfo->depth, InputOutput, visualInfo->visual, mask, &attr);
			
			XFree(visualInfo);
			
			if(!mWindow) 
			{
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to create an X Window", "GLXWindow::create");
			}
			
			if (mIsTopLevel)
			{
				XWMHints *wmHints;
				XSizeHints *sizeHints;
				
				if ((wmHints = XAllocWMHints()) != NULL) 
				{
					wmHints->initial_state = NormalState;
					wmHints->input = True;
					wmHints->flags = StateHint | InputHint;
					
					int depth = DisplayPlanes(xDisplay, DefaultScreen(xDisplay));
					
					// Check if we can give it an icon
					if(depth == 24 || depth == 32) 
					{
						if(mGLSupport->loadIcon("GLX_icon.png", &wmHints->icon_pixmap, &wmHints->icon_mask))
						{
							wmHints->flags |= IconPixmapHint | IconMaskHint;
						}
					}
				}
				
				if ((sizeHints = XAllocSizeHints()) != NULL)
				{
					// Is this really necessary ? Which broken WM might need it?
					sizeHints->flags = USPosition;

					if(!fullScreen && border == "fixed")
					{
						sizeHints->min_width = sizeHints->max_width = width;
						sizeHints->min_height = sizeHints->max_height = height;
						sizeHints->flags |= PMaxSize | PMinSize;
					}
				}
				
				XTextProperty titleprop;
				vector<char>::type  title_ (title.begin(), title.end());
				title_.push_back(0);

				char *lst = &title_[0];
				XStringListToTextProperty((char **)&lst, 1, &titleprop);
				XSetWMProperties(xDisplay, mWindow, &titleprop, NULL, NULL, 0, sizeHints, wmHints, NULL);
				
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
			
			glxDrawable = mWindow;
			
			// setHidden takes care of mapping or unmapping the window
			// and also calls setFullScreen if appropriate.
			setHidden(hidden);
			XFlush(xDisplay);
			
			WindowEventUtilities::_addRenderWindow(this);
		}
		
		mContext = new GLXContext(mGLSupport, fbConfig, glxDrawable, glxContext);
		
		// apply vsync settings. call setVSyncInterval first to avoid 
		// setting vsync more than once.
		setVSyncInterval(vsyncInterval);
		setVSyncEnabled(vsync);
		
		int fbConfigID;
		
		mGLSupport->getFBConfigAttrib(fbConfig, GLX_FBCONFIG_ID, &fbConfigID);
		
		LogManager::getSingleton().logMessage("GLXWindow::create used FBConfigID = " + StringConverter::toString(fbConfigID));
		
		mName = name;
		mWidth = width;
		mHeight = height;
		mLeft = left;
		mTop = top;
		mActive = true;
		mClosed = false;
	}
	
	//-------------------------------------------------------------------------------------------------//
	void GLXWindow::destroy(void)
	{
		if (mClosed)
			return;
		
		mClosed = true;
		mActive = false;
		
		if (! mIsExternal)
			WindowEventUtilities::_removeRenderWindow(this);
		
		if (mIsFullScreen) 
		{
			mGLSupport->switchMode();
			switchFullScreen(false);
		}
	}
	
	//-------------------------------------------------------------------------------------------------//
	void GLXWindow::setFullscreen(bool fullscreen, uint width, uint height)
	{
		short frequency = 0;
		
		if (mClosed || ! mIsTopLevel)
			return;
		
		if (fullscreen == mIsFullScreen && width == mWidth && height == mHeight)
			return;
		
		if (mIsFullScreen != fullscreen && &mGLSupport->mAtomFullScreen == None)
		{
			// Without WM support it is best to give up.
			LogManager::getSingleton().logMessage("GLXWindow::switchFullScreen: Your WM has no fullscreen support");
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
		
		if (! mIsFullScreen)
		{
			resize(width, height);
			reposition(mLeft, mTop);
		}
	}

	//-------------------------------------------------------------------------------------------------//
	bool GLXWindow::isClosed() const
	{
		return mClosed;
	}
	
	//-------------------------------------------------------------------------------------------------//
	bool GLXWindow::isVisible() const
	{
		return mVisible;
	}
	
	//-------------------------------------------------------------------------------------------------//
	void GLXWindow::setVisible(bool visible)
	{
		mVisible = visible;
	}
	
	//-------------------------------------------------------------------------------------------------//
	void GLXWindow::setHidden(bool hidden)
	{
		mHidden = hidden;
		// ignore for external windows as these should handle
		// this externally
		if (mIsExternal)
			return;

		if (hidden)
		{
			XUnmapWindow(mGLSupport->getXDisplay(), mWindow);
		}
		else
		{
			XMapWindow(mGLSupport->getXDisplay(), mWindow);
			if (mIsFullScreen)
			{
				switchFullScreen(true);
			}
		}
	}

	//-------------------------------------------------------------------------------------------------//
	void GLXWindow::setVSyncInterval(unsigned int interval)
	{
		mVSyncInterval = interval;
		if (mVSync)
			setVSyncEnabled(true);
	}

	//-------------------------------------------------------------------------------------------------//
	void GLXWindow::setVSyncEnabled(bool vsync)
	{
		mVSync = vsync;
		// we need to make our context current to set vsync
		// store previous context to restore when finished.
		::GLXDrawable oldDrawable = glXGetCurrentDrawable();
		::GLXContext  oldContext  = glXGetCurrentContext();
		
		mContext->setCurrent();
		
		if (! mIsExternalGLControl)
		{
			if (GLXEW_MESA_swap_control)
				glXSwapIntervalMESA (vsync ? mVSyncInterval : 0);
			else if (GLXEW_EXT_swap_control)
				glXSwapIntervalEXT (mGLSupport->getGLDisplay(), glXGetCurrentDrawable(),
									vsync ? mVSyncInterval : 0);
			else if (GLXEW_SGI_swap_control)
				if (vsync && mVSyncInterval)
					glXSwapIntervalSGI (mVSyncInterval);
		}
		
		mContext->endCurrent();
		
		glXMakeCurrent (mGLSupport->getGLDisplay(), oldDrawable, oldContext);
	}

	//-------------------------------------------------------------------------------------------------//
	bool GLXWindow::isVSyncEnabled() const
	{
		return mVSync;
	}

	//-------------------------------------------------------------------------------------------------//
	unsigned int GLXWindow::getVSyncInterval() const
	{
		return mVSyncInterval;
	}

	//-------------------------------------------------------------------------------------------------//
	void GLXWindow::reposition(int left, int top)
	{
		if (mClosed || ! mIsTopLevel)
			return;
		
		XMoveWindow(mGLSupport->getXDisplay(), mWindow, left, top);
	}

	//-------------------------------------------------------------------------------------------------//
	void GLXWindow::resize(uint width, uint height)
	{
		if (mClosed)
			return;
		
		if(mWidth == width && mHeight == height)
			return;
		
		if(width != 0 && height != 0)
		{
			if (!mIsExternal)
			{
				XResizeWindow(mGLSupport->getXDisplay(), mWindow, width, height);
			}
			else
			{
				mWidth = width;
				mHeight = height;
				
				for (ViewportList::iterator it = mViewportList.begin();	it != mViewportList.end(); ++it)
					(*it).second->_updateDimensions();
			}
		}
	}

	//-------------------------------------------------------------------------------------------------//
	void GLXWindow::windowMovedOrResized()
	{
		if (mClosed || !mWindow)
			return;
		
		Display* xDisplay = mGLSupport->getXDisplay();
		XWindowAttributes windowAttrib;
		
		if (mIsTopLevel && !mIsFullScreen)
		{
			Window parent, root, *children;
			uint nChildren;
			
			XQueryTree(xDisplay, mWindow, &root, &parent, &children, &nChildren);
			
			if (children)
				XFree(children);
			
			XGetWindowAttributes(xDisplay, parent, &windowAttrib);
			
			mLeft = windowAttrib.x;
			mTop  = windowAttrib.y;
		}
		
		XGetWindowAttributes(xDisplay, mWindow, &windowAttrib);
		
		if (mWidth == (unsigned)windowAttrib.width && mHeight == (unsigned)windowAttrib.height)
			return;
		
		mWidth = windowAttrib.width;
		mHeight = windowAttrib.height;
		
		for (ViewportList::iterator it = mViewportList.begin();	it != mViewportList.end(); ++it)
			(*it).second->_updateDimensions();
	}
	
	//-------------------------------------------------------------------------------------------------//
	void GLXWindow::swapBuffers()
	{
		if (mClosed || mIsExternalGLControl) 
			return;
		
		glXSwapBuffers(mGLSupport->getGLDisplay(), mContext->mDrawable);
	}
	
	//-------------------------------------------------------------------------------------------------//
	void GLXWindow::getCustomAttribute( const String& name, void* pData )
	{
		if( name == "DISPLAY NAME" ) 
		{
			*static_cast<String*>(pData) = mGLSupport->getDisplayName();
			return;
		}
		else if( name == "DISPLAY" ) 
		{
			*static_cast<Display**>(pData) = mGLSupport->getGLDisplay();
			return;
		}
		else if( name == "GLCONTEXT" ) 
		{
			*static_cast<GLXContext**>(pData) = mContext;
			return;
		} 
		else if( name == "XDISPLAY" ) 
		{
			*static_cast<Display**>(pData) = mGLSupport->getXDisplay();
			return;
		}
		else if( name == "ATOM" ) 
		{
			*static_cast< ::Atom* >(pData) = mGLSupport->mAtomDeleteWindow;
			return;
		} 
		else if( name == "WINDOW" ) 
		{
			*static_cast<Window*>(pData) = mWindow;
			return;
		} 
	}
	
	//-------------------------------------------------------------------------------------------------//
	void GLXWindow::copyContentsToMemory(const PixelBox &dst, FrameBuffer buffer)
	{
		if (mClosed)
			return;
		
		if ((dst.right > mWidth) ||
			(dst.bottom > mHeight) ||
			(dst.front != 0) || (dst.back != 1))
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Invalid box.", "GLXWindow::copyContentsToMemory" );
		}
		
		if (buffer == FB_AUTO)
		{
			buffer = mIsFullScreen? FB_FRONT : FB_BACK;
		}
		
		GLenum format = Ogre::GLPixelUtil::getGLOriginFormat(dst.format);
		GLenum type = Ogre::GLPixelUtil::getGLOriginDataType(dst.format);
		
		if ((format == GL_NONE) || (type == 0))
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Unsupported format.", "GLXWindow::copyContentsToMemory" );
		}
		
		// Switch context if different from current one
		RenderSystem* rsys = Root::getSingleton().getRenderSystem();
		rsys->_setViewport(this->getViewport(0));
		
        if(dst.getWidth() != dst.rowPitch)
            glPixelStorei(GL_PACK_ROW_LENGTH, dst.rowPitch);
		// Must change the packing to ensure no overruns!
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		
		glReadBuffer((buffer == FB_FRONT)? GL_FRONT : GL_BACK);
        glReadPixels((GLint)0, (GLint)(mHeight - dst.getHeight()),
                     (GLsizei)dst.getWidth(), (GLsizei)dst.getHeight(),
                     format, type, dst.getTopLeftFrontPixelPtr());
		
		// restore default alignment
		glPixelStorei(GL_PACK_ALIGNMENT, 4);
        glPixelStorei(GL_PACK_ROW_LENGTH, 0);
        
        PixelUtil::bulkPixelVerticalFlip(dst);
	}

	//-------------------------------------------------------------------------------------------------//
	void GLXWindow::switchFullScreen(bool fullscreen)
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
			
			XSendEvent(xDisplay, DefaultRootWindow(xDisplay), False, SubstructureRedirectMask | SubstructureNotifyMask, (XEvent*)&xMessage); 
			
			mIsFullScreen = fullscreen;
		}
	}
}
