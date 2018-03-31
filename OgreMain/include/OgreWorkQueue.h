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

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup General
    *  @{
    */

    /** Interface to a general purpose request / response style background work queue.
    @remarks
        A work queue is a simple structure, where requests for work are placed
        onto the queue, then removed by a worker for processing, then finally
        a response is placed on the result queue for the originator to pick up
        at their leisure. The typical use for this is in a threaded environment, 
        although any kind of deferred processing could use this approach to 
        decouple and distribute work over a period of time even 
        if it was single threaded.
    @par
        WorkQueues also incorporate thread pools. One or more background worker threads
        can wait on the queue and be notified when a request is waiting to be
        processed. For maximal thread usage, a WorkQueue instance should be shared
        among many sources of work, rather than many work queues being created.
        This way, you can share a small number of hardware threads among a large 
        number of background tasks. This doesn't mean you have to implement all the
        request processing in one class, you can plug in many handlers in order to
        process the requests.
    @par
        This is an abstract interface definition; users can subclass this and 
        provide their own implementation if required to centralise task management
        in their own subsystems. We also provide a default implementation in the
        form of DefaultWorkQueue.
    */
    class _OgreExport WorkQueue : public UtilityAlloc
    {
    protected:
        typedef std::map<String, uint16> ChannelMap;
        ChannelMap mChannelMap;
        uint16 mNextChannel;
        OGRE_WQ_MUTEX(mChannelMapMutex);
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
            /// Set the abort flag
            void abortRequest() const { mAborted = true; }
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
            /// Get the abort flag
            bool getAborted() const { return mAborted; }
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
            /// Abort the request
            void abortRequest() { mRequest->abortRequest(); mData.reset(); }
        };

        /** Interface definition for a handler of requests. 
        @remarks
        User classes are expected to implement this interface in order to
        process requests on the queue. It's important to realise that
        the calls to this class may be in a separate thread to the main
        render context, and as such it may not be possible to make
        rendersystem or other GPU-dependent calls in this handler. You can only
        do so if the queue was created with 'workersCanAccessRenderSystem'
        set to true, and OGRE_THREAD_SUPPORT=1, but this puts extra strain
        on the thread safety of the render system and is not recommended.
        It is best to perform CPU-side work in these handlers and let the
        response handler transfer results to the GPU in the main render thread.
        */
        class _OgreExport RequestHandler
        {
        public:
            RequestHandler() {}
            virtual ~RequestHandler() {}

            /** Return whether this handler can process a given request. 
            @remarks
            Defaults to true, but if you wish to add several handlers each of
            which deal with different types of request, you can override
            this method. 
            */
            virtual bool canHandleRequest(const Request* req, const WorkQueue* srcQ) 
            { (void)srcQ; return !req->getAborted(); }

            /** The handler method every subclass must implement. 
            If a failure is encountered, return a Response with a failure
            result rather than raise an exception.
            @param req The Request structure, which is effectively owned by the
            handler during this call. It must be attached to the returned
            Response regardless of success or failure.
            @param srcQ The work queue that this request originated from
            @return Pointer to a Response object - the caller is responsible
            for deleting the object.
            */
            virtual Response* handleRequest(const Request* req, const WorkQueue* srcQ) = 0;
        };

        /** Interface definition for a handler of responses. 
        @remarks
        User classes are expected to implement this interface in order to
        process responses from the queue. All calls to this class will be 
        in the main render thread and thus all GPU resources will be
        available. 
        */
        class _OgreExport ResponseHandler
        {
        public:
            ResponseHandler() {}
            virtual ~ResponseHandler() {}

            /** Return whether this handler can process a given response. 
            @remarks
            Defaults to true, but if you wish to add several handlers each of
            which deal with different types of response, you can override
            this method. 
            */
            virtual bool canHandleResponse(const Response* res, const WorkQueue* srcQ) 
            { (void)srcQ; return !res->getRequest()->getAborted(); }

            /** The handler method every subclass must implement. 
            @param res The Response structure. The caller is responsible for
            deleting this after the call is made, none of the data contained
            (except pointers to structures in user Any data) will persist
            after this call is returned.
            @param srcQ The work queue that this request originated from
            */
            virtual void handleResponse(const Response* res, const WorkQueue* srcQ) = 0;
        };

        WorkQueue() : mNextChannel(0) {}
        virtual ~WorkQueue() {}

        /** Start up the queue with the options that have been set.
        @param forceRestart If the queue is already running, whether to shut it
            down and restart.
        */
        virtual void startup(bool forceRestart = true) = 0;
        /** Add a request handler instance to the queue. 
        @remarks
            Every queue must have at least one request handler instance for each 
            channel in which requests are raised. If you 
            add more than one handler per channel, then you must implement canHandleRequest 
            differently in each if you wish them to respond to different requests.
        @param channel The channel for requests you want to handle
        @param rh Your handler
        */
        virtual void addRequestHandler(uint16 channel, RequestHandler* rh) = 0;
        /** Remove a request handler. */
        virtual void removeRequestHandler(uint16 channel, RequestHandler* rh) = 0;

        /** Add a response handler instance to the queue. 
        @remarks
            Every queue must have at least one response handler instance for each 
            channel in which requests are raised. If you add more than one, then you 
            must implement canHandleResponse differently in each if you wish them 
            to respond to different responses.
        @param channel The channel for responses you want to handle
        @param rh Your handler
        */
        virtual void addResponseHandler(uint16 channel, ResponseHandler* rh) = 0;
        /** Remove a Response handler. */
        virtual void removeResponseHandler(uint16 channel, ResponseHandler* rh) = 0;

        /** Add a new request to the queue.
        @param channel The channel this request will go into = 0; the channel is the top-level
            categorisation of the request
        @param requestType An identifier that's unique within this queue which
            identifies the type of the request (user decides the actual value)
        @param rData The data required by the request process. 
        @param retryCount The number of times the request should be retried
            if it fails.
        @param forceSynchronous Forces the request to be processed immediately
            even if threading is enabled.
        @param idleThread Request should be processed on the idle thread.
            Idle requests will be processed on a single worker thread. You should use this in the following situations:
            1. If a request handler can't process multiple requests in parallel.
            2. If you add lot of requests, but you want to keep the game fast.
            3. If you have lot of more important threads. (example: physics).
        @return The ID of the request that has been added
        */
        virtual RequestID addRequest(uint16 channel, uint16 requestType, const Any& rData, uint8 retryCount = 0, 
            bool forceSynchronous = false, bool idleThread = false) = 0;

        /** Abort a previously issued request.
        If the request is still waiting to be processed, it will be 
        removed from the queue.
        @param id The ID of the previously issued request.
        */
        virtual void abortRequest(RequestID id) = 0;

        /** Abort request if it is not being processed currently.
         *
         * @param id The ID of the previously issued request.
         *
         * @retval true If request was aborted successfully.
         * @retval false If request is already being processed so it can not be aborted.
         */
        virtual bool abortPendingRequest(RequestID id) = 0;

        /** Abort all previously issued requests in a given channel.
        Any requests still waiting to be processed of the given channel, will be 
        removed from the queue.
        Requests which are processed, but response handler is not called will also be removed.
        @param channel The type of request to be aborted
        */
        virtual void abortRequestsByChannel(uint16 channel) = 0;

        /** Abort all previously issued requests in a given channel.
        Any requests still waiting to be processed of the given channel, will be 
        removed from the queue.
        It will not remove requests, where the request handler is already called.
        @param channel The type of request to be aborted
        */
        virtual void abortPendingRequestsByChannel(uint16 channel) = 0;

        /** Abort all previously issued requests.
        Any requests still waiting to be processed will be removed from the queue.
        Any requests that are being processed will still complete.
        */
        virtual void abortAllRequests() = 0;
        
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

        /** Process the responses in the queue.
        @remarks
            This method is public, and must be called from the main render
            thread to 'pump' responses through the system. The method will usually
            try to clear all responses before returning = 0; however, you can specify
            a time limit on the response processing to limit the impact of
            spikes in demand by calling setResponseProcessingTimeLimit.
        */
        virtual void processResponses() = 0; 

        /** Get the time limit imposed on the processing of responses in a
            single frame, in milliseconds (0 indicates no limit).
        */
        virtual unsigned long getResponseProcessingTimeLimit() const = 0;

        /** Set the time limit imposed on the processing of responses in a
            single frame, in milliseconds (0 indicates no limit).
            This sets the maximum time that will be spent in processResponses() in 
            a single frame. The default is 8ms.
        */
        virtual void setResponseProcessingTimeLimit(unsigned long ms) = 0;

        /** Shut down the queue.
        */
        virtual void shutdown() = 0;

        /** Get a channel ID for a given channel name. 
        @remarks
            Channels are assigned on a first-come, first-served basis and are
            not persistent across application instances. This method allows 
            applications to not worry about channel clashes through manually
            assigned channel numbers.
        */
        virtual uint16 getChannel(const String& channelName);

    };

    /** Base for a general purpose request / response style background work queue.
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
        @remarks
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

        /// @copydoc WorkQueue::addRequestHandler
        virtual void addRequestHandler(uint16 channel, RequestHandler* rh);
        /// @copydoc WorkQueue::removeRequestHandler
        virtual void removeRequestHandler(uint16 channel, RequestHandler* rh);
        /// @copydoc WorkQueue::addResponseHandler
        virtual void addResponseHandler(uint16 channel, ResponseHandler* rh);
        /// @copydoc WorkQueue::removeResponseHandler
        virtual void removeResponseHandler(uint16 channel, ResponseHandler* rh);

        /// @copydoc WorkQueue::addRequest
        virtual RequestID addRequest(uint16 channel, uint16 requestType, const Any& rData, uint8 retryCount = 0, 
            bool forceSynchronous = false, bool idleThread = false);
        /// @copydoc WorkQueue::abortRequest
        virtual void abortRequest(RequestID id);
        /// @copydoc WorkQueue::abortPendingRequest
        virtual bool abortPendingRequest(RequestID id);
        /// @copydoc WorkQueue::abortRequestsByChannel
        virtual void abortRequestsByChannel(uint16 channel);
        /// @copydoc WorkQueue::abortPendingRequestsByChannel
        virtual void abortPendingRequestsByChannel(uint16 channel);
        /// @copydoc WorkQueue::abortAllRequests
        virtual void abortAllRequests();
        /// @copydoc WorkQueue::setPaused
        virtual void setPaused(bool pause);
        /// @copydoc WorkQueue::isPaused
        virtual bool isPaused() const;
        /// @copydoc WorkQueue::setRequestsAccepted
        virtual void setRequestsAccepted(bool accept);
        /// @copydoc WorkQueue::getRequestsAccepted
        virtual bool getRequestsAccepted() const;
        /// @copydoc WorkQueue::processResponses
        virtual void processResponses(); 
        /// @copydoc WorkQueue::getResponseProcessingTimeLimit
        virtual unsigned long getResponseProcessingTimeLimit() const { return mResposeTimeLimitMS; }
        /// @copydoc WorkQueue::setResponseProcessingTimeLimit
        virtual void setResponseProcessingTimeLimit(unsigned long ms) { mResposeTimeLimitMS = ms; }
    protected:
        String mName;
        size_t mWorkerThreadCount;
        bool mWorkerRenderSystemAccess;
        bool mIsRunning;
        unsigned long mResposeTimeLimitMS;

        typedef std::deque<Request*> RequestQueue;
        typedef std::deque<Response*> ResponseQueue;
        RequestQueue mRequestQueue; // Guarded by mRequestMutex
        RequestQueue mProcessQueue; // Guarded by mProcessMutex
        ResponseQueue mResponseQueue; // Guarded by mResponseMutex

        /// Thread function
        struct _OgreExport WorkerFunc OGRE_THREAD_WORKER_INHERIT
        {
            DefaultWorkQueueBase* mQueue;

            WorkerFunc(DefaultWorkQueueBase* q) 
                : mQueue(q) {}

            void operator()();
            
            void operator()() const;

            void run();
        };
        WorkerFunc* mWorkerFunc;

        /** Intermediate structure to hold a pointer to a request handler which 
            provides insurance against the handler itself being disconnected
            while the list remains unchanged.
        */
        class _OgreExport RequestHandlerHolder : public UtilityAlloc
        {
        protected:
            OGRE_WQ_RW_MUTEX(mRWMutex);
            RequestHandler* mHandler;
        public:
            RequestHandlerHolder(RequestHandler* handler)
                : mHandler(handler) {}

            // Disconnect the handler to allow it to be destroyed
            void disconnectHandler()
            {
                // write lock - must wait for all requests to finish
                OGRE_WQ_LOCK_RW_MUTEX_WRITE(mRWMutex);
                mHandler = 0;
            }

            /** Get handler pointer - note, only use this for == comparison or similar,
                do not attempt to call it as it is not thread safe. 
            */
            RequestHandler* getHandler() { return mHandler; }

            /** Process a request if possible.
            @return Valid response if processed, null otherwise
            */
            Response* handleRequest(const Request* req, const WorkQueue* srcQ)
            {
                // Read mutex so that multiple requests can be processed by the
                // same handler in parallel if required
                OGRE_WQ_LOCK_RW_MUTEX_READ(mRWMutex);
                Response* response = 0;
                if (mHandler)
                {
                    if (mHandler->canHandleRequest(req, srcQ))
                    {
                        response = mHandler->handleRequest(req, srcQ);
                    }
                }
                return response;
            }

        };
        // Hold these by shared pointer so they can be copied keeping same instance
        typedef SharedPtr<RequestHandlerHolder> RequestHandlerHolderPtr;

        typedef std::list<RequestHandlerHolderPtr> RequestHandlerList;
        typedef std::list<ResponseHandler*> ResponseHandlerList;
        typedef std::map<uint16, RequestHandlerList> RequestHandlerListByChannel;
        typedef std::map<uint16, ResponseHandlerList> ResponseHandlerListByChannel;

        RequestHandlerListByChannel mRequestHandlers;
        ResponseHandlerListByChannel mResponseHandlers;
        RequestID mRequestCount; // Guarded by mRequestMutex
        bool mPaused;
        bool mAcceptRequests;
        bool mShuttingDown;

        //NOTE: If you lock multiple mutexes at the same time, the order is important!
        // For example if threadA locks mIdleMutex first then tries to lock mProcessMutex,
        // and threadB locks mProcessMutex first, then mIdleMutex. In this case you can get livelock and the system is dead!
        //RULE: Lock mProcessMutex before other mutex, to prevent livelocks
        OGRE_WQ_MUTEX(mIdleMutex);
        OGRE_WQ_MUTEX(mRequestMutex);
        OGRE_WQ_MUTEX(mProcessMutex);
        OGRE_WQ_MUTEX(mResponseMutex);
        OGRE_WQ_RW_MUTEX(mRequestHandlerMutex);


        void processRequestResponse(Request* r, bool synchronous);
        Response* processRequest(Request* r);
        void processResponse(Response* r);
        /// Notify workers about a new request. 
        virtual void notifyWorkers() = 0;
        /// Put a Request on the queue with a specific RequestID.
        void addRequestWithRID(RequestID rid, uint16 channel, uint16 requestType, const Any& rData, uint8 retryCount);
        
        RequestQueue mIdleRequestQueue; // Guarded by mIdleMutex
        bool mIdleThreadRunning; // Guarded by mIdleMutex
        Request* mIdleProcessed; // Guarded by mProcessMutex
        

        bool processIdleRequests();
    };





    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif

