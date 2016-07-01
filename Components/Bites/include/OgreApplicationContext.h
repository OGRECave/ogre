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
#ifndef __ApplicationContext_H__
#define __ApplicationContext_H__

#include "OgreBitesPrerequisites.h"
#include "OgreBuildSettings.h"
#include "OgreLogManager.h"
#include "OgrePlugin.h"
#include "OgreFileSystemLayer.h"
#include "OgreFrameListener.h"

#ifdef INCLUDE_RTSHADER_SYSTEM
// Remove the comment below in order to make the RTSS use valid path for writing down the generated shaders.
// If cache path is not set - all shaders are generated to system memory.
//#define _RTSS_WRITE_SHADERS_TO_DISK
#include "OgreSGTechniqueResolverListener.h"
#endif // INCLUDE_RTSHADER_SYSTEM

extern "C" struct SDL_Window;

// Static plugins declaration section
// Note that every entry in here adds an extra header / library dependency
#ifdef OGRE_STATIC_LIB
#  ifdef OGRE_BUILD_RENDERSYSTEM_GL
#    define OGRE_STATIC_GL
#  endif
#  ifdef OGRE_BUILD_RENDERSYSTEM_GL3PLUS
#    define OGRE_STATIC_GL3Plus
#  endif
#  ifdef OGRE_BUILD_RENDERSYSTEM_GLES
#    define OGRE_STATIC_GLES
#  endif
#  ifdef OGRE_BUILD_RENDERSYSTEM_GLES2
#    define OGRE_STATIC_GLES2
#  endif
#  ifdef OGRE_BUILD_RENDERSYSTEM_D3D9
#     define OGRE_STATIC_Direct3D9
#  endif
// dx11 will only work on vista and above, so be careful about statically linking
#  ifdef OGRE_BUILD_RENDERSYSTEM_D3D11
#    define OGRE_STATIC_Direct3D11
#  endif

#  ifdef OGRE_BUILD_PLUGIN_BSP
#  define OGRE_STATIC_BSPSceneManager
#  endif
#  ifdef OGRE_BUILD_PLUGIN_PFX
#  define OGRE_STATIC_ParticleFX
#  endif
#  ifdef OGRE_BUILD_PLUGIN_CG
#  define OGRE_STATIC_CgProgramManager
#  endif

#  ifdef OGRE_USE_PCZ
#    ifdef OGRE_BUILD_PLUGIN_PCZ
#    define OGRE_STATIC_PCZSceneManager
#    define OGRE_STATIC_OctreeZone
#    endif
#  else
#    ifdef OGRE_BUILD_PLUGIN_OCTREE
#    define OGRE_STATIC_OctreeSceneManager
#  endif
#     endif
#  include "OgreStaticPluginLoader.h"
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
#include "macUtils.h"
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
#   ifdef __OBJC__
#       import <UIKit/UIKit.h>
#   endif
#endif
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
#include <android_native_app_glue.h>
#endif

#include "OgreInput.h"
#include "OgreWindowEventUtilities.h"

namespace OgreBites
{
    /*=============================================================================
    | Base class responsible for setting up a common context for samples.
    | May be subclassed for specific sample types (not specific samples).
    | Allows one sample to run at a time, while maintaining a sample queue.
    =============================================================================*/
    class _OgreBitesExport ApplicationContext :
            public Ogre::FrameListener,
            public Ogre::WindowEventListener
    {
    public:
        explicit ApplicationContext(const Ogre::String& appName = OGRE_VERSION_NAME, bool grabInput = true);

        virtual ~ApplicationContext();

        Ogre::RenderWindow* getRenderWindow()
        {
            return mWindow;
        }

        Ogre::Root* getRoot() {
            return mRoot;
        }

        /*-----------------------------------------------------------------------------
        | This function initializes the render system and resources.
        -----------------------------------------------------------------------------*/
        virtual void initApp();

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
        /*-----------------------------------------------------------------------------
         | init pre-created window for android
         -----------------------------------------------------------------------------*/
        void initAppForAndroid(Ogre::RenderWindow *window, struct android_app* app);
#endif

        /*-----------------------------------------------------------------------------
        | This function closes down the application - saves the configuration then
        | shutdowns.
        -----------------------------------------------------------------------------*/
        virtual void closeApp();

        // callback interface copied from various listeners to be used by ApplicationContext
        virtual bool frameStarted(const Ogre::FrameEvent& evt) { return true; }
        virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt) { return true; }
        virtual bool frameEnded(const Ogre::FrameEvent& evt) { return true; }
        virtual void windowMoved(Ogre::RenderWindow* rw) {}
        virtual void windowResized(Ogre::RenderWindow* rw) {}
        virtual bool windowClosing(Ogre::RenderWindow* rw) { return true; }
        virtual void windowClosed(Ogre::RenderWindow* rw) {}
        virtual void windowFocusChange(Ogre::RenderWindow* rw) {}
        virtual bool keyPressed(const KeyboardEvent& evt) { return true; }
        virtual bool keyReleased(const KeyboardEvent& evt) { return true; }
        virtual bool touchMoved(const TouchFingerEvent& evt) { return true; }
        virtual bool touchPressed(const TouchFingerEvent& evt) { return true; }
        virtual bool touchReleased(const TouchFingerEvent& evt) { return true; }
        virtual bool mouseMoved(const MouseMotionEvent& evt) { return true; }
        virtual bool mouseWheelRolled(const MouseWheelEvent& evt) { return true; }
        virtual bool mousePressed(const MouseButtonEvent& evt) { return true; }
        virtual bool mouseReleased(const MouseButtonEvent& evt) { return true; }

