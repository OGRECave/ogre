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
		TerrainQuadTreeNode* parent, uint16 xoff, uint16 yoff, uint16 size, uint16 lod, uint16 depth)
		: mTerrain(terrain)
		, mParent(parent)
		, mOffsetX(xoff)
		, mOffsetY(yoff)
		, mBoundaryX(xoff + size)
		, mBoundaryY(yoff + size)
		, mSize(size)
		, mBaseLod(lod)
		, mDepth(depth)
		, mNodeWithVertexData(0)
		, mOwnVertexData(false)
	{
		if (terrain->getMinBatchSize() < size)
		{
			uint16 childSize = ((size - 1) * 0.5) + 1;
			uint16 childOff = childSize - 1;
			uint16 childLod = lod - 1; // LOD levels decrease down the tree (higher detail)
			uint16 childDepth = depth + 1;
			// create children
			mChildren[0] = OGRE_NEW TerrainQuadTreeNode(terrain, this, xoff, yoff, childSize, childLod, childDepth);
			mChildren[1] = OGRE_NEW TerrainQuadTreeNode(terrain, this, xoff + childOff, yoff, childSize, childLod, childDepth);
			mChildren[2] = OGRE_NEW TerrainQuadTreeNode(terrain, this, xoff, yoff + childOff, childSize, childLod, childDepth);
			mChildren[3] = OGRE_NEW TerrainQuadTreeNode(terrain, this, xoff + childOff, yoff + childOff, childSize, childLod, childDepth);

			LodLevel* ll = OGRE_NEW LodLevel();
			// non-leaf nodes always render with minBatchSize vertices
			ll->batchSize = terrain->getMinBatchSize();
			ll->maxHeightDelta = 0;
			mLodLevels.push_back(ll);

		}
		else
		{
			// No children
			memset(mChildren, 0, sizeof(TerrainQuadTreeNode*) * 4);

			// this is a leaf node and may have internal LODs of its own
			uint16 ownLod = terrain->getNumLodLevelsPerLeaf();
			// remember, LOD levels are lower indexed when higher detail
			assert(mBaseLod - ownLod + 1 == 0);
			// leaf nodes render from max batch size to min batch size
			uint16 sz = terrain->getMaxBatchSize();

			while (ownLod--)
			{
				LodLevel* ll = OGRE_NEW LodLevel();
				ll->batchSize = sz;
				ll->maxHeightDelta = 0;
				mLodLevels.push_back(ll);
				if (ownLod)
					sz = ((sz - 1) * 0.5) + 1;
			}
			
			assert(sz == terrain->getMinBatchSize());

		}
	}
	//---------------------------------------------------------------------
	TerrainQuadTreeNode::~TerrainQuadTreeNode()
	{
		for (int i = 0; i < 4; ++i)
			OGRE_DELETE mChildren[i];

		for (LodLevelList::iterator i = mLodLevels.begin(); i != mLodLevels.end(); ++i)
			OGRE_DELETE *i;
	}
	//---------------------------------------------------------------------
	bool TerrainQuadTreeNode::isLeaf() const
	{
		return mChildren[0] != 0;
	}
	//---------------------------------------------------------------------
	TerrainQuadTreeNode* TerrainQuadTreeNode::getChild(unsigned short child) const
	{
		if (isLeaf() || child >= 4)
			return 0;

		return mChildren[child];
	}
	//---------------------------------------------------------------------
	TerrainQuadTreeNode* TerrainQuadTreeNode::getParent() const
	{
		return mParent;
	}
	//---------------------------------------------------------------------
	void TerrainQuadTreeNode::prepare()
	{


		if (isLeaf())
		{
			// calculate error terms
			// A = 1 / tan(fovy)    (== 1 for fovy=45)
			// T = 2 * maxPixelError / vertRes
			// CFactor = A / T
			// delta = abs(vertex_height - interpolated_vertex_height)
			// D2 = delta * delta * CFactor * CFactor;
			// Must find max(D2) for any given LOD

			// delta varies by vertex but not by viewport
			// CFactor varies by viewport (fovy and pixel height) but not vertex

			// to avoid precalculating the final value (and therefore making it 
			// viewport-specific), precalculate only max(delta * delta), 
			// since this will be valid for any CFactor. 


			


		}
		else
		{
			for (int i = 0; i < 4; ++i)
				mChildren[i]->prepare();
		}

	}
	//---------------------------------------------------------------------
	void TerrainQuadTreeNode::load()
	{
		if (!isLeaf())
			for (int i = 0; i < 4; ++i)
				mChildren[i]->load();

	}
	//---------------------------------------------------------------------
	void TerrainQuadTreeNode::unload()
	{
		if (!isLeaf())
			for (int i = 0; i < 4; ++i)
				mChildren[i]->unload();

	}
	//---------------------------------------------------------------------
	void TerrainQuadTreeNode::unprepare()
	{
		if (!isLeaf())
			for (int i = 0; i < 4; ++i)
				mChildren[i]->unprepare();

	}
	//---------------------------------------------------------------------
	uint16 TerrainQuadTreeNode::getLodCount() const
	{
		return static_cast<uint16>(mLodLevels.size());
	}
	//---------------------------------------------------------------------
	const TerrainQuadTreeNode::LodLevel* TerrainQuadTreeNode::getLodLevel(uint16 lod)
	{
		assert(lod < mLodLevels.size());

		return mLodLevels[lod];
	}
	//---------------------------------------------------------------------
	void TerrainQuadTreeNode::preDeltaCalculation(const Rect& rect)
	{
		if (rect.left <= mBoundaryX || rect.right > mOffsetX
			|| rect.top <= mBoundaryY || rect.bottom > mOffsetY)
		{
			// relevant to this node (overlaps)

			// if the rect covers the whole node, reset the max height
			// this means that if you recalculate the deltas progressively, end up keeping
			// a max height that's no longer the case (ie more conservative lod), 
			// but that's the price for not recaculating the whole node. If a 
			// complete recalculation is required, just dirty the entire node. (or terrain)

			if (rect.left <= mOffsetX && rect.right > mBoundaryX 
				&& rect.top <= mOffsetY && rect.bottom > mBoundaryY)
			{
				for (LodLevelList::iterator i = mLodLevels.begin(); i != mLodLevels.end(); ++i)
					(*i)->maxHeightDelta = 0.0;
			}

			// pass on to children
			if (!isLeaf())
			{
				for (int i = 0; i < 4; ++i)
					mChildren[i]->preDeltaCalculation(rect);

			}
		}
	}
	//---------------------------------------------------------------------
	void TerrainQuadTreeNode::notifyDelta(uint16 x, uint16 y, uint16 lod, Real delta)
	{
		if (x >= mOffsetX && x < mBoundaryX 
			&& y >= mOffsetY && y < mBoundaryY)
		{
			// within our bounds, check it's our LOD level
			// remember, LODs decrease!
			if (lod <= mBaseLod && lod >= mBaseLod - mLodLevels.size())
			{
				for (LodLevelList::iterator i = mLodLevels.begin(); i != mLodLevels.end(); ++i)
					(*i)->maxHeightDelta = std::max((*i)->maxHeightDelta, delta);
			}

			// pass on to children
			if (!isLeaf())
			{
				for (int i = 0; i < 4; ++i)
					mChildren[i]->notifyDelta(x, y, lod, delta);

			}

		}
	}
	//---------------------------------------------------------------------
	void TerrainQuadTreeNode::assignVertexData(uint16 treeDepthStart, uint16 treeDepthEnd, uint16 resolution)
	{
		assert(treeDepthStart >= mDepth && "Should not be calling this");

		if (mDepth == treeDepthStart)
		{
			// we own this vertex data
			mOwnVertexData = true;

			// TODO - create vertex structure, not using GPU for now
			// think of children's requirements for skirts!

			// pass on to children
			if (!isLeaf() && treeDepthEnd > mDepth)
			{
				for (int i = 0; i < 4; ++i)
					mChildren[i]->useAncestorVertexData(this, treeDepthEnd, resolution);

			}
			


		}
	}
	//---------------------------------------------------------------------
	void TerrainQuadTreeNode::useAncestorVertexData(TerrainQuadTreeNode* owner, uint16 treeDepthEnd, uint16 resolution)
	{

		// TODO
	}



}

