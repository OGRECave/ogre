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
		, mCellSize(1000)
		, mLoadRadius(2000)
		, mHoldRadius(3000)
		, mMinCellX(-32768)
		, mMinCellY(-32768)
		, mMaxCellX(32767)
		, mMaxCellY(32767)
	{
		updateDerivedMetrics();
		
	}
	//---------------------------------------------------------------------
	void Grid2DPageStrategyData::setCellRange(int32 minX, int32 minY, int32 maxX, int32 maxY)
	{
		mMinCellX = minX;
		mMinCellY = minY;
		mMaxCellX = maxX;
		mMaxCellY = maxY;
	}
	//---------------------------------------------------------------------
	void Grid2DPageStrategyData::setCellRangeMinX(int32 minX)
	{
		mMinCellX = minX;
	}
	//---------------------------------------------------------------------
	void Grid2DPageStrategyData::setCellRangeMinY(int32 minY)
	{
		mMinCellY = minY;
	}
	//---------------------------------------------------------------------
	void Grid2DPageStrategyData::setCellRangeMaxX(int32 maxX)
	{
		mMaxCellX = maxX;
	}
	//---------------------------------------------------------------------
	void Grid2DPageStrategyData::setCellRangeMaxY(int32 maxY)
	{
		mMaxCellY = maxY;
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
	}
	//---------------------------------------------------------------------
	void Grid2DPageStrategyData::determineGridLocation(const Vector2& gridpos, int32 *x, int32 *y)
	{
		Vector2 relPos = gridpos - mOrigin;
		Real offset = mCellSize * 0.5f;
		relPos.x += offset;
		relPos.y += offset;

		*x = static_cast<int32>(floor(relPos.x / mCellSize));
		*y = static_cast<int32>(floor(relPos.y / mCellSize));

	}
	//---------------------------------------------------------------------
	void Grid2DPageStrategyData::getBottomLeftGridSpace(int32 x, int32 y, Vector2& bl)
	{
		Real offset = mCellSize * 0.5f;
		bl.x = mOrigin.x - offset + x * mCellSize;
		bl.y = mOrigin.y - offset + y * mCellSize;
	}
	//---------------------------------------------------------------------
	void Grid2DPageStrategyData::getMidPointGridSpace(int32 x, int32 y, Vector2& mid)
	{
		mid.x = mOrigin.x + x * mCellSize;
		mid.y = mOrigin.y + y * mCellSize;
	}
	//---------------------------------------------------------------------
	void Grid2DPageStrategyData::getCornersGridSpace(int32 x, int32 y, Vector2* pFourPoints)
	{
		getBottomLeftGridSpace(x, y, pFourPoints[0]);
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
	PageID Grid2DPageStrategyData::calculatePageID(int32 x, int32 y)
	{
		// Convert to signed 16-bit so sign bit is in bit 15
		int16 xs16 = static_cast<int16>(x);
		int16 ys16 = static_cast<int16>(y);

		// convert to unsigned because we do not want to propagate sign bit to 32-bits
		uint16 x16 = static_cast<uint16>(xs16);
		uint16 y16 = static_cast<uint16>(ys16);

		uint32 key = 0;
		key = (x16 << 16) | y16;

		return (PageID)key;

	}
	//---------------------------------------------------------------------
	void Grid2DPageStrategyData::calculateCell(PageID inPageID, int32 *x, int32 *y)
	{
		// inverse of calculatePageID
		// unsigned versions
		uint16 y16 = static_cast<uint16>(inPageID & 0xFFFF);
		uint16 x16 = static_cast<uint16>((inPageID >> 16) & 0xFFFF);

		*x = static_cast<int16>(x16);
		*y = static_cast<int16>(y16);

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
		ser.read(&mMinCellX);
		ser.read(&mMaxCellX);
		ser.read(&mMinCellY);
		ser.read(&mMaxCellY);

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
		ser.write(&mMinCellX);
		ser.write(&mMaxCellX);
		ser.write(&mMinCellY);
		ser.write(&mMaxCellY);

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
		int32 x, y;
		stratData->determineGridLocation(gridpos, &x, &y);

		Real loadRadius = stratData->getLoadRadiusInCells();
		Real holdRadius = stratData->getHoldRadiusInCells();
		// scan the whole Hold range
		Real fxmin = (Real)x - holdRadius;
		Real fxmax = (Real)x + holdRadius;
		Real fymin = (Real)y - holdRadius;
		Real fymax = (Real)y + holdRadius;

		int32 xmin = stratData->getCellRangeMinX();
		int32 xmax = stratData->getCellRangeMaxX();
		int32 ymin = stratData->getCellRangeMinY();
		int32 ymax = stratData->getCellRangeMaxY();

		// Round UP max, round DOWN min
		xmin = fxmin < xmin ? xmin : (int32)floor(fxmin);
		xmax = fxmax > xmax ? xmax : (int32)ceil(fxmax);
		ymin = fymin < ymin ? ymin : (int32)floor(fymin);
		ymax = fymax > ymax ? ymax : (int32)ceil(fymax);
		// the inner, active load range
		fxmin = (Real)x - loadRadius;
		fxmax = (Real)x + loadRadius;
		fymin = (Real)y - loadRadius;
		fymax = (Real)y + loadRadius;
		// Round UP max, round DOWN min
		int32 loadxmin = fxmin < xmin ? xmin : (int32)floor(fxmin);
		int32 loadxmax = fxmax > xmax ? xmax : (int32)ceil(fxmax);
		int32 loadymin = fymin < ymin ? ymin : (int32)floor(fymin);
		int32 loadymax = fymax > ymax ? ymax : (int32)ceil(fymax);

		for (int32 cy = ymin; cy <= ymax; ++cy)
		{
			for (int32 cx = xmin; cx <= xmax; ++cx)
			{
				PageID pageID = stratData->calculatePageID(cx, cy);
				if (cx >= loadxmin && cx <= loadxmax && cy >= loadymin && cy <= loadymax)
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
			int32 x, y;
			Grid2DPageStrategyData* stratData = static_cast<Grid2DPageStrategyData*>(
				p->getParentSection()->getStrategyData());
			stratData->calculateCell(p->getID(), &x, &y);

			Grid2DPageStrategyData* data = static_cast<Grid2DPageStrategyData*>(p->getParentSection()->getStrategyData());

			// Determine our centre point, we'll anchor here
			// Note that world points are initialised to ZERO since only 2 dimensions
			// are updated by the grid data (we could display this grid anywhere)
			Vector2 gridMidPoint;
			Vector3 worldMidPoint = Vector3::ZERO;
			data->getMidPointGridSpace(x, y, gridMidPoint);
			data->convertGridToWorldSpace(gridMidPoint, worldMidPoint);

			sn->setPosition(worldMidPoint);

			Vector2 gridCorners[4];
			Vector3 worldCorners[4];

			data->getCornersGridSpace(x, y, gridCorners);
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

			ColourValue vcol = ColourValue::Green;
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
		int32 x, y;
		stratData->determineGridLocation(gridpos, &x, &y);
		return stratData->calculatePageID(x, y);
		
	}


}

