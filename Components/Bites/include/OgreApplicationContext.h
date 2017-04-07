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
#include "OgreStaticPluginLoader.h"

#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
#include "OgreSGTechniqueResolverListener.h"
#endif // INCLUDE_RTSHADER_SYSTEM

// forward declarations
extern "C" struct SDL_Window;

namespace Ogre {
    class OverlaySystem;
}

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE |OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
#include "macUtils.h"
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
#   ifdef __OBJC__
#       import <UIKit/UIKit.h>
#   endif
#endif
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
#include <android/configuration.h>
#include <android/asset_manager.h>
#include <android/native_window.h>
#include <android/input.h>
#endif

#include "OgreInput.h"
#include "OgreWindowEventUtilities.h"

/** \addtogroup Optional Optional Components
*  @{
*/
/** \defgroup Bites Bites
* reusable utilities for rapid prototyping
*  @{
*/
namespace OgreBites
{
    /** 
    Base class responsible for setting up a common context for applications.
    Subclass to implement specific event callbacks.
    */
    class _OgreBitesExport ApplicationContext :
            public Ogre::FrameListener,
            public Ogre::WindowEventListener
    {
    public:
        explicit ApplicationContext(const Ogre::String& appName = OGRE_VERSION_NAME, bool grabInput = true);

        virtual ~ApplicationContext();

        Ogre::RenderWindow* getRenderWindow() const
        {
            return mWindow;
        }

        Ogre::Root* getRoot() const {
            return mRoot;
        }

        Ogre::OverlaySystem* getOverlaySystem() const {
            return mOverlaySystem;
        }

        /**
        This function initializes the render system and resources.
        */
        void initApp();

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
        void initAppForAndroid(AAssetManager* assetMgr, ANativeWindow* window);

        void _fireInputEventAndroid(AInputEvent* event, int wheel = 0);
#endif

        /**
        This function closes down the application - saves the configuration then
        shutdowns.
        */
        void closeApp();

        // callback interface copied from various listeners to be used by ApplicationContext
        virtual bool frameStarted(const Ogre::FrameEvent& evt) {
            pollEvents();
            return true;
        }
        virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
        virtual bool frameEnded(const Ogre::FrameEvent& evt) { return true; }
        virtual void windowMoved(Ogre::RenderWindow* rw) {}
        virtual void windowResized(Ogre::RenderWindow* rw) {}
        virtual bool windowClosing(Ogre::RenderWindow* rw) { return true; }
        virtual void windowClosed(Ogre::RenderWindow* rw) {}
        virtual void windowFocusChange(Ogre::RenderWindow* rw) {}

        /**
         * inspect the event and call one of the corresponding functions on the registered InputListener
         * @param event Input Event
         */
        void _fireInputEvent(const Event& event) const;

        /**
          Initialize the RT Shader system.
          */
        bool initialiseRTShaderSystem();

        /**
         * make the RTSS write out the generated shaders for caching and debugging
         *
         * by default all shaders are generated to system memory.
         * Must be called before loadResources
         * @param write
         */
        void setRTSSWriteShadersToDisk(bool write);

        /**
        Destroy the RT Shader system.
          */
        void destroyRTShaderSystem();

        /**
         Sets up the context after configuration.
         */
        virtual void setup();

        /**
        Creates the OGRE root.
        */
        virtual void createRoot();

        /**
        Configures the startup settings for OGRE. I use the config dialog here,
        but you can also restore from a config file. Note that this only happens
        when you start the context, and not when you reset it.
        */
        virtual bool oneTimeConfig();

        /**
        Sets up SDL input.
        */
        virtual void setupInput(bool grab);


        /**
        Finds context-wide resource groups. I load paths from a config file here,
        but you can choose your resource locations however you want.
        */
        virtual void locateResources();

        /**
        Loads context-wide resource groups. I chose here to simply initialise all
        groups, but you can fully load specific ones if you wish.
        */
        virtual void loadResources();

        /**
        Reconfigures the context. Attempts to preserve the current sample state.
        */
        virtual void reconfigure(const Ogre::String& renderer, Ogre::NameValuePairList& options);


        /**
        Cleans up and shuts down the context.
        */
        virtual void shutdown();

        /**
        poll for any events for the main window
        */
        void pollEvents();

        /**
        Creates dummy scene to allow rendering GUI in viewport.
          */
        void createDummyScene();

        /**
        Destroys dummy scene.
          */
        void destroyDummyScene();

        /**
         * enables the caching of compiled shaders to file
         *
         * also loads any existing cache
         */
        void enableShaderCache() const;

        /// attach input listener
        void addInputListener(InputListener* lis) {
            mInputListeners.insert(lis);
        }

        /// detach input listener
        void removeInputListener(InputListener* lis) {
            mInputListeners.erase(lis);
        }
    protected:

        /**
         * Create the render window to be used for this context here.
         * You must use SDL and not an auto-created window as SDL does not get the events
         * otherwise.
         */
        virtual Ogre::RenderWindow* createWindow();

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
        Ogre::DataStreamPtr openAPKFile(const Ogre::String& fileName);
        AAssetManager* mAAssetMgr;
        ANativeWindow* mAWindow;
        AConfiguration* mAConfig;
#endif

#if (OGRE_THREAD_PROVIDER == 3) && (OGRE_NO_TBB_SCHEDULER == 1)
        tbb::task_scheduler_init mTaskScheduler;
#endif

        Ogre::OverlaySystem* mOverlaySystem;  // Overlay system

        Ogre::FileSystemLayer* mFSLayer; // File system abstraction layer
        Ogre::Root* mRoot;              // OGRE root
        StaticPluginLoader mStaticPluginLoader;
        bool mGrabInput;
        bool mFirstRun;
        Ogre::String mNextRenderer;     // name of renderer used for next run
        Ogre::String mAppName;
        Ogre::NameValuePairList mLastSampleState;     // state of last sample
        Ogre::RenderWindow* mWindow;    // render window
        SDL_Window* mSDLWindow;

        std::set<InputListener*> mInputListeners;

#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
        Ogre::RTShader::ShaderGenerator*       mShaderGenerator; // The Shader generator instance.
        SGTechniqueResolverListener*       mMaterialMgrListener; // Shader generator material manager listener.
        Ogre::String                           mRTShaderLibPath;
#endif // INCLUDE_RTSHADER_SYSTEM
    };
}
/** @} */
/** @} */
#endif
