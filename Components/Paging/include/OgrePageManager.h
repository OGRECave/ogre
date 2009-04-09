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

#ifndef __Ogre_PageManager_H__
#define __Ogre_PageManager_H__

#include "OgrePagingPrerequisites.h"
#include "OgreString.h"
#include "OgreResourceGroupManager.h"
#include "OgreCommon.h"
#include "OgreCamera.h"
#include "OgreFrameListener.h"

namespace Ogre
{
	/** \addtogroup Optional Components
	*  @{
	*/
	/** \addtogroup Paging
	*  Some details on paging component
	*  @{
	*/

	/** Abstract class that can be implemented by the user application to 
		provide a way to retrieve or generate page data from a source of their choosing.
	@note
		All of the methods in this class can be called in a background, non-render thread. 		
	*/
	class PageProvider
	{
	public:
		PageProvider() {}
		virtual ~PageProvider() {}

		/** Give a provider the opportunity to prepare page content procedurally. 
		@remarks
		This call may well happen in a separate thread so it should not access 
		GPU resources, use loadProceduralPage for that
		@returns true if the page was populated, false otherwise
		*/
		virtual bool prepareProceduralPage(Page* page, PagedWorldSection* section) { return false; }
		/** Give a provider the opportunity to load page content procedurally. 
		@remarks
		This call will happen in the main render thread so it can access GPU resources. 
		Use prepareProceduralPage for background preparation.
		@returns true if the page was populated, false otherwise
		*/
		virtual bool loadProceduralPage(Page* page, PagedWorldSection* section) { return false; }
		/** Give a provider the opportunity to unload page content procedurally. 
		@remarks
		You should not call this method directly. This call will happen in 
		the main render thread so it can access GPU resources. Use _unprepareProceduralPage
		for background preparation.
		@returns true if the page was populated, false otherwise
		*/
		virtual bool unloadProceduralPage(Page* page, PagedWorldSection* section) { return false; }
		/** Give a provider the opportunity to unprepare page content procedurally. 
		@remarks
		You should not call this method directly. This call may well happen in 
		a separate thread so it should not access GPU resources, use _unloadProceduralPage
		for that
		@returns true if the page was unpopulated, false otherwise
		*/
		virtual bool unprepareProceduralPage(Page* page, PagedWorldSection* section) { return false; }

		/** Get a serialiser set up to read PagedWorld data for the given world filename. 
		@remarks
		The StreamSerialiser returned is the responsibility of the caller to
		delete. 
		*/
		virtual StreamSerialiser* readWorldStream(const String& filename) { return 0; }
		/** Get a serialiser set up to write PagedWorld data for the given world filename. 
		@remarks
		The StreamSerialiser returned is the responsibility of the caller to
		delete. 
		*/
		virtual StreamSerialiser* writeWorldStream(const String& filename) { return 0; }
		/** Get a serialiser set up to read Page data for the given PageID, 
			or null if this provider cannot supply one. 
		@remarks
			The StreamSerialiser returned is the responsibility of the caller to
			delete. 
		@param pageID The ID of the page being requested
		@param section The parent section to which this page will belong
		*/
		virtual StreamSerialiser* readPageStream(PageID pageID, PagedWorldSection* section) { return 0; }

		/** Get a serialiser set up to write Page data for the given PageID, 
		or null if this provider cannot supply one. 
		@remarks
		The StreamSerialiser returned is the responsibility of the caller to
		delete. 
		@param pageID The ID of the page being requested
		@param section The parent section to which this page will belong
		*/
		virtual StreamSerialiser* writePageStream(PageID pageID, PagedWorldSection* section) { return 0; }
	};
	
	/** The PageManager is the entry point through which you load all PagedWorld instances, 
		and the place where PageStrategy instances and factory classes are
		registered to customise the paging behaviour.
	@remarks
		To get started, the minimum you need is a PagedWorld with at least one PagedWorldSection
		within it, and at least one Camera being tracked (see addCamera). 
	*/
	class _OgrePagingExport PageManager : public PageAlloc
	{
	public:
		PageManager();
		virtual ~PageManager();

		/** Create a new PagedWorld instance. 
		@param name Optionally give a name to the world (if no name is given, one
			will be generated).
		*/
		PagedWorld* createWorld(const String& name = StringUtil::BLANK);

		/** Destroy a world. */
		void destroyWorld(const String& name);

		/** Destroy a world. */
		void destroyWorld(PagedWorld* world);

		/** Attach a pre-created PagedWorld instance to this manager. 
		*/
		void attachWorld(PagedWorld* world);
		/** Detach a PagedWorld instance from this manager (note: the caller then
			becomes responsible for the correct destruction of this instance)
		*/
		void detachWorld(PagedWorld* world);
		/** Load a new PagedWorld from a file. 
		@param filename The name of the file to load (standard is .world)
		@param name Optionally give a name to the world (if no name is given, one
		will be generated).
		*/
		PagedWorld* loadWorld(const String& filename, const String& name = StringUtil::BLANK);
		/** Load a new PagedWorld from a stream. 
		@param stream A stream from which to load the world data
		@param name Optionally give a name to the world (if no name is given, one
		will be generated).
		*/
		PagedWorld* loadWorld(const DataStreamPtr& stream, const String& name = StringUtil::BLANK);
		/** Save a PagedWorld instance to a file. 
		@param world The world to be saved
		@param filename The filename to save the data to
		@param arch The Archive that filename is relative to (optional)
		*/
		void saveWorld(PagedWorld* world, const String& filename);
		/** Save a PagedWorld instance to a file. 
		@param world The world to be saved
		@param stream The stream to save the data to
		*/
		void saveWorld(PagedWorld* world, const DataStreamPtr& stream);
		/** Get a named world.
		@params name The name of the world (not a filename, the identifying name)
		@returns The world, or null if the world doesn't exist.
		*/
		PagedWorld* getWorld(const String& name);
		typedef map<String, PagedWorld*>::type WorldMap;
		/** Get a reference to the worlds that are currently loaded. */
		const WorldMap& getWorlds() const { return mWorlds; }


