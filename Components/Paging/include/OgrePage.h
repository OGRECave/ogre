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

#ifndef __Ogre_Page_H__
#define __Ogre_Page_H__

#include "OgrePagingPrerequisites.h"
#include "OgreWorkQueue.h"


namespace Ogre
{
	/** \addtogroup Optional Components
	*  @{
	*/
	/** \addtogroup Paging
	*  Some details on paging component
	*  @{
	*/

	/** Page class
	*/
	class _OgrePagingExport Page : public WorkQueue::RequestHandler, 
		public WorkQueue::ResponseHandler, public PageAlloc
	{
	public:
		typedef vector<PageContentCollection*>::type ContentCollectionList;
	protected:
		PageID mID;
		PagedWorldSection* mParent;
		unsigned long mFrameLastHeld;
		ContentCollectionList mContentCollections;
		uint16 mWorkQueueChannel;
		bool mDeferredProcessInProgress;
		bool mModified;

		SceneNode* mDebugNode;
		void updateDebugDisplay();

		struct PageData : public PageAlloc
		{
			ContentCollectionList collectionsToAdd;
		};
		/// Structure for holding background page requests
		struct PageRequest
		{
			Page* srcPage;
			_OgrePagingExport friend std::ostream& operator<<(std::ostream& o, const PageRequest& r)
			{ return o; }		

			PageRequest(Page* p): srcPage(p) {}
		};
		struct PageResponse
		{
			PageData* pageData;

			_OgrePagingExport friend std::ostream& operator<<(std::ostream& o, const PageResponse& r)
			{ return o; }		

			PageResponse() : pageData(0) {}
		};



		virtual bool prepareImpl(PageData* dataToPopulate);
		virtual bool prepareImpl(StreamSerialiser& str, PageData* dataToPopulate);
		virtual void loadImpl();

		String generateFilename() const;

	public:
		static const uint32 CHUNK_ID;
		static const uint16 CHUNK_VERSION;

		static const uint32 CHUNK_CONTENTCOLLECTION_DECLARATION_ID;

		Page(PageID pageID, PagedWorldSection* parent);
		virtual ~Page();

		PageManager* getManager() const;
		SceneManager* getSceneManager() const;

		/// If true, it's not safe to access this Page at this time, contents may be changing
		bool isDeferredProcessInProgress() const { return mDeferredProcessInProgress; }

		/// Get the ID of this page, unique withing the parent
		virtual PageID getID() const { return mID; }
		/// Get the PagedWorldSection this page belongs to
		virtual PagedWorldSection* getParentSection() const { return mParent; }
		/** Get the frame number in which this Page was last loaded or held.
		@remarks
			A Page that has not been requested to be loaded or held in the recent
			past will be a candidate for removal.
		*/
		virtual unsigned long getFrameLastHeld() { return mFrameLastHeld; }
		/// 'Touch' the page to let it know it's being used
		virtual void touch();

		/** Load this page. 
		@param synchronous Whether to force this to happen synchronously.
		*/
		virtual void load(bool synchronous);
		/** Unload this page. 
		*/
		virtual void unload();


		/** Returns whether this page was 'held' in the last frame, that is
			was it either directly needed, or requested to stay in memory (held - as
			in a buffer region for example). If not, this page is eligible for 
			removal.
		*/
		virtual bool isHeld() const;

		/// Save page data to an automatically generated file name
		virtual void save();
		/// Save page data to a file
		virtual void save(const String& filename);
		/// Save page data to a serialiser 
		virtual void save(StreamSerialiser& stream);

		/// Called when the frame starts
		virtual void frameStart(Real timeSinceLastFrame);
		/// Called when the frame ends
		virtual void frameEnd(Real timeElapsed);
		/// Notify a section of the current camera
		virtual void notifyCamera(Camera* cam);

		/** Create a new PageContentCollection within this page.
		This is equivalent to calling PageManager::createContentCollection and 
		then attachContentCollection.
		@param typeName The name of the type of content collection (see PageManager::getContentCollectionFactories)
		*/
		virtual PageContentCollection* createContentCollection(const String& typeName);

		/** Destroy a PageContentCollection within this page.
		This is equivalent to calling detachContentCollection and 
			PageManager::destroyContentCollection.
		*/
		virtual void destroyContentCollection(PageContentCollection* coll);
		/** Destroy all PageContentCollections within this page.
		*/
		virtual void destroyAllContentCollections();
		/// Get the number of content collections
		virtual size_t getContentCollectionCount() const;
		/// Get a content collection
		virtual PageContentCollection* getContentCollection(size_t index);
		/// Get the list of content collections
		const ContentCollectionList& getContentCollectionList() const;

		/// WorkQueue::RequestHandler override
		bool canHandleRequest(const WorkQueue::Request* req, const WorkQueue* srcQ);
		/// WorkQueue::RequestHandler override
		WorkQueue::Response* handleRequest(const WorkQueue::Request* req, const WorkQueue* srcQ);
		/// WorkQueue::ResponseHandler override
		bool canHandleResponse(const WorkQueue::Response* res, const WorkQueue* srcQ);
		/// WorkQueue::ResponseHandler override
		void handleResponse(const WorkQueue::Response* res, const WorkQueue* srcQ);


		/// Tell the page that it is modified
		void _notifyModified() { mModified = true; }
		bool isModified() const { return mModified; }

		static const uint16 WORKQUEUE_PREPARE_REQUEST;
		static const uint16 WORKQUEUE_CHANGECOLLECTION_REQUEST;

		/** Function for writing to a stream.
		*/
		_OgrePagingExport friend std::ostream& operator <<( std::ostream& o, const Page& p );



	};

	/** @} */
	/** @} */
}

#endif
