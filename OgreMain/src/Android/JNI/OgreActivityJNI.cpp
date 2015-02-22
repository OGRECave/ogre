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
#include <EGL/egl.h>
#include <android/api-level.h>
#include <android/native_window_jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include "OgrePlatform.h"
#include "OgreRoot.h"
#include "OgreRenderWindow.h"
#include "OgreArchiveManager.h"
#include "OgreViewport.h"

#include "Android/OgreAndroidEGLWindow.h"
#include "Android/OgreAPKFileSystemArchive.h"
#include "Android/OgreAPKZipArchive.h"
#include "Compositor/OgreCompositorManager2.h"

#ifdef OGRE_BUILD_PLUGIN_OCTREE
#   include "OgreOctreePlugin.h"
#endif

#ifdef OGRE_BUILD_PLUGIN_PFX
#   include "OgreParticleFXPlugin.h"
#endif

#ifdef OGRE_BUILD_COMPONENT_OVERLAY
#   include "OgreOverlaySystem.h"
#endif

#include "OgreConfigFile.h"

#ifdef OGRE_BUILD_RENDERSYSTEM_GLES2
#   include "OgreGLES2Plugin.h"
#   define GLESRS GLES2Plugin
#else
#   include "OgreGLESPlugin.h"
#   define GLESRS GLESPlugin
#endif

using namespace Ogre;

