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
#ifndef __SampleContext_H__
#define __SampleContext_H__

#include "OgreBuildSettings.h"
#include "OgreLogManager.h"
#include "OgrePlugin.h"
#include "OgreFileSystemLayer.h"
#include "OgreFrameListener.h"
#include "OgreOverlaySystem.h"

#ifdef HAVE_SDL
#include <SDL_video.h>
#endif

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

#include "Sample.h"

#include "Input.h"

namespace OgreBites
{
    /*=============================================================================
    | Base class responsible for setting up a common context for samples.
    | May be subclassed for specific sample types (not specific samples).
    | Allows one sample to run at a time, while maintaining a sample queue.
    =============================================================================*/
    class SampleContext :
        public Ogre::FrameListener,
        public Ogre::WindowEventListener
    {
    public:

        SampleContext()
#if (OGRE_THREAD_PROVIDER == 3) && (OGRE_NO_TBB_SCHEDULER == 1)
            : mTaskScheduler(tbb::task_scheduler_init::deferred)
#endif
        {
            mFSLayer = OGRE_NEW_T(Ogre::FileSystemLayer, Ogre::MEMCATEGORY_GENERAL)(OGRE_VERSION_NAME);
            mRoot = 0;
            mWindow = 0;
#ifdef HAVE_SDL
            mSDLWindow = NULL;
#endif
            mCurrentSample = 0;
            mOverlaySystem = 0;
            mSamplePaused = false;
            mFirstRun = true;
            mLastRun = false;
            mLastSample = 0;
        }

        virtual ~SampleContext() 
        {
            OGRE_DELETE_T(mFSLayer, FileSystemLayer, Ogre::MEMCATEGORY_GENERAL);
        }

        virtual Ogre::RenderWindow* getRenderWindow()
        {
            return mWindow;
        }

        virtual Sample* getCurrentSample()
        {
            return mCurrentSample;
        }

        /*-----------------------------------------------------------------------------
        | Quits the current sample and starts a new one.
        -----------------------------------------------------------------------------*/
        virtual void runSample(Sample* s)
        {
#if OGRE_PROFILING
            Ogre::Profiler* prof = Ogre::Profiler::getSingletonPtr();
            if (prof)
                prof->setEnabled(false);
#endif

            if (mCurrentSample)
            {
                mCurrentSample->_shutdown();    // quit current sample
                mSamplePaused = false;          // don't pause the next sample
            }

            mWindow->removeAllViewports();                  // wipe viewports

            if (s)
            {
                // retrieve sample's required plugins and currently installed plugins
                Ogre::Root::PluginInstanceList ip = mRoot->getInstalledPlugins();
                Ogre::StringVector rp = s->getRequiredPlugins();

                for (Ogre::StringVector::iterator j = rp.begin(); j != rp.end(); j++)
                {
                    bool found = false;
                    // try to find the required plugin in the current installed plugins
                    for (Ogre::Root::PluginInstanceList::iterator k = ip.begin(); k != ip.end(); k++)
                    {
                        if ((*k)->getName() == *j)
                        {
                            found = true;
                            break;
                        }
                    }
                    if (!found)  // throw an exception if a plugin is not found
                    {
                        Ogre::String desc = "Sample requires plugin: " + *j;
                        Ogre::String src = "SampleContext::runSample";
                        OGRE_EXCEPT(Ogre::Exception::ERR_NOT_IMPLEMENTED, desc, src);
                    }
                }

                // throw an exception if samples requires the use of another renderer
                Ogre::String rrs = s->getRequiredRenderSystem();
                if (!rrs.empty() && rrs != mRoot->getRenderSystem()->getName())
                {
                    Ogre::String desc = "Sample only runs with renderer: " + rrs;
                    Ogre::String src = "SampleContext::runSample";
                    OGRE_EXCEPT(Ogre::Exception::ERR_INVALID_STATE, desc, src);
                }

                // test system capabilities against sample requirements
                s->testCapabilities(mRoot->getRenderSystem()->getCapabilities());

                s->_setup(mWindow, mFSLayer, mOverlaySystem);   // start new sample
            }
#if OGRE_PROFILING
            if (prof)
                prof->setEnabled(true);
#endif

            mCurrentSample = s;
        }

        /*-----------------------------------------------------------------------------
        | This function initializes the render system and resources.
        -----------------------------------------------------------------------------*/
        virtual void initApp( Sample* initialSample = 0 )
        {
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
            createRoot();

            if (!oneTimeConfig()) return;

            if (!mFirstRun) mRoot->setRenderSystem(mRoot->getRenderSystemByName(mNextRenderer));

            setup();

            if (!mFirstRun) recoverLastSample();
            else if (initialSample) runSample(initialSample);

            mRoot->saveConfig();

            Ogre::Root::getSingleton().getRenderSystem()->_initRenderTargets();

            // Clear event times
            Ogre::Root::getSingleton().clearEventTimes();
#elif OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
            createRoot();

            setup();

            if (!mFirstRun) recoverLastSample();
            else if (initialSample) runSample(initialSample);

            //mRoot->saveConfig();

            Ogre::Root::getSingleton().getRenderSystem()->_initRenderTargets();

            // Clear event times
            Ogre::Root::getSingleton().clearEventTimes();

#else
            createRoot();
#if OGRE_PLATFORM == OGRE_PLATFORM_NACL
            mNextRenderer = mRoot->getAvailableRenderers()[0]->getName();
#else
            if (!oneTimeConfig()) return;
#endif

            // if the context was reconfigured, set requested renderer
            if (!mFirstRun) mRoot->setRenderSystem(mRoot->getRenderSystemByName(mNextRenderer));

            setup();

            // restore the last sample if there was one or, if not, start initial sample
            if (!mFirstRun) recoverLastSample();
            else if (initialSample) runSample(initialSample);
#endif
        }


        /*-----------------------------------------------------------------------------
        | This function closes down the application - saves the configuration then 
        | shutdowns.
        -----------------------------------------------------------------------------*/
        virtual void closeApp()
        {
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
            shutdown();
#else
            mRoot->saveConfig();
            shutdown();
            if (mRoot)
            {
                OGRE_DELETE mOverlaySystem;
                OGRE_DELETE mRoot;
            }

#ifdef OGRE_STATIC_LIB
            mStaticPluginLoader.unload();
#endif
#endif
#if (OGRE_THREAD_PROVIDER == 3) && (OGRE_NO_TBB_SCHEDULER == 1)
            if (mTaskScheduler.is_active())
                mTaskScheduler.terminate();
#endif

#ifdef HAVE_SDL
            SDL_DestroyWindow(mSDLWindow);
            mSDLWindow = 0;
#endif
        }

        /*-----------------------------------------------------------------------------
        | This function encapsulates the entire lifetime of the context.
        -----------------------------------------------------------------------------*/
#if OGRE_PLATFORM != OGRE_PLATFORM_NACL
        virtual void go(Sample* initialSample = 0)
        {
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS || ((OGRE_PLATFORM == OGRE_PLATFORM_APPLE) && __LP64__)
            createRoot();

            if (!oneTimeConfig()) return;

            if (!mFirstRun) mRoot->setRenderSystem(mRoot->getRenderSystemByName(mNextRenderer));

            mLastRun = true;  // assume this is our last run

            setup();

            if (!mFirstRun) recoverLastSample();
            else if (initialSample) runSample(initialSample);

            mRoot->saveConfig();
#else
            while (!mLastRun)
            {
                mLastRun = true;  // assume this is our last run

                initApp(initialSample);
                loadStartUpSample();
        
                if (mRoot->getRenderSystem() != NULL)
                {
                    mRoot->startRendering();    // start the render loop
                }

                closeApp();

                mFirstRun = false;
            }
#endif
        }
#endif

        virtual void loadStartUpSample() {}
        
        virtual bool isCurrentSamplePaused()
        {
            if (mCurrentSample) return mSamplePaused;
            return false;
        }

        virtual void pauseCurrentSample()
        {
            if (mCurrentSample && !mSamplePaused)
            {
                mSamplePaused = true;
                mCurrentSample->paused();
            }
        }

        virtual void unpauseCurrentSample()
        {
            if (mCurrentSample && mSamplePaused)
            {
                mSamplePaused = false;
                mCurrentSample->unpaused();
            }
        }
            
        /*-----------------------------------------------------------------------------
        | Processes frame started events.
        -----------------------------------------------------------------------------*/
        virtual bool frameStarted(const Ogre::FrameEvent& evt)
        {
            captureInputDevices();      // capture input

            // manually call sample callback to ensure correct order
            return (mCurrentSample && !mSamplePaused) ? mCurrentSample->frameStarted(evt) : true;
        }
            
        /*-----------------------------------------------------------------------------
        | Processes rendering queued events.
        -----------------------------------------------------------------------------*/
        virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt)
        {
            // manually call sample callback to ensure correct order
            return (mCurrentSample && !mSamplePaused) ? mCurrentSample->frameRenderingQueued(evt) : true;
        }
            
