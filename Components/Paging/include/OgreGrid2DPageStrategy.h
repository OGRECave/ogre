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

#ifndef __Ogre_Grid2DPageStrategy_H__
#define __Ogre_Grid2DPageStrategy_H__

#include "OgrePagingPrerequisites.h"
#include "OgrePageStrategy.h"
#include "OgreVector2.h"
#include "OgreVector3.h"
#include "OgreQuaternion.h"

namespace Ogre
{
	/** \addtogroup Optional Components
	*  @{
	*/
	/** \addtogroup Paging
	*  Some details on paging component
	*/
	/*@{*/


	/// The 2D grid mode
	enum Grid2DMode
	{
		/// Grid is in the X/Z plane
		G2D_X_Z = 0, 
		/// Grid is in the X/Y plane
		G2D_X_Y = 1, 
		/// Grid is in the Y/Z plane
		G2D_Y_Z = 2
	};
	/** Specialisation of PageStrategyData for GridPageStrategy.
	@remarks
		Structurally this data defines with a grid of pages, with the logical 
		origin in the middle of the entire grid.
		The grid cells are indexed from 0 as a 'centre' slot, supporting both 
		positive and negative values. so (0,0) is the centre cell, (1,0) is the
		cell to the right of the centre, (1,0) is the cell above the centre, (-2,1) 
		is the cell two to the left of the centre and one up, etc. The maximum
		extent of each axis is -32768 to +32767, so in other words enough for
		over 4 billion entries. 
	@par
		To limit the page load requests that are generated to a fixed region, 
		you can set the min and max cell indexes (inclusive)for each direction;
		if a page request would address a cell outside this range it is ignored
		so you don't have the expense of checking for page data that will never
		exist.
	@par
		The data format for this in a file is:<br/>
		<b>Grid2DPageStrategyData (Identifier 'G2DD')</b>\n
		[Version 1]
		<table>
		<tr>
			<td><b>Name</b></td>
			<td><b>Type</b></td>
			<td><b>Description</b></td>
		</tr>
		<tr>
			<td>Grid orientation</td>
			<td>uint8</td>
			<td>The orientation of the grid; XZ = 0, XY = 1, YZ = 2</td>
		</tr>
		<tr>
			<td>Grid origin</td>
			<td>Vector3</td>
			<td>World origin of the grid.</td>
		</tr>
		<tr>
			<td>Grid cell size</td>
			<td>Real</td>
			<td>The size of each cell (page) in the grid</td>
		</tr>
		<tr>
			<td>Grid cell range (minx, maxx, miny, maxy)</td>
			<td>int16 * 4</td>
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

	*/
	class _OgrePagingExport Grid2DPageStrategyData : public PageStrategyData
	{
	protected:
		/// Orientation of the grid
		Grid2DMode mMode;
		/// Origin (world space)
		Vector3 mWorldOrigin;
		/// Origin (grid-aligned world space)
		Vector2 mOrigin;
		/// Grid cell (page) size
		Real mCellSize;
		/// Load radius
		Real mLoadRadius;
		/// Hold radius
		Real mHoldRadius;
		Real mLoadRadiusInCells;
		Real mHoldRadiusInCells;
		int32 mMinCellX;
		int32 mMinCellY;
		int32 mMaxCellX;
		int32 mMaxCellY;

		void updateDerivedMetrics();

	public:
		static const uint32 CHUNK_ID;
		static const uint16 CHUNK_VERSION;

		Grid2DPageStrategyData();
		~Grid2DPageStrategyData();

		/// Set the grid alignment mode
		virtual void setMode(Grid2DMode mode);

		/// Set the grid alignment mode
		virtual Grid2DMode getMode() const { return mMode; }

		/// Set the origin of the grid in world space
		virtual void setOrigin(const Vector3& worldOrigin);
		/// Get the origin of the grid in world space
		virtual const Vector3& getOrigin(const Vector3& worldOrigin) { return mWorldOrigin; }
		/// Set the size of the cells in the grid
		virtual void setCellSize(Real sz);
		/// Get the size of the cells in the grid
		virtual Real getCellSize() const { return mCellSize; }
		/// Set the loading radius 
		virtual void setLoadRadius(Real sz);
		/// Get the loading radius 
		virtual Real getLoadRadius() const { return mLoadRadius; }
		/// Set the Holding radius 
		virtual void setHoldRadius(Real sz);
		/// Get the Holding radius 
		virtual Real getHoldRadius() const { return mHoldRadius; }
		/// Get the load radius as a multiple of cells
		virtual Real getLoadRadiusInCells() { return mLoadRadiusInCells; }
		/// Get the Hold radius as a multiple of cells
		virtual Real getHoldRadiusInCells(){ return mHoldRadiusInCells; }

		/// Set the index range of all cells (values outside this will be ignored)
		virtual void setCellRange(int32 minX, int32 minY, int32 maxX, int32 maxY);
		/// Set the index range of all cells (values outside this will be ignored)
		virtual void setCellRangeMinX(int32 minX);
		/// Set the index range of all cells (values outside this will be ignored)
		virtual void setCellRangeMinY(int32 minY);
		/// Set the index range of all cells (values outside this will be ignored)
		virtual void setCellRangeMaxX(int32 maxX);
		/// Set the index range of all cells (values outside this will be ignored)
		virtual void setCellRangeMaxY(int32 maxY);
		/// get the index range of all cells (values outside this will be ignored)
		virtual int32 getCellRangeMinX() const { return mMinCellX; }
		/// get the index range of all cells (values outside this will be ignored)
		virtual int32 getCellRangeMinY() const { return mMinCellY; }
		/// get the index range of all cells (values outside this will be ignored)
		virtual int32 getCellRangeMaxX() const { return mMaxCellX; }
		/// get the index range of all cells (values outside this will be ignored)
		virtual int32 getCellRangeMaxY() const { return mMaxCellY; }

		/// Load this data from a stream (returns true if successful)
		bool load(StreamSerialiser& stream);
		/// Save this data to a stream
		void save(StreamSerialiser& stream);

		/// Convert a world point to grid space (not relative to origin)
		virtual void convertWorldToGridSpace(const Vector3& world, Vector2& grid);
		/// Convert a grid point to world space - note only 2 axes populated
		virtual void convertGridToWorldSpace(const Vector2& grid, Vector3& world);
		/// Get the (grid space) mid point of a cell
		virtual void getMidPointGridSpace(int32 x, int32 y, Vector2& mid);
		/// Get the (grid space) bottom-left of a cell
		virtual void getBottomLeftGridSpace(int32 x, int32 y, Vector2& bl);
		/** Get the (grid space) corners of a cell.
		@remarks
			Populates pFourPoints in anticlockwise order from the bottom left point.
		*/
		virtual void getCornersGridSpace(int32 x, int32 y, Vector2* pFourPoints);
		
		/// Convert a grid position into a row and column index
		void determineGridLocation(const Vector2& gridpos, int32* x, int32* y);

		PageID calculatePageID(int32 x, int32 y);
		void calculateCell(PageID inPageID, int32* x, int32* y);

	};


	/** Page strategy which loads new pages based on a regular 2D grid.
	@remarks
		The grid can be up to 65536 x 65536 cells in size. PageIDs are generated
		like this: (row * 65536) + col. The grid is centred around the grid origin, such 
		that the boundaries of the cell around that origin are [-CellSize/2, CellSize/2)
	*/
	class _OgrePagingExport Grid2DPageStrategy : public PageStrategy
	{
	public:
		Grid2DPageStrategy(PageManager* manager);

		~Grid2DPageStrategy();

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
