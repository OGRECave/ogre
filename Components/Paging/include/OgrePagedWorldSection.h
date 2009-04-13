/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
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

#ifndef __Ogre_PagedWorldSection_H__
#define __Ogre_PagedWorldSection_H__

#include "OgrePagingPrerequisites.h"
#include "OgreAxisAlignedBox.h"

namespace Ogre
{
	/** \addtogroup Optional Components
	*  @{
	*/
	/** \addtogroup Paging
	*  Some details on paging component
	*  @{
	*/

	/** Represents a section of the PagedWorld which uses a given PageStrategy, and
		which is made up of a generally localised set of Page instances.
	@remarks
		The reason for PagedWorldSection is that you may wish to cater for multiple
		sections of your world which use a different approach to paging (ie a
		different PageStrategy), or which are significantly far apart or separate 
		that the parameters you want to pass to the PageStrategy are different.
	@par
		PagedWorldSection instances are fully contained within the PagedWorld and
		their definitions are loaded in their entirety when the PagedWorld is
		loaded. However, no Page instances are initially loaded - those are the
		responsibility of the PageStrategy.
	*/
	class PagedWorldSection : public PageAlloc
	{
	public:
		typedef map<PageID, Page*>::type PageMap;
	protected:
		String mName;
		AxisAlignedBox mAABB;
		PagedWorld* mParent;
		PageStrategy* mStrategy;
		PageStrategyData* mStrategyData;
		PageMap mPages;
		PageProvider* mPageProvider;
		SceneManager* mSceneMgr;


	public:
		static const uint32 CHUNK_ID;
		static const uint16 CHUNK_VERSION;

		/** Construct a new instance, specifying just the parent (expecting to load). */
		PagedWorldSection(PagedWorld* parent); 

		/** Construct a new instance, specifying the parent and assigned strategy. */
		PagedWorldSection(const String& name, PagedWorld* parent, PageStrategy* strategy, 
			SceneManager* sm);
		virtual ~PagedWorldSection();

		/// Get the name of this section
		virtual const String& getName() const { return mName; }
		/// Get the page strategy which this section is using
		virtual PageStrategy* getStrategy() const { return mStrategy; }
		/** Change the page strategy.
		@remarks
			Doing this will invalidate any pages attached to this world section, and
			require the PageStrategyData to be repopulated.
		*/
		virtual void setStrategy(PageStrategy* strat);
		/** Change the page strategy.
		@remarks
			Doing this will invalidate any pages attached to this world section, and
			require the PageStrategyData to be repopulated.
		*/
		virtual void setStrategy(const String& stratName);

		/** Change the SceneManager.
		@remarks
		Doing this will invalidate any pages attached to this world section, and
		require the pages to be reloaded.
		*/
		virtual void setSceneManager(SceneManager* sm);
		
		/** Change the SceneManager.
		@remarks
		Doing this will invalidate any pages attached to this world section, and
		require the pages to be reloaded.
		@param smName The instance name of the SceneManager
		*/
		virtual void setSceneManager(const String& smName);
		/// Get the current SceneManager
		virtual SceneManager* getSceneManager() const { return mSceneMgr; }

		/// Get the parent world
		virtual PagedWorld* getWorld() const { return mParent; }
		/// Get the data required by the PageStrategy which is specific to this world section
		virtual PageStrategyData* getStrategyData() const { return mStrategyData; }
		/// Set the bounds of this section
		virtual void setBoundingBox(const AxisAlignedBox& box);
		/// Get the bounds of this section
		virtual const AxisAlignedBox& getBoundingBox() const;


		/// Load this section from a stream (returns true if successful)
		virtual bool load(StreamSerialiser& stream);
		/// Save this section to a stream
		virtual void save(StreamSerialiser& stream);


		/// Called when the frame starts
		virtual void frameStart(Real timeSinceLastFrame);
		/// Called when the frame ends
		virtual void frameEnd(Real timeElapsed);
		/// Notify a section of the current camera
		virtual void notifyCamera(Camera* cam);


		/** Ask for a page to be loaded with the given (section-relative) PageID
		@remarks
			You would not normally call this manually, the PageStrategy is in 
			charge of it usually.
			If this page is already loaded, this request will not load it again.
			If the page needs loading, then it may be an asynchronous process depending
			on whether threading is enabled.
		*/
		virtual void loadPage(PageID pageID);

