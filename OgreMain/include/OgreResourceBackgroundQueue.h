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
#ifndef __ResourceBackgroundQueue_H__
#define __ResourceBackgroundQueue_H__


#include "OgrePrerequisites.h"
#include "OgreCommon.h"
#include "OgreSingleton.h"
#include "OgreResource.h"

namespace Ogre {

	/// Identifier of a background process
	typedef unsigned long BackgroundProcessTicket;
	
	/** This class is used to perform Resource operations in a
		background thread. 
	@remarks
		If threading is enabled, Ogre will create a single background thread
		which can be used to load / unload resources in parallel. Only one
		resource will be processed at once in this background thread, but it
		will be in parallel with the main thread. 
	@par
		The general approach here is that on requesting a background resource
		process, your request is placed on a queue ready for the background
		thread to be picked up, and you will get a 'ticket' back, identifying
		the request. Your call will then return and your thread can
		proceed, knowing that at some point in the background the operation will 
		be performed. In it's own thread, the resource operation will be 
		performed, and once finished the ticket will be marked as complete. 
		You can check the status of tickets by calling isProcessComplete() 
		from your queueing thread. It is also possible to get immediate 
		callbacks on completion, but these callbacks happen in the background 
		loading thread (not your calling thread), so should only be used if you
		really understand multithreading. 
	@par
		By default, when threading is enabled this class will start its own 
		separate thread to perform the actual loading. However, if you would 
		prefer to use your own existing thread to perform the background load,
		then be sure to call setStartBackgroundThread(false) before initialise() is
		called by Root::initialise. Your own thread should call _initThread
		immediately on startup, before any resources are loaded at all, and
		_doNextQueuedBackgroundProcess to process background requests.
	@note
		This class will only perform tasks in a background thread if 
		OGRE_THREAD_SUPPORT is defined to be 1. Otherwise, all methods will
		call their exact equivalents in ResourceGroupManager synchronously. 
	*/
	class _OgreExport ResourceBackgroundQueue : public Singleton<ResourceBackgroundQueue>, public ResourceAlloc
	{
	public:
		/** This abstract listener interface lets you get notifications of
		completed background processes instead of having to poll ticket 
		statuses.
		@note
		For simplicity, these callbacks are not issued direct from the background
		loading thread, they are queued themselves to be sent from the main thread
		so that you don't have to be concerned about thread safety. 
		*/
		class _OgreExport Listener
		{
		public:
			/** Called when a requested operation completes, queued into main thread. 
			@note
				For simplicity, this callback is not issued direct from the background
				loading thread, it is queued to be sent from the main thread
				so that you don't have to be concerned about thread safety. 
			*/
			virtual void operationCompleted(BackgroundProcessTicket ticket) = 0;
			/** Called when a requested operation completes, immediate in background thread. 
			@note
				This is the advanced version of the background operation notification,
				it happens immediately when the background operation is completed, and
				your callback is executed in the <b>background thread</b>. Therefore if 
				you use this version, you have to be aware of thread safety issues
				and what you can and cannot do in your callback implementation.
			*/
			virtual void operationCompletedInThread(BackgroundProcessTicket ticket) {}
			/// Need virtual destructor in case subclasses use it
			virtual ~Listener() {}

		};
		/// Init notification mutex (must lock before waiting on initCondition)
		OGRE_MUTEX(initMutex)
		/// Synchroniser token to wait / notify on thread init (public incase external thread)
		OGRE_THREAD_SYNCHRONISER(initSync)

	protected:
		/** Enumerates the type of requests */
		enum RequestType
		{
			RT_INITIALISE_GROUP,
			RT_INITIALISE_ALL_GROUPS,
			RT_PREPARE_GROUP,
			RT_PREPARE_RESOURCE,
			RT_LOAD_GROUP,
			RT_LOAD_RESOURCE,
			RT_UNLOAD_GROUP,
			RT_UNLOAD_RESOURCE,
			RT_SHUTDOWN
		};
		/** Encapsulates a queued request for the background queue */
		struct Request
		{
			BackgroundProcessTicket ticketID;
			RequestType type;
			String resourceName;
			ResourceHandle resourceHandle;
			String resourceType;
			String groupName;
			bool isManual; 
			ManualResourceLoader* loader;
			const NameValuePairList* loadParams;
			Listener* listener;
		};
		typedef std::list<Request> RequestQueue;
		typedef std::map<BackgroundProcessTicket, Request*> RequestTicketMap;
		
