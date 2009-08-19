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
#include "OgreWorkQueue.h"
#include "Threading/OgreDefaultWorkQueueTBB.h"
#include "OgreLogManager.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"

namespace Ogre
{
	/// Worker function to register threads with the RenderSystem, if required
	struct RegisterRSWorker
	{
		RegisterRSWorker(DefaultWorkQueue* queue): mQueue(queue) { }
		void operator() () { mQueue->_registerThreadWithRenderSystem(); }
		DefaultWorkQueue* mQueue;
	};

	//---------------------------------------------------------------------
	DefaultWorkQueue::DefaultWorkQueue(const String& name)
		: DefaultWorkQueueBase(name)
		, mTaskScheduler(tbb::task_scheduler_init::deferred)
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

		LogManager::getSingleton().stream() <<
			"DefaultWorkQueue('" << mName << "') initialising.";

		mTaskScheduler.initialize(mWorkerThreadCount);

		if (mWorkerRenderSystemAccess)
		{
			Root::getSingleton().getRenderSystem()->preExtraThreadsStarted();
			RegisterRSWorker worker (this);
			// current thread need not be registered
			mRegisteredThreads.insert(tbb::this_tbb_thread::get_id());
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

		mIsRunning = true;
	}
	//---------------------------------------------------------------------
	void DefaultWorkQueue::_registerThreadWithRenderSystem()
	{
		{
			OGRE_LOCK_MUTEX(mRegisterRSMutex);
			tbb::tbb_thread::id cur = tbb::this_tbb_thread::get_id();
			if (mRegisteredThreads.find(cur) == mRegisteredThreads.end())
			{
				Root::getSingleton().getRenderSystem()->registerThread();
				mRegisteredThreads.insert(cur);
			}
		}

		tbb::this_tbb_thread::yield();
	}
	//---------------------------------------------------------------------
	DefaultWorkQueue::~DefaultWorkQueue()
	{
		shutdown();
	}
	//---------------------------------------------------------------------
	void DefaultWorkQueue::shutdown()
	{
		LogManager::getSingleton().stream() <<
			"DefaultWorkQueue('" << mName << "') shutting down.";

		mShuttingDown = true;
		mTaskGroup.cancel();
		abortAllRequests();

		// wait until all tasks have finished.
		mTaskGroup.wait();

		if (mTaskScheduler.is_active())
			mTaskScheduler.terminate();

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
		// create a new task
		mTaskGroup.run(mWorkerFunc);
	}
}

