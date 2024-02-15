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
#include "OgreStableHeaders.h"
#include "OgreResourceManager.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    ResourceManager::ResourceManager()
        : mNextHandle(1), mMemoryUsage(0), mVerbose(true), mLoadOrder(0)
    {
        // Init memory limit & usage
        mMemoryBudget = std::numeric_limits<unsigned long>::max();
    }
    //-----------------------------------------------------------------------
    ResourceManager::~ResourceManager()
    {
        destroyAllResourcePools();
        removeAll();
    }
    void ResourceManager::parseScript(DataStreamPtr& stream, const String& groupName)
    {
        ScriptCompilerManager::getSingleton().parseScript(stream, groupName);
    }
    //-----------------------------------------------------------------------
    ResourcePtr ResourceManager::createResource(const String& name, const String& group,
        bool isManual, ManualResourceLoader* loader, const NameValuePairList* params)
    {
        OgreAssert(!name.empty(), "resource name must not be empty");

        // Call creation implementation
        ResourcePtr ret = ResourcePtr(
            createImpl(name, getNextHandle(), group, isManual, loader, params));
        if (params)
            ret->setParameterList(*params);

        addImpl(ret);
        // Tell resource group manager
        if(ret)
            ResourceGroupManager::getSingleton()._notifyResourceCreated(ret);
        return ret;

    }
    //-----------------------------------------------------------------------
    ResourceManager::ResourceCreateOrRetrieveResult 
    ResourceManager::createOrRetrieve(
        const String& name, const String& group, 
        bool isManual, ManualResourceLoader* loader, 
        const NameValuePairList* params)
    {
        // Lock for the whole get / insert
            OGRE_LOCK_AUTO_MUTEX;

        ResourcePtr res = getResourceByName(name, group);
        bool created = false;
        if (!res)
        {
            created = true;
            res = createResource(name, group, isManual, loader, params);
        }

        return ResourceCreateOrRetrieveResult(res, created);
    }
    //-----------------------------------------------------------------------
    ResourcePtr ResourceManager::prepare(const String& name, 
        const String& group, bool isManual, ManualResourceLoader* loader, 
        const NameValuePairList* loadParams, bool backgroundThread)
    {
        ResourcePtr r = createOrRetrieve(name,group,isManual,loader,loadParams).first;
        // ensure prepared
        r->prepare(backgroundThread);
        return r;
    }
    //-----------------------------------------------------------------------
    ResourcePtr ResourceManager::load(const String& name, 
        const String& group, bool isManual, ManualResourceLoader* loader, 
        const NameValuePairList* loadParams, bool backgroundThread)
    {
        ResourcePtr r = createOrRetrieve(name,group,isManual,loader,loadParams).first;
        // ensure loaded
        r->load(backgroundThread);

        return r;
    }
    //-----------------------------------------------------------------------
    void ResourceManager::addImpl( ResourcePtr& res )
    {
            OGRE_LOCK_AUTO_MUTEX;

            std::pair<ResourceMap::iterator, bool> result;
        if(ResourceGroupManager::getSingleton().isResourceGroupInGlobalPool(res->getGroup()))
        {
            result = mResources.emplace(res->getName(), res);
        }
        else
        {
            // we will create the group if it doesn't exists in our list
            auto resgroup = mResourcesWithGroup.emplace(res->getGroup(), ResourceMap()).first;
            result = resgroup->second.emplace(res->getName(), res);
        }

        // Attempt to resolve the collision
        ResourceLoadingListener* listener = ResourceGroupManager::getSingleton().getLoadingListener();
        if (!result.second && listener)
        {
            if(listener->resourceCollision(res.get(), this) == false)
            {
                // explicitly use previous instance and destroy current
                res.reset();
                return;
            }

            // Try to do the addition again, no seconds attempts to resolve collisions are allowed
            if(ResourceGroupManager::getSingleton().isResourceGroupInGlobalPool(res->getGroup()))
            {
                result = mResources.emplace(res->getName(), res);
            }
            else
            {
                auto resgroup = mResourcesWithGroup.emplace(res->getGroup(), ResourceMap()).first;
                result = resgroup->second.emplace(res->getName(), res);
            }
        }

        if (!result.second)
        {
            OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM, getResourceType()+" with the name " + res->getName() +
                " already exists.", "ResourceManager::add");
        }

        // Insert the handle
        std::pair<ResourceHandleMap::iterator, bool> resultHandle = mResourcesByHandle.emplace(res->getHandle(), res);
        if (!resultHandle.second)
        {
            OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM, getResourceType()+" with the handle " +
                StringConverter::toString((long) (res->getHandle())) +
                " already exists.", "ResourceManager::add");
        }
    }
    //-----------------------------------------------------------------------
    void ResourceManager::removeImpl(const ResourcePtr& res )
    {
        OgreAssert(res, "attempting to remove nullptr");

#if OGRE_RESOURCEMANAGER_STRICT
        if (res->getCreator() != this)
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Resource '" + res->getName() + "' was not created by the '" +
                                                          getResourceType() + "' ResourceManager");
