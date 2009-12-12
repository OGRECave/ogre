/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
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
#include "OgreHardwarePixelBuffer.h"
#include "OgreTextureManager.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreRay.h"
#include "OgrePlane.h"
#include "OgreTerrainMaterialGeneratorA.h"
#include "OgreMaterialManager.h"


#if OGRE_COMPILER == OGRE_COMPILER_MSVC
// we do lots of conversions here, casting them all is tedious & cluttered, we know what we're doing
#   pragma warning (disable : 4244)
#endif
namespace Ogre
{
	//---------------------------------------------------------------------
	const uint32 Terrain::TERRAIN_CHUNK_ID = StreamSerialiser::makeIdentifier("TERR");
	const uint16 Terrain::TERRAIN_CHUNK_VERSION = 1;
	const uint32 Terrain::TERRAINLAYERDECLARATION_CHUNK_ID = StreamSerialiser::makeIdentifier("TDCL");
	const uint16 Terrain::TERRAINLAYERDECLARATION_CHUNK_VERSION = 1;
	const uint32 Terrain::TERRAINLAYERSAMPLER_CHUNK_ID = StreamSerialiser::makeIdentifier("TSAM");;
	const uint16 Terrain::TERRAINLAYERSAMPLER_CHUNK_VERSION = 1;
	const uint32 Terrain::TERRAINLAYERSAMPLERELEMENT_CHUNK_ID = StreamSerialiser::makeIdentifier("TSEL");;
	const uint16 Terrain::TERRAINLAYERSAMPLERELEMENT_CHUNK_VERSION = 1;
	const uint32 Terrain::TERRAINLAYERINSTANCE_CHUNK_ID = StreamSerialiser::makeIdentifier("TLIN");;
	const uint16 Terrain::TERRAINLAYERINSTANCE_CHUNK_VERSION = 1;
	const uint32 Terrain::TERRAINDERIVEDDATA_CHUNK_ID = StreamSerialiser::makeIdentifier("TDDA");;
	const uint16 Terrain::TERRAINDERIVEDDATA_CHUNK_VERSION = 1;
	// since 129^2 is the greatest power we can address in 16-bit index
	const uint16 Terrain::TERRAIN_MAX_BATCH_SIZE = 129; 
	const uint16 Terrain::WORKQUEUE_DERIVED_DATA_REQUEST = 1;
	const size_t Terrain::LOD_MORPH_CUSTOM_PARAM = 1001;
	const uint8 Terrain::DERIVED_DATA_DELTAS = 1;
	const uint8 Terrain::DERIVED_DATA_NORMALS = 2;
	const uint8 Terrain::DERIVED_DATA_LIGHTMAP = 4;
	// This MUST match the bitwise OR of all the types above with no extra bits!
	const uint8 Terrain::DERIVED_DATA_ALL = 7;


	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	Real TerrainGlobalOptions::msSkirtSize = 30;
	Vector3 TerrainGlobalOptions::msLightMapDir = Vector3(1, -1, 0).normalisedCopy();
	bool TerrainGlobalOptions::msCastsShadows = false;
	Real TerrainGlobalOptions::msMaxPixelError = 3.0;
	uint8 TerrainGlobalOptions::msRenderQueueGroup = RENDER_QUEUE_MAIN;
	uint32 TerrainGlobalOptions::msVisibilityFlags = 0xFFFFFFFF;
	uint32 TerrainGlobalOptions::msQueryFlags = 0xFFFFFFFF;
	bool TerrainGlobalOptions::msUseRayBoxDistanceCalculation = false;
	TerrainMaterialGeneratorPtr TerrainGlobalOptions::msDefaultMaterialGenerator;
	uint16 TerrainGlobalOptions::msLayerBlendMapSize = 1024;
	Real TerrainGlobalOptions::msDefaultLayerTextureWorldSize = 10;
	uint16 TerrainGlobalOptions::msDefaultGlobalColourMapSize = 1024;
	uint16 TerrainGlobalOptions::msLightmapSize = 1024;
	uint16 TerrainGlobalOptions::msCompositeMapSize = 1024;
	ColourValue TerrainGlobalOptions::msCompositeMapAmbient = ColourValue::White;
	ColourValue TerrainGlobalOptions::msCompositeMapDiffuse = ColourValue::White;
	Real TerrainGlobalOptions::msCompositeMapDistance = 4000;
	String TerrainGlobalOptions::msResourceGroup = ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME;
	//---------------------------------------------------------------------
	void TerrainGlobalOptions::setDefaultMaterialGenerator(TerrainMaterialGeneratorPtr gen)
	{
		msDefaultMaterialGenerator = gen;
	}
	//---------------------------------------------------------------------
	TerrainMaterialGeneratorPtr TerrainGlobalOptions::getDefaultMaterialGenerator()
	{
		if (msDefaultMaterialGenerator.isNull())
		{
			// default
			msDefaultMaterialGenerator.bind(OGRE_NEW TerrainMaterialGeneratorA());
		}

		return msDefaultMaterialGenerator;
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	NameGenerator Terrain::msBlendTextureGenerator = NameGenerator("TerrBlend");
	//---------------------------------------------------------------------
	Terrain::Terrain(SceneManager* sm)
		: mSceneMgr(sm)
		, mResourceGroup(StringUtil::BLANK)
		, mIsLoaded(false)
		, mModified(false)
		, mHeightData(0)
		, mDeltaData(0)
		, mPos(Vector3::ZERO)
		, mQuadTree(0)
		, mNumLodLevels(0)
		, mNumLodLevelsPerLeafNode(0)
		, mTreeDepth(0)
		, mDirtyGeometryRect(0, 0, 0, 0)
		, mDirtyDerivedDataRect(0, 0, 0, 0)
		, mDirtyGeometryRectForNeighbours(0, 0, 0, 0)
		, mDirtyLightmapFromNeighboursRect(0, 0, 0, 0)
		, mDerivedDataUpdateInProgress(false)
		, mDerivedUpdatePendingMask(0)
		, mMaterialGenerationCount(0)
		, mMaterialDirty(false)
		, mMaterialParamsDirty(false)
		, mGlobalColourMapSize(0)
		, mGlobalColourMapEnabled(false)
		, mCpuColourMapStorage(0)
		, mCpuLightmapStorage(0)
		, mCpuCompositeMapStorage(0)
		, mCompositeMapDirtyRect(0, 0, 0, 0)
		, mCompositeMapUpdateCountdown(0)
		, mLastMillis(0)
		, mCompositeMapDirtyRectLightmapUpdate(false)
		, mLodMorphRequired(false)
		, mNormalMapRequired(false)
		, mLightMapRequired(false)
		, mLightMapShadowsOnly(true)
		, mCompositeMapRequired(false)
		, mCpuTerrainNormalMap(0)
		, mLastLODCamera(0)
		, mLastLODFrame(0)

	{
		mRootNode = sm->getRootSceneNode()->createChildSceneNode();
		sm->addListener(this);

		WorkQueue* wq = Root::getSingleton().getWorkQueue();
		mWorkQueueChannel = wq->getChannel("Ogre/Terrain");
		wq->addRequestHandler(mWorkQueueChannel, this);
		wq->addResponseHandler(mWorkQueueChannel, this);

		// generate a material name, it's important for the terrain material
		// name to be consistent & unique no matter what generator is being used
		// so use our own pointer as identifier, use FashHash rather than just casting
		// the pointer to a long so we support 64-bit pointers
		Terrain* pTerrain = this;
		mMaterialName = "OgreTerrain/" + StringConverter::toString(FastHash((const char*)&pTerrain, sizeof(Terrain*)));

		memset(mNeighbours, 0, sizeof(Terrain*) * NEIGHBOUR_COUNT);
	}
	//---------------------------------------------------------------------
	Terrain::~Terrain()
	{
		mDerivedUpdatePendingMask = 0;
		waitForDerivedProcesses();
		WorkQueue* wq = Root::getSingleton().getWorkQueue();
		wq->removeRequestHandler(mWorkQueueChannel, this);
		wq->removeResponseHandler(mWorkQueueChannel, this);	

		freeTemporaryResources();
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
	AxisAlignedBox Terrain::getWorldAABB() const
	{
		Matrix4 m = Matrix4::IDENTITY;
		m.setTrans(getPosition());

		AxisAlignedBox ret = getAABB();
		ret.transformAffine(m);
		return ret;
	}
	//---------------------------------------------------------------------
	Real Terrain::getMinHeight() const
	{
		if (!mQuadTree)
			return 0;
		else
			return mQuadTree->getMinHeight();
	}
	//---------------------------------------------------------------------
	Real Terrain::getMaxHeight() const
	{
		if (!mQuadTree)
			return 0;
		else
			return mQuadTree->getMaxHeight();
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
	void Terrain::save(const String& filename)
	{
		DataStreamPtr stream = Root::getSingleton().createFileStream(filename, _getDerivedResourceGroup(), true);
		StreamSerialiser ser(stream);
		save(ser);
	}
	//---------------------------------------------------------------------
	void Terrain::save(StreamSerialiser& stream)
	{
		// wait for any queued processes to finish
		waitForDerivedProcesses();

		stream.writeChunkBegin(TERRAIN_CHUNK_ID, TERRAIN_CHUNK_VERSION);

		uint8 align = (uint8)mAlign;
		stream.write(&align);

		stream.write(&mSize);
		stream.write(&mWorldSize);
		stream.write(&mMaxBatchSize);
		stream.write(&mMinBatchSize);
		stream.write(&mPos);
		stream.write(mHeightData, mSize * mSize);

		writeLayerDeclaration(mLayerDecl, stream);

		// Layers
		checkLayers(false);
		uint8 numLayers = (uint8)mLayers.size();
		writeLayerInstanceList(mLayers, stream);

		// Packed layer blend data
		if(!mCpuBlendMapStorage.empty())
		{
			// save from CPU data if it's there, it means GPU data was never created
			stream.write(&mLayerBlendMapSize);

			// load packed CPU data
			int numBlendTex = getBlendTextureCount(numLayers);
			for (int i = 0; i < numBlendTex; ++i)
			{
				PixelFormat fmt = getBlendTextureFormat(i, numLayers);
				size_t channels = PixelUtil::getNumElemBytes(fmt);
				size_t dataSz = channels * mLayerBlendMapSize * mLayerBlendMapSize;
				uint8* pData = mCpuBlendMapStorage[i];
				stream.write(pData, dataSz);
			}
		}
		else
		{
			if (mLayerBlendMapSize != mLayerBlendMapSizeActual)
			{
				LogManager::getSingleton().stream() << 
					"WARNING: blend maps were requested at a size larger than was supported "
					"on this hardware, which means the quality has been degraded";
			}
			stream.write(&mLayerBlendMapSizeActual);
			uint8* tmpData = (uint8*)OGRE_MALLOC(mLayerBlendMapSizeActual * mLayerBlendMapSizeActual * 4, MEMCATEGORY_GENERAL);
			uint8 texIndex = 0;
			for (TexturePtrList::iterator i = mBlendTextureList.begin(); i != mBlendTextureList.end(); ++i, ++texIndex)
			{
				// Must blit back in CPU format!
				PixelFormat cpuFormat = getBlendTextureFormat(texIndex, getLayerCount());
				PixelBox dst(mLayerBlendMapSizeActual, mLayerBlendMapSizeActual, 1, cpuFormat, tmpData);
				(*i)->getBuffer()->blitToMemory(dst);
				size_t dataSz = PixelUtil::getNumElemBytes((*i)->getFormat()) * 
					mLayerBlendMapSizeActual * mLayerBlendMapSizeActual;
				stream.write(tmpData, dataSz);
			}
			OGRE_FREE(tmpData, MEMCATEGORY_GENERAL);
		}

		// other data
		// normals
		stream.writeChunkBegin(TERRAINDERIVEDDATA_CHUNK_ID, TERRAINDERIVEDDATA_CHUNK_VERSION);
		String normalDataType("normalmap");
		stream.write(&normalDataType);
		stream.write(&mSize);
		if (mCpuTerrainNormalMap)
		{
			// save from CPU data if it's there, it means GPU data was never created
			stream.write((uint8*)mCpuTerrainNormalMap->data, mSize * mSize * 3);
		}
		else
		{
			uint8* tmpData = (uint8*)OGRE_MALLOC(mSize * mSize * 3, MEMCATEGORY_GENERAL);
			PixelBox dst(mSize, mSize, 1, PF_BYTE_RGB, tmpData);
			mTerrainNormalMap->getBuffer()->blitToMemory(dst);
			stream.write(tmpData, mSize * mSize * 3);
			OGRE_FREE(tmpData, MEMCATEGORY_GENERAL);
		}
		stream.writeChunkEnd(TERRAINDERIVEDDATA_CHUNK_ID);


		// colourmap
		if (mGlobalColourMapEnabled)
		{
			stream.writeChunkBegin(TERRAINDERIVEDDATA_CHUNK_ID, TERRAINDERIVEDDATA_CHUNK_VERSION);
			String colourDataType("colourmap");
			stream.write(&colourDataType);
			stream.write(&mSize);
			if (mCpuColourMapStorage)
			{
				// save from CPU data if it's there, it means GPU data was never created
				stream.write(mCpuColourMapStorage, mGlobalColourMapSize * mGlobalColourMapSize * 3);
			}
			else
			{
				uint8* tmpData = (uint8*)OGRE_MALLOC(mGlobalColourMapSize * mGlobalColourMapSize * 3, MEMCATEGORY_GENERAL);
				PixelBox dst(mGlobalColourMapSize, mGlobalColourMapSize, 1, PF_BYTE_RGB, tmpData);
				mColourMap->getBuffer()->blitToMemory(dst);
				stream.write(tmpData, mGlobalColourMapSize * mGlobalColourMapSize * 3);
				OGRE_FREE(tmpData, MEMCATEGORY_GENERAL);
			}
			stream.writeChunkEnd(TERRAINDERIVEDDATA_CHUNK_ID);

		}

		// lightmap
		if (mLightMapRequired)
		{
			stream.writeChunkBegin(TERRAINDERIVEDDATA_CHUNK_ID, TERRAINDERIVEDDATA_CHUNK_VERSION);
			String lightmapDataType("lightmap");
			stream.write(&lightmapDataType);
			stream.write(&mLightmapSize);
			if (mCpuLightmapStorage)
			{
				// save from CPU data if it's there, it means GPU data was never created
				stream.write(mCpuLightmapStorage, mLightmapSize * mLightmapSize);
			}
			else
			{
				uint8* tmpData = (uint8*)OGRE_MALLOC(mLightmapSize * mLightmapSize, MEMCATEGORY_GENERAL);
				PixelBox dst(mLightmapSize, mLightmapSize, 1, PF_L8, tmpData);
				mLightmap->getBuffer()->blitToMemory(dst);
				stream.write(tmpData, mLightmapSize * mLightmapSize);
				OGRE_FREE(tmpData, MEMCATEGORY_GENERAL);
			}
			stream.writeChunkEnd(TERRAINDERIVEDDATA_CHUNK_ID);
		}

		// composite map
		if (mCompositeMapRequired)
		{
			stream.writeChunkBegin(TERRAINDERIVEDDATA_CHUNK_ID, TERRAINDERIVEDDATA_CHUNK_VERSION);
			String compositeMapDataType("compositemap");
			stream.write(&compositeMapDataType);
			stream.write(&mCompositeMapSize);
			if (mCpuCompositeMapStorage)
			{
				// save from CPU data if it's there, it means GPU data was never created
				stream.write(mCpuCompositeMapStorage, mCompositeMapSize * mCompositeMapSize * 4);
			}
			else
			{
				// composite map is 4 channel, 3x diffuse, 1x specular mask
				uint8* tmpData = (uint8*)OGRE_MALLOC(mCompositeMapSize * mCompositeMapSize * 4, MEMCATEGORY_GENERAL);
				PixelBox dst(mCompositeMapSize, mCompositeMapSize, 1, PF_BYTE_RGBA, tmpData);
				mCompositeMap->getBuffer()->blitToMemory(dst);
				stream.write(tmpData, mCompositeMapSize * mCompositeMapSize * 4);
				OGRE_FREE(tmpData, MEMCATEGORY_GENERAL);
			}
			stream.writeChunkEnd(TERRAINDERIVEDDATA_CHUNK_ID);
		}

		// TODO - write deltas

		stream.writeChunkEnd(TERRAIN_CHUNK_ID);

		mModified = false;

	}
	//---------------------------------------------------------------------
	void Terrain::writeLayerDeclaration(const TerrainLayerDeclaration& decl, StreamSerialiser& stream)
	{
		// Layer declaration
		stream.writeChunkBegin(TERRAINLAYERDECLARATION_CHUNK_ID, TERRAINLAYERDECLARATION_CHUNK_VERSION);
		//  samplers
		uint8 numSamplers = (uint8)decl.samplers.size();
		stream.write(&numSamplers);
		for (TerrainLayerSamplerList::const_iterator i = decl.samplers.begin(); 
			i != decl.samplers.end(); ++i)
		{
			const TerrainLayerSampler& sampler = *i;
			stream.writeChunkBegin(TERRAINLAYERSAMPLER_CHUNK_ID, TERRAINLAYERSAMPLER_CHUNK_VERSION);
			stream.write(&sampler.alias);
			uint8 pixFmt = (uint8)sampler.format;
			stream.write(&pixFmt);
			stream.writeChunkEnd(TERRAINLAYERSAMPLER_CHUNK_ID);
		}
		//  elements
		uint8 numElems = (uint8)decl.elements.size();
		stream.write(&numElems);
		for (TerrainLayerSamplerElementList::const_iterator i = decl.elements.begin(); 
			i != decl.elements.end(); ++i)
		{
			const TerrainLayerSamplerElement& elem= *i;
			stream.writeChunkBegin(TERRAINLAYERSAMPLERELEMENT_CHUNK_ID, TERRAINLAYERSAMPLERELEMENT_CHUNK_VERSION);
			stream.write(&elem.source);
			uint8 sem = (uint8)elem.semantic;
			stream.write(&sem);
			stream.write(&elem.elementStart);
			stream.write(&elem.elementCount);
			stream.writeChunkEnd(TERRAINLAYERSAMPLERELEMENT_CHUNK_ID);
		}
		stream.writeChunkEnd(TERRAINLAYERDECLARATION_CHUNK_ID);
	}
	//---------------------------------------------------------------------
	bool Terrain::readLayerDeclaration(StreamSerialiser& stream, TerrainLayerDeclaration& targetdecl)
	{
		if (!stream.readChunkBegin(TERRAINLAYERDECLARATION_CHUNK_ID, TERRAINLAYERDECLARATION_CHUNK_VERSION))
			return false;
		//  samplers
		uint8 numSamplers;
		stream.read(&numSamplers);
		targetdecl.samplers.resize(numSamplers);
		for (uint8 s = 0; s < numSamplers; ++s)
		{
			if (!stream.readChunkBegin(TERRAINLAYERSAMPLER_CHUNK_ID, TERRAINLAYERSAMPLER_CHUNK_VERSION))
				return false;

			stream.read(&(targetdecl.samplers[s].alias));
			uint8 pixFmt;
			stream.read(&pixFmt);
			targetdecl.samplers[s].format = (PixelFormat)pixFmt;
			stream.readChunkEnd(TERRAINLAYERSAMPLER_CHUNK_ID);
		}
		//  elements
		uint8 numElems;
		stream.read(&numElems);
		targetdecl.elements.resize(numElems);
		for (uint8 e = 0; e < numElems; ++e)
		{
			if (!stream.readChunkBegin(TERRAINLAYERSAMPLERELEMENT_CHUNK_ID, TERRAINLAYERSAMPLERELEMENT_CHUNK_VERSION))
				return false;

			stream.read(&(targetdecl.elements[e].source));
			uint8 sem;
			stream.read(&sem);
			targetdecl.elements[e].semantic = (TerrainLayerSamplerSemantic)sem;
			stream.read(&(targetdecl.elements[e].elementStart));
			stream.read(&(targetdecl.elements[e].elementCount));
			stream.readChunkEnd(TERRAINLAYERSAMPLERELEMENT_CHUNK_ID);
		}
		stream.readChunkEnd(TERRAINLAYERDECLARATION_CHUNK_ID);

		return true;
	}
	//---------------------------------------------------------------------
	void Terrain::writeLayerInstanceList(const Terrain::LayerInstanceList& layers, StreamSerialiser& stream)
	{
		uint8 numLayers = (uint8)layers.size();
		stream.write(&numLayers);
		for (LayerInstanceList::const_iterator i = layers.begin(); i != layers.end(); ++i)
		{
			const LayerInstance& inst = *i;
			stream.writeChunkBegin(TERRAINLAYERINSTANCE_CHUNK_ID, TERRAINLAYERINSTANCE_CHUNK_VERSION);
			stream.write(&inst.worldSize);
			for (StringVector::const_iterator t = inst.textureNames.begin(); 
				t != inst.textureNames.end(); ++t)
			{
				stream.write(&(*t));
			}
			stream.writeChunkEnd(TERRAINLAYERINSTANCE_CHUNK_ID);
		}

	}
	//---------------------------------------------------------------------
	bool Terrain::readLayerInstanceList(StreamSerialiser& stream, size_t numSamplers, Terrain::LayerInstanceList& targetlayers)
	{
		uint8 numLayers;
		stream.read(&numLayers);
		targetlayers.resize(numLayers);
		for (uint8 l = 0; l < numLayers; ++l)
		{
			if (!stream.readChunkBegin(TERRAINLAYERINSTANCE_CHUNK_ID, TERRAINLAYERINSTANCE_CHUNK_VERSION))
				return false;
			stream.read(&targetlayers[l].worldSize);
			targetlayers[l].textureNames.resize(numSamplers);
			for (size_t t = 0; t < numSamplers; ++t)
			{
				stream.read(&(targetlayers[l].textureNames[t]));
			}
			stream.readChunkEnd(TERRAINLAYERINSTANCE_CHUNK_ID);
		}

		return true;
	}
	//---------------------------------------------------------------------
	const String& Terrain::_getDerivedResourceGroup() const
	{
		if (mResourceGroup.empty())
			return TerrainGlobalOptions::getDefaultResourceGroup();
		else
			return mResourceGroup;
	}
	//---------------------------------------------------------------------
	bool Terrain::prepare(const String& filename)
	{
		DataStreamPtr stream = Root::getSingleton().openFileStream(filename, 
			_getDerivedResourceGroup());
		StreamSerialiser ser(stream);
		return prepare(ser);
	}
	//---------------------------------------------------------------------
	bool Terrain::prepare(StreamSerialiser& stream)
	{
		freeTemporaryResources();
		freeCPUResources();

		copyGlobalOptions();

		if (!stream.readChunkBegin(TERRAIN_CHUNK_ID, TERRAIN_CHUNK_VERSION))
			return false;

		uint8 align;
		stream.read(&align);
		mAlign = (Alignment)align;
		stream.read(&mSize);
		stream.read(&mWorldSize);

		stream.read(&mMaxBatchSize);
		stream.read(&mMinBatchSize);
		stream.read(&mPos);
		updateBaseScale();
		determineLodLevels();

		size_t numVertices = mSize * mSize;
		mHeightData = OGRE_ALLOC_T(float, numVertices, MEMCATEGORY_GEOMETRY);
		stream.read(mHeightData, numVertices);


		// Layer declaration
		if (!readLayerDeclaration(stream, mLayerDecl))
			return false;
		checkDeclaration();


		// Layers
		if (!readLayerInstanceList(stream, mLayerDecl.samplers.size(), mLayers))
			return false;
		deriveUVMultipliers();

		// Packed layer blend data
		uint8 numLayers = (uint8)mLayers.size();
		stream.read(&mLayerBlendMapSize);
		mLayerBlendMapSizeActual = mLayerBlendMapSize; // for now, until we check
		// load packed CPU data
		int numBlendTex = getBlendTextureCount(numLayers);
		for (int i = 0; i < numBlendTex; ++i)
		{
			PixelFormat fmt = getBlendTextureFormat(i, numLayers);
			size_t channels = PixelUtil::getNumElemBytes(fmt);
			size_t dataSz = channels * mLayerBlendMapSize * mLayerBlendMapSize;
			uint8* pData = (uint8*)OGRE_MALLOC(dataSz, MEMCATEGORY_RESOURCE);
			stream.read(pData, dataSz);
			mCpuBlendMapStorage.push_back(pData);
		}

		// derived data
		while (!stream.isEndOfChunk(TERRAIN_CHUNK_ID) && 
			stream.peekNextChunkID() == TERRAINDERIVEDDATA_CHUNK_ID)
		{
			stream.readChunkBegin(TERRAINDERIVEDDATA_CHUNK_ID, TERRAINDERIVEDDATA_CHUNK_VERSION);
			// name
			String name;
			stream.read(&name);
			uint16 sz;
			stream.read(&sz);
			if (name == "normalmap")
			{
				uint8* pData = static_cast<uint8*>(OGRE_MALLOC(sz * sz * 3, MEMCATEGORY_GENERAL));
				mCpuTerrainNormalMap = OGRE_NEW PixelBox(sz, sz, 1, PF_BYTE_RGB, pData);

				stream.read(pData, sz * sz * 3);
				
			}
			else if (name == "colourmap")
			{
				mGlobalColourMapEnabled = true;
				mGlobalColourMapSize = sz;
				mCpuColourMapStorage = static_cast<uint8*>(OGRE_MALLOC(sz * sz * 3, MEMCATEGORY_GENERAL));
				stream.read(mCpuColourMapStorage, sz * sz * 3);
			}
			else if (name == "lightmap")
			{
				mLightMapRequired = true;
				mLightmapSize = sz;
				mCpuLightmapStorage = static_cast<uint8*>(OGRE_MALLOC(sz * sz, MEMCATEGORY_GENERAL));
				stream.read(mCpuLightmapStorage, sz * sz);
			}
			else if (name == "compositemap")
			{
				mCompositeMapRequired = true;
				mCompositeMapSize = sz;
				mCpuCompositeMapStorage = static_cast<uint8*>(OGRE_MALLOC(sz * sz * 4, MEMCATEGORY_GENERAL));
				stream.read(mCpuCompositeMapStorage, sz * sz * 4);
			}

			stream.readChunkEnd(TERRAINDERIVEDDATA_CHUNK_ID);


		}

		stream.readChunkEnd(TERRAIN_CHUNK_ID);

		mQuadTree = OGRE_NEW TerrainQuadTreeNode(this, 0, 0, 0, mSize, mNumLodLevels - 1, 0, 0);
		mQuadTree->prepare();

		mDeltaData = OGRE_ALLOC_T(float, numVertices, MEMCATEGORY_GEOMETRY);
		memset(mDeltaData, 0, sizeof(float) * numVertices);
		// calculate entire terrain
		Rect rect;
		rect.top = 0; rect.bottom = mSize;
		rect.left = 0; rect.right = mSize;
		calculateHeightDeltas(rect);
		finaliseHeightDeltas(rect, true);

		distributeVertexData();

		mModified = false;

		return true;
	}
	//---------------------------------------------------------------------
	bool Terrain::prepare(const ImportData& importData)
	{
		freeTemporaryResources();
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
		mLayerDecl = importData.layerDeclaration;
		checkDeclaration();
		mLayers = importData.layerList;
		checkLayers(false);
		deriveUVMultipliers();
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
			if (importData.constantHeight == 0)
				memset(mHeightData, 0, sizeof(float) * mSize * mSize);
			else
			{
				float* pFloat = mHeightData;
				for (long i = 0 ; i < mSize * mSize; ++i)
					*pFloat++ = importData.constantHeight;

			}
		}

		mDeltaData = OGRE_ALLOC_T(float, numVertices, MEMCATEGORY_GEOMETRY);
		memset(mDeltaData, 0, sizeof(float) * numVertices);

		mQuadTree = OGRE_NEW TerrainQuadTreeNode(this, 0, 0, 0, mSize, mNumLodLevels - 1, 0, 0);
		mQuadTree->prepare();

		// calculate entire terrain
		Rect rect;
		rect.top = 0; rect.bottom = mSize;
		rect.left = 0; rect.right = mSize;
		calculateHeightDeltas(rect);
		finaliseHeightDeltas(rect, true);

		distributeVertexData();

		// Imported data is treated as modified because it's not saved
		mModified = true;


		return true;

	}
	//---------------------------------------------------------------------
	void Terrain::copyGlobalOptions()
	{
		mSkirtSize = TerrainGlobalOptions::getSkirtSize();
		mRenderQueueGroup = TerrainGlobalOptions::getRenderQueueGroup();
		mVisibilityFlags = TerrainGlobalOptions::getVisibilityFlags();
		mQueryFlags = TerrainGlobalOptions::getQueryFlags();
		mLayerBlendMapSize = TerrainGlobalOptions::getLayerBlendMapSize();
		mLayerBlendMapSizeActual = mLayerBlendMapSize; // for now, until we check
		mLightmapSize = TerrainGlobalOptions::getLightMapSize();
		mLightmapSizeActual = mLightmapSize; // for now, until we check
		mCompositeMapSize = TerrainGlobalOptions::getCompositeMapSize();
		mCompositeMapSizeActual = mCompositeMapSize; // for now, until we check

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
		mNumLodLevelsPerLeafNode = Math::Log2(mMaxBatchSize - 1.0f) - Math::Log2(mMinBatchSize - 1.0f) + 1.0f;
		mNumLodLevels = Math::Log2(mSize - 1.0f) - Math::Log2(mMinBatchSize - 1.0f) + 1.0f;
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
				// determine physical size (as opposed to resolution)
				size_t sz = ((bakedresolution-1) / splits) + 1;
				mQuadTree->assignVertexData(depth, prevdepth, bakedresolution, sz);

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
			mQuadTree->assignVertexData(0, 1, bakedresolution, bakedresolution);
			logMgr.stream(LML_TRIVIAL) << "  Assigning vertex data, resolution: "
				<< bakedresolution << " startDepth=0 endDepth=1 splits=1";

		}

		logMgr.stream(LML_TRIVIAL) << "Terrain::distributeVertexData finished";

	}
	//---------------------------------------------------------------------
	void Terrain::load(const String& filename)
	{
		if (prepare(filename))
			load();
		else
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
			"Error while preparing " + filename + ", see log for details", 
			__FUNCTION__);
	}
	//---------------------------------------------------------------------
	void Terrain::load(StreamSerialiser& stream)
	{
		if (prepare(stream))
			load();
		else
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
			"Error while preparing from stream, see log for details", 
			__FUNCTION__);
	}
	//---------------------------------------------------------------------
	void Terrain::load()
	{
		if (mIsLoaded)
			return;

		if (mQuadTree)
			mQuadTree->load();
		
		checkLayers(true);
		createOrDestroyGPUColourMap();
		createOrDestroyGPUNormalMap();
		createOrDestroyGPULightmap();
		createOrDestroyGPUCompositeMap();
		
		mMaterialGenerator->requestOptions(this);

		mIsLoaded = true;

	}
	//---------------------------------------------------------------------
	void Terrain::unload()
	{
		if (!mIsLoaded)
			return;

		if (mQuadTree)
			mQuadTree->unload();

		mIsLoaded = false;
		mModified = false;

	}
	//---------------------------------------------------------------------
	void Terrain::unprepare()
	{
		if (mQuadTree)
			mQuadTree->unprepare();
	}
	//---------------------------------------------------------------------
	float* Terrain::getHeightData() const
	{
		return mHeightData;
	}
	//---------------------------------------------------------------------
	float* Terrain::getHeightData(long x, long y) const
	{
		assert (x >= 0 && x < mSize && y >= 0 && y < mSize);
		return &mHeightData[y * mSize + x];
	}
	//---------------------------------------------------------------------
	float Terrain::getHeightAtPoint(long x, long y) const
	{
		// clamp
		x = std::min(x, (long)mSize - 1L);
		x = std::max(x, 0L);
		y = std::min(y, (long)mSize - 1L);
		y = std::max(y, 0L);

		return *getHeightData(x, y);
	}
	//---------------------------------------------------------------------
	void Terrain::setHeightAtPoint(long x, long y, float h)
	{
		// clamp
		x = std::min(x, (long)mSize - 1L);
		x = std::max(x, 0L);
		y = std::min(y, (long)mSize - 1L);
		y = std::max(y, 0L);

		*getHeightData(x, y) = h;
		Rect rect;
		rect.left = x;
		rect.right = x+1;
		rect.top = y;
		rect.bottom = y+1;
		dirtyRect(rect);
	}
	//---------------------------------------------------------------------
	float Terrain::getHeightAtTerrainPosition(Real x, Real y)
	{
		// get left / bottom points (rounded down)
		Real factor = (Real)mSize - 1.0f;
		Real invFactor = 1.0f / factor;

		long startX = static_cast<long>(x * factor);
		long startY = static_cast<long>(y * factor);
		long endX = startX + 1;
		long endY = startY + 1;

		// now get points in terrain space (effectively rounding them to boundaries)
		// note that we do not clamp! We need a valid plane
		Real startXTS = startX * invFactor;
		Real startYTS = startY * invFactor;
		Real endXTS = endX * invFactor;
		Real endYTS = endY * invFactor;

		// now clamp
		endX = std::min(endX, (long)mSize-1);
		endY = std::min(endY, (long)mSize-1);

		// get parametric from start coord to next point
		Real xParam = (x - startXTS) / invFactor;
		Real yParam = (y - startYTS) / invFactor;


		/* For even / odd tri strip rows, triangles are this shape:
		even     odd
		3---2   3---2
		| / |   | \ |
		0---1   0---1
		*/

		// Build all 4 positions in terrain space, using point-sampled height
		Vector3 v0 (startXTS, startYTS, getHeightAtPoint(startX, startY));
		Vector3 v1 (endXTS, startYTS, getHeightAtPoint(endX, startY));
		Vector3 v2 (endXTS, endYTS, getHeightAtPoint(endX, endY));
		Vector3 v3 (startXTS, endYTS, getHeightAtPoint(startX, endY));
		// define this plane in terrain space
		Plane plane;
		if (startY % 2)
		{
			// odd row
			bool secondTri = ((1.0 - yParam) > xParam);
			if (secondTri)
				plane.redefine(v0, v1, v3);
			else
				plane.redefine(v1, v2, v3);
		}
		else
		{
			// even row
			bool secondTri = (yParam > xParam);
			if (secondTri)
				plane.redefine(v0, v2, v3);
			else
				plane.redefine(v0, v1, v2);
		}

		// Solve plane equation for z
		return (-plane.normal.x * x 
				-plane.normal.y * y
				- plane.d) / plane.normal.z;


	}
	//---------------------------------------------------------------------
	float Terrain::getHeightAtWorldPosition(Real x, Real y, Real z)
	{
		Vector3 terrPos;
		getTerrainPosition(x, y, z, &terrPos);
		return getHeightAtTerrainPosition(terrPos.x, terrPos.y);
	}
	//---------------------------------------------------------------------
	float Terrain::getHeightAtWorldPosition(const Vector3& pos)
	{
		return getHeightAtWorldPosition(pos.x, pos.y, pos.z);
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
	Vector3 Terrain::convertPosition(Space inSpace, const Vector3& inPos, Space outSpace) const
	{
		Vector3 ret;
		convertPosition(inSpace, inPos, outSpace, ret);
		return ret;
	}
	//---------------------------------------------------------------------
	Vector3 Terrain::convertDirection(Space inSpace, const Vector3& inDir, Space outSpace) const
	{
		Vector3 ret;
		convertDirection(inSpace, inDir, outSpace, ret);
		return ret;
	}
	//---------------------------------------------------------------------
	void Terrain::convertPosition(Space inSpace, const Vector3& inPos, Space outSpace, Vector3& outPos) const
	{
		convertSpace(inSpace, inPos, outSpace, outPos, true);
	}
	//---------------------------------------------------------------------
	void Terrain::convertDirection(Space inSpace, const Vector3& inDir, Space outSpace, Vector3& outDir) const
	{
		convertSpace(inSpace, inDir, outSpace, outDir, false);
	}
	//---------------------------------------------------------------------
	void Terrain::convertSpace(Space inSpace, const Vector3& inVec, Space outSpace, Vector3& outVec, bool translation) const
	{
		Space currSpace = inSpace;
		outVec = inVec;
		while (currSpace != outSpace)
		{
			switch(currSpace)
			{
			case WORLD_SPACE:
				// In all cases, transition to local space
				outVec = outVec - mPos;
				currSpace = LOCAL_SPACE;
				break;
			case LOCAL_SPACE:
				switch(outSpace)
				{
				case WORLD_SPACE:
					if (translation)
						outVec += mPos;
					currSpace = WORLD_SPACE;
					break;
				case POINT_SPACE:
				case TERRAIN_SPACE:
					// go via terrain space
					outVec = convertWorldToTerrainAxes(outVec);
					if (translation)
						outVec.x -= mBase; outVec.y -= mBase;
					outVec.x /= (mSize - 1) * mScale; outVec.y /= (mSize - 1) * mScale;
					currSpace = TERRAIN_SPACE;
					break;
				};
				break;
			case TERRAIN_SPACE:
				switch(outSpace)
				{
				case WORLD_SPACE:
				case LOCAL_SPACE:
					// go via local space
					outVec.x *= (mSize - 1) * mScale; outVec.y *= (mSize - 1) * mScale;
					if (translation)
						outVec.x += mBase; outVec.y += mBase;
					outVec = convertTerrainToWorldAxes(outVec);
					currSpace = LOCAL_SPACE;
					break;
				case POINT_SPACE:
					outVec.x *= (mSize - 1); outVec.y *= (mSize - 1); 
					// rounding up/down
					// this is why POINT_SPACE is the last on the list, because it loses data
					outVec.x = static_cast<Real>(static_cast<int>(outVec.x + 0.5));
					outVec.y = static_cast<Real>(static_cast<int>(outVec.y + 0.5));
					currSpace = POINT_SPACE;
					break;
				};
				break;
			case POINT_SPACE:
				// always go via terrain space
				outVec.x /= (mSize - 1); outVec.y /= (mSize - 1); 
				currSpace = TERRAIN_SPACE;
				break;

			};
		}

	}
	//---------------------------------------------------------------------
	void Terrain::convertWorldToTerrainAxes(Alignment align, const Vector3& worldVec, Vector3* terrainVec) 
	{
		switch (align)
		{
		case ALIGN_X_Z:
			terrainVec->z = worldVec.y;
			terrainVec->x = worldVec.x;
			terrainVec->y = -worldVec.z;
			break;
		case ALIGN_Y_Z:
			terrainVec->z = worldVec.x;
			terrainVec->x = -worldVec.z;
			terrainVec->y = worldVec.y;
			break;
		case ALIGN_X_Y:
			*terrainVec = worldVec;
			break;
		};

	}
	//---------------------------------------------------------------------
	void Terrain::convertTerrainToWorldAxes(Alignment align, const Vector3& terrainVec, Vector3* worldVec)
	{
		switch (align)
		{
		case ALIGN_X_Z:
			worldVec->x = terrainVec.x;
			worldVec->y = terrainVec.z;
			worldVec->z = -terrainVec.y;
			break;
		case ALIGN_Y_Z:
			worldVec->x = terrainVec.z;
			worldVec->y = terrainVec.y;
			worldVec->z = -terrainVec.x;
			break;
		case ALIGN_X_Y:
			*worldVec = terrainVec;
			break;
		};

	}
	//---------------------------------------------------------------------
	Vector3 Terrain::convertWorldToTerrainAxes(const Vector3& inVec) const
	{
		Vector3 ret;
		convertWorldToTerrainAxes(mAlign, inVec, &ret);

		return ret;
	}
	//---------------------------------------------------------------------
	Vector3 Terrain::convertTerrainToWorldAxes(const Vector3& inVec) const
	{
		Vector3 ret;
		convertTerrainToWorldAxes(mAlign, inVec, &ret);

		return ret;
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
			outpos->z = x * -mScale - mBase;
			outpos->y = y * mScale + mBase;
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
	void Terrain::getPosition(const Vector3& TSpos, Vector3* outWSpos)
	{
		getPositionAlign(TSpos, mAlign, outWSpos);
	}
	//---------------------------------------------------------------------
	void Terrain::getPosition(Real x, Real y, Real z, Vector3* outWSpos)
	{
		getPositionAlign(x, y, z, mAlign, outWSpos);
	}
	//---------------------------------------------------------------------
	void Terrain::getTerrainPosition(const Vector3& WSpos, Vector3* outTSpos)
	{
		getTerrainPositionAlign(WSpos, mAlign, outTSpos);
	}
	//---------------------------------------------------------------------
	void Terrain::getTerrainPosition(Real x, Real y, Real z, Vector3* outTSpos)
	{
		getTerrainPositionAlign(x, y, z, mAlign, outTSpos);
	}
	//---------------------------------------------------------------------
	void Terrain::getPositionAlign(const Vector3& TSpos, Alignment align, Vector3* outWSpos)
	{
		getPositionAlign(TSpos.x, TSpos.y, TSpos.z, align, outWSpos);
	}
	//---------------------------------------------------------------------
	void Terrain::getPositionAlign(Real x, Real y, Real z, Alignment align, Vector3* outWSpos)
	{
		switch(align)
		{
		case ALIGN_X_Z:
			outWSpos->y = z;
			outWSpos->x = x * (mSize - 1) * mScale + mBase;
			outWSpos->z = y * (mSize - 1) * -mScale - mBase;
			break;
		case ALIGN_Y_Z:
			outWSpos->x = z;
			outWSpos->y = y * (mSize - 1) * mScale + mBase;
			outWSpos->z = x * (mSize - 1) * -mScale - mBase;
			break;
		case ALIGN_X_Y:
			outWSpos->z = z;
			outWSpos->x = x * (mSize - 1) * mScale + mBase;
			outWSpos->y = y * (mSize - 1) * mScale + mBase;
			break;
		};

	}
	//---------------------------------------------------------------------
	void Terrain::getTerrainPositionAlign(const Vector3& WSpos, Alignment align, Vector3* outTSpos)
	{
		getTerrainPositionAlign(WSpos.x, WSpos.y, WSpos.z, align, outTSpos);
	}
	//---------------------------------------------------------------------
	void Terrain::getTerrainPositionAlign(Real x, Real y, Real z, Alignment align, Vector3* outTSpos)
	{
		switch(align)
		{
		case ALIGN_X_Z:
			outTSpos->x = (x - mBase - mPos.x) / ((mSize - 1) * mScale);
			outTSpos->y = (z + mBase - mPos.z) / ((mSize - 1) * -mScale);
			outTSpos->z = y;
			break;
		case ALIGN_Y_Z:
			outTSpos->x = (z - mBase - mPos.z) / ((mSize - 1) * -mScale);
			outTSpos->y = (y + mBase - mPos.y) / ((mSize - 1) * mScale);
			outTSpos->z = x;
			break;
		case ALIGN_X_Y:
			outTSpos->x = (x - mBase - mPos.x) / ((mSize - 1) * mScale);
			outTSpos->y = (y - mBase - mPos.y) / ((mSize - 1) * mScale);
			outTSpos->z = z;
			break;
		};

	}
	//---------------------------------------------------------------------
	void Terrain::getTerrainVector(const Vector3& inVec, Vector3* outVec)
	{
		getTerrainVectorAlign(inVec.x, inVec.y, inVec.z, mAlign, outVec);
	}
	//---------------------------------------------------------------------
	void Terrain::getTerrainVector(Real x, Real y, Real z, Vector3* outVec)
	{
		getTerrainVectorAlign(x, y, z, mAlign, outVec);
	}
	//---------------------------------------------------------------------
	void Terrain::getTerrainVectorAlign(const Vector3& inVec, Alignment align, Vector3* outVec)
	{
		getTerrainVectorAlign(inVec.x, inVec.y, inVec.z, align, outVec);
	}
	//---------------------------------------------------------------------
	void Terrain::getTerrainVectorAlign(Real x, Real y, Real z, Alignment align, Vector3* outVec)
	{

		switch(align)
		{
		case ALIGN_X_Z:
			outVec->z = y;
			outVec->x = x;
			outVec->y = -z;
			break;
		case ALIGN_Y_Z:
			outVec->z = x;
			outVec->y = y;
			outVec->x = -z;
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
	Real Terrain::getLayerWorldSize(uint8 index) const
	{
		if (index < mLayers.size())
		{
			return mLayers[index].worldSize;
		}
		else if (!mLayers.empty())
		{
			return mLayers[0].worldSize;
		}
		else
		{
			return TerrainGlobalOptions::getDefaultLayerTextureWorldSize();
		}
	}
	//---------------------------------------------------------------------
	void Terrain::setLayerWorldSize(uint8 index, Real size)
	{
		if (index < mLayers.size())
		{

			if (index >= mLayerUVMultiplier.size())
				mLayerUVMultiplier.resize(index + 1);

			mLayers[index].worldSize = size;
			mLayerUVMultiplier[index] = mWorldSize / size;
			mMaterialParamsDirty = true;
		}
	}
	//---------------------------------------------------------------------
	Real Terrain::getLayerUVMultiplier(uint8 index) const
	{
		if (index < mLayerUVMultiplier.size())
		{
			return mLayerUVMultiplier[index];
		}
		else if (!mLayerUVMultiplier.empty())
		{
			return mLayerUVMultiplier[0];
		}
		else
		{
			// default to tile 100 times
			return 100;
		}
	}
	//---------------------------------------------------------------------
	void Terrain::deriveUVMultipliers()
	{
		mLayerUVMultiplier.resize(mLayers.size());
		for (size_t i = 0; i < mLayers.size(); ++i)
		{
			const LayerInstance& inst = mLayers[i];

			mLayerUVMultiplier[i] = mWorldSize / inst.worldSize;

		}
	}
	//---------------------------------------------------------------------
	const String& Terrain::getLayerTextureName(uint8 layerIndex, uint8 samplerIndex) const
	{
		if (layerIndex < mLayers.size() && samplerIndex < mLayerDecl.samplers.size())
		{
			return mLayers[layerIndex].textureNames[samplerIndex];
		}
		else
		{
			return StringUtil::BLANK;
		}

	}
	//---------------------------------------------------------------------
	void Terrain::setLayerTextureName(uint8 layerIndex, uint8 samplerIndex, const String& textureName)
	{
		if (layerIndex < mLayers.size() && samplerIndex < mLayerDecl.samplers.size())
		{
			if (mLayers[layerIndex].textureNames[samplerIndex] != textureName)
			{
				mLayers[layerIndex].textureNames[samplerIndex] = textureName;
				mMaterialDirty = true;
				mMaterialParamsDirty = true;
			}
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
		Rect rect;
		rect.top = 0; rect.bottom = mSize;
		rect.left = 0; rect.right = mSize;
		dirtyRect(rect);
	}
	//---------------------------------------------------------------------
	void Terrain::dirtyRect(const Rect& rect)
	{
		mDirtyGeometryRect.merge(rect);
		mDirtyGeometryRectForNeighbours.merge(rect);
		mDirtyDerivedDataRect.merge(rect);
		mCompositeMapDirtyRect.merge(rect);

		mModified = true;

	}
	//---------------------------------------------------------------------
	void Terrain::_dirtyCompositeMapRect(const Rect& rect)
	{
		mCompositeMapDirtyRect.merge(rect);
		mModified = true;
	}
	//---------------------------------------------------------------------
	void Terrain::update(bool synchronous)
	{
		updateGeometry();
		updateDerivedData(synchronous);
	}
	//---------------------------------------------------------------------
	void Terrain::updateGeometry()
	{
		if (!mDirtyGeometryRect.isNull())
		{
			mQuadTree->updateVertexData(true, false, mDirtyGeometryRect, false);
			mDirtyGeometryRect.setNull();
		}

		// propagate changes
		notifyNeighbours();
	}
	//---------------------------------------------------------------------
	void Terrain::updateDerivedData(bool synchronous, uint8 typeMask)
	{
		if (!mDirtyDerivedDataRect.isNull() || !mDirtyLightmapFromNeighboursRect.isNull())
		{
			mModified = true;
			if (mDerivedDataUpdateInProgress)
			{
				// Don't launch many updates, instead wait for the other one 
				// to finish and issue another afterwards. 
				mDerivedUpdatePendingMask |= typeMask;
			}
			else
			{
				updateDerivedDataImpl(mDirtyDerivedDataRect, mDirtyLightmapFromNeighboursRect, 
					synchronous, typeMask);
				mDirtyDerivedDataRect.setNull();
				mDirtyLightmapFromNeighboursRect.setNull();
			}
		}
		else
		{
			// Usually the composite map is updated after the other background
			// data is updated (no point doing it beforehand), but if there's
			// nothing to update, then we'll do it right now.
			updateCompositeMap();
		}


	}
	//---------------------------------------------------------------------
	void Terrain::updateDerivedDataImpl(const Rect& rect, const Rect& lightmapExtraRect, 
		bool synchronous, uint8 typeMask)
	{
		mDerivedDataUpdateInProgress = true;
		mDerivedUpdatePendingMask = 0;

		DerivedDataRequest req;
		req.terrain = this;
		req.dirtyRect = rect;
		req.lightmapExtraDirtyRect = lightmapExtraRect;
		req.typeMask = typeMask;
		if (!mNormalMapRequired)
			req.typeMask = req.typeMask & ~DERIVED_DATA_NORMALS;
		if (!mLightMapRequired)
			req.typeMask = req.typeMask & ~DERIVED_DATA_LIGHTMAP;

		Root::getSingleton().getWorkQueue()->addRequest(
			mWorkQueueChannel, WORKQUEUE_DERIVED_DATA_REQUEST, 
			Any(req), 0, synchronous);

	}
	//---------------------------------------------------------------------
	void Terrain::waitForDerivedProcesses()
	{
		while (mDerivedDataUpdateInProgress)
		{
			// we need to wait for this to finish
			OGRE_THREAD_SLEEP(50);
			Root::getSingleton().getWorkQueue()->processResponses();
		}

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

		if (mCpuTerrainNormalMap)
		{
			OGRE_FREE(mCpuTerrainNormalMap->data, MEMCATEGORY_GENERAL);
			OGRE_DELETE mCpuTerrainNormalMap;
			mCpuTerrainNormalMap = 0;
		}

		OGRE_FREE(mCpuColourMapStorage, MEMCATEGORY_GENERAL);
		mCpuColourMapStorage = 0;

		OGRE_FREE(mCpuLightmapStorage, MEMCATEGORY_GENERAL);
		mCpuLightmapStorage = 0;

		OGRE_FREE(mCpuCompositeMapStorage, MEMCATEGORY_GENERAL);
		mCpuCompositeMapStorage = 0;
	}
	//---------------------------------------------------------------------
	void Terrain::freeGPUResources()
	{
		// remove textures
		TextureManager* tmgr = TextureManager::getSingletonPtr();
		if (tmgr)
		{
			for (TexturePtrList::iterator i = mBlendTextureList.begin(); i != mBlendTextureList.end(); ++i)
			{	
				tmgr->remove((*i)->getHandle());
			}
			mBlendTextureList.clear();
		}

		if (!mTerrainNormalMap.isNull())
		{
			tmgr->remove(mTerrainNormalMap->getHandle());
			mTerrainNormalMap.setNull();
		}

		if (!mColourMap.isNull())
		{
			tmgr->remove(mColourMap->getHandle());
			mColourMap.setNull();
		}

		if (!mLightmap.isNull())
		{
			tmgr->remove(mLightmap->getHandle());
			mLightmap.setNull();
		}

		if (!mCompositeMap.isNull())
		{
			tmgr->remove(mCompositeMap->getHandle());
			mCompositeMap.setNull();
		}

		if (!mMaterial.isNull())
		{
			MaterialManager::getSingleton().remove(mMaterial->getHandle());
			mMaterial.setNull();
		}
		if (!mCompositeMapMaterial.isNull())
		{
			MaterialManager::getSingleton().remove(mCompositeMapMaterial->getHandle());
			mCompositeMapMaterial.setNull();
		}


	}
	//---------------------------------------------------------------------
	Rect Terrain::calculateHeightDeltas(const Rect& rect)
	{
		Rect clampedRect(rect);
		clampedRect.left = std::max(0L, clampedRect.left);
		clampedRect.top = std::max(0L, clampedRect.top);
		clampedRect.right = std::min((long)mSize, clampedRect.right);
		clampedRect.bottom = std::min((long)mSize, clampedRect.bottom);

		Rect finalRect(clampedRect);

		mQuadTree->preDeltaCalculation(clampedRect);

		/// Iterate over target levels, 
		for (int targetLevel = 1; targetLevel < mNumLodLevels; ++targetLevel)
		{
			int sourceLevel = targetLevel - 1;
			int step = 1 << targetLevel;
			// The step of the next higher LOD
//			int higherstep = step >> 1;

			// need to widen the dirty rectangle since change will affect surrounding
			// vertices at lower LOD
			Rect widenedRect(rect);
			widenedRect.left = std::max(0L, widenedRect.left - step);
			widenedRect.top = std::max(0L, widenedRect.top - step);
			widenedRect.right = std::min((long)mSize, widenedRect.right + step);
			widenedRect.bottom = std::min((long)mSize, widenedRect.bottom + step);

			// keep a merge of the widest
			finalRect.merge(widenedRect);
			

			// now round the rectangle at this level so that it starts & ends on 
			// the step boundaries
			Rect lodRect(widenedRect);
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
					// For even tri strip rows, they are this shape:
					// 2---3
					// | / |
					// 0---1
					// For odd tri strip rows, they are this shape:
					// 2---3
					// | \ |
					// 0---1

					Vector3 v0, v1, v2, v3;
					getPointAlign(i, j, ALIGN_X_Y, &v0);
					getPointAlign(i + step, j, ALIGN_X_Y, &v1);
					getPointAlign(i, j + step, ALIGN_X_Y, &v2);
					getPointAlign(i + step, j + step, ALIGN_X_Y, &v3);

					Plane t1, t2;
					bool backwardTri = false;
					// Odd or even in terms of target level
					if ((j / step) % 2 == 0)
					{
						t1.redefine(v0, v1, v3);
						t2.redefine(v0, v3, v2);
					}
					else
					{
						t1.redefine(v1, v3, v2);
						t2.redefine(v0, v1, v2);
						backwardTri = true;
					}

					// include the bottommost row of vertices if this is the last row
					int yubound = (j == (mSize - step)? step : step - 1);
					for ( int y = 0; y <= yubound; y++ )
					{
						// include the rightmost col of vertices if this is the last col
						int xubound = (i == (mSize - step)? step : step - 1);
						for ( int x = 0; x <= xubound; x++ )
						{
							int fulldetailx = i + x;
							int fulldetaily = j + y;
							if ( fulldetailx % step == 0 && 
								fulldetaily % step == 0 )
							{
								// Skip, this one is a vertex at this level
								continue;
							}

							Real ypct = (Real)y / (Real)step;
							Real xpct = (Real)x / (Real)step;

							//interpolated height
							Vector3 actualPos;
							getPointAlign(fulldetailx, fulldetaily, ALIGN_X_Y, &actualPos);
							Real interp_h;
							// Determine which tri we're on 
							if ((xpct > ypct && !backwardTri) ||
								(xpct > (1-ypct) && backwardTri))
							{
								// Solve for x/z
								interp_h = 
									(-t1.normal.x * actualPos.x
									- t1.normal.y * actualPos.y
									- t1.d) / t1.normal.z;
							}
							else
							{
								// Second tri
								interp_h = 
									(-t2.normal.x * actualPos.x
									- t2.normal.y * actualPos.y
									- t2.d) / t2.normal.z;
							}

							Real actual_h = actualPos.z;
							Real delta = interp_h - actual_h;

							// max(delta) is the worst case scenario at this LOD
							// compared to the original heightmap

							// tell the quadtree about this 
							mQuadTree->notifyDelta(fulldetailx, fulldetaily, sourceLevel, delta);


							// If this vertex is being removed at this LOD, 
							// then save the height difference since that's the move
							// it will need to make. Vertices to be removed at this LOD
							// are halfway between the steps, but exclude those that
							// would have been eliminated at earlier levels
							int halfStep = step / 2;
							if (
							 ((fulldetailx % step) == halfStep && (fulldetaily % halfStep) == 0) ||
							 ((fulldetaily % step) == halfStep && (fulldetailx % halfStep) == 0))
							{
								// Save height difference 
								mDeltaData[fulldetailx + (fulldetaily * mSize)] = delta;
							}

						}

					}
				} // i
			} // j

		} // targetLevel

		mQuadTree->postDeltaCalculation(clampedRect);

		return finalRect;

	}
	//---------------------------------------------------------------------
	void Terrain::finaliseHeightDeltas(const Rect& rect, bool cpuData)
	{

		Rect clampedRect(rect);
		clampedRect.left = std::max(0L, clampedRect.left);
		clampedRect.top = std::max(0L, clampedRect.top);
		clampedRect.right = std::min((long)mSize, clampedRect.right);
		clampedRect.bottom = std::min((long)mSize, clampedRect.bottom);

		// min/max information
		mQuadTree->finaliseDeltaValues(clampedRect);
		// delta vertex data
		mQuadTree->updateVertexData(false, true, clampedRect, cpuData);

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
		// Early-out
		if (!mIsLoaded)
			return;

		// check deferred updates
		unsigned long currMillis = Root::getSingleton().getTimer()->getMilliseconds();
		unsigned long elapsedMillis = currMillis - mLastMillis;
		if (mCompositeMapUpdateCountdown > 0 && elapsedMillis)
		{
			if (elapsedMillis > mCompositeMapUpdateCountdown)
				mCompositeMapUpdateCountdown = 0;
			else
				mCompositeMapUpdateCountdown -= elapsedMillis;

			if (!mCompositeMapUpdateCountdown)
				updateCompositeMap();
		}
		mLastMillis = currMillis;
		// only calculate LOD once per LOD camera, per frame
		// shadow renders will pick up LOD camera from main viewport and so LOD will only
		// be calculated once for that case
		const Camera* lodCamera = v->getCamera()->getLodCamera();
		unsigned long frameNum = Root::getSingleton().getNextFrameNumber();
		if (mLastLODCamera != lodCamera || frameNum != mLastLODFrame)
		{
			mLastLODCamera = lodCamera;
			mLastLODFrame = frameNum;
			calculateCurrentLod(v);
		}
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

			// W. de Boer 2000 calculation
			// A = vp_near / abs(vp_top)
			// A = 1 / tan(fovy*0.5)    (== 1 for fovy=45*2)
			Real A = 1.0f / Math::Tan(cam->getFOVy() * 0.5f);
			// T = 2 * maxPixelError / vertRes
			Real maxPixelError = TerrainGlobalOptions::getMaxPixelError() * cam->_getLodBiasInverse();
			Real T = 2.0f * maxPixelError / (Real)vp->getActualHeight();

			// CFactor = A / T
			Real cFactor = A / T;

			mQuadTree->calculateCurrentLod(cam, cFactor);
		}
	}
	//---------------------------------------------------------------------
	std::pair<bool, Vector3> Terrain::rayIntersects(const Ray& ray, 
		bool cascadeToNeighbours /* = false */, Real distanceLimit /* = 0 */)
	{
		typedef std::pair<bool, Vector3> Result;
		// first step: convert the ray to a local vertex space
		// we assume terrain to be in the x-z plane, with the [0,0] vertex
		// at origin and a plane distance of 1 between vertices.
		// This makes calculations easier.
		Vector3 rayOrigin = ray.getOrigin() - getPosition();
		Vector3 rayDirection = ray.getDirection();
		// change alignment
		Vector3 tmp;
		switch (getAlignment())
		{
		case ALIGN_X_Y:
			std::swap(rayOrigin.y, rayOrigin.z);
			std::swap(rayDirection.y, rayDirection.z);
			break;
		case ALIGN_Y_Z:
			// x = z, z = y, y = -x
			tmp.x = rayOrigin.z; 
			tmp.z = rayOrigin.y; 
			tmp.y = -rayOrigin.x; 
			rayOrigin = tmp;
			tmp.x = rayDirection.z; 
			tmp.z = rayDirection.y; 
			tmp.y = -rayDirection.x; 
			rayDirection = tmp;
			break;
		case ALIGN_X_Z:
			// already in X/Z but values increase in -Z
			rayOrigin.z = -rayOrigin.z;
			rayDirection.z = -rayDirection.z;
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
		Real maxHeight = getMaxHeight();
		Real minHeight = getMinHeight();

		AxisAlignedBox aabb (Vector3(0, minHeight, 0), Vector3(mSize, maxHeight, mSize));
		std::pair<bool, Real> aabbTest = localRay.intersects(aabb);
		if (!aabbTest.first)
		{
			if (cascadeToNeighbours)
			{
				Terrain* neighbour = raySelectNeighbour(ray, distanceLimit);
				if (neighbour)
					return neighbour->rayIntersects(ray, cascadeToNeighbours, distanceLimit);
			}
			return Result(false, Vector3());
		}
		// get intersection point and move inside
		Vector3 cur = localRay.getPoint(aabbTest.second);

		// now check every quad the ray touches
		int quadX = std::min(std::max(static_cast<int>(cur.x), 0), (int)mSize-2);
		int quadZ = std::min(std::max(static_cast<int>(cur.z), 0), (int)mSize-2);
		int flipX = (rayDirection.x < 0 ? 0 : 1);
		int flipZ = (rayDirection.z < 0 ? 0 : 1);
		int xDir = (rayDirection.x < 0 ? -1 : 1);
		int zDir = (rayDirection.z < 0 ? -1 : 1);

		Result result(true, Vector3::ZERO);
		Real dummyHighValue = (Real)mSize * 10000.0f;


		while (cur.y >= (minHeight - 1e-3) && cur.y <= (maxHeight + 1e-3))
		{
			if (quadX < 0 || quadX >= (int)mSize-1 || quadZ < 0 || quadZ >= (int)mSize-1)
				break;

			result = checkQuadIntersection(quadX, quadZ, localRay);
			if (result.first)
				break;

			// determine next quad to test
			Real xDist = Math::RealEqual(rayDirection.x, 0.0) ? dummyHighValue : 
				(quadX - cur.x + flipX) / rayDirection.x;
			Real zDist = Math::RealEqual(rayDirection.z, 0.0) ? dummyHighValue : 
				(quadZ - cur.z + flipZ) / rayDirection.z;
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
				// z = x, y = z, x = -y
				tmp.x = -rayOrigin.y; 
				tmp.y = rayOrigin.z; 
				tmp.z = rayOrigin.x; 
				rayOrigin = tmp;
				break;
			case ALIGN_X_Z:
				result.second.z = -result.second.z;
				break;
			}
			result.second += getPosition();
		}
		else if (cascadeToNeighbours)
		{
			Terrain* neighbour = raySelectNeighbour(ray, distanceLimit);
			if (neighbour)
				result = neighbour->rayIntersects(ray, cascadeToNeighbours, distanceLimit);
		}
		return result;
	}
	//---------------------------------------------------------------------
	std::pair<bool, Vector3> Terrain::checkQuadIntersection(int x, int z, const Ray& ray)
	{
		// build the two planes belonging to the quad's triangles
		Vector3 v1 ((Real)x, *getHeightData(x,z), (Real)z);
		Vector3 v2 ((Real)x+1, *getHeightData(x+1,z), (Real)z);
		Vector3 v3 ((Real)x, *getHeightData(x,z+1), (Real)z+1);
		Vector3 v4 ((Real)x+1, *getHeightData(x+1,z+1), (Real)z+1);

		Plane p1, p2;
		bool oddRow = false;
		if (z % 2)
		{
			/* odd
			3---4
			| \ |
			1---2
			*/
			p1.redefine(v2, v4, v3);
			p2.redefine(v1, v2, v3);
			oddRow = true;
		}
		else
		{
			/* even
			3---4
			| / |
			1---2
			*/
			p1.redefine(v1, v2, v4);
			p2.redefine(v1, v4, v3);
		}

		// Test for intersection with the two planes. 
		// Then test that the intersection points are actually
		// still inside the triangle (with a small error margin)
		// Also check which triangle it is in
		std::pair<bool, Real> planeInt = ray.intersects(p1);
		if (planeInt.first)
		{
			Vector3 where = ray.getPoint(planeInt.second);
			Vector3 rel = where - v1;
			if (rel.x >= -0.01 && rel.x <= 1.01 && rel.z >= -0.01 && rel.z <= 1.01 // quad bounds
				&& ((rel.x >= rel.z && !oddRow) || (rel.x >= (1 - rel.z) && oddRow))) // triangle bounds
				return std::pair<bool, Vector3>(true, where);
		}
		planeInt = ray.intersects(p2);
		if (planeInt.first)
		{
			Vector3 where = ray.getPoint(planeInt.second);
			Vector3 rel = where - v1;
			if (rel.x >= -0.01 && rel.x <= 1.01 && rel.z >= -0.01 && rel.z <= 1.01 // quad bounds
				&& ((rel.x <= rel.z && !oddRow) || (rel.x <= (1 - rel.z) && oddRow))) // triangle bounds
				return std::pair<bool, Vector3>(true, where);
		}

		return std::pair<bool, Vector3>(false, Vector3());
	}
	//---------------------------------------------------------------------
	const MaterialPtr& Terrain::getMaterial() const
	{
		if (mMaterial.isNull() || 
			mMaterialGenerator->getChangeCount() != mMaterialGenerationCount ||
			mMaterialDirty)
		{
			mMaterial = mMaterialGenerator->generate(this);
			mMaterial->load();
			if (mCompositeMapRequired)
			{
				mCompositeMapMaterial = mMaterialGenerator->generateForCompositeMap(this);
				mCompositeMapMaterial->load();
			}
			mMaterialGenerationCount = mMaterialGenerator->getChangeCount();
			mMaterialDirty = false;
		}
		if (mMaterialParamsDirty)
		{
			mMaterialGenerator->updateParams(mMaterial, this);
			if(mCompositeMapRequired)
				mMaterialGenerator->updateParamsForCompositeMap(mCompositeMapMaterial, this);

		}

		return mMaterial;
	}
	//---------------------------------------------------------------------
	const MaterialPtr& Terrain::getCompositeMapMaterial() const
	{
		// both materials updated together since they change at the same time
		getMaterial();
		return mCompositeMapMaterial;
	}
	//---------------------------------------------------------------------
	void Terrain::checkLayers(bool includeGPUResources)
	{
		for (LayerInstanceList::iterator i = mLayers.begin(); i != mLayers.end(); ++i)
		{
			LayerInstance& layer = *i;
			// If we're missing sampler entries compared to the declaration, initialise them
			for (size_t i = layer.textureNames.size(); i < mLayerDecl.samplers.size(); ++i)
			{
				layer.textureNames.push_back(StringUtil::BLANK);
			}

			// if we have too many layers for the declaration, trim them
			if (layer.textureNames.size() > mLayerDecl.samplers.size())
			{
				layer.textureNames.resize(mLayerDecl.samplers.size());
			}
		}

		if (includeGPUResources)
		{
			createGPUBlendTextures();
			createLayerBlendMaps();
		}

	}
	//---------------------------------------------------------------------
	void Terrain::checkDeclaration()
	{
		if (mMaterialGenerator.isNull())
		{
			mMaterialGenerator = TerrainGlobalOptions::getDefaultMaterialGenerator();

		}

		if (mLayerDecl.elements.empty())
		{
			// default the declaration
			mLayerDecl = mMaterialGenerator->getLayerDeclaration();
		}

	}
	//---------------------------------------------------------------------
	void Terrain::addLayer(Real worldSize, const StringVector* textureNames)
	{
		if (!worldSize)
			worldSize = TerrainGlobalOptions::getDefaultLayerTextureWorldSize();

		mLayers.push_back(LayerInstance());
		if (textureNames)
		{
			LayerInstance& inst = mLayers[mLayers.size()-1];
			inst.textureNames = *textureNames;
		}
		// use utility method to update UV scaling
		setLayerWorldSize(mLayers.size()-1, worldSize);
		checkLayers(true);
		mMaterialDirty = true;
		mMaterialParamsDirty = true;

	}
	//---------------------------------------------------------------------
	void Terrain::removeLayer(uint8 index)
	{
		if (index < mLayers.size())
		{
			LayerInstanceList::iterator i = mLayers.begin();
			std::advance(i, index);
			mLayers.erase(i);
			mMaterialDirty = true;
			mMaterialParamsDirty = true;
		}
	}
	//---------------------------------------------------------------------
	uint8 Terrain::getMaxLayers() const
	{
		return mMaterialGenerator->getMaxLayers(this);
	}
	//---------------------------------------------------------------------
	TerrainLayerBlendMap* Terrain::getLayerBlendMap(uint8 layerIndex)
	{
		if (layerIndex == 0 || layerIndex-1 >= (uint8)mLayerBlendMapList.size())
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
			"Invalid layer index", "Terrain::getLayerBlendMap");

		uint8 idx = layerIndex - 1;
		if (!mLayerBlendMapList[idx])
		{
			if (mBlendTextureList.size() < static_cast<size_t>(idx / 4))
				checkLayers(true);

			const TexturePtr& tex = mBlendTextureList[idx / 4];
			mLayerBlendMapList[idx] = OGRE_NEW TerrainLayerBlendMap(this, layerIndex, tex->getBuffer().getPointer());
		}

		return mLayerBlendMapList[idx];

	}
	//---------------------------------------------------------------------
	uint8 Terrain::getBlendTextureCount(uint8 numLayers) const
	{
		return ((numLayers - 1) / 4) + 1;
	}
	//---------------------------------------------------------------------
	uint8 Terrain::getBlendTextureCount() const
	{
		return (uint8)mBlendTextureList.size();
	}
	//---------------------------------------------------------------------
	PixelFormat Terrain::getBlendTextureFormat(uint8 textureIndex, uint8 numLayers)
	{
		/*
		if (numLayers - 1 - (textureIndex * 4) > 3)
			return PF_BYTE_RGBA;
		else
			return PF_BYTE_RGB;
		*/
		// Always create RGBA; no point trying to create RGB since all cards pad to 32-bit (XRGB)
		// and it makes it harder to expand layer count dynamically if format has to change
		return PF_BYTE_RGBA;
	}
	//---------------------------------------------------------------------
	void Terrain::createGPUBlendTextures()
	{
		// Create enough RGBA/RGB textures to cope with blend layers
		uint8 numTex = getBlendTextureCount(getLayerCount());
		// delete extras
		TextureManager* tmgr = TextureManager::getSingletonPtr();
		if (!tmgr)
			return;

		while (mBlendTextureList.size() > numTex)
		{
			tmgr->remove(mBlendTextureList.back()->getHandle());			
			mBlendTextureList.pop_back();
		}

		uint8 currentTex = (uint8)mBlendTextureList.size();
		mBlendTextureList.resize(numTex);
		// create new textures
		for (uint8 i = currentTex; i < numTex; ++i)
		{
			PixelFormat fmt = getBlendTextureFormat(i, getLayerCount());
			// Use TU_STATIC because although we will update this, we won't do it every frame
			// in normal circumstances, so we don't want TU_DYNAMIC. Also we will 
			// read it (if we've cleared local temp areas) so no WRITE_ONLY
			mBlendTextureList[i] = TextureManager::getSingleton().createManual(
				msBlendTextureGenerator.generate(), _getDerivedResourceGroup(), 
				TEX_TYPE_2D, mLayerBlendMapSize, mLayerBlendMapSize, 1, 0, fmt, TU_STATIC);

			mLayerBlendMapSizeActual = mBlendTextureList[i]->getWidth();

			if (mCpuBlendMapStorage.size() > i)
			{
				// Load blend data
				PixelBox src(mLayerBlendMapSize, mLayerBlendMapSize, 1, fmt, mCpuBlendMapStorage[i]);
				mBlendTextureList[i]->getBuffer()->blitFromMemory(src);
				// release CPU copy, don't need it anymore
				OGRE_FREE(mCpuBlendMapStorage[i], MEMCATEGORY_RESOURCE);
			}
			else
			{
				// initialise black
				Box box(0, 0, mLayerBlendMapSize, mLayerBlendMapSize);
				HardwarePixelBufferSharedPtr buf = mBlendTextureList[i]->getBuffer();
				uint8* pInit = static_cast<uint8*>(buf->lock(box, HardwarePixelBuffer::HBL_DISCARD).data);
				memset(pInit, 0, PixelUtil::getNumElemBytes(fmt) * mLayerBlendMapSize * mLayerBlendMapSize);
				buf->unlock();
			}
		}
		mCpuBlendMapStorage.clear();


	}
	//---------------------------------------------------------------------
	void Terrain::createLayerBlendMaps()
	{
		// delete extra blend layers (affects GPU)
		while (mLayerBlendMapList.size() > mLayers.size() - 1)
		{
			OGRE_DELETE mLayerBlendMapList.back();
			mLayerBlendMapList.pop_back();
		}

		// resize up or down (up initialises to 0, populate as necessary)
		mLayerBlendMapList.resize(mLayers.size() - 1, 0);

	}
	//---------------------------------------------------------------------
	void Terrain::createOrDestroyGPUNormalMap()
	{
		if (mNormalMapRequired && mTerrainNormalMap.isNull())
		{
			// create
			mTerrainNormalMap = TextureManager::getSingleton().createManual(
				mMaterialName + "/nm", _getDerivedResourceGroup(), 
				TEX_TYPE_2D, mSize, mSize, 1, 0, PF_BYTE_RGB, TU_STATIC);

			// Upload loaded normal data if present
			if (mCpuTerrainNormalMap)
			{
				mTerrainNormalMap->getBuffer()->blitFromMemory(*mCpuTerrainNormalMap);
				OGRE_FREE(mCpuTerrainNormalMap->data, MEMCATEGORY_GENERAL);
				OGRE_DELETE mCpuTerrainNormalMap;
				mCpuTerrainNormalMap = 0;
			}

		}
		else if (!mNormalMapRequired && !mTerrainNormalMap.isNull())
		{
			// destroy
			TextureManager::getSingleton().remove(mTerrainNormalMap->getHandle());
			mTerrainNormalMap.setNull();
		}

	}
	//---------------------------------------------------------------------
	void Terrain::freeTemporaryResources()
	{
		// CPU blend maps
		for (BytePointerList::iterator i = mCpuBlendMapStorage.begin(); 
			i != mCpuBlendMapStorage.end(); ++i)
		{
			OGRE_FREE(*i, MEMCATEGORY_RESOURCE);
		}
		mCpuBlendMapStorage.clear();

		// Editable structures for blend layers (not needed at runtime,  only blend textures are)
		for (TerrainLayerBlendMapList::iterator i = mLayerBlendMapList.begin(); 
			i != mLayerBlendMapList.end(); ++i)
		{
			OGRE_DELETE *i;
			*i = 0;
		}

	}
	//---------------------------------------------------------------------
	const TexturePtr& Terrain::getLayerBlendTexture(uint8 index)
	{
		assert(index < mBlendTextureList.size());

		return mBlendTextureList[index];
	}
	//---------------------------------------------------------------------
	std::pair<uint8,uint8> Terrain::getLayerBlendTextureIndex(uint8 layerIndex)
	{
		assert(layerIndex > 0 && layerIndex < mLayers.size());
		uint8 idx = layerIndex - 1;
		return std::pair<uint8, uint8>(idx / 4, idx % 4);
	}
	//---------------------------------------------------------------------
	void Terrain::_setNormalMapRequired(bool normalMap)
	{
		if (normalMap != mNormalMapRequired)
		{
			mNormalMapRequired = normalMap;

			// Check NPOT textures supported. We have to use NPOT textures to map
			// texels to vertices directly!
			if (!mNormalMapRequired && Root::getSingleton().getRenderSystem()
				->getCapabilities()->hasCapability(RSC_NON_POWER_OF_2_TEXTURES))
			{
				mNormalMapRequired = false;
				LogManager::getSingleton().stream(LML_CRITICAL) <<
					"Terrain: Ignoring request for normal map generation since "
					"non-power-of-two texture support is required.";
			}

			createOrDestroyGPUNormalMap();

			// if we enabled, generate normal maps
			if (mNormalMapRequired)
			{
				// update derived data for whole terrain, but just normals
				mDirtyDerivedDataRect.left = mDirtyDerivedDataRect.top = 0;
				mDirtyDerivedDataRect.right = mDirtyDerivedDataRect.bottom = mSize;
				updateDerivedData(false, DERIVED_DATA_NORMALS);
			}
			
		}
	}
	//---------------------------------------------------------------------
	void Terrain::_setLightMapRequired(bool lightMap, bool shadowsOnly)
	{
		if (lightMap != mLightMapRequired || shadowsOnly != mLightMapShadowsOnly)
		{
			mLightMapRequired = lightMap;
			mLightMapShadowsOnly = mLightMapShadowsOnly;

			createOrDestroyGPULightmap();

			// if we enabled, generate light maps
			if (mLightMapRequired)
			{
				// update derived data for whole terrain, but just lightmap
				mDirtyDerivedDataRect.left = mDirtyDerivedDataRect.top = 0;
				mDirtyDerivedDataRect.right = mDirtyDerivedDataRect.bottom = mSize;
				updateDerivedData(false, DERIVED_DATA_LIGHTMAP);
			}
		}

	}
	//---------------------------------------------------------------------
	void Terrain::_setCompositeMapRequired(bool compositeMap)
	{
		if (compositeMap != mCompositeMapRequired)
		{
			mCompositeMapRequired = compositeMap;

			createOrDestroyGPUCompositeMap();
			
			// if we enabled, generate normal maps
			if (mCompositeMapRequired)
			{
				mCompositeMapDirtyRect.left = mCompositeMapDirtyRect.top = 0;
				mCompositeMapDirtyRect.right = mCompositeMapDirtyRect.bottom = mSize;
				updateCompositeMap();
			}

		}

	}
	//---------------------------------------------------------------------
	bool Terrain::canHandleRequest(const WorkQueue::Request* req, const WorkQueue* srcQ)
	{
		DerivedDataRequest ddr = any_cast<DerivedDataRequest>(req->getData());
		// only deal with own requests
		// we do this because if we delete a terrain we want any pending tasks to be discarded
		if (ddr.terrain != this)
			return false;
		else
			return true;

	}
	//---------------------------------------------------------------------
	bool Terrain::canHandleResponse(const WorkQueue::Response* res, const WorkQueue* srcQ)
	{
		DerivedDataRequest ddreq = any_cast<DerivedDataRequest>(res->getRequest()->getData());
		// only deal with own requests
		// we do this because if we delete a terrain we want any pending tasks to be discarded
		if (ddreq.terrain != this)
			return false;
		else
			return true;

	}
	//---------------------------------------------------------------------
	WorkQueue::Response* Terrain::handleRequest(const WorkQueue::Request* req, const WorkQueue* srcQ)
	{
		// Background thread (maybe)

		DerivedDataRequest ddr = any_cast<DerivedDataRequest>(req->getData());
		// only deal with own requests; we shouldn't ever get here though
		if (ddr.terrain != this)
			return 0;

		DerivedDataResponse ddres;
		ddres.remainingTypeMask = ddr.typeMask & DERIVED_DATA_ALL;

		// Do only ONE type of task per background iteration, in order of priority
		// this means we return faster, can abort faster and we repeat less redundant calcs
		// we don't do this as separate requests, because we only want one background
		// task per Terrain instance in flight at once
		if (ddr.typeMask & DERIVED_DATA_DELTAS)
		{
			ddres.deltaUpdateRect = calculateHeightDeltas(ddr.dirtyRect);
			ddres.remainingTypeMask &= ~ DERIVED_DATA_DELTAS;
		}
		else if (ddr.typeMask & DERIVED_DATA_NORMALS)
		{
			ddres.normalMapBox = calculateNormals(ddr.dirtyRect, ddres.normalUpdateRect);
			ddres.remainingTypeMask &= ~ DERIVED_DATA_NORMALS;
		}
		else if (ddr.typeMask & DERIVED_DATA_LIGHTMAP)
		{
			ddres.lightMapBox = calculateLightmap(ddr.dirtyRect, ddr.lightmapExtraDirtyRect, ddres.lightmapUpdateRect);
			ddres.remainingTypeMask &= ~ DERIVED_DATA_LIGHTMAP;
		}

		// TODO other data

		ddres.terrain = ddr.terrain;
		WorkQueue::Response* response = OGRE_NEW WorkQueue::Response(req, true, Any(ddres));
		return response;
	}
	//---------------------------------------------------------------------
	void Terrain::handleResponse(const WorkQueue::Response* res, const WorkQueue* srcQ)
	{
		// Main thread

		DerivedDataResponse ddres = any_cast<DerivedDataResponse>(res->getData());
		DerivedDataRequest ddreq = any_cast<DerivedDataRequest>(res->getRequest()->getData());

		// only deal with own requests
		if (ddreq.terrain != this)
			return;

		if ((ddreq.typeMask & DERIVED_DATA_DELTAS) && 
			!(ddres.remainingTypeMask & DERIVED_DATA_DELTAS))
			finaliseHeightDeltas(ddres.deltaUpdateRect, false);
		if ((ddreq.typeMask & DERIVED_DATA_NORMALS) && 
			!(ddres.remainingTypeMask & DERIVED_DATA_NORMALS))
		{
			finaliseNormals(ddres.normalUpdateRect, ddres.normalMapBox);
			mCompositeMapDirtyRect.merge(ddreq.dirtyRect);
		}
		if ((ddreq.typeMask & DERIVED_DATA_LIGHTMAP) && 
			!(ddres.remainingTypeMask & DERIVED_DATA_LIGHTMAP))
		{
			finaliseLightmap(ddres.lightmapUpdateRect, ddres.lightMapBox);
			mCompositeMapDirtyRect.merge(ddreq.dirtyRect);
			mCompositeMapDirtyRectLightmapUpdate = true;
		}
		
		// TODO other data

		mDerivedDataUpdateInProgress = false;

		// Re-trigger another request if there are still things to do, or if
		// we had a new request since this one
		Rect newRect(0,0,0,0);
		if (ddres.remainingTypeMask)
			newRect.merge(ddreq.dirtyRect);
		if (mDerivedUpdatePendingMask)
		{
			newRect.merge(mDirtyDerivedDataRect);
			mDirtyDerivedDataRect.setNull();
		}
		Rect newLightmapExtraRect(0,0,0,0);
		if (ddres.remainingTypeMask)
			newLightmapExtraRect.merge(ddreq.lightmapExtraDirtyRect);
		if (mDerivedUpdatePendingMask)
		{
			newLightmapExtraRect.merge(mDirtyLightmapFromNeighboursRect);
			mDirtyLightmapFromNeighboursRect.setNull();
		}
		uint8 newMask = ddres.remainingTypeMask | mDerivedUpdatePendingMask;
		if (newMask)
		{
			// trigger again
			updateDerivedDataImpl(newRect, newLightmapExtraRect, false, newMask);
		}
		else
		{
			// we've finished all the background processes
			// update the composite map if enabled
			if (mCompositeMapRequired)
				updateCompositeMap();
		}

	}
	//---------------------------------------------------------------------
	uint16 Terrain::getLODLevelWhenVertexEliminated(long x, long y)
	{
		// gets eliminated by either row or column first
		return std::min(getLODLevelWhenVertexEliminated(x), getLODLevelWhenVertexEliminated(y));
	}
	//---------------------------------------------------------------------
	uint16 Terrain::getLODLevelWhenVertexEliminated(long rowOrColulmn)
	{
		// LOD levels bisect the domain.
		// start at the lowest detail
		uint16 currentElim = (mSize - 1) / (mMinBatchSize - 1);
		// start at a non-exitant LOD index, this applies to the min batch vertices
		// which are never eliminated
		uint16 currentLod =  mNumLodLevels;

		while (rowOrColulmn % currentElim)
		{
			// not on this boundary, look finer
			currentElim = currentElim / 2;
			--currentLod;

			// This will always terminate since (anything % 1 == 0)
		}

		return currentLod;
	}
	//---------------------------------------------------------------------
	PixelBox* Terrain::calculateNormals(const Rect &rect, Rect& finalRect)
	{
		// Widen the rectangle by 1 element in all directions since height
		// changes affect neighbours normals
		Rect widenedRect(
			std::max(0L, rect.left - 1L), 
			std::max(0L, rect.top - 1L), 
			std::min((long)mSize, rect.right + 1L), 
			std::min((long)mSize, rect.bottom + 1L)
			);
		// allocate memory for RGB
		uint8* pData = static_cast<uint8*>(
			OGRE_MALLOC(widenedRect.width() * widenedRect.height() * 3, MEMCATEGORY_GENERAL));

		PixelBox* pixbox = OGRE_NEW PixelBox(widenedRect.width(), widenedRect.height(), 1, PF_BYTE_RGB, pData);

		// Evaluate normal like this
		//  3---2---1
		//  | \ | / |
		//	4---P---0
		//  | / | \ |
		//	5---6---7

		Plane plane;
		for (long y = widenedRect.top; y < widenedRect.bottom; ++y)
		{
			for (long x = widenedRect.left; x < widenedRect.right; ++x)
			{
				Vector3 cumulativeNormal = Vector3::ZERO;

				// Build points to sample
				Vector3 centrePoint;
				Vector3 adjacentPoints[8];
				getPointFromSelfOrNeighbour(x  , y,   &centrePoint);
				getPointFromSelfOrNeighbour(x+1, y,   &adjacentPoints[0]);
				getPointFromSelfOrNeighbour(x+1, y+1, &adjacentPoints[1]);
				getPointFromSelfOrNeighbour(x,   y+1, &adjacentPoints[2]);
				getPointFromSelfOrNeighbour(x-1, y+1, &adjacentPoints[3]);
				getPointFromSelfOrNeighbour(x-1, y,   &adjacentPoints[4]);
				getPointFromSelfOrNeighbour(x-1, y-1, &adjacentPoints[5]);
				getPointFromSelfOrNeighbour(x,   y-1, &adjacentPoints[6]);
				getPointFromSelfOrNeighbour(x+1, y-1, &adjacentPoints[7]);

				for (int i = 0; i < 8; ++i)
				{
					plane.redefine(centrePoint, adjacentPoints[i], adjacentPoints[(i+1)%8]);
					cumulativeNormal += plane.normal;
				}

				// normalise & store normal
				cumulativeNormal.normalise();

				// encode as RGB, object space
				// invert the Y to deal with image space
				long storeX = x - widenedRect.left;
				long storeY = widenedRect.bottom - y - 1;

				uint8* pStore = pData + ((storeY * widenedRect.width()) + storeX) * 3;
				*pStore++ = static_cast<uint8>((cumulativeNormal.x + 1.0f) * 0.5f * 255.0f);
				*pStore++ = static_cast<uint8>((cumulativeNormal.y + 1.0f) * 0.5f * 255.0f);
				*pStore++ = static_cast<uint8>((cumulativeNormal.z + 1.0f) * 0.5f * 255.0f);


			}
		}

		finalRect = widenedRect;

		return pixbox;
	}
	//---------------------------------------------------------------------
	void Terrain::finaliseNormals(const Ogre::Rect &rect, Ogre::PixelBox *normalsBox)
	{
		createOrDestroyGPUNormalMap();
		// deal with race condition where nm has been disabled while we were working!
		if (!mTerrainNormalMap.isNull())
		{
			// blit the normals into the texture
			if (rect.left == 0 && rect.top == 0 && rect.bottom == mSize && rect.right == mSize)
			{
				mTerrainNormalMap->getBuffer()->blitFromMemory(*normalsBox);
			}
			else
			{
				// content of normalsBox is already inverted in Y, but rect is still 
				// in terrain space for dealing with sub-rect, so invert
				Image::Box dstBox;
				dstBox.left = rect.left;
				dstBox.right = rect.right;
				dstBox.top = mSize - rect.bottom;
				dstBox.bottom = mSize - rect.top;
				mTerrainNormalMap->getBuffer()->blitFromMemory(*normalsBox, dstBox);
			}
		}
		

		// delete memory
		OGRE_FREE(normalsBox->data, MEMCATEGORY_GENERAL);
		OGRE_DELETE(normalsBox);

	}
	//---------------------------------------------------------------------
	void Terrain::widenRectByVector(const Vector3& vec, const Rect& inRect, Rect& outRect)
	{
		widenRectByVector(vec, inRect, getMinHeight(), getMaxHeight(), outRect);
	}
	//---------------------------------------------------------------------
	void Terrain::widenRectByVector(const Vector3& vec, const Rect& inRect, 
		Real minHeight, Real maxHeight, Rect& outRect)
	{

		outRect = inRect;

		Plane p;
		switch(getAlignment())
		{
		case ALIGN_X_Y:
			p.redefine(Vector3::UNIT_Z, Vector3(0, 0, vec.z < 0.0 ? minHeight : maxHeight));
			break;
		case ALIGN_X_Z:
			p.redefine(Vector3::UNIT_Y, Vector3(0, vec.y < 0.0 ? minHeight : maxHeight, 0));
			break;
		case ALIGN_Y_Z:
			p.redefine(Vector3::UNIT_X, Vector3(vec.x < 0.0 ? minHeight : maxHeight, 0, 0));
			break;
		}
		float verticalVal = vec.dotProduct(p.normal);

		if (Math::RealEqual(verticalVal, 0.0))
			return;

		Vector3 corners[4];
		Real startHeight = verticalVal < 0.0 ? maxHeight : minHeight;
		getPoint(inRect.left, inRect.top, startHeight, &corners[0]);
		getPoint(inRect.right-1, inRect.top, startHeight, &corners[1]);
		getPoint(inRect.left, inRect.bottom-1, startHeight, &corners[2]);
		getPoint(inRect.right-1, inRect.bottom-1, startHeight, &corners[3]);

		for (int i = 0; i < 4; ++i)
		{
			Ray ray(corners[i] + mPos, vec);
			std::pair<bool, Real> rayHit = ray.intersects(p);
			if(rayHit.first)
			{
				Vector3 pt = ray.getPoint(rayHit.second);
				// convert back to terrain point
				Vector3 terrainHitPos;
				getTerrainPosition(pt, &terrainHitPos);
				// build rectangle which has rounded down & rounded up values
				// remember right & bottom are exclusive
				Rect mergeRect(
					(long)(terrainHitPos.x * (mSize - 1)), 
					(long)(terrainHitPos.y * (mSize - 1)), 
					(long)(terrainHitPos.x * (float)(mSize - 1) + 0.5) + 1, 
					(long)(terrainHitPos.y * (float)(mSize - 1) + 0.5) + 1
					);
				outRect.merge(mergeRect);
			}
		}

	}
	//---------------------------------------------------------------------
	PixelBox* Terrain::calculateLightmap(const Rect& rect, const Rect& extraTargetRect, Rect& outFinalRect)
	{
		// TODO - handle neighbour page casting

		// as well as calculating the lighting changes for the area that is
		// dirty, we also need to calculate the effect on casting shadow on
		// other areas. To do this, we project the dirt rect by the light direction
		// onto the minimum height


		const Vector3& lightVec = TerrainGlobalOptions::getLightMapDirection();
		Rect widenedRect;
		widenRectByVector(lightVec, rect, widenedRect);

		// merge in the extra area (e.g. from neighbours)
		widenedRect.merge(extraTargetRect);

		// widenedRect now contains terrain point space version of the area we
		// need to calculate. However, we need to calculate in lightmap image space
		float terrainToLightmapScale = (float)mLightmapSizeActual / (float)mSize;
		widenedRect.left *= terrainToLightmapScale;
		widenedRect.right *= terrainToLightmapScale;
		widenedRect.top *= terrainToLightmapScale;
		widenedRect.bottom *= terrainToLightmapScale;

		// clamp 
		widenedRect.left = std::max(0L, widenedRect.left);
		widenedRect.top = std::max(0L, widenedRect.top);
		widenedRect.right = std::min((long)mLightmapSizeActual, widenedRect.right);
		widenedRect.bottom = std::min((long)mLightmapSizeActual, widenedRect.bottom);

		outFinalRect = widenedRect;

		// allocate memory (L8)
		uint8* pData = static_cast<uint8*>(
			OGRE_MALLOC(widenedRect.width() * widenedRect.height(), MEMCATEGORY_GENERAL));

		PixelBox* pixbox = OGRE_NEW PixelBox(widenedRect.width(), widenedRect.height(), 1, PF_L8, pData);

		Real heightPad = (getMaxHeight() - getMinHeight()) * 1.0e-3f;

		for (long y = widenedRect.top; y < widenedRect.bottom; ++y)
		{
			for (long x = widenedRect.left; x < widenedRect.right; ++x)
			{
				float litVal = 1.0f;

				// convert to terrain space (not points, allow this to go between points)
				float Tx = (float)x / (float)(mLightmapSizeActual-1);
				float Ty = (float)y / (float)(mLightmapSizeActual-1);

				// get world space point
				// add a little height padding to stop shadowing self
				Vector3 wpos = Vector3::ZERO;
				getPosition(Tx, Ty, getHeightAtTerrainPosition(Tx, Ty) + heightPad, &wpos);
				wpos += getPosition();
				// build ray, cast backwards along light direction
				Ray ray(wpos, -lightVec);

				// Cascade into neighbours when casting, but don't travel further
				// than world size
				std::pair<bool, Vector3> rayHit = rayIntersects(ray, true, mWorldSize);

				// TODO - cast multiple rays to antialias?
				// TODO - fade by distance?

				if (rayHit.first)
					litVal = 0.0f;

				// encode as L8
				// invert the Y to deal with image space
				long storeX = x - widenedRect.left;
				long storeY = widenedRect.bottom - y - 1;

				uint8* pStore = pData + ((storeY * widenedRect.width()) + storeX);
				*pStore = litVal * 255.0;

			}
		}


		return pixbox;



	}
	//---------------------------------------------------------------------
	void Terrain::finaliseLightmap(const Rect& rect, PixelBox* lightmapBox)
	{
		createOrDestroyGPULightmap();
		// deal with race condition where lm has been disabled while we were working!
		if (!mLightmap.isNull())
		{
			// blit the normals into the texture
			if (rect.left == 0 && rect.top == 0 && rect.bottom == mLightmapSizeActual && rect.right == mLightmapSizeActual)
			{
				mLightmap->getBuffer()->blitFromMemory(*lightmapBox);
			}
			else
			{
				// content of PixelBox is already inverted in Y, but rect is still 
				// in terrain space for dealing with sub-rect, so invert
				Image::Box dstBox;
				dstBox.left = rect.left;
				dstBox.right = rect.right;
				dstBox.top = mLightmapSizeActual - rect.bottom;
				dstBox.bottom = mLightmapSizeActual - rect.top;
				mLightmap->getBuffer()->blitFromMemory(*lightmapBox, dstBox);
			}
		}

	}
	//---------------------------------------------------------------------
	void Terrain::updateCompositeMap()
	{
		// All done in the render thread
		if (mCompositeMapRequired && !mCompositeMapDirtyRect.isNull())
		{
			mModified = true;
			createOrDestroyGPUCompositeMap();
			if (mCompositeMapDirtyRectLightmapUpdate &&
				(mCompositeMapDirtyRect.width() < mSize	|| mCompositeMapDirtyRect.height() < mSize))
			{
				// widen the dirty rectangle since lighting makes it wider
				Rect widenedRect;
				widenRectByVector(TerrainGlobalOptions::getLightMapDirection(), mCompositeMapDirtyRect, widenedRect);
				// clamp
				widenedRect.left = std::max(widenedRect.left, 0L);
				widenedRect.top = std::max(widenedRect.top, 0L);
				widenedRect.right = std::min(widenedRect.right, (long)mSize);
				widenedRect.bottom = std::min(widenedRect.bottom, (long)mSize);
				mMaterialGenerator->updateCompositeMap(this, widenedRect);	
			}
			else
				mMaterialGenerator->updateCompositeMap(this, mCompositeMapDirtyRect);

			mCompositeMapDirtyRectLightmapUpdate = false;
			mCompositeMapDirtyRect.setNull();


		}
	}
	//---------------------------------------------------------------------
	void Terrain::updateCompositeMapWithDelay(Real delay)
	{
		mCompositeMapUpdateCountdown = delay * 1000;
	}
	//---------------------------------------------------------------------
	uint8 Terrain::getBlendTextureIndex(uint8 layerIndex) const
	{
		if (layerIndex == 0 || layerIndex-1 >= (uint8)mLayerBlendMapList.size())
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
			"Invalid layer index", "Terrain::getBlendTextureIndex");

		return (layerIndex - 1) % 4;

	}
	//---------------------------------------------------------------------
	const String& Terrain::getBlendTextureName(uint8 textureIndex) const
	{
		if (textureIndex >= (uint8)mBlendTextureList.size())
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
			"Invalid texture index", "Terrain::getBlendTextureName");
		
		return mBlendTextureList[textureIndex]->getName();
	}
	//---------------------------------------------------------------------
	void Terrain::setGlobalColourMapEnabled(bool enabled, uint16 sz)
	{
		if (!sz)
			sz = TerrainGlobalOptions::getDefaultGlobalColourMapSize();

		if (enabled != mGlobalColourMapEnabled ||
			(enabled && mGlobalColourMapSize != sz))
		{
			mGlobalColourMapEnabled = enabled;
			mGlobalColourMapSize = sz;

			createOrDestroyGPUColourMap();

			mMaterialDirty = true;
			mMaterialParamsDirty = true;
		}

	}
	//---------------------------------------------------------------------
	void Terrain::createOrDestroyGPUColourMap()
	{
		if (mGlobalColourMapEnabled && mColourMap.isNull())
		{
			// create
			mColourMap = TextureManager::getSingleton().createManual(
				mMaterialName + "/cm", _getDerivedResourceGroup(), 
				TEX_TYPE_2D, mGlobalColourMapSize, mGlobalColourMapSize, MIP_DEFAULT, PF_BYTE_RGB);

			if (mCpuColourMapStorage)
			{
				// Load cached data
				PixelBox src(mGlobalColourMapSize, mGlobalColourMapSize, 1, PF_BYTE_RGB, mCpuColourMapStorage);
				mColourMap->getBuffer()->blitFromMemory(src);
				// release CPU copy, don't need it anymore
				OGRE_FREE(mCpuColourMapStorage, MEMCATEGORY_RESOURCE);
				mCpuColourMapStorage = 0;

			}
		}
		else if (!mGlobalColourMapEnabled && !mColourMap.isNull())
		{
			// destroy
			TextureManager::getSingleton().remove(mColourMap->getHandle());
			mColourMap.setNull();
		}

	}
	//---------------------------------------------------------------------
	void Terrain::createOrDestroyGPULightmap()
	{
		if (mLightMapRequired && mLightmap.isNull())
		{
			// create
			mLightmap = TextureManager::getSingleton().createManual(
				mMaterialName + "/lm", _getDerivedResourceGroup(), 
				TEX_TYPE_2D, mLightmapSize, mLightmapSize, 0, PF_L8, TU_STATIC);

			mLightmapSizeActual = mLightmap->getWidth();

			if (mCpuLightmapStorage)
			{
				// Load cached data
				PixelBox src(mLightmapSize, mLightmapSize, 1, PF_L8, mCpuLightmapStorage);
				mLightmap->getBuffer()->blitFromMemory(src);
				// release CPU copy, don't need it anymore
				OGRE_FREE(mCpuLightmapStorage, MEMCATEGORY_RESOURCE);
				mCpuLightmapStorage = 0;

			}
			else
			{
				// initialise to full-bright
				Box box(0, 0, mLightmapSizeActual, mLightmapSizeActual);
				HardwarePixelBufferSharedPtr buf = mLightmap->getBuffer();
				uint8* pInit = static_cast<uint8*>(buf->lock(box, HardwarePixelBuffer::HBL_DISCARD).data);
				memset(pInit, 255, mLightmapSizeActual * mLightmapSizeActual);
				buf->unlock();

			}
		}
		else if (!mLightMapRequired && !mLightmap.isNull())
		{
			// destroy
			TextureManager::getSingleton().remove(mLightmap->getHandle());
			mLightmap.setNull();
		}

	}
	//---------------------------------------------------------------------
	void Terrain::createOrDestroyGPUCompositeMap()
	{
		if (mCompositeMapRequired && mCompositeMap.isNull())
		{
			// create
			mCompositeMap = TextureManager::getSingleton().createManual(
				mMaterialName + "/comp", _getDerivedResourceGroup(), 
				TEX_TYPE_2D, mCompositeMapSize, mCompositeMapSize, 0, PF_BYTE_RGBA, TU_STATIC);

			mCompositeMapSizeActual = mCompositeMap->getWidth();

			if (mCpuCompositeMapStorage)
			{
				// Load cached data
				PixelBox src(mCompositeMapSize, mCompositeMapSize, 1, PF_BYTE_RGBA, mCpuCompositeMapStorage);
				mCompositeMap->getBuffer()->blitFromMemory(src);
				// release CPU copy, don't need it anymore
				OGRE_FREE(mCpuCompositeMapStorage, MEMCATEGORY_RESOURCE);
				mCpuCompositeMapStorage = 0;

			}
			else
			{
				// initialise to black
				Box box(0, 0, mCompositeMapSizeActual, mCompositeMapSizeActual);
				HardwarePixelBufferSharedPtr buf = mCompositeMap->getBuffer();
				uint8* pInit = static_cast<uint8*>(buf->lock(box, HardwarePixelBuffer::HBL_DISCARD).data);
				memset(pInit, 0, mCompositeMapSizeActual * mCompositeMapSizeActual * 4);
				buf->unlock();

			}
		}
		else if (!mCompositeMapRequired && !mCompositeMap.isNull())
		{
			// destroy
			TextureManager::getSingleton().remove(mCompositeMap->getHandle());
			mCompositeMap.setNull();
		}

	}
	//---------------------------------------------------------------------
	Terrain* Terrain::getNeighbour(NeighbourIndex index)
	{
		return mNeighbours[index];
	}
	//---------------------------------------------------------------------
	void Terrain::setNeighbour(NeighbourIndex index, Terrain* neighbour, 
		bool recalculate /*= false*/, bool notifyOther /* = true */)
	{
		if (mNeighbours[index] != neighbour)
		{
			assert(neighbour != this && "Can't set self as own neighbour!");

			// detach existing
			if (mNeighbours[index] && notifyOther)
				mNeighbours[index]->setNeighbour(getOppositeNeighbour(index), 0, false, false);

			mNeighbours[index] = neighbour;
			if (neighbour && notifyOther)
				mNeighbours[index]->setNeighbour(getOppositeNeighbour(index), this, recalculate, false);

			if (recalculate)
			{
				// Recalculate, pass OUR edge rect
				Rect edgerect;
				getEdgeRect(index, 2, &edgerect);
				neighbourModified(index, edgerect, edgerect);
			}
		}
	}
	//---------------------------------------------------------------------
	Terrain::NeighbourIndex Terrain::getOppositeNeighbour(NeighbourIndex index)
	{
		int intindex = static_cast<int>(index);
		intindex += NEIGHBOUR_COUNT / 2;
		intindex = intindex % NEIGHBOUR_COUNT;
		return static_cast<NeighbourIndex>(intindex);
	}
	//---------------------------------------------------------------------
	Terrain::NeighbourIndex Terrain::getNeighbourIndex(long x, long y)
	{
		if (x < 0)
		{
			if (y < 0)
				return NEIGHBOUR_SOUTHWEST;
			else if (y > 0)
				return NEIGHBOUR_NORTHWEST;
			else
				return NEIGHBOUR_WEST;
		}
		else if (x > 0)
		{
			if (y < 0)
				return NEIGHBOUR_SOUTHEAST;
			else if (y > 0)
				return NEIGHBOUR_NORTHEAST;
			else
				return NEIGHBOUR_EAST;
		}

		if (y < 0)
		{
			if (x == 0)
				return NEIGHBOUR_SOUTH;
		}
		else if (y > 0)
		{
			if (x == 0)
				return NEIGHBOUR_NORTH;
		}

		return NEIGHBOUR_NORTH;

	}
	//---------------------------------------------------------------------
	void Terrain::notifyNeighbours()
	{
		// There are 3 things that can need updating:
		// Height at edge - match to neighbour (first one to update 'loses' to other since read-only)
		// Normal at edge - use heights from across boundary too
		// Shadows across edge
		// The extent to which these can affect the current tile vary:
		// Height at edge - only affected by a change at the adjoining edge / corner
		// Normal at edge - only affected by a change to the 2 rows adjoining the edge / corner
		// Shadows across edge - possible effect extends based on the projection of the
		//   neighbour AABB along the light direction (worst case scenario)

		if (!mDirtyGeometryRectForNeighbours.isNull())
		{
			Rect dirtyRect(mDirtyGeometryRectForNeighbours);
			mDirtyGeometryRectForNeighbours.setNull();
			// calculate light update rectangle
			const Vector3& lightVec = TerrainGlobalOptions::getLightMapDirection();
			Rect lightmapRect;
			widenRectByVector(lightVec, dirtyRect, getMinHeight(), getMaxHeight(), lightmapRect);

			for (int i = 0; i < (int)NEIGHBOUR_COUNT; ++i)
			{
				NeighbourIndex ni = static_cast<NeighbourIndex>(i);
				Terrain* neighbour = getNeighbour(ni);
				if (!neighbour)
					continue;

				// Intersect the incoming rectangles with the edge regions related to this neighbour
				Rect edgeRect;
				getEdgeRect(ni, 2, &edgeRect);
				Rect heightEdgeRect = edgeRect.intersect(dirtyRect);
				Rect lightmapEdgeRect = edgeRect.intersect(lightmapRect);

				if (!heightEdgeRect.isNull() || !lightmapRect.isNull())
				{
					// ok, we have something valid to pass on
					Rect neighbourHeightEdgeRect, neighbourLightmapEdgeRect;
					if (!heightEdgeRect.isNull())
						getNeighbourEdgeRect(ni, heightEdgeRect, &neighbourHeightEdgeRect);
					if (!lightmapRect.isNull())
						getNeighbourEdgeRect(ni, lightmapEdgeRect, &neighbourLightmapEdgeRect);

					neighbour->neighbourModified(getOppositeNeighbour(ni), 
						neighbourHeightEdgeRect, neighbourLightmapEdgeRect);
					
				}

			}
		}
	}
	//---------------------------------------------------------------------
	void Terrain::neighbourModified(NeighbourIndex index, const Rect& edgerect, const Rect& shadowrect)
	{
		// We can safely assume that we would not have been contacted if it wasn't 
		// important
		const Terrain* neighbour = getNeighbour(index);
		if (!neighbour)
			return; // bogus request

		bool updateGeom = false;
		uint8 updateDerived = 0;


		if (!edgerect.isNull())
		{
			// update edges; match heights first, then recalculate normals
			// reduce to just single line / corner
			Rect heightMatchRect;
			getEdgeRect(index, 1, &heightMatchRect);
			heightMatchRect = heightMatchRect.intersect(edgerect);

			for (long y = heightMatchRect.top; y < heightMatchRect.bottom; ++y)
			{
				for (long x = heightMatchRect.left; x < heightMatchRect.right; ++x)
				{
					long nx, ny;
					getNeighbourPoint(index, x, y, &nx, &ny);
					float neighbourHeight = neighbour->getHeightAtPoint(nx, ny); 
					if (!Math::RealEqual(neighbourHeight, getHeightAtPoint(x, y), 1e-3f))
					{
						setHeightAtPoint(x, y, neighbourHeight);
						if (!updateGeom)
						{
							updateGeom = true;
							updateDerived |= DERIVED_DATA_ALL;
						}

					}
				}
			}
			// if we didn't need to update heights, we still need to update normals
			// because this was called only if neighbor changed
			if (!updateGeom)
			{
				// ideally we would deal with normal dirty rect separately (as we do with
				// lightmaps) because a dirty geom rectangle will actually grow by one 
				// element in each direction for normals recalculation. However for
				// the sake of one row/column it's really not worth it.
				mDirtyDerivedDataRect.merge(edgerect);
				updateDerived |= DERIVED_DATA_NORMALS;
			}
		}

		if (!shadowrect.isNull())
		{
			// update shadows
			// here we need to widen the rect passed in based on the min/max height 
			// of the *neighbour*
			const Vector3& lightVec = TerrainGlobalOptions::getLightMapDirection();
			Rect widenedRect;
			widenRectByVector(lightVec, shadowrect, neighbour->getMinHeight(), neighbour->getMaxHeight(), widenedRect);

			// set the special-case lightmap dirty rectangle
			mDirtyLightmapFromNeighboursRect.merge(widenedRect);
			updateDerived |= DERIVED_DATA_LIGHTMAP;
		}

		if (updateGeom)
			updateGeometry();
		if (updateDerived)
			updateDerivedData(false, updateDerived);



	}
	//---------------------------------------------------------------------
	void Terrain::getEdgeRect(NeighbourIndex index, long range, Rect* outRect)
	{
		// We make the edge rectangle 2 rows / columns at the edge of the tile
		// 2 because this copes with normal changes and potentially filtered
		// shadows.
		// all right / bottom values are exclusive
		// terrain origin is bottom-left remember so north is highest value

		// set left/right
		switch(index)
		{
		case NEIGHBOUR_EAST:
		case NEIGHBOUR_NORTHEAST:
		case NEIGHBOUR_SOUTHEAST:
			outRect->left = mSize - range;
			outRect->right = mSize;
			break;
		case NEIGHBOUR_WEST:
		case NEIGHBOUR_NORTHWEST:
		case NEIGHBOUR_SOUTHWEST:
			outRect->left = 0;
			outRect->right = range;
			break;
		case NEIGHBOUR_NORTH:
		case NEIGHBOUR_SOUTH: 
			outRect->left = 0;
			outRect->right = mSize;
			break;
		};

		// set top / bottom
		switch(index)
		{
		case NEIGHBOUR_NORTH:
		case NEIGHBOUR_NORTHEAST:
		case NEIGHBOUR_NORTHWEST:
			outRect->top = mSize - range;
			outRect->bottom = mSize;
			break;
		case NEIGHBOUR_SOUTH: 
		case NEIGHBOUR_SOUTHWEST:
		case NEIGHBOUR_SOUTHEAST:
			outRect->top = 0;
			outRect->bottom = range;
			break;
		case NEIGHBOUR_EAST:
		case NEIGHBOUR_WEST:
			outRect->top = 0;
			outRect->bottom = mSize;
			break;
		};
	}
	//---------------------------------------------------------------------
	void Terrain::getNeighbourEdgeRect(NeighbourIndex index, const Rect& inRect, Rect* outRect)
	{
		assert (mSize == getNeighbour(index)->getSize());

		// Basically just reflect the rect 
		// remember index is neighbour relationship from OUR perspective so
		// arrangement is backwards to getEdgeRect

		// left/right
		switch(index)
		{
		case NEIGHBOUR_EAST:
		case NEIGHBOUR_NORTHEAST:
		case NEIGHBOUR_SOUTHEAST:
		case NEIGHBOUR_WEST:
		case NEIGHBOUR_NORTHWEST:
		case NEIGHBOUR_SOUTHWEST:
			outRect->left = mSize - inRect.right;
			outRect->right = mSize - inRect.left;
			break;
		default:
			outRect->left = inRect.left;
			outRect->right = inRect.right;
			break;
		};

		// top / bottom
		switch(index)
		{
		case NEIGHBOUR_NORTH:
		case NEIGHBOUR_NORTHEAST:
		case NEIGHBOUR_NORTHWEST:
		case NEIGHBOUR_SOUTH: 
		case NEIGHBOUR_SOUTHWEST:
		case NEIGHBOUR_SOUTHEAST:
			outRect->top = mSize - inRect.bottom;
			outRect->bottom = mSize - inRect.top;
			break;
		default:
			outRect->top = inRect.top;
			outRect->bottom = inRect.bottom;
			break;
		};

	}
	//---------------------------------------------------------------------
	void Terrain::getNeighbourPoint(NeighbourIndex index, long x, long y, long *outx, long *outy)
	{
		// Get the index of the point we should be looking at on a neighbour
		// in order to match up points
		assert (mSize == getNeighbour(index)->getSize());

		// left/right
		switch(index)
		{
		case NEIGHBOUR_EAST:
		case NEIGHBOUR_NORTHEAST:
		case NEIGHBOUR_SOUTHEAST:
		case NEIGHBOUR_WEST:
		case NEIGHBOUR_NORTHWEST:
		case NEIGHBOUR_SOUTHWEST:
			*outx = mSize - x - 1;
			break;
		default:
			*outx = x;
			break;
		};

		// top / bottom
		switch(index)
		{
		case NEIGHBOUR_NORTH:
		case NEIGHBOUR_NORTHEAST:
		case NEIGHBOUR_NORTHWEST:
		case NEIGHBOUR_SOUTH: 
		case NEIGHBOUR_SOUTHWEST:
		case NEIGHBOUR_SOUTHEAST:
			*outy = mSize - y - 1;
			break;
		default:
			*outy = y;
			break;
		};
	}
	//---------------------------------------------------------------------
	void Terrain::getPointFromSelfOrNeighbour(long x, long y, Vector3* outpos)
	{
		if (x >= 0 && y >=0 && x < mSize && y < mSize)
			getPoint(x, y, outpos);
		else
		{
			long nx, ny;
			NeighbourIndex ni;
			getNeighbourPointOverflow(x, y, &ni, &nx, &ny);
			Terrain* neighbour = getNeighbour(ni);
			if (neighbour)
			{
				Vector3 neighbourPos;
				neighbour->getPoint(nx, ny, &neighbourPos);
				// adjust to make it relative to our position
				*outpos = neighbourPos + neighbour->getPosition() - getPosition();
			}
			else
			{
				// use our getPoint() after all, just clamp
				x = std::min(x, mSize - 1L);
				y = std::min(y, mSize - 1L);
				x = std::max(x, 0L);
				y = std::max(y, 0L);
				getPoint(x, y, outpos);
			}

		}
	}
	//---------------------------------------------------------------------
	void Terrain::getNeighbourPointOverflow(long x, long y, NeighbourIndex *outindex, long *outx, long *outy)
	{
		if (x < 0)
		{
			*outx = x + mSize - 1;
			if (y < 0)
				*outindex = NEIGHBOUR_SOUTHWEST;
			else if (y >= mSize)
				*outindex = NEIGHBOUR_NORTHWEST;
			else
				*outindex = NEIGHBOUR_WEST;
		}
		else if (x >= mSize)
		{
			*outx = x - mSize + 1;
			if (y < 0)
				*outindex = NEIGHBOUR_SOUTHEAST;
			else if (y >= mSize)
				*outindex = NEIGHBOUR_NORTHEAST;
			else
				*outindex = NEIGHBOUR_EAST;
		}
		else
			*outx = x;

		if (y < 0)
		{
			*outy = y + mSize - 1;
			if (x >= 0 && x < mSize)
				*outindex = NEIGHBOUR_SOUTH;
		}
		else if (y >= mSize)
		{
			*outy = y - mSize + 1;
			if (x >= 0 && x < mSize)
				*outindex = NEIGHBOUR_NORTH;
		}
		else
			*outy = y;
	}
	//---------------------------------------------------------------------
	Terrain* Terrain::raySelectNeighbour(const Ray& ray, Real distanceLimit /* = 0 */)
	{
		Real dNear, dFar;
		Ray localRay(ray.getOrigin() - getPosition(), ray.getDirection());
		// Move back half a square - if we're on the edge of the AABB we might
		// miss the intersection otherwise; it's ok for everywhere else since
		// we want the far intersection anyway
		localRay.setOrigin(localRay.getPoint(-mWorldSize/mSize * 0.5f));
		if (Math::intersects(localRay, getAABB(), &dNear, &dFar))
		{
			// discard out of range
			if (dFar <= 0 || (distanceLimit && dFar > distanceLimit))
				return 0;

			// we're interested in the exit point
			// convert to standard form so we can use x/y always
			Ray terrainRay(convertWorldToTerrainAxes(localRay.getOrigin()), 
				convertWorldToTerrainAxes(localRay.getDirection()));


			Vector3 terrainIntersectPos = terrainRay.getPoint(dFar);
			Real x = terrainIntersectPos.x;
			Real y = terrainIntersectPos.y;
			Real dx = terrainRay.getDirection().x;
			Real dy = terrainRay.getDirection().y;

			if (Math::RealEqual(Math::Abs(x), Math::Abs(y)))
			{
				if (x > 0 && y > 0 && dx > 0 && dy > 0)
					return getNeighbour(NEIGHBOUR_NORTHEAST);
				if (x > 0 && y < 0 && dx > 0 && dy < 0)
					return getNeighbour(NEIGHBOUR_SOUTHEAST);
				if (x < 0 && y > 0 && dx < 0 && dy > 0)
					return getNeighbour(NEIGHBOUR_NORTHWEST);
				if (x < 0 && y < 0 && dx < 0 && dy < 0)
					return getNeighbour(NEIGHBOUR_SOUTHWEST);
			}
			if (x > 0 && x > y && dx > 0)
				return getNeighbour(NEIGHBOUR_EAST);
			if (x < 0 && x < y && dx < 0)
				return getNeighbour(NEIGHBOUR_WEST);
			if (y > 0 && y > x && dy > 0)
				return getNeighbour(NEIGHBOUR_NORTH);
			if (y < 0 && y < x && dy < 0)
				return getNeighbour(NEIGHBOUR_SOUTH);

		}

		return 0;
	}
	//---------------------------------------------------------------------
	void Terrain::_dumpTextures(const String& prefix, const String& suffix)
	{
		if (!mTerrainNormalMap.isNull())
		{
			Image img;
			mTerrainNormalMap->convertToImage(img);
			img.save(prefix + "_normalmap" + suffix);
		}

		if (!mColourMap.isNull())
		{
			Image img;
			mColourMap->convertToImage(img);
			img.save(prefix + "_colourmap" + suffix);
		}

		if (!mLightmap.isNull())
		{
			Image img;
			mLightmap->convertToImage(img);
			img.save(prefix + "_lightmap" + suffix);
		}

		if (!mCompositeMap.isNull())
		{
			Image img;
			mCompositeMap->convertToImage(img);
			img.save(prefix + "_compositemap" + suffix);
		}

		int blendTexture = 0;
		for (TexturePtrList::iterator i = mBlendTextureList.begin(); i != mBlendTextureList.end(); ++i, ++blendTexture)
		{
			if (!i->isNull())
			{
				Image img;
				(*i)->convertToImage(img);
				img.save(prefix + "_blendtexture" + StringConverter::toString(blendTexture) + suffix);
			}
		}

	}







	
	
}

