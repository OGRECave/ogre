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
#include "OgreWorkQueue.h"

namespace Ogre {
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Resources
	*  @{
	*/

	/// Identifier of a background process
	typedef WorkQueue::RequestID BackgroundProcessTicket;

	/** Encapsulates the result of a background queue request */
	struct BackgroundProcessResult
	{
		/// Whether an error occurred
		bool error;
		/// Any messages from the process
		String message;

		BackgroundProcessResult() : error(false) {}
	};

	
	/** This class is used to perform Resource operations in a
		background thread. 
	@remarks
		All these requests are now queued via Root::getWorkQueue in order
		to share the thread pool amongst all background tasks. You should therefore
		refer to that class for configuring the behaviour of the threads
		themselves, this class merely provides an interface that is specific
		to resource loading around this common functionality.
	@par
		The general approach here is that on requesting a background resource
		process, your request is placed on a queue ready for the background
		thread to be picked up, and you will get a 'ticket' back, identifying
		the request. Your call will then return and your thread can
		proceed, knowing that at some point in the background the operation will 
		be performed. In it's own thread, the resource operation will be 
		performed, and once finished the ticket will be marked as complete. 
		You can check the status of tickets by calling isProcessComplete() 
		from your queueing thread. 
	*/
	class _OgreExport ResourceBackgroundQueue : public Singleton<ResourceBackgroundQueue>, public ResourceAlloc, 
		public WorkQueue::RequestHandler, public WorkQueue::ResponseHandler
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
			virtual void operationCompleted(BackgroundProcessTicket ticket, const BackgroundProcessResult& result) = 0;
			/// Need virtual destructor in case subclasses use it
			virtual ~Listener() {}

		};

	protected:
		/** Enumerates the type of requests */
		enum RequestType
		{
			RT_INITIALISE_GROUP = 0,
			RT_INITIALISE_ALL_GROUPS = 1,
			RT_PREPARE_GROUP = 2,
			RT_PREPARE_RESOURCE = 3,
			RT_LOAD_GROUP = 4,
			RT_LOAD_RESOURCE = 5,
			RT_UNLOAD_GROUP = 6,
			RT_UNLOAD_RESOURCE = 7
		};
		/** Encapsulates a queued request for the background queue */
		struct ResourceRequest
		{
			RequestType type;
			String resourceName;
			ResourceHandle resourceHandle;
			String resourceType;
			String groupName;
			bool isManual; 
			ManualResourceLoader* loader;
			const NameValuePairList* loadParams;
			Listener* listener;
			BackgroundProcessResult result;

			_OgreExport friend std::ostream& operator<<(std::ostream& o, const ResourceRequest& r)
			{ return o; }
		};

		typedef set<BackgroundProcessTicket>::type OutstandingRequestSet;	
		OutstandingRequestSet mOutstandingRequestSet;

		/// Struct that holds details of queued notifications
		struct ResourceResponse
		{
			ResourceResponse(Resource* r, const ResourceRequest& req)
				: resource(r), request(req)
			{}

			Resource* resource;
			ResourceRequest request;

			_OgreExport friend std::ostream& operator<<(std::ostream& o, const ResourceResponse& r)
			{ return o; }
		};

		BackgroundProcessTicket addRequest(ResourceRequest& req);

	public:
		ResourceBackgroundQueue();
		virtual ~ResourceBackgroundQueue();

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

		/// Implementation for WorkQueue::RequestHandler
		WorkQueue::Response* handleRequest(const WorkQueue::Request* req, const WorkQueue* srcQ);
		/// Implementation for WorkQueue::RequestHandler
		void handleResponse(const WorkQueue::Response* res, const WorkQueue* srcQ);

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

	/** @} */
	/** @} */

}

#endif

