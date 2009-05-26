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
#ifndef __OgreWorkQueue_H__
#define __OgreWorkQueue_H__

#include "OgrePrerequisites.h"
#include "OgreAtomicWrappers.h"

namespace Ogre
{
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup General
	*  @{
	*/

	/** Implementation of a general purpose request / response style background work queue.
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
		You can run your own threads to process the queue if you wish, but be aware
		that you should always terminate your own thread yourself too.
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
			/// The request channel, as an integer (user can define enumerations on this, but Ogre reserves channels 0-63)
			uint16 mChannel;
			/// The request type, as an integer within the channel (user can define enumerations on this)
			uint16 mType;
			/// The details of the request (user defined)
			Any mData;
			/// Retry count - set this to non-zero to have the request try again on failure
			uint8 mRetryCount;
			/// Identifier (assigned by the system)
			RequestID mID;

			/// Constructor (WorkQueue only)
			Request(uint16 channel, uint16 rtype, const Any& rData, uint8 retry, RequestID rid);
		public:
			~Request();
			/// Get the request channel (top level categorisation)
			uint32 getChannel() const { return mChannel; }
			/// Get the type of this request within the given channel
			uint32 getType() const { return mType; }
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
			Request* mRequest;
			/// Whether the work item succeeded or not
			bool mSuccess;
			/// Any diagnostic messages
			String mMessages;
			/// Data associated with the result of the process
			Any mData;

			Response(Request* rq, bool success, const Any& data, const String& msg = StringUtil::BLANK);
		public:
			~Response();
			/// Get the request that this is a response to (NB destruction destroys this)
			Request* getRequest() const { return mRequest; }
			/// Return whether this is a successful response
			bool succeeded() const { return mSuccess; }
			/// Get any diagnostic messages about the process
			const String& getMessages() const { return mMessages; }
			/// Return the response data (user defined, only valid on success)
			const Any& getData() const { return mData; }
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
			{ return true; }

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
			{ return true; }

			/** The handler method every subclass must implement. 
			@param res The Response structure. The caller is responsible for
				deleting this after the call is made, none of the data contained
				(except pointers to structures in user Any data) will persist
				after this call is returned.
			@param srcQ The work queue that this request originated from
			*/
			virtual void handleResponse(const Response* res, const WorkQueue* srcQ) = 0;
		};

		/** Constructor.
			Call startup() to initialise.
		@param name Optional name, just helps to identify logging output
		*/
		WorkQueue(const String& name = StringUtil::BLANK);
		virtual ~WorkQueue();
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

		/** Start up the queue with the options that have been set.
		@param forceRestart If the queue is already running, whether to shut it
			down and restart.
		*/
		virtual void startup(bool forceRestart = true);
		/** Add a request handler instance to the queue. 
		@remarks
			Every queue must have at least one request handler instance for each 
			channel in which requests are raised. If you 
			add more than one handler per channel, then you must implement canHandleRequest 
			differently	in each if you wish them to respond to different requests.
		@param channel The channel for requests you want to handle
		@param rh Your handler
		*/
		virtual void addRequestHandler(uint16 channel, RequestHandler* rh);
		/** Remove a request handler. */
		virtual void removeRequestHandler(uint16 channel, RequestHandler* rh);

		/** Add a response handler instance to the queue. 
		@remarks
			Every queue must have at least one response handler instance for each 
			channel in which requests are raised. If you add more than one, then you 
			must implement canHandleResponse differently in each if you wish them 
			to respond to different responses.
		@param channel The channel for responses you want to handle
		@param rh Your handler
		*/
		virtual void addResponseHandler(uint16 channel, ResponseHandler* rh);
		/** Remove a Response handler. */
		virtual void removeResponseHandler(uint16 channel, ResponseHandler* rh);

		/** Add a new request to the queue.
		@param channel The channel this request will go into; the channel is the top-level
			categorisation of the request, and OGRE reserves channels 0-63, so 
			users must use channels 64 or higher for their custom requests.
		@param requestType An identifier that's unique within this queue which
			identifies the type of the request (user decides the actual value)
		@param rData The data required by the request process. 
		@param retryCount The number of times the request should be retried
			if it fails.
		@param forceSynchronous Forces the request to be processed immediately
			even if threading is enabled.
		@returns The ID of the request that has been added
		*/
		virtual RequestID addRequest(uint16 channel, uint16 requestType, const Any& rData, uint8 retryCount = 0, 
			bool forceSynchronous = false);

		/** Abort a previously issued request.
		If the request is still waiting to be processed, it will be 
		removed from the queue.
		@param id The ID of the previously issued request.
		*/
		virtual void abortRequest(RequestID id);

		/** Abort all previously issued requests in a given channel.
		Any requests still waiting to be processed of the given channel, will be 
		removed from the queue.
		@param channel The type of request to be aborted
		*/
		virtual void abortRequestsByChannel(uint16 channel);

