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
#include "OgreRenderSystem.h"
#include "OgreImageCodec.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreViewport.h"

#include "OgreGLXContext.h"
#include "OgreGLXGLSupport.h"
#include "OgreX11.h"

#include <climits>

namespace Ogre
{
    //-------------------------------------------------------------------------------------------------//
    GLXWindow::GLXWindow(GLXGLSupport *glsupport) : GLWindow(),
        mGLSupport(glsupport)
    {
        mWindow = 0;
    }

    //-------------------------------------------------------------------------------------------------//
    GLXWindow::~GLXWindow()
    {
        Display* xDisplay = mGLSupport->getXDisplay();

        destroy();

        if (mWindow && mIsTopLevel)
        {
            destroyXWindow(xDisplay, mWindow);
        }

        if (mContext)
        {
            delete mContext;
        }

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
        bool gamma = false;
        ::GLXContext glxContext = 0;
        ::GLXDrawable glxDrawable = 0;
        Window parentWindow = DefaultRootWindow(xDisplay);
        int left = DisplayWidth(xDisplay, DefaultScreen(xDisplay))/2 - width/2;
        int top  = DisplayHeight(xDisplay, DefaultScreen(xDisplay))/2 - height/2;

        int minBufferSize = 16;

        mIsFullScreen = fullScreen;

        if(miscParams)
        {
            NameValuePairList::const_iterator opt;
            NameValuePairList::const_iterator end = miscParams->end();

            // NB: Do not try to implement the externalGLContext option.
            //
            //   Accepting a non-current context would expose us to the
            //   risk of segfaults when we made it current. Since the
            //   application programmers would be responsible for these
            //   segfaults, they are better discovering them in their code.

            if ((opt = miscParams->find("currentGLContext")) != end &&
                StringConverter::parseBool(opt->second))
            {
                glxContext = glXGetCurrentContext();

                if (!glxContext)
                {
                    OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "currentGLContext was specified with no current GL context", "GLXWindow::create");
                }

                glxDrawable = glXGetCurrentDrawable();
            }

            // Note: Some platforms support AA inside ordinary windows
            if((opt = miscParams->find("FSAA")) != end)
                samples = StringConverter::parseUnsignedInt(opt->second);

            if( (opt = miscParams->find("displayFrequency")) != end && opt->second != "N/A" )
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

            if((opt = miscParams->find("minColourBufferSize")) != end)
                minBufferSize = StringConverter::parseInt(opt->second);

            if ((opt = miscParams->find("externalGLControl")) != end)
                mIsExternalGLControl = StringConverter::parseBool(opt->second);
            
#if OGRE_NO_QUAD_BUFFER_STEREO == 0
			if ((opt = miscParams->find("stereoMode")) != end)
			{
				StereoModeType stereoMode = StringConverter::parseStereoMode(opt->second);
				if (SMT_NONE != stereoMode)
					mStereoEnabled = true;
			}
#endif

            if ((opt = miscParams->find("parentWindowHandle")) != end ||
                (opt = miscParams->find("externalWindowHandle")) != end)
            {
                std::vector<String> tokens = StringUtil::split(opt->second, " :");

                if (tokens.size() >= 3)
                {
                    // deprecated display:screen:xid:visualinfo format
                    StringConverter::parse(tokens[2], parentWindow);
                }
                else
                {
                    // xid format
                    StringConverter::parse(tokens[0], parentWindow);
                }

                // reset drawable in case currentGLContext was used
                // it should be queried from the parentWindow
                glxDrawable = 0;
            }
        }

        validateParentWindow(xDisplay, parentWindow);

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

        if (! fbConfig)
        {
            int minComponentSize = minBufferSize;
            int maxComponentSize = 8;

            bool fourComponents = (minBufferSize % 3) != 0;
            minComponentSize /= fourComponents ? 4 : 3;

            if(minComponentSize > maxComponentSize)
                maxComponentSize = minComponentSize;

            int minAttribs[] = {
                GLX_DRAWABLE_TYPE,  GLX_WINDOW_BIT,
                GLX_RENDER_TYPE,        GLX_RGBA_BIT,
                GLX_RED_SIZE,      minComponentSize,
                GLX_BLUE_SIZE,    minComponentSize,
                GLX_GREEN_SIZE,  minComponentSize,
                GLX_ALPHA_SIZE,  fourComponents ? minComponentSize : 0,
#if OGRE_NO_QUAD_BUFFER_STEREO == 0
				GLX_STEREO, mStereoEnabled ? True : False,
#endif
                GLX_FRAMEBUFFER_SRGB_CAPABLE_EXT, gamma,
                None
            };

            int maxAttribs[] = {
                GLX_SAMPLES,            static_cast<int>(samples),
                GLX_RED_SIZE,      maxComponentSize,
                GLX_BLUE_SIZE,    maxComponentSize,
                GLX_GREEN_SIZE,  maxComponentSize,
                GLX_ALPHA_SIZE,  fourComponents ? maxComponentSize : 0,
                GLX_DOUBLEBUFFER,   1,
                GLX_STENCIL_SIZE,   INT_MAX,
                GLX_FRAMEBUFFER_SRGB_CAPABLE_EXT, 1,
                None
            };

            fbConfig = mGLSupport->selectFBConfig(minAttribs, maxAttribs);
        }

        // This should never happen.
        if(!fbConfig)
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unexpected failure to determine a GLXFBConfig");


