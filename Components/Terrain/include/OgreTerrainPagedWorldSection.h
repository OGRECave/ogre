/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

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

#ifndef __Ogre_TerrainPagedWorldSection_H__
#define __Ogre_TerrainPagedWorldSection_H__

#include "OgreTerrainPrerequisites.h"
#include "OgrePagedWorldSection.h"
#include "OgrePageManager.h"
#include "OgreWorkQueue.h"
#include "OgreTerrainGroup.h"


namespace Ogre
{
	class Grid2DPageStrategy;
	class Grid2DPageStrategyData;
	class TerrainGroup;

	/** \addtogroup Optional Components
	*  @{
	*/
	/** \addtogroup Terrain
	*  Some details on the terrain component
	*  @{
	*/

	/** A world section which includes paged terrain. 
	@remarks
		Rather than implement terrain paging as a PageContent subclass, because terrain
		benefits from direct knowledge of neighbour arrangements and the tight
		coupling between that and the paging strategy, instead we use a PagedWorldSection
		subclass. This automatically provides a PageStrategy subclass of the
		correct type (Grid2DPageStrategy) and derives the correct settings for
		it compared to the terrain being used. This frees the user from having to 
		try to match all these up through the generic interfaces. 
	@par
		When creating this in code, the user should make use of the helper
		functions on the TerrainPaging class. The basic sequence is that 
		you define your terrain settings in a TerrainGroup, and then create an 
		instance of TerrainPagedWorldSection via TerrainPaging::createWorldSection. 
		That's basically all there is to it - the normal rules of TerrainGroup
		apply, it's just that instead of having to choose when to try to load / unload
		pages from the TerrainGroup, that is automatically done by the paging system.
		You can also create other types of content in this PagedWorldSection other
		than terrain, it's just that this comes with terrain handling built-in.
	@par
		When this world data is saved, you basically get 3 sets of data - firstly
		the top-level 'world data' which is where the definition of the world
		sections are stored (and hence, the definition of this class, although none
		of the terrain instance content is included). Secondly, you get a number
		of .page files which include any other content that you might have inserted
		into the pages. Lastly, you get the standard terrain data files which are
		saved as per TerrainGroup.
	*/
	class _OgreTerrainExport TerrainPagedWorldSection : public PagedWorldSection,
		public WorkQueue::RequestHandler, public WorkQueue::ResponseHandler
	{
	public:
		/** Constructor.
		@param name The name of the section
		@param parent The parent world
		@param sm The SceneManager to use (can be left as null if to be loaded)
		*/
		TerrainPagedWorldSection(const String& name, PagedWorld* parent, SceneManager* sm);
		virtual ~TerrainPagedWorldSection();

		/** Initialise this section from an existing TerrainGroup instance. 
		@remarks
			This is the route you will take if you're defining this world section
			from scratch in code. The other alternative is that you'll be loading
			this section from a file, in which case all the settings will be
			derived from that. 
		@param grp The TerrainGroup which will form the basis of this world section. 
			The instance will be owned by this class from now on and will be destroyed by it.
		*/
		virtual void init(TerrainGroup* grp);

		/** Get the TerrainGroup which this world section is using. 
		@remarks For information, you can use the returned TerrainGroup to 
			convert to/from x/y locations and the pageID, since the grid strategy
			is the same. 
		*/
		virtual TerrainGroup* getTerrainGroup() { return mTerrainGroup; }

		/// Set the loading radius 
		virtual void setLoadRadius(Real sz);
		/// Get the loading radius 
		virtual Real getLoadRadius() const;
		/// Set the Holding radius 
		virtual void setHoldRadius(Real sz);
		/// Get the Holding radius 
		virtual Real getHoldRadius();
		/// Set the index range of all Pages (values outside this will be ignored)
		virtual void setPageRange(int32 minX, int32 minY, int32 maxX, int32 maxY);
		/// Set the index range of all Pages (values outside this will be ignored)
		virtual void setPageRangeMinX(int32 minX);
		/// Set the index range of all Pages (values outside this will be ignored)
		virtual void setPageRangeMinY(int32 minY);
		/// Set the index range of all Pages (values outside this will be ignored)
		virtual void setPageRangeMaxX(int32 maxX);
		/// Set the index range of all Pages (values outside this will be ignored)
		virtual void setPageRangeMaxY(int32 maxY);
		/// get the index range of all Pages (values outside this will be ignored)
		virtual int32 getPageRangeMinX() const;
		/// get the index range of all Pages (values outside this will be ignored)
		virtual int32 getPageRangeMinY() const;
		/// get the index range of all Pages (values outside this will be ignored)
		virtual int32 getPageRangeMaxX() const;
		/// get the index range of all Pages (values outside this will be ignored)
		virtual int32 getPageRangeMaxY() const;

		/// Convenience method - this section always uses a grid strategy
		virtual Grid2DPageStrategy* getGridStrategy() const;
		/// Convenience method - this section always uses a grid strategy
		virtual Grid2DPageStrategyData* getGridStrategyData() const;

		/// Set the interval between the loading of single pages in milliseconds (ms)
		virtual void setLoadingIntervalMs(uint32 loadingIntervalMs);
		/// Get the interval between the loading of single pages in milliseconds (ms)
		virtual uint32 getLoadingIntervalMs() const;

		/// Overridden from PagedWorldSection
		void loadPage(PageID pageID, bool forceSynchronous = false);
		/// Overridden from PagedWorldSection
		void unloadPage(PageID pageID, bool forceSynchronous = false);

		/// WorkQueue::RequestHandler override
		WorkQueue::Response* handleRequest(const WorkQueue::Request* req, const WorkQueue* srcQ);
		/// WorkQueue::ResponseHandler override
		void handleResponse(const WorkQueue::Response* res, const WorkQueue* srcQ);

		static const uint16 WORKQUEUE_LOAD_TERRAIN_PAGE_REQUEST;

		class TerrainDefiner : public TerrainAlloc
		{
		public:
			virtual void define(TerrainGroup* terrainGroup, long x, long y)
			{
				terrainGroup->defineTerrain(x,y);
			}
            virtual ~TerrainDefiner() {}
		};

		void setDefiner(TerrainDefiner* terrainDefiner)
		{
			if(mTerrainDefiner)
				OGRE_DELETE mTerrainDefiner;
			mTerrainDefiner = terrainDefiner;
		}

	protected:
		TerrainGroup* mTerrainGroup;
		TerrainDefiner* mTerrainDefiner;
		std::list<PageID> mPagesInLoading;
		bool mHasRunningTasks;
		uint16 mWorkQueueChannel;
		unsigned long mNextLoadingTime;
		uint32 mLoadingIntervalMs;

		/// Overridden from PagedWorldSection
		void loadSubtypeData(StreamSerialiser& ser);
		void saveSubtypeData(StreamSerialiser& ser);

		virtual void syncSettings();

	};





	/** @} */
	/** @} */
}

#endif
