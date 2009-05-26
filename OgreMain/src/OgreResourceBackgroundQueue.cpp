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
#include "OgreResourceBackgroundQueue.h"
#include "OgreLogManager.h"
#include "OgreException.h"
#include "OgreResourceGroupManager.h"
#include "OgreResourceManager.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"

namespace Ogre {

	// Note, no locks are required here anymore because all of the parallelisation
	// is now contained in WorkQueue - this class is entirely single-threaded

#define RESOURCE_CHANNEL Root::MAX_USER_WORKQUEUE_CHANNEL + 1

	//------------------------------------------------------------------------
    //-----------------------------------------------------------------------
    template<> ResourceBackgroundQueue* Singleton<ResourceBackgroundQueue>::ms_Singleton = 0;
    ResourceBackgroundQueue* ResourceBackgroundQueue::getSingletonPtr(void)
    {
        return ms_Singleton;
    }
    ResourceBackgroundQueue& ResourceBackgroundQueue::getSingleton(void)
    {  
        assert( ms_Singleton );  return ( *ms_Singleton );  
    }
    //-----------------------------------------------------------------------	
	//------------------------------------------------------------------------
	ResourceBackgroundQueue::ResourceBackgroundQueue()
	{
	}
	//------------------------------------------------------------------------
	ResourceBackgroundQueue::~ResourceBackgroundQueue()
	{
		shutdown();
	}
	//---------------------------------------------------------------------
	void ResourceBackgroundQueue::initialise()
	{
		Root::getSingleton().getWorkQueue()->addResponseHandler(RESOURCE_CHANNEL, this);
		Root::getSingleton().getWorkQueue()->addRequestHandler(RESOURCE_CHANNEL, this);
	}
	//---------------------------------------------------------------------
	void ResourceBackgroundQueue::shutdown()
	{
		Root::getSingleton().getWorkQueue()->removeRequestHandler(RESOURCE_CHANNEL, this);
		Root::getSingleton().getWorkQueue()->removeResponseHandler(RESOURCE_CHANNEL, this);
	}
	//------------------------------------------------------------------------
	BackgroundProcessTicket ResourceBackgroundQueue::initialiseResourceGroup(
		const String& name, ResourceBackgroundQueue::Listener* listener)
	{
#if OGRE_THREAD_SUPPORT
		// queue a request
		ResourceRequest req;
		req.type = RT_INITIALISE_GROUP;
		req.groupName = name;
		req.listener = listener;
		return addRequest(req);
#else
		// synchronous
		ResourceGroupManager::getSingleton().initialiseResourceGroup(name);
		return 0; 
#endif
	}
	//------------------------------------------------------------------------
	BackgroundProcessTicket 
	ResourceBackgroundQueue::initialiseAllResourceGroups( 
		ResourceBackgroundQueue::Listener* listener)
	{
#if OGRE_THREAD_SUPPORT
		// queue a request
		ResourceRequest req;
		req.type = RT_INITIALISE_ALL_GROUPS;
		req.listener = listener;
		return addRequest(req);
#else
		// synchronous
		ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
		return 0; 
#endif
	}
	//------------------------------------------------------------------------
	BackgroundProcessTicket ResourceBackgroundQueue::prepareResourceGroup(
		const String& name, ResourceBackgroundQueue::Listener* listener)
	{
#if OGRE_THREAD_SUPPORT
		// queue a request
		ResourceRequest req;
		req.type = RT_PREPARE_GROUP;
		req.groupName = name;
		req.listener = listener;
		return addRequest(req);
#else
		// synchronous
		ResourceGroupManager::getSingleton().prepareResourceGroup(name);
		return 0; 
#endif
	}
	//------------------------------------------------------------------------
	BackgroundProcessTicket ResourceBackgroundQueue::loadResourceGroup(
		const String& name, ResourceBackgroundQueue::Listener* listener)
	{
#if OGRE_THREAD_SUPPORT
		// queue a request
		ResourceRequest req;
		req.type = RT_LOAD_GROUP;
		req.groupName = name;
		req.listener = listener;
		return addRequest(req);
#else
		// synchronous
		ResourceGroupManager::getSingleton().loadResourceGroup(name);
		return 0; 
#endif
	}
	//------------------------------------------------------------------------
	BackgroundProcessTicket ResourceBackgroundQueue::prepare(
		const String& resType, const String& name, 
		const String& group, bool isManual, 
		ManualResourceLoader* loader, 
		const NameValuePairList* loadParams, 
		ResourceBackgroundQueue::Listener* listener)
	{
#if OGRE_THREAD_SUPPORT
		// queue a request
		ResourceRequest req;
		req.type = RT_PREPARE_RESOURCE;
		req.resourceType = resType;
		req.resourceName = name;
		req.groupName = group;
		req.isManual = isManual;
		req.loader = loader;
		req.loadParams = loadParams;
		req.listener = listener;
		return addRequest(req);
#else
		// synchronous
		ResourceManager* rm = 
			ResourceGroupManager::getSingleton()._getResourceManager(resType);
		rm->prepare(name, group, isManual, loader, loadParams);
		return 0; 
#endif
	}
	//------------------------------------------------------------------------
	BackgroundProcessTicket ResourceBackgroundQueue::load(
		const String& resType, const String& name, 
		const String& group, bool isManual, 
		ManualResourceLoader* loader, 
		const NameValuePairList* loadParams, 
		ResourceBackgroundQueue::Listener* listener)
	{
#if OGRE_THREAD_SUPPORT
		// queue a request
		ResourceRequest req;
		req.type = RT_LOAD_RESOURCE;
		req.resourceType = resType;
		req.resourceName = name;
		req.groupName = group;
		req.isManual = isManual;
		req.loader = loader;
		req.loadParams = loadParams;
		req.listener = listener;
		return addRequest(req);
#else
		// synchronous
		ResourceManager* rm = 
			ResourceGroupManager::getSingleton()._getResourceManager(resType);
		rm->load(name, group, isManual, loader, loadParams);
		return 0; 
#endif
	}
	//---------------------------------------------------------------------
	BackgroundProcessTicket ResourceBackgroundQueue::unload(
		const String& resType, const String& name, Listener* listener)
	{
#if OGRE_THREAD_SUPPORT
		// queue a request
		ResourceRequest req;
		req.type = RT_UNLOAD_RESOURCE;
		req.resourceType = resType;
		req.resourceName = name;
		req.listener = listener;
		return addRequest(req);
#else
		// synchronous
		ResourceManager* rm = 
			ResourceGroupManager::getSingleton()._getResourceManager(resType);
		rm->unload(name);
		return 0; 
#endif

	}
	//---------------------------------------------------------------------
	BackgroundProcessTicket ResourceBackgroundQueue::unload(
		const String& resType, ResourceHandle handle, Listener* listener)
	{
#if OGRE_THREAD_SUPPORT
		// queue a request
		ResourceRequest req;
		req.type = RT_UNLOAD_RESOURCE;
		req.resourceType = resType;
		req.resourceHandle = handle;
		req.listener = listener;
		return addRequest(req);
#else
		// synchronous
		ResourceManager* rm = 
			ResourceGroupManager::getSingleton()._getResourceManager(resType);
		rm->unload(handle);
		return 0; 
#endif

	}
	//---------------------------------------------------------------------
	BackgroundProcessTicket ResourceBackgroundQueue::unloadResourceGroup(
		const String& name, Listener* listener)
	{
#if OGRE_THREAD_SUPPORT
		// queue a request
		ResourceRequest req;
		req.type = RT_UNLOAD_GROUP;
		req.groupName = name;
		req.listener = listener;
		return addRequest(req);
#else
		// synchronous
		ResourceGroupManager::getSingleton().unloadResourceGroup(name);
		return 0; 
#endif

	}
	//------------------------------------------------------------------------
	bool ResourceBackgroundQueue::isProcessComplete(
			BackgroundProcessTicket ticket)
	{
		return mOutstandingRequestSet.find(ticket) == mOutstandingRequestSet.end();
	}
	//------------------------------------------------------------------------
	BackgroundProcessTicket ResourceBackgroundQueue::addRequest(ResourceRequest& req)
	{
		WorkQueue* queue = Root::getSingleton().getWorkQueue();

		Any data(req);

		WorkQueue::RequestID requestID = 
			queue->addRequest(RESOURCE_CHANNEL, (uint16)req.type, data);


		mOutstandingRequestSet.insert(requestID);

		return requestID;
	}
	//-----------------------------------------------------------------------
	WorkQueue::Response* ResourceBackgroundQueue::handleRequest(const WorkQueue::Request* req, const WorkQueue* srcQ)
	{

		ResourceRequest resreq = any_cast<ResourceRequest>(req->getData());
		
		ResourceManager* rm = 0;
		Resource* resource = 0;
		try
		{

			switch (resreq.type)
			{
			case RT_INITIALISE_GROUP:
				ResourceGroupManager::getSingleton().initialiseResourceGroup(
					resreq.groupName);
				break;
			case RT_INITIALISE_ALL_GROUPS:
				ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
				break;
			case RT_PREPARE_GROUP:
				ResourceGroupManager::getSingleton().prepareResourceGroup(
					resreq.groupName);
				break;
			case RT_LOAD_GROUP:
	#if OGRE_THREAD_SUPPORT == 2
				ResourceGroupManager::getSingleton().prepareResourceGroup(
					resreq.groupName);
	#else
				ResourceGroupManager::getSingleton().loadResourceGroup(
					resreq.groupName);
	#endif
				break;
			case RT_UNLOAD_GROUP:
				ResourceGroupManager::getSingleton().unloadResourceGroup(
					resreq.groupName);
				break;
			case RT_PREPARE_RESOURCE:
				rm = ResourceGroupManager::getSingleton()._getResourceManager(
					resreq.resourceType);
				resource = rm->prepare(resreq.resourceName, resreq.groupName, resreq.isManual, 
					resreq.loader, resreq.loadParams).get();
				break;
			case RT_LOAD_RESOURCE:
				rm = ResourceGroupManager::getSingleton()._getResourceManager(
					resreq.resourceType);
	#if OGRE_THREAD_SUPPORT == 2
				resource = rm->prepare(resreq.resourceName, resreq.groupName, resreq.isManual, 
					resreq.loader, resreq.loadParams).get();
	#else
				resource = rm->load(resreq.resourceName, resreq.groupName, resreq.isManual, 
					resreq.loader, resreq.loadParams).get();
	#endif
				break;
			case RT_UNLOAD_RESOURCE:
				rm = ResourceGroupManager::getSingleton()._getResourceManager(
					resreq.resourceType);
				if (resreq.resourceName.empty())
					rm->unload(resreq.resourceHandle);
				else
					rm->unload(resreq.resourceName);
				break;
			};
		}
		catch (Exception& e)
		{
			resreq.result.error = true;
			resreq.result.message = e.getFullDescription();

			// return error response
			ResourceResponse resresp(resource, resreq);
			return OGRE_NEW WorkQueue::Response(req, false, Any(resresp), e.getFullDescription());
		}


		// success
		resreq.result.error = false;
		ResourceResponse resresp(resource, resreq);
		return OGRE_NEW WorkQueue::Response(req, true, Any(resresp));

	}
	//------------------------------------------------------------------------
	void ResourceBackgroundQueue::handleResponse(const WorkQueue::Response* res, const WorkQueue* srcQ)
	{
		if (res->succeeded())
		{
			ResourceResponse resresp = any_cast<ResourceResponse>(res->getData());

			// Complete full loading in main thread if semithreading
			const ResourceRequest& req = resresp.request;
#if OGRE_THREAD_SUPPORT == 2
			// These load commands would have been downgraded to prepare() for the background
			if (req.type == RT_LOAD_RESOURCE)
			{
				ResourceManager *rm = ResourceGroupManager::getSingleton()
					._getResourceManager(req.resourceType);
				rm->load(req.resourceName, req.groupName, req.isManual, req.loader, req.loadParams);
			} 
			else if (req.type == RT_LOAD_GROUP) 
			{
				ResourceGroupManager::getSingleton().loadResourceGroup(req.groupName);
			}
#endif

			// Call resource listener
			if (resresp.resource) 
			{

				if (req.type == RT_LOAD_RESOURCE) 
				{
					resresp.resource->_fireLoadingComplete();
				} 
				else 
				{
					resresp.resource->_firePreparingComplete();
				}
			} 

			// Call queue listener
			if (req.listener)
				req.listener->operationCompleted(res->getRequest()->getID(), req.result);

		}
	}
	//------------------------------------------------------------------------

}



