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
        void operator() () const { mQueue->_registerThreadWithRenderSystem(); }
        DefaultWorkQueue* mQueue;
    };

    //---------------------------------------------------------------------
    DefaultWorkQueue::DefaultWorkQueue(const String& name)
        : DefaultWorkQueueBase(name)
#if OGRE_NO_TBB_SCHEDULER == 0
        , mTaskScheduler(tbb::task_scheduler_init::deferred)
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
            "DefaultWorkQueue('" << mName << "') initialising.";

#if OGRE_NO_TBB_SCHEDULER == 0
        mTaskScheduler.initialize(mWorkerThreadCount);
#endif

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
#ifdef __cplusplus >= 201103L
    DefaultWorkQueue::~DefaultWorkQueue() noexcept(true)
#else
    DefaultWorkQueue::~DefaultWorkQueue()
#endif // __cplusplus
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

#if OGRE_NO_TBB_SCHEDULER == 0
        if (mTaskScheduler.is_active())
            mTaskScheduler.terminate();
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
        //  Root::getSingleton().getRenderSystem()->registerThread();
        //  _notifyThreadRegistered();
        //}

        // Task main function. Process a single request.

        _processNextRequest();
    }
    //---------------------------------------------------------------------
    void DefaultWorkQueue::notifyWorkers()
    {
        // create a new task
        mTaskGroup.run(*mWorkerFunc);
    }
}

