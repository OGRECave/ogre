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

#include "OgreGLES2Prerequisites.h"
#include "OgreGLES2RenderSystem.h"
#include "OgreGLES2ManagedResourceManager.h"

#include "OgreEmscriptenEGLSupport.h"
#include "OgreEmscriptenEGLWindow.h"
#include "OgreEmscriptenEGLContext.h"
#include "OgreViewport.h"

#include <iostream>
#include <algorithm>
#include <climits>

namespace Ogre {
    EmscriptenEGLWindow::EmscriptenEGLWindow(EmscriptenEGLSupport *glsupport)
        : EGLWindow(glsupport),
          mMaxBufferSize(32),
          mMinBufferSize(16),
          mMaxDepthSize(16),
          mMaxStencilSize(0),
          mMSAA(0),
          mCSAA(0)
    {
        emscripten_set_fullscreenchange_callback(NULL, (void*)this, 1, &EmscriptenEGLWindow::fullscreenCallback);
        emscripten_set_webglcontextlost_callback(NULL, (void*)this, 1, &EmscriptenEGLWindow::contextLostCallback);
        emscripten_set_webglcontextrestored_callback(NULL, (void*)this, 1, &EmscriptenEGLWindow::contextRestoredCallback);
        emscripten_set_resize_callback(NULL, (void*)this, 1, &EmscriptenEGLWindow::canvasWindowResized);
    }

    EmscriptenEGLWindow::~EmscriptenEGLWindow()
    {
        emscripten_set_fullscreenchange_callback(NULL, NULL, 0, NULL);
        emscripten_set_resize_callback(NULL, NULL, 0, NULL);
        emscripten_set_webglcontextlost_callback(NULL, NULL, 0, NULL);
        emscripten_set_webglcontextrestored_callback(NULL, NULL, 0, NULL);
    }

    EGLContext* EmscriptenEGLWindow::createEGLContext() const
    {
        return new EmscriptenEGLContext(mEglDisplay, mGLSupport, mEglConfig, mEglSurface);
    }

    void EmscriptenEGLWindow::getLeftAndTopFromNativeWindow( int & left, int & top, uint width, uint height )
    {
        // We don't have a native window, so return 0.
        left = top = 0;
    }

    void EmscriptenEGLWindow::initNativeCreatedWindow(const NameValuePairList *miscParams)
    {
    }

    void EmscriptenEGLWindow::createNativeWindow( int &left, int &top, uint &width, uint &height, String &title )
    {
    }

    void EmscriptenEGLWindow::reposition( int left, int top )
    {
    }

    void EmscriptenEGLWindow::resize(uint width, uint height)
    {
        mWidth = width;
        mHeight = height;
        
        emscripten_set_canvas_size(mWidth, mHeight);
        LogManager::getSingleton().logMessage("EmscriptenEGLWindow::resize w:" + Ogre::StringConverter::toString(mWidth) + " h:" + Ogre::StringConverter::toString(mHeight));
        
        windowMovedOrResized();
    }

    void EmscriptenEGLWindow::windowMovedOrResized()
    {
        if(mActive)
        {   
            // Notify viewports of resize
            ViewportList::iterator it = mViewportList.begin();
            while( it != mViewportList.end() )
                (*it++)->_updateDimensions();
        }
    }
    
    void EmscriptenEGLWindow::switchFullScreen(bool fullscreen)
    {
        if(fullscreen)
            emscripten_request_fullscreen(NULL, 1);
        else
            emscripten_exit_fullscreen();
    }
    
    void EmscriptenEGLWindow::create(const String& name, uint width, uint height,
                               bool fullScreen, const NameValuePairList *miscParams)
    {
        mName = name;
        mWidth = width;
        mHeight = height;
        mLeft = 0;
        mTop = 0;
        void* eglContext = nullptr;
         
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
            _createInternalResources(mWindow);
            mHwGamma = false;
        }
        
        mContext = createEGLContext();
        mContext->setCurrent();
        emscripten_set_canvas_size(width, height);
        mOldWidth = width;
        mOldHeight = height;
        switchFullScreen(fullScreen);
        
        EGL_CHECK_ERROR

        mActive = true;
        mVisible = true;
        mClosed = false;
    }

    int EmscriptenEGLWindow::contextLostCallback(int eventType, const void *reserved, void *userData)
    {
        Ogre::EmscriptenEGLWindow* thiz = static_cast<Ogre::EmscriptenEGLWindow*>(userData);
        thiz->_destroyInternalResources();
        return 0;
    }
    
    int EmscriptenEGLWindow::contextRestoredCallback(int eventType, const void *reserved, void *userData)
    {
        Ogre::EmscriptenEGLWindow* thiz = static_cast<Ogre::EmscriptenEGLWindow*>(userData);
        thiz->_createInternalResources(thiz->mWindow);
        return 0;
    }
    
    void EmscriptenEGLWindow::_destroyInternalResources()
    {
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
    
    void EmscriptenEGLWindow::_createInternalResources(NativeWindowType window)
    {
        mWindow = window;
        
        int minAttribs[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_BUFFER_SIZE, mMinBufferSize,
            EGL_DEPTH_SIZE, 16,
            EGL_NONE
        };
        
        int maxAttribs[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
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
                    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
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
                LogManager::getSingleton().logMessage("EmscriptenEGLWindow::_createInternalResources: setting CSAA failed");
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
                    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
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
                LogManager::getSingleton().logMessage("EmscriptenEGLWindow::_createInternalResources: setting MSAA failed");
            }
        }
        
        mEglDisplay = mGLSupport->getGLDisplay();
        if (!bAASuccess) mEglConfig = mGLSupport->selectGLConfig(minAttribs, maxAttribs);
        
        mEglSurface = createSurfaceFromWindow(mEglDisplay, mWindow);
        
        if(mContext)
        {
            mActive = true;
            mVisible = true;
            mClosed = false;
            
            mContext->_createInternalResources(mEglDisplay, mEglConfig, mEglSurface, nullptr);
            
            static_cast<GLES2RenderSystem*>(Ogre::Root::getSingletonPtr()->getRenderSystem())->resetRenderer(this);
        }
    }

    void EmscriptenEGLWindow::swapBuffers()
    {
        RenderWindow::swapBuffers();
        // Not used on emscripten
        RenderWindow::swapBuffers();
    }

    EM_BOOL EmscriptenEGLWindow::canvasWindowResized(int eventType, const EmscriptenUiEvent *uiEvent, void *userData)
    {
        EmscriptenEGLWindow* thiz = static_cast<EmscriptenEGLWindow*>(userData);
        //thiz->resize(event->documentBodyClientWidth, event->documentBodyClientHeight);
        return EMSCRIPTEN_RESULT_SUCCESS;
    }
    
    EM_BOOL EmscriptenEGLWindow::fullscreenCallback(int eventType, const EmscriptenFullscreenChangeEvent* event, void* userData)
    {
        EmscriptenEGLWindow* thiz = static_cast<EmscriptenEGLWindow*>(userData);
        
        thiz->mIsFullScreen = event->isFullscreen;
        if (event->isFullscreen)
        {
            thiz->mOldWidth = thiz->mWidth;
            thiz->mOldHeight = thiz->mHeight;
            thiz->resize(event->screenWidth, event->screenHeight);
        }
        else
        {
            thiz->resize(thiz->mOldWidth, thiz->mOldHeight);
        }
    
        thiz->windowMovedOrResized();
        return EMSCRIPTEN_RESULT_SUCCESS;
    }
}
