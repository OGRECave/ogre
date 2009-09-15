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

#ifndef __Ogre_PageRequestQueue_H__
#define __Ogre_PageRequestQueue_H__

#include "OgrePagingPrerequisites.h"
#include "OgreString.h"
#include "OgreCommon.h"

namespace Ogre
{
	/** \addtogroup Optional Components
	*  @{
	*/
	/** \addtogroup Paging
	*  Some details on paging component
	*  @{
	*/

	/** The PageRequestQueue is where pages are queued for loading and freeing.
	*/
	class _OgrePagingExport PageRequestQueue : public PageAlloc
	{
	public:
		PageRequestQueue(PageManager* manager);
		virtual ~PageRequestQueue();

		/** Load a Page, for a given PagedWorldSection.
		@remarks
			This method is called from the main rendering thread in all cases. 
			If threading is enabled, the request is queued and processed by 
			a separate thread. At some point later on the loaded page will be attached
			to the section which requested it. 
		*/
		void loadPage(Page* page, PagedWorldSection* section, bool forceSync = false);
		/** Dispose of a page */
		void unloadPage(Page* page, PagedWorldSection* section, bool forceSync = false);

		/** Cancel any pending operations for a Page.
		@remarks
			Any queued operations for a given page will be removed. If an operation
			is in progress, it will be allowed to finish before this method
			returns.
		*/
		void cancelOperationsForPage(Page* p);

		/** Enable this option if you want to force synchronous loading of all 
			future requests.
		*/
		void setForceSynchronous(bool force) { mForceSynchronous = force; }
		/** Gets whether requests will be dealt with synchronously even if a thread
			is available. 
		*/
		bool getForceSynchronous() const { return mForceSynchronous; }

		/** Set the amount of time the render thread is allowed to spend on pending requests.
		@remarks
			Even though much work is done in a background thread, the main render 
			thread is still required to manage GPU resources. Therefore each frame
			pending requests of this type will be dealt with in the main thread too.
			This option allows you to specify how long the main thread is allowed
			to spend each frame on this task, and if any tasks are left over after
			this time then they are deferred to the next frame. Note that at
			least one task is always performed. 
		@param milliseconds The amount of time budgeted in milliseconds, default is
			0 (no limit)
		*/
		void setRenderThreadFrameTimeBudget(unsigned long milliseconds)
		{ mRenderThreadTimeLimit = milliseconds; }

		unsigned long getRenderThreadFrameTimeBudget() const { return mRenderThreadTimeLimit; }
		/// To be called in the main render thread each frame
		void processRenderThreadRequests();


	protected:
		PageManager* mManager;
		bool mForceSynchronous;
		unsigned long mRenderThreadTimeLimit;

		/// The request type for a request - listed in general lifecycle order
		enum RequestType
		{
			PREPARE_PAGE = 0, 
			LOAD_PAGE = 1,
			UNLOAD_PAGE = 2, 
			UNPREPARE_PAGE = 3,
			DELETE_PAGE = 4
		};

		struct _OgrePagingExport Request
		{
			RequestType requestType;
			Page* page;
			PagedWorldSection* section;

			Request(RequestType rt, Page* p, PagedWorldSection* s) 
				: requestType(rt), page(p), section(s) {}
		};

		typedef deque<Request>::type RequestQueue;
		/// Requests pending for the background queue
		RequestQueue mBackgroundQueue;
		/// Requests pending for the render queue (follow on from background)
		RequestQueue mRenderQueue;

		void addBackgroundRequest(const Request& r, bool forceSync = false);
		void addRenderRequest(const Request& r, bool forceSync = false);
		
		/// Process the background portion of a request (may be threaded)
		void processBackgroundRequest(const Request& r);
		/// Process the render portion of a request (may be threaded)
		void processRenderRequest(const Request& r);
	};

	/** @} */
	/** @} */
}

#endif
