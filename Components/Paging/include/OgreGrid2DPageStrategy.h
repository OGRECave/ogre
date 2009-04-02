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
		origin in the middle of the entire grid, but with an indexing system 
		of rows and columns which starts in the bottom left and works to the right
		and up. Therefore the page at row 0 and column 0 is in the bottom left, but
		this is actually at -(CellSize * 65536 / 2) in terms of coordinates from 
		the origin. It's done this way because most people want to specify the 
		'origin' to be the middle, stretching in all directions of the plane, 
		but the row/column indexes are always positive for simplicity.
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
			<td>The orientation of the grid; XY = 0, XY = 1, YZ = 2</td>
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
		/// Bottom-left position (grid-aligned world space)
		Vector2 mBottomLeft;
		/// Grid cell (page) size
		Real mCellSize;
		/// Load radius
		Real mLoadRadius;
		/// Hold radius
		Real mHoldRadius;
		Real mLoadRadiusInCells;
		Real mHoldRadiusInCells;

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

		/// Load this data from a stream (returns true if successful)
		bool load(StreamSerialiser& stream);
		/// Save this data to a stream
		void save(StreamSerialiser& stream);

		/// Convert a world point to grid space
		virtual void convertWorldToGridSpace(const Vector3& world, Vector2& grid);
		
		/// Convert a grid position into a row and column index
		void determineGridLocation(const Vector2& gridpos, uint16* row, uint16* col);

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
	protected:
		PageID calculatePageID(uint16 row, uint16 col);
	};

	/*@}*/
	/*@}*/
}




#endif 