		/// Queue of requests, used to store and order requests
		RequestQueue mRequestQueue;
		
		/// Request lookup by ticket
		RequestTicketMap mRequestTicketMap;

		/// Next ticket ID
		unsigned long mNextTicketID;

		/// Struct that holds details of queued notifications
		struct QueuedNotification
		{
			QueuedNotification(Resource* r, bool load)
				: load(load), resource(r)
			{}

			QueuedNotification(const Request &req)
				: load(false), resource(0), req(req)  
			{}

            bool load;
			// Type 1 - Resource::Listener kind
			Resource* resource;
			// Type 2 - ResourceBackgroundQueue::Listener kind
            Request req;
		};
		typedef std::list<QueuedNotification> NotificationQueue;
		/// Queued notifications of background loading being finished
		NotificationQueue mNotificationQueue;
		/// Mutex to protect the background event queue]
		OGRE_MUTEX(mNotificationQueueMutex)

		/// Whether this class should start it's own thread or not
		bool mStartThread;

#if OGRE_THREAD_SUPPORT
		/// The single background thread which will process loading requests
		boost::thread* mThread;
		/// Synchroniser token to wait / notify on queue
		boost::condition mCondition;
		/// Thread function
		static void threadFunc(void);
		/// Internal method for adding a request; also assigns a ticketID
		BackgroundProcessTicket addRequest(Request& req);
		/// Thread shutdown?
		bool mShuttingDown;
#else
		/// Dummy
		void* mThread;
#endif

		/// Private mutex, not allowed to lock from outside
		OGRE_AUTO_MUTEX

		/** Queue the firing of the 'background loading complete' event to
			a Resource::Listener event.
		@remarks
			The purpose of this is to allow the background loading thread to 
			call this method to queue the notification to listeners waiting on
			the background loading of a resource. Rather than allow the resource
			background loading thread to directly call these listeners, which 
			would require all the listeners to be thread-safe, this method
			implements a thread-safe queue which can be processed in the main
			frame loop thread each frame to clear the events in a simpler 
			manner.
		@param listener The listener to be notified
		@param ticket The ticket for the operation that has completed
		*/
		virtual void queueFireBackgroundOperationComplete(Request *req);

	public:
		ResourceBackgroundQueue();
		virtual ~ResourceBackgroundQueue();

		/** Sets whether or not a thread should be created and started to handle
			the background loading, or whether a user thread will call the 
			appropriate hooks.
		@remarks
			By default, a new thread will be started to handle the background 
			load requests. However, the application may well have some threads
			of its own which is wishes to use to perform the background loading
			as well as other tasks (for example on most platforms there will be
			a fixed number of hardware threads which the application will wish
			to work within). Use this method to turn off the creation of a separate
			thread if you wish, and call the _doNextQueuedBackgroundProcess
			method from your own thread to process background requests.
		@note
			You <b>must</b> call this method prior to initialisation. Initialisation
			of this class is automatically done when Root::initialise is called.
		*/
		void setStartBackgroundThread(bool startThread) { mStartThread = startThread; }

		/** Gets whether or not a thread should be created and started to handle
			the background loading, or whether a user thread will call the 
			appropriate hooks.
		*/
		bool getStartBackgroundThread(void) { return mStartThread; }
		/** Initialise the background queue system. 
		@note Called automatically by Root::initialise.
		*/
		virtual void initialise(void);
		
		/** Shut down the background queue system. 
		@note Called automatically by Root::shutdown.
		*/
		virtual void shutdown(void);

		/** Initialise a resource group in the background.
		@see ResourceGroupManager::initialiseResourceGroup
		@param name The name of the resource group to initialise
		@param listener Optional callback interface, take note of warnings in 
			the header and only use if you understand them.
		@returns Ticket identifying the request, use isProcessComplete() to 
			determine if completed if not using listener
		*/
		virtual BackgroundProcessTicket initialiseResourceGroup(
			const String& name, Listener* listener = 0);