namespace {
bool gInit = false;
Ogre::Root* gRoot = NULL;
Ogre::RenderWindow* gRenderWnd = NULL;

#ifdef OGRE_BUILD_PLUGIN_OCTREE
Ogre::OctreePlugin* gOctreePlugin = NULL;
#endif

#ifdef OGRE_BUILD_PLUGIN_PFX
Ogre::ParticleFXPlugin* gParticleFXPlugin = NULL;
#endif

#ifdef OGRE_BUILD_COMPONENT_OVERLAY
Ogre::OverlaySystem* gOverlaySystem = NULL;
#endif

Ogre::GLESRS* gGLESPlugin = NULL;

static Ogre::CompositorManager2* gCompositorManager = NULL;
static Ogre::SceneManager* pSceneMgr = NULL;
static Ogre::Camera* pCamera = NULL;
static JavaVM* gVM = NULL;
extern "C" 
{
    JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved);
    JNIEXPORT void JNICALL  Java_org_ogre3d_android_OgreActivityJNI_create(JNIEnv * env, jobject obj, jobject assetManager);
    JNIEXPORT void JNICALL Java_org_ogre3d_android_OgreActivityJNI_destroy(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_org_ogre3d_android_OgreActivityJNI_initWindow(JNIEnv * env, jobject obj,  jobject surface);
    JNIEXPORT void JNICALL Java_org_ogre3d_android_OgreActivityJNI_termWindow(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_org_ogre3d_android_OgreActivityJNI_renderOneFrame(JNIEnv * env, jobject obj);
}

jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
    gVM = vm;
    return JNI_VERSION_1_4;
}

void Java_org_ogre3d_android_OgreActivityJNI_create(JNIEnv * env, jobject obj, jobject assetManager)
{
    if(gInit)
        return;

    gRoot = new Ogre::Root();

    gGLESPlugin = OGRE_NEW GLESRS();
    gRoot->installPlugin(gGLESPlugin);

#ifdef OGRE_BUILD_PLUGIN_OCTREE
    gOctreePlugin = OGRE_NEW OctreePlugin();
    gRoot->installPlugin(gOctreePlugin);
#endif

#ifdef OGRE_BUILD_PLUGIN_PFX
    gParticleFXPlugin = OGRE_NEW ParticleFXPlugin();
    gRoot->installPlugin(gParticleFXPlugin);
#endif

#ifdef OGRE_BUILD_COMPONENT_OVERLAY
    gOverlaySystem = OGRE_NEW OverlaySystem();
#endif
    
    gRoot->setRenderSystem(gRoot->getAvailableRenderers().at(0));
    gRoot->initialise(false);
    gInit = true;

    AAssetManager* assetMgr = AAssetManager_fromJava(env, assetManager);
    if (assetMgr)
    {
        ArchiveManager::getSingleton().addArchiveFactory( new APKFileSystemArchiveFactory(assetMgr) );
        ArchiveManager::getSingleton().addArchiveFactory( new APKZipArchiveFactory(assetMgr) );
    }
}

void Java_org_ogre3d_android_OgreActivityJNI_destroy(JNIEnv * env, jobject obj)
{
    if(!gInit)
        return;

    gInit = false;

#ifdef OGRE_BUILD_COMPONENT_OVERLAY
    OGRE_DELETE gOverlaySystem;
    gOverlaySystem = NULL;
#endif

    OGRE_DELETE gRoot;
    gRoot = NULL;
    gRenderWnd = NULL;

#ifdef OGRE_BUILD_PLUGIN_PFX
    OGRE_DELETE gParticleFXPlugin;
    gParticleFXPlugin = NULL;
#endif

#ifdef OGRE_BUILD_PLUGIN_OCTREE
    OGRE_DELETE gOctreePlugin;
    gOctreePlugin = NULL;
#endif
        
        OGRE_DELETE gGLESPlugin;
        gGLESPlugin = NULL;
        
        gCompositorManager->removeAllWorkspaces();
        gCompositorManager->removeAllWorkspaceDefinitions();
        gCompositorManager->removeAllNodeDefinitions();
        gCompositorManager->removeAllShadowNodeDefinitions();
    }
    

void Java_org_ogre3d_android_OgreActivityJNI_initWindow(JNIEnv * env, jobject obj,  jobject surface)
{
    if (!surface) {
        return;
    }

    ANativeWindow* nativeWnd = ANativeWindow_fromSurface(env, surface);

    if (!nativeWnd || !gRoot) {
        return;
    }

    if (!gRenderWnd) {
        Ogre::NameValuePairList opt;
        opt["externalWindowHandle"] = Ogre::StringConverter::toString(reinterpret_cast<size_t>(nativeWnd));
        gRenderWnd = Ogre::Root::getSingleton().createRenderWindow("OgreWindow", 0, 0, false, &opt);

        if(gCompositorManager == NULL)
        {
            pSceneMgr = gRoot->createSceneManager(Ogre::ST_GENERIC);
            pCamera = pSceneMgr->createCamera("MyCam");

            gCompositorManager = mRoot->getCompositorManager2();
            if( !gCompositorManager->hasWorkspaceDefinition( "SampleBrowserWorkspace" ) )
            {
                gCompositorManager->createBasicWorkspaceDef( "SampleBrowserWorkspace",
                                                              Ogre::ColourValue( 1.0f, 0.0f, 0.0f ),
                                                              Ogre::IdString() );
            }
            compositorManager->addWorkspace( pSceneMgr, gRenderWnd, pCamera,
                                            "SampleBrowserWorkspace", true );
        }
    } else {
        static_cast<Ogre::AndroidEGLWindow*>(gRenderWnd)->_createInternalResources(nativeWnd, NULL);
    }
}

void Java_org_ogre3d_android_OgreActivityJNI_termWindow(JNIEnv * env, jobject obj)
{
    if(gRoot && gRenderWnd)
    {
        static_cast<Ogre::AndroidEGLWindow*>(gRenderWnd)->_destroyInternalResources();
    }
}

void Java_org_ogre3d_android_OgreActivityJNI_renderOneFrame(JNIEnv * env, jobject obj)
{
    if(gRenderWnd && gRenderWnd->isActive())
    {
        try
        {
            if(gVM->AttachCurrentThread(&env, NULL) < 0)
                return;

            gRenderWnd->windowMovedOrResized();
            gRoot->renderOneFrame();

            //gVM->DetachCurrentThread();
        }catch(Ogre::RenderingAPIException& ex) {}
    }
}