#endif

        OGRE_LOCK_AUTO_MUTEX;

        if(ResourceGroupManager::getSingleton().isResourceGroupInGlobalPool(res->getGroup()))
        {
            ResourceMap::iterator nameIt = mResources.find(res->getName());
            if (nameIt != mResources.end())
            {
                mResources.erase(nameIt);
            }
        }
        else
        {
            ResourceWithGroupMap::iterator groupIt = mResourcesWithGroup.find(res->getGroup());
            if (groupIt != mResourcesWithGroup.end())
            {
                ResourceMap::iterator nameIt = groupIt->second.find(res->getName());
                if (nameIt != groupIt->second.end())
                {
                    groupIt->second.erase(nameIt);
                }

                if (groupIt->second.empty())
                {
                    mResourcesWithGroup.erase(groupIt);
                }
            }
        }

        ResourceHandleMap::iterator handleIt = mResourcesByHandle.find(res->getHandle());
        if (handleIt != mResourcesByHandle.end())
        {
            mResourcesByHandle.erase(handleIt);
        }
        // Tell resource group manager
        ResourceGroupManager::getSingleton()._notifyResourceRemoved(res);
    }
    //-----------------------------------------------------------------------
    void ResourceManager::setMemoryBudget( size_t bytes)
    {
        // Update limit & check usage
        mMemoryBudget = bytes;
        checkUsage();
    }
    //-----------------------------------------------------------------------
    size_t ResourceManager::getMemoryBudget(void) const
    {
        return mMemoryBudget;
    }
    //-----------------------------------------------------------------------
    void ResourceManager::unload(const String& name, const String& group)
    {
        ResourcePtr res = getResourceByName(name, group);

#if OGRE_RESOURCEMANAGER_STRICT
        if (!res)
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "attempting to unload unknown resource: " + name + " in group " + group);
#endif

        if (res)
        {
            res->unload();
        }
    }
    //-----------------------------------------------------------------------
    void ResourceManager::unload(ResourceHandle handle)
    {
        ResourcePtr res = getByHandle(handle);

#if OGRE_RESOURCEMANAGER_STRICT
        OgreAssert(res, "attempting to unload unknown resource");
#endif

        if (res)
        {
            res->unload();
        }
    }
    //-----------------------------------------------------------------------
    void ResourceManager::unloadAll(Resource::LoadingFlags flags)
    {
        OGRE_LOCK_AUTO_MUTEX;

        bool reloadableOnly = (flags & Resource::LF_INCLUDE_NON_RELOADABLE) == 0;
        bool unreferencedOnly = (flags & Resource::LF_ONLY_UNREFERENCED) != 0;
        for (auto& r : mResources)
        {
            // A use count of 3 means that only RGM and RM have references
            // RGM has one (this one) and RM has 2 (by name and by handle)
            if (!unreferencedOnly || r.second.use_count() == ResourceGroupManager::RESOURCE_SYSTEM_NUM_REFERENCE_COUNTS)
            {
                Resource* res = r.second.get();
                if (!reloadableOnly || res->isReloadable())
                {
                    res->unload();
                }
            }
        }
    }
    //-----------------------------------------------------------------------
    void ResourceManager::reloadAll(Resource::LoadingFlags flags)
    {
        OGRE_LOCK_AUTO_MUTEX;

        bool reloadableOnly = (flags & Resource::LF_INCLUDE_NON_RELOADABLE) == 0;
        bool unreferencedOnly = (flags & Resource::LF_ONLY_UNREFERENCED) != 0;
        for (auto& r : mResources)
        {
            // A use count of 3 means that only RGM and RM have references
            // RGM has one (this one) and RM has 2 (by name and by handle)
            if (!unreferencedOnly || r.second.use_count() == ResourceGroupManager::RESOURCE_SYSTEM_NUM_REFERENCE_COUNTS)
            {
                Resource* res = r.second.get();
                if (!reloadableOnly || res->isReloadable())
                {
                    res->reload(flags);
                }
            }
        }
    }
    //-----------------------------------------------------------------------
    void ResourceManager::remove(const ResourcePtr& res)
    {
        removeImpl(res);
    }
    //-----------------------------------------------------------------------
    void ResourceManager::remove(const String& name, const String& group)
    {
        ResourcePtr res = getResourceByName(name, group);

#if OGRE_RESOURCEMANAGER_STRICT
        if (!res)
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "attempting to remove unknown resource: " + name + " in group " + group);
#endif

        if (res)
        {
            removeImpl(res);
        }
    }
    //-----------------------------------------------------------------------
    void ResourceManager::remove(ResourceHandle handle)
    {
        ResourcePtr res = getByHandle(handle);

#if OGRE_RESOURCEMANAGER_STRICT
        OgreAssert(res, "attempting to remove unknown resource");
#endif

        if (res)
        {
            removeImpl(res);
        }
    }
    //-----------------------------------------------------------------------
    void ResourceManager::removeAll(void)
    {
        OGRE_LOCK_AUTO_MUTEX;

        mResources.clear();
        mResourcesWithGroup.clear();
        mResourcesByHandle.clear();
        // Notify resource group manager
        ResourceGroupManager::getSingleton()._notifyAllResourcesRemoved(this);
    }
    //-----------------------------------------------------------------------
    void ResourceManager::removeUnreferencedResources(bool reloadableOnly)
    {
        OGRE_LOCK_AUTO_MUTEX;

        ResourceMap::iterator i, iend;
        iend = mResources.end();
        for (i = mResources.begin(); i != iend;)
        {
            // A use count of 3 means that only RGM and RM have references
            // RGM has one (this one) and RM has 2 (by name and by handle)
            if (i->second.use_count() == ResourceGroupManager::RESOURCE_SYSTEM_NUM_REFERENCE_COUNTS)
            {
                Resource* res = (i++)->second.get();
                if (!reloadableOnly || res->isReloadable())
                {
                    remove(res->getHandle());
                }
            }
            else {
                ++i;
            }
        }
    }
    //-----------------------------------------------------------------------
    ResourcePtr ResourceManager::getResourceByName(const String& name, const String& groupName) const
    {
        OGRE_LOCK_AUTO_MUTEX;

        // resource should be in global pool
        bool isGlobal = ResourceGroupManager::getSingleton().isResourceGroupInGlobalPool(groupName);

        if(isGlobal)
        {
            auto it = mResources.find(name);
            if( it != mResources.end())
            {
                return it->second;
            }
        }

        // look in all grouped pools
        if (groupName == ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME)
        {
            for (auto& r : mResourcesWithGroup)
            {
                auto resMapIt = r.second.find(name);

                if( resMapIt != r.second.end())
                {
                    return resMapIt->second;
                }
            }
        }
        else if (!isGlobal)
        {
            // look in the grouped pool
            auto itGroup = mResourcesWithGroup.find(groupName);
            if (itGroup != mResourcesWithGroup.end())
            {
                auto it = itGroup->second.find(name);

                if( it != itGroup->second.end())
                {
                    return it->second;
                }
            }

#if !OGRE_RESOURCEMANAGER_STRICT
            // fall back to global
            auto it = mResources.find(name);
            if( it != mResources.end())
            {
                return it->second;
            }
#endif
        }
    
        return ResourcePtr();
    }
    //-----------------------------------------------------------------------
    ResourcePtr ResourceManager::getByHandle(ResourceHandle handle) const
    {
        OGRE_LOCK_AUTO_MUTEX;
        auto it = mResourcesByHandle.find(handle);
        return it == mResourcesByHandle.end() ? ResourcePtr() : it->second;
    }
    //-----------------------------------------------------------------------
    ResourceHandle ResourceManager::getNextHandle(void)
    {
        // This is an atomic operation and hence needs no locking
        return mNextHandle++;
    }
    //-----------------------------------------------------------------------
    void ResourceManager::checkUsage(void)
    {
        if (getMemoryUsage() > mMemoryBudget)
        {
            OGRE_LOCK_AUTO_MUTEX;
            // unload unreferenced resources until we are within our budget again
            ResourceMap::iterator i, iend;
            iend = mResources.end();
            for (i = mResources.begin(); i != iend && getMemoryUsage() > mMemoryBudget; ++i)
            {
                // A use count of 3 means that only RGM and RM have references
                // RGM has one (this one) and RM has 2 (by name and by handle)
                if (i->second.use_count() == ResourceGroupManager::RESOURCE_SYSTEM_NUM_REFERENCE_COUNTS)
                {
                    Resource* res = i->second.get();
                    if (res->isReloadable())
                    {
                        res->unload();
                    }
                }
            }
        }
    }
    //-----------------------------------------------------------------------
    void ResourceManager::_notifyResourceTouched(Resource* res)
    {
        // TODO
    }
    //-----------------------------------------------------------------------
    void ResourceManager::_notifyResourceLoaded(Resource* res)
    {
        mMemoryUsage += res->getSize();
        checkUsage();
    }
    //-----------------------------------------------------------------------
    void ResourceManager::_notifyResourceUnloaded(Resource* res)
    {
        mMemoryUsage -= res->getSize();
    }
    //---------------------------------------------------------------------
    ResourceManager::ResourcePool* ResourceManager::getResourcePool(const String& name)
    {
        OGRE_LOCK_AUTO_MUTEX;

        ResourcePoolMap::iterator i = mResourcePoolMap.find(name);
        if (i == mResourcePoolMap.end())
        {
            i = mResourcePoolMap.insert(ResourcePoolMap::value_type(name, 
                OGRE_NEW ResourcePool(name))).first;
        }
        return i->second;

    }
    //---------------------------------------------------------------------
    void ResourceManager::destroyResourcePool(ResourcePool* pool)
    {
        OgreAssert(pool, "Cannot destroy a null ResourcePool");

        OGRE_LOCK_AUTO_MUTEX;

        ResourcePoolMap::iterator i = mResourcePoolMap.find(pool->getName());
        if (i != mResourcePoolMap.end())
            mResourcePoolMap.erase(i);

        OGRE_DELETE pool;
        
    }
    //---------------------------------------------------------------------
    void ResourceManager::destroyResourcePool(const String& name)
    {
        OGRE_LOCK_AUTO_MUTEX;

        ResourcePoolMap::iterator i = mResourcePoolMap.find(name);
        if (i != mResourcePoolMap.end())
        {
            OGRE_DELETE i->second;
            mResourcePoolMap.erase(i);
        }

    }
    //---------------------------------------------------------------------
    void ResourceManager::destroyAllResourcePools()
    {
        OGRE_LOCK_AUTO_MUTEX;

        for (auto & i : mResourcePoolMap)
            OGRE_DELETE i.second;

        mResourcePoolMap.clear();
    }
    //-----------------------------------------------------------------------
    //---------------------------------------------------------------------
    ResourceManager::ResourcePool::ResourcePool(const String& name)
        : mName(name)
    {

    }
    //---------------------------------------------------------------------
    ResourceManager::ResourcePool::~ResourcePool()
    {
        clear();
    }
    //---------------------------------------------------------------------
    const String& ResourceManager::ResourcePool::getName() const
    {
        return mName;
    }
    //---------------------------------------------------------------------
    void ResourceManager::ResourcePool::clear()
    {
            OGRE_LOCK_AUTO_MUTEX;
        for (auto & i : mItems)
        {
            i->getCreator()->remove(i->getHandle());
        }
        mItems.clear();
    }
}




