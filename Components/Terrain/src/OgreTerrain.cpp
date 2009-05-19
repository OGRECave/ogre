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
#include "OgreException.h"
#include "OgreBitwise.h"
#include "OgreStringConverter.h"
#include "OgreViewport.h"
#include "OgreLogManager.h"

namespace Ogre
{
	//---------------------------------------------------------------------
	const uint32 Terrain::TERRAIN_CHUNK_ID = StreamSerialiser::makeIdentifier("TERR");
	const uint16 Terrain::TERRAIN_CHUNK_VERSION = 1;
	// since 129^2 is the greatest power we can address in 16-bit index
	const uint16 Terrain::TERRAIN_MAX_BATCH_SIZE = 129; 
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	bool TerrainGlobalOptions::msUseTriangleStrips = true;
	bool TerrainGlobalOptions::msUseLodMorph = true;
	Real TerrainGlobalOptions::msSkirtSize = 10;
	bool TerrainGlobalOptions::msGenerateVertexNormals = false;
	bool TerrainGlobalOptions::msGenerateNormalMap = true;
	bool TerrainGlobalOptions::msGenerateShadowMap = false;
	Vector3 TerrainGlobalOptions::msShadowMapDir = Vector3(1, -1, 0).normalisedCopy();
	bool TerrainGlobalOptions::msGenerateHorizonMap = false;
	Radian TerrainGlobalOptions::msHorizonMapAzimuth = Radian(0);
	Radian TerrainGlobalOptions::msHorizonMapZenith = Radian(0);
	bool TerrainGlobalOptions::msCastsShadows = false;
	Real TerrainGlobalOptions::msMaxPixelError = 3.0;
	uint8 TerrainGlobalOptions::msRenderQueueGroup = RENDER_QUEUE_MAIN;
	//---------------------------------------------------------------------
	Terrain::Terrain(SceneManager* sm)
		: mSceneMgr(sm)
		, mHeightData(0)
		, mDeltaData(0)
		, mPos(Vector3::ZERO)
		, mQuadTree(0)
		, mNumLodLevels(0)
		, mNumLodLevelsPerLeafNode(0)
		, mTreeDepth(0)
	{
		mRootNode = sm->getRootSceneNode()->createChildSceneNode();
		sm->addListener(this);
	}
	//---------------------------------------------------------------------
	Terrain::~Terrain()
	{
		freeGPUResources();
		freeCPUResources();
		if (mSceneMgr)
		{
			mSceneMgr->destroySceneNode(mRootNode);
			mSceneMgr->removeListener(this);
		}
	}
	//---------------------------------------------------------------------
	const AxisAlignedBox& Terrain::getAABB() const
	{
		if (!mQuadTree)
			return AxisAlignedBox::BOX_NULL;
		else
			return mQuadTree->getAABB();
	}
	//---------------------------------------------------------------------
	Real Terrain::getBoundingRadius() const
	{
		if (!mQuadTree)
			return 0;
		else
			return mQuadTree->getBoundingRadius();
	}
	//---------------------------------------------------------------------
	void Terrain::save(StreamSerialiser& stream)
	{
		stream.writeChunkBegin(TERRAIN_CHUNK_ID, TERRAIN_CHUNK_VERSION);

		uint8 align = (uint8)mAlign;
		stream.write(&align);

		stream.write(&mSize);
		stream.write(&mWorldSize);
		uint16 splatDimCount;
		if (mSplatTextureWorldSize.empty())
		{
			splatDimCount = 1;
			Real dim = getSplatTextureWorldSize(0);
			stream.write(&splatDimCount);
			stream.write(&dim);
		}
		else
		{
			splatDimCount = static_cast<uint16>(mSplatTextureWorldSize.size());
			stream.write(&splatDimCount);
			stream.write(&mSplatTextureWorldSize[0], mSplatTextureWorldSize.size());
		}
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

		copyGlobalOptions();

		if (!stream.readChunkBegin(TERRAIN_CHUNK_ID, TERRAIN_CHUNK_VERSION))
			return false;

		uint8 align;
		stream.read(&align);
		mAlign = (Alignment)align;
		stream.read(&mWorldSize);
		uint16 splatDimCount;
		stream.read(&splatDimCount);
		for (uint16 i = 0; i < splatDimCount; ++i)
		{
			Real dim;
			stream.read(&dim);
			setSplatTextureWorldSize(i, dim);
		}
		stream.read(&mMaxBatchSize);
		stream.read(&mMinBatchSize);
		stream.read(&mPos);
		updateBaseScale();
		determineLodLevels();


		size_t numVertices = mSize * mSize;
		mHeightData = OGRE_ALLOC_T(float, numVertices, MEMCATEGORY_GEOMETRY);
		stream.read(mHeightData, numVertices);

		stream.readChunkEnd(TERRAIN_CHUNK_ID);

		mQuadTree = OGRE_NEW TerrainQuadTreeNode(this, 0, 0, 0, mSize, mNumLodLevels - 1, 0, 0);
		mQuadTree->prepare();

		mDeltaData = OGRE_ALLOC_T(float, numVertices, MEMCATEGORY_GEOMETRY);
		// calculate entire terrain
		Rect rect;
		rect.top = 0; rect.bottom = mSize;
		rect.left = 0; rect.right = mSize;
		calculateHeightDeltas(rect);

		distributeVertexData();

		return true;
	}
	//---------------------------------------------------------------------
	bool Terrain::prepare(const ImportData& importData)
	{
		freeCPUResources();

		copyGlobalOptions();

		// validate
		if (!(Bitwise::isPO2(importData.terrainSize - 1) && Bitwise::isPO2(importData.minBatchSize - 1)
			&& Bitwise::isPO2(importData.maxBatchSize - 1)))
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
				"terrainSise, minBatchSize and maxBatchSize must all be n^2 + 1", 
				"Terrain::prepare");
		}

		if (importData.minBatchSize > importData.maxBatchSize)
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
				"minBatchSize must be less than or equal to maxBatchSize",
				"Terrain::prepare");
		}

		if (importData.maxBatchSize > TERRAIN_MAX_BATCH_SIZE)
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
				"maxBatchSize must be no larger than " + 
					StringConverter::toString(TERRAIN_MAX_BATCH_SIZE),
				"Terrain::prepare");
		}

		mAlign = importData.terrainAlign;
		mSize = importData.terrainSize;
		mWorldSize = importData.worldSize;
		for (uint16 i = 0; i < importData.splatTextureWorldSizeList.size(); ++i)
		{
			setSplatTextureWorldSize(i, importData.splatTextureWorldSizeList[i]);
		}
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
		else if (importData.inputImage)
		{
			Image* img = importData.inputImage;

			if (img->getWidth() != mSize || img->getHeight() != mSize)
				img->resize(mSize, mSize);

			// convert image data to floats
			// Do this on a row-by-row basis, because we describe the terrain in
			// a bottom-up fashion (ie ascending world coords), while Image is top-down
			unsigned char* pSrcBase = img->getData();
			for (size_t i = 0; i < mSize; ++i)
			{
				size_t srcy = mSize - i - 1;
				unsigned char* pSrc = pSrcBase + srcy * img->getRowSpan();
				float* pDst = mHeightData + i * mSize;
				PixelUtil::bulkPixelConversion(pSrc, img->getFormat(), 
					pDst, PF_FLOAT32_R, mSize);
			}

			if (!Math::RealEqual(importData.inputBias, 0.0) || !Math::RealEqual(importData.inputScale, 1.0))
			{
				float* pAdj = mHeightData;
				for (size_t i = 0; i < numVertices; ++i)
				{
					*pAdj = (*pAdj * importData.inputScale) + importData.inputBias;	
					++pAdj;
				}
			}

		}
		else
		{
			// start with flat terrain
			memset(mHeightData, 0, sizeof(float) * mSize * mSize);
		}

		mDeltaData = OGRE_ALLOC_T(float, numVertices, MEMCATEGORY_GEOMETRY);


		mQuadTree = OGRE_NEW TerrainQuadTreeNode(this, 0, 0, 0, mSize, mNumLodLevels - 1, 0, 0);
		mQuadTree->prepare();

		// calculate entire terrain
		Rect rect;
		rect.top = 0; rect.bottom = mSize;
		rect.left = 0; rect.right = mSize;
		calculateHeightDeltas(rect);

		distributeVertexData();


		return true;

	}
	//---------------------------------------------------------------------
	void Terrain::copyGlobalOptions()
	{
		mUseTriangleStrips = TerrainGlobalOptions::getUseTriangleStrips();
		mUseLodMorph = TerrainGlobalOptions::getUseLodMorph();
		mSkirtSize = TerrainGlobalOptions::getSkirtSize();
		mGenerateVertexNormals = TerrainGlobalOptions::getGenerateVertexNormals();
		mGenerateNormalMap = TerrainGlobalOptions::getGenerateNormalMap();
		mGenerateShadowMap = TerrainGlobalOptions::getGenerateShadowMap();
		mGenerateHorizonMap = TerrainGlobalOptions::getGenerateHorizonMap();
		mRenderQueueGroup = TerrainGlobalOptions::getRenderQueueGroup();

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
				TreeDepth = log2((size - 1) / (maxBatch - 1)) + 1

			.. it's just that at the max LOD, the terrain is divided into 
			(size - 1) / (maxBatch - 1) tiles each of maxBatch vertices, and
			at the lowest LOD the terrain is made up of one single tile of 
			minBatch vertices. 

			Example: size = 257, minBatch = 17, maxBatch = 33

			LODlevels = log2(257 - 1) - log2(17 - 1) + 1 = 8 - 4 + 1 = 5
			TreeDepth = log2((size - 1) / (maxBatch - 1)) + 1 = 4

			LOD list - this assumes everything changes at once, which rarely happens of course
			           in fact except where groupings must occur, tiles can change independently
			LOD 0: 257 vertices, 8 x 33 vertex tiles (tree depth 3)
			LOD 1: 129 vertices, 8 x 17 vertex tiles (tree depth 3)
			LOD 2: 65  vertices, 4 x 17 vertex tiles (tree depth 2)
			LOD 3: 33  vertices, 2 x 17 vertex tiles (tree depth 1)
			LOD 4: 17  vertices, 1 x 17 vertex tiles (tree depth 0)

			Notice how we only have 2 sizes of index buffer to be concerned about,
			17 vertices (per side) or 33. This makes buffer re-use much easier while
			still giving the full range of LODs.
		*/
		mNumLodLevelsPerLeafNode = Math::Log2(mMaxBatchSize - 1) - Math::Log2(mMinBatchSize - 1) + 1;
		mNumLodLevels = Math::Log2(mSize - 1) - Math::Log2(mMinBatchSize - 1) + 1;
		//mTreeDepth = Math::Log2(mMaxBatchSize - 1) - Math::Log2(mMinBatchSize - 1) + 2;
		mTreeDepth = mNumLodLevels - mNumLodLevelsPerLeafNode + 1;

		LogManager::getSingleton().stream() << "Terrain created; size=" << mSize
			<< " minBatch=" << mMinBatchSize << " maxBatch=" << mMaxBatchSize
			<< " treeDepth=" << mTreeDepth << " lodLevels=" << mNumLodLevels 
			<< " leafLods=" << mNumLodLevelsPerLeafNode;
	}
	//---------------------------------------------------------------------
	void Terrain::distributeVertexData()
	{
		/* Now we need to figure out how to distribute vertex data. We want to 
		use 16-bit indexes for compatibility, which means that the maximum patch
		size that we can address (even sparsely for lower LODs) is 129x129 
		(the next one up, 257x257 is too big). 

		So we need to split the vertex data into chunks of 129. The number of 
		primary tiles this creates also indicates the point above which in
		the node tree that we can no longer merge tiles at lower LODs without
		using different vertex data. For example, using the 257x257 input example
		above, the vertex data would have to be split in 2 (in each dimension)
		in order to fit within the 129x129 range. This data could be shared by
		all tree depths from 1 onwards, it's just that LODs 3-1 would sample 
		the 129x129 data sparsely. LOD 0 would sample all of the vertices.

		LOD 4 however, the lowest LOD, could not work with the same vertex data
		because it needs to cover the entire terrain. There are 2 choices here:
		create another set of vertex data at 17x17 which is only used by LOD 4, 
		or make LOD 4 occur at tree depth 1 instead (ie still split up, and 
		rendered as 2x9 along each edge instead. 

		Since rendering very small batches is not desirable, and the vertex counts
		are inherently not going to be large, creating a separate vertex set is
		preferable. This will also be more efficient on the vertex cache with
		distant terrains. 

		We probably need a larger example, because in this case only 1 level (LOD 0)
		needs to use this separate vertex data. Higher detail terrains will need
		it for multiple levels, here's a 2049x2049 example with 65 / 33 batch settings:

		LODlevels = log2(2049 - 1) - log2(33 - 1) + 1 = 11 - 5 + 1 = 7
		TreeDepth = log2((2049 - 1) / (65 - 1)) + 1 = 6
		Number of vertex data splits at most detailed level: 
		(size - 1) / (TERRAIN_MAX_BATCH_SIZE - 1) = 2048 / 128 = 16

		LOD 0: 2049 vertices, 32 x 65 vertex tiles (tree depth 5) vdata 0-15  [129x16]
		LOD 1: 1025 vertices, 32 x 33 vertex tiles (tree depth 5) vdata 0-15  [129x16]
		LOD 2: 513  vertices, 16 x 33 vertex tiles (tree depth 4) vdata 0-15  [129x16]
		LOD 3: 257  vertices, 8  x 33 vertex tiles (tree depth 3) vdata 16-17 [129x2] 
		LOD 4: 129  vertices, 4  x 33 vertex tiles (tree depth 2) vdata 16-17 [129x2]
		LOD 5: 65   vertices, 2  x 33 vertex tiles (tree depth 1) vdata 16-17 [129x2]
		LOD 6: 33   vertices, 1  x 33 vertex tiles (tree depth 0) vdata 18    [33]

		All the vertex counts are to be squared, they are just along one edge. 
		So as you can see, we need to have 3 levels of vertex data to satisy this
		(admittedly quite extreme) case, and a total of 19 sets of vertex data. 
		The full detail geometry, which is  16(x16) sets of 129(x129), used by 
		LODs 0-2. LOD 3 can't use this set because it needs to group across them, 
		because it has only 8 tiles, so we make another set which satisfies this 
		at a maximum of 129 vertices per vertex data section. In this case LOD 
		3 needs 257(x257) total vertices so we still split into 2(x2) sets of 129. 
		This set is good up to and including LOD 5, but LOD 6 needs a single 
		contiguous set of vertices, so we make a 33x33 vertex set for it. 

		In terms of vertex data stored, this means that while our primary data is:
		2049^2 = 4198401 vertices
		our final stored vertex data is 
		(16 * 129)^2 + (2 * 129)^2 + 33^2 = 4327749 vertices

		That equals a 3% premium, but it's both necessary and worth it for the
		reduction in batch count resulting from the grouping. In addition, at
		LODs 3 and 6 (or rather tree depth 3 and 0) there is the opportunity 
		to free up the vertex data used by more detailed LODs, which is
		important when dealing with large terrains. For example, if we
		freed the (GPU) vertex data for LOD 0-2 in the medium distance, 
		we would save 98% of the memory overhead for this terrain. 

		*/

		LogManager& logMgr = LogManager::getSingleton();
		logMgr.stream(LML_TRIVIAL) << "Terrain::distributeVertexData processing source "
			"terrain size of " << mSize;

		uint16 depth = mTreeDepth;
		uint16 prevdepth = depth;
		uint16 currresolution = mSize;
		uint16 bakedresolution = mSize;
		uint16 targetSplits = (bakedresolution - 1) / (TERRAIN_MAX_BATCH_SIZE - 1);
		while(depth-- && targetSplits)
		{
			uint splits = 1 << depth;
			if (splits == targetSplits)
			{
				logMgr.stream(LML_TRIVIAL) << "  Assigning vertex data, resolution="
					<< bakedresolution << " startDepth=" << depth << " endDepth=" << prevdepth
					<< " splits=" << splits;
				// vertex data goes at this level, at bakedresolution
				// applies to all lower levels (except those with a closer vertex data)
				mQuadTree->assignVertexData(depth, prevdepth, bakedresolution);

				// next set to look for
				bakedresolution =  ((currresolution - 1) >> 1) + 1;
				targetSplits = (bakedresolution - 1) / (TERRAIN_MAX_BATCH_SIZE - 1);
				prevdepth = depth;

			}

			currresolution = ((currresolution - 1) >> 1) + 1;


		}

		// Always assign vertex data to the top of the tree
		if (prevdepth > 0)
		{
			mQuadTree->assignVertexData(0, 1, bakedresolution);
			logMgr.stream(LML_TRIVIAL) << "  Assigning vertex data, resolution: "
				<< getMinBatchSize() << " startDepth: 0 endDepth: 1 splits: 1";

		}

		logMgr.stream(LML_TRIVIAL) << "Terrain::distributeVertexData finished";

	}
	//---------------------------------------------------------------------
	void Terrain::load()
	{
		if (mQuadTree)
			mQuadTree->load();
	}
	//---------------------------------------------------------------------
	void Terrain::unload()
	{
		if (mQuadTree)
			mQuadTree->unload();
	}
	//---------------------------------------------------------------------
	void Terrain::unprepare()
	{
		if (mQuadTree)
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
	float Terrain::getHeight(long x, long y)
	{
		return *getHeightData(x, y);
	}
	//---------------------------------------------------------------------
	const float* Terrain::getDeltaData()
	{
		return mDeltaData;
	}
	//---------------------------------------------------------------------
	const float* Terrain::getDeltaData(long x, long y)
	{
		assert (x >= 0 && x < mSize && y >= 0 && y < mSize);
		return &mDeltaData[y * mSize + x];
	}
	//---------------------------------------------------------------------
	void Terrain::getPoint(long x, long y, Vector3* outpos)
	{
		getPointAlign(x, y, mAlign, outpos);
	}
	//---------------------------------------------------------------------
	void Terrain::getPoint(long x, long y, float height, Vector3* outpos)
	{
		getPointAlign(x, y, height, mAlign, outpos);
	}
	//---------------------------------------------------------------------
	void Terrain::getPointAlign(long x, long y, Alignment align, Vector3* outpos)
	{
		getPointAlign(x, y, *getHeightData(x, y), align, outpos);
	}
	//---------------------------------------------------------------------
	void Terrain::getPointAlign(long x, long y, float height, Alignment align, Vector3* outpos)
	{
		switch(align)
		{
		case ALIGN_X_Z:
			outpos->y = height;
			outpos->x = x * mScale + mBase;
			outpos->z = y * -mScale - mBase;
			break;
		case ALIGN_Y_Z:
			outpos->x = height;
			outpos->y = x * mScale + mBase;
			outpos->z = y * -mScale - mBase;
			break;
		case ALIGN_X_Y:
			outpos->z = height;
			outpos->x = x * mScale + mBase;
			outpos->y = y * mScale + mBase;
			break;
		};

	}
	//---------------------------------------------------------------------
	void Terrain::getVector(const Vector3& inVec, Vector3* outVec)
	{
		getVectorAlign(inVec.x, inVec.y, inVec.z, mAlign, outVec);
	}
	//---------------------------------------------------------------------
	void Terrain::getVector(Real x, Real y, Real z, Vector3* outVec)
	{
		getVectorAlign(x, y, z, mAlign, outVec);
	}
	//---------------------------------------------------------------------
	void Terrain::getVectorAlign(const Vector3& inVec, Alignment align, Vector3* outVec)
	{
		getVectorAlign(inVec.x, inVec.y, inVec.z, align, outVec);
	}
	//---------------------------------------------------------------------
	void Terrain::getVectorAlign(Real x, Real y, Real z, Alignment align, Vector3* outVec)
	{

		switch(align)
		{
		case ALIGN_X_Z:
			outVec->y = z;
			outVec->x = x;
			outVec->z = -y;
			break;
		case ALIGN_Y_Z:
			outVec->x = z;
			outVec->y = y;
			outVec->z = -x;
			break;
		case ALIGN_X_Y:
			outVec->x = x;
			outVec->y = y;
			outVec->z = z;
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
	Real Terrain::getSplatTextureWorldSize(uint16 index) const
	{
		if (index < mSplatTextureWorldSize.size())
		{
			return mSplatTextureWorldSize[index];
		}
		else if (!mSplatTextureWorldSize.empty())
		{
			return mSplatTextureWorldSize[0];
		}
		else
		{
			// default to tile 100 times
			return mWorldSize * 0.01;
		}
	}
	//---------------------------------------------------------------------
	void Terrain::setSplatTextureWorldSize(uint16 index, Real size)
	{
		if (index >= mSplatTextureWorldSize.size())
		{
			mSplatTextureWorldSize.resize(index + 1);
			mSplatTextureUVMultiplier.resize(index + 1);
		}

		mSplatTextureWorldSize[index] = size;
		mSplatTextureUVMultiplier[index] = mWorldSize / size;
	}
	//---------------------------------------------------------------------
	Real Terrain::getSplatTextureUVMultipler(uint16 index) const
	{
		if (index < mSplatTextureUVMultiplier.size())
		{
			return mSplatTextureUVMultiplier[index];
		}
		else if (!mSplatTextureUVMultiplier.empty())
		{
			return mSplatTextureUVMultiplier[0];
		}
		else
		{
			// default to tile 100 times
			return 100;
		}
	}
	//---------------------------------------------------------------------
	void Terrain::setPosition(const Vector3& pos)
	{
		mPos = pos;
		mRootNode->setPosition(pos);
		updateBaseScale();
	}
	//---------------------------------------------------------------------
	SceneNode* Terrain::_getRootSceneNode() const
	{
		return mRootNode;
	}
	//---------------------------------------------------------------------
	void Terrain::updateBaseScale()
	{
		// centre the terrain on local origin
		mBase = -mWorldSize * 0.5; 
		// scale determines what 1 unit on the grid becomes in world space
		mScale =  mWorldSize / (Real)(mSize-1);
	}
	//---------------------------------------------------------------------
	void Terrain::dirty()
	{
		// TODO - geometry

		// calculate entire terrain
		Rect rect;
		rect.top = 0; rect.bottom = mSize;
		rect.left = 0; rect.right = mSize;
		calculateHeightDeltas(rect);
	}
	//---------------------------------------------------------------------
	void Terrain::dirtyRect(const Rect& rect)
	{
		// TODO - geometry

		calculateHeightDeltas(rect);

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
		Rect clampedRect(rect);
		clampedRect.left = std::max(0L, clampedRect.left);
		clampedRect.top = std::max(0L, clampedRect.top);
		clampedRect.right = std::min((long)mSize, clampedRect.right);
		clampedRect.bottom = std::min((long)mSize, clampedRect.bottom);

		mQuadTree->preDeltaCalculation(clampedRect);

		/// Iterate over target levels, 
		for (int targetLevel = 1; targetLevel < mNumLodLevels; ++targetLevel)
		{
			int sourceLevel = targetLevel - 1;
			int step = 1 << targetLevel;
			// The step of the next higher LOD
			int higherstep = step >> 1;

			// round the rectangle at this level so that it starts & ends on 
			// the step boundaries
			Rect lodRect(clampedRect);
			lodRect.left -= lodRect.left % step;
			lodRect.top -= lodRect.top % step;
			if (lodRect.right % step)
				lodRect.right += step - (lodRect.right % step);
			if (lodRect.bottom % step)
				lodRect.bottom += step - (lodRect.bottom % step);

			for (int j = lodRect.top; j < lodRect.bottom - step; j += step )
			{
				for (int i = lodRect.left; i < lodRect.right - step; i += step )
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

							// max(delta) is the worst case scenario at this LOD
							// compared to the original heightmap

							// tell the quadtree about this 
							mQuadTree->notifyDelta(fulldetailx, fulldetailz, sourceLevel, delta);


							// If this vertex is being removed at this LOD, 
							// then save the height difference since that's the move
							// it will need to make. Vertices to be removed at this LOD
							// are halfway between the steps
							if ((fulldetailx % step) == step / 2 || (fulldetailz % step) == step / 2)
							{
								// Save height difference 
								mDeltaData[fulldetailx + (fulldetailz * mSize)] = delta;
							}

						}

					}
				} // i
			} // j
		} // targetLevel

		mQuadTree->postDeltaCalculation(clampedRect);

	}

	//---------------------------------------------------------------------
	uint16 Terrain::getResolutionAtLod(uint16 lodLevel)
	{
		return ((mSize - 1) >> lodLevel) + 1;
	}
	//---------------------------------------------------------------------
	void Terrain::preFindVisibleObjects(SceneManager* source, 
		SceneManager::IlluminationRenderStage irs, Viewport* v)
	{
		// only calculate LOD on main render passes
		if (irs == SceneManager::IRS_NONE)
			calculateCurrentLod(v);
	}
	//---------------------------------------------------------------------
	void Terrain::sceneManagerDestroyed(SceneManager* source)
	{
		unload();
		unprepare();
		if (source == mSceneMgr)
			mSceneMgr = 0;
	}
	//---------------------------------------------------------------------
	void Terrain::calculateCurrentLod(Viewport* vp)
	{
		if (mQuadTree)
		{
			// calculate error terms
			const Camera* cam = vp->getCamera()->getLodCamera();

			// A = 1 / tan(fovy)    (== 1 for fovy=45)
			Real A = 1.0 / Math::Tan(cam->getFOVy());
			// T = 2 * maxPixelError / vertRes
			Real maxPixelError = TerrainGlobalOptions::getMaxPixelError() * cam->_getLodBiasInverse();
			Real T = 2.0 * maxPixelError / (Real)vp->getActualHeight();

			// CFactor = A / T
			Real cFactor = A / T;

			// CFactor should be squared before being used
			cFactor = Math::Sqr(cFactor);

			mQuadTree->calculateCurrentLod(cam, cFactor);
		}
	}
	//---------------------------------------------------------------------
	std::pair<bool, Vector3> Terrain::rayIntersects(const Ray& ray) 
	{
		typedef std::pair<bool, Vector3> Result;
		// first step: convert the ray to a local vertex space
		// we assume terrain to be in the x-z plane, with the [0,0] vertex
		// at origin and a plane distance of 1 between vertices.
		// This makes calculations easier.
		Vector3 rayOrigin = ray.getOrigin() - getPosition();
		Vector3 rayDirection = ray.getDirection();
		// relabel axes (probably wrong? need correct coordinate transformation)
		switch (getAlignment())
		{
			case ALIGN_X_Y:
				std::swap(rayOrigin.y, rayOrigin.z);
				std::swap(rayDirection.y, rayDirection.z);
				break;
			case ALIGN_Y_Z:
				std::swap(rayOrigin.x, rayOrigin.y);
				std::swap(rayDirection.x, rayDirection.y);
				break;
			case ALIGN_X_Z:
				break;
		}
		// readjust coordinate origin
		rayOrigin.x += mWorldSize/2;
		rayOrigin.z += mWorldSize/2;
		// scale down to vertex level
		rayOrigin.x /= mScale;
		rayOrigin.z /= mScale;
		rayDirection.x /= mScale;
		rayDirection.z /= mScale;
		rayDirection.normalise();
		Ray localRay (rayOrigin, rayDirection);
		
		// test if the ray actually hits the terrain's bounds
		// TODO: Replace y values with actual heightmap height limits
		AxisAlignedBox aabb (Vector3(0, 0, 0), Vector3(mSize, 1000000, mSize));
		std::pair<bool, Real> aabbTest = localRay.intersects(aabb);
		if (!aabbTest.first)
			return Result(false, Vector3());
		// get intersection point and move inside
		Vector3 cur = localRay.getPoint(aabbTest.second);
		
		// now check every quad the ray touches
		int quadX = std::min(std::max(static_cast<int>(cur.x), 0), (int)mSize-2);
		int quadZ = std::min(std::max(static_cast<int>(cur.z), 0), (int)mSize-2);
		int flipX = (rayDirection.x < 0 ? 0 : 1);
		int flipZ = (rayDirection.z < 0 ? 0 : 1);
		int xDir = (rayDirection.x < 0 ? -1 : 1);
		int zDir = (rayDirection.z < 0 ? -1 : 1);
		Result result;
		while (cur.y >= -1 && cur.y <= 2)
		{
			if (quadX < 0 || quadX >= (int)mSize-1 || quadZ < 0 || quadZ >= (int)mSize-1)
				break;
			
			result = checkQuadIntersection(quadX, quadZ, localRay);
			if (result.first)
				break;
			
			// determine next quad to test
			Real xDist = (quadX - cur.x + flipX) / rayDirection.x;
			Real zDist = (quadZ - cur.z + flipZ) / rayDirection.z;
			if (xDist < zDist)
			{
				quadX += xDir;
				cur += rayDirection * xDist;
			}
			else
			{
				quadZ += zDir;
				cur += rayDirection * zDist;
			}
			
		}
		
		if (result.first)
		{
			// transform the point of intersection back to world space
			result.second.x *= mScale;
			result.second.z *= mScale;
			result.second.x -= mWorldSize/2;
			result.second.z -= mWorldSize/2;
			switch (getAlignment())
			{
				case ALIGN_X_Y:
					std::swap(result.second.y, result.second.z);
					break;
				case ALIGN_Y_Z:
					std::swap(result.second.x, result.second.y);
					break;
				case ALIGN_X_Z:
					break;
			}
			result.second += getPosition();
		}
		return result;
	}
	//---------------------------------------------------------------------
	std::pair<bool, Vector3> Terrain::checkQuadIntersection(int x, int z, const Ray& ray)
	{
		// build the two planes belonging to the quad's triangles
		Vector3 v1 (x, *getHeightData(x,z), z);
		Vector3 v2 (x+1, *getHeightData(x+1,z), z);
		Vector3 v3 (x, *getHeightData(x,z+1), z+1);
		Vector3 v4 (x+1, *getHeightData(x+1,z+1), z+1);
		// TODO: Is this the correct triangle order?
		Plane p1 (v1, v3, v2);
		Plane p2 (v3, v4, v2);
		
		// Test for intersection with the two planes. 
		// Then test that the intersection points are actually
		// still inside the triangle (with a small error margin)
		std::pair<bool, Real> planeInt = ray.intersects(p1);
		if (planeInt.first)
		{
			Vector3 where = ray.getPoint(planeInt.second);
			Vector3 rel = where - v1;
			if (rel.x >= -0.01 && rel.x <= 1.01 && rel.z >= -0.01 && rel.z <= 1.01 && rel.x+rel.z <= 1.01)
				return std::pair<bool, Vector3>(true, where);
		}
		planeInt = ray.intersects(p2);
		if (planeInt.first)
		{
			Vector3 where = ray.getPoint(planeInt.second);
			Vector3 rel = where - v1;
			if (rel.x >= -0.01 && rel.x <= 1.01 && rel.z >= -0.01 && rel.z <= 1.01 && rel.x+rel.z >= 0.99)
				return std::pair<bool, Vector3>(true, where);
		}
		
		return std::pair<bool, Vector3>(false, Vector3());
	}
	//---------------------------------------------------------------------
	const MaterialPtr& Terrain::getMaterial() const
	{
		// TODO - generate material

		return mMaterial;
	}

	
	
}

