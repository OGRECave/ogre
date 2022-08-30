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
#ifndef _ResourceManager_H__
#define _ResourceManager_H__

#include "OgrePrerequisites.h"

#include "OgreResource.h"
#include "OgreResourceGroupManager.h"
#include "OgreCommon.h"
#include "OgreStringVector.h"
#include "OgreScriptLoader.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {

    /** Template class describing a simple pool of items.
     */
    template <typename T>
    class Pool
    {
    protected:
        typedef typename std::list<T> ItemList;
        ItemList mItems;
        OGRE_AUTO_MUTEX;
    public:
        Pool() {}
        virtual ~Pool() {}

        /** Get the next item from the pool.
         @return pair indicating whether there was a free item, and the item if so
         */
        virtual std::pair<bool, T> removeItem()
        {
            OGRE_LOCK_AUTO_MUTEX;
            std::pair<bool, T> ret;
            if (mItems.empty())
            {
                ret.first = false;
            }
            else
            {
                ret.first = true;
                ret.second = mItems.front();
                mItems.pop_front();
            }
            return ret;
        }

        /** Add a new item to the pool.
         */
        virtual void addItem(const T& i)
        {
            OGRE_LOCK_AUTO_MUTEX;
            mItems.push_front(i);
        }
        /// Clear the pool
        virtual void clear()
        {
            OGRE_LOCK_AUTO_MUTEX;
            mItems.clear();
        }
    };

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Resources
    *  @{
    */
    /** Defines a generic resource handler.
    @see @ref Resource-Management
    @note
        If OGRE_THREAD_SUPPORT is 1, this class is thread-safe.
    */
    class _OgreExport ResourceManager : public ScriptLoader, public ResourceAlloc
    {
    public:
        OGRE_AUTO_MUTEX; // public to allow external locking
        ResourceManager();
        virtual ~ResourceManager();

        /** Creates a new blank resource, but does not immediately load it.
        @param name The unique name of the %Resource
        @param group The name of the resource group to attach this new resource to
        @param isManual Is this resource manually loaded? If so, you should really
            populate the loader parameter in order that the load process
            can call the loader back when loading is required. 
        @param loader Pointer to a ManualLoader implementation which will be called
            when the Resource wishes to load (should be supplied if you set
            isManual to true). You can in fact leave this parameter null 
            if you wish, but the Resource will never be able to reload if 
            anything ever causes it to unload. Therefore provision of a proper
            ManualLoader instance is strongly recommended.
        @param createParams If any parameters are required to create an instance,
            they should be supplied here as name / value pairs
        */
        ResourcePtr createResource(const String& name, const String& group,
            bool isManual = false, ManualResourceLoader* loader = 0, 
            const NameValuePairList* createParams = 0);

        typedef std::pair<ResourcePtr, bool> ResourceCreateOrRetrieveResult;
        /** Create a new resource, or retrieve an existing one with the same
            name if it already exists.

            This method performs the same task as calling getByName() followed
            by create() if that returns null. The advantage is that it does it
            in one call so there are no race conditions if using multiple
            threads that could cause getByName() to return null, but create() to
            fail because another thread created a resource in between.
        @copydetails ResourceManager::createResource
        @return A pair, the first element being the pointer, and the second being 
            an indicator specifying whether the resource was newly created.
        */
        ResourceCreateOrRetrieveResult createOrRetrieve(const String& name,
            const String& group, bool isManual = false, 
            ManualResourceLoader* loader = 0, 
            const NameValuePairList* createParams = 0);
        
        /** Set a limit on the amount of memory this resource handler may use.

                If, when asked to load a new resource, the manager believes it will exceed this memory
                budget, it will temporarily unload a resource to make room for the new one. This unloading
                is not permanent and the Resource is not destroyed; it simply needs to be reloaded when
                next used.
        */
        void setMemoryBudget(size_t bytes);

        /** Get the limit on the amount of memory this resource handler may use.
        */
        size_t getMemoryBudget(void) const;

        /** Gets the current memory usage, in bytes. */
        size_t getMemoryUsage(void) const { return mMemoryUsage.load(); }

        /** Unloads a single resource by name.

            Unloaded resources are not removed, they simply free up their memory
            as much as they can and wait to be reloaded.
            @see ResourceGroupManager for unloading of resource groups.
        */
        void  unload(const String& name, const String& group OGRE_RESOURCE_GROUP_INIT);
        
        /** Unloads a single resource by handle.

            Unloaded resources are not removed, they simply free up their memory
            as much as they can and wait to be reloaded.
            @see ResourceGroupManager for unloading of resource groups.
        */
        void unload(ResourceHandle handle);

        /** Unloads all resources.

            Unloaded resources are not removed, they simply free up their memory
            as much as they can and wait to be reloaded.
            @see ResourceGroupManager for unloading of resource groups.
        @param reloadableOnly If true (the default), only unload the resource that
            is reloadable. Because some resources isn't reloadable, they will be
            unloaded but can't load them later. Thus, you might not want to them
            unloaded. Or, you might unload all of them, and then populate them
            manually later.
            @see Resource::isReloadable for resource is reloadable.
        */
        void unloadAll(bool reloadableOnly = true)
            { unloadAll(reloadableOnly ? Resource::LF_DEFAULT : Resource::LF_INCLUDE_NON_RELOADABLE); }

        /** Caused all currently loaded resources to be reloaded.

            All resources currently being held in this manager which are also
            marked as currently loaded will be unloaded, then loaded again.
        @param reloadableOnly If true (the default), only reload the resource that
            is reloadable. Because some resources isn't reloadable, they will be
            unloaded but can't loaded again. Thus, you might not want to them
            unloaded. Or, you might unload all of them, and then populate them
            manually later.
            @see Resource::isReloadable for resource is reloadable.
        */
        void reloadAll(bool reloadableOnly = true)
            { reloadAll(reloadableOnly ? Resource::LF_DEFAULT : Resource::LF_INCLUDE_NON_RELOADABLE); }

        /** Unload all resources which are not referenced by any other object.

            This method behaves like unloadAll, except that it only unloads resources
            which are not in use, ie not referenced by other objects. This allows you
            to free up some memory selectively whilst still keeping the group around
            (and the resources present, just not using much memory).
        @par
            Some referenced resource may exists 'weak' pointer to their sub-components
            (e.g. Entity held pointer to SubMesh), in this case, unload or reload that
            resource will cause dangerous pointer access. Use this function instead of
            unloadAll allows you avoid fail in those situations.
        @param reloadableOnly If true (the default), only unloads resources
            which can be subsequently automatically reloaded.
        */
        void unloadUnreferencedResources(bool reloadableOnly = true)
            { unloadAll(reloadableOnly ? Resource::LF_ONLY_UNREFERENCED : Resource::LF_ONLY_UNREFERENCED_INCLUDE_NON_RELOADABLE); }

        /** Caused all currently loaded but not referenced by any other object
            resources to be reloaded.

            This method behaves like reloadAll, except that it only reloads resources
            which are not in use, i.e. not referenced by other objects.
        @par
            Some referenced resource may exists 'weak' pointer to their sub-components
            (e.g. Entity held pointer to SubMesh), in this case, unload or reload that
            resource will cause dangerous pointer access. Use this function instead of
            reloadAll allows you avoid fail in those situations.
        @param reloadableOnly If true (the default), only reloads resources
            which can be subsequently automatically reloaded.
        */
        void reloadUnreferencedResources(bool reloadableOnly = true)
            { reloadAll(reloadableOnly ? Resource::LF_ONLY_UNREFERENCED : Resource::LF_ONLY_UNREFERENCED_INCLUDE_NON_RELOADABLE); }

        /** Unloads all resources.

            Unloaded resources are not removed, they simply free up their memory
            as much as they can and wait to be reloaded.
            @see ResourceGroupManager for unloading of resource groups.
        @param flags Allow to restrict processing to only reloadable and/or
            unreferenced resources.
            @see Resource::LoadingFlags for additional information.
        */
        virtual void unloadAll(Resource::LoadingFlags flags);

        /** Caused all currently loaded resources to be reloaded.

            All resources currently being held in this manager which are also
            marked as currently loaded will be unloaded, then loaded again.
        @param flags Allow to restrict processing to only reloadable and/or
            unreferenced resources. Additionally, reloading could be done with
            preserving some selected resource states that could be used elsewhere.
            @see Resource::LoadingFlags for additional information.
        */
        virtual void reloadAll(Resource::LoadingFlags flags);

        /** Remove a single resource.

            Removes a single resource, meaning it will be removed from the list
            of valid resources in this manager, also causing it to be unloaded. 
        @note
            The word 'Destroy' is not used here, since
            if any other pointers are referring to this resource, it will persist
            until they have finished with it; however to all intents and purposes
            it no longer exists and will likely get destroyed imminently.
        @note
            If you do have shared pointers to resources hanging around after the 
            ResourceManager is destroyed, you may get problems on destruction of
            these resources if they were relying on the manager (especially if
            it is a plugin). If you find you get problems on shutdown in the
            destruction of resources, try making sure you release all your
            shared pointers before you shutdown OGRE.
        */
        void remove(const ResourcePtr& r);

        /// @overload
        void remove(const String& name, const String& group OGRE_RESOURCE_GROUP_INIT);
        
        /// @overload
        void remove(ResourceHandle handle);
        /** Removes all resources.
        @note
            The word 'Destroy' is not used here, since
            if any other pointers are referring to these resources, they will persist
            until they have been finished with; however to all intents and purposes
            the resources no longer exist and will get destroyed imminently.
        @note
            If you do have shared pointers to resources hanging around after the 
            ResourceManager is destroyed, you may get problems on destruction of
            these resources if they were relying on the manager (especially if
            it is a plugin). If you find you get problems on shutdown in the
            destruction of resources, try making sure you release all your
            shared pointers before you shutdown OGRE.
        */
        virtual void removeAll(void);

        /** Remove all resources which are not referenced by any other object.

            This method behaves like removeAll, except that it only removes resources
            which are not in use, ie not referenced by other objects. This allows you
            to free up some memory selectively whilst still keeping the group around
            (and the resources present, just not using much memory).
        @par
            Some referenced resource may exists 'weak' pointer to their sub-components
            (e.g. Entity held pointer to SubMesh), in this case, remove or reload that
            resource will cause dangerous pointer access. Use this function instead of
            removeAll allows you avoid fail in those situations.
        @param reloadableOnly If true (the default), only removes resources
            which can be subsequently automatically reloaded.
        */
        virtual void removeUnreferencedResources(bool reloadableOnly = true);

        /** Retrieves a pointer to a resource by name, or null if the resource does not exist.
        */
        virtual ResourcePtr getResourceByName(const String& name, const String& groupName OGRE_RESOURCE_GROUP_INIT) const;

        /** Retrieves a pointer to a resource by handle, or null if the resource does not exist.
        */
        virtual ResourcePtr getByHandle(ResourceHandle handle) const;
        
        /// Returns whether the named resource exists in this manager
        bool resourceExists(const String& name, const String& group OGRE_RESOURCE_GROUP_INIT) const
        {
            return getResourceByName(name, group).get() != 0;
        }
        /// Returns whether a resource with the given handle exists in this manager
        bool resourceExists(ResourceHandle handle) const
        {
            return getByHandle(handle).get() != 0;
        }

        /** Notify this manager that a resource which it manages has been 
            'touched', i.e. used. 
        */
        virtual void _notifyResourceTouched(Resource* res);

        /** Notify this manager that a resource which it manages has been 
            loaded. 
        */
        virtual void _notifyResourceLoaded(Resource* res);

        /** Notify this manager that a resource which it manages has been 
            unloaded.
        */
        virtual void _notifyResourceUnloaded(Resource* res);

        /** Generic prepare method, used to create a Resource specific to this 
            ResourceManager without using one of the specialised 'prepare' methods
            (containing per-Resource-type parameters).
        @param name The name of the %Resource
        @param group The resource group to which this resource will belong
        @param isManual Is the resource to be manually loaded? If so, you should
            provide a value for the loader parameter
        @param loader The manual loader which is to perform the required actions
            when this resource is loaded; only applicable when you specify true
            for the previous parameter
        @param loadParams Optional pointer to a list of name/value pairs 
            containing loading parameters for this type of resource.
        @param backgroundThread Optional boolean which lets the load routine know if it
            is being run on the background resource loading thread
        */
        ResourcePtr prepare(const String& name,
            const String& group, bool isManual = false, 
            ManualResourceLoader* loader = 0, const NameValuePairList* loadParams = 0,
            bool backgroundThread = false);

        /** Generic load method, used to create a Resource specific to this 
            ResourceManager without using one of the specialised 'load' methods
            (containing per-Resource-type parameters).
        @copydetails ResourceManager::prepare()
        */
        ResourcePtr load(const String& name,
            const String& group, bool isManual = false, 
            ManualResourceLoader* loader = 0, const NameValuePairList* loadParams = 0,
            bool backgroundThread = false);

        const StringVector& getScriptPatterns(void) const override { return mScriptPatterns; }
        void parseScript(DataStreamPtr& stream, const String& groupName) override;
        Real getLoadingOrder(void) const override { return mLoadOrder; }

        /** Gets a string identifying the type of resource this manager handles. */
        const String& getResourceType(void) const { return mResourceType; }

        /** Sets whether this manager and its resources habitually produce log output */
        void setVerbose(bool v) { mVerbose = v; }

        /** Gets whether this manager and its resources habitually produce log output */
        bool getVerbose(void) { return mVerbose; }

        /** Definition of a pool of resources, which users can use to reuse similar
            resources many times without destroying and recreating them.

            This is a simple utility class which allows the reuse of resources
            between code which has a changing need for them. For example, 
        */
        class _OgreExport ResourcePool : public Pool<ResourcePtr>, public ResourceAlloc
        {
            String mName;
        public:
            ResourcePool(const String& name);
            ~ResourcePool();
            /// Get the name of the pool
            const String& getName() const;
            void clear() override;
        };
        
        /// Create a resource pool, or reuse one that already exists
        ResourcePool* getResourcePool(const String& name);
        /// Destroy a resource pool
        void destroyResourcePool(ResourcePool* pool);
        /// Destroy a resource pool
        void destroyResourcePool(const String& name);
        /// destroy all pools
        void destroyAllResourcePools();




    protected:

        /** Allocates the next handle. */
        ResourceHandle getNextHandle(void);

        /** Create a new resource instance compatible with this manager (no custom 
            parameters are populated at this point). 

            Subclasses must override this method and create a subclass of Resource.
        @param name The unique name of the resource
        @param handle The unique handle of the resource
        @param group The name of the resource group to attach this new resource to
        @param isManual Is this resource manually loaded? If so, you should really
            populate the loader parameter in order that the load process
            can call the loader back when loading is required. 
        @param loader Pointer to a ManualLoader implementation which will be called
            when the Resource wishes to load (should be supplied if you set
            isManual to true). You can in fact leave this parameter null 
            if you wish, but the Resource will never be able to reload if 
            anything ever causes it to unload. Therefore provision of a proper
            ManualLoader instance is strongly recommended.
        @param createParams If any parameters are required to create an instance,
            they should be supplied here as name / value pairs. These do not need 
            to be set on the instance (handled elsewhere), just used if required
            to differentiate which concrete class is created.

        */
        virtual Resource* createImpl(const String& name, ResourceHandle handle, 
            const String& group, bool isManual, ManualResourceLoader* loader, 
            const NameValuePairList* createParams) = 0;
        /** Add a newly created resource to the manager (note weak reference) */
        virtual void addImpl( ResourcePtr& res );
        /** Remove a resource from this manager; remove it from the lists. */
        virtual void removeImpl(const ResourcePtr& res );
        /** Checks memory usage and pages out if required. This is automatically done after a new resource is loaded.
        */
        void checkUsage(void);


    public:
        typedef std::unordered_map< String, ResourcePtr > ResourceMap;
        typedef std::unordered_map< String, ResourceMap > ResourceWithGroupMap;
        typedef std::map<ResourceHandle, ResourcePtr> ResourceHandleMap;
    protected:
        ResourceHandleMap mResourcesByHandle;
        ResourceMap mResources;
        ResourceWithGroupMap mResourcesWithGroup;
        size_t mMemoryBudget; /// In bytes
        std::atomic<ResourceHandle> mNextHandle;
        std::atomic<size_t> mMemoryUsage; /// In bytes

        bool mVerbose;

        // IMPORTANT - all subclasses must populate the fields below

        /// Patterns to use to look for scripts if supported (e.g. *.overlay)
        StringVector mScriptPatterns; 
        /// Loading order relative to other managers, higher is later
        Real mLoadOrder; 
        /// String identifying the resource type this manager handles
        String mResourceType; 

    public:
        typedef MapIterator<ResourceHandleMap> ResourceMapIterator;
        /** Returns an iterator over all resources in this manager. 
        @note
            Use of this iterator is NOT thread safe!
        */
        ResourceMapIterator getResourceIterator(void) 
        {
            return ResourceMapIterator(mResourcesByHandle.begin(), mResourcesByHandle.end());
        }

    protected:
        typedef std::map<String, ResourcePool*> ResourcePoolMap;
        ResourcePoolMap mResourcePoolMap;
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
