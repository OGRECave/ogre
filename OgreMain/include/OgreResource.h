/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
#ifndef _Resource_H__
#define _Resource_H__

#include "OgrePrerequisites.h"
#include "OgreString.h"
#include "OgreSharedPtr.h"
#include "OgreStringInterface.h"
#include "OgreAtomicWrappers.h"

namespace Ogre {

    typedef unsigned long long int ResourceHandle;


	// Forward declaration
	class ManualResourceLoader;

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Resources
	*  @{
	*/
	/** Abstract class representing a loadable resource (e.g. textures, sounds etc)
        @remarks
            Resources are data objects that must be loaded and managed throughout
			an application. A resource might be a mesh, a texture, or any other
			piece of data - the key thing is that they must be identified by 
			a name which is unique, must be loaded only once,
			must be managed efficiently in terms of retrieval, and they may
			also be unloadable to free memory up when they have not been used for
			a while and the memory budget is under stress.
		@par
			All Resource instances must be a member of a resource group; see
			ResourceGroupManager for full details.
        @par
            Subclasses must implement:
			<ol>
			<li>A constructor, overriding the same parameters as the constructor
			    defined by this class. Subclasses are not allowed to define
				constructors with other parameters; other settings must be
				settable through accessor methods before loading.</li>
            <li>The loadImpl() and unloadImpl() methods - mSize must be set 
				after loadImpl()</li>
			<li>StringInterface ParamCommand and ParamDictionary setups
			    in order to allow setting of core parameters (prior to load)
				through a generic interface.</li>
			</ol>
    */
	class _OgreExport Resource : public StringInterface, public ResourceAlloc
    {
	public:
		OGRE_AUTO_MUTEX // public to allow external locking
		class Listener
		{
		public:
		   	Listener() {}
			virtual ~Listener() {}

			/** Callback to indicate that background loading has completed.
			@deprecated
				Use loadingComplete instead.
			*/
			virtual void backgroundLoadingComplete(Resource*) {}

			/** Callback to indicate that background preparing has completed.
			@deprecated
				Use preparingComplete instead.
			*/
			virtual void backgroundPreparingComplete(Resource*) {}

			/** Called whenever the resource finishes loading. 
			@remarks
				If a Resource has been marked as background loaded (@see Resource::setBackgroundLoaded), 
				the call does not itself occur in the thread which is doing the loading;
				when loading is complete a response indicator is placed with the
				ResourceGroupManager, which will then be sent back to the 
				listener as part of the application's primary frame loop thread.
			*/
			virtual void loadingComplete(Resource*) {}


			/** called whenever the resource finishes preparing (paging into memory).
			@remarks
				If a Resource has been marked as background loaded (@see Resource::setBackgroundLoaded)
				the call does not itself occur in the thread which is doing the preparing;
				when preparing is complete a response indicator is placed with the
				ResourceGroupManager, which will then be sent back to the 
				listener as part of the application's primary frame loop thread.
			*/
			virtual void preparingComplete(Resource*) {}

			/** Called whenever the resource has been unloaded. */
			virtual void unloadingComplete(Resource*) {}
		};
		
		/// Enum identifying the loading state of the resource
		enum LoadingState
		{
            /// Not loaded
            LOADSTATE_UNLOADED,
            /// Loading is in progress
            LOADSTATE_LOADING,
            /// Fully loaded
            LOADSTATE_LOADED,
            /// Currently unloading
            LOADSTATE_UNLOADING,
            /// Fully prepared
            LOADSTATE_PREPARED,
            /// Preparing is in progress
            LOADSTATE_PREPARING
		};
    protected:
		/// Creator
		ResourceManager* mCreator;
		/// Unique name of the resource
        String mName;
		/// The name of the resource group
		String mGroup;
		/// Numeric handle for more efficient look up than name
        ResourceHandle mHandle;
		/// Is the resource currently loaded?
        AtomicScalar<LoadingState> mLoadingState;
		/// Is this resource going to be background loaded? Only applicable for multithreaded
		volatile bool mIsBackgroundLoaded;
		/// The size of the resource in bytes
        size_t mSize;
		/// Is this file manually loaded?
		bool mIsManual;
		/// Origin of this resource (e.g. script name) - optional
		String mOrigin;
		/// Optional manual loader; if provided, data is loaded from here instead of a file
		ManualResourceLoader* mLoader;
		/// State count, the number of times this resource has changed state
		size_t mStateCount;

		typedef set<Listener*>::type ListenerList;
		ListenerList mListenerList;
		OGRE_MUTEX(mListenerListMutex)

		/** Protected unnamed constructor to prevent default construction. 
		*/
		Resource() 
			: mCreator(0), mHandle(0), mLoadingState(LOADSTATE_UNLOADED), 
			mIsBackgroundLoaded(false),	mSize(0), mIsManual(0), mLoader(0)
		{ 
		}

		/** Internal hook to perform actions before the load process, but
			after the resource has been marked as 'loading'.
		@note Mutex will have already been acquired by the loading thread.
			Also, this call will occur even when using a ManualResourceLoader 
			(when loadImpl is not actually called)
		*/
		virtual void preLoadImpl(void) {}
		/** Internal hook to perform actions after the load process, but
			before the resource has been marked as fully loaded.
		@note Mutex will have already been acquired by the loading thread.
			Also, this call will occur even when using a ManualResourceLoader 
			(when loadImpl is not actually called)
		*/
		virtual void postLoadImpl(void) {}

		/** Internal hook to perform actions before the unload process.
		@note Mutex will have already been acquired by the unloading thread.
		*/
		virtual void preUnloadImpl(void) {}
		/** Internal hook to perform actions after the unload process, but
		before the resource has been marked as fully unloaded.
		@note Mutex will have already been acquired by the unloading thread.
		*/
		virtual void postUnloadImpl(void) {}

        /** Internal implementation of the meat of the 'prepare' action. 
        */
        virtual void prepareImpl(void) {}
        /** Internal function for undoing the 'prepare' action.  Called when
            the load is completed, and when resources are unloaded when they
            are prepared but not yet loaded.
        */
        virtual void unprepareImpl(void) {}
		/** Internal implementation of the meat of the 'load' action, only called if this 
			resource is not being loaded from a ManualResourceLoader. 
		*/
		virtual void loadImpl(void) = 0;
		/** Internal implementation of the 'unload' action; called regardless of
			whether this resource is being loaded from a ManualResourceLoader. 
		*/
		virtual void unloadImpl(void) = 0;
		/** Calculate the size of a resource; this will only be called after 'load' */
		virtual size_t calculateSize(void) const = 0;

    public:
		/** Standard constructor.
		@param creator Pointer to the ResourceManager that is creating this resource
		@param name The unique name of the resource
		@param group The name of the resource group to which this resource belongs
		@param isManual Is this resource manually loaded? If so, you should really
			populate the loader parameter in order that the load process
			can call the loader back when loading is required. 
		@param loader Pointer to a ManualResourceLoader implementation which will be called
			when the Resource wishes to load (should be supplied if you set
			isManual to true). You can in fact leave this parameter null 
			if you wish, but the Resource will never be able to reload if 
			anything ever causes it to unload. Therefore provision of a proper
			ManualResourceLoader instance is strongly recommended.
		*/
		Resource(ResourceManager* creator, const String& name, ResourceHandle handle,
			const String& group, bool isManual = false, ManualResourceLoader* loader = 0);

        /** Virtual destructor. Shouldn't need to be overloaded, as the resource
            deallocation code should reside in unload()
            @see
                Resource::unload()
        */
        virtual ~Resource();

        /** Prepares the resource for load, if it is not already.  One can call prepare()
            before load(), but this is not required as load() will call prepare() 
            itself, if needed.  When OGRE_THREAD_SUPPORT==1 both load() and prepare() 
            are thread-safe.  When OGRE_THREAD_SUPPORT==2 however, only prepare() 
            is thread-safe.  The reason for this function is to allow a background 
            thread to do some of the loading work, without requiring the whole render
            system to be thread-safe.  The background thread would call
            prepare() while the main render loop would later call load().  So long as
            prepare() remains thread-safe, subclasses can arbitrarily split the work of
            loading a resource between load() and prepare().  It is best to try and
            do as much work in prepare(), however, since this will leave less work for
            the main render thread to do and thus increase FPS.
			@param backgroundThread Whether this is occurring in a background thread
        */
        virtual void prepare(bool backgroundThread = false);

     /** Loads the resource, if it is not already.
		@remarks
			If the resource is loaded from a file, loading is automatic. If not,
			if for example this resource gained it's data from procedural calls
			rather than loading from a file, then this resource will not reload 
			on it's own.
		@param backgroundThread Indicates whether the caller of this method is
			the background resource loading thread. 
			
        */
        virtual void load(bool backgroundThread = false);

		/** Reloads the resource, if it is already loaded.
		@remarks
			Calls unload() and then load() again, if the resource is already
			loaded. If it is not loaded already, then nothing happens.
		*/
		virtual void reload(void);

        /** Returns true if the Resource is reloadable, false otherwise.
        */
        virtual bool isReloadable(void) const
        {
            return !mIsManual || mLoader;
        }

        /** Is this resource manually loaded?
		*/
		virtual bool isManuallyLoaded(void) const
		{
			return mIsManual;
		}

		/** Unloads the resource; this is not permanent, the resource can be
			reloaded later if required.
        */
		virtual void unload(void);

        /** Retrieves info about the size of the resource.
        */
        virtual size_t getSize(void) const
        { 
            return mSize; 
        }

        /** 'Touches' the resource to indicate it has been used.
        */
        virtual void touch(void);

        /** Gets resource name.
        */
        virtual const String& getName(void) const 
        { 
            return mName; 
        }

        virtual ResourceHandle getHandle(void) const
        {
            return mHandle;
        }

        /** Returns true if the Resource has been prepared, false otherwise.
        */
        virtual bool isPrepared(void) const 
        { 
			// No lock required to read this state since no modify
            return (mLoadingState.get() == LOADSTATE_PREPARED); 
        }

        /** Returns true if the Resource has been loaded, false otherwise.
        */
        virtual bool isLoaded(void) const 
        { 
			// No lock required to read this state since no modify
            return (mLoadingState.get() == LOADSTATE_LOADED); 
        }

		/** Returns whether the resource is currently in the process of
			background loading.
		*/
		virtual bool isLoading() const
		{
			return (mLoadingState.get() == LOADSTATE_LOADING);
		}

		/** Returns the current loading state.
		*/
		virtual LoadingState getLoadingState() const
		{
			return mLoadingState.get();
		}



		/** Returns whether this Resource has been earmarked for background loading.
		@remarks
			This option only makes sense when you have built Ogre with 
			thread support (OGRE_THREAD_SUPPORT). If a resource has been marked
			for background loading, then it won't load on demand like normal
			when load() is called. Instead, it will ignore request to load()
			except if the caller indicates it is the background loader. Any
			other users of this resource should check isLoaded(), and if that
			returns false, don't use the resource and come back later.
		*/
		virtual bool isBackgroundLoaded(void) const { return mIsBackgroundLoaded; }

		/** Tells the resource whether it is background loaded or not.
		@remarks
			@see Resource::isBackgroundLoaded . Note that calling this only
			defers the normal on-demand loading behaviour of a resource, it
			does not actually set up a thread to make sure the resource gets
			loaded in the background. You should use ResourceBackgroundLoadingQueue
			to manage the actual loading (which will call this method itself).
		*/
		virtual void setBackgroundLoaded(bool bl) { mIsBackgroundLoaded = bl; }

		/** Escalates the loading of a background loaded resource. 
		@remarks
			If a resource is set to load in the background, but something needs
			it before it's been loaded, there could be a problem. If the user
			of this resource really can't wait, they can escalate the loading
			which basically pulls the loading into the current thread immediately.
			If the resource is already being loaded but just hasn't quite finished
			then this method will simply wait until the background load is complete.
		*/
		virtual void escalateLoading();

		/** Register a listener on this resource.
			@see Resource::Listener
		*/
		virtual void addListener(Listener* lis);

		/** Remove a listener on this resource.
			@see Resource::Listener
		*/
		virtual void removeListener(Listener* lis);

		/// Gets the group which this resource is a member of
		virtual const String& getGroup(void) { return mGroup; }

		/** Change the resource group ownership of a Resource.
		@remarks
			This method is generally reserved for internal use, although
			if you really know what you're doing you can use it to move
			this resource from one group to another.
		@param newGroup Name of the new group
		*/
		virtual void changeGroupOwnership(const String& newGroup);

		/// Gets the manager which created this resource
		virtual ResourceManager* getCreator(void) { return mCreator; }
		/** Get the origin of this resource, e.g. a script file name.
		@remarks
			This property will only contain something if the creator of
			this resource chose to populate it. Script loaders are advised
			to populate it.
		*/
		virtual const String& getOrigin(void) const { return mOrigin; }
		/// Notify this resource of it's origin
		virtual void _notifyOrigin(const String& origin) { mOrigin = origin; }

		/** Returns the number of times this resource has changed state, which 
			generally means the number of times it has been loaded. Objects that 
			build derived data based on the resource can check this value against 
			a copy they kept last time they built this derived data, in order to
			know whether it needs rebuilding. This is a nice way of monitoring
			changes without having a tightly-bound callback.
		*/
		virtual size_t getStateCount() const { return mStateCount; }

		/** Manually mark the state of this resource as having been changed.
		@remarks
			You only need to call this from outside if you explicitly want derived
			objects to think this object has changed. @see getStateCount.
		*/
		virtual void _dirtyState();


		/** Firing of loading complete event
		@remarks
			You should call this from the thread that runs the main frame loop 
			to avoid having to make the receivers of this event thread-safe.
			If you use Ogre's built in frame loop you don't need to call this
			yourself.
			@param wasBackgroundLoaded Whether this was a background loaded event
		*/
		virtual void _fireLoadingComplete(bool wasBackgroundLoaded);

		/** Firing of preparing complete event
		@remarks
			You should call this from the thread that runs the main frame loop 
			to avoid having to make the receivers of this event thread-safe.
			If you use Ogre's built in frame loop you don't need to call this
			yourself.
			@param wasBackgroundLoaded Whether this was a background loaded event
		*/
		virtual void _firePreparingComplete(bool wasBackgroundLoaded);

		/** Firing of unloading complete event
		@remarks
		You should call this from the thread that runs the main frame loop 
		to avoid having to make the receivers of this event thread-safe.
		If you use Ogre's built in frame loop you don't need to call this
		yourself.
		*/
		virtual void _fireUnloadingComplete(void);


    };

	/** Shared pointer to a Resource.
	@remarks
		This shared pointer allows many references to a resource to be held, and
		when the final reference is removed, the resource will be destroyed. 
		Note that the ResourceManager which created this Resource will be holding
		at least one reference, so this resource will not get destroyed until 
		someone removes the resource from the manager - this at least gives you
		strong control over when resources are freed. But the nature of the 
		shared pointer means that if anyone refers to the removed resource in the
		meantime, the resource will remain valid.
	@par
		You may well see references to ResourcePtr (i.e. ResourcePtr&) being passed 
		around internally within Ogre. These are 'weak references' ie they do 
		not increment the reference count on the Resource. This is done for 
		efficiency in temporary operations that shouldn't need to incur the 
		overhead of maintaining the reference count; however we don't recommend 
		you do it yourself since these references are not guaranteed to remain valid.
	*/
	typedef SharedPtr<Resource> ResourcePtr;

	/** Interface describing a manual resource loader.
	@remarks
		Resources are usually loaded from files; however in some cases you
		want to be able to set the data up manually instead. This provides
		some problems, such as how to reload a Resource if it becomes
		unloaded for some reason, either because of memory constraints, or
		because a device fails and some or all of the data is lost.
	@par
		This interface should be implemented by all classes which wish to
		provide manual data to a resource. They provide a pointer to themselves
		when defining the resource (via the appropriate ResourceManager), 
		and will be called when the Resource tries to load. 
		They should implement the loadResource method such that the Resource 
		is in the end set up exactly as if it had loaded from a file, 
		although the implementations will likely differ	between subclasses 
		of Resource, which is why no generic algorithm can be stated here. 
	@note
		The loader must remain valid for the entire life of the resource,
		so that if need be it can be called upon to re-load the resource
		at any time.
	*/
	class _OgreExport ManualResourceLoader
	{
	public:
		ManualResourceLoader() {}
		virtual ~ManualResourceLoader() {}

        /** Called when a resource wishes to load.  Note that this could get
         * called in a background thread even in just a semithreaded ogre
         * (OGRE_THREAD_SUPPORT==2).  Thus, you must not access the rendersystem from
         * this callback.  Do that stuff in loadResource.
		@param resource The resource which wishes to load
		*/
		virtual void prepareResource(Resource* resource)
                { (void)resource; }

		/** Called when a resource wishes to prepare.
		@param resource The resource which wishes to prepare
		*/
		virtual void loadResource(Resource* resource) = 0;
	};
	/** @} */
	/** @} */
}

#endif
