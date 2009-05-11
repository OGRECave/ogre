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
	{
		if (terrain->getMinBatchSize() < size)
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
		
		// Local centre calculation
		// Because of pow2 + 1 there is always a middle point
		uint16 midoffset = (size - 1) / 2;
		uint16 midpointx = mOffsetX + midoffset;
		uint16 midpointy = mOffsetY + midoffset;
		// derive the local centre, but give it a height of 0
		// TODO - what if we actually centred this at the terrain height at this point?
		// would this be better?
		mTerrain->getPoint(midpointx, midpointy, 0, &mLocalCentre);
		
	}
	//---------------------------------------------------------------------
	TerrainQuadTreeNode::~TerrainQuadTreeNode()
	{
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
			if (lod >= mBaseLod && lod < mBaseLod + mLodLevels.size())
			{
				// Apply the delta to all LODs equal or lower detail to lod
				LodLevelList::iterator i = mLodLevels.begin();
				std::advance(i, lod - mBaseLod);
				(*i)->maxHeightDelta = std::max((*i)->maxHeightDelta, delta);
			}

			// pass on to children
			if (!isLeaf())
			{
				LodLevel* lowestOwnLod = *mLodLevels.rbegin();

				for (int i = 0; i < 4; ++i)
				{
					mChildren[i]->notifyDelta(x, y, lod, delta);

					// make sure that our highest delta value is greater than all children's
					// otherwise we could have some crossover problems
					// TODO

				}

			}

			// TODO - make sure own LOD levels delta values ascend

		}
	}
	//---------------------------------------------------------------------
	void TerrainQuadTreeNode::assignVertexData(uint16 treeDepthStart, uint16 treeDepthEnd, uint16 resolution)
	{
		assert(treeDepthStart < mDepth && "Should not be calling this");

		if (mDepth == treeDepthStart)
		{
			// we own this vertex data
			mNodeWithVertexData = this;
			mVertexDataRecord = OGRE_NEW VertexDataRecord(resolution, treeDepthEnd - treeDepthStart + 1);

			createCpuVertexData();

			// pass on to children
			if (!isLeaf() && treeDepthEnd > mDepth)
			{
				for (int i = 0; i < 4; ++i)
					mChildren[i]->useAncestorVertexData(this, treeDepthEnd, resolution);

			}
		}
		else
		{
			assert(!isLeaf() && "No more levels below this!");

			for (int i = 0; i < 4; ++i)
				mChildren[i]->assignVertexData(treeDepthStart, treeDepthEnd, resolution);
			
		}
		createCpuIndexData();

	}
	//---------------------------------------------------------------------
	void TerrainQuadTreeNode::useAncestorVertexData(TerrainQuadTreeNode* owner, uint16 treeDepthEnd, uint16 resolution)
	{
		mNodeWithVertexData = owner; 
		mVertexDataRecord = 0;

		if (!isLeaf() && treeDepthEnd > mDepth)
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
			// POSITION binding depends on whether we're geomorphing
			// TODO - should we check that materials have geomorphing?
			if (mTerrain->getUseLodMorph() && 
				Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_VERTEX_PROGRAM))
			{
				// float4(x, y, z, delta)
				offset += dcl->addElement(0, offset, VET_FLOAT4, VES_POSITION).getSize();
			}
			else
			{
				// float3(x, y, z)
				offset += dcl->addElement(0, offset, VET_FLOAT3, VES_POSITION).getSize();
			}

			if (mTerrain->getGenerateVertexNormals())
			{
				offset += dcl->addElement(0, offset, VET_FLOAT3, VES_NORMAL).getSize();
			}

			offset += dcl->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES).getSize();

			// Calculate number of vertices
			// Base geometry res * res
			size_t baseNumVerts = Math::Sqr(mVertexDataRecord->resolution);
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
			numVerts += mVertexDataRecord->resolution * mVertexDataRecord->numSkirtRowsCols;
			numVerts += mVertexDataRecord->resolution * mVertexDataRecord->numSkirtRowsCols;

			// manually create CPU-side buffer
			HardwareVertexBufferSharedPtr vbuf(
				OGRE_NEW DefaultHardwareVertexBuffer(dcl->getVertexSize(0), numVerts, HardwareBuffer::HBU_STATIC_WRITE_ONLY));
			
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

		bool morph = mTerrain->getUseLodMorph() && 
			Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_VERTEX_PROGRAM);

		long destOffsetX = rect.left - mOffsetX;
		long destOffsetY = rect.top - mOffsetY;
		// Fill the buffers
		// Main data
		uint16 inc = mTerrain->getSize() / mVertexDataRecord->resolution;
		
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
		uint16 destRowSkip = mVertexDataRecord->resolution * vbuf->getVertexSize();
		Vector3 pos;
		float* pRootBuf = static_cast<float*>(vbuf->lock(lockMode));
		float* pRowBuf = pRootBuf;
		// skip dest buffer in by left/top
		pRowBuf += destOffsetY * destRowSkip + destOffsetX * vbuf->getVertexSize();
		for (uint16 y = rect.top; y < rect.bottom; y += inc)
		{
			const float* pHeight = pBaseHeight;
			const float* pDelta = pBaseDelta;
			float* pBuf = pRowBuf;
			for (uint16 x = rect.left; x < rect.right; x += inc)
			{
				mTerrain->getPoint(x, y, *pHeight, &pos);
				pHeight += inc;

				*pBuf++ = pos.x;
				*pBuf++ = pos.y;
				*pBuf++ = pos.z;
				if (morph)
				{
					*pBuf++ = *pDelta;
					pDelta += inc;
				}

				if (mTerrain->getGenerateVertexNormals())
				{
					// complete these later
					*pBuf++;
					*pBuf++;
					*pBuf++;
				}

				// UVs - base UVs vary from 0 to 1, all other values
				// will be derived using scalings
				*pBuf++ = x * uvScale;
				*pBuf++ = 1.0 - (y * uvScale);
				
				// Update bounds
				mergeIntoBounds(x, y, pos);

				
			}
			pBaseHeight += rowskip;
			pBaseDelta += rowskip;
			pRowBuf += destRowSkip;

		}

		// skirt spacing based on top-level resolution (* inc to cope with resolution which is not the max)
		uint16 skirtSpacing = (mVertexDataRecord->resolution-1) / (mVertexDataRecord->numSkirtRowsCols-1) * inc;
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
			* mVertexDataRecord->resolution * mVertexDataRecord->resolution;
		// move it onwards to skip the skirts we don't need to update
		pRowBuf += destRowSkip * (skirtStartY - mOffsetY) / skirtSpacing;
		pRowBuf += vbuf->getVertexSize() * (skirtStartX - mOffsetX) / skirtSpacing;
		for (uint16 y = skirtStartY; y < rect.bottom; y += skirtSpacing)
		{
			const float* pHeight = pBaseHeight;
			float* pBuf = pRowBuf;
			for (uint16 x = skirtStartX; x < rect.right; x += inc)
			{
				mTerrain->getPoint(x, y, *pHeight, &pos);
				pHeight += inc;

				pos += skirtOffset;

				*pBuf++ = pos.x;
				*pBuf++ = pos.y;
				*pBuf++ = pos.z;
				if (morph)
					*pBuf++ = 0; // no morphing

				if (mTerrain->getGenerateVertexNormals())
				{
					// complete these later - should be a copy of the main
					*pBuf++;
					*pBuf++;
					*pBuf++;
				}

				// UVs - same as base
				*pBuf++ = x * uvScale;
				*pBuf++ = 1.0 - (y * uvScale);
			}
			pBaseHeight += rowskip * skirtSpacing;
			pRowBuf += destRowSkip;
		}
		// skirt cols
		// position dest buffer just after the main vertex data and skirt rows
		pRowBuf = pRootBuf + vbuf->getVertexSize() 
		* mVertexDataRecord->resolution * mVertexDataRecord->resolution;
		// skip the row skirts
		pRowBuf += mVertexDataRecord->numSkirtRowsCols * mVertexDataRecord->resolution * vbuf->getVertexSize();
		// move it onwards to skip the skirts we don't need to update
		pRowBuf += destRowSkip * (skirtStartX - mOffsetX) / skirtSpacing;
		pRowBuf += vbuf->getVertexSize() * (skirtStartY - mOffsetY) / skirtSpacing;
		
		for (uint16 x = skirtStartX; x < rect.right; x += skirtSpacing)
		{
			float* pBuf = pRowBuf;
			for (uint16 y = skirtStartY; y < rect.bottom; y += inc)
			{
				mTerrain->getPoint(x, y, mTerrain->getHeight(x, y), &pos);
				pos += skirtOffset;

				*pBuf++ = pos.x;
				*pBuf++ = pos.y;
				*pBuf++ = pos.z;
				if (morph)
					*pBuf++ = 0; // no morphing

				if (mTerrain->getGenerateVertexNormals())
				{
					// complete these later - should be a copy of the main
					*pBuf++;
					*pBuf++;
					*pBuf++;
				}

				// UVs - same as base
				*pBuf++ = x * uvScale;
				*pBuf++ = 1.0 - (y * uvScale);
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
		uint16 row = mainIndex / vdr->resolution;
		uint16 col = mainIndex % vdr->resolution;
		
		// No offsets used here, this is an index into the current vertex data, 
		// which is already relative
		if (isCol)
		{
			uint16 base = vdr->numSkirtRowsCols * vdr->resolution;
			return base + vdr->resolution * col + row;
		}
		else
		{
			return vdr->resolution * row + col;
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

			if (mTerrain->getUseTriangleStrips())
				createTriangleStripBuffer(ll->batchSize, ll->cpuIndexData);
			else
				createTriangleListBuffer(ll->batchSize, ll->cpuIndexData);
			
		}
	}
	//---------------------------------------------------------------------
	void TerrainQuadTreeNode::createTriangleListBuffer(uint16 batchSize, IndexData* destData)
	{
		/* Triangles are this shape for all rows:
		 6---7---8
		 | \ | \ |
		 3---4---5
		 | \ | \ |
		 0---1---2
		 Represented as (0,1,3), (3,1,4) etc
		Skirts are added to the end of the list, and for consistency with the
		strip generation of skirts, are generated from the top-right and go
		anticlockwise around the edge. This goes: (8,s8,7,s8,s7,7,7,s7,6,s7,s6,6)
		 for the top edge, and correspondingly for the other edges.
		*/
		

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
		// this includes the degenerate triangle
		size_t indexesPerRow = batchSize * 2;
		size_t numRows = batchSize - 1;
		size_t mainIndexCount = indexesPerRow * numRows;
		// skirts take indexesPerRow for the first one, then indexesPerRow-1 for the
		// other 3 sides because we chain then together
		size_t skirtIndexCount = indexesPerRow + (indexesPerRow - 1) * 3;
		
		destData->indexStart = 0;
		destData->indexCount = mainIndexCount + skirtIndexCount;
		destData->indexBuffer.bind(OGRE_NEW DefaultHardwareIndexBuffer(
			HardwareIndexBuffer::IT_16BIT, destData->indexCount, HardwareBuffer::HBU_STATIC)); 
		
		uint16* pI = static_cast<uint16*>(destData->indexBuffer->lock(HardwareBuffer::HBL_DISCARD));
		
		// Main terrain area
		size_t vertexIncrement = (getVertexDataRecord()->resolution - 1) / (batchSize - 1);
		size_t rowSize = getVertexDataRecord()->resolution * vertexIncrement;
		uint16 currentVertex = batchSize * vertexIncrement;
		bool rightToLeft = true;
		for (uint16 r = 0; r < numRows; ++r)
		{
			for (uint16 c = 0; c < batchSize - 1; ++c)
			{
								
				*pI++ = currentVertex;
				*pI++ = currentVertex + rowSize;
				
				currentVertex = rightToLeft ? 
					currentVertex - vertexIncrement : currentVertex + vertexIncrement;
				
			}
			rightToLeft = !rightToLeft;
			currentVertex += rowSize;
		}
		
		
		// Skirts
		for (uint16 s = 0; s < 4; ++s)
		{
			int edgeIncrement;
			switch(s)
			{
				case 0: // top
					edgeIncrement = -static_cast<int>(vertexIncrement);
					break;
				case 1: // left
					edgeIncrement = -static_cast<int>(rowSize);
					break;
				case 2: // bottom
					edgeIncrement = static_cast<int>(vertexIncrement);
					break;
				case 3: // right
					edgeIncrement = static_cast<int>(rowSize);
					break;
			}
			// Skirts are stored in contiguous rows / columns (rows 0/2, cols 1/3)
			uint16 skirtIndex = calcSkirtVertexIndex(currentVertex, (s % 2) != 0);
			for (uint16 c = 0; c < batchSize - 1; ++c)
			{
				*pI++ = currentVertex;
				*pI++ = skirtIndex;	
				currentVertex += edgeIncrement;
				skirtIndex += vertexIncrement;
			}
			if (s == 3)
			{
				// we issue an extra 2 indices to finish the skirt off
				*pI++ = currentVertex;
				*pI++ = skirtIndex;
				currentVertex += edgeIncrement;
				skirtIndex += vertexIncrement;
			}
		}
		
		


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
		if (mVertexDataRecord && mVertexDataRecord->cpuVertexData)
		{
			destroyGpuVertexData();

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
		if (mVertexDataRecord->gpuVertexDataDirty)
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

			OGRE_DELETE ll->gpuIndexData;
			// clone, using default buffer manager ie hardware
			ll->gpuIndexData = ll->cpuIndexData->clone();
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
		bool ret = false;

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
			// Get distance to this terrain node (to closest point of the box)
			Vector3 localPos = cam->getDerivedPosition() - mLocalCentre;
			Ray ray(localPos, -localPos);
			std::pair<bool, Real> intersectRes = Math::intersects(ray, mAABB);

			// ray will always intersect, we just want the distance
			Real dist = intersectRes.second;

			// For each LOD, the distance at which the LOD will transition *downwards*
			// is given by 
			// distTransition = maxDelta * cFactor;
			int lodLvl = 0;
			mCurrentLod = -1;
			for (LodLevelList::const_iterator i = mLodLevels.begin(); i != mLodLevels.end(); ++i, ++lodLvl)
			{
				const LodLevel* ll = *i;

				Real distTransition = ll->maxHeightDelta * cFactor;
				if (dist < distTransition)
				{
					// we're within range of this LOD
					mCurrentLod = lodLvl;

					if (mTerrain->getUseLodMorph())
					{
						// calculate the transition percentage
						Real distRemain = distTransition - dist;
						// we need a percentage of the total distance for this LOD, 
						// which means taking off the distance for the next higher LOD
						// TODO

					}


					// since LODs are ordered from highest to lowest detail, 
					// we can stop looking now
					break;


				}
			}

		}
		else 
		{
			// we should not render ourself
			mCurrentLod = -1;
			if (childRenderedCount < 4)
			{
				// only *some* children decided to render on their own, but either 
				// none or all need to render, so set the others manually to their lowest
				for (int i = 0; i < 4; ++i)
				{
					TerrainQuadTreeNode* child = mChildren[i];
					if (child->getCurrentLod() == -1)
					{
						child->setCurrentLod(child->getLodCount()-1);
						child->setLodTransition(1.0);
					}
				}
			} // (childRenderedCount < 4)

			ret = true;

		} // (childRenderedCount == 0)


		return ret;

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
		// TODO
	}
	//------------------------------------------------------------------------
	void TerrainQuadTreeNode::Movable::visitRenderables(Renderable::Visitor* visitor,  bool debugRenderables)
	{
		// TODO
	}
	//------------------------------------------------------------------------

	



}

