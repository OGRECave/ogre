/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#include "OgreStableHeaders.h"
#include "OgreResourceManager.h"

#include "OgreException.h"
#include "OgreArchive.h"
#include "OgreArchiveManager.h"
#include "OgreStringVector.h"
#include "OgreStringConverter.h"
#include "OgreResourceGroupManager.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    ResourceManager::ResourceManager()
		: mNextHandle(1), mMemoryUsage(0), mLoadOrder(0), mVerbose(true)
    {
        // Init memory limit & usage
        mMemoryBudget = std::numeric_limits<unsigned long>::max();
    }
    //-----------------------------------------------------------------------
    ResourceManager::~ResourceManager()
    {
        removeAll();
    }
	//-----------------------------------------------------------------------
    ResourcePtr ResourceManager::create(const String& name, const String& group, 
		bool isManual, ManualResourceLoader* loader, const NameValuePairList* params)
	{
		// Call creation implementation
		ResourcePtr ret = ResourcePtr(
            createImpl(name, getNextHandle(), group, isManual, loader, params));
        if (params)
            ret->setParameterList(*params);

		addImpl(ret);
		// Tell resource group manager
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
		OGRE_LOCK_AUTO_MUTEX

		ResourcePtr res = getByName(name, group);
		bool created = false;
		if (res.isNull())
		{
			created = true;
			res = create(name, group, isManual, loader, params);
		}

		return ResourceCreateOrRetrieveResult(res, created);
	}
    //-----------------------------------------------------------------------
    ResourcePtr ResourceManager::prepare(const String& name, 
        const String& group, bool isManual, ManualResourceLoader* loader, 
        const NameValuePairList* loadParams)
    {
        ResourcePtr r = createOrRetrieve(name,group,isManual,loader,loadParams).first;
		// ensure prepared
        r->prepare();
        return r;
    }
    //-----------------------------------------------------------------------
    ResourcePtr ResourceManager::load(const String& name, 
        const String& group, bool isManual, ManualResourceLoader* loader, 
        const NameValuePairList* loadParams)
    {
        ResourcePtr r = createOrRetrieve(name,group,isManual,loader,loadParams).first;
		// ensure loaded
        r->load();
        return r;
    }
    //-----------------------------------------------------------------------
	void ResourceManager::addImpl( ResourcePtr& res )
	{
		OGRE_LOCK_AUTO_MUTEX

			std::pair<ResourceMap::iterator, bool> result;
		if(ResourceGroupManager::getSingleton().isResourceGroupInGlobalPool(res->getGroup()))
		{
			result = mResources.insert( ResourceMap::value_type( res->getName(), res ) );
		}
		else
		{
			ResourceWithGroupMap::iterator itGroup = mResourcesWithGroup.find(res->getGroup());

			// we will create the group if it doesn't exists in our list
			if( itGroup == mResourcesWithGroup.end())
			{
				ResourceMap dummy;
				mResourcesWithGroup.insert( ResourceWithGroupMap::value_type( res->getGroup(), dummy ) );
				itGroup = mResourcesWithGroup.find(res->getGroup());
			}
			result = itGroup->second.insert( ResourceMap::value_type( res->getName(), res ) );

		}

		if (!result.second)
		{
			// Attempt to resolve the collision
			if(ResourceGroupManager::getSingleton().getLoadingListener())
			{
				if(ResourceGroupManager::getSingleton().getLoadingListener()->resourceCollision(res.get(), this))
				{
					// Try to do the addition again, no seconds attempts to resolve collisions are allowed
					std::pair<ResourceMap::iterator, bool> result;
					if(ResourceGroupManager::getSingleton().isResourceGroupInGlobalPool(res->getGroup()))
					{
						result = mResources.insert( ResourceMap::value_type( res->getName(), res ) );
					}
					else
					{
						ResourceWithGroupMap::iterator itGroup = mResourcesWithGroup.find(res->getGroup());
						result = itGroup->second.insert( ResourceMap::value_type( res->getName(), res ) );
					}
					if (!result.second)
					{
						OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM, "Resource with the name " + res->getName() + 
							" already exists.", "ResourceManager::add");
					}

					std::pair<ResourceHandleMap::iterator, bool> resultHandle = 
						mResourcesByHandle.insert( ResourceHandleMap::value_type( res->getHandle(), res ) );
					if (!resultHandle.second)
					{
						OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM, "Resource with the handle " + 
							StringConverter::toString((long) (res->getHandle())) + 
							" already exists.", "ResourceManager::add");
					}
				}
			}
		}
		else
		{
			// Insert the handle
			std::pair<ResourceHandleMap::iterator, bool> resultHandle = 
				mResourcesByHandle.insert( ResourceHandleMap::value_type( res->getHandle(), res ) );
			if (!resultHandle.second)
			{
				OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM, "Resource with the handle " + 
					StringConverter::toString((long) (res->getHandle())) + 
					" already exists.", "ResourceManager::add");
			}
		}
	}
	//-----------------------------------------------------------------------
	void ResourceManager::removeImpl( ResourcePtr& res )
	{
		OGRE_LOCK_AUTO_MUTEX

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
	void ResourceManager::unload(const String& name)
	{
		ResourcePtr res = getByName(name);

		if (!res.isNull())
		{
			// Unload resource
			res->unload();

		}
	}
	//-----------------------------------------------------------------------
	void ResourceManager::unload(ResourceHandle handle)
	{
		ResourcePtr res = getByHandle(handle);

		if (!res.isNull())
		{
			// Unload resource
			res->unload();

		}
	}
	//-----------------------------------------------------------------------
	void ResourceManager::unloadAll(bool reloadableOnly)
	{
		OGRE_LOCK_AUTO_MUTEX

		ResourceMap::iterator i, iend;
		iend = mResources.end();
		for (i = mResources.begin(); i != iend; ++i)
		{
			if (!reloadableOnly || i->second->isReloadable())
			{
				i->second->unload();
			}
		}

	}
	//-----------------------------------------------------------------------
	void ResourceManager::reloadAll(bool reloadableOnly)
	{
		OGRE_LOCK_AUTO_MUTEX

		ResourceMap::iterator i, iend;
		iend = mResources.end();
		for (i = mResources.begin(); i != iend; ++i)
		{
			if (!reloadableOnly || i->second->isReloadable())
			{
				i->second->reload();
			}
		}

	}
    //-----------------------------------------------------------------------
    void ResourceManager::unloadUnreferencedResources(bool reloadableOnly)
    {
        OGRE_LOCK_AUTO_MUTEX

        ResourceMap::iterator i, iend;
        iend = mResources.end();
        for (i = mResources.begin(); i != iend; ++i)
        {
            // A use count of 3 means that only RGM and RM have references
            // RGM has one (this one) and RM has 2 (by name and by handle)
            if (i->second.useCount() == ResourceGroupManager::RESOURCE_SYSTEM_NUM_REFERENCE_COUNTS)
            {
                Resource* res = i->second.get();
                if (!reloadableOnly || res->isReloadable())
                {
                    res->unload();
                }
            }
        }
    }
    //-----------------------------------------------------------------------
    void ResourceManager::reloadUnreferencedResources(bool reloadableOnly)
    {
        OGRE_LOCK_AUTO_MUTEX

        ResourceMap::iterator i, iend;
        iend = mResources.end();
        for (i = mResources.begin(); i != iend; ++i)
        {
            // A use count of 3 means that only RGM and RM have references
            // RGM has one (this one) and RM has 2 (by name and by handle)
            if (i->second.useCount() == ResourceGroupManager::RESOURCE_SYSTEM_NUM_REFERENCE_COUNTS)
            {
                Resource* res = i->second.get();
                if (!reloadableOnly || res->isReloadable())
                {
                    res->reload();
                }
            }
        }
    }
    //-----------------------------------------------------------------------
    void ResourceManager::remove(ResourcePtr& res)
    {
        removeImpl(res);
    }
	//-----------------------------------------------------------------------
	void ResourceManager::remove(const String& name)
	{
		ResourcePtr res = getByName(name);

		if (!res.isNull())
		{
			removeImpl(res);
		}
	}
	//-----------------------------------------------------------------------
	void ResourceManager::remove(ResourceHandle handle)
	{
		ResourcePtr res = getByHandle(handle);

		if (!res.isNull())
		{
			removeImpl(res);
		}
	}
	//-----------------------------------------------------------------------
	void ResourceManager::removeAll(void)
	{
		OGRE_LOCK_AUTO_MUTEX

		mResources.clear();
		mResourcesWithGroup.clear();
		mResourcesByHandle.clear();
		// Notify resource group manager
		ResourceGroupManager::getSingleton()._notifyAllResourcesRemoved(this);
	}
    //-----------------------------------------------------------------------
    ResourcePtr ResourceManager::getByName(const String& name, const String& groupName /* = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME */)
    {
		OGRE_LOCK_AUTO_MUTEX
		
		ResourcePtr res;

		// if not in the global pool - get it from the grouped pool 
		if(!ResourceGroupManager::getSingleton().isResourceGroupInGlobalPool(groupName))
		{
			ResourceWithGroupMap::iterator itGroup = mResourcesWithGroup.find(groupName);

			if( itGroup != mResourcesWithGroup.end())
			{
				ResourceMap::iterator it = itGroup->second.find(name);

				if( it != itGroup->second.end())
				{
					res = it->second;
				}
			}
		}

		// if didn't find it the grouped pool - get it from the global pool 
		if (res.isNull())
		{

			ResourceMap::iterator it = mResources.find(name);

			if( it != mResources.end())
			{
				res = it->second;
			}
			else
			{
				// this is the case when we need to search also in the grouped hash
				if (groupName == ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME)
				{
					ResourceWithGroupMap::iterator iter = mResourcesWithGroup.begin();
					ResourceWithGroupMap::iterator iterE = mResourcesWithGroup.end();
					for ( ; iter != iterE ; iter++ )
					{
						ResourceMap::iterator it = iter->second.find(name);

						if( it != iter->second.end())
						{
							res = it->second;
							break;
						}
					}
				}
			}
		}
		

		return res;


    }
    //-----------------------------------------------------------------------
    ResourcePtr ResourceManager::getByHandle(ResourceHandle handle)
    {
		OGRE_LOCK_AUTO_MUTEX

        ResourceHandleMap::iterator it = mResourcesByHandle.find(handle);
        if (it == mResourcesByHandle.end())
        {
            return ResourcePtr();
        }
        else
        {
            return it->second;
        }
    }
    //-----------------------------------------------------------------------
    ResourceHandle ResourceManager::getNextHandle(void)
    {
		OGRE_LOCK_AUTO_MUTEX

        return mNextHandle++;
    }
    //-----------------------------------------------------------------------
    void ResourceManager::checkUsage(void)
    {
        // TODO Page out here?
    }
	//-----------------------------------------------------------------------
	void ResourceManager::_notifyResourceTouched(Resource* res)
	{
		// TODO
	}
	//-----------------------------------------------------------------------
	void ResourceManager::_notifyResourceLoaded(Resource* res)
	{
		OGRE_LOCK_AUTO_MUTEX

		mMemoryUsage += res->getSize();
	}
	//-----------------------------------------------------------------------
	void ResourceManager::_notifyResourceUnloaded(Resource* res)
	{
		OGRE_LOCK_AUTO_MUTEX

		mMemoryUsage -= res->getSize();
	}
	//-----------------------------------------------------------------------

}




