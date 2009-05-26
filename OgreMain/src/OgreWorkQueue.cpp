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
#include "OgreLogManager.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"

namespace Ogre {
	//---------------------------------------------------------------------
	WorkQueue::Request::Request(uint16 channel, uint16 rtype, const Any& rData, uint8 retry, RequestID rid)
		: mChannel(channel), mType(rtype), mData(rData), mRetryCount(retry), mID(rid)
	{

	}
	//---------------------------------------------------------------------
	WorkQueue::Request::~Request()
	{

	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	WorkQueue::Response::Response(const Request* rq, bool success, const Any& data, const String& msg)
		: mRequest(rq), mSuccess(success), mData(data), mMessages(msg)
	{
		
	}
	//---------------------------------------------------------------------
	WorkQueue::Response::~Response()
	{
		OGRE_DELETE mRequest;
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	WorkQueue::WorkQueue(const String& name)
		: mName(name)
		, mWorkerThreadCount(1)
		, mWorkerRenderSystemAccess(false)
		, mIsRunning(false)
		, mResposeTimeLimitMS(0)
		, mRequestCount(0)
		, mPaused(false)
		, mAcceptRequests(true)
		, mRequestsQueuedCount(0)
		, mResponsesQueuedCount(0)
	{
	}
	//---------------------------------------------------------------------
	const String& WorkQueue::getName() const
	{
		return mName;
	}
	//---------------------------------------------------------------------
	size_t WorkQueue::getWorkerThreadCount() const
	{
		return mWorkerThreadCount;
	}
	//---------------------------------------------------------------------
	void WorkQueue::setWorkerThreadCount(size_t c)
	{
		mWorkerThreadCount = c;
	}
	//---------------------------------------------------------------------
	bool WorkQueue::getWorkersCanAccessRenderSystem() const
	{
		return mWorkerRenderSystemAccess;
	}
	//---------------------------------------------------------------------
	void WorkQueue::setWorkersCanAccessRenderSystem(bool access)
	{
		mWorkerRenderSystemAccess = access;
	}
	//---------------------------------------------------------------------
	void WorkQueue::startup(bool forceRestart)
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
			"WorkQueue('" << mName << "') initialising on thread " <<
#if OGRE_THREAD_SUPPORT
			boost::this_thread::get_id()
#else
			"main"
#endif
			<< ".";

#if OGRE_THREAD_SUPPORT
		if (mWorkerRenderSystemAccess)
			Root::getSingleton().getRenderSystem()->preExtraThreadsStarted();

		mNumThreadsRegisteredWithRS = 0;
		for (uint8 i = 0; i < mWorkerThreadCount; ++i)
		{
			WorkerFunc f(this, mWorkerRenderSystemAccess);
			mWorkers.push_back(OGRE_NEW_T(boost::thread, MEMCATEGORY_GENERAL)(f));
		}

		if (mWorkerRenderSystemAccess)
		{
			OGRE_LOCK_MUTEX_NAMED(mInitMutex, initLock)
			// have to wait until all threads are registered with the render system
			while (mNumThreadsRegisteredWithRS < mWorkerThreadCount)
				OGRE_THREAD_WAIT(mInitSync, initLock);

			Root::getSingleton().getRenderSystem()->postExtraThreadsStarted();

		}
#endif

		mIsRunning = true;
	}
	//---------------------------------------------------------------------
	void WorkQueue::_notifyThreadRegistered()
	{
		OGRE_LOCK_MUTEX(mInitMutex)

		++mNumThreadsRegisteredWithRS;

		// wake up main thread
		OGRE_THREAD_NOTIFY_ALL(mInitSync);

	}
	//---------------------------------------------------------------------
	WorkQueue::~WorkQueue()
	{
		shutdown();
	}
	//---------------------------------------------------------------------
	void WorkQueue::shutdown()
	{
		LogManager::getSingleton().stream() <<
			"WorkQueue('" << mName << "') shutting down on thread " <<
#if OGRE_THREAD_SUPPORT
			boost::this_thread::get_id()
#else
			"main"
#endif
			<< ".";

		mShuttingDown = true;
		abortAllRequests();
#if OGRE_THREAD_SUPPORT
		// wake all threads (they should check shutting down as first thing after wait)
		OGRE_THREAD_NOTIFY_ALL(mRequestCondition)

		// all our threads should have been woken now, so join
		for (WorkerThreadList::iterator i = mWorkers.begin(); i != mWorkers.end(); ++i)
		{
			(*i)->join();
			using namespace boost;
			OGRE_DELETE_T(*i, thread, MEMCATEGORY_GENERAL);
		}
		mWorkers.clear();
#endif
		mRequestsQueuedCount = 0;
		mResponsesQueuedCount = 0;

		mIsRunning = false;


	}
	//---------------------------------------------------------------------
	void WorkQueue::addRequestHandler(uint16 channel, RequestHandler* rh)
	{
		OGRE_LOCK_MUTEX(mRequestHandlerMutex)

		RequestHandlerListByChannel::iterator i = mRequestHandlers.find(channel);
		if (i == mRequestHandlers.end())
			i = mRequestHandlers.insert(RequestHandlerListByChannel::value_type(channel, RequestHandlerList())).first;

		RequestHandlerList& handlers = i->second;
		if (std::find(handlers.begin(), handlers.end(), rh) == handlers.end())
			handlers.push_back(rh);

	}
	//---------------------------------------------------------------------
	void WorkQueue::removeRequestHandler(uint16 channel, RequestHandler* rh)
	{
		OGRE_LOCK_MUTEX(mRequestHandlerMutex)

		RequestHandlerListByChannel::iterator i = mRequestHandlers.find(channel);
		if (i != mRequestHandlers.end())
		{
			RequestHandlerList& handlers = i->second;
			RequestHandlerList::iterator j = std::find(
				handlers.begin(), handlers.end(), rh);
			if (j != handlers.end())
				handlers.erase(j);

		}

	}
	//---------------------------------------------------------------------
	void WorkQueue::addResponseHandler(uint16 channel, ResponseHandler* rh)
	{
		OGRE_LOCK_MUTEX(mResponseHandlerMutex)

		ResponseHandlerListByChannel::iterator i = mResponseHandlers.find(channel);
		if (i == mResponseHandlers.end())
			i = mResponseHandlers.insert(ResponseHandlerListByChannel::value_type(channel, ResponseHandlerList())).first;

		ResponseHandlerList& handlers = i->second;
		if (std::find(handlers.begin(), handlers.end(), rh) == handlers.end())
			handlers.push_back(rh);
	}
	//---------------------------------------------------------------------
	void WorkQueue::removeResponseHandler(uint16 channel, ResponseHandler* rh)
	{
		OGRE_LOCK_MUTEX(mResponseHandlerMutex)

		ResponseHandlerListByChannel::iterator i = mResponseHandlers.find(channel);
		if (i != mResponseHandlers.end())
		{
			ResponseHandlerList& handlers = i->second;
			ResponseHandlerList::iterator j = std::find(
				handlers.begin(), handlers.end(), rh);
			if (j != handlers.end())
				handlers.erase(j);

		}
	}
	//---------------------------------------------------------------------
	WorkQueue::RequestID WorkQueue::addRequest(uint16 channel, uint16 requestType, 
		const Any& rData, uint8 retryCount, bool forceSynchronous)
	{
		OGRE_LOCK_MUTEX(mRequestMutex)

		if (!mAcceptRequests)
			return 0;

		RequestID rid = ++mRequestCount;
		Request* req = OGRE_NEW Request(channel, requestType, rData, retryCount, rid);

		LogManager::getSingleton().stream(LML_TRIVIAL) << 
			"WorkQueue('" << mName << "') - QUEUED(thread:" <<
#if OGRE_THREAD_SUPPORT
			boost::this_thread::get_id()
#else
			"main"
#endif
			<< "): ID=" << rid
			<< " channel=" << channel << " requestType=" << requestType;

#if OGRE_THREAD_SUPPORT
		if (forceSynchronous)
		{
			processRequestResponse(req, true);
		}
		else
		{
			mRequestQueue.push_back(req);
			++mRequestsQueuedCount;
			// wake up waiting thread
			OGRE_THREAD_NOTIFY_ONE(mRequestCondition)
		}
#else
		processRequestResponse(req, true);
#endif

		return rid;

	}
	//---------------------------------------------------------------------
	void WorkQueue::abortRequest(RequestID id)
	{
		OGRE_LOCK_MUTEX(mRequestMutex)

		for (RequestQueue::iterator i = mRequestQueue.begin(); i != mRequestQueue.end(); ++i)
		{
			if ((*i)->getID() == id)
			{
				OGRE_DELETE *i;
				mRequestQueue.erase(i);
				--mRequestsQueuedCount;
				break;
			}
		}
	}
	//---------------------------------------------------------------------
	void WorkQueue::abortRequestsByChannel(uint16 channel)
	{
		OGRE_LOCK_MUTEX(mRequestMutex)

		for (RequestQueue::iterator i = mRequestQueue.begin(); i != mRequestQueue.end();)
		{
			if ((*i)->getChannel() == channel)
			{
				OGRE_DELETE *i;
				i = mRequestQueue.erase(i);
				--mRequestsQueuedCount;
			}
			else
				++i;
		}
	}
	//---------------------------------------------------------------------
	void WorkQueue::abortAllRequests()
	{
		OGRE_LOCK_MUTEX(mRequestMutex)

		for (RequestQueue::iterator i = mRequestQueue.begin(); i != mRequestQueue.end();)
		{
			OGRE_DELETE *i;
		}
		mRequestQueue.clear();
		mRequestsQueuedCount = 0;
	}
	//---------------------------------------------------------------------
	void WorkQueue::setPaused(bool pause)
	{
		OGRE_LOCK_MUTEX(mRequestMutex)

		mPaused = pause;
	}
	//---------------------------------------------------------------------
	bool WorkQueue::isPaused() const
	{
		return mPaused;
	}
	//---------------------------------------------------------------------
	void WorkQueue::setRequestsAccepted(bool accept)
	{
		OGRE_LOCK_MUTEX(mRequestMutex)

		mAcceptRequests = accept;
	}
	//---------------------------------------------------------------------
	bool WorkQueue::getRequestsAccepted() const
	{
		return mAcceptRequests;
	}
	//---------------------------------------------------------------------
	size_t WorkQueue::geRequestsQueuedCount() const
	{
		return mRequestsQueuedCount;
	}
	//---------------------------------------------------------------------
	size_t WorkQueue::getResponsesQueuedCount() const
	{
		return mResponsesQueuedCount;
	}
	//---------------------------------------------------------------------
	void WorkQueue::_processNextRequest()
	{
		Request* request = 0;
		{
			// scoped to only lock while retrieving the next request
			OGRE_LOCK_MUTEX(mRequestMutex)

			if (!mRequestQueue.empty())
			{
				request = mRequestQueue.front();
				mRequestQueue.pop_front();
				--mRequestsQueuedCount;
			}
		}

		if (request)
		{
			processRequestResponse(request, false);
		}


	}
	//---------------------------------------------------------------------
	void WorkQueue::processRequestResponse(Request* r, bool synchronous)
	{
		Response* response = processRequest(r);

		if (response)
		{
			if (!response->succeeded())
			{
				// Failed, should we retry?
				const Request* req = response->getRequest();
				if (req->getRetryCount())
				{
					addRequest(req->getChannel(), req->getType(), req->getData(), 
						req->getRetryCount() - 1, false);
					// discard response (this also deletes request)
					OGRE_DELETE response;
					return;
				}
			}
			if (synchronous)
			{
				processResponse(response);
			}
			else
			{
				// Queue response
				OGRE_LOCK_MUTEX(mResponseMutex)
				mResponseQueue.push_back(response);
				++mResponsesQueuedCount;
				// no need to wake thread, this is processed by the main thread
			}

		}
		else
		{
			// no response, delete request
			LogManager::getSingleton().stream() << 
				"WorkQueue('" << mName << "') warning: no handler processed request "
				<< r->getID() << ", channel " << r->getChannel()
				<< ", type " << r->getType();
			OGRE_DELETE r;
		}

	}
	//---------------------------------------------------------------------
	void WorkQueue::_waitForNextRequest()
	{
#if OGRE_THREAD_SUPPORT
		// Lock; note that 'boost::condition.wait()' will free the lock
		OGRE_LOCK_MUTEX_NAMED(mRequestMutex, queueLock);
		if (mRequestQueue.empty())
		{
			// frees lock and suspends the thread
			OGRE_THREAD_WAIT(mRequestCondition, queueLock);
		}
		// When we get back here, it's because we've been notified 
		// and thus the thread as been woken up. Lock has also been
		// re-acquired, but we won't use it. It's safe to try processing and fail
		// if another thread has got in first and grabbed the request
#endif

	}
	//---------------------------------------------------------------------
	void WorkQueue::processResponses() 
	{
		unsigned long msStart = Root::getSingleton().getTimer()->getMilliseconds();
		unsigned long msCurrent = msStart;

		// keep going until we run out of responses or out of time
		while(true)
		{
			Response* response = 0;
			{
				OGRE_LOCK_MUTEX(mResponseMutex)

				if (mResponseQueue.empty())
					break; // exit loop
				else
				{
					response = mResponseQueue.front();
					mResponseQueue.pop_front();
					--mResponsesQueuedCount;
				}
			}

			if (response)
			{
				processResponse(response);

				OGRE_DELETE response;

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
	//---------------------------------------------------------------------
	WorkQueue::Response* WorkQueue::processRequest(Request* r)
	{
		OGRE_LOCK_MUTEX(mRequestHandlerMutex)

		Response* response = 0;

		StringUtil::StrStreamType dbgMsg;
		dbgMsg <<
#if OGRE_THREAD_SUPPORT
			boost::this_thread::get_id()
#else
			"main"
#endif
			<< "): ID=" << r->getID() << " channel=" << r->getChannel() 
			<< " requestType=" << r->getType();

		LogManager::getSingleton().stream(LML_TRIVIAL) << 
			"WorkQueue('" << mName << "') - PROCESS_REQUEST_START(" << dbgMsg.str();

		RequestHandlerListByChannel::iterator i = mRequestHandlers.find(r->getChannel());
		if (i != mRequestHandlers.end())
		{
			RequestHandlerList& handlers = i->second;
			for (RequestHandlerList::reverse_iterator j = handlers.rbegin(); j != handlers.rend(); ++j)
			{
				if ((*j)->canHandleRequest(r, this))
				{
					response = (*j)->handleRequest(r, this);
				}
			}
		}

		LogManager::getSingleton().stream(LML_TRIVIAL) << 
			"WorkQueue('" << mName << "') - PROCESS_REQUEST_END(" << dbgMsg.str()
			<< " processed=" << (response!=0);

		return response;

	}
	//---------------------------------------------------------------------
	void WorkQueue::processResponse(Response* r)
	{
		OGRE_LOCK_MUTEX(mResponseHandlerMutex)

		StringUtil::StrStreamType dbgMsg;
		dbgMsg << "thread:" <<
#if OGRE_THREAD_SUPPORT
			boost::this_thread::get_id()
#else
			"main"
#endif
			<< "): ID=" << r->getRequest()->getID()
			<< " success=" << r->succeeded() << " messages=[" << r->getMessages() << "] channel=" 
			<< r->getRequest()->getChannel() << " requestType=" << r->getRequest()->getType();

		LogManager::getSingleton().stream(LML_TRIVIAL) << 
			"WorkQueue('" << mName << "') - PROCESS_RESPONSE_START(" << dbgMsg.str();

		ResponseHandlerListByChannel::iterator i = mResponseHandlers.find(r->getRequest()->getChannel());
		if (i != mResponseHandlers.end())
		{
			ResponseHandlerList& handlers = i->second;
			for (ResponseHandlerList::reverse_iterator j = handlers.rbegin(); j != handlers.rend(); ++j)
			{
				if ((*j)->canHandleResponse(r, this))
				{
					(*j)->handleResponse(r, this);
				}
			}
		}
		LogManager::getSingleton().stream(LML_TRIVIAL) << 
			"WorkQueue('" << mName << "') - PROCESS_RESPONSE_END(" << dbgMsg.str();

	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	void WorkQueue::WorkerFunc::operator()()
	{
		// default worker thread
#if OGRE_THREAD_SUPPORT
		LogManager::getSingleton().stream() << 
			"WorkQueue('" << mQueue->getName() << "')::WorkerFunc - thread " 
			<< boost::this_thread::get_id() << " starting.";

		// Initialise the thread for RS if necessary
		if (mRenderSysAccess)
		{
			Root::getSingleton().getRenderSystem()->registerThread();
			mQueue->_notifyThreadRegistered();
		}

		// Spin forever until we're told to shut down
		while (!mQueue->isShuttingDown())
		{
			mQueue->_waitForNextRequest();
			mQueue->_processNextRequest();
		}

		LogManager::getSingleton().stream() << 
			"WorkQueue('" << mQueue->getName() << "')::WorkerFunc - thread " 
			<< boost::this_thread::get_id() << " stopped.";
#endif
	}

}
