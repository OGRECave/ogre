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

namespace Ogre
{
	//---------------------------------------------------------------------
	const uint32 Terrain::TERRAIN_CHUNK_ID = StreamSerialiser::makeIdentifier("TERR");
	const uint16 Terrain::TERRAIN_CHUNK_VERSION = 1;
	//---------------------------------------------------------------------
	Terrain::Terrain()
		: mHeightData(0)
		, mQuadTree(0)
	{

	}
	//---------------------------------------------------------------------
	Terrain::~Terrain()
	{
		freeGPUResources();
		freeCPUResources();
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
		stream.write(mHeightData, mSize * mSize);

		stream.writeChunkEnd(TERRAIN_CHUNK_ID);
	}
	//---------------------------------------------------------------------
	bool Terrain::prepare(StreamSerialiser& stream)
	{
		freeCPUResources();

		if (!stream.readChunkBegin(TERRAIN_CHUNK_ID, TERRAIN_CHUNK_VERSION))
			return false;

		uint8 align;
		stream.read(&align);
		stream.read(&mWorldSize);
		stream.read(&mMaxBatchSize);
		stream.read(&mMinBatchSize);

		size_t numVertices = mSize * mSize;
		mHeightData = OGRE_ALLOC_T(float, numVertices, MEMCATEGORY_GEOMETRY);
		stream.read(mHeightData, numVertices);

		stream.readChunkEnd(TERRAIN_CHUNK_ID);

		return true;
	}
	//---------------------------------------------------------------------
	bool Terrain::prepare(const ImportData& importData)
	{
		freeCPUResources();

		mAlign = importData.terrainAlign;
		mSize = importData.terrainSize;
		mWorldSize = importData.worldSize;
		mMaxBatchSize = importData.maxBatchSize;
		mMinBatchSize = importData.minBatchSize;

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

		mQuadTree = OGRE_NEW TerrainQuadTreeNode(this, 0, 0, 0, mSize);

		return true;

	}
	//---------------------------------------------------------------------
	void Terrain::load()
	{
		// TODO
	}
	//---------------------------------------------------------------------
	void Terrain::unload()
	{
		// TODO
	}
	//---------------------------------------------------------------------
	void Terrain::unprepare()
	{
		// TODO
	}
	//---------------------------------------------------------------------
	float* Terrain::getHeightData()
	{
		return mHeightData;
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

		OGRE_DELETE mQuadTree;
		mQuadTree = 0;



	}
	//---------------------------------------------------------------------
	void Terrain::freeGPUResources()
	{
		// delete batched geometry

		// SHARE geometry between Terrain instances!

	}



}

