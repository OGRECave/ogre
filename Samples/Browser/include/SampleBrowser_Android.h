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

#ifndef __SampleBrowser_Android_H__
#define __SampleBrowser_Android_H__

#include <android_native_app_glue.h>
#include <android/log.h>
#include <EGL/egl.h>
#include "OgrePlatform.h"
#include "SampleBrowser.h"
#include "Input.h"
#include "Android/OgreAndroidEGLWindow.h"

#ifdef INCLUDE_RTSHADER_SYSTEM
#   include "OgreRTShaderSystem.h"
#endif

#ifdef OGRE_STATIC_LIB
#   include "OgreStaticPluginLoader.h"
#endif

#if OGRE_PLATFORM != OGRE_PLATFORM_ANDROID
#   error This header is for use with Android only
#endif

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "Ogre", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "Ogre", __VA_ARGS__))

namespace OgreBites
{

    /*=============================================================================
     | Ogre Android bridge
     =============================================================================*/
    class OgreAndroidBridge
    {
    public:
        static void init(struct android_app* state)
        {
            state->onAppCmd = &OgreAndroidBridge::handleCmd;
            state->onInputEvent = &OgreAndroidBridge::handleInput;

            if(mInit)
                return;

            mRoot = new Ogre::Root();
#ifdef OGRE_STATIC_LIB
            mStaticPluginLoader = new Ogre::StaticPluginLoader();
            mStaticPluginLoader->load();
#endif
            mRoot->setRenderSystem(mRoot->getAvailableRenderers().at(0));
            mRoot->initialise(false);
            mInit = true;
        }

        static void shutdown()
        {
            if(!mInit)
                return;

            mInit = false;

            if(mBrowser)
            {
                mBrowser->closeApp();
                OGRE_DELETE mBrowser;
                mBrowser = NULL;
            }

            OGRE_DELETE mRoot;
            mRoot = NULL;
            mRenderWnd = NULL;

#ifdef OGRE_STATIC_LIB
            mStaticPluginLoader->unload();
            delete mStaticPluginLoader;
            mStaticPluginLoader = NULL;
#endif
        }

        static void injectKeyEvent(int action, int32_t keyCode)
        {
            if(keyCode != AKEYCODE_BACK)
                return;

            KeyboardEvent evt = {SDL_SCANCODE_ESCAPE, 0};

            if(action == AKEY_EVENT_ACTION_DOWN){
                mBrowser->keyPressed(evt);
            } else {
                mBrowser->keyReleased(evt);
            }
        }
        
        static void injectTouchEvent(int action, float x, float y, int pointerId = 0)
        {
            static TouchFingerEvent last = {0};
            TouchFingerEvent evt = {0};

            switch (action) {
            case AMOTION_EVENT_ACTION_DOWN:
                evt.type = SDL_FINGERDOWN;
                break;
            case AMOTION_EVENT_ACTION_UP:
                evt.type = SDL_FINGERUP;
                break;
            case AMOTION_EVENT_ACTION_MOVE:
                evt.type = SDL_FINGERMOTION;
                break;
            default:
                return;
            }

            evt.x = x / mBrowser->getRenderWindow()->getWidth();
            evt.y = y / mBrowser->getRenderWindow()->getHeight();

            if(evt.type == SDL_FINGERMOTION) {
                evt.dx = evt.x - last.x;
                evt.dy = evt.y - last.y;
            }

            last = evt;

            switch (evt.type) {
            case SDL_FINGERDOWN:
                // for finger down we have to move the pointer first
                mBrowser->touchMoved(evt);
                mBrowser->touchPressed(evt);
                break;
            case SDL_FINGERUP:
                mBrowser->touchReleased(evt);
                break;
            case SDL_FINGERMOTION:
                mBrowser->touchMoved(evt);
                break;
            }
        }
        
        static int32_t handleInput(struct android_app* app, AInputEvent* event) 
        {
            if (!mBrowser)
                return 0;

            if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION)
            {
                int32_t action = AMOTION_EVENT_ACTION_MASK & AMotionEvent_getAction(event);
                injectTouchEvent(action, AMotionEvent_getRawX(event, 0),
                                 AMotionEvent_getRawY(event, 0));
            }
            else
            {
                injectKeyEvent(AKeyEvent_getAction(event), AKeyEvent_getKeyCode(event));
            }

            return 1;
        }
        
        static void handleCmd(struct android_app* app, int32_t cmd)
        {
            switch (cmd) 
            {
                case APP_CMD_SAVE_STATE:
                break;
                case APP_CMD_INIT_WINDOW:
                    if (app->window && mRoot)
                    {
                        AConfiguration* config = AConfiguration_new();
                        AConfiguration_fromAssetManager(config, app->activity->assetManager);
                        
                        if (!mRenderWnd) 
                        {
                            Ogre::NameValuePairList opt;
                            opt["externalWindowHandle"] = Ogre::StringConverter::toString(reinterpret_cast<size_t>(app->window));
                            opt["androidConfig"] = Ogre::StringConverter::toString(reinterpret_cast<size_t>(config));
                            opt["preserveContext"] = "true"; //Optionally preserve the gl context, prevents reloading all resources, this is false by default
                            
                            mRenderWnd = Ogre::Root::getSingleton().createRenderWindow("OgreWindow", 0, 0, false, &opt);

                            if(!mBrowser)
                            {
                                mBrowser = OGRE_NEW SampleBrowser();
                                mBrowser->initAppForAndroid(mRenderWnd, app);
                                mBrowser->initApp();
                            }
                        }
                        else
                        {
                            static_cast<AndroidEGLWindow*>(mRenderWnd)->_createInternalResources(app->window, config);
                        }
                        
                        AConfiguration_delete(config);
                    }
                    break;
                case APP_CMD_TERM_WINDOW:
                    if(mRoot && mRenderWnd)
                        static_cast<AndroidEGLWindow*>(mRenderWnd)->_destroyInternalResources();
                    break;
                case APP_CMD_GAINED_FOCUS:
                    break;
                case APP_CMD_LOST_FOCUS:
                    break;
                case APP_CMD_CONFIG_CHANGED:
                    break;
            }
        }
        
        static void go(struct android_app* state)
        {
            int ident, events;
            struct android_poll_source* source;
            
            while (true)
            {
                while ((ident = ALooper_pollAll(0, NULL, &events, (void**)&source)) >= 0)
                {
                    if (source != NULL)
                        source->process(state, source);
                    
                    if (state->destroyRequested != 0)
                        return;
                }
                
                if(mRenderWnd != NULL && mRenderWnd->isActive())
                {
                    mRenderWnd->windowMovedOrResized();
                    mRoot->renderOneFrame();
                }
            }
        }
        
        static Ogre::RenderWindow* getRenderWindow()
        {
            return mRenderWnd;
        }
            
    private:
        static SampleBrowser* mBrowser;
        static Ogre::RenderWindow* mRenderWnd;
        static Ogre::Root* mRoot;
        static bool mInit;
        
#ifdef OGRE_STATIC_LIB
        static Ogre::StaticPluginLoader* mStaticPluginLoader;
#endif
    };
    
}

#endif
