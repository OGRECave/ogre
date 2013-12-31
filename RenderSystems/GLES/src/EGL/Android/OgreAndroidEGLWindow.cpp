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

#include "OgreRoot.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreWindowEventUtilities.h"

#include "OgreGLESPrerequisites.h"
#include "OgreGLESRenderSystem.h"

#include "OgreAndroidEGLSupport.h"
#include "OgreAndroidEGLWindow.h"
#include "OgreAndroidEGLContext.h"
#include "OgreAndroidResourceManager.h"

#include <iostream>
#include <algorithm>
#include <climits>

namespace Ogre {
	AndroidEGLWindow::AndroidEGLWindow(AndroidEGLSupport *glsupport)
		: EGLWindow(glsupport),
		  mMaxBufferSize(32),
		  mMaxDepthSize(24),
		  mMaxStencilSize(8)
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
	}

	void AndroidEGLWindow::reposition( int left, int top )
	{
	}

	void AndroidEGLWindow::resize(uint width, uint height)
	{
	}

	void AndroidEGLWindow::windowMovedOrResized()
	{
        if(mActive)
        {
            eglQuerySurface(mEglDisplay, mEglSurface, EGL_WIDTH, (EGLint*)&mWidth);
			EGL_CHECK_ERROR
			
            eglQuerySurface(mEglDisplay, mEglSurface, EGL_HEIGHT, (EGLint*)&mHeight);
            EGL_CHECK_ERROR
            
            // Notify viewports of resize
            ViewportList::iterator it = mViewportList.begin();
            while( it != mViewportList.end() )
                (*it++).second->_updateDimensions();
        }
	}
	
    void AndroidEGLWindow::switchFullScreen(bool fullscreen)
    {
    
    }
    
    void AndroidEGLWindow::create(const String& name, uint width, uint height,
                               bool fullScreen, const NameValuePairList *miscParams)
    {
		mName = name;
        mWidth = width;
        mHeight = height;
        mLeft = 0;
        mTop = 0;
        mIsFullScreen = fullScreen;
        void* eglContext = NULL;
        AConfiguration* config = NULL;
        
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
            
            if((opt = miscParams->find("androidConfig")) != end)
            {
                config = (AConfiguration*)(Ogre::StringConverter::parseInt(opt->second));
            }
            
            int ctxHandle = -1;
            if((miscParams->find("externalGLContext")) != end)
            {
                mIsExternalGLControl = true;
                ctxHandle = Ogre::StringConverter::parseInt(opt->second);
            }
			
			if((opt = miscParams->find("maxColourBufferSize")) != end)
            {
                mMaxBufferSize = Ogre::StringConverter::parseInt(opt->second);
            }
			
			if((opt = miscParams->find("maxDepthBufferSize")) != end)
            {
                mMaxDepthSize = Ogre::StringConverter::parseInt(opt->second);
            }
			
			if((opt = miscParams->find("maxStencilBufferSize")) != end)
            {
                mMaxStencilSize = Ogre::StringConverter::parseInt(opt->second);
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

            _createInternalResources(mWindow, config);
            mHwGamma = false;
        }
        
        mContext = createEGLContext();
        
		mActive = true;
		mVisible = true;
		mClosed = false;
	}

    void AndroidEGLWindow::_destroyInternalResources()
    {
        GLESRenderSystem::getResourceManager()->notifyOnContextLost();
        mContext->_destroyInternalResources();
        
        eglDestroySurface(mEglDisplay, mEglSurface);
        EGL_CHECK_ERROR
        
        eglTerminate(mEglDisplay);
        EGL_CHECK_ERROR
        
        mEglDisplay = 0;
        mEglSurface = 0;
        
        mActive = false;
		mVisible = false;
        mClosed = true;
    }
    
    void AndroidEGLWindow::_createInternalResources(NativeWindowType window, AConfiguration* config)
    {
        mWindow = window;
        
        int minAttribs[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT,
            EGL_BUFFER_SIZE, 16,
            EGL_DEPTH_SIZE, 16,
            EGL_NONE
        };
        
        int maxAttribs[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT,
			EGL_BUFFER_SIZE, mMaxBufferSize,
            EGL_DEPTH_SIZE, mMaxDepthSize,
            EGL_STENCIL_SIZE, mMaxStencilSize,
            EGL_NONE
        };
        
        mEglDisplay = mGLSupport->getGLDisplay();
        mEglConfig = mGLSupport->selectGLConfig(minAttribs, maxAttribs);
        
        EGLint format;
        eglGetConfigAttrib(mEglDisplay, mEglConfig, EGL_NATIVE_VISUAL_ID, &format);
        EGL_CHECK_ERROR
        
        ANativeWindow_setBuffersGeometry(mWindow, 0, 0, format);
        
        mEglSurface = createSurfaceFromWindow(mEglDisplay, mWindow);
        
        if(config)
        {
            bool isLandscape = (int)AConfiguration_getOrientation(config) == 2;
            mGLSupport->setConfigOption("Orientation", isLandscape ? "Landscape" : "Portrait");
        }
        
        eglQuerySurface(mEglDisplay, mEglSurface, EGL_WIDTH, (EGLint*)&mWidth);
        eglQuerySurface(mEglDisplay, mEglSurface, EGL_HEIGHT, (EGLint*)&mHeight);
        EGL_CHECK_ERROR
        
        if(mContext)
        {
            mActive = true;
            mVisible = true;
            mClosed = false;
            
            mContext->_createInternalResources(mEglDisplay, mEglConfig, mEglSurface, NULL);
            mContext->setCurrent();
            
            windowMovedOrResized();
            static_cast<GLESRenderSystem*>(Ogre::Root::getSingletonPtr()->getRenderSystem())->resetRenderer(this);
        }
    }
}