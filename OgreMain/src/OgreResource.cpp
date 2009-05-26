/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
// Ogre includes
#include "OgreStableHeaders.h"

#include "OgreResource.h"
#include "OgreResourceManager.h"
#include "OgreResourceBackgroundQueue.h"
#include "OgreLogManager.h"
#include "OgreException.h"

namespace Ogre 
{
	//-----------------------------------------------------------------------
	Resource::Resource(ResourceManager* creator, const String& name, ResourceHandle handle,
		const String& group, bool isManual, ManualResourceLoader* loader)
		: mCreator(creator), mName(name), mGroup(group), mHandle(handle), 
		mLoadingState(LOADSTATE_UNLOADED), mIsBackgroundLoaded(false),
		mSize(0), mIsManual(isManual), mLoader(loader), mStateCount(0)
	{
	}
	//-----------------------------------------------------------------------
	Resource::~Resource() 
	{ 
	}
	//-----------------------------------------------------------------------
	void Resource::escalateLoading()
	{
		// Just call load as if this is the background thread, locking on
		// load status will prevent race conditions
		load(true);
	}
	//-----------------------------------------------------------------------
	void Resource::prepare()
	{
        // quick check that avoids any synchronisation
        if (mLoadingState.get() != LOADSTATE_UNLOADED) return;

        // atomically do slower check to make absolutely sure,
        // and set the load state to PREPARING
		if (!mLoadingState.cas(LOADSTATE_UNLOADED,LOADSTATE_PREPARING))
			return;

		// Scope lock for actual loading
        try
		{

			OGRE_LOCK_AUTO_MUTEX

			if (mIsManual)
			{
                if (mLoader)
				{
					mLoader->prepareResource(this);
				}
				else
				{
					// Warn that this resource is not reloadable
					LogManager::getSingleton().stream(LML_TRIVIAL) 
						<< "WARNING: " << mCreator->getResourceType()  
						<< " instance '" << mName << "' was defined as manually "
						<< "loaded, but no manual loader was provided. This Resource "
						<< "will be lost if it has to be reloaded.";
				}
			}
			else
			{
				if (mGroup == ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME)
				{
					// Derive resource group
					changeGroupOwnership(
						ResourceGroupManager::getSingleton()
						.findGroupContainingResource(mName));
				}
				prepareImpl();
			}
		}
        catch (...)
        {
            mLoadingState.set(LOADSTATE_UNLOADED);
            throw;
        }

        mLoadingState.set(LOADSTATE_PREPARED);

		// Since we don't distinguish between GPU and CPU RAM, this
		// seems pointless
		//if(mCreator)
		//	mCreator->_notifyResourcePrepared(this);

		// Fire events (if not background)
		if (!mIsBackgroundLoaded)
			_firePreparingComplete();


	}

