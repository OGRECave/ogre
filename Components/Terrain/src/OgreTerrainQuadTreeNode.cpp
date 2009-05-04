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
		, mVertexDataRecord(0)
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
			VertexBufferBinding* buf = OGRE_NEW VertexBufferBinding();

			mVertexDataRecord->cpuVertexData = OGRE_NEW VertexData(dcl, buf);

			// Vertex declaration
			// TODO: consider vertex compression
			size_t offset = 0;
			// POSITION binding depends on whether we're geomorphing
			// TODO - should we check that materials have geomorphing?
			bool morph = false;
			if (mTerrain->getUseLodMorph() && 
				Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_VERTEX_PROGRAM))
			{
				// float4(x, y, z, delta)
				offset += dcl->addElement(0, offset, VET_FLOAT4, VES_POSITION).getSize();
				morph = true;
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
			size_t numVerts = Math::Sqr(mVertexDataRecord->resolution);
			// Now add space for skirts
			uint16 currentRes = mVertexDataRecord->resolution;
			uint16 levels = mVertexDataRecord->treeLevels;
			uint16 numTiles = 1;
			while (levels--)
			{
				// skirts require a strip around the outside 
				// top & bottom row (resolution + 2 each)
				// left & right sides (resolution each)
				// so resolution * 4 + 4
				// multiply by numTiles because at each level, we have to supply 4 children
				numVerts += numTiles * (currentRes * 4 + 4);
				numTiles *= 4;
				currentRes = ((currentRes - 1) * 0.5) + 1;
			}

			// manually create CPU-side buffer
			HardwareVertexBufferSharedPtr vbuf(
				OGRE_NEW DefaultHardwareVertexBuffer(dcl->getVertexSize(0), numVerts, HardwareBuffer::HBU_STATIC_WRITE_ONLY));

			// Fill the buffers
			// Main data
			uint16 xlimit = mOffsetX + mVertexDataRecord->resolution;
			uint16 ylimit = mOffsetY + mVertexDataRecord->resolution;
			Real uvScale = 1.0 / (mTerrain->getSize() - 1);
			const float* pBaseHeight = mTerrain->getHeightData(mOffsetX, mOffsetY);
			const float* pBaseDelta = mTerrain->getDeltaData(mOffsetX, mOffsetY);
			uint16 rowskip = mTerrain->getSize();
			Vector3 pos;
			float* pBuf = static_cast<float*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));
			for (uint16 y = mOffsetY; y < ylimit; ++y)
			{
				const float* pHeight = pBaseHeight;
				const float* pDelta = pBaseDelta;
				for (uint16 x = mOffsetX; x < xlimit; ++x)
				{
					mTerrain->getPoint(x, y, *pHeight++, &pos);

					*pBuf++ = pos.x;
					*pBuf++ = pos.y;
					*pBuf++ = pos.z;
					if (morph)
						*pBuf++ = *pDelta++;

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

					
				}
				pBaseHeight += rowskip;
				pBaseDelta += rowskip;

			}
			// skirts



			vbuf->unlock();


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
	void TerrainQuadTreeNode::createGpuVertexData()
	{
		if (mVertexDataRecord && mVertexDataRecord->cpuVertexData)
		{
			destroyGpuVertexData();

			// clone CPU data into GPU data
			// default is to create new declarations from hardware manager
			mVertexDataRecord->gpuVertexData = mVertexDataRecord->cpuVertexData->clone();

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



}

