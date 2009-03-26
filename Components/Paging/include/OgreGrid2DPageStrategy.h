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
	/** Specialisation of PageStrategyData for GridPageStrategy
	*/
	class _OgrePagingExport Grid2DPageStrategyData : public PageStrategyData
	{
	protected:
		/// Orientation of the grid
		Grid2DMode mMode;
		/// Origin (world space)
		Vector3 mWorldOrigin;
		/// Origin (grid space)
		Vector2 mOrigin;
		/// Grid cell (page) size
		Real mCellSize;

	public:
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

		/// Load this data from a stream
		void load(StreamSerialiser& stream);
		/// Save this data to a stream
		void save(StreamSerialiser& stream);

		/// Convert a world point to grid space
		virtual void convertWorldToGridSpace(const Vector3& world, Vector2& grid);

	};


	/** Page strategy which loads new pages based on a regular 2D grid.
	@remarks
	*/
	class _OgrePagingExport Grid2DPageStrategy : public PageAlloc
	{
	protected:
	public:
		Grid2DPageStrategy(const String& name, PageManager* manager);

		~Grid2DPageStrategy();

		void frameStart(Real timeSinceLastFrame);
		void frameEnd(Real timeElapsed);
		void notifyCamera(Camera* cam);
		PageStrategyData* createData();
		void destroyData(PageStrategyData* d);
	};

	/*@}*/
	/*@}*/
}




#endif 