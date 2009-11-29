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

		mWorkerFunc = OGRE_NEW_T(WorkerFunc(this), MEMCATEGORY_GENERAL);

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
			OGRE_THREAD_CREATE(t, *mWorkerFunc);
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
		if( !mIsRunning )
			return;

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

		if (mWorkerFunc)
		{
			OGRE_DELETE_T(mWorkerFunc, WorkerFunc, MEMCATEGORY_GENERAL);
			mWorkerFunc = 0;
		}


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
