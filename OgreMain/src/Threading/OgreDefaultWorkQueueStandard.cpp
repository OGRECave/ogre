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
#include "Threading/OgreDefaultWorkQueueStandard.h"
#include "OgreLogManager.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"

namespace Ogre
{
	//---------------------------------------------------------------------
	DefaultWorkQueue::DefaultWorkQueue(const String& name)
	: DefaultWorkQueueBase(name)
	{
	}
	//---------------------------------------------------------------------
	DefaultWorkQueue::~DefaultWorkQueue()
	{
		shutdown();
	}
	//---------------------------------------------------------------------
	void DefaultWorkQueue::startup(bool forceRestart)
	{
		if (mIsRunning)
		{
			if (forceRestart)
				shutdown();
			else
				return;
		}

		mShuttingDown = false;

		LogManager::getSingleton().stream() <<
			"DefaultWorkQueue('" << mName << "') initialising on thread " <<
#if OGRE_THREAD_SUPPORT
			OGRE_THREAD_CURRENT_ID
#else
			"main"
#endif
			<< ".";

#if OGRE_THREAD_SUPPORT
		if (mWorkerRenderSystemAccess)
			Root::getSingleton().getRenderSystem()->preExtraThreadsStarted();

		mNumThreadsRegisteredWithRS = 0;
		for (uint8 i = 0; i < mWorkerThreadCount; ++i)
		{
			OGRE_THREAD_CREATE(t, mWorkerFunc);
			mWorkers.push_back(t);
		}

		if (mWorkerRenderSystemAccess)
		{
			OGRE_LOCK_MUTEX_NAMED(mInitMutex, initLock)
			// have to wait until all threads are registered with the render system
			while (mNumThreadsRegisteredWithRS < mWorkerThreadCount)
				OGRE_THREAD_WAIT(mInitSync, mInitMutex, initLock);

			Root::getSingleton().getRenderSystem()->postExtraThreadsStarted();

		}
#endif

		mIsRunning = true;
	}
	//---------------------------------------------------------------------
	void DefaultWorkQueue::notifyThreadRegistered()
	{
		OGRE_LOCK_MUTEX(mInitMutex)

		++mNumThreadsRegisteredWithRS;

		// wake up main thread
		OGRE_THREAD_NOTIFY_ALL(mInitSync);

	}
	//---------------------------------------------------------------------
	void DefaultWorkQueue::shutdown()
	{
		LogManager::getSingleton().stream() <<
			"DefaultWorkQueue('" << mName << "') shutting down on thread " <<
#if OGRE_THREAD_SUPPORT
			OGRE_THREAD_CURRENT_ID
#else
			"main"
#endif
			<< ".";

		mShuttingDown = true;
		abortAllRequests();
#if OGRE_THREAD_SUPPORT
		// wake all threads (they should check shutting down as first thing after wait)
		OGRE_THREAD_NOTIFY_ALL(mRequestCondition)

		// all our threads should have been woken now, so join
		for (WorkerThreadList::iterator i = mWorkers.begin(); i != mWorkers.end(); ++i)
		{
			(*i)->join();
			OGRE_THREAD_DESTROY(*i);
		}
		mWorkers.clear();
#endif

		mIsRunning = false;


	}
	//---------------------------------------------------------------------
	void DefaultWorkQueue::notifyWorkers()
	{
		// wake up waiting thread
		OGRE_THREAD_NOTIFY_ONE(mRequestCondition)
	}

	//---------------------------------------------------------------------
	void DefaultWorkQueue::waitForNextRequest()
	{
#if OGRE_THREAD_SUPPORT
		// Lock; note that OGRE_THREAD_WAIT will free the lock
		OGRE_LOCK_MUTEX_NAMED(mRequestMutex, queueLock);
		if (mRequestQueue.empty())
		{
			// frees lock and suspends the thread
			OGRE_THREAD_WAIT(mRequestCondition, mRequestMutex, queueLock);
		}
		// When we get back here, it's because we've been notified 
		// and thus the thread has been woken up. Lock has also been
		// re-acquired, but we won't use it. It's safe to try processing and fail
		// if another thread has got in first and grabbed the request
#endif

	}
	//---------------------------------------------------------------------
	void DefaultWorkQueue::_threadMain()
	{
		// default worker thread
#if OGRE_THREAD_SUPPORT
		LogManager::getSingleton().stream() << 
			"DefaultWorkQueue('" << getName() << "')::WorkerFunc - thread " 
			<< OGRE_THREAD_CURRENT_ID << " starting.";

		// Initialise the thread for RS if necessary
		if (mWorkerRenderSystemAccess)
		{
			Root::getSingleton().getRenderSystem()->registerThread();
			notifyThreadRegistered();
		}

		// Spin forever until we're told to shut down
		while (!isShuttingDown())
		{
			waitForNextRequest();
			_processNextRequest();
		}

		LogManager::getSingleton().stream() << 
			"DefaultWorkQueue('" << getName() << "')::WorkerFunc - thread " 
			<< OGRE_THREAD_CURRENT_ID << " stopped.";
#endif
	}

}