        // Now check the actual fsaa and gamma value

        GLint fsaa;
        mGLSupport->getFBConfigAttrib(fbConfig, GLX_SAMPLES, &fsaa);
        mFSAA = fsaa;

        if (gamma)
        {
            int val = 0;
            gamma = mGLSupport->getFBConfigAttrib(fbConfig, GLX_FRAMEBUFFER_SRGB_CAPABLE_EXT, &val) == 0;
            gamma = gamma && val; // can an supported extension return 0? lets rather be safe..
        }
        mHwGamma = gamma;

        int bufferSize = 0;
        for(int i = GLX_RED_SIZE; i < GLX_ALPHA_SIZE + 1; i++)
        {
            int val = 0;
            mGLSupport->getFBConfigAttrib(fbConfig, i, &val);
            bufferSize += val;
        }

        LogManager::getSingleton().logMessage(StringUtil::format(
            "GLXWindow::create colourBufferSize=%d gamma=%d FSAA=%d", bufferSize, mHwGamma, fsaa));

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
            XVisualInfo *visualInfo = mGLSupport->getVisualFromFBConfig (fbConfig);

            // Create window on server
            mWindow = createXWindow(xDisplay, parentWindow, visualInfo, left, top, width, height, mGLSupport->mAtomFullScreen, mIsFullScreen);

            if (mIsTopLevel)
            {
                finaliseTopLevel(xDisplay, mWindow, left, top, width, height, title, mGLSupport->mAtomDeleteWindow);
            }

            glxDrawable = mWindow;

            // setHidden takes care of mapping or unmapping the window
            // and also calls setFullScreen if appropriate.
            setHidden(hidden);
            XFlush(xDisplay);
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
        mVisible = true;
    }

    //-------------------------------------------------------------------------------------------------//
    void GLXWindow::destroy(void)
    {
        if (mClosed)
            return;

        mClosed = true;
        mActive = false;

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

        if (mIsFullScreen != fullscreen && mGLSupport->mAtomFullScreen == None)
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
    void GLXWindow::setVSyncEnabled(bool vsync)
    {
        mVSync = vsync;
        // we need to make our context current to set vsync
        // store previous context to restore when finished.
        ::GLXDrawable oldDrawable = glXGetCurrentDrawable();
        ::GLXContext  oldContext  = glXGetCurrentContext();

        mContext->setCurrent();

        PFNGLXSWAPINTERVALEXTPROC _glXSwapIntervalEXT;
        _glXSwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)mGLSupport->getProcAddress("glXSwapIntervalEXT");
        PFNGLXSWAPINTERVALMESAPROC _glXSwapIntervalMESA;
        _glXSwapIntervalMESA = (PFNGLXSWAPINTERVALMESAPROC)mGLSupport->getProcAddress("glXSwapIntervalMESA");
        PFNGLXSWAPINTERVALSGIPROC _glXSwapIntervalSGI;
        _glXSwapIntervalSGI = (PFNGLXSWAPINTERVALSGIPROC)mGLSupport->getProcAddress("glXSwapIntervalSGI");

        if (! mIsExternalGLControl )
        {
            if( _glXSwapIntervalEXT )
            {
                _glXSwapIntervalEXT( mGLSupport->getGLDisplay(), static_cast<GLXContext*>(mContext)->mDrawable,
                                     vsync ? mVSyncInterval : 0 );
            }
            else if( _glXSwapIntervalMESA )
                _glXSwapIntervalMESA( vsync ? mVSyncInterval : 0 );
            else {
		OgreAssert(_glXSwapIntervalSGI, "no glx swap interval function found");
                _glXSwapIntervalSGI( vsync ? mVSyncInterval : 0 );
	    }
        }

        mContext->endCurrent();

        glXMakeCurrent (mGLSupport->getGLDisplay(), oldDrawable, oldContext);
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
            if (!mIsTopLevel)
            {
                XResizeWindow(mGLSupport->getXDisplay(), mWindow, width, height);
                XFlush(mGLSupport->getXDisplay());
            }

            mWidth = width;
            mHeight = height;

            for (ViewportList::iterator it = mViewportList.begin(); it != mViewportList.end(); ++it)
                (*it).second->_updateDimensions();
        }
    }

    //-------------------------------------------------------------------------------------------------//
    void GLXWindow::windowMovedOrResized()
    {
        if (mClosed || !mWindow)
            return;

        Display* xDisplay = mGLSupport->getXDisplay();
        uint width, height;
        queryRect(xDisplay, mWindow, mLeft, mTop, width, height, mIsTopLevel && !mIsFullScreen);
        resize(width, height);
    }

    //-------------------------------------------------------------------------------------------------//
    void GLXWindow::swapBuffers()
    {
        if (mClosed || mIsExternalGLControl)
            return;

        glXSwapBuffers(mGLSupport->getGLDisplay(), static_cast<GLXContext*>(mContext)->mDrawable);
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
            *static_cast<GLContext**>(pData) = mContext;
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
    PixelFormat GLXWindow::suggestPixelFormat() const
    {
        return mGLSupport->getContextProfile() == GLNativeSupport::CONTEXT_ES ? PF_BYTE_RGBA : PF_BYTE_RGB;
    }

    //-------------------------------------------------------------------------------------------------//
    void GLXWindow::switchFullScreen(bool fullscreen)
    {
        if (mGLSupport->mAtomFullScreen != None)
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
