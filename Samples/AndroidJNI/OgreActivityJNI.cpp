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

#include <jni.h>
#include <android/native_window_jni.h>
#include <android/asset_manager_jni.h>

#include "OgrePlatform.h"
#include "OgreRoot.h"
#include "OgreRenderWindow.h"
#include "OgreViewport.h"

#include "OgreApplicationContext.h"
#include "OgreGLRenderSystemCommon.h"

using namespace Ogre;

namespace {
OgreBites::ApplicationContext gAppContext;

Ogre::SceneManager* pSceneMgr = NULL;
Ogre::Camera* pCamera = NULL;
JavaVM* gVM = NULL;
}

// enable JNI calling conventions for functions defined here
extern "C" 
{
    JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved);
    JNIEXPORT void JNICALL Java_org_ogre3d_android_OgreActivityJNI_shutdown(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_org_ogre3d_android_OgreActivityJNI_init(JNIEnv * env, jobject obj, jobject assetManager, jobject surface);
    JNIEXPORT void JNICALL Java_org_ogre3d_android_OgreActivityJNI_termWindow(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_org_ogre3d_android_OgreActivityJNI_renderOneFrame(JNIEnv * env, jobject obj);
}

jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
    gVM = vm;
    return JNI_VERSION_1_4;
}

void Java_org_ogre3d_android_OgreActivityJNI_shutdown(JNIEnv * env, jobject obj)
{
    if(!gAppContext.getRoot())
        return;

    gAppContext.shutdown();
}


void Java_org_ogre3d_android_OgreActivityJNI_init(JNIEnv * env, jobject obj, jobject assetManager, jobject surface)
{
    if (!surface) {
        return;
    }

    ANativeWindow* nativeWnd = ANativeWindow_fromSurface(env, surface);

    if (!nativeWnd) {
        return;
    }

    AAssetManager* assetMgr = AAssetManager_fromJava(env, assetManager);

    if (!gAppContext.getRenderWindow()) {
        gAppContext.initAppForAndroid(assetMgr, nativeWnd);

        if (!pSceneMgr) {
            pSceneMgr = gAppContext.getRoot()->createSceneManager(Ogre::ST_GENERIC);
            pCamera = pSceneMgr->createCamera("MyCam");

            Ogre::Viewport* vp = gAppContext.getRenderWindow()->addViewport(pCamera);
            vp->setBackgroundColour(Ogre::ColourValue(1,0,0));
        }
    } else {
        GLRenderSystemCommon::_createInternalResources(gAppContext.getRenderWindow(), nativeWnd, NULL);
    }
}

void Java_org_ogre3d_android_OgreActivityJNI_termWindow(JNIEnv * env, jobject obj)
{
    if(gAppContext.getRenderWindow())
    {
        GLRenderSystemCommon::_destroyInternalResources(gAppContext.getRenderWindow());
    }
}

void Java_org_ogre3d_android_OgreActivityJNI_renderOneFrame(JNIEnv * env, jobject obj)
{
    if(gAppContext.getRenderWindow() && gAppContext.getRenderWindow()->isActive())
    {
        try
        {
            if(gVM->AttachCurrentThread(&env, NULL) < 0)
                return;

            gAppContext.getRenderWindow()->windowMovedOrResized();
            gAppContext.getRoot()->renderOneFrame();

            //gVM->DetachCurrentThread();
        }catch(RenderingAPIException& ex) {}
    }
}
