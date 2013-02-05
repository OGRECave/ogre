/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2011 Torus Knot Software Ltd

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
#include "OgreWorkQueue.h"
#include "Threading/OgreDefaultWorkQueueConcrt.h"
#include "OgreLogManager.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"

namespace Ogre
{
	/// Worker function to register threads with the RenderSystem, if required
	struct RegisterRSWorker
	{
		RegisterRSWorker(DefaultWorkQueue* queue)
			: mQueue(queue)
		{
		}
		void operator()() const
		{
			mQueue->_registerThreadWithRenderSystem(); 
		}
		DefaultWorkQueue* mQueue;
	};

	//---------------------------------------------------------------------
	DefaultWorkQueue::DefaultWorkQueue(const String& name)
		: DefaultWorkQueueBase(name)
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		, mShutdownEvent(0)
		, mTaskScheduler(0)
#endif
	{
		
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
			"DefaultWorkQueueConcrt('" << mName << "') initialising.";
		
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		/*
		// Create scheduler with maximum worker count (not available in WinRT)
		mTaskScheduler = ::Concurrency::Scheduler::Create(
			::Concurrency::SchedulerPolicy(2,
				::Concurrency::MinConcurrency, 1,
				::Concurrency::MaxConcurrency, mWorkerThreadCount
				));
		*/
			
		// Create event to register for shutdown event of scheduler
		//mShutdownEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
		//::Concurrency::CurrentScheduler::RegisterShutdownEvent(mShutdownEvent);
		//mTaskScheduler->RegisterShutdownEvent(mShutdownEvent);
#endif
		
		if (mWorkerRenderSystemAccess)
		{
			Root::getSingleton().getRenderSystem()->preExtraThreadsStarted();
			RegisterRSWorker worker(this);
			// current thread need not be registered
			mRegisteredThreads.insert(OGRE_THREAD_CURRENT_ID);
			while (mRegisteredThreads.size() < mWorkerThreadCount)
			{
				// spawn tasks until all worker threads have registered themselves with the RS
				for (size_t i = 0; i < mWorkerThreadCount*3; ++i)
				{
					mTaskGroup.run(worker);
				}
				mTaskGroup.wait();
			}
			Root::getSingleton().getRenderSystem()->postExtraThreadsStarted();
		}

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		// Restore previous scheduler as current (not available in WinRT)
		//::Concurrency::CurrentScheduler::Detach();
#endif

		mIsRunning = true;
	}
	//---------------------------------------------------------------------
	void DefaultWorkQueue::_registerThreadWithRenderSystem()
	{
		{
			OGRE_LOCK_MUTEX(mRegisterRSMutex);
			if (mRegisteredThreads.find(OGRE_THREAD_CURRENT_ID) == mRegisteredThreads.end())
			{
				Root::getSingleton().getRenderSystem()->registerThread();
				mRegisteredThreads.insert(OGRE_THREAD_CURRENT_ID);
			}
		}
		
		OGRE_THREAD_YIELD;
	}
	//---------------------------------------------------------------------
	DefaultWorkQueue::~DefaultWorkQueue()
	{
		shutdown();
	}
	//---------------------------------------------------------------------
	void DefaultWorkQueue::shutdown()
	{
		if (!mIsRunning)
		{
			return;
		}

		LogManager::getSingleton().stream() <<
			"DefaultWorkQueueConcrt('" << mName << "') shutting down.";

		mShuttingDown = true;
		mTaskGroup.cancel();
		abortAllRequests();

		// wait until all tasks have finished.
		mTaskGroup.wait();

		// Release the scheduler and wait for it to be shutdown
		// (not available in WinRT)
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		//if (mTaskScheduler != 0)
		//{
		//	//mTaskScheduler->Release();
		//	::WaitForSingleObject(mShutdownEvent, INFINITE);
		//	::CloseHandle(mShutdownEvent);
		//
		//	// Scheduler has destroyed itself
		//	mTaskScheduler = 0;
		//	mShutdownEvent = 0;
		//}
#endif

		if (mWorkerFunc)
		{
			OGRE_DELETE_T(mWorkerFunc, WorkerFunc, MEMCATEGORY_GENERAL);
			mWorkerFunc = 0;
		}
			
		mIsRunning = false;
	}
	//---------------------------------------------------------------------
	void DefaultWorkQueue::_threadMain()
	{
		//// Initialise the thread for RS if necessary
		//if (mWorkerRenderSystemAccess)
		//{
		//	Root::getSingleton().getRenderSystem()->registerThread();
		//	_notifyThreadRegistered();
		//}

		// Task main function. Process a single request.

		_processNextRequest();
	}
	//---------------------------------------------------------------------
	void DefaultWorkQueue::notifyWorkers()
	{
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		// Associate scheduler with current thread (not available in WinRT)
		//mTaskScheduler->Attach();
#endif
		
		// Run a new task
		mTaskGroup.run(*mWorkerFunc);
		
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		// Restore previous scheduler as current (not available in WinRT)
		//::Concurrency::CurrentScheduler::Detach();
#endif
	}
}