		/** Initialise all resource groups which are yet to be initialised in 
			the background.
		@see ResourceGroupManager::intialiseResourceGroup
		@param listener Optional callback interface, take note of warnings in 
			the header and only use if you understand them.
		@returns Ticket identifying the request, use isProcessComplete() to 
			determine if completed if not using listener
		*/
		virtual BackgroundProcessTicket initialiseAllResourceGroups( 
			Listener* listener = 0);
		/** Prepares a resource group in the background.
		@see ResourceGroupManager::prepareResourceGroup
		@param name The name of the resource group to prepare
		@param listener Optional callback interface, take note of warnings in 
			the header and only use if you understand them.
		@returns Ticket identifying the request, use isProcessComplete() to 
			determine if completed if not using listener
		*/
		virtual BackgroundProcessTicket prepareResourceGroup(const String& name, 
			Listener* listener = 0);

		/** Loads a resource group in the background.
		@see ResourceGroupManager::loadResourceGroup
		@param name The name of the resource group to load
		@param listener Optional callback interface, take note of warnings in 
			the header and only use if you understand them.
		@returns Ticket identifying the request, use isProcessComplete() to 
			determine if completed if not using listener
		*/
		virtual BackgroundProcessTicket loadResourceGroup(const String& name, 
			Listener* listener = 0);


		/** Unload a single resource in the background. 
		@see ResourceManager::unload
		@param resType The type of the resource 
			(from ResourceManager::getResourceType())
		@param name The name of the Resource
		*/
		virtual BackgroundProcessTicket unload(
			const String& resType, const String& name, 
			Listener* listener = 0);

		/** Unload a single resource in the background. 
		@see ResourceManager::unload
		@param resType The type of the resource 
			(from ResourceManager::getResourceType())
		@param handle Handle to the resource 
		*/
		virtual BackgroundProcessTicket unload(
			const String& resType, ResourceHandle handle, 
			Listener* listener = 0);

		/** Unloads a resource group in the background.
		@see ResourceGroupManager::unloadResourceGroup
		@param name The name of the resource group to load
		@returns Ticket identifying the request, use isProcessComplete() to 
			determine if completed if not using listener
		*/
		virtual BackgroundProcessTicket unloadResourceGroup(const String& name, 
			Listener* listener = 0);


		/** Prepare a single resource in the background. 
		@see ResourceManager::prepare
		@param resType The type of the resource 
			(from ResourceManager::getResourceType())
		@param name The name of the Resource
		@param group The resource group to which this resource will belong
		@param isManual Is the resource to be manually loaded? If so, you should
			provide a value for the loader parameter
		@param loader The manual loader which is to perform the required actions
			when this resource is loaded; only applicable when you specify true
			for the previous parameter. NOTE: must be thread safe!!
        @param loadParams Optional pointer to a list of name/value pairs 
            containing loading parameters for this type of resource. Remember 
			that this must have a lifespan longer than the return of this call!
		*/
		virtual BackgroundProcessTicket prepare(
			const String& resType, const String& name, 
            const String& group, bool isManual = false, 
			ManualResourceLoader* loader = 0, 
			const NameValuePairList* loadParams = 0, 
			Listener* listener = 0);

		/** Load a single resource in the background. 
		@see ResourceManager::load
		@param resType The type of the resource 
			(from ResourceManager::getResourceType())
		@param name The name of the Resource
		@param group The resource group to which this resource will belong
		@param isManual Is the resource to be manually loaded? If so, you should
			provide a value for the loader parameter
		@param loader The manual loader which is to perform the required actions
			when this resource is loaded; only applicable when you specify true
			for the previous parameter. NOTE: must be thread safe!!
        @param loadParams Optional pointer to a list of name/value pairs 
            containing loading parameters for this type of resource. Remember 
			that this must have a lifespan longer than the return of this call!
		*/
		virtual BackgroundProcessTicket load(
			const String& resType, const String& name, 
            const String& group, bool isManual = false, 
			ManualResourceLoader* loader = 0, 
			const NameValuePairList* loadParams = 0, 
			Listener* listener = 0);
		/** Returns whether a previously queued process has completed or not. 
		@remarks
			This method of checking that a background process has completed is
			the 'polling' approach. Each queued method takes an optional listener
			parameter to allow you to register a callback instead, which is
			arguably more efficient.
		@param ticket The ticket which was returned when the process was queued
		@returns true if process has completed (or if the ticket is 
			unrecognised), false otherwise
		@note Tickets are not stored once complete so do not accumulate over 
			time.
		This is why a non-existent ticket will return 'true'.
		*/
		virtual bool isProcessComplete(BackgroundProcessTicket ticket);