		/** Ask for a page to be unloaded with the given (section-relative) PageID
		@remarks
			You would not normally call this manually, the PageStrategy is in 
			charge of it usually.
		*/
		virtual void unloadPage(PageID pageID);
		/** Ask for a page to be unloaded with the given (section-relative) PageID
		@remarks
		You would not normally call this manually, the PageStrategy is in 
		charge of it usually.
		*/
		virtual void unloadPage(Page* p);
		/** Give a section the opportunity to prepare page content procedurally. 
		@remarks
		You should not call this method directly. This call may well happen in 
		a separate thread so it should not access GPU resources, use _loadProceduralPage
		for that
		@returns true if the page was populated, false otherwise
		*/
		virtual bool _prepareProceduralPage(Page* page);
		/** Give a section the opportunity to prepare page content procedurally. 
		@remarks
		You should not call this method directly. This call will happen in 
		the main render thread so it can access GPU resources. Use _prepareProceduralPage
		for background preparation.
		@returns true if the page was populated, false otherwise
		*/
		virtual bool _loadProceduralPage(Page* page);
		/** Give a section  the opportunity to unload page content procedurally. 
		@remarks
		You should not call this method directly. This call will happen in 
		the main render thread so it can access GPU resources. Use _unprepareProceduralPage
		for background preparation.
		@returns true if the page was populated, false otherwise
		*/
		virtual bool _unloadProceduralPage(Page* page);
		/** Give a section  the opportunity to unprepare page content procedurally. 
		@remarks
		You should not call this method directly. This call may well happen in 
		a separate thread so it should not access GPU resources, use _unloadProceduralPage
		for that
		@returns true if the page was unpopulated, false otherwise
		*/
		virtual bool _unprepareProceduralPage(Page* page);

		/** Ask for a page to be kept in memory if it's loaded.
		@remarks
			This method indicates that a page should be retained if it's already
			in memory, but if it's not then it won't trigger a load. This is useful
			for retaining pages that have just gone out of range, but which you
			don't want to unload just yet because it's quite possible they may come
			back into the active set again very quickly / easily. But at the same
			time, if they've already been purged you don't want to force them to load. 
			This is the 'maybe' region of pages. 
		@par
			Any Page that is neither requested nor held in a frame will be
			deemed a candidate for unloading.
		*/
		virtual void holdPage(PageID pageID);

		/** Retrieves a Page.
		@remarks
			This method will only return Page instances that are already loaded. It
			will return null if a page is not loaded. 
		*/
		virtual Page* getPage(PageID pageID);

		/** Attach a page to this section. 
		@remarks
			This method is usually called by the loading routine and not directly. 
			This class becomes responsible for deleting the Page.
		*/
		virtual void attachPage(Page* page);

		/** Detach a page to this section. 
		@remarks
		This method is usually called by the unloading routine and not directly. 
		This class is no longer responsible for deleting the Page.
		*/
		virtual void detachPage(Page* page);

		/** Remove all pages immediately. 
		@remarks
			Effectively 'resets' this section by deleting all pages. 
		*/
		virtual void removeAllPages();

		/** Set the PageProvider which can provide streams Pages in this section. 
		@remarks
			This is the top-level way that you can direct how Page data is loaded. 
			When data for a Page is requested for a PagedWorldSection, the following
			sequence of classes will be checked to see if they have a provider willing
			to supply the stream: PagedWorldSection, PagedWorld, PageManager.
			If none of these do, then the default behaviour is to look for a file
			called worldname_sectionname_pageID.page. 
		@note
			The caller remains responsible for the destruction of the provider.
		*/
		void setPageProvider(PageProvider* provider) { mPageProvider = provider; }
		
		/** Get the PageProvider which can provide streams for Pages in this section. */
		PageProvider* getPageProvider() const { return mPageProvider; }

		/** Get a serialiser set up to read Page data for the given PageID. 
		@param pageID The ID of the page being requested
		@remarks
		The StreamSerialiser returned is the responsibility of the caller to
		delete. 
		*/
		StreamSerialiser* _readPageStream(PageID pageID);

		/** Get a serialiser set up to write Page data for the given PageID. 
		@param pageID The ID of the page being requested
		@remarks
		The StreamSerialiser returned is the responsibility of the caller to
		delete. 
		*/
		StreamSerialiser* _writePageStream(PageID pageID);

		/** Function for writing to a stream.
		*/
		_OgrePagingExport friend std::ostream& operator <<( std::ostream& o, const PagedWorldSection& p );

	};

	/** @} */
	/** @} */
}




#endif 