        /*-----------------------------------------------------------------------------
        | Processes frame ended events.
        -----------------------------------------------------------------------------*/
        virtual bool frameEnded(const Ogre::FrameEvent& evt)
        {
            // manually call sample callback to ensure correct order
            if (mCurrentSample && !mSamplePaused && !mCurrentSample->frameEnded(evt)) return false;
            // quit if window was closed
            if (mWindow->isClosed()) return false;
            // go into idle mode if current sample has ended
            if (mCurrentSample && mCurrentSample->isDone()) runSample(0);

            return true;
        }

        /*-----------------------------------------------------------------------------
        | Processes window size change event. Adjusts mouse's region to match that
        | of the window. You could also override this method to prevent resizing.
        -----------------------------------------------------------------------------*/
        virtual void windowResized(Ogre::RenderWindow* rw)
        {
            // manually call sample callback to ensure correct order
            if (mCurrentSample && !mSamplePaused) mCurrentSample->windowResized(rw);
        }

        // window event callbacks which manually call their respective sample callbacks to ensure correct order

        virtual void windowMoved(Ogre::RenderWindow* rw)
        {
            if (mCurrentSample && !mSamplePaused) mCurrentSample->windowMoved(rw);
        }

        virtual bool windowClosing(Ogre::RenderWindow* rw)
        {
            if (mCurrentSample && !mSamplePaused) return mCurrentSample->windowClosing(rw);
            return true;
        }

