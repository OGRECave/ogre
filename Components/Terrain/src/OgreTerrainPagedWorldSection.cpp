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
#include "OgreTerrainPagedWorldSection.h"
#include "OgreTerrainGroup.h"
#include "OgreGrid2DPageStrategy.h"
#include "OgrePagedWorld.h"
#include "OgrePageManager.h"

namespace Ogre
{
	//---------------------------------------------------------------------
	TerrainPagedWorldSection::TerrainPagedWorldSection(const String& name, PagedWorld* parent, SceneManager* sm)
		: PagedWorldSection(name, parent, sm)
		, mTerrainGroup(0)
	{
		// we always use a grid strategy
		setStrategy(parent->getManager()->getStrategy("Grid2D"));

	}
	//---------------------------------------------------------------------
	TerrainPagedWorldSection::~TerrainPagedWorldSection()
	{
		OGRE_DELETE mTerrainGroup;
	}
	//---------------------------------------------------------------------
	void TerrainPagedWorldSection::init(TerrainGroup* grp)
	{
		if (mTerrainGroup == grp)
			return;

		if (mTerrainGroup)
			OGRE_DELETE mTerrainGroup;

		mTerrainGroup = grp;
		syncSettings();

		// Unload all existing terrain pages, because we want the paging system
		// to be in charge of this
		mTerrainGroup->removeAllTerrains();
	}
	//---------------------------------------------------------------------
	void TerrainPagedWorldSection::syncSettings()
	{

		// Base grid on terrain settings
		Grid2DPageStrategyData* gridData = getGridStrategyData();
		switch (mTerrainGroup->getAlignment())
		{
		case Terrain::ALIGN_X_Y:
			gridData->setMode(G2D_X_Y);
			break;
		case Terrain::ALIGN_X_Z:
			gridData->setMode(G2D_X_Z);
			break;
		case Terrain::ALIGN_Y_Z:
			gridData->setMode(G2D_Y_Z);
			break;
		}
		gridData->setOrigin(mTerrainGroup->getOrigin());

		gridData->setCellSize(mTerrainGroup->getTerrainWorldSize());

	}
	//---------------------------------------------------------------------
	void TerrainPagedWorldSection::setLoadRadius(Real sz)
	{
		getGridStrategyData()->setLoadRadius(sz);
	}
	//---------------------------------------------------------------------
	Real TerrainPagedWorldSection::getLoadRadius() const
	{
		return getGridStrategyData()->getLoadRadius();
	}
	//---------------------------------------------------------------------
	void TerrainPagedWorldSection::setHoldRadius(Real sz)
	{
		getGridStrategyData()->setHoldRadius(sz);
	}
	//---------------------------------------------------------------------
	Real TerrainPagedWorldSection::getHoldRadius()
	{
		return getGridStrategyData()->getHoldRadius();
	}
	//---------------------------------------------------------------------
	void TerrainPagedWorldSection::setPageRange(int32 minX, int32 minY, int32 maxX, int32 maxY)
	{
		getGridStrategyData()->setCellRange(minX, minY, maxX, maxY);
	}
	//---------------------------------------------------------------------
	void TerrainPagedWorldSection::setPageRangeMinX(int32 minX)
	{
		getGridStrategyData()->setCellRangeMinX(minX);
	}
	//---------------------------------------------------------------------
	void TerrainPagedWorldSection::setPageRangeMinY(int32 minY)
	{
		getGridStrategyData()->setCellRangeMinY(minY);
	}
	//---------------------------------------------------------------------
	void TerrainPagedWorldSection::setPageRangeMaxX(int32 maxX)
	{
		getGridStrategyData()->setCellRangeMaxX(maxX);
	}
	//---------------------------------------------------------------------
	void TerrainPagedWorldSection::setPageRangeMaxY(int32 maxY)
	{
		getGridStrategyData()->setCellRangeMaxY(maxY);
	}
	//---------------------------------------------------------------------
	int32 TerrainPagedWorldSection::getPageRangeMinX() const
	{
		return getGridStrategyData()->getCellRangeMinX();
	}
	//---------------------------------------------------------------------
	int32 TerrainPagedWorldSection::getPageRangeMinY() const
	{
		return getGridStrategyData()->getCellRangeMinY();
	}
	//---------------------------------------------------------------------
	int32 TerrainPagedWorldSection::getPageRangeMaxX() const
	{
		return getGridStrategyData()->getCellRangeMaxX();
	}
	//---------------------------------------------------------------------
	int32 TerrainPagedWorldSection::getPageRangeMaxY() const
	{
		return getGridStrategyData()->getCellRangeMaxY();
	}
	//---------------------------------------------------------------------
	Grid2DPageStrategy* TerrainPagedWorldSection::getGridStrategy() const
	{
		return static_cast<Grid2DPageStrategy*>(this->getStrategy());
	}
	//---------------------------------------------------------------------
	Grid2DPageStrategyData* TerrainPagedWorldSection::getGridStrategyData() const
	{
		return static_cast<Grid2DPageStrategyData*>(mStrategyData);
	}
	//---------------------------------------------------------------------
	void TerrainPagedWorldSection::loadSubtypeData(StreamSerialiser& ser)
	{
		// we load the TerrainGroup information from here
		if (!mTerrainGroup)
			mTerrainGroup = OGRE_NEW TerrainGroup(getSceneManager());

		mTerrainGroup->loadGroupDefinition(ser);

		// params that are in the Grid2DStrategyData will have already been loaded
		// as part of the main load() routine

		syncSettings();
	}
	//---------------------------------------------------------------------
	void TerrainPagedWorldSection::saveSubtypeData(StreamSerialiser& ser)
	{
		mTerrainGroup->saveGroupDefinition(ser);

		// params that are in the Grid2DStrategyData will have already been saved
		// as part of the main save() routine

	}
	//---------------------------------------------------------------------
	void TerrainPagedWorldSection::loadPage(PageID pageID, bool forceSynchronous)
	{
		PageMap::iterator i = mPages.find(pageID);
		if (i == mPages.end())
		{
			// trigger terrain load
			long x, y;
			// pageID is the same as a packed index
			mTerrainGroup->unpackIndex(pageID, &x, &y);
			mTerrainGroup->defineTerrain(x, y);
			mTerrainGroup->loadTerrain(x, y, forceSynchronous);
		}

		PagedWorldSection::loadPage(pageID, forceSynchronous);
	}
	//---------------------------------------------------------------------
	void TerrainPagedWorldSection::unloadPage(PageID pageID, bool forceSynchronous)
	{
		PagedWorldSection::unloadPage(pageID, forceSynchronous);


		// trigger terrain unload
		long x, y;
		// pageID is the same as a packed index
		mTerrainGroup->unpackIndex(pageID, &x, &y);
		mTerrainGroup->unloadTerrain(x, y);


	}




}