		/** Process a single queued background operation. 
		@remarks
			If you are using your own thread to perform background loading, calling
			this method from that thread triggers the processing of a single
			background loading request from the queue. This method will not 
			return until the request has been fully processed. It also returns
			whether it did in fact process anything - if it returned false, there
			was nothing more in the queue.
		@note
			<b>Do not</b> call this method unless you are using your own thread
			to perform the background loading and called setStartBackgroundThread(false).
			You must only have one background loading thread.
		@returns true if a request was processed, false if the queue was empty.
		*/
		bool _doNextQueuedBackgroundProcess();

		/** Initialise processing for a background thread.
		@remarks
			You must call this method if you use your own thread rather than 
			letting this class create its own. Moreover, you must call it after
			initialise() and after you've started your own thread, but before 
			any resources have been loaded. There are some
			per-thread tasks which have to be performed on some rendering APIs
			and it's important that they are done before rendering resources are
			created.
		@par
			You must call this method in your own background thread, not the main
			thread. It's important to block the main thread whilst this initialisation
			is happening, use an OGRE_THREAD_WAIT on the public initSync token
			after locking the initMutex.
		*/
		void _initThread();

		/** Queue the firing of the 'background preparing complete' event to
			a Resource::Listener event.
		@remarks
			The purpose of this is to allow the background thread to 
			call this method to queue the notification to listeners waiting on
			the background preparing of a resource. Rather than allow the resource
			background thread to directly call these listeners, which 
			would require all the listeners to be thread-safe, this method
			implements a thread-safe queue which can be processed in the main
			frame loop thread each frame to clear the events in a simpler 
			manner.
		@param res The resource listened on
		*/
		virtual void _queueFireBackgroundPreparingComplete(Resource* res);

		/** Queue the firing of the 'background loading complete' event to
			a Resource::Listener event.
		@remarks
			The purpose of this is to allow the background loading thread to 
			call this method to queue the notification to listeners waiting on
			the background loading of a resource. Rather than allow the resource
			background loading thread to directly call these listeners, which 
			would require all the listeners to be thread-safe, this method
			implements a thread-safe queue which can be processed in the main
			frame loop thread each frame to clear the events in a simpler 
			manner.
		@param res The resource listened on
		*/
		virtual void _queueFireBackgroundLoadingComplete(Resource* res);

		/** Fires all the queued events for background loaded resources.
		@remarks
			You should call this from the thread that runs the main frame loop 
			to avoid having to make the receivers of this event thread-safe.
			If you use Ogre's built in frame loop you don't need to call this
			yourself.
		*/
		virtual void _fireOnFrameCallbacks(void);

		/** Override standard Singleton retrieval.
        @remarks
        Why do we do this? Well, it's because the Singleton
        implementation is in a .h file, which means it gets compiled
        into anybody who includes it. This is needed for the
        Singleton template to work, but we actually only want it
        compiled into the implementation of the class based on the
        Singleton, not all of them. If we don't change this, we get
        link errors when trying to use the Singleton-based class from
        an outside dll.
        @par
        This method just delegates to the template version anyway,
        but the implementation stays in this single compilation unit,
        preventing link errors.
        */
        static ResourceBackgroundQueue& getSingleton(void);
        /** Override standard Singleton retrieval.
        @remarks
        Why do we do this? Well, it's because the Singleton
        implementation is in a .h file, which means it gets compiled
        into anybody who includes it. This is needed for the
        Singleton template to work, but we actually only want it
        compiled into the implementation of the class based on the
        Singleton, not all of them. If we don't change this, we get
        link errors when trying to use the Singleton-based class from
        an outside dll.
        @par
        This method just delegates to the template version anyway,
        but the implementation stays in this single compilation unit,
        preventing link errors.
        */
        static ResourceBackgroundQueue* getSingletonPtr(void);
		

	};


}

#endif

