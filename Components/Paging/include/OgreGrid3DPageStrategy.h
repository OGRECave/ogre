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

#ifndef __Ogre_Grid3DPageStrategy_H__
#define __Ogre_Grid3DPageStrategy_H__

#include "OgrePagingPrerequisites.h"
#include "OgrePageStrategy.h"
#include "OgreVector3.h"

namespace Ogre
{
	/** \addtogroup Optional Components
	*  @{
	*/
	/** \addtogroup Paging
	*  Some details on paging component
	*/
	/*@{*/


    /** Specialisation of PageStrategyData for Grid3DPageStrategy.
	@remarks
		Structurally this data defines with a 3D grid of pages, with the logical 
		origin in the center of the entire grid.
		The grid cells are indexed from 0 as a 'centre' slot, supporting both 
		positive and negative values. so (0,0,0) is the centre cell, (1,0,0) is the
		cell to the right of the centre, (0,0,1) is the cell in front of the centre,
        (0,1,0) the cell above the center, etc.
        The maximum extent of each axis is -512 to +511, so in other words enough for
		over 2 billion different entries. 
	@par
		To limit the page load requests that are generated to a fixed region, 
		you can set the min and max cell indexes (inclusive)for each direction;
		if a page request would address a cell outside this range it is ignored
		so you don't have the expense of checking for page data that will never
		exist.
    @par
		The data format for this in a file is:<br/>
		<b>Grid3DPageStrategyData (Identifier 'G3DD')</b>\n
		[Version 1]
		<table>
		<tr>
			<td><b>Name</b></td>
			<td><b>Type</b></td>
			<td><b>Description</b></td>
		</tr>
		<tr>
			<td>Grid origin</td>
			<td>Vector3</td>
			<td>World origin of the grid.</td>
		</tr>
		<tr>
			<td>Grid cell size</td>
			<td>Vector3</td>
			<td>The size of each cell (page) in the grid</td>
		</tr>
		<tr>
			<td>Grid cell range (minx, maxx, miny, maxy, minz, maxz)</td>
			<td>int16 * 6</td>
			<td>The extents of the world in cell indexes</td>
		</tr>
		<tr>
			<td>Load radius</td>
			<td>Real</td>
			<td>The outer radius at which new pages should start loading</td>
		</tr>
		<tr>
			<td>Hold radius</td>
			<td>Real</td>
			<td>The radius at which existing pages should be held if already loaded 
				but not actively loaded (should be larger than Load radius)</td>
		</tr>
		</table>

        @sa Grid2DPageStrategyData

	*/
	class _OgrePagingExport Grid3DPageStrategyData : public PageStrategyData
	{
	protected:
		/// Origin (world space)
		Vector3 mWorldOrigin;
		/// Origin (grid-aligned world space)
		Vector3 mOrigin;
		/// Grid cell (page) size
		Vector3 mCellSize;
		/// Load radius
		Real mLoadRadius;
		/// Hold radius
		Real mHoldRadius;
		int32 mMinCellX;
		int32 mMinCellY;
		int32 mMinCellZ;
		int32 mMaxCellX;
		int32 mMaxCellY;
		int32 mMaxCellZ;

	public:
		static const uint32 CHUNK_ID;
		static const uint16 CHUNK_VERSION;

		Grid3DPageStrategyData();
		~Grid3DPageStrategyData();

		/// Set the origin of the grid in world space
		virtual void setOrigin(const Vector3& worldOrigin);
		/// Get the origin of the grid in world space
		virtual const Vector3& getOrigin(const Vector3& worldOrigin) { return mWorldOrigin; }
		/// Set the size of the cells in the grid
		virtual void setCellSize(const Vector3& sz);
		/// Get the size of the cells in the grid
		virtual Vector3 getCellSize() const { return mCellSize; }
		/// Set the loading radius 
		virtual void setLoadRadius(Real sz);
		/// Get the loading radius 
		virtual Real getLoadRadius() const { return mLoadRadius; }
		/// Set the Holding radius 
		virtual void setHoldRadius(Real sz);
		/// Get the Holding radius 
		virtual Real getHoldRadius() const { return mHoldRadius; }