        virtual void windowClosed(Ogre::RenderWindow* rw)
        {
            if (mCurrentSample && !mSamplePaused) mCurrentSample->windowClosed(rw);
        }

        virtual void windowFocusChange(Ogre::RenderWindow* rw)
        {
            if (mCurrentSample && !mSamplePaused) mCurrentSample->windowFocusChange(rw);
        }

        // keyboard and mouse callbacks which manually call their respective sample callbacks to ensure correct order

        virtual bool keyPressed(const KeyboardEvent& evt)
        {
            if (mCurrentSample && !mSamplePaused) return mCurrentSample->keyPressed(evt);
            return true;
        }

        virtual bool keyReleased(const KeyboardEvent& evt)
        {
            if (mCurrentSample && !mSamplePaused) return mCurrentSample->keyReleased(evt);
            return true;
        }

        void transformInputState(TouchFingerEvent &state)
        {
#if 0
            int w = mWindow->getViewport(0)->getActualWidth();
            int h = mWindow->getViewport(0)->getActualHeight();
            int absX = state.X.abs;
            int absY = state.Y.abs;
            int relX = state.X.rel;
            int relY = state.Y.rel;

            // as OIS work in windowing system units we need to convert them to pixels
            float scale = mWindow->getViewPointToPixelScale();
            if(scale != 1.0f)
            {
                absX = (int)(absX * scale);
                absY = (int)(absY * scale);
                relX = (int)(relX * scale);
                relY = (int)(relY * scale);
            }

            // determine required orientation
            Ogre::OrientationMode orientation = Ogre::OR_DEGREE_0;
#    if (OGRE_NO_VIEWPORT_ORIENTATIONMODE == 0)
            orientation = mWindow->getViewport(0)->getOrientationMode();
#    elif (OGRE_NO_VIEWPORT_ORIENTATIONMODE == 1) && (OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS)
            UIInterfaceOrientation interfaceOrientation = [UIApplication sharedApplication].statusBarOrientation;
            switch (interfaceOrientation)
            {
            case UIInterfaceOrientationPortrait:           break;
            case UIInterfaceOrientationLandscapeLeft:      orientation = Ogre::OR_DEGREE_90;  break;
            case UIInterfaceOrientationPortraitUpsideDown: orientation = Ogre::OR_DEGREE_180; break;
            case UIInterfaceOrientationLandscapeRight:     orientation = Ogre::OR_DEGREE_270; break;
            }
#    endif

            // apply changes
            switch (orientation)
            {
            case Ogre::OR_DEGREE_0:
                state.X.abs = absX;
                state.Y.abs = absY;
                state.X.rel = relX;
                state.Y.rel = relY;
                state.width = w;
                state.height = h;
                break;
            case Ogre::OR_DEGREE_90:
                state.X.abs = w - absY;
                state.Y.abs = absX;
                state.X.rel = -relY;
                state.Y.rel = relX;
                state.width = h;
                state.height = w;
                break;
            case Ogre::OR_DEGREE_180:
                state.X.abs = w - absX;
                state.Y.abs = h - absY;
                state.X.rel = -relX;
                state.Y.rel = -relY;
                state.width = w;
                state.height = h;
                break;
            case Ogre::OR_DEGREE_270:
                state.X.abs = absY;
                state.Y.abs = h - absX;
                state.X.rel = relY;
                state.Y.rel = -relX;
                state.width = h;
                state.height = w;
                break;
            }
#endif
        }

