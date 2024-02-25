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
#ifndef __ApplicationContextBase_H__
#define __ApplicationContextBase_H__

#include "OgreBitesPrerequisites.h"
#include "OgreBuildSettings.h"
#include "OgreComponents.h"
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
    class ImGuiOverlay;
}

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
#include <android/native_window.h>
#endif

#include "OgreInput.h"

namespace OgreBites
{
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
    typedef ANativeWindow NativeWindowType;
#else
    typedef SDL_Window NativeWindowType;
#endif

    /** \addtogroup Optional Optional Components
    *  @{
    */
    /** \defgroup Bites Bites
    * reusable utilities for rapid prototyping
    *  @{
    */

    /**
     * link between a renderwindow and a platform specific window
     */
    struct NativeWindowPair
    {
        Ogre::RenderWindow* render;
        NativeWindowType* native;
    };

    /**
    Base class responsible for setting up a common context for applications.
    Subclass to implement specific event callbacks.
    */
    class _OgreBitesExport ApplicationContextBase : public Ogre::FrameListener
    {
    public:
        explicit ApplicationContextBase(const Ogre::String& appName = "Ogre3D");

        virtual ~ApplicationContextBase();

        /**
         * get the main RenderWindow
         * owns the context on OpenGL
         */
        Ogre::RenderWindow* getRenderWindow() const
        {
            return mWindows.empty() ? NULL : mWindows[0].render;
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

        /**
        This function closes down the application - saves the configuration then
        shutdowns.
        */
        void closeApp();

        // callback interface copied from various listeners to be used by ApplicationContext
        bool frameStarted(const Ogre::FrameEvent& evt) override {
            pollEvents();
            return true;
        }
        bool frameRenderingQueued(const Ogre::FrameEvent& evt) override;
        bool frameEnded(const Ogre::FrameEvent& evt) override { return true; }
        virtual void windowMoved(Ogre::RenderWindow* rw) {}
        virtual void windowResized(Ogre::RenderWindow* rw) {}
        virtual bool windowClosing(Ogre::RenderWindow* rw) { return true; }
        virtual void windowClosed(Ogre::RenderWindow* rw) {}
        virtual void windowFocusChange(Ogre::RenderWindow* rw) {}

        /**
         * inspect the event and call one of the corresponding functions on the registered InputListener
         * @param event Input Event
         * @param windowID only call listeners of this window
         */
        void _fireInputEvent(const Event& event, uint32_t windowID) const;

        /**
          Initialize the RT Shader system.
          */
        bool initialiseRTShaderSystem();

        /**
         * make the RTSS write out the generated shaders for caching and debugging
         *
         * by default all shaders are generated to system memory.
         * Must be called before loadResources
         * @param write Whether to write out the generated shaders
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
        When input is grabbed the mouse is confined to the window.
        */
        virtual void setWindowGrab(NativeWindowType* win, bool grab = true) {}

        /// get the vertical DPI of the display
        virtual float getDisplayDPI() const { return 96.0f; }

        /// @overload
        void setWindowGrab(bool grab = true) {
            OgreAssert(!mWindows.empty(), "create a window first");
            setWindowGrab(mWindows[0].native, grab);
        }

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

        /// @deprecated use do not use
        OGRE_DEPRECATED void reconfigure(const Ogre::String& renderer, Ogre::NameValuePairList& options);


        /**
        Cleans up and shuts down the context.
        */
        virtual void shutdown();

        /**
        process all window events since last call
        */
        virtual void pollEvents();

        /**
        Creates dummy scene to allow rendering GUI in viewport.
          */
        void createDummyScene();

        /**
        Destroys dummy scene.
          */
        void destroyDummyScene();

        /** Show the renderer configuration menu
         *
         * creates a dummy scene to allow rendering the dialog
         */
        void runRenderingSettingsDialog();

        /**
         * enables the caching of compiled shaders to file
         *
         * also loads any existing cache
         */
        void enableShaderCache() const;

        /** attach input listener
         *
         * @param lis the listener
         * @param win the window to receive the events for.
         */
        virtual void addInputListener(NativeWindowType* win, InputListener* lis);

        /// @overload
        void addInputListener(InputListener* lis) {
            OgreAssert(!mWindows.empty(), "create a window first");
            addInputListener(mWindows[0].native, lis);
        }

        /** detatch input listener
         *
         * @param lis the listener
         * @param win the window to receive the events for.
         */
        virtual void removeInputListener(NativeWindowType* win, InputListener* lis);

        /// @overload
        void removeInputListener(InputListener* lis) {
            OgreAssert(!mWindows.empty(), "called after all windows were deleted");
            removeInputListener(mWindows[0].native, lis);
        }

        /**
         * Create a new render window
         *
         * You must use SDL and not an auto-created window as SDL does not get the events
         * otherwise.
         *
         * By default the values from ogre.cfg are used for w, h and miscParams.
         */
        virtual NativeWindowPair
        createWindow(const Ogre::String& name, uint32_t w = 0, uint32_t h = 0,
                     Ogre::NameValuePairList miscParams = Ogre::NameValuePairList());

        /// destroy and erase an NativeWindowPair by name
        void destroyWindow(const Ogre::String& name);

        /**
         * get the FileSystemLayer instance pointing to an application specific directory
         */
        Ogre::FileSystemLayer& getFSLayer() const { return *mFSLayer; }

        /**
         * the directory where the media files were installed
         *
         * same as OGRE_MEDIA_DIR in CMake
         */
        static Ogre::String getDefaultMediaDir();

        /**
         * Set up the overlay system for usage with ImGui
         */
        Ogre::ImGuiOverlay* initialiseImGui();

        InputListener* getImGuiInputListener() const { return mImGuiListener.get(); }
    protected:
        /// internal method to destroy both the render and the native window
        virtual void _destroyWindow(const NativeWindowPair& win);

        Ogre::OverlaySystem* mOverlaySystem;  // Overlay system

        Ogre::FileSystemLayer* mFSLayer; // File system abstraction layer
        Ogre::Root* mRoot;              // OGRE root
        StaticPluginLoader mStaticPluginLoader;
        bool mFirstRun;
        Ogre::String mNextRenderer;     // name of renderer used for next run
        Ogre::String mAppName;

        typedef std::vector<NativeWindowPair> WindowList;
        WindowList mWindows; // all windows

        typedef std::set<std::pair<uint32_t, InputListener*> > InputListenerList;
        InputListenerList mInputListeners;

        std::unique_ptr<InputListener> mImGuiListener;

#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
        Ogre::RTShader::ShaderGenerator*       mShaderGenerator; // The Shader generator instance.
        SGTechniqueResolverListener*       mMaterialMgrListener; // Shader generator material manager listener.
#endif // INCLUDE_RTSHADER_SYSTEM
    };

    /** @} */
    /** @} */
}
#endif
