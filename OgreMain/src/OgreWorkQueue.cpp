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
#include "OgreTimer.h"

namespace Ogre {
    void WorkQueue::processMainThreadTasks()
    {
        OGRE_IGNORE_DEPRECATED_BEGIN
        processResponses();
        OGRE_IGNORE_DEPRECATED_END
    }
    //---------------------------------------------------------------------
    WorkQueue::Request::Request(uint16 channel, uint16 rtype, const Any& rData, uint8 retry, RequestID rid)
        : mChannel(channel), mType(rtype), mData(rData), mRetryCount(retry), mID(rid), mAborted(false)
    {

    }
    //---------------------------------------------------------------------
    WorkQueue::Request::~Request()
    {

    }
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    WorkQueue::Response::Response(const Request* rq, bool success, const Any& data, const String& msg)
        : mRequest(rq), mSuccess(success), mMessages(msg), mData(data)
    {
        
    }
    //---------------------------------------------------------------------
    WorkQueue::Response::~Response()
    {
        OGRE_DELETE mRequest;
    }
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    DefaultWorkQueueBase::DefaultWorkQueueBase(const String& name)
        : mName(name)
        , mWorkerThreadCount(1)
        , mWorkerRenderSystemAccess(false)
        , mIsRunning(false)
        , mResposeTimeLimitMS(10)
        , mPaused(false)
        , mAcceptRequests(true)
        , mShuttingDown(false)
    {
    }
    //---------------------------------------------------------------------
    const String& DefaultWorkQueueBase::getName() const
    {
        return mName;
    }
    //---------------------------------------------------------------------
    size_t DefaultWorkQueueBase::getWorkerThreadCount() const
    {
        return mWorkerThreadCount;
    }
    //---------------------------------------------------------------------
    void DefaultWorkQueueBase::setWorkerThreadCount(size_t c)
    {
        mWorkerThreadCount = c;
    }
    //---------------------------------------------------------------------
    bool DefaultWorkQueueBase::getWorkersCanAccessRenderSystem() const
    {
        return mWorkerRenderSystemAccess;
    }
    //---------------------------------------------------------------------
    void DefaultWorkQueueBase::setWorkersCanAccessRenderSystem(bool access)
    {
        mWorkerRenderSystemAccess = access;
    }
    //---------------------------------------------------------------------
    DefaultWorkQueueBase::~DefaultWorkQueueBase()
    {
        //shutdown(); // can't call here; abstract function
    }
    void DefaultWorkQueueBase::addMainThreadTask(std::function<void()> task)
    {
        if (!mAcceptRequests || mShuttingDown)
            return;
        OGRE_WQ_LOCK_MUTEX(mResponseMutex);
        mMainThreadTasks.push_back(task);
    }
    //---------------------------------------------------------------------
    void DefaultWorkQueueBase::addTask(std::function<void()> task)
    {
        OGRE_WQ_LOCK_MUTEX(mRequestMutex);
        if (!mAcceptRequests || mShuttingDown)
            return;

#if OGRE_THREAD_SUPPORT
        mTasks.push_back(task);
        notifyWorkers();
#else
        task(); // no threading, just run it
#endif
        LogManager::getSingleton().stream(LML_TRIVIAL)
            << "DefaultWorkQueueBase('" << mName << "') - QUEUED(thread:" << OGRE_THREAD_CURRENT_ID << ")";
    }
    //---------------------------------------------------------------------
    void DefaultWorkQueueBase::setPaused(bool pause)
    {
            OGRE_WQ_LOCK_MUTEX(mRequestMutex);

        mPaused = pause;
    }
    //---------------------------------------------------------------------
    bool DefaultWorkQueueBase::isPaused() const
    {
        return mPaused;
    }
    //---------------------------------------------------------------------
    void DefaultWorkQueueBase::setRequestsAccepted(bool accept)
    {
            OGRE_WQ_LOCK_MUTEX(mRequestMutex);

        mAcceptRequests = accept;
    }
    //---------------------------------------------------------------------
    bool DefaultWorkQueueBase::getRequestsAccepted() const
    {
        return mAcceptRequests;
    }
    //---------------------------------------------------------------------
    void DefaultWorkQueueBase::_processNextRequest()
    {
        if (mTasks.empty())
            return;

        std::function<void()> task;
        {
            // scoped to only lock while retrieving the next request
            OGRE_WQ_LOCK_MUTEX(mRequestMutex);
            LogManager::getSingleton().stream(LML_TRIVIAL)
                << "DefaultWorkQueueBase('" << mName << "') - PROCESS_TASK(thread:" << OGRE_THREAD_CURRENT_ID
                << ")";
            task = std::move(mTasks.front());
            mTasks.pop_front();
        }
        task();
    }
    //---------------------------------------------------------------------
    void DefaultWorkQueueBase::processMainThreadTasks()
    {
        unsigned long msStart = Root::getSingleton().getTimer()->getMilliseconds();
        unsigned long msCurrent = 0;

        // keep going until we run out of responses or out of time
        while(true)
        {
            if(!mMainThreadTasks.empty())
            {
                std::function<void()> task;
                {
                    OGRE_WQ_LOCK_MUTEX(mResponseMutex);
                    LogManager::getSingleton().stream(LML_TRIVIAL)
                        << "DefaultWorkQueueBase('" << mName << "') - PROCESS_MAIN_TASK";
                    task = std::move(mMainThreadTasks.front());
                    mMainThreadTasks.pop_front();
                }
                task();
            }

            // time limit
            if (mResposeTimeLimitMS)
            {
                msCurrent = Root::getSingleton().getTimer()->getMilliseconds();
                if (msCurrent - msStart > mResposeTimeLimitMS)
                    break;
            }
        }
    }
}
