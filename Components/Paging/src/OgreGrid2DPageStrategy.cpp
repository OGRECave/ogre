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
#include "OgreGrid2DPageStrategy.h"
#include "OgreStreamSerialiser.h"
#include "OgreException.h"
#include "OgreCamera.h"
#include "OgrePagedWorldSection.h"
#include "OgrePage.h"
#include "OgreSceneNode.h"
#include "OgreSceneManager.h"
#include "OgreMaterialManager.h"
#include "OgreManualObject.h"
#include "OgrePageManager.h"

namespace Ogre
{
	//---------------------------------------------------------------------
	const uint32 Grid2DPageStrategyData::CHUNK_ID = StreamSerialiser::makeIdentifier("G2DD");
	const uint16 Grid2DPageStrategyData::CHUNK_VERSION = 1;
	//---------------------------------------------------------------------
	Grid2DPageStrategyData::Grid2DPageStrategyData()
		: PageStrategyData()
		, mMode(G2D_X_Z)
		, mWorldOrigin(Vector3::ZERO)
		, mOrigin(Vector2::ZERO)
		, mGridExtentsHorz(65536)
		, mGridExtentsVert(65536)
		, mCellSize(1000)
		, mLoadRadius(2000)
		, mHoldRadius(3000)
	{
		updateDerivedMetrics();
		
	}
	//---------------------------------------------------------------------
	Grid2DPageStrategyData::~Grid2DPageStrategyData()
	{

	}
	//---------------------------------------------------------------------
	void Grid2DPageStrategyData::setMode(Grid2DMode mode)
	{
		mMode = mode;
		// reset origin
		setOrigin(mWorldOrigin);
	}
	//---------------------------------------------------------------------
	void Grid2DPageStrategyData::setOrigin(const Vector3& worldOrigin)
	{
		mWorldOrigin = worldOrigin;
		convertWorldToGridSpace(mWorldOrigin, mOrigin);
		updateDerivedMetrics();
	}
	//---------------------------------------------------------------------
	void Grid2DPageStrategyData::convertWorldToGridSpace(const Vector3& world, Vector2& grid)
	{
		switch(mMode)
		{
		case G2D_X_Z:
			grid.x = world.x;
			grid.y = -world.z;
			break;
		case G2D_X_Y:
			grid.x = world.x;
			grid.y = world.y;
			break;
		case G2D_Y_Z:
			grid.x = -world.z;
			grid.y = world.y;
			break;
		}
	}
	//---------------------------------------------------------------------
	void Grid2DPageStrategyData::convertGridToWorldSpace(const Vector2& grid, Vector3& world)
	{
		// Note that we don't set the 3rd coordinate, let the caller determine that
		switch(mMode)
		{
		case G2D_X_Z:
			world.x = grid.x;
			world.z = -grid.y;
			break;
		case G2D_X_Y:
			world.x = grid.x;
			world.y = grid.y;
			break;
		case G2D_Y_Z:
			world.z = -grid.x;
			world.y = grid.y;
			break;
		}
	}
	//---------------------------------------------------------------------
	void Grid2DPageStrategyData::updateDerivedMetrics()
	{
		mLoadRadiusInCells = mLoadRadius / mCellSize;
		mHoldRadiusInCells = mHoldRadius / mCellSize;
		mBottomLeft = mOrigin - Vector2(mCellSize * mGridExtentsHorz * 0.5f, mCellSize * mGridExtentsVert* 0.5f);
	}
	//---------------------------------------------------------------------
	void Grid2DPageStrategyData::determineGridLocation(const Vector2& gridpos, uint16* row, uint16* col)
	{
		// get distance from bottom-left (indexing start)
		Vector2 localPos = gridpos - mBottomLeft;
		// in cells
		localPos = localPos / mCellSize;

		// truncate
		*col = static_cast<uint16>(localPos.x);
		*row = static_cast<uint16>(localPos.y);


	}
	//---------------------------------------------------------------------
	void Grid2DPageStrategyData::getBottomLeftGridSpace(uint16 row, uint16 col, Vector2& bl)
	{
		bl.x = mBottomLeft.x + col * mCellSize;
		bl.y = mBottomLeft.y + row * mCellSize;
	}
	//---------------------------------------------------------------------
	void Grid2DPageStrategyData::getMidPointGridSpace(uint16 row, uint16 col, Vector2& mid)
	{
		getBottomLeftGridSpace(row, col, mid);
		mid.x += mCellSize * 0.5f;
		mid.y += mCellSize * 0.5f;
	}
	//---------------------------------------------------------------------
	void Grid2DPageStrategyData::getCornersGridSpace(uint16 row, uint16 col, Vector2* pFourPoints)
	{
		getBottomLeftGridSpace(row, col, pFourPoints[0]);
		pFourPoints[1] = pFourPoints[0] + Vector2(mCellSize, 0);
		pFourPoints[2] = pFourPoints[0] + Vector2(mCellSize, mCellSize);
		pFourPoints[3] = pFourPoints[0] + Vector2(0, mCellSize);
	}
	//---------------------------------------------------------------------
	void Grid2DPageStrategyData::setCellSize(Real sz)
	{
		mCellSize = sz;
		updateDerivedMetrics();
	}
	//---------------------------------------------------------------------
	void Grid2DPageStrategyData::setCellCount(uint32 horz, uint32 vert)
	{
		mGridExtentsHorz = horz;
		mGridExtentsVert = vert;
		updateDerivedMetrics();
	}
	//---------------------------------------------------------------------
	void Grid2DPageStrategyData::setCellCountHorz(uint32 horz)
	{
		mGridExtentsHorz = horz;
		updateDerivedMetrics();
	}
	//---------------------------------------------------------------------
	void Grid2DPageStrategyData::setCellCountVert(uint32 vert)
	{
		mGridExtentsVert = vert;
		updateDerivedMetrics();
	}
	//---------------------------------------------------------------------
	uint32 Grid2DPageStrategyData::getCellCountHorz() const
	{
		return mGridExtentsHorz;
	}
	//---------------------------------------------------------------------
	uint32 Grid2DPageStrategyData::getCellCountVert() const
	{
		return mGridExtentsVert;
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	void Grid2DPageStrategyData::setLoadRadius(Real sz)
	{
		mLoadRadius = sz;
		updateDerivedMetrics();
	}
	//---------------------------------------------------------------------
	void Grid2DPageStrategyData::setHoldRadius(Real sz)
	{
		mHoldRadius = sz;
		updateDerivedMetrics();
	}
	//---------------------------------------------------------------------
	PageID Grid2DPageStrategyData::calculatePageID(uint16 row, uint16 col)
	{
		return (PageID)row * mGridExtentsHorz + col;
	}
	//---------------------------------------------------------------------
	void Grid2DPageStrategyData::calculateRowCol(PageID inPageID, uint16 *row, uint16 *col)
	{
		// inverse of calculatePageID
		*row = inPageID / mGridExtentsHorz;
		*col = inPageID % mGridExtentsHorz;
	}
	//---------------------------------------------------------------------
	bool Grid2DPageStrategyData::load(StreamSerialiser& ser)
	{
		if (!ser.readChunkBegin(CHUNK_ID, CHUNK_VERSION, "Grid2DPageStrategyData"))
			return false;

		uint8 readMode;
		ser.read(&readMode);
		mMode = (Grid2DMode)readMode;

		Vector3 origin;
		ser.read(&origin);
		setOrigin(origin);

		ser.read(&mCellSize);
		ser.read(&mLoadRadius);
		ser.read(&mHoldRadius);

		ser.readChunkEnd(CHUNK_ID);

		return true;
	}
	//---------------------------------------------------------------------
	void Grid2DPageStrategyData::save(StreamSerialiser& ser)
	{
		ser.writeChunkBegin(CHUNK_ID, CHUNK_VERSION);

		uint8 readMode = (uint8)mMode;
		ser.write(&readMode);

		ser.write(&mWorldOrigin);
		ser.write(&mCellSize);
		ser.write(&mLoadRadius);
		ser.write(&mHoldRadius);

		ser.writeChunkEnd(CHUNK_ID);
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	Grid2DPageStrategy::Grid2DPageStrategy(PageManager* manager)
		: PageStrategy("Grid2D", manager)
	{

	}
	//---------------------------------------------------------------------
	Grid2DPageStrategy::~Grid2DPageStrategy()
	{

	}
	//---------------------------------------------------------------------
	void Grid2DPageStrategy::notifyCamera(Camera* cam, PagedWorldSection* section)
	{
		Grid2DPageStrategyData* stratData = static_cast<Grid2DPageStrategyData*>(section->getStrategyData());

		const Vector3& pos = cam->getDerivedPosition();
		Vector2 gridpos;
		stratData->convertWorldToGridSpace(pos, gridpos);
		uint16 row, col;
		stratData->determineGridLocation(gridpos, &row, &col);

		Real loadRadius = stratData->getLoadRadiusInCells();
		Real holdRadius = stratData->getHoldRadiusInCells();
		// scan the whole Hold range
		Real frowmin = (Real)row - holdRadius;
		Real frowmax = (Real)row + holdRadius;
		Real fcolmin = (Real)col - holdRadius;
		Real fcolmax = (Real)col + holdRadius;

		uint16 clampRowAt = stratData->getCellCountVert() - 1;
		uint16 clampColAt = stratData->getCellCountHorz() - 1;

		// Round UP max, round DOWN min
		uint16 rowmin = frowmin < 0 ? 0 : (uint16)floor(frowmin);
		uint16 rowmax = frowmax > clampRowAt ? clampRowAt : (uint16)ceil(frowmax);
		uint16 colmin = fcolmin < 0 ? 0 : (uint16)floor(fcolmin);
		uint16 colmax = fcolmax > clampColAt ? clampColAt : (uint16)ceil(fcolmax);
		// the inner, active load range
		frowmin = (Real)row - loadRadius;
		frowmax = (Real)row + loadRadius;
		fcolmin = (Real)col - loadRadius;
		fcolmax = (Real)col + loadRadius;
		// Round UP max, round DOWN min
		uint16 loadrowmin = frowmin < 0 ? 0 : (uint16)floor(frowmin);
		uint16 loadrowmax = frowmax > clampRowAt ? clampRowAt : (uint16)ceil(frowmax);
		uint16 loadcolmin = fcolmin < 0 ? 0 : (uint16)floor(fcolmin);
		uint16 loadcolmax = fcolmax > clampColAt ? clampColAt : (uint16)ceil(fcolmax);

		for (uint16 r = rowmin; r <= rowmax; ++r)
		{
			for (uint16 c = colmin; c <= colmax; ++c)
			{
				PageID pageID = stratData->calculatePageID(r, c);
				if (r >= loadrowmin && r <= loadrowmax && c >= loadcolmin && c <= loadcolmax)
				{
					// in the 'load' range, request it
					section->loadPage(pageID);
				}
				else
				{
					// in the outer 'hold' range, keep it but don't actively load
					section->holdPage(pageID);
				}
				// other pages will by inference be marked for unloading
			}
		}	
		


	}
	//---------------------------------------------------------------------
	PageStrategyData* Grid2DPageStrategy::createData()
	{
		return OGRE_NEW Grid2DPageStrategyData();
	}
	//---------------------------------------------------------------------
	void Grid2DPageStrategy::destroyData(PageStrategyData* d)
	{
		OGRE_DELETE d;
	}
	//---------------------------------------------------------------------
	void Grid2DPageStrategy::updateDebugDisplay(Page* p, SceneNode* sn)
	{
		uint8 dbglvl = mManager->getDebugDisplayLevel();
		if (dbglvl)
		{
			// we could try to avoid updating the geometry every time here, but this 
			// wouldn't easily deal with paging parameter changes. There shouldn't 
			// be that many pages anyway, and this is debug after all, so update every time
			uint16 row, col;
			Grid2DPageStrategyData* stratData = static_cast<Grid2DPageStrategyData*>(
				p->getParentSection()->getStrategyData());
			stratData->calculateRowCol(p->getID(), &row, &col);

			Grid2DPageStrategyData* data = static_cast<Grid2DPageStrategyData*>(p->getParentSection()->getStrategyData());

			// Determine our centre point, we'll anchor here
			// Note that world points are initialised to ZERO since only 2 dimensions
			// are updated by the grid data (we could display this grid anywhere)
			Vector2 gridMidPoint;
			Vector3 worldMidPoint = Vector3::ZERO;
			data->getMidPointGridSpace(row, col, gridMidPoint);
			data->convertGridToWorldSpace(gridMidPoint, worldMidPoint);

			sn->setPosition(worldMidPoint);

			Vector2 gridCorners[4];
			Vector3 worldCorners[4];

			data->getCornersGridSpace(row, col, gridCorners);
			for (int i = 0; i < 4; ++i)
			{
				worldCorners[i] = Vector3::ZERO;
				data->convertGridToWorldSpace(gridCorners[i], worldCorners[i]);
				// make relative to mid point
				worldCorners[i] -= worldMidPoint;
			}

			String matName = "Ogre/G2D/Debug";
			MaterialPtr mat = MaterialManager::getSingleton().getByName(matName);
			if (mat.isNull())
			{
				mat = MaterialManager::getSingleton().create(matName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
				Pass* pass = mat->getTechnique(0)->getPass(0);
				pass->setLightingEnabled(false);
				pass->setVertexColourTracking(TVC_AMBIENT);
				pass->setDepthWriteEnabled(false);
				mat->load();
			}


			ManualObject* mo = 0;
			if (sn->numAttachedObjects() == 0)
			{
				mo = p->getParentSection()->getSceneManager()->createManualObject();
				mo->begin(matName, RenderOperation::OT_LINE_STRIP);
			}
			else
			{
				mo = static_cast<ManualObject*>(sn->getAttachedObject(0));
				mo->beginUpdate(0);
			}

			ColourValue vcol = p->getStatus() == Page::STATUS_LOADED ? 
				ColourValue::Green : ColourValue::Red;
			for(int i = 0; i < 5; ++i)
			{
				mo->position(worldCorners[i%4]);
				mo->colour(vcol);
			}

			mo->end();

			if (sn->numAttachedObjects() == 0)
				sn->attachObject(mo);

		}

	}
	//---------------------------------------------------------------------
	PageID Grid2DPageStrategy::getPageID(const Vector3& worldPos, PagedWorldSection* section)
	{
		Grid2DPageStrategyData* stratData = static_cast<Grid2DPageStrategyData*>(section->getStrategyData());

		Vector2 gridpos;
		stratData->convertWorldToGridSpace(worldPos, gridpos);
		uint16 row, col;
		stratData->determineGridLocation(gridpos, &row, &col);
		return stratData->calculatePageID(row, col);
		
	}


}

