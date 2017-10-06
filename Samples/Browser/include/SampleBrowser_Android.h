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

#include "OgrePlatform.h"
#include "SampleBrowser.h"
#include "OgreInput.h"
#include "OgreGLRenderSystemCommon.h"

#include "gestureDetector.h"

#if OGRE_PLATFORM != OGRE_PLATFORM_ANDROID
#   error This header is for use with Android only
#endif

namespace OgreBites
{

    /*=============================================================================
     | Ogre Android bridge
     =============================================================================*/
    struct OgreAndroidBridge
    {
        static void init(struct android_app* state)
        {
            state->onAppCmd = &OgreAndroidBridge::handleCmd;
            state->onInputEvent = &OgreAndroidBridge::handleInput;
        }

        static void shutdown()
        {
            if(!mBrowser.getRoot())
                return;

            mBrowser.closeApp();
        }
        
        static int32_t handleInput(struct android_app* app, AInputEvent* event) 
        {
            if (!mBrowser.getRenderWindow())
                return 0;

            static float len = 0;

            int wheel = 0; // overrides other events if mPinchGesture triggers

            if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION)
            {

                ndk_helper::GESTURE_STATE s = mPinchGesture.Detect(event);

                if(s & ndk_helper::GESTURE_STATE_START) {
                    ndk_helper::Vec2 p1, p2;
                    mPinchGesture.GetPointers(p1, p2);
                    len = (p1 - p2).length();
                } else if (s & ndk_helper::GESTURE_STATE_MOVE) {
                    ndk_helper::Vec2 p1, p2;
                    mPinchGesture.GetPointers(p1, p2);
                    float curr = (p1 - p2).length();

                    if(fabs(curr - len)/mBrowser.getRenderWindow()->getWidth() > 0.01) {
                        wheel = (curr - len) > 0 ? 1 : -1;
                        len = curr;
                    }
                }
            }

            mBrowser._fireInputEventAndroid(event, wheel);

            return 1;
        }
        
        static void handleCmd(struct android_app* app, int32_t cmd)
        {
            switch (cmd) 
            {
                case APP_CMD_SAVE_STATE:
                    break;
                case APP_CMD_INIT_WINDOW:
                    if (app->window)
                    {
                        if (!mBrowser.getRenderWindow())
                        {
                            mBrowser.initAppForAndroid(app->activity->assetManager, app->window);
                        }
                        else
                        {
                            AConfiguration* config = AConfiguration_new();
                            AConfiguration_fromAssetManager(config, app->activity->assetManager);
                            mBrowser.getRenderWindow()->_notifySurfaceCreated(app->window, config);
                            AConfiguration_delete(config);
                        }
                    }
                    break;
                case APP_CMD_TERM_WINDOW:
                    if(mBrowser.getRenderWindow())
                        mBrowser.getRenderWindow()->_notifySurfaceDestroyed();
                    break;
                case APP_CMD_RESUME:
                    if(mBrowser.getRenderWindow())
                        mBrowser.getRenderWindow()->setVisible(true);
                    break;
                case APP_CMD_PAUSE:
                    if(mBrowser.getRenderWindow())
                        mBrowser.getRenderWindow()->setVisible(false);
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
                
                if(mBrowser.getRenderWindow() && mBrowser.getRenderWindow()->isActive())
                {
                    mBrowser.getRoot()->renderOneFrame();
                }
            }
        }

    private:
        static SampleBrowser mBrowser;
        static ndk_helper::PinchDetector mPinchGesture;
    };
    
}

#endif