		/** Abort all previously issued requests.
		Any requests still waiting to be processed will be removed from the queue.
		Any requests that are being processed will still complete.
		*/
		virtual void abortAllRequests();
		
		/** Set whether to pause further processing of any requests. 
		If true, any further requests will simply be queued and not processed until
		setPaused(false) is called. Any requests which are in the process of being
		worked on already will still continue. 
		*/
		virtual void setPaused(bool pause);
		/// Return whether the queue is paused ie not sending more work to workers
		virtual bool isPaused() const;

		/** Set whether to accept new requests or not. 
		If true, requests are added to the queue as usual. If false, requests
		are silently ignored until setRequestsAccepted(true) is called. 
		*/
		virtual void setRequestsAccepted(bool accept);
		/// Returns whether requests are being accepted right now
		virtual bool getRequestsAccepted() const;


		/** Get the number of requests that are currently being processed,
			that is they've been moved from the queue to a worker, but have
			not completed yet. 
			@remarks For obvious reasons this value can never exceed the
			number of configured worker threads.
		*/
		virtual size_t getRequestsInProgressCount() const; 
		/** Get the number of requests which are currently queued for processing. 
		These requests have been added to the queue but are waiting for a worker
		to begin processing them.
		*/
		virtual size_t geRequestsQueuedCount() const;

		/** Get the number of responses currently queued for processing. 
		*/
		virtual size_t getResponsesQueuedCount() const;

		/** Process the next request on the queue. 
		@remarks
			This method is public, but only intended for advanced users to call. 
			The only reason you would call this, is if you were using your 
			own thread to drive the worker processing. The thread calling this
			method will be the thread used to call the RequestHandler.
		*/
		virtual void _processNextRequest();

		/** Process the next response on the queue.
		@remarks
			This method is public, but intended for only the owner of this
			WorkQueue to call. It is imperitive that the thread
			that calls this method is the main render context thread, since
			ResponseHandler implementations are allowed to access GPU resources.
		*/
		virtual void _processNextResponse(); 

		/** To be called by a separate thread; will return immediately if there
			are items in the queue, or suspend the thread until new items are added
			otherwise.
		*/
		virtual void _waitForNextRequest();

		/// Notify that a thread has registered itself with the render system
		virtual void _notifyThreadRegistered();

		/** Returns whether the queue is trying to shut down. */
		virtual bool isShuttingDown() const { return mShuttingDown; }

		/** Shut down the queue.
		*/
		virtual void shutdown();

	protected:
		String mName;
		size_t mWorkerThreadCount;
		bool mWorkerRenderSystemAccess;
		bool mIsRunning;

		typedef deque<Request*>::type RequestQueue;
		typedef deque<Response*>::type ResponseQueue;
		RequestQueue mRequestQueue;
		ResponseQueue mResponseQueue;

		/// Thread function
		struct WorkerFunc
		{
			WorkQueue* mQueue;
			bool mRenderSysAccess;

			WorkerFunc(WorkQueue* q, bool renderSysAccess) 
				: mQueue(q), mRenderSysAccess(renderSysAccess) {}

			void operator()();
		};

		typedef list<RequestHandler*>::type RequestHandlerList;
		typedef list<ResponseHandler*>::type ResponseHandlerList;
		typedef map<uint16, RequestHandlerList>::type RequestHandlerListByChannel;
		typedef map<uint16, ResponseHandlerList>::type ResponseHandlerListByChannel;

		RequestHandlerListByChannel mRequestHandlers;
		ResponseHandlerListByChannel mResponseHandlers;
		RequestID mRequestCount;
		bool mPaused;
		bool mAcceptRequests;
		size_t mRequestsQueuedCount;
		size_t mRequestsInProgressCount;
		size_t mResponsesQueuedCount;
		bool mShuttingDown;
		size_t mNumThreadsRegisteredWithRS;
		/// Init notification mutex (must lock before waiting on initCondition)
		OGRE_MUTEX(mInitMutex)
		/// Synchroniser token to wait / notify on thread init 
		OGRE_THREAD_SYNCHRONISER(mInitSync)

		OGRE_MUTEX(mRequestMutex)
		OGRE_MUTEX(mResponseMutex)
		OGRE_MUTEX(mRequestHandlerMutex)
		OGRE_MUTEX(mResponseHandlerMutex)
		OGRE_THREAD_SYNCHRONISER(mRequestCondition)
#if OGRE_THREAD_SUPPORT
		typedef vector<boost::thread*>::type WorkerThreadList;
		WorkerThreadList mWorkers;
#endif


		void processRequestResponse(Request* r, bool synchronous);
		Response* processRequest(Request* r);
		void processResponse(Response* r);




	};





	/** @} */
	/** @} */

}
#endif

