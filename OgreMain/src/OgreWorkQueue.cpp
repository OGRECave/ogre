/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
#include "OgreLogManager.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"

namespace Ogre {
	//---------------------------------------------------------------------
	uint16 WorkQueue::getChannel(const String& channelName)
	{
		OGRE_LOCK_MUTEX(mChannelMapMutex)

		ChannelMap::iterator i = mChannelMap.find(channelName);
		if (i == mChannelMap.end())
		{
			i = mChannelMap.insert(ChannelMap::value_type(channelName, mNextChannel++)).first;
		}
		return i->second;
	}
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
		, mResposeTimeLimitMS(0)
		, mWorkerFunc(0)
		, mRequestCount(0)
		, mPaused(false)
		, mAcceptRequests(true)
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
	//---------------------------------------------------------------------
	void DefaultWorkQueueBase::addRequestHandler(uint16 channel, RequestHandler* rh)
	{
		OGRE_LOCK_RW_MUTEX_WRITE(mRequestHandlerMutex);

		RequestHandlerListByChannel::iterator i = mRequestHandlers.find(channel);
		if (i == mRequestHandlers.end())
			i = mRequestHandlers.insert(RequestHandlerListByChannel::value_type(channel, RequestHandlerList())).first;

		RequestHandlerList& handlers = i->second;
		if (std::find(handlers.begin(), handlers.end(), rh) == handlers.end())
			handlers.push_back(rh);

	}
	//---------------------------------------------------------------------
	void DefaultWorkQueueBase::removeRequestHandler(uint16 channel, RequestHandler* rh)
	{
		OGRE_LOCK_RW_MUTEX_WRITE(mRequestHandlerMutex);

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
	void DefaultWorkQueueBase::addResponseHandler(uint16 channel, ResponseHandler* rh)
	{
		ResponseHandlerListByChannel::iterator i = mResponseHandlers.find(channel);
		if (i == mResponseHandlers.end())
			i = mResponseHandlers.insert(ResponseHandlerListByChannel::value_type(channel, ResponseHandlerList())).first;

		ResponseHandlerList& handlers = i->second;
		if (std::find(handlers.begin(), handlers.end(), rh) == handlers.end())
			handlers.push_back(rh);
	}
	//---------------------------------------------------------------------
	void DefaultWorkQueueBase::removeResponseHandler(uint16 channel, ResponseHandler* rh)
	{
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
	WorkQueue::RequestID DefaultWorkQueueBase::addRequest(uint16 channel, uint16 requestType, 
		const Any& rData, uint8 retryCount, bool forceSynchronous)
	{
		Request* req = 0;
		RequestID rid = 0;

		{
			// lock to acquire rid and push request to the queue
			OGRE_LOCK_MUTEX(mRequestMutex)

			if (!mAcceptRequests || mShuttingDown)
				return 0;

			rid = ++mRequestCount;
			req = OGRE_NEW Request(channel, requestType, rData, retryCount, rid);

			LogManager::getSingleton().stream(LML_TRIVIAL) << 
				"DefaultWorkQueueBase('" << mName << "') - QUEUED(thread:" <<
#if OGRE_THREAD_SUPPORT
				OGRE_THREAD_CURRENT_ID
#else
				"main"
#endif
				<< "): ID=" << rid
			    << " channel=" << channel << " requestType=" << requestType;
#if OGRE_THREAD_SUPPORT
			if (!forceSynchronous)
			{
				mRequestQueue.push_back(req);
				notifyWorkers();
				return rid;
			}
#endif
		}
		

		processRequestResponse(req, true);

		return rid;

	}
	//---------------------------------------------------------------------
	void DefaultWorkQueueBase::addRequestWithRID(WorkQueue::RequestID rid, uint16 channel, 
		uint16 requestType, const Any& rData, uint8 retryCount)
	{
		// lock to push request to the queue
		OGRE_LOCK_MUTEX(mRequestMutex)

		if (mShuttingDown)
			return;

		Request* req = OGRE_NEW Request(channel, requestType, rData, retryCount, rid);

		LogManager::getSingleton().stream(LML_TRIVIAL) << 
			"DefaultWorkQueueBase('" << mName << "') - REQUEUED(thread:" <<
#if OGRE_THREAD_SUPPORT
			OGRE_THREAD_CURRENT_ID
#else
			"main"
#endif
			<< "): ID=" << rid
				   << " channel=" << channel << " requestType=" << requestType;
#if OGRE_THREAD_SUPPORT
		mRequestQueue.push_back(req);
		notifyWorkers();
#else
		processRequestResponse(req, true);
#endif
	}
	//---------------------------------------------------------------------
	void DefaultWorkQueueBase::abortRequest(RequestID id)
	{
		OGRE_LOCK_MUTEX(mRequestMutex)

		for (RequestQueue::iterator i = mRequestQueue.begin(); i != mRequestQueue.end(); ++i)
		{
			if ((*i)->getID() == id)
			{
				OGRE_DELETE *i;
				mRequestQueue.erase(i);
				break;
			}
		}
	}
	//---------------------------------------------------------------------
	void DefaultWorkQueueBase::abortRequestsByChannel(uint16 channel)
	{
		OGRE_LOCK_MUTEX(mRequestMutex)

		for (RequestQueue::iterator i = mRequestQueue.begin(); i != mRequestQueue.end();)
		{
			if ((*i)->getChannel() == channel)
			{
				OGRE_DELETE *i;
				i = mRequestQueue.erase(i);
			}
			else
				++i;
		}
	}
	//---------------------------------------------------------------------
	void DefaultWorkQueueBase::abortAllRequests()
	{
		OGRE_LOCK_MUTEX(mRequestMutex)

		for (RequestQueue::iterator i = mRequestQueue.begin(); i != mRequestQueue.end();)
		{
			OGRE_DELETE *i;
		}
		mRequestQueue.clear();
	}
	//---------------------------------------------------------------------
	void DefaultWorkQueueBase::setPaused(bool pause)
	{
		OGRE_LOCK_MUTEX(mRequestMutex)

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
		OGRE_LOCK_MUTEX(mRequestMutex)

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
		Request* request = 0;
		{
			// scoped to only lock while retrieving the next request
			OGRE_LOCK_MUTEX(mRequestMutex)

			if (!mRequestQueue.empty())
			{
				request = mRequestQueue.front();
				mRequestQueue.pop_front();
			}
		}

		if (request)
		{
			processRequestResponse(request, false);
		}


	}
	//---------------------------------------------------------------------
	void DefaultWorkQueueBase::processRequestResponse(Request* r, bool synchronous)
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
					addRequestWithRID(req->getID(), req->getChannel(), req->getType(), req->getData(), 
						req->getRetryCount() - 1);
					// discard response (this also deletes request)
					OGRE_DELETE response;
					return;
				}
			}
			if (synchronous)
			{
				processResponse(response);
				OGRE_DELETE response;
			}
			else
			{
				// Queue response
				OGRE_LOCK_MUTEX(mResponseMutex)
				mResponseQueue.push_back(response);
				// no need to wake thread, this is processed by the main thread
			}

		}
		else
		{
			// no response, delete request
			LogManager::getSingleton().stream() << 
				"DefaultWorkQueueBase('" << mName << "') warning: no handler processed request "
				<< r->getID() << ", channel " << r->getChannel()
				<< ", type " << r->getType();
			OGRE_DELETE r;
		}

	}
	//---------------------------------------------------------------------
	void DefaultWorkQueueBase::processResponses() 
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
	WorkQueue::Response* DefaultWorkQueueBase::processRequest(Request* r)
	{
		OGRE_LOCK_RW_MUTEX_READ(mRequestHandlerMutex);

		Response* response = 0;

		StringUtil::StrStreamType dbgMsg;
		dbgMsg <<
#if OGRE_THREAD_SUPPORT
			OGRE_THREAD_CURRENT_ID
#else
			"main"
#endif
			<< "): ID=" << r->getID() << " channel=" << r->getChannel() 
			<< " requestType=" << r->getType();

		LogManager::getSingleton().stream(LML_TRIVIAL) << 
			"DefaultWorkQueueBase('" << mName << "') - PROCESS_REQUEST_START(" << dbgMsg.str();

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
			"DefaultWorkQueueBase('" << mName << "') - PROCESS_REQUEST_END(" << dbgMsg.str()
			<< " processed=" << (response!=0);

		return response;

	}
	//---------------------------------------------------------------------
	void DefaultWorkQueueBase::processResponse(Response* r)
	{
		StringUtil::StrStreamType dbgMsg;
		dbgMsg << "thread:" <<
#if OGRE_THREAD_SUPPORT
			OGRE_THREAD_CURRENT_ID
#else
			"main"
#endif
			<< "): ID=" << r->getRequest()->getID()
			<< " success=" << r->succeeded() << " messages=[" << r->getMessages() << "] channel=" 
			<< r->getRequest()->getChannel() << " requestType=" << r->getRequest()->getType();

		LogManager::getSingleton().stream(LML_TRIVIAL) << 
			"DefaultWorkQueueBase('" << mName << "') - PROCESS_RESPONSE_START(" << dbgMsg.str();

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
			"DefaultWorkQueueBase('" << mName << "') - PROCESS_RESPONSE_END(" << dbgMsg.str();

	}
	//---------------------------------------------------------------------

	void DefaultWorkQueueBase::WorkerFunc::operator()()
	{
		mQueue->_threadMain();
	}

	void DefaultWorkQueueBase::WorkerFunc::run()
	{
		mQueue->_threadMain();
	}
}
