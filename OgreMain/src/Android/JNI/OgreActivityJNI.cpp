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
#include "Android/OgreAndroidEGLWindow.h"
#include "Android/OgreAPKFileSystemArchive.h"
#include "Android/OgreAPKZipArchive.h"

#ifdef OGRE_BUILD_PLUGIN_OCTREE
#	include "OgreOctreePlugin.h"
#endif

#ifdef OGRE_BUILD_PLUGIN_PFX
#	include "OgreParticleFXPlugin.h"
#endif

#ifdef OGRE_BUILD_COMPONENT_OVERLAY
#	include "OgreOverlaySystem.h"
#endif

#include "OgreConfigFile.h"

#ifdef OGRE_BUILD_RENDERSYSTEM_GLES2
#	include "OgreGLES2Plugin.h"
#	define GLESRS GLES2Plugin
#else
#	include "OgreGLESPlugin.h"
#	define GLESRS GLESPlugin
#endif

using namespace Ogre;

static bool gInit = false;
static Ogre::Root* gRoot = NULL;
static Ogre::RenderWindow* gRenderWnd = NULL;

#ifdef OGRE_BUILD_PLUGIN_OCTREE
static Ogre::OctreePlugin* gOctreePlugin = NULL;
#endif

#ifdef OGRE_BUILD_PLUGIN_PFX
static Ogre::ParticleFXPlugin* gParticleFXPlugin = NULL;
#endif

#ifdef OGRE_BUILD_COMPONENT_OVERLAY
static Ogre::OverlaySystem* gOverlaySystem = NULL; 
#endif

static Ogre::GLESRS* gGLESPlugin = NULL;

static Ogre::SceneManager* pSceneMgr = NULL;
static Ogre::Camera* pCamera = NULL;
static JavaVM* gVM = NULL;
extern "C" 
{
	JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) 
	{
		gVM = vm;
		return JNI_VERSION_1_4;
	}

	JNIEXPORT void JNICALL 	Java_org_ogre3d_android_OgreActivityJNI_create(JNIEnv * env, jobject obj, jobject assetManager)
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
	
	JNIEXPORT void JNICALL Java_org_ogre3d_android_OgreActivityJNI_destroy(JNIEnv * env, jobject obj)
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
	}
	

    JNIEXPORT void JNICALL Java_org_ogre3d_android_OgreActivityJNI_initWindow(JNIEnv * env, jobject obj,  jobject surface)
	{
		if(surface)
		{
			ANativeWindow* nativeWnd = ANativeWindow_fromSurface(env, surface);
			if (nativeWnd && gRoot)
			{
				if (!gRenderWnd) 
				{
					Ogre::NameValuePairList opt;
					opt["externalWindowHandle"] = Ogre::StringConverter::toString((int)nativeWnd);
					gRenderWnd = Ogre::Root::getSingleton().createRenderWindow("OgreWindow", 0, 0, false, &opt);
					
					
					if(pSceneMgr == NULL)
					{
						pSceneMgr = gRoot->createSceneManager(Ogre::ST_GENERIC);
						pCamera = pSceneMgr->createCamera("MyCam");
		
						Ogre::Viewport* vp = gRenderWnd->addViewport(pCamera);
						vp->setBackgroundColour(Ogre::ColourValue(1,0,0));
					}						
				}
				else
				{
					static_cast<Ogre::AndroidEGLWindow*>(gRenderWnd)->_createInternalResources(nativeWnd, NULL);
				}                        
			}
		}
	}
	
    JNIEXPORT void JNICALL Java_org_ogre3d_android_OgreActivityJNI_termWindow(JNIEnv * env, jobject obj)
	{
		if(gRoot && gRenderWnd)
		{
			static_cast<Ogre::AndroidEGLWindow*>(gRenderWnd)->_destroyInternalResources();
		}
	}
	
	JNIEXPORT void JNICALL Java_org_ogre3d_android_OgreActivityJNI_renderOneFrame(JNIEnv * env, jobject obj)
	{
		if(gRenderWnd != NULL && gRenderWnd->isActive())
		{
			try
			{
				if(gVM->AttachCurrentThread(&env, NULL) < 0) 					
					return;
				
				gRenderWnd->windowMovedOrResized();
				gRoot->renderOneFrame();
				
				//gVM->DetachCurrentThread();				
			}catch(Ogre::RenderingAPIException ex) {}
		}
	}
};
