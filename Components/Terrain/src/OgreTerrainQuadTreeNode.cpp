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
#include "OgreTerrainQuadTreeNode.h"
#include "OgreTerrain.h"

namespace Ogre
{
	//---------------------------------------------------------------------
	TerrainQuadTreeNode::TerrainQuadTreeNode(Terrain* terrain, 
		TerrainQuadTreeNode* parent, uint16 xoff, uint16 yoff, uint16 size)
		: mTerrain(terrain)
		, mParent(parent)
		, mOffsetX(xoff)
		, mOffsetY(yoff)
		, mSize(size)
	{
		if (terrain->getMinBatchSize() < size)
		{
			uint16 childSize = ((size - 1) * 0.5) + 1;
			uint16 childOff = childSize - 1;
			// create children
			mChildren[0] = OGRE_NEW TerrainQuadTreeNode(terrain, this, xoff, yoff, childSize);
			mChildren[1] = OGRE_NEW TerrainQuadTreeNode(terrain, this, xoff + childOff, yoff, childSize);
			mChildren[2] = OGRE_NEW TerrainQuadTreeNode(terrain, this, xoff, yoff + childOff, childSize);
			mChildren[3] = OGRE_NEW TerrainQuadTreeNode(terrain, this, xoff + childOff, yoff + childOff, childSize);

		}
		else
		{
			memset(mChildren, 0, sizeof(TerrainQuadTreeNode*) * 4);
		}
	}
	//---------------------------------------------------------------------
	TerrainQuadTreeNode::~TerrainQuadTreeNode()
	{
		for (int i = 0; i < 4; ++i)
			OGRE_DELETE mChildren[i];
	}
	//---------------------------------------------------------------------
}

