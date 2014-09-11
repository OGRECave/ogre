/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
		_fireLoadingComplete(true);
	}
	//-----------------------------------------------------------------------
	void Resource::prepare(bool background)
	{
        // quick check that avoids any synchronisation
        LoadingState old = mLoadingState.get();
        if (old != LOADSTATE_UNLOADED && old != LOADSTATE_PREPARING) return;

        // atomically do slower check to make absolutely sure,
        // and set the load state to PREPARING
		if (!mLoadingState.cas(LOADSTATE_UNLOADED,LOADSTATE_PREPARING))
		{
			while( mLoadingState.get() == LOADSTATE_PREPARING )
			{
                            OGRE_LOCK_AUTO_MUTEX;
			}

			LoadingState state = mLoadingState.get();
			if( state != LOADSTATE_PREPARED && state != LOADSTATE_LOADING && state != LOADSTATE_LOADED )
			{
				OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Another thread failed in resource operation",
					"Resource::prepare");
			}
			return;
		}

		// Scope lock for actual loading
        try
		{

                    OGRE_LOCK_AUTO_MUTEX;

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
		if (!background)
			_firePreparingComplete(false);


	}
	//---------------------------------------------------------------------
    void Resource::load(bool background)
	{
		// Early-out without lock (mitigate perf cost of ensuring loaded)
		// Don't load if:
		// 1. We're already loaded
		// 2. Another thread is loading right now
		// 3. We're marked for background loading and this is not the background
		//    loading thread we're being called by

        if (mIsBackgroundLoaded && !background) return;

		// This next section is to deal with cases where 2 threads are fighting over
		// who gets to prepare / load - this will only usually happen if loading is escalated
		bool keepChecking = true;
		LoadingState old = LOADSTATE_UNLOADED;
		while (keepChecking)
		{
			// quick check that avoids any synchronisation
			old = mLoadingState.get();

			if ( old == LOADSTATE_PREPARING )
			{
				while( mLoadingState.get() == LOADSTATE_PREPARING )
				{
                                    OGRE_LOCK_AUTO_MUTEX;
				}
				old = mLoadingState.get();
			}

			if (old!=LOADSTATE_UNLOADED && old!=LOADSTATE_PREPARED && old!=LOADSTATE_LOADING) return;

			// atomically do slower check to make absolutely sure,
			// and set the load state to LOADING
			if (old==LOADSTATE_LOADING || !mLoadingState.cas(old,LOADSTATE_LOADING))
			{
				while( mLoadingState.get() == LOADSTATE_LOADING )
				{
                                    OGRE_LOCK_AUTO_MUTEX;
				}

				LoadingState state = mLoadingState.get();
				if( state == LOADSTATE_PREPARED || state == LOADSTATE_PREPARING )
				{
					// another thread is preparing, loop around
					continue;
				}
				else if( state != LOADSTATE_LOADED )
				{
					OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Another thread failed in resource operation",
						"Resource::load");
				}
				return;
			}
			keepChecking = false;
		}

		// Scope lock for actual loading
        try
		{

                    OGRE_LOCK_AUTO_MUTEX;



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
		if (!background)
			_fireLoadingComplete(false);


	}
    //---------------------------------------------------------------------
    size_t Resource::calculateSize(void) const
    {
        size_t memSize = 0;
        memSize += sizeof(ResourceManager);
        memSize += sizeof(ManualResourceLoader);
        memSize += sizeof(ResourceHandle);
        memSize += mName.size() * sizeof(char);
        memSize += mGroup.size() * sizeof(char);
        memSize += mOrigin.size() * sizeof(char);
        memSize += sizeof(size_t) * 2;
        memSize += sizeof(bool) * 2;
        memSize += sizeof(Listener) * mListenerList.size();
        memSize += sizeof(AtomicScalar<LoadingState>);

        return memSize;
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
                    OGRE_LOCK_AUTO_MUTEX;
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
            OGRE_LOCK_AUTO_MUTEX;
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
            OGRE_LOCK_MUTEX(mListenerListMutex);
		mListenerList.insert(lis);
	}
	//-----------------------------------------------------------------------
	void Resource::removeListener(Resource::Listener* lis)
	{
		// O(n) but not called very often
            OGRE_LOCK_MUTEX(mListenerListMutex);
		mListenerList.erase(lis);
	}
	//-----------------------------------------------------------------------
	void Resource::_fireLoadingComplete(bool wasBackgroundLoaded)
	{
		// Lock the listener list
            OGRE_LOCK_MUTEX(mListenerListMutex);
		for (ListenerList::iterator i = mListenerList.begin();
			i != mListenerList.end(); ++i)
		{
			// deprecated call
			if (wasBackgroundLoaded)
				(*i)->backgroundLoadingComplete(this);

			(*i)->loadingComplete(this);
		}
	}
	//-----------------------------------------------------------------------
	void Resource::_firePreparingComplete(bool wasBackgroundLoaded)
	{
		// Lock the listener list
            OGRE_LOCK_MUTEX(mListenerListMutex);
		for (ListenerList::iterator i = mListenerList.begin();
			i != mListenerList.end(); ++i)
		{
			// deprecated call
			if (wasBackgroundLoaded)
				(*i)->backgroundPreparingComplete(this);

			(*i)->preparingComplete(this);

		}
	}
	//-----------------------------------------------------------------------
	void Resource::_fireUnloadingComplete(void)
	{
		// Lock the listener list
            OGRE_LOCK_MUTEX(mListenerListMutex);
			for (ListenerList::iterator i = mListenerList.begin();
				i != mListenerList.end(); ++i)
			{

				(*i)->unloadingComplete(this);

			}
	}

}
