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

    /** Interface to a general purpose task-based background work queue.

        A work queue is a simple structure, where tasks of work are placed
        onto the queue, then removed by a worker for processing.
        The typical use for this is in a threaded environment,
        although any kind of deferred processing could use this approach to
        decouple and distribute work over a period of time even
        if it was single threaded.

        WorkQueues also incorporate thread pools. One or more background worker threads
        can wait on the queue and be notified when a task is waiting to be
        processed. For maximal thread usage, a WorkQueue instance should be shared
        among many sources of work, rather than many work queues being created.
        This way, you can share a small number of hardware threads among a large
        number of background tasks. This doesn't mean you have to implement all the
        task processing in one class, you can submit as many independent tasks as
        you like.

        The WorkQueue distinguishes between two types of tasks:
        1. Background tasks: Added via @ref addTask. These are processed by the worker threads.
        2. Main thread tasks: Added via @ref addMainThreadTask. These are processed by the main thread
           when @ref processMainThreadTasks is called. This is useful for tasks that need to access
           resources that are not thread-safe or must be accessed from the main thread (e.g. GPU resources).

        @ref processMainThreadTasks is automatically called by @ref Root::renderOneFrame (via @ref Root::_fireFrameEnded)
        or can be called manually by the user in the main loop.

        Both queues are FIFO and thread-safe; `addTask` and `addMainThreadTask` may be
        called from any thread. There is no priority and no cancellation. With more than
        one worker thread, background tasks may complete out of order.

        Tasks are `std::function<void()>`, therefore their captures must be copyable.

        @par Task lifetime
        A task may run after the object that submitted it has been destroyed. Capture a
        `std::shared_ptr` (or check a `std::weak_ptr`) if no other mechanism guarantees
        that everything the task accesses is still alive. Keep resource references needed
        by the task alive in its capture.

        @par Exceptions
        An exception thrown by a background task is not caught and will terminate the
        process. Use a `std::packaged_task` to capture the exception in a `std::future`
        instead. An exception thrown by a main-thread task propagates out of
        @ref processMainThreadTasks.

        @see @ref background-resource for a tutorial and usage examples.

        This is an abstract interface definition; users can subclass this and
        provide their own implementation if required to centralise task management
        in their own subsystems. We also provide a default implementation in the
        form of DefaultWorkQueue.
    */
    class _OgreExport WorkQueue : public UtilityAlloc
    {
    public:
        /// Legacy identifier type of the request/response API
        typedef unsigned long long int RequestID;

        /** Legacy request structure.

            The task-based API passes the work directly as a `std::function`, so
            Request/Response are not needed and are not used by %Ogre internally.
            They are kept for source compatibility with code written against the
            old request/response API.
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

        /** Legacy response structure.

            @warning A Response takes ownership of its Request: destroying the
            response deletes the request as well. Do not delete both.
            Like Request, this type is not used by %Ogre internally and is kept
            for source compatibility with the old request/response API.
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

        /** Get the number of worker threads that this queue will start when
            startup() is called.

            Implementations are free to ignore this setting; the default is 1.
        */
        virtual size_t getWorkerThreadCount() const { return 1; }

        /** Set the number of worker threads that this queue will start
            when startup() is called (default 1).
            Calling this will have no effect unless the queue is shut down and
            restarted.
        */
        virtual void setWorkerThreadCount(size_t c) {}

        /** Start up the queue with the options that have been set.
        @param forceRestart If the queue is already running, whether to shut it
            down and restart.
        */
        virtual void startup(bool forceRestart = true) = 0;

        /** Add a new task to the background queue.

            May be called from any thread. Tasks are executed in submission order,
            but with more than one worker thread the completion order is unspecified.

            The task is silently dropped if the queue is shutting down or if
            @ref setRequestsAccepted has been called with `false`.

            Tasks are `std::function<void()>`, so the captures must be copyable.
            With `OGRE_THREAD_SUPPORT == 0` the task is executed inline on the
            calling thread instead of being queued.

            @warning An exception escaping the task terminates the process.
            Wrap potentially throwing work in a `std::packaged_task`.
        */
        virtual void addTask(std::function<void()> task) = 0;

        /** Set whether to pause further processing of tasks.

            If true, further tasks will simply be queued and not processed until
            setPaused(false) is called. Tasks which are already being worked on
            will still continue.

            @note Pausing is not currently implemented by DefaultWorkQueueBase;
            this flag is retained for custom implementations. Use
            @ref setRequestsAccepted to stop accepting new tasks.
        */
        virtual void setPaused(bool pause) = 0;
        /// Return whether the queue is paused ie not sending more work to workers
        virtual bool isPaused() const = 0;

        /** Set whether to accept new tasks or not.

            If true, tasks are added to the queues as usual. If false, both
            @ref addTask and @ref addMainThreadTask silently ignore new tasks
            until setRequestsAccepted(true) is called.
        */
        virtual void setRequestsAccepted(bool accept) = 0;
        /// Returns whether tasks are being accepted right now
        virtual bool getRequestsAccepted() const = 0;

        /** Process the tasks in the main-thread queue.

            This method must be called from the main render
            thread to 'pump' tasks through the system. The method will usually
            try to clear all tasks before returning; however, you can specify
            a time limit on the tasks processing to limit the impact of
            spikes in demand by calling @ref setMainThreadProcessingTimeLimit.
            Tasks added while the queue is being processed may be picked up in
            the same call, subject to the time limit.

            @note This is automatically called by @ref Root::renderOneFrame, after
            the `frameEnded` listeners.
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

            The limit is checked after each task completes, so a single long task
            can overshoot it. Each call to @ref processMainThreadTasks gets its own
            budget.
        */
        void setMainThreadProcessingTimeLimit(uint64 ms) { setResponseProcessingTimeLimit(static_cast<unsigned long>(ms)); }

        /// @deprecated use @ref setMainThreadProcessingTimeLimit
        virtual void setResponseProcessingTimeLimit(unsigned long ms) = 0;

        /** Add a task that will be processed on the main render thread.

            May be called from any thread. The task does not run immediately, even
            when called from the main thread; it runs during the next
            @ref processMainThreadTasks call.

            The task is silently dropped if the queue is shutting down or if
            @ref setRequestsAccepted has been called with `false`.

            @warning An exception escaping the task propagates out of
            @ref processMainThreadTasks.
        */
        virtual void addMainThreadTask(std::function<void()> task) = 0;

        /** Shut down the queue.

            Stops the worker threads and waits for them to finish. Tasks that are
            already running complete, but tasks still waiting in the queues are not
            executed. The queues are not cleared, so starting the queue again may
            still process them.
        */
        virtual void shutdown() = 0;
    };

    /** Base implementation of a general purpose task-based background work queue.

        It provides the two FIFO queues, the accept/pause flags and the main-thread
        processing loop. Concrete implementations only have to provide the thread
        management via `_threadMain()` and `notifyWorkers()`, plus
        @ref startup / @ref shutdown.

        @see @ref background-resource
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

        /** Pop and execute the next task from the background queue.

            This method is public, but only intended for advanced users. The only
            reason you would call it is if you drive the worker processing with your
            own thread. The task runs on the calling thread; an exception escaping it
            is not caught here.
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
        size_t getWorkerThreadCount() const override { return mWorkerThreadCount; }
        void setWorkerThreadCount(size_t c) override { mWorkerThreadCount = c; }
        /// @copydoc WorkQueue::addMainThreadTask
        void addMainThreadTask(std::function<void()> task) override;
        /// @copydoc WorkQueue::addTask
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

        /// Notify workers about a new task.
        virtual void notifyWorkers() = 0;
    };





    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif

