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

#include "OgreGLRenderSystemCommon.h"

#include "OgreEmscriptenEGLSupport.h"
#include "OgreEmscriptenEGLWindow.h"
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
        // already handled by resize
        emscripten_set_fullscreenchange_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, (void*)this, 1, &EmscriptenEGLWindow::fullscreenCallback);
        emscripten_set_webglcontextlost_callback("#canvas", (void*)this, 1, &EmscriptenEGLWindow::contextLostCallback);
        emscripten_set_webglcontextrestored_callback("#canvas", (void*)this, 1, &EmscriptenEGLWindow::contextRestoredCallback);
        emscripten_set_resize_callback("#canvas", (void*)this, 1, &EmscriptenEGLWindow::canvasWindowResized);
    }

    EmscriptenEGLWindow::~EmscriptenEGLWindow()
    {
        emscripten_set_fullscreenchange_callback("#canvas", NULL, 0, NULL);
        emscripten_set_resize_callback("#canvas", NULL, 0, NULL);
        emscripten_set_webglcontextlost_callback("#canvas", NULL, 0, NULL);
        emscripten_set_webglcontextrestored_callback("#canvas", NULL, 0, NULL);
    }

    void EmscriptenEGLWindow::resize(uint width, uint height)
    {
        mWidth = width;
        mHeight = height;
        

        EMSCRIPTEN_RESULT result = emscripten_set_canvas_element_size(mCanvasSelector.c_str(), width, height);
        // This is a workaroud for issue: https://github.com/emscripten-core/emscripten/issues/3283.
        // The setTimeout of 0 will ensure that this code is runs on the next JSEventLoop.
        EM_ASM(setTimeout(function(){var canvas = document.getElementById('canvas'); canvas.width = $0; canvas.height = $1;}, 0), width, height);
        
        if(result < 0)
        {
           OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
             "Unexpected failure at emscripten_set_canvas_element_size with selector=" + mCanvasSelector,
             "EmscriptenEGLWindow::resize");
        }
        
        
        LogManager::getSingleton().logMessage("EmscriptenEGLWindow::resize "+mCanvasSelector+" w:" + Ogre::StringConverter::toString(mWidth) + " h:" + Ogre::StringConverter::toString(mHeight));
        
        // Notify viewports of resize
        ViewportList::iterator it = mViewportList.begin();
        while( it != mViewportList.end() )
            (*it++).second->_updateDimensions();
    }

    void EmscriptenEGLWindow::windowMovedOrResized()
    {
        if(!mActive) return;

        int w, h;
        emscripten_get_canvas_element_size(mCanvasSelector.c_str(), &w, &h);
        mWidth = w;
        mHeight = h;

        // Notify viewports of resize
        ViewportList::iterator it = mViewportList.begin();
        while( it != mViewportList.end() )
            (*it++).second->_updateDimensions();
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
        mCanvasSelector = "#canvas";
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

            if((opt = miscParams->find("FSAA")) != end)
            {
                mMSAA = Ogre::StringConverter::parseInt(opt->second);
            }
            
            if((opt = miscParams->find("CSAA")) != end)
            {
                mCSAA = Ogre::StringConverter::parseInt(opt->second);
            }

            if ((opt = miscParams->find("externalWindowHandle")) != end ||
                (opt = miscParams->find("parentWindowHandle")) != end)
            {
                mCanvasSelector = opt->second;
            }
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
        
        if (!mEglConfig)
        {
            _notifySurfaceCreated(&mWindow);
            mHwGamma = false;
        }
        
        mContext = createEGLContext(eglContext);
        mContext->setCurrent();
        EMSCRIPTEN_RESULT result = emscripten_set_canvas_element_size(mCanvasSelector.c_str(), width, height);
        
        if(result < 0)
        {
           OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
             "Unexpected failure at emscripten_set_canvas_element_size with selector=" + mCanvasSelector ,
             "EmscriptenEGLWindow::create");
        }
        
        mOldWidth = width;
        mOldHeight = height;
        if(fullScreen)
            switchFullScreen(true);
        
        EGL_CHECK_ERROR

        mActive = true;
        mVisible = true;
        mClosed = false;
    }

    int EmscriptenEGLWindow::contextLostCallback(int eventType, const void *reserved, void *userData)
    {
        Ogre::EmscriptenEGLWindow* thiz = static_cast<Ogre::EmscriptenEGLWindow*>(userData);
        thiz->_notifySurfaceDestroyed();
        return 0;
    }
    
    int EmscriptenEGLWindow::contextRestoredCallback(int eventType, const void *reserved, void *userData)
    {
        Ogre::EmscriptenEGLWindow* thiz = static_cast<Ogre::EmscriptenEGLWindow*>(userData);
        thiz->_notifySurfaceCreated(&thiz->mWindow);
        return 0;
    }
    
    void EmscriptenEGLWindow::_notifySurfaceDestroyed()
    {
        mContext->setCurrent();
        
        static_cast<GLRenderSystemCommon*>(Root::getSingleton().getRenderSystem())->notifyOnContextLost();
        static_cast<EGLContext*>(mContext)->_destroyInternalResources();
        
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
    
    void EmscriptenEGLWindow::_notifySurfaceCreated(void* window, void*)
    {
        mWindow = *reinterpret_cast<EGLNativeWindowType*>(window);
        
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
            
            static_cast<EGLContext*>(mContext)->_createInternalResources(mEglDisplay, mEglConfig, mEglSurface, nullptr);
            
            static_cast<GLRenderSystemCommon*>(Ogre::Root::getSingleton().getRenderSystem())->resetRenderer(this);
        }
    }

    void EmscriptenEGLWindow::swapBuffers()
    {
        // Not used on emscripten
    }

    EM_BOOL EmscriptenEGLWindow::canvasWindowResized(int eventType, const EmscriptenUiEvent *event, void *userData)
    {
        EmscriptenEGLWindow* thiz = static_cast<EmscriptenEGLWindow*>(userData);
        thiz->windowMovedOrResized();
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
