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
#include "OgreVertexIndexData.h"
#include "OgreDefaultHardwareBufferManager.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreRenderSystemCapabilities.h"

namespace Ogre
{
	//---------------------------------------------------------------------
	TerrainQuadTreeNode::TerrainQuadTreeNode(Terrain* terrain, 
		TerrainQuadTreeNode* parent, uint16 xoff, uint16 yoff, uint16 size, 
		uint16 lod, uint16 depth, uint16 quadrant)
		: mTerrain(terrain)
		, mParent(parent)
		, mOffsetX(xoff)
		, mOffsetY(yoff)
		, mBoundaryX(xoff + size)
		, mBoundaryY(yoff + size)
		, mSize(size)
		, mBaseLod(lod)
		, mDepth(depth)
		, mQuadrant(quadrant)
		, mNodeWithVertexData(0)
		, mVertexDataRecord(0)
		, mMovable(0)
		, mRend(0)
	{
		if (terrain->getMaxBatchSize() < size)
		{
			uint16 childSize = ((size - 1) * 0.5) + 1;
			uint16 childOff = childSize - 1;
			uint16 childLod = lod - 1; // LOD levels decrease down the tree (higher detail)
			uint16 childDepth = depth + 1;
			// create children
			mChildren[0] = OGRE_NEW TerrainQuadTreeNode(terrain, this, xoff, yoff, childSize, childLod, childDepth, 0);
			mChildren[1] = OGRE_NEW TerrainQuadTreeNode(terrain, this, xoff + childOff, yoff, childSize, childLod, childDepth, 1);
			mChildren[2] = OGRE_NEW TerrainQuadTreeNode(terrain, this, xoff, yoff + childOff, childSize, childLod, childDepth, 2);
			mChildren[3] = OGRE_NEW TerrainQuadTreeNode(terrain, this, xoff + childOff, yoff + childOff, childSize, childLod, childDepth, 3);

			LodLevel* ll = OGRE_NEW LodLevel();
			// non-leaf nodes always render with minBatchSize vertices
			ll->batchSize = terrain->getMinBatchSize();
			ll->maxHeightDelta = 0;
			ll->calcMaxHeightDelta = 0;
			mLodLevels.push_back(ll);

		}
		else
		{
			// No children
			memset(mChildren, 0, sizeof(TerrainQuadTreeNode*) * 4);

			// this is a leaf node and may have internal LODs of its own
			uint16 ownLod = terrain->getNumLodLevelsPerLeaf();
			assert (lod == (ownLod - 1) && "The lod passed in should reflect the number of "
				"lods in a leaf");
			// leaf nodes always have a base LOD of 0, because they're always handling
			// the highest level of detail
			mBaseLod = 0;
			// leaf nodes render from max batch size to min batch size
			uint16 sz = terrain->getMaxBatchSize();

			while (ownLod--)
			{
				LodLevel* ll = OGRE_NEW LodLevel();
				ll->batchSize = sz;
				ll->maxHeightDelta = 0;
				ll->calcMaxHeightDelta = 0;
				mLodLevels.push_back(ll);
				if (ownLod)
					sz = ((sz - 1) * 0.5) + 1;
			}
			
			assert(sz == terrain->getMinBatchSize());

		}
		
		// Local centre calculation
		// Because of pow2 + 1 there is always a middle point
		uint16 midoffset = (size - 1) / 2;
		uint16 midpointx = mOffsetX + midoffset;
		uint16 midpointy = mOffsetY + midoffset;
		// derive the local centre, but give it a height of 0
		// TODO - what if we actually centred this at the terrain height at this point?
		// would this be better?
		mTerrain->getPoint(midpointx, midpointy, 0, &mLocalCentre);

		mMovable = OGRE_NEW Movable(this);
		mTerrain->_getRootSceneNode()->createChildSceneNode(mLocalCentre)->attachObject(mMovable);
		mRend = OGRE_NEW Rend(this);
		
	}
	//---------------------------------------------------------------------
	TerrainQuadTreeNode::~TerrainQuadTreeNode()
	{
		OGRE_DELETE mMovable;
		mMovable = 0;
		OGRE_DELETE mRend;
		mRend = 0;

		for (int i = 0; i < 4; ++i)
			OGRE_DELETE mChildren[i];

		for (LodLevelList::iterator i = mLodLevels.begin(); i != mLodLevels.end(); ++i)
			OGRE_DELETE *i;

		destroyCpuVertexData();
		destroyGpuVertexData();
		OGRE_DELETE mVertexDataRecord;
	}
	//---------------------------------------------------------------------
	bool TerrainQuadTreeNode::isLeaf() const
	{
		return mChildren[0] == 0;
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
	Terrain* TerrainQuadTreeNode::getTerrain() const
	{
		return mTerrain;
	}
	//---------------------------------------------------------------------
	void TerrainQuadTreeNode::prepare()
	{


		if (isLeaf())
		{


		

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
		createGpuVertexData();
		createGpuIndexData();

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

		destroyGpuVertexData();

	}
	//---------------------------------------------------------------------
	void TerrainQuadTreeNode::unprepare()
	{
		if (!isLeaf())
			for (int i = 0; i < 4; ++i)
				mChildren[i]->unprepare();

		destroyCpuVertexData();
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

			// Note we use the 'calc' field here to avoid interfering with any
			// ongoing LOD calculations (this can be in the background)

			if (rect.left <= mOffsetX && rect.right > mBoundaryX 
				&& rect.top <= mOffsetY && rect.bottom > mBoundaryY)
			{
				for (LodLevelList::iterator i = mLodLevels.begin(); i != mLodLevels.end(); ++i)
					(*i)->calcMaxHeightDelta = 0.0;
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
			if (lod >= mBaseLod && lod < mBaseLod + mLodLevels.size())
			{
				// Apply the delta to all LODs equal or lower detail to lod
				LodLevelList::iterator i = mLodLevels.begin();
				std::advance(i, lod - mBaseLod);
				(*i)->calcMaxHeightDelta = std::max((*i)->calcMaxHeightDelta, delta);
			}

			// pass on to children
			if (!isLeaf())
			{
				LodLevel* lowestOwnLod = *mLodLevels.rbegin();

				for (int i = 0; i < 4; ++i)
				{
					mChildren[i]->notifyDelta(x, y, lod, delta);
				}

			}

		}
	}
	//---------------------------------------------------------------------
	void TerrainQuadTreeNode::postDeltaCalculation(const Rect& rect)
	{
		if (rect.left <= mBoundaryX || rect.right > mOffsetX
			|| rect.top <= mBoundaryY || rect.bottom > mOffsetY)
		{
			// relevant to this node (overlaps)

			// each non-leaf node should know which of its children transitions
			// to the lower LOD level last, because this is the one which controls
			// when the parent takes over
			if (!isLeaf())
			{
				Real maxChildDelta = -1;
				TerrainQuadTreeNode* childWithMaxHeightDelta = 0;
				for (int i = 0; i < 4; ++i)
				{
					TerrainQuadTreeNode* child = mChildren[i];

					child->postDeltaCalculation(rect);

					Real childDelta = child->getLodLevel(child->getLodCount()-1)->calcMaxHeightDelta;
					if (childDelta > maxChildDelta)
					{
						childWithMaxHeightDelta = child;
						maxChildDelta = childDelta;
					}

				}

				// make sure that our highest delta value is greater than all children's
				// otherwise we could have some crossover problems
				// for a non-leaf, there is only one LOD level
				mLodLevels[0]->calcMaxHeightDelta = std::max(mLodLevels[0]->calcMaxHeightDelta, maxChildDelta);
				mChildWithMaxHeightDelta = childWithMaxHeightDelta;

			}
			else
			{
				// make sure own LOD levels delta values ascend
				for (size_t i = 0; i < mLodLevels.size() - 1; ++i)
				{
					// the next LOD after this one should have a higher delta
					// otherwise it won't come into affect further back like it should!
					mLodLevels[i+1]->calcMaxHeightDelta = 
						std::max(mLodLevels[i+1]->calcMaxHeightDelta, 
							mLodLevels[i]->calcMaxHeightDelta);
				}

			}
		}

	}
	//---------------------------------------------------------------------
	void TerrainQuadTreeNode::finaliseDeltaValues(const Rect& rect)
	{
		if (rect.left <= mBoundaryX || rect.right > mOffsetX
			|| rect.top <= mBoundaryY || rect.bottom > mOffsetY)
		{
			// relevant to this node (overlaps)

			// Children
			if (!isLeaf())
			{
				for (int i = 0; i < 4; ++i)
				{
					TerrainQuadTreeNode* child = mChildren[i];
					child->finaliseDeltaValues(rect);
				}

			}

			// Self
			for (LodLevelList::iterator i = mLodLevels.begin(); i != mLodLevels.end(); ++i)
			{
				// copy from 'calc' area to runtime value
				(*i)->maxHeightDelta = (*i)->calcMaxHeightDelta;
			}

		}

	}
	//---------------------------------------------------------------------
	void TerrainQuadTreeNode::assignVertexData(uint16 treeDepthStart, 
		uint16 treeDepthEnd, uint16 resolution, uint sz)
	{
		assert(treeDepthStart >= mDepth && "Should not be calling this");

		if (mDepth == treeDepthStart)
		{
			// we own this vertex data
			mNodeWithVertexData = this;
			mVertexDataRecord = OGRE_NEW VertexDataRecord(resolution, sz, treeDepthEnd - treeDepthStart);

			createCpuVertexData();
			createCpuIndexData();

			// pass on to children
			if (!isLeaf() && treeDepthEnd > (mDepth + 1)) // treeDepthEnd is exclusive, and this is children
			{
				for (int i = 0; i < 4; ++i)
					mChildren[i]->useAncestorVertexData(this, treeDepthEnd, resolution);

			}
		}
		else
		{
			assert(!isLeaf() && "No more levels below this!");

			for (int i = 0; i < 4; ++i)
				mChildren[i]->assignVertexData(treeDepthStart, treeDepthEnd, resolution, sz);
			
		}

	}
	//---------------------------------------------------------------------
	void TerrainQuadTreeNode::useAncestorVertexData(TerrainQuadTreeNode* owner, uint16 treeDepthEnd, uint16 resolution)
	{
		mNodeWithVertexData = owner; 
		mVertexDataRecord = 0;

		if (!isLeaf() && treeDepthEnd > (mDepth + 1)) // treeDepthEnd is exclusive, and this is children
		{
			for (int i = 0; i < 4; ++i)
				mChildren[i]->useAncestorVertexData(owner, treeDepthEnd, resolution);

		}
		createCpuIndexData();
	}
	//---------------------------------------------------------------------
	const TerrainQuadTreeNode::VertexDataRecord* TerrainQuadTreeNode::getVertexDataRecord() const
	{
		return mNodeWithVertexData ? mNodeWithVertexData->mVertexDataRecord : 0;
	}
	//---------------------------------------------------------------------
	void TerrainQuadTreeNode::createCpuVertexData()
	{
		if (mVertexDataRecord)
		{
			destroyCpuVertexData();

			// create vertex structure, not using GPU for now (these are CPU structures)
			VertexDeclaration* dcl = OGRE_NEW VertexDeclaration();
			VertexBufferBinding* bufbind = OGRE_NEW VertexBufferBinding();

			mVertexDataRecord->cpuVertexData = OGRE_NEW VertexData(dcl, bufbind);

			// Vertex declaration
			// TODO: consider vertex compression
			size_t offset = 0;
			// POSITION 
			// float3(x, y, z)
			offset += dcl->addElement(0, offset, VET_FLOAT3, VES_POSITION).getSize();
			// UV0
			// float4(u, v, delta, deltaLODthreshold)
			offset += dcl->addElement(0, offset, VET_FLOAT4, VES_TEXTURE_COORDINATES).getSize();

			// Calculate number of vertices
			// Base geometry size * size
			size_t baseNumVerts = Math::Sqr(mVertexDataRecord->size);
			size_t numVerts = baseNumVerts;
			// Now add space for skirts
			// Skirts will be rendered as copies of the edge vertices translated downwards
			// Some people use one big fan with only 3 vertices at the bottom, 
			// but this requires creating them much bigger that necessary, meaning
			// more unnecessary overdraw, so we'll use more vertices 
			// You need 2^levels + 1 rows of full resolution (max 129) vertex copies, plus
			// the same number of columns. There are common vertices at intersections
			uint16 levels = mVertexDataRecord->treeLevels;
			mVertexDataRecord->numSkirtRowsCols = (Math::Pow(2, levels) + 1);
			mVertexDataRecord->skirtRowColSkip = (mVertexDataRecord->size - 1) / (mVertexDataRecord->numSkirtRowsCols - 1);
			numVerts += mVertexDataRecord->size * mVertexDataRecord->numSkirtRowsCols;
			numVerts += mVertexDataRecord->size * mVertexDataRecord->numSkirtRowsCols;

			// manually create CPU-side buffer
			HardwareVertexBufferSharedPtr vbuf(
				OGRE_NEW DefaultHardwareVertexBuffer(dcl->getVertexSize(0), numVerts, HardwareBuffer::HBU_STATIC_WRITE_ONLY));

			mVertexDataRecord->cpuVertexData->vertexStart = 0;
			mVertexDataRecord->cpuVertexData->vertexCount = numVerts;
			
			Rect updateRect(mOffsetX, mOffsetY, mBoundaryX, mBoundaryY);
			updateVertexBuffer(vbuf, updateRect);
			bufbind->setBinding(0, vbuf);
		}
	}
	//----------------------------------------------------------------------
	void TerrainQuadTreeNode::updateVertexBuffer(HardwareVertexBufferSharedPtr& vbuf, const Rect& rect)
	{
		// potentially reset our bounds depending on coverage of the update
		resetBounds(rect);

		long destOffsetX = rect.left - mOffsetX;
		long destOffsetY = rect.top - mOffsetY;
		// Fill the buffers
		// Main data
		uint16 inc = (mTerrain->getSize()-1) / (mVertexDataRecord->resolution-1);
		
		HardwareBuffer::LockOptions lockMode;
		if (destOffsetX || destOffsetY || rect.right - rect.left < mSize
			|| rect.bottom - rect.top < mSize)
		{
			lockMode = HardwareBuffer::HBL_NORMAL;
		}
		else
		{
			lockMode = HardwareBuffer::HBL_DISCARD;
		}

		Real uvScale = 1.0 / (mTerrain->getSize() - 1);
		const float* pBaseHeight = mTerrain->getHeightData(rect.left, rect.top);
		const float* pBaseDelta = mTerrain->getDeltaData(rect.left, rect.top);
		uint16 rowskip = mTerrain->getSize() * inc;
		uint16 destRowSkip = mVertexDataRecord->size * vbuf->getVertexSize();
		Vector3 pos;
		unsigned char* pRootBuf = static_cast<unsigned char*>(vbuf->lock(lockMode));
		unsigned char* pRowBuf = pRootBuf;
		// skip dest buffer in by left/top
		pRowBuf += destOffsetY * destRowSkip + destOffsetX * vbuf->getVertexSize();
		for (uint16 y = rect.top; y < rect.bottom; y += inc)
		{
			const float* pHeight = pBaseHeight;
			const float* pDelta = pBaseDelta;
			float* pBuf = static_cast<float*>(static_cast<void*>(pRowBuf));
			for (uint16 x = rect.left; x < rect.right; x += inc)
			{
				mTerrain->getPoint(x, y, *pHeight, &pos);
				// relative to local centre
				pos -= mLocalCentre;

				pHeight += inc;

				*pBuf++ = pos.x;
				*pBuf++ = pos.y;
				*pBuf++ = pos.z;

				// UVs - base UVs vary from 0 to 1, all other values
				// will be derived using scalings
				*pBuf++ = x * uvScale;
				*pBuf++ = 1.0 - (y * uvScale);
				// delta
				*pBuf++ = *pDelta;
				pDelta += inc;
				// delta LOD threshold
				// we want delta to apply to LODs no higher than this value
				// at runtime this will be combined with a per-renderable parameter
				// to ensure we only apply morph to the correct LOD
				*pBuf++ = (float)mTerrain->getLODLevelWhenVertexEliminated(x, y) - 1.0f;
				// Update bounds
				mergeIntoBounds(x, y, pos);

				
			}
			pBaseHeight += rowskip;
			pBaseDelta += rowskip;
			pRowBuf += destRowSkip;

		}


		// skirt spacing based on top-level resolution (* inc to cope with resolution which is not the max)
		uint16 skirtSpacing = mVertexDataRecord->skirtRowColSkip * inc;
		Vector3 skirtOffset;
		mTerrain->getVector(0, 0, -mTerrain->getSkirtSize(), &skirtOffset);

		// clamp to skirt spacing (round up)
		long skirtStartX = rect.left;
		if (skirtStartX % skirtSpacing)
			skirtStartX += skirtSpacing - (skirtStartX % skirtSpacing);
		long skirtStartY = rect.top;
		if (skirtStartY % skirtSpacing)
			skirtStartY += skirtSpacing - (skirtStartY % skirtSpacing);
		
		// skirt rows
		pBaseHeight = mTerrain->getHeightData(skirtStartX, skirtStartY);
		// position dest buffer just after the main vertex data
		pRowBuf = pRootBuf + vbuf->getVertexSize() 
			* mVertexDataRecord->size * mVertexDataRecord->size;
		// move it onwards to skip the skirts we don't need to update
		pRowBuf += destRowSkip * (skirtStartY - mOffsetY) / skirtSpacing;
		pRowBuf += vbuf->getVertexSize() * (skirtStartX - mOffsetX) / skirtSpacing;
		for (uint16 y = skirtStartY; y < rect.bottom; y += skirtSpacing)
		{
			const float* pHeight = pBaseHeight;
			float* pBuf = static_cast<float*>(static_cast<void*>(pRowBuf));
			for (uint16 x = skirtStartX; x < rect.right; x += inc)
			{
				mTerrain->getPoint(x, y, *pHeight, &pos);
				// relative to local centre
				pos -= mLocalCentre;
				pHeight += inc;

				pos += skirtOffset;

				*pBuf++ = pos.x;
				*pBuf++ = pos.y;
				*pBuf++ = pos.z;

				// UVs - same as base
				*pBuf++ = x * uvScale;
				*pBuf++ = 1.0 - (y * uvScale);
				// delta (none)
				*pBuf++ = 0; 
				// delta threshold (irrelevant)
				*pBuf++ = 99;
			}
			pBaseHeight += mTerrain->getSize() * skirtSpacing;
			pRowBuf += destRowSkip;
		}
		// skirt cols
		// position dest buffer just after the main vertex data and skirt rows
		pRowBuf = pRootBuf + vbuf->getVertexSize() 
			* mVertexDataRecord->size * mVertexDataRecord->size;
		// skip the row skirts
		pRowBuf += mVertexDataRecord->numSkirtRowsCols * mVertexDataRecord->size * vbuf->getVertexSize();
		// move it onwards to skip the skirts we don't need to update
		pRowBuf += destRowSkip * (skirtStartX - mOffsetX) / skirtSpacing;
		pRowBuf += vbuf->getVertexSize() * (skirtStartY - mOffsetY) / skirtSpacing;
		
		for (uint16 x = skirtStartX; x < rect.right; x += skirtSpacing)
		{
			float* pBuf = static_cast<float*>(static_cast<void*>(pRowBuf));
			for (uint16 y = skirtStartY; y < rect.bottom; y += inc)
			{
				mTerrain->getPoint(x, y, mTerrain->getHeightAtPoint(x, y), &pos);
				// relative to local centre
				pos -= mLocalCentre;
				pos += skirtOffset;

				*pBuf++ = pos.x;
				*pBuf++ = pos.y;
				*pBuf++ = pos.z;

				// UVs - same as base
				*pBuf++ = x * uvScale;
				*pBuf++ = 1.0 - (y * uvScale);
				// delta (none)
				*pBuf++ = 0; 
				// delta threshold (irrelevant)
				*pBuf++ = 99;
			}
			pRowBuf += destRowSkip;
		}

		vbuf->unlock();
		
		mVertexDataRecord->gpuVertexDataDirty = true;

	}
	//---------------------------------------------------------------------
	uint16 TerrainQuadTreeNode::calcSkirtVertexIndex(uint16 mainIndex, bool isCol)
	{
		const VertexDataRecord* vdr = getVertexDataRecord();
		// row / col in main vertex resolution
		uint16 row = mainIndex / vdr->size;
		uint16 col = mainIndex % vdr->size;
		
		// skrits are after main vertices, so skip them
		uint16 base = vdr->size * vdr->size;

		// The layout in vertex data is:
		// 1. row skirts
		//    numSkirtRowsCols rows of resolution vertices each
		// 2. column skirts
		//    numSkirtRowsCols cols of resolution vertices each

		// No offsets used here, this is an index into the current vertex data, 
		// which is already relative
		if (isCol)
		{
			uint16 skirtNum = col / vdr->skirtRowColSkip;
			uint16 colbase = vdr->numSkirtRowsCols * vdr->size;
			return base + colbase + vdr->size * skirtNum + row;
		}
		else
		{
			uint16 skirtNum = row / vdr->skirtRowColSkip;
			return base + vdr->size * skirtNum + col;
		}
		
	}
	//---------------------------------------------------------------------
	void TerrainQuadTreeNode::destroyCpuVertexData()
	{
		if (mVertexDataRecord && mVertexDataRecord->cpuVertexData)
		{
			// delete the bindings and declaration manually since not from a buf mgr
			OGRE_DELETE mVertexDataRecord->cpuVertexData->vertexDeclaration;
			mVertexDataRecord->cpuVertexData->vertexDeclaration = 0;

			OGRE_DELETE mVertexDataRecord->cpuVertexData->vertexBufferBinding;
			mVertexDataRecord->cpuVertexData->vertexBufferBinding = 0;

			OGRE_DELETE mVertexDataRecord->cpuVertexData;
			mVertexDataRecord->cpuVertexData = 0;
		}


	}
	//---------------------------------------------------------------------
	void TerrainQuadTreeNode::createCpuIndexData()
	{
		for (size_t lod = 0; lod < mLodLevels.size(); ++lod)
		{
			LodLevel* ll = mLodLevels[lod];

			OGRE_DELETE ll->cpuIndexData;

			ll->cpuIndexData = OGRE_NEW IndexData();

			createTriangleStripBuffer(ll->batchSize, ll->cpuIndexData);
			
		}
	}
	//---------------------------------------------------------------------
	void TerrainQuadTreeNode::createTriangleStripBuffer(uint16 batchSize, IndexData* destData)
	{
		/* For even / odd tri strip rows, triangles are this shape:
		6---7---8
		| \ | \ |
		3---4---5
		| / | / |
		0---1---2
		Note how vertex rows count upwards. In order to match up the anti-clockwise
		winding and this upward transitioning list, we need to start from the
		right hand side. So we get (2,5,1,4,0,3) etc on even lines (right-left)
		and (3,6,4,7,5,8) etc on odd lines (left-right). At the turn, we emit the end index 
		twice, this forms a degenerate triangle, which lets us turn without any artefacts. 
		So the full list in this simple case is (2,5,1,4,0,3,3,6,4,7,5,8)

		Skirts are part of the same strip, so after finishing on 8, where sX is
		 the skirt vertex corresponding to main vertex X, we go
		 anticlockwise around the edge, (s8,7,s7,6,s6) to do the top skirt, 
		then (3,s3,0,s0),(1,s1,2,s2),(5,s5,8,s8) to finish the left, bottom, and
		 right skirts respectively.
		*/

		// to issue a complete row, it takes issuing the upper and lower row
		// and one extra index, which is the degenerate triangle and also turning
		// around the winding
		const VertexDataRecord* vdr = getVertexDataRecord();
		size_t mainIndexesPerRow = batchSize * 2 + 1;
		size_t numRows = batchSize - 1;
		size_t mainIndexCount = mainIndexesPerRow * numRows;
		// skirts share edges, so they take 1 less row per side than batchSize, 
		// but with 2 extra at the end (repeated) to finish the strip
		// * 2 for the vertical line, * 4 for the sides, +2 to finish
		size_t skirtIndexCount = (batchSize - 1) * 2 * 4 + 2;
		
		destData->indexStart = 0;
		destData->indexCount = mainIndexCount + skirtIndexCount;
		destData->indexBuffer.bind(OGRE_NEW DefaultHardwareIndexBuffer(
			HardwareIndexBuffer::IT_16BIT, destData->indexCount, HardwareBuffer::HBU_STATIC)); 
		
		uint16* pI = static_cast<uint16*>(destData->indexBuffer->lock(HardwareBuffer::HBL_DISCARD));
		uint16* basepI = pI;
		
		// Ratio of the main terrain resolution in relation to this vertex data resolution
		size_t resolutionRatio = (mTerrain->getSize() - 1) / (vdr->resolution - 1);
		// At what frequency do we sample the vertex data we're using?
		// mSize is the coverage in terms of the original terrain data (not split to fit in 16-bit)
		size_t vertexIncrement = (mSize-1) / (batchSize-1);
		// however, the vertex data we're referencing may not be at the full resolution anyway
		vertexIncrement /= resolutionRatio;
		size_t rowSize = vdr->size * vertexIncrement;

		// Start on the right
		uint16 currentVertex = (batchSize - 1) * vertexIncrement;
		// but, our quad area might not start at 0 in this vertex data
		// offsets are at main terrain resolution, remember
		uint16 columnStart = (mOffsetX - mNodeWithVertexData->mOffsetX) / resolutionRatio;
		uint16 rowStart = ((mOffsetY - mNodeWithVertexData->mOffsetY) / resolutionRatio);
		currentVertex += rowStart * vdr->size + columnStart;
		bool rightToLeft = true;
		for (uint16 r = 0; r < numRows; ++r)
		{
			for (uint16 c = 0; c < batchSize; ++c)
			{
								
				*pI++ = currentVertex;
				*pI++ = currentVertex + rowSize;
				
				// don't increment / decrement at a border, keep this vertex for next
				// row as we 'snake' across the tile
				if (c+1 < batchSize)
				{
					currentVertex = rightToLeft ? 
						currentVertex - vertexIncrement : currentVertex + vertexIncrement;
				}				
			}
			rightToLeft = !rightToLeft;
			currentVertex += rowSize;
			// issue one extra index to turn winding around
			*pI++ = currentVertex;
		}
		
		
		// Skirts
		for (uint16 s = 0; s < 4; ++s)
		{
			// edgeIncrement is the index offset from one original edge vertex to the next
			// in this row or column. Columns skip based on a row size here
			// skirtIncrement is the index offset from one skirt vertex to the next, 
			// because skirts are packed in rows/cols then there is no row multiplier for
			// processing columns
			int edgeIncrement, skirtIncrement;
			switch(s)
			{
				case 0: // top
					edgeIncrement = -static_cast<int>(vertexIncrement);
					skirtIncrement = -static_cast<int>(vertexIncrement);
					break;
				case 1: // left
					edgeIncrement = -static_cast<int>(rowSize);
					skirtIncrement = -static_cast<int>(vertexIncrement);
					break;
				case 2: // bottom
					edgeIncrement = static_cast<int>(vertexIncrement);
					skirtIncrement = static_cast<int>(vertexIncrement);
					break;
				case 3: // right
					edgeIncrement = static_cast<int>(rowSize);
					skirtIncrement = static_cast<int>(vertexIncrement);
					break;
			}
			// Skirts are stored in contiguous rows / columns (rows 0/2, cols 1/3)
			uint16 skirtIndex = calcSkirtVertexIndex(currentVertex, (s % 2) != 0);
			for (uint16 c = 0; c < batchSize - 1; ++c)
			{
				*pI++ = currentVertex;
				*pI++ = skirtIndex;	
				currentVertex += edgeIncrement;
				skirtIndex += skirtIncrement;
			}
			if (s == 3)
			{
				// we issue an extra 2 indices to finish the skirt off
				*pI++ = currentVertex;
				*pI++ = skirtIndex;
				currentVertex += edgeIncrement;
				skirtIndex += skirtIncrement;
			}
		}

		assert (pI - basepI == destData->indexCount);


	}
	//---------------------------------------------------------------------
	void TerrainQuadTreeNode::destroyCpuIndexData()
	{
		for (size_t lod = 0; lod < mLodLevels.size(); ++lod)
		{
			LodLevel* ll = mLodLevels[lod];

			OGRE_DELETE ll->cpuIndexData;
			ll->cpuIndexData = 0;
		}

	}
	//---------------------------------------------------------------------
	void TerrainQuadTreeNode::createGpuVertexData()
	{
		// TODO - mutex cpu data
		if (mVertexDataRecord && mVertexDataRecord->cpuVertexData && !mVertexDataRecord->gpuVertexData)
		{

			// clone CPU data into GPU data
			// default is to create new declarations from hardware manager
			mVertexDataRecord->gpuVertexData = mVertexDataRecord->cpuVertexData->clone();
			mVertexDataRecord->gpuVertexDataDirty = false;
		}

	}
	//---------------------------------------------------------------------
	void TerrainQuadTreeNode::updateGpuVertexData()
	{
		// TODO - mutex cpu data
		if (mVertexDataRecord && mVertexDataRecord->gpuVertexDataDirty)
		{
			mVertexDataRecord->gpuVertexData->vertexBufferBinding->getBuffer(0)->
				copyData(*mVertexDataRecord->cpuVertexData->vertexBufferBinding->getBuffer(0).get());
			mVertexDataRecord->gpuVertexDataDirty = false;
		}
	}
	//---------------------------------------------------------------------
	void TerrainQuadTreeNode::destroyGpuVertexData()
	{
		if (mVertexDataRecord && mVertexDataRecord->gpuVertexData)
		{
			OGRE_DELETE mVertexDataRecord->gpuVertexData;
			mVertexDataRecord->gpuVertexData = 0;
		}


	}
	//---------------------------------------------------------------------
	void TerrainQuadTreeNode::createGpuIndexData()
	{
		for (size_t lod = 0; lod < mLodLevels.size(); ++lod)
		{
			LodLevel* ll = mLodLevels[lod];

			if (!ll->gpuIndexData)
			{
				// clone, using default buffer manager ie hardware
				ll->gpuIndexData = ll->cpuIndexData->clone();
			}
		}

	}
	//---------------------------------------------------------------------
	void TerrainQuadTreeNode::destroyGpuIndexData()
	{
		for (size_t lod = 0; lod < mLodLevels.size(); ++lod)
		{
			LodLevel* ll = mLodLevels[lod];

			OGRE_DELETE ll->gpuIndexData;
			ll->gpuIndexData = 0;
		}

	}
	//---------------------------------------------------------------------
	void TerrainQuadTreeNode::mergeIntoBounds(long x, long y, const Vector3& pos)
	{
		if (pointIntersectsNode(x, y))
		{
			// Make relative to local centre
			Vector3 localPos = pos - mLocalCentre;
			mAABB.merge(localPos);
			mBoundingRadius = std::max(mBoundingRadius, localPos.length());
			
			if (!isLeaf())
			{
				for (int i = 0; i < 4; ++i)
					mChildren[i]->mergeIntoBounds(x, y, pos);
			}
		}
	}
	//---------------------------------------------------------------------
	void TerrainQuadTreeNode::resetBounds(const Rect& rect)
	{
		if (rectIntersectsNode(rect))
		{
			mAABB.setNull();
			mBoundingRadius = 0;
			
			if (!isLeaf())
			{
				for (int i = 0; i < 4; ++i)
					mChildren[i]->resetBounds(rect);
			}

			
		}
	}
	//---------------------------------------------------------------------
	bool TerrainQuadTreeNode::rectIntersectsNode(const Rect& rect)
	{
		return (rect.right >= mOffsetX && rect.left <= mBoundaryX &&
				rect.bottom >= mOffsetY && rect.top <= mBoundaryY);
	}
	//---------------------------------------------------------------------
	bool TerrainQuadTreeNode::pointIntersectsNode(long x, long y)
	{
		return x >= mOffsetX && x < mBoundaryX && 
			y >= mOffsetY && y < mBoundaryY;
	}
	//---------------------------------------------------------------------
	const AxisAlignedBox& TerrainQuadTreeNode::getAABB() const
	{
		return mAABB;
	}
	//---------------------------------------------------------------------
	Real TerrainQuadTreeNode::getBoundingRadius() const
	{
		return mBoundingRadius;
	}
	//---------------------------------------------------------------------
	bool TerrainQuadTreeNode::calculateCurrentLod(const Camera* cam, Real cFactor)
	{
		mSelfOrChildRendered = false;

		// early-out
		if (!cam->isVisible(mMovable->getWorldBoundingBox(true)))
		{
			mCurrentLod = -1;
			return mSelfOrChildRendered;
		}

		// Check children first
		int childRenderedCount = 0;
		if (!isLeaf())
		{
			for (int i = 0; i < 4; ++i)
			{
				if (mChildren[i]->calculateCurrentLod(cam, cFactor))
					++childRenderedCount;
			}

		}

		if (childRenderedCount == 0)
		{
			// no children were within their LOD ranges, so we should consider our own
			Vector3 localPos = cam->getDerivedPosition() - mLocalCentre;
			Real dist;
			if (TerrainGlobalOptions::getUseRayBoxDistanceCalculation())
			{
				// Get distance to this terrain node (to closest point of the box)
				// head towards centre of the box (note, box may not cover mLocalCentre because of height)
				Vector3 dir(mAABB.getCenter() - localPos);
				dir.normalise();
				Ray ray(localPos, dir);
				std::pair<bool, Real> intersectRes = Math::intersects(ray, mAABB);

				// ray will always intersect, we just want the distance
				dist = intersectRes.second;
			}
			else
			{
				// distance to tile centre
				Vector3 diff = mAABB.getCenter() - localPos;
				dist = diff.length();
				// deduct half the radius of the box, assume that on average the 
				// worst case is best approximated by this
				dist -= (mBoundingRadius * 0.5);
			}

			// TODO: calculate material LOD too

			// For each LOD, the distance at which the LOD will transition *downwards*
			// is given by 
			// distTransition = maxDelta * cFactor;
			int lodLvl = 0;
			mCurrentLod = -1;
			for (LodLevelList::iterator i = mLodLevels.begin(); i != mLodLevels.end(); ++i, ++lodLvl)
			{
				// If we have no parent, and this is the lowest LOD, we always render
				// this is the 'last resort' so to speak, we always enoucnter this last
				if (lodLvl+1 == mLodLevels.size() && !mParent)
				{
					mCurrentLod = lodLvl;
					mSelfOrChildRendered = true;
					mLodTransition = 0;
				}
				else
				{
					// check the distance
					LodLevel* ll = *i;

					// Calculate or reuse transition distance
					Real distTransition;
					if (Math::RealEqual(cFactor, ll->lastCFactor))
						distTransition = ll->lastTransitionDist;
					else
					{
						distTransition = ll->maxHeightDelta * cFactor;
						ll->lastCFactor = cFactor;
						ll->lastTransitionDist = distTransition;
					}

					if (dist < distTransition)
					{
						// we're within range of this LOD
						mCurrentLod = lodLvl;
						mSelfOrChildRendered = true;

						if (mTerrain->_getMorphRequired())
						{
							// calculate the transition percentage
							// we need a percentage of the total distance for just this LOD, 
							// which means taking off the distance for the next higher LOD
							// which is either the previous entry in the LOD list, 
							// or the largest of any children. In both cases these will
							// have been calculated before this point, since we process
							// children first. Distances at lower LODs are guaranteed
							// to be larger than those at higher LODs

							Real distTotal = distTransition;
							if (isLeaf())
							{
								// Any higher LODs?
								if (i != mLodLevels.begin())
								{
									LodLevelList::iterator prev = i - 1;
									distTotal -= (*prev)->lastTransitionDist;
								}
							}
							else
							{
								// Take the distance of the lowest LOD of child
								const LodLevel* childLod = mChildWithMaxHeightDelta->getLodLevel(
									mChildWithMaxHeightDelta->getLodCount()-1);
								distTotal -= childLod->lastTransitionDist;
							}
							// fade from 0 to 1 in the last 25% of the distance
							Real distMorphRegion = distTotal * 0.25;
							Real distRemain = distTransition - dist;

							mLodTransition = 1.0 - (distRemain / distMorphRegion);
							mLodTransition = std::min((Real)1.0, mLodTransition);
							mLodTransition = std::max((Real)0.0, mLodTransition);

							// Pass both the transition % and target LOD (GLOBAL current + 1)
							// this selectively applies the morph just to the
							// vertices which would drop out at this LOD, even 
							// while using the single shared vertex data
							mRend->setCustomParameter(Terrain::LOD_MORPH_CUSTOM_PARAM, 
								Vector4(mLodTransition, mCurrentLod + mBaseLod + 1, 0, 0));

						}
						// since LODs are ordered from highest to lowest detail, 
						// we can stop looking now
						break;
					}

				}
			}

		}
		else 
		{
			// we should not render ourself
			mCurrentLod = -1;
			mSelfOrChildRendered = true; 
			if (childRenderedCount < 4)
			{
				// only *some* children decided to render on their own, but either 
				// none or all need to render, so set the others manually to their lowest
				for (int i = 0; i < 4; ++i)
				{
					TerrainQuadTreeNode* child = mChildren[i];
					if (!child->isSelfOrChildRenderedAtCurrentLod())
					{
						child->setCurrentLod(child->getLodCount()-1);
						child->setLodTransition(1.0);
					}
				}
			} // (childRenderedCount < 4)

		} // (childRenderedCount == 0)


		return mSelfOrChildRendered;

	}
	//---------------------------------------------------------------------
	bool TerrainQuadTreeNode::isRenderedAtCurrentLod() const
	{
		return mCurrentLod != -1;
	}
	//---------------------------------------------------------------------
	bool TerrainQuadTreeNode::isSelfOrChildRenderedAtCurrentLod() const
	{
		return mSelfOrChildRendered;
	}
	//---------------------------------------------------------------------
	void TerrainQuadTreeNode::updateRenderQueue(RenderQueue* queue)
	{
		if (isRenderedAtCurrentLod())
		{
			queue->addRenderable(mRend, mTerrain->getRenderQueueGroup());			
		}
	}
	//---------------------------------------------------------------------
	void TerrainQuadTreeNode::visitRenderables(Renderable::Visitor* visitor,  bool debugRenderables)
	{
		visitor->visit(mRend, 0, false);
	}
	//---------------------------------------------------------------------
	const MaterialPtr& TerrainQuadTreeNode::getMaterial(void) const
	{
		return mTerrain->getMaterial();
	}
	//---------------------------------------------------------------------
	Technique* TerrainQuadTreeNode::getTechnique(void) const
	{ 
		// TODO material LOD
		return getMaterial()->getBestTechnique(0, mRend); 
	}
	//---------------------------------------------------------------------
	void TerrainQuadTreeNode::getRenderOperation(RenderOperation& op)
	{
		mNodeWithVertexData->updateGpuVertexData();

		op.indexData = mLodLevels[mCurrentLod]->gpuIndexData;
		op.operationType = RenderOperation::OT_TRIANGLE_STRIP;
		op.useIndexes = true;
		op.vertexData = getVertexDataRecord()->gpuVertexData;
	}
	//---------------------------------------------------------------------
	void TerrainQuadTreeNode::getWorldTransforms(Matrix4* xform) const
	{
		// the vertex data is relative to the node that owns the vertex data
		*xform = mNodeWithVertexData->mMovable->_getParentNodeFullTransform();
	}
	//---------------------------------------------------------------------
	Real TerrainQuadTreeNode::getSquaredViewDepth(const Camera* cam) const
	{
		return mMovable->getParentSceneNode()->getSquaredViewDepth(cam);
	}
	//---------------------------------------------------------------------
	const LightList& TerrainQuadTreeNode::getLights(void) const
	{
		return mMovable->queryLights();
	}
	//---------------------------------------------------------------------
	bool TerrainQuadTreeNode::getCastsShadows(void) const
	{
		return TerrainGlobalOptions::getCastsDynamicShadows();
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	TerrainQuadTreeNode::Movable::Movable(TerrainQuadTreeNode* parent)
		: mParent(parent)
	{
	}
	//---------------------------------------------------------------------
	TerrainQuadTreeNode::Movable::~Movable()
	{

	}
	//---------------------------------------------------------------------
	const String& TerrainQuadTreeNode::Movable::getMovableType(void) const
	{
		static String stype("OgreTerrainNodeMovable");

		return stype;

	}
	//---------------------------------------------------------------------
	const AxisAlignedBox& TerrainQuadTreeNode::Movable::getBoundingBox(void) const
	{
		return mParent->getAABB();
	}
	//---------------------------------------------------------------------
	Real TerrainQuadTreeNode::Movable::getBoundingRadius(void) const
	{
		return mParent->getBoundingRadius();
	}
	//------------------------------------------------------------------------
	void TerrainQuadTreeNode::Movable::_updateRenderQueue(RenderQueue* queue)
	{
		mParent->updateRenderQueue(queue);		
	}
	//------------------------------------------------------------------------
	void TerrainQuadTreeNode::Movable::visitRenderables(Renderable::Visitor* visitor,  bool debugRenderables)
	{
		mParent->visitRenderables(visitor, debugRenderables);	
	}
	//------------------------------------------------------------------------
	//---------------------------------------------------------------------
	TerrainQuadTreeNode::Rend::Rend(TerrainQuadTreeNode* parent)
		:mParent(parent)
	{
	}
	//---------------------------------------------------------------------
	TerrainQuadTreeNode::Rend::~Rend()
	{
	}
	//---------------------------------------------------------------------
	const MaterialPtr& TerrainQuadTreeNode::Rend::getMaterial(void) const
	{
		return mParent->getMaterial();
	}
	//---------------------------------------------------------------------
	Technique* TerrainQuadTreeNode::Rend::getTechnique() const
	{
		return mParent->getTechnique();
	}
	//---------------------------------------------------------------------
	void TerrainQuadTreeNode::Rend::getRenderOperation(RenderOperation& op)
	{
		mParent->getRenderOperation(op);
	}
	//---------------------------------------------------------------------
	void TerrainQuadTreeNode::Rend::getWorldTransforms(Matrix4* xform) const
	{
		mParent->getWorldTransforms(xform);
	}
	//---------------------------------------------------------------------
	Real TerrainQuadTreeNode::Rend::getSquaredViewDepth(const Camera* cam) const
	{
		return mParent->getSquaredViewDepth(cam);
	}
	//---------------------------------------------------------------------
	const LightList& TerrainQuadTreeNode::Rend::getLights(void) const
	{
		return mParent->getLights();
	}
	//---------------------------------------------------------------------
	bool TerrainQuadTreeNode::Rend::getCastsShadows(void) const
	{
		return mParent->getCastsShadows();
	}
	//---------------------------------------------------------------------


	



}