		/// Set the index range of all cells (values outside this will be ignored)
		virtual void setCellRange(int32 minX, int32 minY, int32 minZ, int32 maxX, int32 maxY, int32 maxZ);
		/// Set the index range of all cells (values outside this will be ignored)
		virtual void setCellRangeMinX(int32 minX);
		/// Set the index range of all cells (values outside this will be ignored)
		virtual void setCellRangeMinY(int32 minY);
		/// Set the index range of all cells (values outside this will be ignored)
		virtual void setCellRangeMinZ(int32 minZ);
		/// Set the index range of all cells (values outside this will be ignored)
		virtual void setCellRangeMaxX(int32 maxX);
		/// Set the index range of all cells (values outside this will be ignored)
		virtual void setCellRangeMaxY(int32 maxY);
		/// get the index range of all cells (values outside this will be ignored)
		virtual void setCellRangeMaxZ(int32 maxZ);
		/// get the index range of all cells (values outside this will be ignored)
		virtual int32 getCellRangeMinX() const { return mMinCellX; }
		/// get the index range of all cells (values outside this will be ignored)
		virtual int32 getCellRangeMinY() const { return mMinCellY; }
		/// get the index range of all cells (values outside this will be ignored)
		virtual int32 getCellRangeMinZ() const { return mMinCellZ; }
		/// get the index range of all cells (values outside this will be ignored)
		virtual int32 getCellRangeMaxX() const { return mMaxCellX; }
		/// get the index range of all cells (values outside this will be ignored)
		virtual int32 getCellRangeMaxY() const { return mMaxCellY; }
		/// get the index range of all cells (values outside this will be ignored)
		virtual int32 getCellRangeMaxZ() const { return mMaxCellZ; }

		/// Load this data from a stream (returns true if successful)
		bool load(StreamSerialiser& stream);
		/// Save this data to a stream
		void save(StreamSerialiser& stream);

		virtual void getMidPointGridSpace(int32 x, int32 y, int32 z, Vector3& mid);
		/// Get the (grid space) bottom-left of a cell
		virtual void getBottomLeftGridSpace(int32 x, int32 y, int z, Vector3& bl);
		/** Get the (grid space) corners of a cell.
		@remarks
			Populates pEightPoints in anticlockwise order from the bottom left point.
		*/
		virtual void getCornersGridSpace(int32 x, int32 y, int32 z, Vector3* pEightPoints);
		
		/// Convert a grid position into a row and column index
		void determineGridLocation(const Vector3& gridpos, int32* x, int32* y, int32* z);

		PageID calculatePageID(int32 x, int32 y, int32 z);
		void calculateCell(PageID inPageID, int32* x, int32* y, int32* z);
	};


	/** Page strategy which loads new pages based on a regular 3D grid.
	@remarks
		The grid can be up to 1024 x 1024 x 1024 cells in size. PageIDs are generated
		like this: (slice*1024 + row) * 1024 + col. The grid is centred around the grid origin, such 
		that the boundaries of the cell around that origin are [-CellSize/2, CellSize/2)
	*/
	class _OgrePagingExport Grid3DPageStrategy : public PageStrategy
	{
	public:
		Grid3DPageStrategy(PageManager* manager);

		~Grid3DPageStrategy();

		// Overridden members
		void notifyCamera(Camera* cam, PagedWorldSection* section);
		PageStrategyData* createData();
		void destroyData(PageStrategyData* d);
		void updateDebugDisplay(Page* p, SceneNode* sn);
		PageID getPageID(const Vector3& worldPos, PagedWorldSection* section);
	};

	/*@}*/
	/*@}*/
}

#endif
