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
#include "OgrePixelBox.h"

#include "OgreGLES2Prerequisites.h"
#include "OgreGLES2RenderSystem.h"

#include "OgreAndroidEGLSupport.h"
#include "OgreAndroidEGLWindow.h"
#include "OgreAndroidEGLContext.h"
#include "OgreGLES2ManagedResourceManager.h"
#include "OgreViewport.h"

#include <iostream>
#include <algorithm>
#include <climits>

namespace Ogre {
    AndroidEGLWindow::AndroidEGLWindow(AndroidEGLSupport *glsupport)
        : EGLWindow(glsupport),
          mMaxBufferSize(32),
          mMinBufferSize(32),
          mMaxDepthSize(16),
          mMaxStencilSize(0),
          mMSAA(0),
          mCSAA(0)
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
			// When using GPU rendering for Android UI the os creates a context in the main thread
			// Now we have 2 choices create OGRE in its own thread or set our context current before doing
			// anything else. I put this code here because this function called before any rendering is done.
			// Because the events for screen rotation / resizing did not worked on all devices it is the best way
			// to query the correct dimensions.
	        mContext->setCurrent(); 
            eglQuerySurface(mEglDisplay, mEglSurface, EGL_WIDTH, (EGLint*)&mWidth);
            eglQuerySurface(mEglDisplay, mEglSurface, EGL_HEIGHT, (EGLint*)&mHeight);
            
            // Notify viewports of resize
            ViewportList::iterator it = mViewportList.begin();
            while( it != mViewportList.end() )
                (*it++)->_updateDimensions();
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
                if (!eglContext)
                {
                    OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                                "currentGLContext was specified with no current GL context",
                                "EGLWindow::create");
                }

                mEglSurface = eglGetCurrentSurface(EGL_DRAW);
                mEglDisplay = eglGetCurrentDisplay();
            }
            
            
            if((opt = miscParams->find("externalWindowHandle")) != end)
            {
                mWindow = (ANativeWindow*)(Ogre::StringConverter::parseSizeT(opt->second));
            }
            
            if((opt = miscParams->find("androidConfig")) != end)
            {
                config = (AConfiguration*)(Ogre::StringConverter::parseSizeT(opt->second));
            }
            
            int ctxHandle = -1;
            if((opt = miscParams->find("externalGLContext")) != end)
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

            if((opt = miscParams->find("minColourBufferSize")) != end)
            {
                mMinBufferSize = Ogre::StringConverter::parseInt(opt->second);
                if (mMinBufferSize > mMaxBufferSize) mMinBufferSize = mMaxBufferSize;
            }

            if((opt = miscParams->find("MSAA")) != end)
            {
                mMSAA = Ogre::StringConverter::parseInt(opt->second);
            }
            
            if((opt = miscParams->find("CSAA")) != end)
            {
                mCSAA = Ogre::StringConverter::parseInt(opt->second);
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
        mContext->setCurrent();
               
        eglQuerySurface(mEglDisplay, mEglSurface, EGL_WIDTH, (EGLint*)&mWidth);
        eglQuerySurface(mEglDisplay, mEglSurface, EGL_HEIGHT, (EGLint*)&mHeight);
        EGL_CHECK_ERROR

        mActive = true;
        mVisible = true;
        mClosed = false;
    }

    void AndroidEGLWindow::_destroyInternalResources()
    {
        if(mClosed)
            return;
        
        mContext->setCurrent();
        
        GLES2RenderSystem::getResourceManager()->notifyOnContextLost();
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
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_BUFFER_SIZE, mMinBufferSize,
            EGL_DEPTH_SIZE, 16,
            EGL_NONE
        };
        
        int maxAttribs[] = {
            EGL_BUFFER_SIZE, mMaxBufferSize,
            EGL_DEPTH_SIZE, mMaxDepthSize,
            EGL_STENCIL_SIZE, mMaxStencilSize,
            EGL_NONE
        };

        bool bAASuccess = false;
        if (mCSAA)
        {
            try
            {
                int CSAAminAttribs[] = {
                    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                    EGL_BUFFER_SIZE, mMinBufferSize,
                    EGL_DEPTH_SIZE, 16,
                    EGL_COVERAGE_BUFFERS_NV, 1,
                    EGL_COVERAGE_SAMPLES_NV, mCSAA,
                    EGL_NONE
                };
                int CSAAmaxAttribs[] = {
                    EGL_BUFFER_SIZE, mMaxBufferSize,
                    EGL_DEPTH_SIZE, mMaxDepthSize,
                    EGL_STENCIL_SIZE, mMaxStencilSize,
                    EGL_COVERAGE_BUFFERS_NV, 1,
                    EGL_COVERAGE_SAMPLES_NV, mCSAA,
                    EGL_NONE
                };
                mEglConfig = mGLSupport->selectGLConfig(CSAAminAttribs, CSAAmaxAttribs);
                bAASuccess = true;
            }
            catch (Exception& e)
            {
                LogManager::getSingleton().logMessage("AndroidEGLWindow::_createInternalResources: setting CSAA failed");
            }
        }

        if (mMSAA && !bAASuccess)
        {
            try
            {
                int MSAAminAttribs[] = {
                    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                    EGL_BUFFER_SIZE, mMinBufferSize,
                    EGL_DEPTH_SIZE, 16,
                    EGL_SAMPLE_BUFFERS, 1,
                    EGL_SAMPLES, mMSAA,
                    EGL_NONE
                };
                int MSAAmaxAttribs[] = {
                    EGL_BUFFER_SIZE, mMaxBufferSize,
                    EGL_DEPTH_SIZE, mMaxDepthSize,
                    EGL_STENCIL_SIZE, mMaxStencilSize,
                    EGL_SAMPLE_BUFFERS, 1,
                    EGL_SAMPLES, mMSAA,
                    EGL_NONE
                };
                mEglConfig = mGLSupport->selectGLConfig(MSAAminAttribs, MSAAmaxAttribs);
                bAASuccess = true;
            }
            catch (Exception& e)
            {
                LogManager::getSingleton().logMessage("AndroidEGLWindow::_createInternalResources: setting MSAA failed");
            }
        }
        
        mEglDisplay = mGLSupport->getGLDisplay();
        if (!bAASuccess) mEglConfig = mGLSupport->selectGLConfig(minAttribs, maxAttribs);
        
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
        
        if(mContext)
        {
            mActive = true;
            mVisible = true;
            mClosed = false;
            
            mContext->_createInternalResources(mEglDisplay, mEglConfig, mEglSurface, NULL);

            static_cast<GLES2RenderSystem*>(Ogre::Root::getSingletonPtr()->getRenderSystem())->resetRenderer(this);
        }
    }
}
