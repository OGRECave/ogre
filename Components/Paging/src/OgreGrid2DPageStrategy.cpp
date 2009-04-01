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
#include "OgreGrid2DPageStrategy.h"
#include "OgreStreamSerialiser.h"
#include "OgreException.h"
#include "OgreCamera.h"
#include "OgrePagedWorldSection.h"

namespace Ogre
{
	//---------------------------------------------------------------------
	const uint32 Grid2DPageStrategyData::msChunkID = StreamSerialiser::makeIdentifier("G2DD");
	const uint16 Grid2DPageStrategyData::msChunkVersion = 1;
	//---------------------------------------------------------------------
	Grid2DPageStrategyData::Grid2DPageStrategyData()
		: PageStrategyData()
		, mMode(G2D_X_Z)
		, mWorldOrigin(Vector3::ZERO)
		, mOrigin(Vector2::ZERO)
		, mCellSize(1000)
		, mLoadRadius(10000)
		, mHoldRadius(12000)
	{
		
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
		mBottomLeft = mOrigin - Vector2(mCellSize * 65536 * 0.5, mCellSize * 65536 * 0.5);
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
	void Grid2DPageStrategyData::setCellSize(Real sz)
	{
		mCellSize = sz;
		mLoadRadiusInCells = mLoadRadius / mCellSize;
		mHoldRadiusInCells = mHoldRadius / mCellSize;
		mBottomLeft = mOrigin - Vector2(mCellSize * 65536 * 0.5, mCellSize * 65536 * 0.5);
	}
	//---------------------------------------------------------------------
	void Grid2DPageStrategyData::setLoadRadius(Real sz)
	{
		mLoadRadius = sz;
		mLoadRadiusInCells = mLoadRadius / mCellSize;
	}
	//---------------------------------------------------------------------
	void Grid2DPageStrategyData::setHoldRadius(Real sz)
	{
		mHoldRadius = sz;
		mHoldRadiusInCells = mHoldRadius / mCellSize;
	}
	//---------------------------------------------------------------------
	void Grid2DPageStrategyData::load(StreamSerialiser& ser)
	{
		const StreamSerialiser::Chunk* chunk = ser.readChunkBegin();
		if (chunk->id != msChunkID)
		{
			ser.undoReadChunk(chunk->id);
			OGRE_EXCEPT(Exception::ERR_INVALID_STATE, 
				"Stream does not contain Grid2DPageStrategyData data!", 
				"Grid2DPageStrategyData::load");
		}

		// Check version
		if (chunk->version > msChunkVersion)
		{
			// skip the rest
			ser.readChunkEnd(chunk->id);
			OGRE_EXCEPT(Exception::ERR_INVALID_STATE, 
				"Grid2DPageStrategyData data version exceeds what this software can read!", 
				"Grid2DPageStrategyData::load");
		}

		uint8 readMode;
		ser.read(&readMode);
		mMode = (Grid2DMode)readMode;

		Vector3 origin;
		ser.read(&origin);
		setOrigin(origin);

		ser.read(&mCellSize);
		ser.read(&mLoadRadius);
		ser.read(&mHoldRadius);

		ser.readChunkEnd(chunk->id);
	}
	//---------------------------------------------------------------------
	void Grid2DPageStrategyData::save(StreamSerialiser& ser)
	{
		ser.writeChunkBegin(msChunkID, msChunkVersion);

		uint8 readMode = (uint8)mMode;
		ser.write(&readMode);

		ser.write(&mWorldOrigin);
		ser.write(&mCellSize);
		ser.write(&mLoadRadius);
		ser.write(&mHoldRadius);

		ser.writeChunkEnd(msChunkID);
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
		// Round UP max, round DOWN min
		uint16 rowmin = frowmin < 0 ? 0 : (uint16)floor(frowmin);
		uint16 rowmax = frowmax > 65535 ? 65535 : (uint16)ceil(frowmax);
		uint16 colmin = fcolmin < 0 ? 0 : (uint16)floor(fcolmin);
		uint16 colmax = fcolmax > 65535 ? 65535 : (uint16)ceil(fcolmax);
		// the inner, active load range
		frowmin = (Real)row - loadRadius;
		frowmax = (Real)row + loadRadius;
		fcolmin = (Real)col - loadRadius;
		fcolmax = (Real)col + loadRadius;
		// Round UP max, round DOWN min
		uint16 loadrowmin = frowmin < 0 ? 0 : (uint16)floor(frowmin);
		uint16 loadrowmax = frowmax > 65535 ? 65535 : (uint16)ceil(frowmax);
		uint16 loadcolmin = fcolmin < 0 ? 0 : (uint16)floor(fcolmin);
		uint16 loadcolmax = fcolmax > 65535 ? 65535 : (uint16)ceil(fcolmax);

		for (uint16 r = rowmin; r <= rowmax; ++r)
		{
			for (uint16 c = colmin; c <= colmax; ++c)
			{
				PageID pageID = calculatePageID(r, c);
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
	PageID Grid2DPageStrategy::calculatePageID(uint16 row, uint16 col)
	{
		return (PageID)row * 65536 + col;
	}


}

