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
#include "OgrePageLoadableUnit.h"


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
	class Page : public PageLoadableUnit
	{
	public:
		typedef vector<PageContentCollection*>::type ContentCollectionList;
	protected:
		PageID mID;
		PagedWorldSection* mParent;
		unsigned long mFrameLastHeld;
		ContentCollectionList mContentCollections;

		SceneNode* mDebugNode;
		void updateDebugDisplay();

		bool prepareImpl(StreamSerialiser& stream);
		void loadImpl();
		void unprepareImpl();
		void unloadImpl();

	public:
		static const uint32 CHUNK_ID;
		static const uint16 CHUNK_VERSION;

		Page(PageID pageID);
		virtual ~Page();

		PageManager* getManager() const;
		SceneManager* getSceneManager() const;

		/// Get the ID of this page, unique withing the parent
		virtual PageID getID() const { return mID; }
		/// Get the PagedWorldSection this page belongs to, or zero if unattached
		virtual PagedWorldSection* getParentSection() const { return mParent; }
		/** Get the frame number in which this Page was last loaded or held.
		@remarks
			A Page that has not been requested to be loaded or held in the recent
			past will be a candidate for removal.
		*/
		virtual unsigned long getFrameLastHeld() { return mFrameLastHeld; }
		/// 'Touch' the page to let it know it's being used
		virtual void touch();
		/// Get whether or not this page is currently attached 
		virtual bool isAttached() const { return mParent != 0; }


		/** Returns whether this page was 'held' in the last frame, that is
			was it either directly needed, or requested to stay in memory (held - as
			in a buffer region for example). If not, this page is eligible for 
			removal.
		*/
		virtual bool isHeld() const;

		/// Save page data to a serialiser 
		virtual void save(StreamSerialiser& stream);

		/// Internal method to notify a page that it is attached
		virtual void _notifyAttached(PagedWorldSection* parent);

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
		/** Add a content collection to this Page. 
		@remarks
			This class now becomes responsible for deleting this collection
		*/
		virtual void attachContentCollection(PageContentCollection* coll);
		/** Remove a content collection from this Page. 
		@remarks
			This class ceases to be responsible for deleting this collection.
		*/
		virtual void detachContentCollection(PageContentCollection* coll);
		/// Get the number of content collections
		virtual size_t getContentCollectionCount() const;
		/// Get a content collection
		virtual PageContentCollection* getContentCollection(size_t index);
		/// Get the list of content collections
		const ContentCollectionList& getContentCollectionList() const;



		/** Function for writing to a stream.
		*/
		_OgrePagingExport friend std::ostream& operator <<( std::ostream& o, const Page& p );


	};

	/** @} */
	/** @} */
}

#endif