        virtual bool touchMoved(const TouchFingerEvent& evt)
        {
            if (mCurrentSample && !mSamplePaused)
                return mCurrentSample->touchMoved(evt);
            return true;
        }

        virtual bool mouseMoved(const MouseMotionEvent& evt)
        {
            // Miniscule mouse movements are still considered hovering.
            // if (evt.xrel > 100000 || evt.yrel > 100000)
            // {
            //     mTimeSinceMouseMoved = 0;
            // }

            if (mCurrentSample && !mSamplePaused)
                return mCurrentSample->mouseMoved(evt);
            return true;
        }

        virtual bool touchPressed(const TouchFingerEvent& evt)
        {
            if (mCurrentSample && !mSamplePaused)
                return mCurrentSample->touchPressed(evt);
            return true;
        }

        virtual bool mousePressed(const MouseButtonEvent& evt)
        {
            if (mCurrentSample && !mSamplePaused)
                return mCurrentSample->mousePressed(evt);
            return true;
        }

        virtual bool touchReleased(const TouchFingerEvent& evt)
        {
            if (mCurrentSample && !mSamplePaused)
                return mCurrentSample->touchReleased(evt);
            return true;
        }

        virtual bool mouseReleased(const MouseButtonEvent& evt)
        {
            if (mCurrentSample && !mSamplePaused)
                return mCurrentSample->mouseReleased(evt);
            return true;
        }

#if (OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS) || (OGRE_PLATFORM == OGRE_PLATFORM_ANDROID)
        //FIXME: Handle mouse wheel wheel events on mobile devices.
        // virtual bool touchReleased(const SDL_TouchFingerEvent& evt)
        // {
        //     if (mCurrentSample && !mSamplePaused)
        //         return mCurrentSample->touchReleased(evt);
        //     return true;
        // }
#endif
        virtual bool mouseWheelRolled(const MouseWheelEvent& evt)
        {
            if (mCurrentSample && !mSamplePaused)
                return mCurrentSample->mouseWheelRolled(evt);
            return true;
        }

