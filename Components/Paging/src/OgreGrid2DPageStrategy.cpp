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

namespace Ogre
{
	//---------------------------------------------------------------------
	Grid2DPageStrategyData::Grid2DPageStrategyData()
		: PageStrategyData()
		, mMode(G2D_X_Z)
		, mWorldOrigin(Vector3::ZERO)
		, mOrigin(Vector2::ZERO)
		, mCellSize(10000)
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
	void Grid2DPageStrategyData::load(StreamSerialiser& stream)
	{
		// TODO
	}
	//---------------------------------------------------------------------
	void Grid2DPageStrategyData::save(StreamSerialiser& stream)
	{
		// TODO
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------


}

