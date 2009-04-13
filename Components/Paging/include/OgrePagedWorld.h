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

#ifndef __Ogre_PagedWorld_H__
#define __Ogre_PagedWorld_H__

#include "OgrePagingPrerequisites.h"
#include "OgreString.h"
#include "OgreDataStream.h"
#include "OgreCommon.h"

namespace Ogre
{
	class PageManager;

	/** \addtogroup Optional Components
	*  @{
	*/
	/** \addtogroup Paging
	*  Some details on paging component
	*  @{
	*/


	/** This class represents a collection of pages which make up a world. 
	@remarks
		It's important to bear in mind that the PagedWorld only delineates the
		world and knows how to find out about the contents of it. It does not, 
		by design, contain all elements of the world, in memory, at once. 
	*/
	class _OgrePagingExport PagedWorld : public PageAlloc
	{
	protected:
		String mName;
		PageManager* mManager;
		PageProvider* mPageProvider;

	public:
		static const uint32 CHUNK_ID;
		static const uint16 CHUNK_VERSION;
		/** Constructor.
		@param name The name of the world, which must be enough to identify the 
			place where data for it can be loaded from (doesn't have to be a filename
			necessarily).
		@param manager The PageManager that is in charge of providing this world with
			services such as related object factories.
		*/
		PagedWorld(const String& name, PageManager* manager);
		virtual ~PagedWorld();

		const String& getName() const { return mName; }
		/// Get the manager of this world
		PageManager* getManager() const { return mManager; }

		/// Load world data from a file
		void load(const String& filename);
		/// Load world data from a stream
		void load(const DataStreamPtr& stream);
		/// Load world data from a serialiser (returns true if successful)
		bool load(StreamSerialiser& stream);
		/** Save world data to a file
		@param filename The name of the file to create; this can either be an 
			absolute filename or 
		*/
		void save(const String& filename);
		/// Save world data to a stream
		void save(const DataStreamPtr& stream);
		/// Save world data to a serialiser
		void save(StreamSerialiser& stream);

		/** Create a new section of the world.
		@remarks
			World sections are areas of the world that use a particular
			PageStrategy, with a certain set of parameters specific to that
			strategy. So you would have more than one section in a world only 
			if you needed different simultaneous paging strategies, or you 
			wanted the same strategy but parameterised differently.
		@param strategyName The name of the strategy to use (must be registered	
			with PageManager)
		@param sceneMgr The SceneManager to use for this section
		@param sectionName An optional name to give the section (if none is
			provided, one will be generated)
		*/
		PagedWorldSection* createSection(const String& strategyName, SceneManager* sceneMgr,
			const String& sectionName = StringUtil::BLANK);


		/** Create a new section of the world.
		@remarks
			World sections are areas of the world that use a particular
			PageStrategy, with a certain set of parameters specific to that
			strategy. So you would have more than one section in a world only 
			if you needed different simultaneous paging strategies, or you 
			wanted the same strategy but parameterised differently.
		@param strategy The strategy to use 
		@param sceneMgr The SceneManager to use for this section
		@param sectionName An optional name to give the section (if none is
			provided, one will be generated)
		*/
		PagedWorldSection* createSection(PageStrategy* strategy, SceneManager* sceneMgr, 
			const String& sectionName = StringUtil::BLANK);

		/** Destroy a section of world. */
		void destroySection(const String& name);
		/** Destroy a section of world. */
		void destroySection(PagedWorldSection* sec);

		/** Get the number of sections this world has. */
		size_t getSectionCount() const { return mSections.size(); }

		/** Retrieve a section of the world. */
		PagedWorldSection* getSection(const String& name);

		typedef map<String, PagedWorldSection*>::type SectionMap;
		/// Retrieve a const reference to all the sections in this world
		const SectionMap& getSections() const { return mSections; }

		/** Set the PageProvider which can provide streams for Pages in this world. 
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
		
		/** Get the PageProvider which can provide streams for Pages in this world. */
		PageProvider* getPageProvider() const { return mPageProvider; }

		/** Give a world  the opportunity to prepare page content procedurally. 
		@remarks
		You should not call this method directly. This call may well happen in 
		a separate thread so it should not access GPU resources, use _loadProceduralPage
		for that
		@returns true if the page was populated, false otherwise
		*/
		virtual bool _prepareProceduralPage(Page* page, PagedWorldSection* section);
		/** Give a world  the opportunity to prepare page content procedurally. 
		@remarks
		You should not call this method directly. This call will happen in 
		the main render thread so it can access GPU resources. Use _prepareProceduralPage
		for background preparation.
		@returns true if the page was populated, false otherwise
		*/
		virtual bool _loadProceduralPage(Page* page, PagedWorldSection* section);
		/** Give a world  the opportunity to unload page content procedurally. 
		@remarks
		You should not call this method directly. This call will happen in 
		the main render thread so it can access GPU resources. Use _unprepareProceduralPage
		for background preparation.
		@returns true if the page was populated, false otherwise
		*/
		virtual bool _unloadProceduralPage(Page* page, PagedWorldSection* section);
		/** Give a world  the opportunity to unprepare page content procedurally. 
		@remarks
		You should not call this method directly. This call may well happen in 
		a separate thread so it should not access GPU resources, use _unloadProceduralPage
		for that
		@returns true if the page was unpopulated, false otherwise
		*/
		virtual bool _unprepareProceduralPage(Page* page, PagedWorldSection* section);
		/** Get a serialiser set up to read Page data for the given PageID. 
		@param pageID The ID of the page being requested
		@param section The parent section to which this page will belong
		@remarks
		The StreamSerialiser returned is the responsibility of the caller to
		delete. 
		*/
		StreamSerialiser* _readPageStream(PageID pageID, PagedWorldSection* section);

		/** Get a serialiser set up to read Page data for the given PageID. 
		@param pageID The ID of the page being requested
		@param section The parent section to which this page will belong
		@remarks
		The StreamSerialiser returned is the responsibility of the caller to
		delete. 
		*/
		StreamSerialiser* _writePageStream(PageID pageID, PagedWorldSection* section);

		/// Called when the frame starts
		virtual void frameStart(Real timeSinceLastFrame);
		/// Called when the frame ends
		virtual void frameEnd(Real timeElapsed);
		/// Notify a world of the current camera
		virtual void notifyCamera(Camera* cam);

		/** Function for writing to a stream.
		*/
		_OgrePagingExport friend std::ostream& operator <<( std::ostream& o, const PagedWorld& p );


	protected:
		SectionMap mSections;
		NameGenerator mSectionNameGenerator;




	};
	
	/** @} */
	/** @} */

}




#endif 