    void Resource::load(bool background)
	{
		// Early-out without lock (mitigate perf cost of ensuring loaded)
		// Don't load if:
		// 1. We're already loaded
		// 2. Another thread is loading right now
		// 3. We're marked for background loading and this is not the background
		//    loading thread we're being called by

        if (mIsBackgroundLoaded && !background) return;

        // quick check that avoids any synchronisation
        LoadingState old = mLoadingState.get();
        if (old!=LOADSTATE_UNLOADED && old!=LOADSTATE_PREPARED) return;

        // atomically do slower check to make absolutely sure,
        // and set the load state to LOADING
        if (!mLoadingState.cas(old,LOADSTATE_LOADING)) return;

		// Scope lock for actual loading
        try
		{

			OGRE_LOCK_AUTO_MUTEX



			if (mIsManual)
			{
                preLoadImpl();
				// Load from manual loader
				if (mLoader)
				{
					mLoader->loadResource(this);
				}
				else
				{
					// Warn that this resource is not reloadable
					LogManager::getSingleton().stream(LML_TRIVIAL) 
						<< "WARNING: " << mCreator->getResourceType()  
						<< " instance '" << mName << "' was defined as manually "
						<< "loaded, but no manual loader was provided. This Resource "
						<< "will be lost if it has to be reloaded.";
				}
                postLoadImpl();
			}
			else
			{

                if (old==LOADSTATE_UNLOADED)
                    prepareImpl();

                preLoadImpl();

                old = LOADSTATE_PREPARED;

				if (mGroup == ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME)
				{
					// Derive resource group
					changeGroupOwnership(
						ResourceGroupManager::getSingleton()
						.findGroupContainingResource(mName));
				}

				loadImpl();

                postLoadImpl();
			}

			// Calculate resource size
			mSize = calculateSize();

		}
        catch (...)
        {
            // Reset loading in-progress flag, in case failed for some reason.
            // We reset it to UNLOADED because the only other case is when
            // old == PREPARED in which case the loadImpl should wipe out
            // any prepared data since it might be invalid.
            mLoadingState.set(LOADSTATE_UNLOADED);
            // Re-throw
            throw;
        }

        mLoadingState.set(LOADSTATE_LOADED);
        _dirtyState();

		// Notify manager
		if(mCreator)
			mCreator->_notifyResourceLoaded(this);

		// Fire events, if not background
		if (!mIsBackgroundLoaded)
			_fireLoadingComplete();


	}
	//---------------------------------------------------------------------
	void Resource::_dirtyState()
	{
		// don't worry about threading here, count only ever increases so 
		// doesn't matter if we get a lost increment (one is enough)
		++mStateCount;	
	}
	//-----------------------------------------------------------------------
	void Resource::changeGroupOwnership(const String& newGroup)
	{
		if (mGroup != newGroup)
		{
			String oldGroup = mGroup;
			mGroup = newGroup;
			ResourceGroupManager::getSingleton()
				._notifyResourceGroupChanged(oldGroup, this);
		}
	}
	//-----------------------------------------------------------------------
	void Resource::unload(void) 
	{ 
		// Early-out without lock (mitigate perf cost of ensuring unloaded)
        LoadingState old = mLoadingState.get();
        if (old!=LOADSTATE_LOADED && old!=LOADSTATE_PREPARED) return;


        if (!mLoadingState.cas(old,LOADSTATE_UNLOADING)) return;

		// Scope lock for actual unload
		{
			OGRE_LOCK_AUTO_MUTEX
            if (old==LOADSTATE_PREPARED) {
                unprepareImpl();
            } else {
                preUnloadImpl();
                unloadImpl();
                postUnloadImpl();
            }
		}

        mLoadingState.set(LOADSTATE_UNLOADED);

		// Notify manager
		// Note if we have gone from PREPARED to UNLOADED, then we haven't actually
		// unloaded, i.e. there is no memory freed on the GPU.
		if(old==LOADSTATE_LOADED && mCreator)
			mCreator->_notifyResourceUnloaded(this);

		_fireUnloadingComplete();


	}
	//-----------------------------------------------------------------------
	void Resource::reload(void) 
	{ 
		OGRE_LOCK_AUTO_MUTEX
		if (mLoadingState.get() == LOADSTATE_LOADED)
		{
			unload();
			load();
		}
	}
	//-----------------------------------------------------------------------
	void Resource::touch(void) 
	{
        // make sure loaded
        load();

		if(mCreator)
			mCreator->_notifyResourceTouched(this);
	}
	//-----------------------------------------------------------------------
	void Resource::addListener(Resource::Listener* lis)
	{
		OGRE_LOCK_MUTEX(mListenerListMutex)
		mListenerList.push_back(lis);
	}
	//-----------------------------------------------------------------------
	void Resource::removeListener(Resource::Listener* lis)
	{
		// O(n) but not called very often
		OGRE_LOCK_MUTEX(mListenerListMutex)
		mListenerList.remove(lis);
	}
	//-----------------------------------------------------------------------
	void Resource::_fireLoadingComplete(void)
	{
		// Lock the listener list
		OGRE_LOCK_MUTEX(mListenerListMutex)
		for (ListenerList::iterator i = mListenerList.begin();
			i != mListenerList.end(); ++i)
		{
			// deprecated call
			if (isBackgroundLoaded())
				(*i)->backgroundLoadingComplete(this);

			(*i)->loadingComplete(this);
		}
	}
	//-----------------------------------------------------------------------
	void Resource::_firePreparingComplete(void)
	{
		// Lock the listener list
		OGRE_LOCK_MUTEX(mListenerListMutex)
		for (ListenerList::iterator i = mListenerList.begin();
			i != mListenerList.end(); ++i)
		{
			// deprecated call
			if (isBackgroundLoaded())
				(*i)->backgroundPreparingComplete(this);

			(*i)->preparingComplete(this);

		}
	}
	//-----------------------------------------------------------------------
	void Resource::_fireUnloadingComplete(void)
	{
		// Lock the listener list
		OGRE_LOCK_MUTEX(mListenerListMutex)
			for (ListenerList::iterator i = mListenerList.begin();
				i != mListenerList.end(); ++i)
			{

				(*i)->unloadingComplete(this);

			}
	}

}
