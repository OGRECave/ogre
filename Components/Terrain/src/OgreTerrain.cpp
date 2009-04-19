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
#include "OgreTerrain.h"
#include "OgreTerrainQuadTreeNode.h"
#include "OgreStreamSerialiser.h"
#include "OgreMath.h"
#include "OgreImage.h"
#include "OgrePixelFormat.h"
#include "OgreSceneManager.h"
#include "OgreSceneNode.h"

namespace Ogre
{
	//---------------------------------------------------------------------
	const uint32 Terrain::TERRAIN_CHUNK_ID = StreamSerialiser::makeIdentifier("TERR");
	const uint16 Terrain::TERRAIN_CHUNK_VERSION = 1;
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	bool TerrainGlobalOptions::msUseTriangleStrips = true;
	bool TerrainGlobalOptions::msUseLodMorph = true;
	Real TerrainGlobalOptions::msSkirtSize = 10;
	//---------------------------------------------------------------------
	Terrain::Terrain(SceneManager* sm)
		: mSceneMgr(sm)
		, mHeightData(0)
		, mDeltaData(0)
		, mPos(Vector3::ZERO)
		, mQuadTree(0)
		, mNumLodLevels(0)
		, mTreeDepth(0)
	{
		mRootNode = sm->getRootSceneNode()->createChildSceneNode();
	}
	//---------------------------------------------------------------------
	Terrain::~Terrain()
	{
		freeGPUResources();
		freeCPUResources();
		mSceneMgr->destroySceneNode(mRootNode);
	}
	//---------------------------------------------------------------------
	void Terrain::save(StreamSerialiser& stream)
	{
		stream.writeChunkBegin(TERRAIN_CHUNK_ID, TERRAIN_CHUNK_VERSION);

		uint8 align = (uint8)mAlign;
		stream.write(&align);

		stream.write(&mSize);
		stream.write(&mWorldSize);
		stream.write(&mMaxBatchSize);
		stream.write(&mMinBatchSize);
		stream.write(&mPos);
		stream.write(mHeightData, mSize * mSize);

		stream.writeChunkEnd(TERRAIN_CHUNK_ID);
	}
	//---------------------------------------------------------------------
	bool Terrain::prepare(StreamSerialiser& stream)
	{
		freeCPUResources();

		// get settings
		mUseTriangleStrips = TerrainGlobalOptions::getUseTriangleStrips();
		mUseLodMorph = TerrainGlobalOptions::getUseLodMorph();
		mSkirtSize = TerrainGlobalOptions::getSkirtSize();

		if (!stream.readChunkBegin(TERRAIN_CHUNK_ID, TERRAIN_CHUNK_VERSION))
			return false;

		uint8 align;
		stream.read(&align);
		mAlign = (Alignment)align;
		stream.read(&mWorldSize);
		stream.read(&mMaxBatchSize);
		stream.read(&mMinBatchSize);
		stream.read(&mPos);
		updateBaseScale();
		determineLodLevels();


		size_t numVertices = mSize * mSize;
		mHeightData = OGRE_ALLOC_T(float, numVertices, MEMCATEGORY_GEOMETRY);
		stream.read(mHeightData, numVertices);
		mDeltaData = OGRE_ALLOC_T(float, numVertices, MEMCATEGORY_GEOMETRY);
		stream.read(mDeltaData, numVertices);

		stream.readChunkEnd(TERRAIN_CHUNK_ID);

		mQuadTree = OGRE_NEW TerrainQuadTreeNode(this, 0, 0, 0, mSize);
		mQuadTree->prepare();

		return true;
	}
	//---------------------------------------------------------------------
	bool Terrain::prepare(const ImportData& importData)
	{
		freeCPUResources();

		// get settings
		mUseTriangleStrips = TerrainGlobalOptions::getUseTriangleStrips();
		mUseLodMorph = TerrainGlobalOptions::getUseLodMorph();
		mSkirtSize = TerrainGlobalOptions::getSkirtSize();

		mAlign = importData.terrainAlign;
		mSize = importData.terrainSize;
		mWorldSize = importData.worldSize;
		mMaxBatchSize = importData.maxBatchSize;
		mMinBatchSize = importData.minBatchSize;
		mPos = importData.pos;
		updateBaseScale();
		determineLodLevels();

		size_t numVertices = mSize * mSize;

		mHeightData = OGRE_ALLOC_T(float, numVertices, MEMCATEGORY_GEOMETRY);

		if (importData.inputFloat)
		{
			if (Math::RealEqual(importData.inputBias, 0.0) && Math::RealEqual(importData.inputScale, 1.0))
			{
				// straight copy
				memcpy(mHeightData, importData.inputFloat, sizeof(float) * numVertices);
			}
			else
			{
				// scale & bias
				float* src = importData.inputFloat;
				float* dst = mHeightData;
				for (size_t i = 0; i < numVertices; ++i)
					*dst++ = (*src++ * importData.inputScale) + importData.inputBias;
			}
		}
		if (importData.inputImage)
		{
			Image* img = importData.inputImage;

			if (img->getWidth() != mSize || img->getHeight() != mSize)
				img->resize(mSize, mSize);

			// convert image data to floats
			PixelBox destBox(mSize, mSize, 1, PF_FLOAT32_R, mHeightData);
			PixelUtil::bulkPixelConversion(img->getPixelBox(), destBox);

			if (!Math::RealEqual(importData.inputBias, 0.0) || !Math::RealEqual(importData.inputScale, 1.0))
			{
				float* pAdj = mHeightData;
				for (size_t i = 0; i < numVertices; ++i)
					*pAdj++ = (*pAdj * importData.inputScale) + importData.inputBias;

			}

		}
		else
		{
			// start with flat terrain
			memset(mHeightData, 0, sizeof(float) * mSize * mSize);
		}

		mDeltaData = OGRE_ALLOC_T(float, numVertices, MEMCATEGORY_GEOMETRY);
		// calculate entire terrain
		Rect rect;
		rect.bottom = 0; rect.top = mSize - 1;
		rect.left = 0; rect.right = mSize - 1;
		calculateHeightDeltas(rect);

		mQuadTree = OGRE_NEW TerrainQuadTreeNode(this, 0, 0, 0, mSize);
		mQuadTree->prepare();

		return true;

	}
	//---------------------------------------------------------------------
	void Terrain::determineLodLevels()
	{
		/* On a leaf-node basis, LOD can vary from maxBatch to minBatch in 
			number of vertices. After that, nodes will be gathered into parent
			nodes with the same number of vertices, but they are combined with
			3 of their siblings. In practice, the number of LOD levels overall 
			is:
				LODlevels = log2(size - 1) - log2(minBatch - 1) + 1
				TreeDepth = log2(maxBatch - 1) - log2(minBatch - 1) + 2

			.. it's just that at the max LOD, the terrain is divided into 
			(size - 1) / (maxBatch - 1) tiles each of maxBatch vertices, and
			at the lowest LOD the terrain is made up of one single tile of 
			minBatch vertices. 

			Example: size = 257, minBatch = 17, maxBatch = 33

			LODlevels = log2(257 - 1) - log2(17 - 1) + 1 = 8 - 4 + 1 = 5
			TreeDepth = log2(33 - 1) - log2(17 - 1) + 2 = 5 - 4 + 2 = 3

			LOD list - this assumes everything changes at once, which rarely happens of course
			           in fact except where groupings must occur, tiles can change independently
			LOD 0: 257 vertices, 8 x 33 vertex tiles (tree depth 2)
			LOD 1: 129 vertices, 8 x 17 vertex tiles (tree depth 2)
			LOD 2: 65  vertices, 4 x 17 vertex tiles (tree depth 1)
			LOD 3: 33  vertices, 2 x 17 vertex tiles (tree depth 1)
			LOD 4: 17  vertices, 1 x 17 vertex tiles (tree depth 0)

			Notice how we only have 2 sizes of index buffer to be concerned about,
			17 vertices (per side) or 33. This makes buffer re-use much easier while
			still giving the full range of LODs.
		*/
		mNumLodLevels = Math::Log2(mSize - 1) - Math::Log2(mMinBatchSize - 1) + 1;
		mTreeDepth = Math::Log2(mMaxBatchSize - 1) - Math::Log2(mMinBatchSize - 1) + 2;
	}
	//---------------------------------------------------------------------
	void Terrain::load()
	{
		mQuadTree->load();
	}
	//---------------------------------------------------------------------
	void Terrain::unload()
	{
		mQuadTree->unload();
	}
	//---------------------------------------------------------------------
	void Terrain::unprepare()
	{
		mQuadTree->unprepare();
	}
	//---------------------------------------------------------------------
	float* Terrain::getHeightData()
	{
		return mHeightData;
	}
	//---------------------------------------------------------------------
	float* Terrain::getHeightData(long x, long y)
	{
		assert (x >= 0 && x < mSize && y >= 0 && y < mSize);
		return &mHeightData[y * mSize + x];
	}
	//---------------------------------------------------------------------
	void Terrain::getPoint(long x, long y, Vector3* outpos)
	{
		getPointAlign(x, y, mAlign, outpos);
	}
	//---------------------------------------------------------------------
	void Terrain::getPointAlign(long x, long y, Alignment align, Vector3* outpos)
	{
		switch(align)
		{
		case ALIGN_X_Z:
			outpos->y = *getHeightData(x, y);
			outpos->x = x * mScale + mBase;
			outpos->z = y * mScale + mBase;
			break;
		case ALIGN_Y_Z:
			outpos->x = *getHeightData(x, y);
			outpos->y = x * mScale + mBase;
			outpos->z = y * mScale + mBase;
			break;
		case ALIGN_X_Y:
			outpos->z = *getHeightData(x, y);
			outpos->x = x * mScale + mBase;
			outpos->y = y * mScale + mBase;
			break;
		};

	}
	//---------------------------------------------------------------------
	Terrain::Alignment Terrain::getAlignment() const
	{
		return mAlign;
	}
	//---------------------------------------------------------------------
	uint16 Terrain::getSize() const
	{
		return mSize;
	}
	//---------------------------------------------------------------------
	uint16 Terrain::getMaxBatchSize() const
	{
		return mMaxBatchSize;
	}
	//---------------------------------------------------------------------
	uint16 Terrain::getMinBatchSize() const
	{
		return mMinBatchSize;
	}
	//---------------------------------------------------------------------
	Real Terrain::getWorldSize() const
	{
		return mWorldSize;
	}
	//---------------------------------------------------------------------
	void Terrain::setPosition(const Vector3& pos)
	{
		mPos = pos;
		mRootNode->setPosition(pos);
		updateBaseScale();
	}
	//---------------------------------------------------------------------
	void Terrain::updateBaseScale()
	{
		// centre the terrain on local origin
		mBase = -mWorldSize * 0.5; 
		// scale determines what 1 unit on the grid becomes in world space
		mScale =  mWorldSize / (Real)mSize;
	}
	//---------------------------------------------------------------------
	void Terrain::dirty()
	{
		// TODO
	}
	//---------------------------------------------------------------------
	void Terrain::dirtyRect(const Rect& rect)
	{
		// TODO
	}
	//---------------------------------------------------------------------
	void Terrain::freeCPUResources()
	{
		OGRE_FREE(mHeightData, MEMCATEGORY_GEOMETRY);
		mHeightData = 0;

		OGRE_FREE(mDeltaData, MEMCATEGORY_GEOMETRY);
		mHeightData = 0;

		OGRE_DELETE mQuadTree;
		mQuadTree = 0;



	}
	//---------------------------------------------------------------------
	void Terrain::freeGPUResources()
	{
		// delete batched geometry

		// SHARE geometry between Terrain instances!

	}
	//---------------------------------------------------------------------
	void Terrain::calculateHeightDeltas(const Rect& rect)
	{

		/// Iterate over target levels, 
		for (int targetLevel = 1; targetLevel < mNumLodLevels; ++targetLevel)
		{
			int step = 1 << targetLevel;
			// The step of the next higher LOD
			int higherstep = step >> 1;

			// Adjust rectangle 

			for (int j = rect.bottom; j < rect.top - step; j += step )
			{
				for (int i = rect.left; i < rect.right - step; i += step )
				{
					// Form planes relating to the lower detail tris to be produced
					// For tri lists and even tri strip rows, they are this shape:
					// x---x
					// | / |
					// x---x
					// For odd tri strip rows, they are this shape:
					// x---x
					// | \ |
					// x---x

					Vector3 v1, v2, v3, v4;
					getPointAlign(i, j, ALIGN_X_Z, &v1);
					getPointAlign(i + step, j, ALIGN_X_Z, &v2);
					getPointAlign(i, j + step, ALIGN_X_Z, &v3);
					getPointAlign(i + step, j + step, ALIGN_X_Z, &v4);

					Plane t1, t2;
					bool backwardTri = false;
					if (!mUseTriangleStrips || j % 2 == 0)
					{
						t1.redefine(v1, v3, v2);
						t2.redefine(v2, v3, v4);
					}
					else
					{
						t1.redefine(v1, v3, v4);
						t2.redefine(v1, v4, v2);
						backwardTri = true;
					}

					// include the bottommost row of vertices if this is the last row
					int zubound = (j == (mSize - step)? step : step - 1);
					for ( int z = 0; z <= zubound; z++ )
					{
						// include the rightmost col of vertices if this is the last col
						int xubound = (i == (mSize - step)? step : step - 1);
						for ( int x = 0; x <= xubound; x++ )
						{
							int fulldetailx = i + x;
							int fulldetailz = j + z;
							if ( fulldetailx % step == 0 && 
								fulldetailz % step == 0 )
							{
								// Skip, this one is a vertex at this level
								continue;
							}

							Real zpct = (Real)z / (Real)step;
							Real xpct = (Real)x / (Real)step;

							//interpolated height
							Vector3 actualPos;
							getPointAlign(fulldetailx, fulldetailz, ALIGN_X_Z, &actualPos);
							Real interp_h;
							// Determine which tri we're on 
							if ((xpct + zpct <= 1.0f && !backwardTri) ||
								(xpct + (1-zpct) <= 1.0f && backwardTri))
							{
								// Solve for x/z
								interp_h = 
									(-(t1.normal.x * actualPos.x)
									- t1.normal.z * actualPos.z
									- t1.d) / t1.normal.y;
							}
							else
							{
								// Second tri
								interp_h = 
									(-(t2.normal.x * actualPos.x)
									- t2.normal.z * actualPos.z
									- t2.d) / t2.normal.y;
							}

							Real actual_h = actualPos.y;
							Real delta = interp_h - actual_h;

							// Save height difference 
							mDeltaData[fulldetailx + (fulldetailz * mSize)] = delta;

						}

					}
				} // i
			} // j
		} // targetLevel

	}

	//---------------------------------------------------------------------
	Real getMaxHeightDelta(const Rect& rect, unsigned short srcLOD)
	{
		return 0;
	}



}

