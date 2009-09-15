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
#include "OgrePageRequestQueue.h"
#include "OgreStringConverter.h"
#include "OgreException.h"
#include "OgrePagedWorldSection.h"
#include "OgrePage.h"
#include "OgreStreamSerialiser.h"
#include "OgreLogManager.h"
#include "OgreRoot.h"
#include "OgreTimer.h"
#include "OgrePagedWorld.h"

namespace Ogre
{
	//---------------------------------------------------------------------
	PageRequestQueue::PageRequestQueue(PageManager* manager)
		: mManager(manager)
		// TODO - don't force sync (threading primitives still TBA)
		, mForceSynchronous(true)
		, mRenderThreadTimeLimit(0)
	{
	}
	//---------------------------------------------------------------------
	PageRequestQueue::~PageRequestQueue()
	{
	}
	//---------------------------------------------------------------------
	void PageRequestQueue::loadPage(Page* page, PagedWorldSection* section, bool forceSync)
	{
		
		// Prepare in the background
		Request req(PREPARE_PAGE, page, section);
		addBackgroundRequest(req, forceSync);

		// load will happen in the main thread once preparation is complete


	}
	//---------------------------------------------------------------------
	void PageRequestQueue::unloadPage(Page* page, PagedWorldSection* section, bool forceSync)
	{
		// unload in main thread, then unprepare in background
		Request req(UNLOAD_PAGE, page, section);
		addRenderRequest(req, forceSync);

	}
	//---------------------------------------------------------------------
	void PageRequestQueue::cancelOperationsForPage(Page* p)
	{
		// cancel background
		{
			// TODO lock background queue
			for(RequestQueue::iterator i = mBackgroundQueue.begin(); i != mBackgroundQueue.end(); )
			{
				if (i->page == p)
					i = mBackgroundQueue.erase(i);
				else
					++i;
			}

		}

		// cancel render
		{
			// TODO lock render queue
			for(RequestQueue::iterator i = mRenderQueue.begin(); i != mRenderQueue.end(); )
			{
				if (i->page == p)
					i = mRenderQueue.erase(i);
				else
					++i;
			}
		}

	}
	//---------------------------------------------------------------------
	void PageRequestQueue::addBackgroundRequest(const Request& r, bool forceSync)
	{
		Log* log = LogManager::getSingleton().getDefaultLog();
		if (log->getLogDetail() == LL_BOREME)
		{
			LogManager::getSingleton().stream(LML_TRIVIAL)
				<< "PageRequestQueue: queueing background thread request " << r.requestType << 
				" for page ID " << r.page->getID() << " world " << r.section->getWorld()->getName()
				<< ":" << r.section->getName();
		}

		if (mForceSynchronous || forceSync)
		{
			processBackgroundRequest(r);
		}
		else
		{
			// TODO lock
			mBackgroundQueue.push_back(r);
		}

	}
	//---------------------------------------------------------------------
	void PageRequestQueue::addRenderRequest(const Request& r, bool forceSync)
	{
		Log* log = LogManager::getSingleton().getDefaultLog();
		if (log->getLogDetail() == LL_BOREME)
		{
			LogManager::getSingleton().stream(LML_TRIVIAL)
				<< "PageRequestQueue: queueing render thread request " << r.requestType << 
				" for page ID " << r.page->getID() << " world " << r.section->getWorld()->getName()
				<< ":" << r.section->getName();
		}
		if (mForceSynchronous || forceSync)
		{
			processRenderRequest(r);
		}
		else
		{
			// TODO lock
			mRenderQueue.push_back(r);
		}

	}
	//---------------------------------------------------------------------
	void PageRequestQueue::processBackgroundRequest(const Request& r)
	{
		Log* log = LogManager::getSingleton().getDefaultLog();
		if (log->getLogDetail() == LL_BOREME)
		{
			LogManager::getSingleton().stream(LML_TRIVIAL)
				<< "PageRequestQueue: processing background thread request " << r.requestType << 
				" for page ID " << r.page->getID() << " world " << r.section->getWorld()->getName()
				<< ":" << r.section->getName();
		}
		try
		{
			switch(r.requestType)
			{
			case PREPARE_PAGE:
				{
					// Allow procedural generation
					if (r.section->_prepareProceduralPage(r.page))
					{
						r.page->_changeStatus(PageLoadableUnit::STATUS_UNLOADED, PageLoadableUnit::STATUS_PREPARED);
					}
					else
					{
						StreamSerialiser* ser = r.section->_readPageStream(r.page->getID());
						r.page->prepare(*ser);
						OGRE_DELETE ser;
					}


					// Pass back to render thread to finalise
					Request newreq(LOAD_PAGE, r.page, r.section);
					addRenderRequest(newreq);
				}

				break;
			case UNPREPARE_PAGE:
				{

					// Allow procedural generation
					if (r.section->_unprepareProceduralPage(r.page))
					{
						r.page->_changeStatus(PageLoadableUnit::STATUS_PREPARED, PageLoadableUnit::STATUS_UNLOADED);
					}
					else
					{
						r.page->unprepare();
					}

					// Pass back to render thread to finalise
					Request newreq(DELETE_PAGE, r.page, r.section);
					addRenderRequest(newreq);
				}

				break;
			default:
				// not a background request
				;
			};
		}
		catch (Exception& e)
		{
			LogManager::getSingleton().stream() << "Error processing background request: "
				<< e.getFullDescription();
		}

	}
	//---------------------------------------------------------------------
	void PageRequestQueue::processRenderRequest(const Request& r)
	{
		Log* log = LogManager::getSingleton().getDefaultLog();
		if (log->getLogDetail() == LL_BOREME)
		{
			LogManager::getSingleton().stream(LML_TRIVIAL)
				<< "PageRequestQueue: processing render thread request " << r.requestType << 
				" for page ID " << r.page->getID() << " world " << r.section->getWorld()->getName()
				<< ":" << r.section->getName();
		}
		try
		{
			switch(r.requestType)
			{
			case LOAD_PAGE:
				// Allow procedural generation
				if (r.section->_loadProceduralPage(r.page))
				{		
					r.page->_changeStatus(PageLoadableUnit::STATUS_PREPARED, PageLoadableUnit::STATUS_LOADED);
				}
				else
				{
					r.page->load();
				}
				

				break;
			case UNLOAD_PAGE:
				{
					// Allow procedural unload
					if (r.section->_unloadProceduralPage(r.page))
					{		
						r.page->_changeStatus(PageLoadableUnit::STATUS_LOADED, PageLoadableUnit::STATUS_PREPARED);
					}
					else
					{
						r.page->unload();
					}

					// Pass back to background thread to finalise
					Request newreq(UNPREPARE_PAGE, r.page, r.section);
					addBackgroundRequest(newreq);
				}

				break;
			case DELETE_PAGE:
				OGRE_DELETE r.page;
				break;
			default:
				// not a render request
				;
			};
		}
		catch (Exception& e)
		{
			LogManager::getSingleton().stream() << "Error processing render request: "
				<< e.getFullDescription();
		}


	}
	//---------------------------------------------------------------------
	void PageRequestQueue::processRenderThreadRequests()
	{
		Timer* timer = Root::getSingleton().getTimer();
		unsigned long msStart = timer->getMilliseconds();

		while(!mRenderQueue.empty())
		{
			// FIFO
			// TODO lock
			Request r = mRenderQueue.front();
			mRenderQueue.pop_front();

			processRenderRequest(r);

			if (mRenderThreadTimeLimit && 
				(msStart + mRenderThreadTimeLimit) <= timer->getMilliseconds())
			{
				// time up!
				break;
			}
		}
	}


}

