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
#ifndef __OgreWorkQueue_H__
#define __OgreWorkQueue_H__

#include "OgrePrerequisites.h"
#include "OgreAny.h"
#include "OgreSharedPtr.h"
#include "OgreCommon.h"
#include "Threading/OgreThreadHeaders.h"
#include "OgreHeaderPrefix.h"

#include <deque>
#include <functional>

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup General
    *  @{
    */

    /** Interface to a general purpose task-basedbackground work queue.

        A work queue is a simple structure, where tasks of work are placed
        onto the queue, then removed by a worker for processing.
        The typical use for this is in a threaded environment,
        although any kind of deferred processing could use this approach to 
        decouple and distribute work over a period of time even 
        if it was single threaded.

        WorkQueues also incorporate thread pools. One or more background worker threads
        can wait on the queue and be notified when a request is waiting to be
        processed. For maximal thread usage, a WorkQueue instance should be shared
        among many sources of work, rather than many work queues being created.
        This way, you can share a small number of hardware threads among a large 
        number of background tasks. This doesn't mean you have to implement all the
        request processing in one class, you can plug in many handlers in order to
        process the tasks.

        This is an abstract interface definition; users can subclass this and 
        provide their own implementation if required to centralise task management
        in their own subsystems. We also provide a default implementation in the
        form of DefaultWorkQueue.
    */
    class _OgreExport WorkQueue : public UtilityAlloc
    {
    public:
        /// Numeric identifier for a request
        typedef unsigned long long int RequestID;

        /** General purpose request structure. 
        */
        class _OgreExport Request : public UtilityAlloc
        {
            friend class WorkQueue;
        protected:
            /// The request channel, as an integer 
            uint16 mChannel;
            /// The request type, as an integer within the channel (user can define enumerations on this)
            uint16 mType;
            /// The details of the request (user defined)
            Any mData;
            /// Retry count - set this to non-zero to have the request try again on failure
            uint8 mRetryCount;
            /// Identifier (assigned by the system)
            RequestID mID;
            /// Abort Flag
            mutable bool mAborted;

        public:
            /// Constructor 
            Request(uint16 channel, uint16 rtype, const Any& rData, uint8 retry, RequestID rid);
            ~Request();
            /// Get the request channel (top level categorisation)
            uint16 getChannel() const { return mChannel; }
            /// Get the type of this request within the given channel
            uint16 getType() const { return mType; }
            /// Get the user details of this request
            const Any& getData() const { return mData; }
            /// Get the remaining retry count
            uint8 getRetryCount() const { return mRetryCount; }
            /// Get the identifier of this request
            RequestID getID() const { return mID; }
        };

        /** General purpose response structure. 
        */
        struct _OgreExport Response : public UtilityAlloc
        {
            /// Pointer to the request that this response is in relation to
            const Request* mRequest;
            /// Whether the work item succeeded or not
            bool mSuccess;
            /// Any diagnostic messages
            String mMessages;
            /// Data associated with the result of the process
            Any mData;

        public:
            Response(const Request* rq, bool success, const Any& data, const String& msg = BLANKSTRING);
            ~Response();
            /// Get the request that this is a response to (NB destruction destroys this)
            const Request* getRequest() const { return mRequest; }
            /// Return whether this is a successful response
            bool succeeded() const { return mSuccess; }
            /// Get any diagnostic messages about the process
            const String& getMessages() const { return mMessages; }
            /// Return the response data (user defined, only valid on success)
            const Any& getData() const { return mData; }
        };
        WorkQueue() {}
        virtual ~WorkQueue() {}

        /** Start up the queue with the options that have been set.
        @param forceRestart If the queue is already running, whether to shut it
            down and restart.
        */
        virtual void startup(bool forceRestart = true) = 0;

        /** Add a new task to the queue */
        virtual void addTask(std::function<void()> task) = 0;
        
        /** Set whether to pause further processing of any requests. 
        If true, any further requests will simply be queued and not processed until
        setPaused(false) is called. Any requests which are in the process of being
        worked on already will still continue. 
        */
        virtual void setPaused(bool pause) = 0;
        /// Return whether the queue is paused ie not sending more work to workers
        virtual bool isPaused() const = 0;

        /** Set whether to accept new requests or not. 
        If true, requests are added to the queue as usual. If false, requests
        are silently ignored until setRequestsAccepted(true) is called. 
        */
        virtual void setRequestsAccepted(bool accept) = 0;
        /// Returns whether requests are being accepted right now
        virtual bool getRequestsAccepted() const = 0;

        /** Process the tasks in the main-thread queue.

            This method must be called from the main render
            thread to 'pump' tasks through the system. The method will usually
            try to clear all tasks before returning; however, you can specify
            a time limit on the tasks processing to limit the impact of
            spikes in demand by calling @ref setMainThreadProcessingTimeLimit.
        */
        virtual void processMainThreadTasks();

        /// @deprecated use @ref processMainThreadTasks
        OGRE_DEPRECATED virtual void processResponses() { }

        /** Get the time limit imposed on the processing of tasks in a
            single frame, in milliseconds (0 indicates no limit).
        */
        uint64 getMainThreadProcessingTimeLimit() const { return getResponseProcessingTimeLimit(); }

        /// @deprecated use @ref getMainThreadProcessingTimeLimit()
        virtual unsigned long getResponseProcessingTimeLimit() const = 0;

        /** Set the time limit imposed on the processing of tasks in a
            single frame, in milliseconds (0 indicates no limit).
            This sets the maximum time that will be spent in @ref processMainThreadTasks() in
            a single frame. The default is 10ms.
        */
        void setMainThreadProcessingTimeLimit(uint64 ms) { setResponseProcessingTimeLimit(ms); }

        /// @deprecated use @ref setMainThreadProcessingTimeLimit
        virtual void setResponseProcessingTimeLimit(unsigned long ms) = 0;

        /** Add a deferred task that will be processed on the main render thread */
        virtual void addMainThreadTask(std::function<void()> task) = 0;

        /** Shut down the queue.
        */
        virtual void shutdown() = 0;
    };

    /** Base for a general purpose task-based background work queue.
    */
    class _OgreExport DefaultWorkQueueBase : public WorkQueue
    {
    public:

        /** Constructor.
            Call startup() to initialise.
        @param name Optional name, just helps to identify logging output
        */
        DefaultWorkQueueBase(const String& name = BLANKSTRING);
        virtual ~DefaultWorkQueueBase();
        /// Get the name of the work queue
        const String& getName() const;
        /** Get the number of worker threads that this queue will start when 
            startup() is called. 
        */
        virtual size_t getWorkerThreadCount() const;

        /** Set the number of worker threads that this queue will start
            when startup() is called (default 1).
            Calling this will have no effect unless the queue is shut down and
            restarted.
        */
        virtual void setWorkerThreadCount(size_t c);

        /** Get whether worker threads will be allowed to access render system
            resources. 
            Accessing render system resources from a separate thread can require that
            a context is maintained for that thread. Also, it requires that the
            render system is running in threadsafe mode, which only happens
            when OGRE_THREAD_SUPPORT=1. This option defaults to false, which means
            that threads can not use GPU resources, and the render system can 
            work in non-threadsafe mode, which is more efficient.
        */
        virtual bool getWorkersCanAccessRenderSystem() const;


        /** Set whether worker threads will be allowed to access render system
            resources. 
            Accessing render system resources from a separate thread can require that
            a context is maintained for that thread. Also, it requires that the
            render system is running in threadsafe mode, which only happens
            when OGRE_THREAD_SUPPORT=1. This option defaults to false, which means
            that threads can not use GPU resources, and the render system can 
            work in non-threadsafe mode, which is more efficient.
            Calling this will have no effect unless the queue is shut down and
            restarted.
        */
        virtual void setWorkersCanAccessRenderSystem(bool access);

        /** Process the next request on the queue. 

            This method is public, but only intended for advanced users to call. 
            The only reason you would call this, is if you were using your 
            own thread to drive the worker processing. The thread calling this
            method will be the thread used to call the RequestHandler.
        */
        virtual void _processNextRequest();

        /// Main function for each thread spawned.
        virtual void _threadMain() = 0;

        /** Returns whether the queue is trying to shut down. */
        virtual bool isShuttingDown() const { return mShuttingDown; }

        /// @copydoc WorkQueue::setPaused
        void setPaused(bool pause) override;
        /// @copydoc WorkQueue::isPaused
        bool isPaused() const override;
        /// @copydoc WorkQueue::setRequestsAccepted
        void setRequestsAccepted(bool accept) override;
        /// @copydoc WorkQueue::getRequestsAccepted
        virtual bool getRequestsAccepted() const override;
        void processMainThreadTasks() override;
        /// @copydoc WorkQueue::getResponseProcessingTimeLimit
        unsigned long getResponseProcessingTimeLimit() const override { return mResposeTimeLimitMS; }
        /// @copydoc WorkQueue::setResponseProcessingTimeLimit
        void setResponseProcessingTimeLimit(unsigned long ms) override { mResposeTimeLimitMS = ms; }

        void addMainThreadTask(std::function<void()> task) override;
        void addTask(std::function<void()> task) override;
    protected:
        String mName;
        size_t mWorkerThreadCount;
        bool mWorkerRenderSystemAccess;
        bool mIsRunning;
        unsigned long mResposeTimeLimitMS;

        std::deque<std::function<void()>> mTasks;
        std::deque<std::function<void()>> mMainThreadTasks;

        bool mPaused;
        bool mAcceptRequests;
        bool mShuttingDown;

        OGRE_WQ_MUTEX(mRequestMutex);
        OGRE_WQ_MUTEX(mResponseMutex);

        /// Notify workers about a new request. 
        virtual void notifyWorkers() = 0;
    };





    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif

