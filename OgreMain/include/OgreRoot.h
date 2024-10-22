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
#ifndef __ROOT__
#define __ROOT__

// Precompiler options
#include "OgrePrerequisites.h"

#include <exception>
#include <deque>
#include "OgreSingleton.h"
#include "OgreSceneManager.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup General
    *  @{
    */

    class AndroidLogListener;
    class ShadowTextureManager;
    class SceneManagerEnumerator;

    typedef std::vector<RenderSystem*> RenderSystemList;

    /// Scene manager instances, indexed by instance name
    typedef std::map<String, SceneManager*> SceneManagerInstanceMap;

    OGRE_DEBUG_NS_BEGIN

    /** The root class of the Ogre system.

        The Ogre::Root class represents a starting point for the client
        application. From here, the application can gain access to the
        fundamentals of the system, namely the rendering systems
        available, management of saved configurations, logging, and
        access to other classes in the system. Acts as a hub from which
        all other objects may be reached. An instance of Root must be
        created before any other Ogre operations are called. Once an
        instance has been created, the same instance is accessible
        throughout the life of that object by using Root::getSingleton
        (as a reference) or Root::getSingletonPtr (as a pointer).
    */
    class _OgreExport Root : public Singleton<Root>, public RootAlloc
    {
        // To allow update of active renderer if
        // RenderSystem::initialise is used directly
        friend class RenderSystem;
    public:
        typedef std::map<String, MovableObjectFactory*> MovableObjectFactoryMap;
        typedef std::vector<DynLib*> PluginLibList;
        typedef std::vector<Plugin*> PluginInstanceList;
    private:
        RenderSystemList mRenderers;
        RenderSystem* mActiveRenderer;
        String mVersion;
        String mConfigFileName;
        bool mQueuedEnd;
        // In case multiple render windows are created, only once are the resources loaded.
        bool mFirstTimePostWindowInit;

        // ordered in reverse destruction sequence
        std::unique_ptr<LogManager> mLogManager;
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
        std::unique_ptr<AndroidLogListener> mAndroidLogger;
#endif
        std::unique_ptr<ScriptCompilerManager> mCompilerManager;
        std::unique_ptr<DynLibManager> mDynLibManager;
        std::unique_ptr<Timer> mTimer;
        std::unique_ptr<WorkQueue> mWorkQueue;
        std::unique_ptr<ResourceGroupManager> mResourceGroupManager;
        std::unique_ptr<ResourceBackgroundQueue> mResourceBackgroundQueue;
        std::unique_ptr<MaterialManager> mMaterialManager;
        std::unique_ptr<GpuProgramManager> mGpuProgramManager;
        std::unique_ptr<ControllerManager> mControllerManager;
        std::unique_ptr<MeshManager> mMeshManager;
        std::unique_ptr<SkeletonManager> mSkeletonManager;

        std::unique_ptr<ArchiveFactory> mFileSystemArchiveFactory;
        std::unique_ptr<ArchiveFactory> mEmbeddedZipArchiveFactory;
        std::unique_ptr<ArchiveFactory> mZipArchiveFactory;
        std::unique_ptr<ArchiveManager> mArchiveManager;

        MovableObjectFactoryMap mMovableObjectFactoryMap;
        std::unique_ptr<MovableObjectFactory> mRibbonTrailFactory;
        std::unique_ptr<MovableObjectFactory> mBillboardChainFactory;
        std::unique_ptr<MovableObjectFactory> mManualObjectFactory;
        std::unique_ptr<MovableObjectFactory> mBillboardSetFactory;
        std::unique_ptr<MovableObjectFactory> mLightFactory;
        std::unique_ptr<MovableObjectFactory> mEntityFactory;
        std::unique_ptr<MovableObjectFactory> mStaticGeometryFactory;
        std::unique_ptr<MovableObjectFactory> mRectangle2DFactory;

        std::unique_ptr<ParticleSystemManager> mParticleManager;
        std::unique_ptr<LodStrategyManager> mLodStrategyManager;
        std::unique_ptr<Profiler> mProfiler;

        std::unique_ptr<ExternalTextureSourceManager> mExternalTextureSourceManager;
        std::unique_ptr<CompositorManager> mCompositorManager;
        std::unique_ptr<RenderSystemCapabilitiesManager> mRenderSystemCapabilitiesManager;

        std::unique_ptr<SceneManagerEnumerator> mSceneManagerEnum;
        SceneManager* mCurrentSceneManager;

        std::unique_ptr<ShadowTextureManager> mShadowTextureManager;

        RenderWindow* mAutoWindow;

        unsigned long mNextFrame;
        Real mFrameSmoothingTime;
        bool mRemoveQueueStructuresOnClear;
        Real mDefaultMinPixelSize;

    private:
        /// List of plugin DLLs loaded
        PluginLibList mPluginLibs;
        /// List of Plugin instances registered
        PluginInstanceList mPlugins;

        uint32 mNextMovableObjectTypeFlag;

        /// Are we initialised yet?
        bool mIsInitialised;
        ///Tells whether blend indices information needs to be passed to the GPU
        bool mIsBlendIndicesGpuRedundant;
        ///Tells whether blend weights information needs to be passed to the GPU
        bool mIsBlendWeightsGpuRedundant;

        /** Method reads a plugins configuration file and instantiates all
            plugins.
            @param
                pluginsfile The file that contains plugins information.
        */
        void loadPlugins(const String& pluginsfile = "plugins.cfg");
        /** Initialise all loaded plugins - allows plugins to perform actions
            once the renderer is initialised.
        */
        void initialisePlugins();
        /** Shuts down all loaded plugins - allows things to be tidied up whilst
            all plugins are still loaded.
        */
        void shutdownPlugins();

        /** Unloads all loaded plugins.
        */
        void unloadPlugins();

        /// Internal method for one-time tasks after first window creation
        void oneTimePostWindowInit(void);

        /** Set of registered frame listeners */
        std::set<FrameListener*> mFrameListeners;

        /** Set of frame listeners marked for removal and addition*/
        std::set<FrameListener*> mRemovedFrameListeners;
        std::set<FrameListener*> mAddedFrameListeners;
        void _syncAddedRemovedFrameListeners();

        /** Indicates the type of event to be considered by calculateEventTime(). */
        enum FrameEventTimeType {
            FETT_ANY = 0,
            FETT_STARTED = 1,
            FETT_QUEUED = 2,
            FETT_ENDED = 3,
            FETT_COUNT = 4
        };

        /// Contains the times of recently fired events
        typedef std::deque<unsigned long> EventTimesQueue;
        EventTimesQueue mEventTimes[FETT_COUNT];

        /** Internal method for calculating the average time between recently fired events.
        @param now The current time in ms.
        @param type The type of event to be considered.
        */
        Real calculateEventTime(unsigned long now, FrameEventTimeType type);

        /** Update a set of event times (note, progressive, only call once for each type per frame) */
        void populateFrameEvent(FrameEventTimeType type, FrameEvent& evtToUpdate);

    public:

        /** Constructor
        @param pluginFileName The file that contains plugins information.
            May be left blank to ignore.
        @param configFileName The file that contains the configuration to be loaded.
            Defaults to "ogre.cfg", may be left blank to load nothing.
        @param logFileName The logfile to create, defaults to Ogre.log, may be
            left blank if you've already set up LogManager & Log yourself
        */
        Root(const String& pluginFileName = "plugins.cfg",
            const String& configFileName = "ogre.cfg",
            const String& logFileName = "Ogre.log");
        ~Root();

        /** Saves the details of the current configuration

            Stores details of the current configuration so it may be
            restored later on.
        */
        void saveConfig(void);

        /** Checks for saved video/sound/etc settings

            This method checks to see if there is a valid saved configuration
            from a previous run. If there is, the state of the system will
            be restored to that configuration.

            @return
                If a valid configuration was found, <b>true</b> is returned.
            @par
                If there is no saved configuration, or if the system failed
                with the last config settings, <b>false</b> is returned.
        */
        bool restoreConfig(void);

        /** Displays a dialog asking the user to choose system settings.

            This method displays the default dialog allowing the user to
            choose the rendering system, video mode etc. If there is are
            any settings saved already, they will be restored automatically
            before displaying the dialogue. When the user accepts a group of
            settings, this will automatically call Root::setRenderSystem,
            RenderSystem::setConfigOption and Root::saveConfig with the
            user's choices. This is the easiest way to get the system
            configured.
            @param dialog ConfigDialog implementation to use.
                If NULL, the first available render system with the default options
                will be selected.
            @return
                If the user clicked 'Ok', <b>true</b> is returned.
            @par
                If they clicked 'Cancel' (in which case the app should
                strongly consider terminating), <b>false</b> is returned.
         */
        bool showConfigDialog(ConfigDialog* dialog);

        /** Adds a new rendering subsystem to the list of available renderers.

            Intended for use by advanced users and plugin writers only!
            Calling this method with a pointer to a valid RenderSystem
            (subclass) adds a rendering API implementation to the list of
            available ones. Typical examples would be an OpenGL
            implementation and a Direct3D implementation.
            @note
                This should usually be called from the dllStartPlugin()
                function of an extension plug-in.
        */
        void addRenderSystem(RenderSystem* newRend);

        /** Retrieve a list of the available render systems.

            Retrieves a pointer to the list of available renderers as a
            list of RenderSystem subclasses. Can be used to build a
            custom settings dialog.
        */
        const RenderSystemList& getAvailableRenderers(void);

        /** Retrieve a pointer to the render system by the given name
            @param
                name Name of the render system intend to retrieve.
            @return
                A pointer to the render system, <b>NULL</b> if no found.
        */
        RenderSystem* getRenderSystemByName(const String& name);

        /** Sets the rendering subsystem to be used.

            This method indicates to OGRE which rendering system is to be
            used (e.g. Direct3D, OpenGL etc). This is called
            automatically by the default config dialog, and when settings
            are restored from a previous configuration. If used manually
            it could be used to set the renderer from a custom settings
            dialog. Once this has been done, the renderer can be
            initialised using Root::initialise.
            @par
                This method is also called by render systems if they are
                initialised directly.
            @param
                system Pointer to the render system to use.
            @see
                RenderSystem
        */
        void setRenderSystem(RenderSystem* system);

        /** Retrieve a pointer to the currently selected render system.
        */
        RenderSystem* getRenderSystem(void);

        /** Initialises the renderer.

            This method can only be called after a renderer has been
            selected with Root::setRenderSystem, and it will initialise
            the selected rendering system ready for use.
            @param
                autoCreateWindow If true, a rendering window will
                automatically be created (saving a call to
                Root::createRenderWindow). The window will be
                created based on the options currently set on the render
                system.
            @param windowTitle
            @return
                A pointer to the automatically created window, if
                requested, otherwise <b>NULL</b>.
        */
        RenderWindow* initialise(bool autoCreateWindow, const String& windowTitle = "OGRE Render Window");

        /** Returns whether the system is initialised or not. */
        bool isInitialised(void) const { return mIsInitialised; }

        /** Requests active RenderSystem to use custom RenderSystemCapabilities

            This is useful for testing how the RenderSystem would behave on a machine with
            less advanced GPUs. This method MUST be called before creating the first RenderWindow
        */
        void useCustomRenderSystemCapabilities(RenderSystemCapabilities* capabilities);

        /** Get whether the entire render queue structure should be emptied on clearing,
            or whether just the objects themselves should be cleared.
        */
        bool getRemoveRenderQueueStructuresOnClear() const { return mRemoveQueueStructuresOnClear; }

        /** Set whether the entire render queue structure should be emptied on clearing,
        or whether just the objects themselves should be cleared.
        */
        void setRemoveRenderQueueStructuresOnClear(bool r) { mRemoveQueueStructuresOnClear = r; }

        /** Register a new SceneManagerFactory, a factory object for creating instances
            of specific SceneManagers.

            Plugins should call this to register as new SceneManager providers.
        */
        void addSceneManagerFactory(SceneManagerFactory* fact);

        /** Remove a SceneManagerFactory.
        */
        void removeSceneManagerFactory(SceneManagerFactory* fact);

        /// get all types of SceneManager available for construction
        const StringVector& getSceneManagerTypes() const;

        /// create a default scene manager
        SceneManager* createSceneManager() { return createSceneManager(SMT_DEFAULT); }

        /** Create a SceneManager instance of a given type.

            You can use this method to create a SceneManager instance of a
            given specific type. You may know this type already, or you may
            have discovered it by looking at the results from getMetaDataIterator.
        @note
            This method throws an exception if the named type is not found.
        @param typeName String identifying a unique SceneManager type
        @param instanceName Optional name to given the new instance that is
            created. If you leave this blank, an auto name will be assigned.
        */
        SceneManager* createSceneManager(const String& typeName, const String& instanceName = BLANKSTRING);

        /// @deprecated do not use
        OGRE_DEPRECATED SceneManager* createSceneManager(uint16 typeMask, const String& instanceName = BLANKSTRING)
        {
            return createSceneManager(SMT_DEFAULT, instanceName);
        }

        /** Destroy an instance of a SceneManager. */
        void destroySceneManager(SceneManager* sm);

        /** Get an existing SceneManager instance that has already been created,
            identified by the instance name.
        @param instanceName The name of the instance to retrieve.
        */
        SceneManager* getSceneManager(const String& instanceName) const;

        /** Identify if a SceneManager instance already exists.
        @param instanceName The name of the instance to retrieve.
        */
        bool hasSceneManager(const String& instanceName) const;

        /// Get all the existing SceneManager instances.
        const SceneManagerInstanceMap& getSceneManagers(void) const;

        /** Retrieves a reference to the current TextureManager.

            This performs the same function as
            TextureManager::getSingleton, but is provided for convenience
            particularly to scripting engines.
            @par
                Note that a TextureManager will NOT be available until the
                Ogre system has been initialised by selecting a RenderSystem,
                calling Root::initialise and a window having been created
                (this may have been done by initialise if required). This is
                because the exact runtime subclass which will be implementing
                the calls will differ depending on the rendering engine
                selected, and these typically require a window upon which to
                base texture format decisions.
        */
        TextureManager* getTextureManager(void);

        /** Retrieves a reference to the current MeshManager.

            This performs the same function as MeshManager::getSingleton
            and is provided for convenience to scripting engines.
        */
        MeshManager* getMeshManager(void);

        /** Registers a FrameListener which will be called back every frame.

            A FrameListener is a class which implements methods which
            will be called every frame.
            @par
                See the FrameListener class for more details on the specifics
                It is imperative that the instance passed to this method is
                not destroyed before either the rendering loop ends, or the
                class is removed from the listening list using
                removeFrameListener().
            @note
                This method can only be called after Root::initialise has
                been called.
        */
        void addFrameListener(FrameListener* newListener);

        /** Removes a FrameListener from the list of listening classes.
        */
        void removeFrameListener(FrameListener* oldListener);

        /** Queues the end of rendering.

            This method will do nothing unless startRendering() has
            been called, in which case before the next frame is rendered
            the rendering loop will bail out.
        */
        void queueEndRendering(bool state = true);

        /** Check for planned end of rendering.

            This method return true if queueEndRendering() was called before.
        */
        bool endRenderingQueued(void);

        /** Starts / restarts the automatic rendering cycle.

            This method begins the automatic rendering of the scene. It
            will <b>NOT</b> return until the rendering cycle is halted.
            @par
                During rendering, any FrameListener classes registered using
                addFrameListener will be called back for each frame that is
                to be rendered, These classes can tell OGRE to halt the
                rendering if required, which will cause this method to
                return.
            @note
                <br>Users of the OGRE library do not have to use this
                automatic rendering loop. It is there as a convenience and is
                most useful for high frame rate applications e.g. games. For
                applications that don't need to constantly refresh the
                rendering targets (e.g. an editor utility), it is better to
                manually refresh each render target only when required by
                calling RenderTarget::update, or if you want to run your own
                render loop you can update all targets on demand using
                Root::renderOneFrame.
            @note
                This frees up the CPU to do other things in between
                refreshes, since in this case frame rate is less important.
            @note
                This method can only be called after Root::initialise has
                been called.
        */
        void startRendering(void);

        /** Updates all the render targets automatically

            Raises frame events before and after.

            Overview of the render cycle
            ![](renderOneFrame.svg)
        */
        bool renderOneFrame(void);

        /** Updates all the render targets with custom frame time information

        Updates all the render targets automatically and then returns,
        raising frame events before and after - all per-frame times are based on
        the time value you pass in.
        */
        bool renderOneFrame(Real timeSinceLastFrame);

        /** Shuts down the system manually.

            This is normally done by Ogre automatically so don't think
            you have to call this yourself. However this is here for
            convenience, especially for dealing with unexpected errors or
            for systems which need to shut down Ogre on demand.
        */
        void shutdown(void);

        /** Helper method to assist you in creating writeable file streams.

            This is a high-level utility method which you can use to find a place to
            save a file more easily. If the filename you specify is either an
            absolute or relative filename (ie it includes path separators), then
            the file will be created in the normal filesystem using that specification.
            If it doesn't, then the method will look for a writeable resource location
            via ResourceGroupManager::createResource using the other params provided.
        @param filename The name of the file to create. If it includes path separators,
            the filesystem will be accessed direct. If no path separators are
            present the resource system is used, falling back on the raw filesystem after.
        @param groupName The name of the group in which to create the file, if the
            resource system is used
        @param overwrite If true, an existing file will be overwritten, if false
            an error will occur if the file already exists
        @param locationPattern If the resource group contains multiple locations,
            then usually the file will be created in the first writable location. If you
            want to be more specific, you can include a location pattern here and
            only locations which match that pattern (as determined by StringUtil::match)
            will be considered candidates for creation.
        */
        static DataStreamPtr createFileStream(const String& filename,
                const String& groupName = ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                bool overwrite = false, const String& locationPattern = BLANKSTRING);

        /** Helper method to assist you in accessing readable file streams.

            This is a high-level utility method which you can use to find a place to
            open a file more easily. It checks the resource system first, and if
            that fails falls back on accessing the file system directly.
        @param filename The name of the file to open.
        @param groupName The name of the group in which to create the file, if the
            resource system is used
        */
        static DataStreamPtr openFileStream(const String& filename,
                const String& groupName = ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

        /// @deprecated use ColourValue::getAsBYTE()
        OGRE_DEPRECATED static void convertColourValue(const ColourValue& colour, uint32* pDest) { *pDest = colour.getAsBYTE(); }

        /** Retrieves a pointer to the window that was created automatically

            When Root is initialised an optional window is created. This
            method retrieves a pointer to that window.
            @note
                returns a null pointer when Root has not been initialised with
                the option of creating a window.
        */
        RenderWindow* getAutoCreatedWindow(void);

        /** @copydoc RenderSystem::_createRenderWindow
        */
        RenderWindow* createRenderWindow(const String &name, unsigned int width, unsigned int height,
            bool fullScreen, const NameValuePairList *miscParams = 0) ;

        /// @overload
        RenderWindow* createRenderWindow(const RenderWindowDescription& desc)
        {
            return createRenderWindow(desc.name, desc.width, desc.height,
                                      desc.useFullScreen, &desc.miscParams);
        }

        /** Detaches a RenderTarget from the active render system
        and returns a pointer to it.
        @note
        If the render target cannot be found, NULL is returned.
        */
        RenderTarget* detachRenderTarget( RenderTarget* pWin );

        /** Detaches a named RenderTarget from the active render system
        and returns a pointer to it.
        @note
        If the render target cannot be found, NULL is returned.
        */
        RenderTarget* detachRenderTarget( const String & name );

        /** Destroys the given RenderTarget.
        */
        void destroyRenderTarget(RenderTarget* target);

        /** Destroys the given named RenderTarget.
        */
        void destroyRenderTarget(const String &name);

        /** Retrieves a pointer to a named render target.
        */
        RenderTarget * getRenderTarget(const String &name);

        /** Manually load a Plugin contained in a DLL / DSO.

            Plugins embedded in DLLs can be loaded at startup using the plugin
            configuration file specified when you create Root.
            This method allows you to load plugin DLLs directly in code.
            The DLL in question is expected to implement a dllStartPlugin
            method which instantiates a Plugin subclass and calls Root::installPlugin.
            It should also implement dllStopPlugin (see Root::unloadPlugin)
        @param pluginName Name of the plugin library to load
        */
        void loadPlugin(const String& pluginName);

        /** Manually unloads a Plugin contained in a DLL / DSO.

            Plugin DLLs are unloaded at shutdown automatically. This method
            allows you to unload plugins in code, but make sure their
            dependencies are decoupled first. This method will call the
            dllStopPlugin method defined in the DLL, which in turn should call
            Root::uninstallPlugin.
        @param pluginName Name of the plugin library to unload
        */
        void unloadPlugin(const String& pluginName);

        /** Install a new plugin.

            This installs a new extension to OGRE. The plugin itself may be loaded
            from a DLL / DSO, or it might be statically linked into your own
            application. Either way, something has to call this method to get
            it registered and functioning. You should only call this method directly
            if your plugin is not in a DLL that could otherwise be loaded with
            loadPlugin, since the DLL function dllStartPlugin should call this
            method when the DLL is loaded.
        */
        void installPlugin(Plugin* plugin);

        /** Uninstall an existing plugin.

            This uninstalls an extension to OGRE. Plugins are automatically
            uninstalled at shutdown but this lets you remove them early.
            If the plugin was loaded from a DLL / DSO you should call unloadPlugin
            which should result in this method getting called anyway (if the DLL
            is well behaved).
        */
        void uninstallPlugin(Plugin* plugin);

        /** Gets a read-only list of the currently installed plugins. */
        const PluginInstanceList& getInstalledPlugins() const { return mPlugins; }

        /** Gets a pointer to the central timer used for all OGRE timings */
        Timer* getTimer(void);

        /** Method for raising frame started events.

            This method is only for internal use when you use OGRE's inbuilt rendering
            loop (Root::startRendering). However, if you run your own rendering loop then
            you should call this method to ensure that FrameListener objects are notified
            of frame events; processes like texture animation and particle systems rely on
            this.
        @par
            Calling this method also increments the frame number, which is
            important for keeping some elements of the engine up to date.
        @note
            This method takes an event object as a parameter, so you can specify the times
            yourself. If you are happy for OGRE to automatically calculate the frame time
            for you, then call the other version of this method with no parameters.
        @param evt Event object which includes all the timing information which you have
            calculated for yourself
        @return False if one or more frame listeners elected that the rendering loop should
            be terminated, true otherwise.
        */
        bool _fireFrameStarted(FrameEvent& evt);
        /** Method for raising frame rendering queued events.

            This method is only for internal use when you use OGRE's inbuilt rendering
            loop (Root::startRendering). However, if you run your own rendering loop then
            you should call this method too, to ensure that all state is updated
            correctly. You should call it after the windows have been updated
            but before the buffers are swapped, or if you are not separating the
            update and buffer swap, then after the update just before _fireFrameEnded.
        */
        bool _fireFrameRenderingQueued(FrameEvent& evt);

        /** Method for raising frame ended events.

            This method is only for internal use when you use OGRE's inbuilt rendering
            loop (Root::startRendering). However, if you run your own rendering loop then
            you should call this method to ensure that FrameListener objects are notified
            of frame events; processes like texture animation and particle systems rely on
            this.
        @note
            This method takes an event object as a parameter, so you can specify the times
            yourself. If you are happy for OGRE to automatically calculate the frame time
            for you, then call the other version of this method with no parameters.
        @param evt Event object which includes all the timing information which you have
            calculated for yourself
        @return False if one or more frame listeners elected that the rendering loop should
            be terminated, true otherwise.
        */
        bool _fireFrameEnded(FrameEvent& evt);
        /** Method for raising frame started events.

            This method is only for internal use when you use OGRE's inbuilt rendering
            loop (Root::startRendering). However, if you run your own rendering loop then
            you should call this method to ensure that FrameListener objects are notified
            of frame events; processes like texture animation and particle systems rely on
            this.
        @par
            Calling this method also increments the frame number, which is
            important for keeping some elements of the engine up to date.
        @note
            This method calculates the frame timing information for you based on the elapsed
            time. If you want to specify elapsed times yourself you should call the other
            version of this method which takes event details as a parameter.
        @return False if one or more frame listeners elected that the rendering loop should
            be terminated, true otherwise.
        */
        bool _fireFrameStarted();
        /** Method for raising frame rendering queued events.

            This method is only for internal use when you use OGRE's inbuilt rendering
            loop (Root::startRendering). However, if you run your own rendering loop then
            you you may want to call this method too, although nothing in OGRE relies on this
            particular event. Really if you're running your own rendering loop at
            this level of detail then you can get the same effect as doing your
            updates in a frameRenderingQueued callback by just calling
            RenderWindow::update with the 'swapBuffers' option set to false.
        */
        bool _fireFrameRenderingQueued();
        /** Method for raising frame ended events.

            This method is only for internal use when you use OGRE's inbuilt rendering
            loop (Root::startRendering). However, if you run your own rendering loop then
            you should call this method to ensure that FrameListener objects are notified
            of frame events; processes like texture animation and particle systems rely on
            this.
        @note
            This method calculates the frame timing information for you based on the elapsed
            time. If you want to specify elapsed times yourself you should call the other
            version of this method which takes event details as a parameter.
        @return False if one or more frame listeners elected that the rendering loop should
            be terminated, true otherwise.
        */
        bool _fireFrameEnded();

        /** Gets the number of the next frame to be rendered.

            Note that this is 'next frame' rather than 'current frame' because
            it indicates the frame number that current changes made to the scene
            will take effect. It is incremented after all rendering commands for
            the current frame have been queued, thus reflecting that if you
            start performing changes then, you will actually see them in the
            next frame. */
        unsigned long getNextFrameNumber(void) const { return mNextFrame; }

        /** Returns the scene manager currently being used to render a frame.

            This is only intended for internal use; it is only valid during the
            rendering of a frame.
        */
        SceneManager* _getCurrentSceneManager(void) const { return mCurrentSceneManager; }
        /** Sets the scene manager currently being used to render.

            This is only intended for internal use.
        */
        void _setCurrentSceneManager(SceneManager* sm) { mCurrentSceneManager = sm; }

        /** Internal method used for updating all RenderTarget objects (windows,
            renderable textures etc) which are set to auto-update.

            You don't need to use this method if you're using Ogre's own internal
            rendering loop (Root::startRendering). If you're running your own loop
            you may wish to call it to update all the render targets which are
            set to auto update (RenderTarget::setAutoUpdated). You can also update
            individual RenderTarget instances using their own update() method.
        @return false if a FrameListener indicated it wishes to exit the render loop
        */
        bool _updateAllRenderTargets(void);

        /** Internal method used for updating all RenderTarget objects (windows,
            renderable textures etc) which are set to auto-update, with a custom time
            passed to the frameRenderingQueued events.

            You don't need to use this method if you're using Ogre's own internal
            rendering loop (Root::startRendering). If you're running your own loop
            you may wish to call it to update all the render targets which are
            set to auto update (RenderTarget::setAutoUpdated). You can also update
            individual RenderTarget instances using their own update() method.
        @return false if a FrameListener indicated it wishes to exit the render loop
        */
        bool _updateAllRenderTargets(FrameEvent& evt);

        /// @copydoc Singleton::getSingleton()
        static Root& getSingleton(void);
        /// @copydoc Singleton::getSingleton()
        static Root* getSingletonPtr(void);

        /** Clears the history of all event times.

            OGRE stores a history of the last few event times in order to smooth
            out any inaccuracies and temporary fluctuations. However, if you
            pause or don't render for a little while this can cause a lurch, so
            if you're resuming rendering after a break, call this method to reset
            the stored times
        */
        void clearEventTimes(void);

        /** Sets the period over which OGRE smooths out fluctuations in frame times.

            OGRE by default gives you the raw frame time, but can optionally
            smooths it out over several frames, in order to reduce the
            noticeable effect of occasional hiccups in framerate.
            These smoothed values are passed back as parameters to FrameListener
            calls.
        @par
            This method allow you to tweak the smoothing period, and is expressed
            in seconds. Setting it to 0 will result in completely unsmoothed
            frame times (the default).
        */
        void setFrameSmoothingPeriod(Real period) { mFrameSmoothingTime = period; }
        /** Gets the period over which OGRE smooths out fluctuations in frame times. */
        Real getFrameSmoothingPeriod(void) const { return mFrameSmoothingTime; }

        /** Register a new MovableObjectFactory which will create new MovableObject
            instances of a particular type, as identified by the getType() method.

            Plugin creators can create subclasses of MovableObjectFactory which
            construct custom subclasses of MovableObject for insertion in the
            scene. This is the primary way that plugins can make custom objects
            available.
        @param fact Pointer to the factory instance
        @param overrideExisting Set this to true to override any existing
            factories which are registered for the same type. You should only
            change this if you are very sure you know what you're doing.
        */
        void addMovableObjectFactory(MovableObjectFactory* fact,
            bool overrideExisting = false);
        /** Removes a previously registered MovableObjectFactory.

            All instances of objects created by this factory will be destroyed
            before removing the factory (by calling back the factories
            'destroyInstance' method). The plugin writer is responsible for actually
            destroying the factory.
        */
        void removeMovableObjectFactory(MovableObjectFactory* fact);
        /// Checks whether a factory is registered for a given MovableObject type
        bool hasMovableObjectFactory(const String& typeName) const;
        /// Get a MovableObjectFactory for the given type
        MovableObjectFactory* getMovableObjectFactory(const String& typeName);
        /** Allocate the next MovableObject type flag.

            This is done automatically if MovableObjectFactory::requestTypeFlags
            returns true; don't call this manually unless you're sure you need to.
        */
        uint32 _allocateNextMovableObjectTypeFlag(void);

        typedef ConstMapIterator<MovableObjectFactoryMap> MovableObjectFactoryIterator;
        /** Return an iterator over all the MovableObjectFactory instances currently
            registered.
        */
        const MovableObjectFactoryMap& getMovableObjectFactories() const
        {
            return mMovableObjectFactoryMap;
        }

        /// @deprecated use getMovableObjectFactories
        OGRE_DEPRECATED MovableObjectFactoryIterator getMovableObjectFactoryIterator(void) const;

        /** Get the WorkQueue for processing background tasks.
            You are free to add new requests and handlers to this queue to
            process your custom background tasks using the shared thread pool.
            However, you must remember to assign yourself a new channel through
            which to process your tasks.
        */
        WorkQueue* getWorkQueue() const { return mWorkQueue.get(); }

        /** Replace the current work queue with an alternative.
            You can use this method to replace the internal implementation of
            WorkQueue with  your own, e.g. to externalise the processing of
            background events. Doing so will delete the existing queue and
            replace it with this one.
        @param queue The new WorkQueue instance. Root will delete this work queue
            at shutdown, so do not destroy it yourself.
        */
        void setWorkQueue(WorkQueue* queue);

        /** Sets whether blend indices information needs to be passed to the GPU.
            When entities use software animation they remove blend information such as
            indices and weights from the vertex buffers sent to the graphic card. This function
            can be used to limit which information is removed.
        @param redundant Set to true to remove blend indices information.
        */
        void setBlendIndicesGpuRedundant(bool redundant) {  mIsBlendIndicesGpuRedundant = redundant; }
        /** Returns whether blend indices information needs to be passed to the GPU
        see setBlendIndicesGpuRedundant() for more information
        */
        bool isBlendIndicesGpuRedundant() const { return mIsBlendIndicesGpuRedundant; }

        /** Sets whether blend weights information needs to be passed to the GPU.
        When entities use software animation they remove blend information such as
        indices and weights from the vertex buffers sent to the graphic card. This function
        can be used to limit which information is removed.
        @param redundant Set to true to remove blend weights information.
        */
        void setBlendWeightsGpuRedundant(bool redundant) {  mIsBlendWeightsGpuRedundant = redundant; }
        /** Returns whether blend weights information needs to be passed to the GPU
        see setBlendWeightsGpuRedundant() for more information
        */
        bool isBlendWeightsGpuRedundant() const { return mIsBlendWeightsGpuRedundant; }

        /** Set the default minimum pixel size for object to be rendered by
        @note
            To use this feature see Camera::setUseMinPixelSize()
        */
        void setDefaultMinPixelSize(Real pixelSize) { mDefaultMinPixelSize = pixelSize; }

        /** Get the default minimum pixel size for object to be rendered by
        */
        Real getDefaultMinPixelSize() { return mDefaultMinPixelSize; }
    };
    OGRE_DEBUG_NS_END
    /** @} */
    /** @} */
} // Namespace Ogre

#include "OgreHeaderSuffix.h"

#endif