        bool isFirstRun() { return mFirstRun; }
        void setFirstRun(bool flag) { mFirstRun = flag; }
        bool isLastRun() { return mLastRun; }
        void setLastRun(bool flag) { mLastRun = flag; }
    protected:

        /*-----------------------------------------------------------------------------
         | Sets up the context after configuration.
         -----------------------------------------------------------------------------*/
        virtual void setup()
        {
#ifdef HAVE_SDL
            SDL_Init(0);
            SDL_InitSubSystem(SDL_INIT_VIDEO);
#endif

            mWindow = createWindow();
            setupInput();
            locateResources();
            loadResources();
            
            Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
            
            // adds context as listener to process context-level (above the sample level) events
            mRoot->addFrameListener(this);
#if OGRE_PLATFORM != OGRE_PLATFORM_ANDROID
            Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);
#endif
        }
        
        /*-----------------------------------------------------------------------------
        | Creates the OGRE root.
        -----------------------------------------------------------------------------*/
        virtual void createRoot()
        {
#if (OGRE_THREAD_PROVIDER == 3) && (OGRE_NO_TBB_SCHEDULER == 1)
            mTaskScheduler.initialize(OGRE_THREAD_HARDWARE_CONCURRENCY);
#endif
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
            mRoot = Ogre::Root::getSingletonPtr();
#else
            Ogre::String pluginsPath = Ogre::BLANKSTRING;
#   ifndef OGRE_STATIC_LIB
            pluginsPath = mFSLayer->getConfigFilePath("plugins.cfg");
#   endif
            mRoot = OGRE_NEW Ogre::Root(pluginsPath, mFSLayer->getWritablePath("ogre.cfg"), 
                mFSLayer->getWritablePath("ogre.log"));
            
#   ifdef OGRE_STATIC_LIB
            mStaticPluginLoader.load();
#   endif
#endif
            mOverlaySystem = OGRE_NEW Ogre::OverlaySystem();
        }

        /*-----------------------------------------------------------------------------
        | Configures the startup settings for OGRE. I use the config dialog here,
        | but you can also restore from a config file. Note that this only happens
        | when you start the context, and not when you reset it.
        -----------------------------------------------------------------------------*/
        virtual bool oneTimeConfig()
        {
            return mRoot->showConfigDialog();
            // return mRoot->restoreConfig();
        }

        /**
         * Create the render window to be used for this context here.
         * You must use SDL and not an auto-created window as SDL does not get the events
         * otherwise.
         */
        virtual Ogre::RenderWindow* createWindow()
        {
            return NULL;
        }

        /*-----------------------------------------------------------------------------
        | Sets up SDL input.
        -----------------------------------------------------------------------------*/
        virtual void setupInput(bool nograb = false)
        {
#ifdef HAVE_SDL
            if (SDL_InitSubSystem(SDL_INIT_EVENTS) != 0)
            {
                OGRE_EXCEPT(Ogre::Exception::ERR_INVALID_STATE,
                            Ogre::String("Could not initialize SDL2 input: ")
                            + SDL_GetError(),
                            "SampleContext::setupInput");
            }
            
            SDL_ShowCursor(SDL_FALSE);
            
            SDL_bool grab = SDL_bool(!nograb);

            SDL_SetWindowGrab(mSDLWindow, grab);
            SDL_SetRelativeMouseMode(grab);
#endif
        }


        /*-----------------------------------------------------------------------------
        | Finds context-wide resource groups. I load paths from a config file here,
        | but you can choose your resource locations however you want.
        -----------------------------------------------------------------------------*/
        virtual void locateResources()
        {
#if OGRE_PLATFORM == OGRE_PLATFORM_NACL
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation("Essential.zip", "EmbeddedZip", "Essential");
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation("Popular.zip", "EmbeddedZip", "Popular");
#else
            // load resource paths from config file
            Ogre::ConfigFile cf;
#   if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
            cf.load(openAPKFile(mFSLayer->getConfigFilePath("resources.cfg")));
#   else
            cf.load(mFSLayer->getConfigFilePath("resources.cfg"));
#   endif
            Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
            Ogre::String sec, type, arch;

            // go through all specified resource groups
            while (seci.hasMoreElements())
            {
                sec = seci.peekNextKey();
                Ogre::ConfigFile::SettingsMultiMap* settings = seci.getNext();
                Ogre::ConfigFile::SettingsMultiMap::iterator i;

                // go through all resource paths
                for (i = settings->begin(); i != settings->end(); i++)
                {
                    type = i->first;
                    arch = i->second;

                    #if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
                    // OS X does not set the working directory relative to the app,
                    // In order to make things portable on OS X we need to provide
                    // the loading with it's own bundle path location
                    if (!Ogre::StringUtil::startsWith(arch, "/", false)) // only adjust relative dirs
                        arch = Ogre::String(Ogre::macBundlePath() + "/" + arch);
                    #endif
                    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(arch, type, sec);
                }
            }


        const Ogre::ResourceGroupManager::LocationList genLocs = Ogre::ResourceGroupManager::getSingleton().getResourceLocationList("General");
        arch = genLocs.front()->archive->getName();
#   if OGRE_PLATFORM != OGRE_PLATFORM_ANDROID
#       if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
            arch = Ogre::macBundlePath() + "/Contents/Resources/Media";
#       elif OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
            arch = Ogre::macBundlePath() + "/Media";
#       else       
            arch = Ogre::StringUtil::replaceAll(arch, "Media/../../Tests/Media", "");  
            arch = Ogre::StringUtil::replaceAll(arch, "media/../../Tests/Media", ""); 
#       endif
            type = "FileSystem";
            sec = "Popular";

#		ifdef OGRE_BUILD_PLUGIN_CG
			bool use_HLSL_Cg_shared = true;
#		else
			bool use_HLSL_Cg_shared = Ogre::GpuProgramManager::getSingleton().isSyntaxSupported("hlsl");
#		endif

            // Add locations for supported shader languages
            if(Ogre::GpuProgramManager::getSingleton().isSyntaxSupported("glsles"))
            {
                Ogre::ResourceGroupManager::getSingleton().addResourceLocation(arch + "/materials/programs/GLSLES", type, sec);
            }
            else if(Ogre::GpuProgramManager::getSingleton().isSyntaxSupported("glsl"))
            {
                if(Ogre::GpuProgramManager::getSingleton().isSyntaxSupported("glsl150"))
                {
                    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(arch + "/materials/programs/GLSL150", type, sec);
                }
                else
                {
                    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(arch + "/materials/programs/GLSL", type, sec);
                }

                if(Ogre::GpuProgramManager::getSingleton().isSyntaxSupported("glsl400"))
                {
                    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(arch + "/materials/programs/GLSL400", type, sec);
                }
            }
            else if(Ogre::GpuProgramManager::getSingleton().isSyntaxSupported("hlsl"))
            {
                Ogre::ResourceGroupManager::getSingleton().addResourceLocation(arch + "/materials/programs/HLSL", type, sec);
            }
#       ifdef OGRE_BUILD_PLUGIN_CG
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(arch + "/materials/programs/Cg", type, sec);
#       endif
            if (use_HLSL_Cg_shared)
            {
                Ogre::ResourceGroupManager::getSingleton().addResourceLocation(arch + "/materials/programs/HLSL_Cg", type, sec);
            }

#       ifdef INCLUDE_RTSHADER_SYSTEM
            if(Ogre::GpuProgramManager::getSingleton().isSyntaxSupported("glsles"))
            {
                Ogre::ResourceGroupManager::getSingleton().addResourceLocation(arch + "/RTShaderLib/GLSLES", type, sec);
            }
            else if(Ogre::GpuProgramManager::getSingleton().isSyntaxSupported("glsl"))
            {
                Ogre::ResourceGroupManager::getSingleton().addResourceLocation(arch + "/RTShaderLib/GLSL", type, sec);
                if(Ogre::GpuProgramManager::getSingleton().isSyntaxSupported("glsl150"))
                {
                    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(arch + "/RTShaderLib/GLSL150", type, sec);
                }
            }
            else if(Ogre::GpuProgramManager::getSingleton().isSyntaxSupported("hlsl"))
            {
                Ogre::ResourceGroupManager::getSingleton().addResourceLocation(arch + "/RTShaderLib/HLSL", type, sec);
            }
#           ifdef OGRE_BUILD_PLUGIN_CG
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(arch + "/RTShaderLib/Cg", type, sec);
#           endif
            if (use_HLSL_Cg_shared)
            {
                Ogre::ResourceGroupManager::getSingleton().addResourceLocation(arch + "/RTShaderLib/HLSL_Cg", type, sec);
            }
#       endif /* INCLUDE_RTSHADER_SYSTEM */
#   endif /* OGRE_PLATFORM != OGRE_PLATFORM_ANDROID */
#endif /* OGRE_PLATFORM == OGRE_PLATFORM_NACL */
        }

        /*-----------------------------------------------------------------------------
        | Loads context-wide resource groups. I chose here to simply initialise all
        | groups, but you can fully load specific ones if you wish.
        -----------------------------------------------------------------------------*/
        virtual void loadResources()
        {
            Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
        }

        /*-----------------------------------------------------------------------------
        | Reconfigures the context. Attempts to preserve the current sample state.
        -----------------------------------------------------------------------------*/
        virtual void reconfigure(const Ogre::String& renderer, Ogre::NameValuePairList& options)
        {
            // save current sample state
            mLastSample = mCurrentSample;
            if (mCurrentSample) mCurrentSample->saveState(mLastSampleState);

            mNextRenderer = renderer;
            Ogre::RenderSystem* rs = mRoot->getRenderSystemByName(renderer);

            // set all given render system options
            for (Ogre::NameValuePairList::iterator it = options.begin(); it != options.end(); it++)
            {
                rs->setConfigOption(it->first, it->second);
                
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
                // Change the viewport orientation on the fly if requested
                if(it->first == "Orientation")
                {
                    if (it->second == "Landscape Left")
                        mWindow->getViewport(0)->setOrientationMode(Ogre::OR_LANDSCAPELEFT, true);
                    else if (it->second == "Landscape Right")
                        mWindow->getViewport(0)->setOrientationMode(Ogre::OR_LANDSCAPERIGHT, true);
                    else if (it->second == "Portrait")
                        mWindow->getViewport(0)->setOrientationMode(Ogre::OR_PORTRAIT, true);
                }
#endif
            }

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
            // Need to save the config on iOS to make sure that changes are kept on disk
            mRoot->saveConfig();
#endif
            mLastRun = false;             // we want to go again with the new settings
            mRoot->queueEndRendering();   // break from render loop
        }

        /*-----------------------------------------------------------------------------
        | Recovers the last sample after a reset. You can override in the case that
        | the last sample is destroyed in the process of resetting, and you have to
        | recover it through another means.
        -----------------------------------------------------------------------------*/
        virtual void recoverLastSample()
        {
            runSample(mLastSample);
            mLastSample->restoreState(mLastSampleState);
            mLastSample = 0;
            mLastSampleState.clear();
        }

        /*-----------------------------------------------------------------------------
        | Cleans up and shuts down the context.
        -----------------------------------------------------------------------------*/
        virtual void shutdown()
        {
            if (mCurrentSample)
            {
                mCurrentSample->_shutdown();
                mCurrentSample = 0;
            }

            // remove window event listener before shutting down OIS
            Ogre::WindowEventUtilities::removeWindowEventListener(mWindow, this);

#ifdef HAVE_SDL
            SDL_QuitSubSystem(SDL_INIT_EVENTS);
            SDL_QuitSubSystem(SDL_INIT_VIDEO);
#endif
        }

        /*-----------------------------------------------------------------------------
        | Captures input device states.
        -----------------------------------------------------------------------------*/
        virtual void captureInputDevices()
        {
#ifdef HAVE_SDL
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                switch (event.type)
                {
                case SDL_QUIT:
                    mRoot->queueEndRendering();
                    break;
                case SDL_WINDOWEVENT:
                    if(event.window.event == SDL_WINDOWEVENT_RESIZED) {
                        mWindow->resize(event.window.data1, event.window.data2);
                        windowResized(mWindow);
                    }
                    break;
                case SDL_KEYDOWN:
                    // Ignore repeated signals from key being held down.
                    if (event.key.repeat) break;
                    keyPressed(event.key);
                    break;
                case SDL_KEYUP:
                    keyReleased(event.key);
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    mousePressed(event.button);
                    break;
                case SDL_MOUSEBUTTONUP:
                    mouseReleased(event.button);
                    break;
                case SDL_MOUSEWHEEL:
                    mouseWheelRolled(event.wheel);
                    break;
                case SDL_MOUSEMOTION:
                    mouseMoved(event.motion);
                    break;
                case SDL_FINGERDOWN:
                    touchPressed(event.tfinger);
                    break;
                case SDL_FINGERUP:
                    touchReleased(event.tfinger);
                    break;
                case SDL_FINGERMOTION:
                    touchMoved(event.tfinger);
                    break;
                }
        	}