        /*-----------------------------------------------------------------------------
          | Initialize the RT Shader system.
          -----------------------------------------------------------------------------*/
        virtual bool initialiseRTShaderSystem();

        /*-----------------------------------------------------------------------------
        | Destroy the RT Shader system.
          -----------------------------------------------------------------------------*/
        virtual void destroyRTShaderSystem();

        /*-----------------------------------------------------------------------------
         | Sets up the context after configuration.
         -----------------------------------------------------------------------------*/
        virtual void setup();

        /*-----------------------------------------------------------------------------
        | Creates the OGRE root.
        -----------------------------------------------------------------------------*/
        virtual void createRoot();

        /*-----------------------------------------------------------------------------
        | Configures the startup settings for OGRE. I use the config dialog here,
        | but you can also restore from a config file. Note that this only happens
        | when you start the context, and not when you reset it.
        -----------------------------------------------------------------------------*/
        virtual bool oneTimeConfig();

        /*-----------------------------------------------------------------------------
        | Sets up SDL input.
        -----------------------------------------------------------------------------*/
        virtual void setupInput(bool grab);


        /*-----------------------------------------------------------------------------
        | Finds context-wide resource groups. I load paths from a config file here,
        | but you can choose your resource locations however you want.
        -----------------------------------------------------------------------------*/
        virtual void locateResources();

        /*-----------------------------------------------------------------------------
        | Loads context-wide resource groups. I chose here to simply initialise all
        | groups, but you can fully load specific ones if you wish.
        -----------------------------------------------------------------------------*/
        virtual void loadResources();

        /*-----------------------------------------------------------------------------
        | Reconfigures the context. Attempts to preserve the current sample state.
        -----------------------------------------------------------------------------*/
        virtual void reconfigure(const Ogre::String& renderer, Ogre::NameValuePairList& options);


        /*-----------------------------------------------------------------------------
        | Cleans up and shuts down the context.
        -----------------------------------------------------------------------------*/
        virtual void shutdown();

        /*-----------------------------------------------------------------------------
        | Captures input device states.
        -----------------------------------------------------------------------------*/
        virtual void captureInputDevices();
    protected:

        /**
         * Create the render window to be used for this context here.
         * You must use SDL and not an auto-created window as SDL does not get the events
         * otherwise.
         */
        virtual Ogre::RenderWindow* createWindow();

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
        Ogre::DataStreamPtr openAPKFile(const Ogre::String& fileName);
        AAssetManager* mAssetMgr;       // Android asset manager to access files inside apk
#endif

#if (OGRE_THREAD_PROVIDER == 3) && (OGRE_NO_TBB_SCHEDULER == 1)
        tbb::task_scheduler_init mTaskScheduler;
#endif

        Ogre::FileSystemLayer* mFSLayer; // File system abstraction layer
        Ogre::Root* mRoot;              // OGRE root
#ifdef OGRE_STATIC_LIB
        Ogre::StaticPluginLoader mStaticPluginLoader;
#endif
        bool mGrabInput;
        bool mFirstRun;
        Ogre::String mNextRenderer;     // name of renderer used for next run
        Ogre::String mAppName;
        Ogre::NameValuePairList mLastSampleState;     // state of last sample
        Ogre::RenderWindow* mWindow;    // render window
        SDL_Window* mSDLWindow;

#ifdef INCLUDE_RTSHADER_SYSTEM
        Ogre::RTShader::ShaderGenerator*       mShaderGenerator;                       // The Shader generator instance.
        SGTechniqueResolverListener*       mMaterialMgrListener;           // Shader generator material manager listener.
#endif // INCLUDE_RTSHADER_SYSTEM
    };
}

#endif