		typedef map<String, PageStrategy*>::type StrategyMap;
		/** Add a new PageStrategy implementation. 
		@remarks
			The caller remains resonsible for destruction of this instance. 
		*/
		void addStrategy(PageStrategy* strategy);

		/** Remove a PageStrategy implementation. 
		*/
		void removeStrategy(PageStrategy* strategy);

		/** Get a PageStrategy.
		@param name The name of the strategy to retrieve
		@returns Pointer to a PageStrategy, or null if the strategy was not found.
		*/
		PageStrategy* getStrategy(const String& name);

		/** Get a reference to the registered strategies.
		*/
		const StrategyMap& getStrategies() const;
		/// Get the request queue
		PageRequestQueue* getQueue() const { return mQueue; }


		/** Set the PageProvider which can provide streams for any Page. 
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
		
		/** Get the PageProvider which can provide streams for any Page. */
		PageProvider* getPageProvider() const { return mPageProvider; }

		/** Give a provider the opportunity to prepare page content procedurally. 
		@remarks
		You should not call this method directly. This call may well happen in 
		a separate thread so it should not access GPU resources, use _loadProceduralPage
		for that
		@returns true if the page was populated, false otherwise
		*/
		virtual bool _prepareProceduralPage(Page* page, PagedWorldSection* section);
		/** Give a provider the opportunity to prepare page content procedurally. 
		@remarks
		You should not call this method directly. This call will happen in 
		the main render thread so it can access GPU resources. Use _prepareProceduralPage
		for background preparation.
		@returns true if the page was populated, false otherwise
		*/
		virtual bool _loadProceduralPage(Page* page, PagedWorldSection* section);
		/** Give a manager  the opportunity to unload page content procedurally. 
		@remarks
		You should not call this method directly. This call will happen in 
		the main render thread so it can access GPU resources. Use _unprepareProceduralPage
		for background preparation.
		@returns true if the page was populated, false otherwise
		*/
		virtual bool _unloadProceduralPage(Page* page, PagedWorldSection* section);
		/** Give a manager  the opportunity to unprepare page content procedurally. 
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

		/** Get a serialiser set up to write Page data for the given PageID. 
		@param pageID The ID of the page being requested
		@param section The parent section to which this page will belong
		@remarks
		The StreamSerialiser returned is the responsibility of the caller to
		delete. 
		*/
		StreamSerialiser* _writePageStream(PageID pageID, PagedWorldSection* section);
		/** Get a serialiser set up to read PagedWorld data for the given world name. 
		@remarks
			The StreamSerialiser returned is the responsibility of the caller to
			delete. 
		*/
		StreamSerialiser* _readWorldStream(const String& filename);

		/** Get a serialiser set up to write PagedWorld data. 
		@remarks
		The StreamSerialiser returned is the responsibility of the caller to
		delete. 
		*/
		StreamSerialiser* _writeWorldStream(const String& filename);

		/** Get the resource group that will be used to read/write files when the
			default load routines are used. 
		*/
		const String& getPageResourceGroup() const { return mPageResourceGroup; }
		/** Set the resource group that will be used to read/write files when the
		default load routines are used. 
		*/
		void getPageResourceGroup(const String& g) { mPageResourceGroup = g; }

		/** Tells the paging system to start tracking a given camera. 
		@remarks
			In order for the paging system to funciton it needs to know which
			Cameras to track. You may not want to have all your cameras affect
			the paging system, so just add the cameras you want it to keep track of
			here. 
		*/
		void addCamera(Camera* c);

		/** Tells the paging system to stop tracking a given camera. 
		*/
		void removeCamera(Camera* c);

		/** Returns whether or not a given camera is being watched by the paging system.
		*/
		bool hasCamera(Camera* c) const;

		typedef vector<Camera*>::type CameraList;
		/** Returns a list of camerasl being tracked. */
		const CameraList& getCameraList() const;

		/** Set the debug display level.
		@remarks
			This setting controls how much debug information is displayed in the scene.
			The exact interpretation of this value depends on the PageStrategy you're
			using, and whether the PageContent decides to also display debug information.
			Generally speaking, 0 means no debug display, 1 means simple debug
			display (usually the PageStrategy) and anything more than that is
			up to the classes involved. 
		*/
		void setDebugDisplayLevel(uint8 lvl) { mDebugDisplayLvl = lvl; }
		/** Get the debug display level. */
		uint8 getDebugDisplayLevel() const { return mDebugDisplayLvl; }


	protected:

		class EventRouter : public Camera::Listener, public FrameListener
		{
		public:
			PageManager* pManager;
			WorldMap* pWorldMap;

			EventRouter() {}
			~EventRouter() {}

			void cameraPreRenderScene(Camera* cam);
			void cameraDestroyed(Camera* cam);
			bool frameStarted(const FrameEvent& evt);
			bool frameEnded(const FrameEvent& evt);
		};

		void createStandardStrategies();

		WorldMap mWorlds;
		StrategyMap mStrategies;
		NameGenerator mWorldNameGenerator;
		PageRequestQueue* mQueue;
		PageProvider* mPageProvider;
		String mPageResourceGroup;
		CameraList mCameraList;
		EventRouter mEventRouter;
		uint8 mDebugDisplayLvl;

		Grid2DPageStrategy* mGrid2DPageStrategy;
	};

	/** @} */
	/** @} */
}





#endif 