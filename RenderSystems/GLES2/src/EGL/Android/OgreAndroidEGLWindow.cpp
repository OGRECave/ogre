/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2012 Torus Knot Software Ltd

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

#include "OgreRoot.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreWindowEventUtilities.h"

#include "OgreGLES2Prerequisites.h"
#include "OgreGLES2RenderSystem.h"

#include "OgreAndroidEGLSupport.h"
#include "OgreAndroidEGLWindow.h"
#include "OgreAndroidEGLContext.h"

#include <iostream>
#include <algorithm>
#include <climits>

namespace Ogre {
	AndroidEGLWindow::AndroidEGLWindow(AndroidEGLSupport *glsupport)
		: EGLWindow(glsupport)
	{
	}

	AndroidEGLWindow::~AndroidEGLWindow()
	{
	}

	EGLContext* AndroidEGLWindow::createEGLContext() const
	{
		return new AndroidEGLContext(mEglDisplay, mGLSupport, mEglConfig, mEglSurface);
	}

	void AndroidEGLWindow::getLeftAndTopFromNativeWindow( int & left, int & top, uint width, uint height )
	{
		// We don't have a native window.... but I think all android windows are origined
		left = top = 0;
	}

	void AndroidEGLWindow::initNativeCreatedWindow(const NameValuePairList *miscParams)
	{
	}

	void AndroidEGLWindow::createNativeWindow( int &left, int &top, uint &width, uint &height, String &title )
	{
		LogManager::getSingleton().logMessage("\tcreateNativeWindow called");
	}

	void AndroidEGLWindow::reposition( int left, int top )
	{
		LogManager::getSingleton().logMessage("\treposition called");
	}

	void AndroidEGLWindow::resize(uint width, uint height)
	{
		LogManager::getSingleton().logMessage("\tresize called");
	}

	void AndroidEGLWindow::windowMovedOrResized()
	{
		LogManager::getSingleton().logMessage("\twindowMovedOrResized called");
	}
	
    void AndroidEGLWindow::switchFullScreen(bool fullscreen)
    {
    
    }
    
    void AndroidEGLWindow::create(const String& name, uint width, uint height,
                               bool fullScreen, const NameValuePairList *miscParams)
    {
        LogManager::getSingleton().logMessage("\tcreate called");
				
		mName = name;
        mWidth = width;
        mHeight = height;
        mLeft = 0;
        mTop = 0;
        mIsFullScreen = fullScreen;
        void* eglContext = NULL;
        
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
            
            
            if((opt = miscParams->find("externalWindowHandle")) != end)
            {
                mWindow = (ANativeWindow*)(Ogre::StringConverter::parseInt(opt->second));
            }
            
            int ctxHandle = -1;
            if((miscParams->find("externalGLContext")) != end)
            {
                mIsExternalGLControl = true;
                ctxHandle = Ogre::StringConverter::parseInt(opt->second);
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
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                EGL_BLUE_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_RED_SIZE, 8,
                EGL_NONE
            };
            
            int maxAttribs[] = {
                EGL_NONE
            };
            
            mEglDisplay = mGLSupport->getGLDisplay();
            mEglConfig = mGLSupport->selectGLConfig(minAttribs, maxAttribs);
            
            EGLint format;
            eglGetConfigAttrib(mEglDisplay, mEglConfig, EGL_NATIVE_VISUAL_ID, &format);
            ANativeWindow_setBuffersGeometry(mWindow, mWidth, mHeight, format);
            
            mEglSurface = createSurfaceFromWindow(mEglDisplay, mWindow);
            
            mHwGamma = false;
        }
        
        mContext = createEGLContext();
        
		mActive = true;
		mVisible = true;
		mClosed = false;
	}



}