#endif
        }
        
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
        Ogre::DataStreamPtr openAPKFile(const Ogre::String& fileName)
        {
            Ogre::DataStreamPtr stream;
            AAsset* asset = AAssetManager_open(mAssetMgr, fileName.c_str(), AASSET_MODE_BUFFER);
            if(asset)
            {
                off_t length = AAsset_getLength(asset);
                void* membuf = OGRE_MALLOC(length, Ogre::MEMCATEGORY_GENERAL);
                memcpy(membuf, AAsset_getBuffer(asset), length);
                AAsset_close(asset);
                
                stream = Ogre::DataStreamPtr(new Ogre::MemoryDataStream(membuf, length, true, true));
            }
            return stream;
        }
        AAssetManager* mAssetMgr;       // Android asset manager to access files inside apk
#endif

#if (OGRE_THREAD_PROVIDER == 3) && (OGRE_NO_TBB_SCHEDULER == 1)
        tbb::task_scheduler_init mTaskScheduler;
#endif

        Ogre::FileSystemLayer* mFSLayer; // File system abstraction layer
        Ogre::Root* mRoot;              // OGRE root
        Ogre::OverlaySystem* mOverlaySystem;  // Overlay system
#ifdef OGRE_STATIC_LIB
        Ogre::StaticPluginLoader mStaticPluginLoader;
#endif
        Sample* mCurrentSample;         // currently running sample
        bool mSamplePaused;             // whether current sample is paused
        bool mFirstRun;                 // whether or not this is the first run
        bool mLastRun;                  // whether or not this is the final run
        Ogre::String mNextRenderer;     // name of renderer used for next run
        Sample* mLastSample;            // last sample run before reconfiguration
        Ogre::NameValuePairList mLastSampleState;     // state of last sample
#ifdef HAVE_SDL
        SDL_Window* mSDLWindow;
#endif
    public:
        Ogre::RenderWindow* mWindow;    // render window
    };
}

#endif
