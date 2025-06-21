/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

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
#include "OgreCamera.h"
#include "OgreImage.h"
#include "OgrePixelFormat.h"
#include "OgreSceneManager.h"
#include "OgreSceneNode.h"
#include "OgreException.h"
#include "OgreBitwise.h"
#include "OgreViewport.h"
#include "OgreLogManager.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreTextureManager.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreRay.h"
#include "OgrePlane.h"
#include "OgreHardwareBufferManager.h"
#include "OgreMaterialManager.h"
#include "OgreTimer.h"
#include "OgreTerrainMaterialGeneratorA.h"

#if OGRE_COMPILER == OGRE_COMPILER_MSVC
// we do lots of conversions here, casting them all is tedious & cluttered, we know what we're doing
#   pragma warning (disable : 4244)
#endif
namespace Ogre
{
    //---------------------------------------------------------------------
    const uint32 Terrain::TERRAIN_CHUNK_ID = StreamSerialiser::makeIdentifier("TERR");
    const uint16 Terrain::TERRAIN_CHUNK_VERSION = 2;
    const uint32 Terrain::TERRAINGENERALINFO_CHUNK_ID = StreamSerialiser::makeIdentifier("TGIN");
    const uint16 Terrain::TERRAINGENERALINFO_CHUNK_VERSION = 1;
    const uint32 Terrain::TERRAINLAYERDECLARATION_CHUNK_ID = StreamSerialiser::makeIdentifier("TDCL");
    const uint16 Terrain::TERRAINLAYERDECLARATION_CHUNK_VERSION = 1;
    const uint32 Terrain::TERRAINLAYERSAMPLER_CHUNK_ID = StreamSerialiser::makeIdentifier("TSAM");
    const uint16 Terrain::TERRAINLAYERSAMPLER_CHUNK_VERSION = 1;
    const uint32 Terrain::TERRAINLAYERSAMPLERELEMENT_CHUNK_ID = StreamSerialiser::makeIdentifier("TSEL");
    const uint16 Terrain::TERRAINLAYERSAMPLERELEMENT_CHUNK_VERSION = 1;
    const uint32 Terrain::TERRAINLAYERINSTANCE_CHUNK_ID = StreamSerialiser::makeIdentifier("TLIN");
    const uint16 Terrain::TERRAINLAYERINSTANCE_CHUNK_VERSION = 1;
    const uint32 Terrain::TERRAINDERIVEDDATA_CHUNK_ID = StreamSerialiser::makeIdentifier("TDDA");
    const uint16 Terrain::TERRAINDERIVEDDATA_CHUNK_VERSION = 1;
    // since 129^2 is the greatest power we can address in 16-bit index
    const uint16 Terrain::TERRAIN_MAX_BATCH_SIZE = 129; 
    const uint32 Terrain::LOD_MORPH_CUSTOM_PARAM = 1001;
    const uint8 Terrain::DERIVED_DATA_DELTAS = 1;
    const uint8 Terrain::DERIVED_DATA_NORMALS = 2;
    const uint8 Terrain::DERIVED_DATA_LIGHTMAP = 4;
    // This MUST match the bitwise OR of all the types above with no extra bits!
    const uint8 Terrain::DERIVED_DATA_ALL = 7;
    //-----------------------------------------------------------------------
    template<> TerrainGlobalOptions* Singleton<TerrainGlobalOptions>::msSingleton = 0;
    TerrainGlobalOptions* TerrainGlobalOptions::getSingletonPtr(void)
    {
        return msSingleton;
    }
    TerrainGlobalOptions& TerrainGlobalOptions::getSingleton(void)
    {  
        assert( msSingleton );  return ( *msSingleton );  
    }
    // need to implement in cpp due to how Ogre::Singleton works
    TerrainGlobalOptions::~TerrainGlobalOptions() {}
    //---------------------------------------------------------------------
    TerrainGlobalOptions::TerrainGlobalOptions()
        : mSkirtSize(30)
        , mLightMapDir(Vector3(1, -1, 0).normalisedCopy())
        , mCastsShadows(false)
        , mMaxPixelError(3.0)
        , mRenderQueueGroup(RENDER_QUEUE_MAIN)
        , mVisibilityFlags(0xFFFFFFFF)
        , mQueryFlags(0xFFFFFFFF)
        , mUseRayBoxDistanceCalculation(false)
        , mLayerBlendMapSize(1024)
        , mDefaultLayerTextureWorldSize(10)
        , mDefaultGlobalColourMapSize(1024)
        , mLightmapSize(1024)
        , mCompositeMapSize(1024)
        , mCompositeMapAmbient(ColourValue::White)
        , mCompositeMapDiffuse(ColourValue::White)
        , mCompositeMapDistance(4000)
        , mResourceGroup(ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME)
        , mUseVertexCompressionWhenAvailable(true)
    {
    }
    //---------------------------------------------------------------------
    void TerrainGlobalOptions::setDefaultMaterialGenerator(const TerrainMaterialGeneratorPtr& gen)
    {
        mDefaultMaterialGenerator = gen;
    }
    //---------------------------------------------------------------------
    TerrainMaterialGeneratorPtr TerrainGlobalOptions::getDefaultMaterialGenerator()
    {
        if (!mDefaultMaterialGenerator)
        {
            // default
            mDefaultMaterialGenerator.reset(OGRE_NEW TerrainMaterialGeneratorA());
        }

        return mDefaultMaterialGenerator;
    }
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    NameGenerator Terrain::msBlendTextureGenerator = NameGenerator("TerrBlend");
    //---------------------------------------------------------------------
    Terrain::Terrain(SceneManager* sm)
        : mSceneMgr(sm)
        , mResourceGroup(BLANKSTRING)
        , mIsLoaded(false)
        , mModified(false)
        , mHeightDataModified(false)
        , mHeightData(0)
        , mDeltaData(0)
        , mAlign(ALIGN_X_Z)
        , mWorldSize(0)
        , mSize(0)
        , mMaxBatchSize(0)
        , mMinBatchSize(0)
        , mPos(Vector3::ZERO)
        , mQuadTree(0)
        , mNumLodLevels(0)
        , mNumLodLevelsPerLeafNode(0)
        , mTreeDepth(0)
        , mBase(0)
        , mScale(0)
        , mSkirtSize(0)
        , mRenderQueueGroup(0)
        , mVisibilityFlags(0)
        , mQueryFlags(0)
        , mDirtyGeometryRect(0, 0, 0, 0)
        , mDirtyDerivedDataRect(0, 0, 0, 0)
        , mDirtyGeometryRectForNeighbours(0, 0, 0, 0)
        , mDirtyLightmapFromNeighboursRect(0, 0, 0, 0)
        , mDerivedDataUpdateInProgress(false)
        , mDerivedUpdatePendingMask(0)
        , mGenerateMaterialInProgress(false)
        , mPrepareInProgress(false)
        , mMaterialGenerationCount(0)
        , mMaterialDirty(false)
        , mMaterialParamsDirty(false)
        , mLayerBlendMapSize(0)
        , mLayerBlendMapSizeActual(0)
        , mGlobalColourMapSize(0)
        , mGlobalColourMapEnabled(false)
        , mLightmapSize(0)
        , mLightmapSizeActual(0)
        , mCompositeMapSize(0)
        , mCompositeMapSizeActual(0)
        , mCompositeMapDirtyRect(0, 0, 0, 0)
        , mCompositeMapUpdateCountdown(0)
        , mLastMillis(0)
        , mCompositeMapDirtyRectLightmapUpdate(false)
        , mLodMorphRequired(false)
        , mNormalMapRequired(false)
        , mLightMapRequired(false)
        , mLightMapShadowsOnly(true)
        , mCompositeMapRequired(false)
        , mLastLODCamera(0)
        , mLastLODFrame(0)
        , mLastViewportHeight(0)
        , mCustomGpuBufferAllocator(0)
        , mLodManager(0)

    {
        mRootNode = sm->getRootSceneNode()->createChildSceneNode();
        sm->addListener(this);

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
        removeFromNeighbours();

        freeLodData();
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
            return mQuadTree->getBoundingBox();
    }
    //---------------------------------------------------------------------
    AxisAlignedBox Terrain::getWorldAABB() const
    {
        Affine3 m = Affine3::IDENTITY;
        m.setTrans(getPosition());

        AxisAlignedBox ret = getAABB();
        ret.transform(m);
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
        // force to load highest lod, or quadTree may contain hole
        load(0,true);

        bool wasOpen = false;

        if (mLodManager)
        {
            wasOpen = mLodManager->isOpen();
            mLodManager->close();
        }

        {
            DataStreamPtr stream = Root::createFileStream(filename, _getDerivedResourceGroup(), true);
            StreamSerialiser ser(stream);
            save(ser);
        }

        if (mLodManager && wasOpen)
            mLodManager->open(filename);
    }
    //---------------------------------------------------------------------
    void Terrain::save(StreamSerialiser& stream)
    {
        // wait for any queued processes to finish
        waitForDerivedProcesses();

        if (mHeightDataModified)
        {
            // When modifying, for efficiency we only increase the max deltas at each LOD,
            // we never reduce them (since that would require re-examining more samples)
            // Since we now save this data in the file though, we need to make sure we've
            // calculated the optimal
            Rect rect(0, 0, mSize, mSize);
            calculateHeightDeltas(rect);
            finaliseHeightDeltas(rect, false);
        }

        stream.writeChunkBegin(TERRAIN_CHUNK_ID, TERRAIN_CHUNK_VERSION);

        stream.writeChunkBegin(TERRAINGENERALINFO_CHUNK_ID, TERRAINGENERALINFO_CHUNK_VERSION);
        uint8 align = (uint8)mAlign;
        stream.write(&align);

        stream.write(&mSize);
        stream.write(&mWorldSize);
        stream.write(&mMaxBatchSize);
        stream.write(&mMinBatchSize);
        stream.write(&mPos);
        stream.writeChunkEnd(TERRAINGENERALINFO_CHUNK_ID);

        TerrainLodManager::saveLodData(stream,this);

        // start compressing
        stream.startDeflate();

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
                stream.write(mCpuBlendMapStorage[i].getData(), mCpuBlendMapStorage[i].getSize());
            }
        }
        else
        {
            if (mLayerBlendMapSize != mLayerBlendMapSizeActual)
            {
                LogManager::getSingleton().logWarning(
                    "blend maps were requested at a size larger than was supported "
                    "on this hardware, which means the quality has been degraded");
            }
            stream.write(&mLayerBlendMapSizeActual);
            Image tmp(PF_BYTE_RGBA, mLayerBlendMapSizeActual, mLayerBlendMapSizeActual);
            for (const auto& tex : mBlendTextureList)
            {
                // Must blit back in CPU format!
                tex->getBuffer()->blitToMemory(tmp.getPixelBox());
                stream.write(tmp.getData(), tmp.getSize());
            }
        }

        // other data
        // normals
		if (mNormalMapRequired)
		{
			stream.writeChunkBegin(TERRAINDERIVEDDATA_CHUNK_ID, TERRAINDERIVEDDATA_CHUNK_VERSION);
			String normalDataType("normalmap");
			stream.write(&normalDataType);
			stream.write(&mSize);
			if (mCpuTerrainNormalMap.getData())
			{
				// save from CPU data if it's there, it means GPU data was never created
				stream.write(mCpuTerrainNormalMap.getData(), mCpuTerrainNormalMap.getSize());
			}
			else
			{
                Image tmp(PF_BYTE_RGB, mSize, mSize);
				mTerrainNormalMap->getBuffer()->blitToMemory(tmp.getPixelBox());
				stream.write(tmp.getData(), tmp.getSize());
			}
			stream.writeChunkEnd(TERRAINDERIVEDDATA_CHUNK_ID);
		}

        // colourmap
        if (mGlobalColourMapEnabled)
        {
            stream.writeChunkBegin(TERRAINDERIVEDDATA_CHUNK_ID, TERRAINDERIVEDDATA_CHUNK_VERSION);
            String colourDataType("colourmap");
            stream.write(&colourDataType);
            stream.write(&mGlobalColourMapSize);
            if (mCpuColourMap.getData())
            {
                // save from CPU data if it's there, it means GPU data was never created
                stream.write(mCpuColourMap.getData(), mCpuColourMap.getSize());
            }
            else
            {
                Image tmp(PF_BYTE_RGB, mGlobalColourMapSize, mGlobalColourMapSize);
                mColourMap->getBuffer()->blitToMemory(tmp.getPixelBox());
                stream.write(tmp.getData(), tmp.getSize());
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
            if (mCpuLightmap.getData())
            {
                // save from CPU data if it's there, it means GPU data was never created
                stream.write(mCpuLightmap.getData(), mCpuLightmap.getSize());
            }
            else
            {
                Image tmp(PF_L8, mLightmapSize, mLightmapSize);
                mLightmap->getBuffer()->blitToMemory(tmp.getPixelBox());
                stream.write(tmp.getData(), tmp.getSize());
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
            if (mCpuCompositeMap.getData())
            {
                // save from CPU data if it's there, it means GPU data was never created
                stream.write(mCpuCompositeMap.getData(), mCpuCompositeMap.getSize());
            }
            else
            {
                // composite map is 4 channel, 3x diffuse, 1x specular mask
                Image tmp(PF_BYTE_RGBA, mCompositeMapSize, mCompositeMapSize);
                mCompositeMap->getBuffer()->blitToMemory(tmp.getPixelBox());
                stream.write(tmp.getData(), tmp.getSize());
            }
            stream.writeChunkEnd(TERRAINDERIVEDDATA_CHUNK_ID);
        }

        // write the quadtree
        mQuadTree->save(stream);

        // stop compressing
        stream.stopDeflate();

        stream.writeChunkEnd(TERRAIN_CHUNK_ID);

        mModified = false;
        mHeightDataModified = false;

    }
    //---------------------------------------------------------------------
    void Terrain::writeLayerDeclaration(const TerrainLayerDeclaration& decl, StreamSerialiser& stream)
    {
        // Layer declaration
        stream.writeChunkBegin(TERRAINLAYERDECLARATION_CHUNK_ID, TERRAINLAYERDECLARATION_CHUNK_VERSION);
        //  samplers
        uint8 numSamplers = (uint8)decl.size();
        stream.write(&numSamplers);
        for (const auto & sampler : decl)
        {
            stream.writeChunkBegin(TERRAINLAYERSAMPLER_CHUNK_ID, TERRAINLAYERSAMPLER_CHUNK_VERSION);
            stream.write(&sampler.alias);
            uint8 pixFmt = (uint8)sampler.format;
            stream.write(&pixFmt);
            stream.writeChunkEnd(TERRAINLAYERSAMPLER_CHUNK_ID);
        }
        //  elements
        uint8 numElems = 0;
        stream.write(&numElems);
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
        targetdecl.resize(numSamplers);
        for (uint8 s = 0; s < numSamplers; ++s)
        {
            if (!stream.readChunkBegin(TERRAINLAYERSAMPLER_CHUNK_ID, TERRAINLAYERSAMPLER_CHUNK_VERSION))
                return false;

            stream.read(&(targetdecl[s].alias));
            uint8 pixFmt;
            stream.read(&pixFmt);
            targetdecl[s].format = (PixelFormat)pixFmt;
            stream.readChunkEnd(TERRAINLAYERSAMPLER_CHUNK_ID);
        }
        //  elements are gone, keeping for backward compatibility
        uint8 numElems;
        stream.read(&numElems);
        for (uint8 e = 0; e < numElems; ++e)
        {
            if (!stream.readChunkBegin(TERRAINLAYERSAMPLERELEMENT_CHUNK_ID, TERRAINLAYERSAMPLERELEMENT_CHUNK_VERSION))
                return false;

            uint8 unused;
            stream.read(&unused); // source
            stream.read(&unused); // semantic
            stream.read(&unused); // start
            stream.read(&unused); // count
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
        for (const auto & inst : layers)
        {
            stream.writeChunkBegin(TERRAINLAYERINSTANCE_CHUNK_ID, TERRAINLAYERINSTANCE_CHUNK_VERSION);
            stream.write(&inst.worldSize);
            for (const auto & textureName : inst.textureNames)
            {
                stream.write(&textureName);
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
            return TerrainGlobalOptions::getSingleton().getDefaultResourceGroup();
        else
            return mResourceGroup;
    }
    //---------------------------------------------------------------------
    bool Terrain::prepare(const String& filename)
    {
        DataStreamPtr stream = ResourceGroupManager::getSingleton().openResource(filename, _getDerivedResourceGroup());

        return prepare(stream);
    }
    //---------------------------------------------------------------------
    bool Terrain::prepare(DataStreamPtr& stream)
    {
        freeLodData();
        mLodManager = OGRE_NEW TerrainLodManager( this, stream );
        StreamSerialiser ser(stream);
        return prepare(ser);
    }
    //---------------------------------------------------------------------
    bool Terrain::prepare(StreamSerialiser& stream)
    {
        mPrepareInProgress = true;

        freeTemporaryResources();
        freeCPUResources();

        if(mLodManager == NULL)
        {
            mLodManager = OGRE_NEW TerrainLodManager( this );
        }

        copyGlobalOptions();

        const StreamSerialiser::Chunk *mainChunk = stream.readChunkBegin(TERRAIN_CHUNK_ID, TERRAIN_CHUNK_VERSION);
        if (!mainChunk)
            return false;

        if(mainChunk->version > 1)
            stream.readChunkBegin(Terrain::TERRAINGENERALINFO_CHUNK_ID, Terrain::TERRAINGENERALINFO_CHUNK_VERSION);
        uint8 align;
        stream.read(&align);
        mAlign = (Alignment)align;
        stream.read(&mSize);
        stream.read(&mWorldSize);

        stream.read(&mMaxBatchSize);
        stream.read(&mMinBatchSize);
        stream.read(&mPos);
        mRootNode->setPosition(mPos);
        updateBaseScale();
        determineLodLevels();

        if(mainChunk->version > 1)
            stream.readChunkEnd(Terrain::TERRAINGENERALINFO_CHUNK_ID);

        size_t numVertices = mSize * mSize;
        mHeightData = OGRE_ALLOC_T(float, numVertices, MEMCATEGORY_GEOMETRY);
        mDeltaData = OGRE_ALLOC_T(float, numVertices, MEMCATEGORY_GEOMETRY);
        // As we may not load full data, so we should make it clean first
        memset(mHeightData, 0.0f, sizeof(float)*numVertices);
        memset(mDeltaData, 0.0f, sizeof(float)*numVertices);

        if(mainChunk->version > 1)
        {
            // skip height/delta data
            for (int i = 0; i < mNumLodLevels; i++)
            {
                stream.readChunkBegin(TerrainLodManager::TERRAINLODDATA_CHUNK_ID, TerrainLodManager::TERRAINLODDATA_CHUNK_VERSION);
                stream.readChunkEnd(TerrainLodManager::TERRAINLODDATA_CHUNK_ID);
            }

            // start uncompressing
            stream.startDeflate( mainChunk->length - stream.getOffsetFromChunkStart() );
        }
        else
        {
            stream.read(mHeightData, numVertices);
        }

        // Layer declaration
        if (!readLayerDeclaration(stream, mLayerDecl))
            return false;
        checkDeclaration();


        // Layers
        if (!readLayerInstanceList(stream, mLayerDecl.size(), mLayers))
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
            mCpuBlendMapStorage.emplace_back(PF_BYTE_RGBA, mLayerBlendMapSize, mLayerBlendMapSize);
            stream.read(mCpuBlendMapStorage.back().getData(), mCpuBlendMapStorage.back().getSize());
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
                mNormalMapRequired = true;
                mCpuTerrainNormalMap.create(PF_BYTE_RGB, sz, sz);
                stream.read(mCpuTerrainNormalMap.getData(), mCpuTerrainNormalMap.getSize());
                
            }
            else if (name == "colourmap")
            {
                mGlobalColourMapEnabled = true;
                mGlobalColourMapSize = sz;
                mCpuColourMap.create(PF_BYTE_RGB, sz, sz);
                stream.read(mCpuColourMap.getData(), mCpuColourMap.getSize());
            }
            else if (name == "lightmap")
            {
                mLightMapRequired = true;
                mLightmapSize = sz;
                mCpuLightmap.create(PF_L8, sz, sz);
                stream.read(mCpuLightmap.getData(), mCpuLightmap.getSize());
            }
            else if (name == "compositemap")
            {
                mCompositeMapRequired = true;
                mCompositeMapSize = sz;
                mCpuCompositeMap.create(PF_BYTE_RGBA, sz, sz);
                stream.read(mCpuCompositeMap.getData(), mCpuCompositeMap.getSize());
            }

            stream.readChunkEnd(TERRAINDERIVEDDATA_CHUNK_ID);


        }

        if(mainChunk->version == 1)
        {
            // Load delta data
            mDeltaData = OGRE_ALLOC_T(float, numVertices, MEMCATEGORY_GEOMETRY);
            stream.read(mDeltaData, numVertices);
        }

        // Create & load quadtree
        mQuadTree = OGRE_NEW TerrainQuadTreeNode(this, 0, 0, 0, mSize, mNumLodLevels - 1, 0);
        mQuadTree->prepare(stream);

        // stop uncompressing
        if(mainChunk->version > 1)
            stream.stopDeflate();

        stream.readChunkEnd(TERRAIN_CHUNK_ID);

        mModified = false;
        mHeightDataModified = false;

        mPrepareInProgress = false;

        return true;
    }
    //---------------------------------------------------------------------
    bool Terrain::prepare(const ImportData& importData)
    {
        mPrepareInProgress = true;
        freeTemporaryResources();
        freeLodData();
        freeCPUResources();
        mLodManager = OGRE_NEW TerrainLodManager( this );

        copyGlobalOptions();

        // validate
        if (!(Bitwise::isPO2(importData.terrainSize - 1) && Bitwise::isPO2(importData.minBatchSize - 1)
            && Bitwise::isPO2(importData.maxBatchSize - 1)))
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                "terrainSize, minBatchSize and maxBatchSize must all be 2^n + 1", 
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
        setPosition(importData.pos);

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
            for (uint16 i = 0; i < mSize; ++i)
            {
                uint32 srcy = mSize - i - 1;
                float* pDst = mHeightData + i * mSize;
                PixelUtil::bulkPixelConversion(img->getData(0, srcy), img->getFormat(), pDst, PF_FLOAT32_R,
                                               mSize);
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

        mQuadTree = OGRE_NEW TerrainQuadTreeNode(this, 0, 0, 0, mSize, mNumLodLevels - 1, 0);
        mQuadTree->prepare();

        // calculate entire terrain
        Rect rect(0, 0, mSize, mSize);
        calculateHeightDeltas(rect);
        finaliseHeightDeltas(rect, true);

        distributeVertexData();

        // Imported data is treated as modified because it's not saved
        mModified = true;
        mHeightDataModified = true;

        mPrepareInProgress = false;

        return true;

    }
    //---------------------------------------------------------------------
    void Terrain::copyGlobalOptions()
    {
        TerrainGlobalOptions& opts = TerrainGlobalOptions::getSingleton();
        mSkirtSize = opts.getSkirtSize();
        mRenderQueueGroup = opts.getRenderQueueGroup();
        mVisibilityFlags = opts.getVisibilityFlags();
        mQueryFlags = opts.getQueryFlags();
        mLayerBlendMapSize = opts.getLayerBlendMapSize();
        mLayerBlendMapSizeActual = mLayerBlendMapSize; // for now, until we check
        mLightmapSize = opts.getLightMapSize();
        mLightmapSizeActual = mLightmapSize; // for now, until we check
        mCompositeMapSize = opts.getCompositeMapSize();
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
        mNumLodLevelsPerLeafNode = (uint16) (Math::Log2(mMaxBatchSize - 1.0f) - Math::Log2(mMinBatchSize - 1.0f) + 1.0f);
        mNumLodLevels = (uint16) (Math::Log2(mSize - 1.0f) - Math::Log2(mMinBatchSize - 1.0f) + 1.0f);
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
                uint sz = ((bakedresolution-1) / splits) + 1;
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
            "Error while preparing " + filename + ", see log for details");
    }
    //---------------------------------------------------------------------
    void Terrain::load(StreamSerialiser& stream)
    {
        if (prepare(stream))
            load();
        else
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
            "Error while preparing from stream, see log for details");
    }
    //---------------------------------------------------------------------
    void Terrain::load(int lodLevel, bool synchronous)
    {
        if (mQuadTree)
            mLodManager->updateToLodLevel(lodLevel,synchronous);

        if (mIsLoaded || mGenerateMaterialInProgress)
            return;

        checkLayers(true);
        createOrDestroyGPUColourMap();
        createOrDestroyGPUNormalMap();
        createOrDestroyGPULightmap();
        createOrDestroyGPUCompositeMap();
        
        mMaterialGenerator->requestOptions(this);

        mGenerateMaterialInProgress = true;

        if(synchronous)
        {
            generateMaterial();
            return;
        }

        Root::getSingleton().getWorkQueue()->addMainThreadTask([this]() { generateMaterial(); });
    }
    //---------------------------------------------------------------------
    void Terrain::unload()
    {
        if (!mIsLoaded)
            return;

        if (mQuadTree)
            mQuadTree->unload();

        // free own buffers if used, but not custom
        mDefaultGpuBufferAllocator.freeAllBuffers();

        mIsLoaded = false;
        mModified = false;
        mHeightDataModified = false;

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
    float* Terrain::getHeightData(uint32 x, uint32 y) const
    {
        assert (x < mSize && y < mSize);
        return &mHeightData[y * mSize + x];
    }
    //---------------------------------------------------------------------
    float Terrain::getHeightAtPoint(uint32 x, uint32 y) const
    {
        // clamp
        x = std::min(x, mSize - 1u);
        y = std::min(y, mSize - 1u);

        int highestLod = mLodManager->getHighestLodPrepared();
        uint32 skip = 1 << (highestLod != -1 ? highestLod : 0);
        if (x % skip == 0 && y % skip == 0)
        return *getHeightData(x, y);

        uint32 x1 = std::min( (x/skip) * skip         , mSize - 1u );
        uint32 x2 = std::min( ((x+skip) / skip) * skip, mSize - 1u );
        uint32 y1 = std::min( (y/skip) * skip         , mSize - 1u );
        uint32 y2 = std::min( ((y+skip) / skip) * skip, mSize - 1u );

        float rx = (float(x % skip) / skip);
        float ry = (float(y % skip) / skip);

        return *getHeightData(x1, y1) * (1.0f-rx) * (1.0f-ry)
            + *getHeightData(x2, y1) * rx * (1.0f-ry)
            + *getHeightData(x1, y2) * (1.0f-rx) * ry
            + *getHeightData(x2, y2) * rx * ry;
    }
    //---------------------------------------------------------------------
    void Terrain::setHeightAtPoint(uint32 x, uint32 y, float h)
    {
        // force to load all data
        load(0,true);
        // clamp
        x = std::min(x, mSize - 1u);
        y = std::min(y, mSize - 1u);

        *getHeightData(x, y) = h;
        Rect rect;
        rect.left = x;
        rect.right = x+1;
        rect.top = y;
        rect.bottom = y+1;
        dirtyRect(rect);
    }
    //---------------------------------------------------------------------
    float Terrain::getHeightAtTerrainPosition(Real x, Real y) const
    {
        // get left / bottom points (rounded down)
        Real factor = (Real)mSize - 1.0f;
        Real invFactor = 1.0f / factor;

        uint32 startX = static_cast<uint32>(x * factor);
        uint32 startY = static_cast<uint32>(y * factor);
        uint32 endX = startX + 1;
        uint32 endY = startY + 1;

        // now get points in terrain space (effectively rounding them to boundaries)
        // note that we do not clamp! We need a valid plane
        Real startXTS = startX * invFactor;
        Real startYTS = startY * invFactor;
        Real endXTS = endX * invFactor;
        Real endYTS = endY * invFactor;

        // now clamp
        endX = std::min(endX, mSize-1u);
        endY = std::min(endY, mSize-1u);

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
        // define a plane in terrain space
        // do not normalise as the normalization factor cancels out in the final
        // equation anyway
        Vector4 plane;
        if (startY % 2)
        {
            // odd row
            bool secondTri = ((1.0 - yParam) > xParam);
            if (secondTri)
                plane = Math::calculateFaceNormalWithoutNormalize(v0, v1, v3);
            else
                plane = Math::calculateFaceNormalWithoutNormalize(v1, v2, v3);
        }
        else
        {
            // even row
            bool secondTri = (yParam > xParam);
            if (secondTri)
                plane = Math::calculateFaceNormalWithoutNormalize(v0, v2, v3);
            else
                plane = Math::calculateFaceNormalWithoutNormalize(v0, v1, v2);
        }

        // Solve plane equation for z
        return (-plane.x * x - plane.y * y - plane.w) / plane.z;
    }
    //---------------------------------------------------------------------
    float Terrain::getHeightAtWorldPosition(Real x, Real y, Real z) const
    {
        Vector3 terrPos;
        getTerrainPosition(x, y, z, &terrPos);
        return getHeightAtTerrainPosition(terrPos.x, terrPos.y);
    }
    //---------------------------------------------------------------------
    float Terrain::getHeightAtWorldPosition(const Vector3& pos) const
    {
        return getHeightAtWorldPosition(pos.x, pos.y, pos.z);
    }
    //---------------------------------------------------------------------
    const float* Terrain::getDeltaData() const
    {
        return mDeltaData;
    }
    //---------------------------------------------------------------------
    const float* Terrain::getDeltaData(uint32 x, uint32 y) const
    {
        assert (x < mSize && y < mSize);
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
                if (translation)
                    outVec -= mPos;
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
                    {
                        outVec.x -= mBase; outVec.y -= mBase;
                        outVec.x /= (mSize - 1) * mScale; outVec.y /= (mSize - 1) * mScale;
                    }
                    currSpace = TERRAIN_SPACE;
                    break;
                case LOCAL_SPACE:
                default:
                    break;
                };
                break;
            case TERRAIN_SPACE:
                switch(outSpace)
                {
                case WORLD_SPACE:
                case LOCAL_SPACE:
                    // go via local space
                    if (translation)
                    {
                        outVec.x *= (mSize - 1) * mScale; outVec.y *= (mSize - 1) * mScale;
                        outVec.x += mBase; outVec.y += mBase;
                    }
                    outVec = convertTerrainToWorldAxes(outVec);
                    currSpace = LOCAL_SPACE;
                    break;
                case POINT_SPACE:
                    if (translation)
                    {
                        outVec.x *= (mSize - 1); outVec.y *= (mSize - 1); 
                        // rounding up/down
                        // this is why POINT_SPACE is the last on the list, because it loses data
                        outVec.x = static_cast<Real>(static_cast<int>(outVec.x + 0.5));
                        outVec.y = static_cast<Real>(static_cast<int>(outVec.y + 0.5));
                    }
                    currSpace = POINT_SPACE;
                    break;
                case TERRAIN_SPACE:
                default:
                    break;
                };
                break;
            case POINT_SPACE:
                // always go via terrain space
                if (translation)
                {
                    outVec.x /= (mSize - 1); 
                    outVec.y /= (mSize - 1); 
                }
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
    void Terrain::getPoint(uint32 x, uint32 y, Vector3* outpos) const
    {
        getPointAlign(x, y, mAlign, outpos);
    }
    //---------------------------------------------------------------------
    void Terrain::getPoint(uint32 x, uint32 y, float height, Vector3* outpos) const
    {
        getPointAlign(x, y, height, mAlign, outpos);
    }
    //---------------------------------------------------------------------
    void Terrain::getPointAlign(uint32 x, uint32 y, Alignment align, Vector3* outpos) const
    {
        getPointAlign(x, y, *getHeightData(x, y), align, outpos);
    }
    //---------------------------------------------------------------------
    void Terrain::getPointAlign(uint32 x, uint32 y, float height, Alignment align, Vector3* outpos) const
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
    Affine3 Terrain::getPointTransform() const
    {
        auto outXform = Affine3::ZERO;
        switch(mAlign)
        {
            case ALIGN_X_Z:
                //outpos->y = height (z)
                outXform[1][2] = 1.0f;
                //outpos->x = x * mScale + mBase;
                outXform[0][0] = mScale;
                outXform[0][3] = mBase;
                //outpos->z = y * -mScale - mBase;
                outXform[2][1] = -mScale;
                outXform[2][3] = -mBase;
                break;
            case ALIGN_Y_Z:
                //outpos->x = height;
                outXform[0][2] = 1.0f;
                //outpos->z = x * -mScale - mBase;
                outXform[2][0] = -mScale;
                outXform[2][3] = -mBase;
                //outpos->y = y * mScale + mBase;
                outXform[1][1] = mScale;
                outXform[1][3] = mBase;
                break;
            case ALIGN_X_Y:
                //outpos->z = height;
                outXform[2][2] = 1.0f;
                //outpos->x = x * mScale + mBase;
                outXform[0][0] = mScale;
                outXform[0][3] = mBase;
                //outpos->y = y * mScale + mBase;
                outXform[1][1] = mScale;
                outXform[1][3] = mBase;
                break;
        };
        return outXform;
    }
    //---------------------------------------------------------------------
    void Terrain::getVector(const Vector3& inVec, Vector3* outVec) const
    {
        getVectorAlign(inVec.x, inVec.y, inVec.z, mAlign, outVec);
    }
    //---------------------------------------------------------------------
    void Terrain::getVector(Real x, Real y, Real z, Vector3* outVec) const
    {
        getVectorAlign(x, y, z, mAlign, outVec);
    }
    //---------------------------------------------------------------------
    void Terrain::getVectorAlign(const Vector3& inVec, Alignment align, Vector3* outVec) const
    {
        getVectorAlign(inVec.x, inVec.y, inVec.z, align, outVec);
    }
    //---------------------------------------------------------------------
    void Terrain::getVectorAlign(Real x, Real y, Real z, Alignment align, Vector3* outVec) const
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
    void Terrain::getPosition(const Vector3& TSpos, Vector3* outWSpos) const
    {
        getPositionAlign(TSpos, mAlign, outWSpos);
    }
    //---------------------------------------------------------------------
    void Terrain::getPosition(Real x, Real y, Real z, Vector3* outWSpos) const
    {
        getPositionAlign(x, y, z, mAlign, outWSpos);
    }
    //---------------------------------------------------------------------
    void Terrain::getTerrainPosition(const Vector3& WSpos, Vector3* outTSpos) const
    {
        getTerrainPositionAlign(WSpos, mAlign, outTSpos);
    }
    //---------------------------------------------------------------------
    void Terrain::getTerrainPosition(Real x, Real y, Real z, Vector3* outTSpos) const
    {
        getTerrainPositionAlign(x, y, z, mAlign, outTSpos);
    }
    //---------------------------------------------------------------------
    void Terrain::getPositionAlign(const Vector3& TSpos, Alignment align, Vector3* outWSpos) const
    {
        getPositionAlign(TSpos.x, TSpos.y, TSpos.z, align, outWSpos);
    }
    //---------------------------------------------------------------------
    void Terrain::getPositionAlign(Real x, Real y, Real z, Alignment align, Vector3* outWSpos) const
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
    void Terrain::getTerrainPositionAlign(const Vector3& WSpos, Alignment align, Vector3* outTSpos) const
    {
        getTerrainPositionAlign(WSpos.x, WSpos.y, WSpos.z, align, outTSpos);
    }
    //---------------------------------------------------------------------
    void Terrain::getTerrainPositionAlign(Real x, Real y, Real z, Alignment align, Vector3* outTSpos) const
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
    void Terrain::getTerrainVector(const Vector3& inVec, Vector3* outVec) const
    {
        getTerrainVectorAlign(inVec.x, inVec.y, inVec.z, mAlign, outVec);
    }
    //---------------------------------------------------------------------
    void Terrain::getTerrainVector(Real x, Real y, Real z, Vector3* outVec) const
    {
        getTerrainVectorAlign(x, y, z, mAlign, outVec);
    }
    //---------------------------------------------------------------------
    void Terrain::getTerrainVectorAlign(const Vector3& inVec, Alignment align, Vector3* outVec) const
    {
        getTerrainVectorAlign(inVec.x, inVec.y, inVec.z, align, outVec);
    }
    //---------------------------------------------------------------------
    void Terrain::getTerrainVectorAlign(Real x, Real y, Real z, Alignment align, Vector3* outVec) const
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
            return TerrainGlobalOptions::getSingleton().getDefaultLayerTextureWorldSize();
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
            mModified = true;
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
        if (layerIndex < mLayers.size() && samplerIndex < mLayerDecl.size())
        {
            return mLayers[layerIndex].textureNames[samplerIndex];
        }
        else
        {
            return BLANKSTRING;
        }

    }
    //---------------------------------------------------------------------
    void Terrain::setLayerTextureName(uint8 layerIndex, uint8 samplerIndex, const String& textureName)
    {
        if (layerIndex < mLayers.size() && samplerIndex < mLayerDecl.size())
        {
            if (mLayers[layerIndex].textureNames[samplerIndex] != textureName)
            {
                mLayers[layerIndex].textureNames[samplerIndex] = textureName;
                mMaterialDirty = true;
                mMaterialParamsDirty = true;
                mModified = true;
            }
        }
    }
    //---------------------------------------------------------------------
    void Terrain::setPosition(const Vector3& pos)
    {
        if (pos != mPos)
        {
            mPos = pos;
            mRootNode->setPosition(pos);
            updateBaseScale();
            mModified = true;
        }
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
        dirtyRect(Rect(0, 0, mSize, mSize));
    }
    //---------------------------------------------------------------------
    void Terrain::dirtyRect(const Rect& rect)
    {
        mDirtyGeometryRect.merge(rect);
        mDirtyGeometryRectForNeighbours.merge(rect);
        mDirtyDerivedDataRect.merge(rect);
        mCompositeMapDirtyRect.merge(rect);

        mModified = true;
        mHeightDataModified = true;

    }
    //---------------------------------------------------------------------
    void Terrain::_dirtyCompositeMapRect(const Rect& rect)
    {
        mCompositeMapDirtyRect.merge(rect);
        mModified = true;
    }
    //---------------------------------------------------------------------
    void Terrain::dirtyLightmapRect(const Rect& rect)
    {
        mDirtyDerivedDataRect.merge(rect);

        mModified = true;

    }
    //---------------------------------------------------------------------
    void Terrain::dirtyLightmap()
    {
        dirtyLightmapRect(Rect(0, 0, mSize, mSize));
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
    void Terrain::updateGeometryWithoutNotifyNeighbours()
    {
        if (!mDirtyGeometryRect.isNull())
        {
            mQuadTree->updateVertexData(true, false, mDirtyGeometryRect, false);
            mDirtyGeometryRect.setNull();
        }
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

        if(synchronous)
        {
            auto r = new WorkQueue::Request(0, 0, req, 0, 0);
            auto res = handleRequest(r, NULL);
            handleResponse(res, NULL);
            delete res;
            return;
        }

        Root::getSingleton().getWorkQueue()->addTask(
            [this, req]()
            {
                auto r = new WorkQueue::Request(0, 0, req, 0, 0);
                auto res = handleRequest(r, NULL);
                Root::getSingleton().getWorkQueue()->addMainThreadTask([this, res](){
                    handleResponse(res, NULL);
                    delete res;
                });
            });
    }
    //---------------------------------------------------------------------
    void Terrain::waitForDerivedProcesses()
    {
        while (mDerivedDataUpdateInProgress || mGenerateMaterialInProgress || mPrepareInProgress)
        {
            // we need to wait for this to finish
            OGRE_THREAD_SLEEP(50);
            Root::getSingleton().getWorkQueue()->processMainThreadTasks();
        }

    }
    //---------------------------------------------------------------------
    void Terrain::freeCPUResources()
    {
        OGRE_FREE(mHeightData, MEMCATEGORY_GEOMETRY);
        mHeightData = 0;

        OGRE_FREE(mDeltaData, MEMCATEGORY_GEOMETRY);
        mDeltaData = 0;

        OGRE_DELETE mQuadTree;
        mQuadTree = 0;

        mCpuTerrainNormalMap.freeMemory();
        mCpuColourMap.freeMemory();
        mCpuLightmap.freeMemory();
        mCpuCompositeMap.freeMemory();
    }
    //---------------------------------------------------------------------
    void Terrain::freeGPUResources()
    {
        // remove textures
        TextureManager* tmgr = TextureManager::getSingletonPtr();
        if (tmgr)
        {
            for (auto & i : mBlendTextureList)
            {   
                tmgr->remove(i->getHandle());
            }
            mBlendTextureList.clear();

            if (mTerrainNormalMap)
            {
                tmgr->remove(mTerrainNormalMap->getHandle());
                mTerrainNormalMap.reset();
            }

            if (mColourMap)
            {
                tmgr->remove(mColourMap->getHandle());
                mColourMap.reset();
            }

            if (mLightmap)
            {
                tmgr->remove(mLightmap->getHandle());
                mLightmap.reset();
            }

            if (mCompositeMap)
            {
                tmgr->remove(mCompositeMap->getHandle());
                mCompositeMap.reset();
            }
        }

        if (mMaterial)
        {
            MaterialManager::getSingleton().remove(mMaterial->getHandle());
            mMaterial.reset();
        }
        if (mCompositeMapMaterial)
        {
            MaterialManager::getSingleton().remove(mCompositeMapMaterial->getHandle());
            mCompositeMapMaterial.reset();
        }


    }
    //---------------------------------------------------------------------
    void Terrain::freeLodData()
    {
        if(mLodManager)
        {
            OGRE_DELETE mLodManager;
            mLodManager = 0;
        }
    }
    //---------------------------------------------------------------------
    Rect Terrain::calculateHeightDeltas(const Rect& rect)
    {
        Rect clampedRect = rect.intersect(Rect(0, 0, mSize, mSize));
        Rect finalRect(clampedRect);

        mQuadTree->preDeltaCalculation(clampedRect);

        /// Iterate over target levels, 
        for (int targetLevel = 1; targetLevel < mNumLodLevels; ++targetLevel)
        {
            int sourceLevel = targetLevel - 1;
            int step = 1 << targetLevel;
            // The step of the next higher LOD
//          int higherstep = step >> 1;

            // need to widen the dirty rectangle since change will affect surrounding
            // vertices at lower LOD
            Rect widenedRect(rect);
            widenedRect.left = std::max(0, widenedRect.left - step);
            widenedRect.top = std::max(0, widenedRect.top - step);
            widenedRect.right = std::min((int)mSize, widenedRect.right + step);
            widenedRect.bottom = std::min((int)mSize, widenedRect.bottom + step);

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

                    Vector4 t1, t2;
                    bool backwardTri = false;
                    // Odd or even in terms of target level
                    if ((j / step) % 2 == 0)
                    {
                        t1 = Math::calculateFaceNormalWithoutNormalize(v0, v1, v3);
                        t2 = Math::calculateFaceNormalWithoutNormalize(v0, v3, v2);
                    }
                    else
                    {
                        t1 = Math::calculateFaceNormalWithoutNormalize(v1, v3, v2);
                        t2 = Math::calculateFaceNormalWithoutNormalize(v0, v1, v2);
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
                            int fulldetailx = static_cast<int>(i + x);
                            int fulldetaily = static_cast<int>(j + y);
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
                                    (-t1.x * actualPos.x
                                    - t1.y * actualPos.y
                                    - t1.w) / t1.z;
                            }
                            else
                            {
                                // Second tri
                                interp_h = 
                                    (-t2.x * actualPos.x
                                    - t2.y * actualPos.y
                                    - t2.w) / t2.z;
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

        Rect clampedRect = rect.intersect(Rect(0, 0, mSize, mSize));
        // min/max information
        mQuadTree->finaliseDeltaValues(clampedRect);
        // delta vertex data
        mQuadTree->updateVertexData(false, true, clampedRect, cpuData);

    }

    //---------------------------------------------------------------------
    uint16 Terrain::getResolutionAtLod(uint16 lodLevel) const
    {
        return ((mSize - 1) >> lodLevel) + 1;
    }
    //---------------------------------------------------------------------
    uint Terrain::getGeoDataSizeAtLod(uint16 lodLevel) const
    {
        uint size = getResolutionAtLod(lodLevel);
        uint prevSize = (lodLevel<mNumLodLevels-1) ? getResolutionAtLod(lodLevel+1) : 0;
        return size*size - prevSize*prevSize;
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
        // only calculate LOD once per LOD camera, per frame, per viewport height
        const Camera* lodCamera = v->getCamera()->getLodCamera();
        unsigned long frameNum = Root::getSingleton().getNextFrameNumber();
        int vpHeight = v->getActualHeight();
        if (mLastLODCamera != lodCamera || frameNum != mLastLODFrame
            || mLastViewportHeight != vpHeight)
        {
            mLastLODCamera = lodCamera;
            mLastLODFrame = frameNum;
            mLastViewportHeight = vpHeight;
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
            Real maxPixelError = TerrainGlobalOptions::getSingleton().getMaxPixelError() * cam->_getLodBiasInverse();
            Viewport* lodVp = cam->getViewport();
            Real T = 2.0f * maxPixelError / (Real)lodVp->getActualHeight();

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
                OGRE_LOCK_RW_MUTEX_READ(mNeighbourMutex);
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
    std::pair<bool, Vector3> Terrain::checkQuadIntersection(int x, int z, const Ray& ray) const
    {
        // build the two planes belonging to the quad's triangles
        Vector3 v1 ((Real)x, *getHeightData(x,z), (Real)z);
        Vector3 v2 ((Real)x+1, *getHeightData(x+1,z), (Real)z);
        Vector3 v3 ((Real)x, *getHeightData(x,z+1), (Real)z+1);
        Vector3 v4 ((Real)x+1, *getHeightData(x+1,z+1), (Real)z+1);

        Vector4 p1, p2;
        bool oddRow = false;
        if (z % 2)
        {
            /* odd
            3---4
            | \ |
            1---2
            */
            p1 = Math::calculateFaceNormalWithoutNormalize(v2, v4, v3);
            p2 = Math::calculateFaceNormalWithoutNormalize(v1, v2, v3);
            oddRow = true;
        }
        else
        {
            /* even
            3---4
            | / |
            1---2
            */
            p1 = Math::calculateFaceNormalWithoutNormalize(v1, v2, v4);
            p2 = Math::calculateFaceNormalWithoutNormalize(v1, v4, v3);
        }

        // Test for intersection with the two planes. 
        // Then test that the intersection points are actually
        // still inside the triangle (with a small error margin)
        // Also check which triangle it is in
        RayTestResult planeInt = ray.intersects(Plane(p1));
        if (planeInt.first)
        {
            Vector3 where = ray.getPoint(planeInt.second);
            Vector3 rel = where - v1;
            if (rel.x >= -0.01 && rel.x <= 1.01 && rel.z >= -0.01 && rel.z <= 1.01 // quad bounds
                && ((rel.x >= rel.z && !oddRow) || (rel.x >= (1 - rel.z) && oddRow))) // triangle bounds
                return std::pair<bool, Vector3>(true, where);
        }
        planeInt = ray.intersects(Plane(p2));
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
        if (!mMaterial || 
            mMaterialGenerator->getChangeCount() != mMaterialGenerationCount ||
            mMaterialDirty)
        {
            const_cast<Terrain*>(this)->generateMaterial();
        }
        if (mMaterialParamsDirty)
        {
            mMaterialGenerator->updateParams(mMaterial, this);
            if(mCompositeMapRequired)
                mMaterialGenerator->updateParamsForCompositeMap(mCompositeMapMaterial, this);
            mMaterialParamsDirty = false;

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
        for (auto & layer : mLayers)
        {
            // adjust number of textureNames to number declared samplers
            layer.textureNames.resize(mLayerDecl.size());
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
        if (!mMaterialGenerator)
        {
            mMaterialGenerator = TerrainGlobalOptions::getSingleton().getDefaultMaterialGenerator();
        }

        if (mLayerDecl.empty())
        {
            // default the declaration
            mLayerDecl = mMaterialGenerator->getLayerDeclaration();
        }

    }
    //---------------------------------------------------------------------
    void Terrain::replaceLayer(uint8 index, bool keepBlends, Real worldSize, const StringVector* textureNames)
    {
        if (getLayerCount() > 0)
        {
            if (index >= getLayerCount())
                index = getLayerCount() - 1;

            LayerInstanceList::iterator i = mLayers.begin();
            std::advance(i, index);
            
            if (textureNames)
            {
                (*i).textureNames = *textureNames;
            }

            // use utility method to update UV scaling
            setLayerWorldSize(index, worldSize);

            // Delete the blend map if its not the base
            if ( !keepBlends && index > 0 )
            {
                if (mLayerBlendMapList[index-1])
                {
                    delete mLayerBlendMapList[index-1];
                    mLayerBlendMapList[index-1] = 0;
                }

                // Reset the layer to black
                std::pair<uint8,uint8> layerPair = getLayerBlendTextureIndex(index);
                clearGPUBlendChannel( layerPair.first, layerPair.second );
            }

            mMaterialDirty = true;
            mMaterialParamsDirty = true;
            mModified = true;
        }
    }
    //---------------------------------------------------------------------
    void Terrain::addLayer(Real worldSize, const StringVector* textureNames)
    {
        addLayer(getLayerCount(), worldSize, textureNames);
    }
    //---------------------------------------------------------------------
    void Terrain::addLayer(uint8 index, Real worldSize, const StringVector* textureNames)
    {
        if (!worldSize)
            worldSize = TerrainGlobalOptions::getSingleton().getDefaultLayerTextureWorldSize();

        uint8 blendIndex = std::max(index-1,0); 
        if (index >= getLayerCount())
        {
            mLayers.push_back(LayerInstance());
            index = getLayerCount() - 1;
        }
        else
        {
            LayerInstanceList::iterator i = mLayers.begin();
            std::advance(i, index);
            mLayers.insert(i, LayerInstance());

            RealVector::iterator uvi = mLayerUVMultiplier.begin();
            std::advance(uvi, index);
            mLayerUVMultiplier.insert(uvi, 0.0f);

            TerrainLayerBlendMapList::iterator bi = mLayerBlendMapList.begin();
            std::advance(bi, blendIndex);
            mLayerBlendMapList.insert(bi, static_cast<TerrainLayerBlendMap*>(0));
        }
        if (textureNames)
        {
            LayerInstance& inst = mLayers[index];
            inst.textureNames = *textureNames;
        }
        // use utility method to update UV scaling
        setLayerWorldSize(index, worldSize);
        checkLayers(true);

        // Is this an insert into the middle of the layer list?
        if (index < getLayerCount() - 1)
        {
            // Shift all GPU texture channels up one
            shiftUpGPUBlendChannels(blendIndex);

            // All blend maps above this layer index will need to be recreated since their buffers/channels have changed
            deleteBlendMaps(index);
        }

        mMaterialDirty = true;
        mMaterialParamsDirty = true;
        mModified = true;

    }
    //---------------------------------------------------------------------
    void Terrain::removeLayer(uint8 index)
    {
        if (index < mLayers.size())
        {
            uint8 blendIndex = std::max(index-1,0); 

            // Shift all GPU texture channels down one
            shiftDownGPUBlendChannels(blendIndex);

            LayerInstanceList::iterator i = mLayers.begin();
            std::advance(i, index);
            mLayers.erase(i);

            RealVector::iterator uvi = mLayerUVMultiplier.begin();
            std::advance(uvi, index);
            mLayerUVMultiplier.erase(uvi);

            if (mLayerBlendMapList.size() > 0)
            {
                // If they removed the base OR the first layer, we need to erase the first blend map
                TerrainLayerBlendMapList::iterator bi = mLayerBlendMapList.begin();
                std::advance(bi, blendIndex);
                OGRE_DELETE *bi;
                mLayerBlendMapList.erase(bi);

                // Check to see if a GPU textures can be released
                checkLayers(true);

                // All blend maps for layers above the erased will need to be recreated since their buffers/channels have changed
                deleteBlendMaps(blendIndex);
            }
            
            mMaterialDirty = true;
            mMaterialParamsDirty = true;
            mModified = true;
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
            mLayerBlendMapList[idx] = OGRE_NEW TerrainLayerBlendMap(this, layerIndex, tex->getBuffer().get());
        }

        return mLayerBlendMapList[idx];

    }
    //---------------------------------------------------------------------
    uint8 Terrain::getBlendTextureCount() const
    {
        return (uint8)mBlendTextureList.size();
    }
    //---------------------------------------------------------------------
    void Terrain::shiftUpGPUBlendChannels(uint8 index)
    {
        // checkLayers() has been called to make sure the blend textures have been created
        assert( mBlendTextureList.size() == getBlendTextureCount(getLayerCount()) );

        // Shift all blend channels > index UP one slot, possibly into the next texture
        // Example:  index = 2
        //      Before: [1 2 3 4] [5]
        //      After:  [1 2 0 3] [4 5]

        uint8 layerIndex = index + 1;
        for (uint8 i=getLayerCount()-1; i > layerIndex; --i )
        {   
            std::pair<uint8,uint8> destPair = getLayerBlendTextureIndex(i);
            std::pair<uint8,uint8> srcPair = getLayerBlendTextureIndex(i - 1);
            
            copyBlendTextureChannel( srcPair.first, srcPair.second, destPair.first, destPair.second );
        }

        // Reset the layer to black
        std::pair<uint8,uint8> layerPair = getLayerBlendTextureIndex(layerIndex);
        clearGPUBlendChannel( layerPair.first, layerPair.second );
    }
    //---------------------------------------------------------------------
    void Terrain::shiftDownGPUBlendChannels(uint8 index)
    {
        // checkLayers() has been called to make sure the blend textures have been created
        assert( mBlendTextureList.size() == getBlendTextureCount(getLayerCount()) );

        // Shift all blend channels above layerIndex DOWN one slot, possibly into the previous texture
        // Example:  index = 2
        //      Before: [1 2 3 4] [5]
        //      After:  [1 2 4 5] [0]

        uint8 layerIndex = index + 1;
        for (uint8 i=layerIndex; i < getLayerCount() - 1; ++i )
        {   
            std::pair<uint8,uint8> destPair = getLayerBlendTextureIndex(i);
            std::pair<uint8,uint8> srcPair = getLayerBlendTextureIndex(i + 1);
            
            copyBlendTextureChannel( srcPair.first, srcPair.second, destPair.first, destPair.second );
        }

        // Reset the layer to black
        if ( getLayerCount() > 1 )
        {
            std::pair<uint8,uint8> layerPair = getLayerBlendTextureIndex(getLayerCount() - 1);
            clearGPUBlendChannel( layerPair.first, layerPair.second );
        }
    }
    //---------------------------------------------------------------------
    void Terrain::copyBlendTextureChannel(uint8 srcIndex, uint8 srcChannel, uint8 destIndex, uint8 destChannel )
    {
        HardwarePixelBufferSharedPtr srcBuffer = getLayerBlendTexture(srcIndex)->getBuffer();
        HardwarePixelBufferSharedPtr destBuffer = getLayerBlendTexture(destIndex)->getBuffer();

        unsigned char rgbaShift[4];
        Box box(0, 0, destBuffer->getWidth(), destBuffer->getHeight());

        uint8* pDestBase = destBuffer->lock(box, HardwareBuffer::HBL_NORMAL).data;
        PixelUtil::getBitShifts(destBuffer->getFormat(), rgbaShift);
        uint8* pDest = pDestBase + rgbaShift[destChannel] / 8;
        size_t destInc = PixelUtil::getNumElemBytes(destBuffer->getFormat());

        size_t srcInc;
        uint8* pSrc;

        if ( destBuffer == srcBuffer )
        {
            pSrc = pDestBase + rgbaShift[srcChannel] / 8;
            srcInc = destInc;
        }
        else
        {
            pSrc = srcBuffer->lock(box, HardwareBuffer::HBL_READ_ONLY).data;
            PixelUtil::getBitShifts(srcBuffer->getFormat(), rgbaShift);
            pSrc += rgbaShift[srcChannel] / 8;
            srcInc = PixelUtil::getNumElemBytes(srcBuffer->getFormat());
        }

        for (size_t y = box.top; y < box.bottom; ++y)
        {
            for (size_t x = box.left; x < box.right; ++x)
            {
                *pDest = *pSrc;
                pSrc += srcInc;
                pDest += destInc;
            }
        }
        
        destBuffer->unlock();
        if ( destBuffer != srcBuffer )
            srcBuffer->unlock();
    }
    //---------------------------------------------------------------------
    void Terrain::clearGPUBlendChannel(uint8 index, uint channel)
    {
        HardwarePixelBufferSharedPtr buffer = getLayerBlendTexture(index)->getBuffer();

        unsigned char rgbaShift[4];
        Box box(0, 0, buffer->getWidth(), buffer->getHeight());

        uint8* pData = buffer->lock(box, HardwareBuffer::HBL_NORMAL).data;
        PixelUtil::getBitShifts(buffer->getFormat(), rgbaShift);
        pData += rgbaShift[channel] / 8;
        size_t inc = PixelUtil::getNumElemBytes(buffer->getFormat());

        for (size_t y = box.top; y < box.bottom; ++y)
        {
            for (size_t x = box.left; x < box.right; ++x)
            {
                *pData = 0;
                pData += inc;
            }
        }
        buffer->unlock();
    }
    //---------------------------------------------------------------------
    void Terrain::createGPUBlendTextures()
    {
        // Create enough RGBA textures to cope with blend layers
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
            // Use TU_STATIC because although we will update this, we won't do it every frame
            // in normal circumstances, so we don't want TU_DYNAMIC. Also we will 
            // read it (if we've cleared local temp areas) so no WRITE_ONLY
            mBlendTextureList[i] = TextureManager::getSingleton().createManual(
                msBlendTextureGenerator.generate(), _getDerivedResourceGroup(), 
                TEX_TYPE_2D, mLayerBlendMapSize, mLayerBlendMapSize, 1, 0, PF_BYTE_RGBA, TU_STATIC);

            mLayerBlendMapSizeActual = mBlendTextureList[i]->getWidth();

            if (mCpuBlendMapStorage.size() > i)
            {
                // Load blend data
                mBlendTextureList[i]->getBuffer()->blitFromMemory(mCpuBlendMapStorage[i].getPixelBox());
                // release CPU copy, don't need it anymore
                mCpuBlendMapStorage[i].freeMemory();
            }
            else
            {
                // initialise black
                auto buf = mBlendTextureList[i]->getBuffer();
                uint8* pInit = buf->lock(Box(buf->getSize()), HardwarePixelBuffer::HBL_DISCARD).data;
                memset(pInit, 0, buf->getSizeInBytes());
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

        // resize up (initialises to 0, populate as necessary)
        if ( mLayers.size() > 1 )
            mLayerBlendMapList.resize(mLayers.size() - 1, 0);

    }
    //---------------------------------------------------------------------
    void Terrain::createOrDestroyGPUNormalMap()
    {
        if (mNormalMapRequired && !mTerrainNormalMap)
        {
            // create
            mTerrainNormalMap = TextureManager::getSingleton().createManual(
                mMaterialName + "/nm", _getDerivedResourceGroup(), 
                TEX_TYPE_2D, mSize, mSize, 1, 0, PF_BYTE_RGB, TU_STATIC);

            // Upload loaded normal data if present
            if (mCpuTerrainNormalMap.getData())
            {
                mTerrainNormalMap->getBuffer()->blitFromMemory(mCpuTerrainNormalMap.getPixelBox());
                mCpuTerrainNormalMap.freeMemory();
            }
        }
        else if (!mNormalMapRequired && mTerrainNormalMap)
        {
            // destroy
            TextureManager::getSingleton().remove(mTerrainNormalMap->getHandle());
            mTerrainNormalMap.reset();
        }

    }
    //---------------------------------------------------------------------
    void Terrain::freeTemporaryResources()
    {
        // CPU blend maps
        mCpuBlendMapStorage.clear();

        // Editable structures for blend layers (not needed at runtime,  only blend textures are)
        deleteBlendMaps(0); 
    }
    //---------------------------------------------------------------------
    void Terrain::deleteBlendMaps(uint8 lowIndex)
    {
        TerrainLayerBlendMapList::iterator i = mLayerBlendMapList.begin();
        std::advance(i, lowIndex);
        for (; i != mLayerBlendMapList.end(); ++i )
        {
            OGRE_DELETE *i;
            *i = 0;
        }
    }
    //---------------------------------------------------------------------
    const TexturePtr& Terrain::getLayerBlendTexture(uint8 index) const
    {
        assert(index < mBlendTextureList.size());

        return mBlendTextureList[index];
    }
    //---------------------------------------------------------------------
    std::pair<uint8,uint8> Terrain::getLayerBlendTextureIndex(uint8 layerIndex) const
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
            mLightMapShadowsOnly = shadowsOnly;

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
            
            // if we enabled, generate composite maps
            if (mCompositeMapRequired)
            {
                mCompositeMapDirtyRect.left = mCompositeMapDirtyRect.top = 0;
                mCompositeMapDirtyRect.right = mCompositeMapDirtyRect.bottom = mSize;
                updateCompositeMap();
            }

        }

    }
    //---------------------------------------------------------------------
    bool Terrain::_getUseVertexCompression() const
    {
        return mMaterialGenerator->isVertexCompressionSupported() &&
            TerrainGlobalOptions::getSingleton().getUseVertexCompressionWhenAvailable();
    }
    //---------------------------------------------------------------------
    WorkQueue::Response* Terrain::handleRequest(const WorkQueue::Request* req, const WorkQueue* srcQ)
    {
        // Background thread (maybe)
        DerivedDataRequest ddr = any_cast<DerivedDataRequest>(req->getData());
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

        ddres.terrain = ddr.terrain;
        WorkQueue::Response* response = OGRE_NEW WorkQueue::Response(req, true, ddres);
        return response;
    }
    //---------------------------------------------------------------------
    void Terrain::handleResponse(const WorkQueue::Response* res, const WorkQueue* srcQ)
    {
        // Main thread
        DerivedDataResponse ddres = any_cast<DerivedDataResponse>(res->getData());
        DerivedDataRequest ddreq = any_cast<DerivedDataRequest>(res->getRequest()->getData());

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
    void Terrain::generateMaterial()
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

        mGenerateMaterialInProgress = false;
        mIsLoaded = true;
    }
    //---------------------------------------------------------------------
    uint16 Terrain::getLODLevelWhenVertexEliminated(long x, long y) const
    {
        // gets eliminated by either row or column first
        return std::min(getLODLevelWhenVertexEliminated(x), getLODLevelWhenVertexEliminated(y));
    }
    //---------------------------------------------------------------------
    uint16 Terrain::getLODLevelWhenVertexEliminated(long rowOrColulmn) const
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
            std::max(0, rect.left - 1),
            std::max(0, rect.top - 1),
            std::min((int32)mSize, rect.right + 1),
            std::min((int32)mSize, rect.bottom + 1)
            );
        // allocate memory for RGB
        uint8* pData = static_cast<uint8*>(
            OGRE_MALLOC(widenedRect.width() * widenedRect.height() * 3, MEMCATEGORY_GENERAL));

        PixelBox* pixbox = OGRE_NEW PixelBox(static_cast<uint32>(widenedRect.width()),
                                             static_cast<uint32>(widenedRect.height()), 1, PF_BYTE_RGB, pData);

        // Evaluate normal like this
        //  3---2---1
        //  | \ | / |
        //  4---P---0
        //  | / | \ |
        //  5---6---7

        for (int y = widenedRect.top; y < widenedRect.bottom; ++y)
        {
            for (int x = widenedRect.left; x < widenedRect.right; ++x)
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
                    cumulativeNormal += Math::calculateBasicFaceNormal(centrePoint, adjacentPoints[i], adjacentPoints[(i+1)%8]);
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
        if (mTerrainNormalMap)
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
                Box dstBox;
                dstBox.left = static_cast<uint32>(rect.left);
                dstBox.right = static_cast<uint32>(rect.right);
                dstBox.top = static_cast<uint32>(mSize - rect.bottom);
                dstBox.bottom = static_cast<uint32>(mSize - rect.top);
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

        for (auto & corner : corners)
        {
            Ray ray(corner + mPos, vec);
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
                    (terrainHitPos.x * (mSize - 1)),
                    (terrainHitPos.y * (mSize - 1)),
                    (terrainHitPos.x * (float)(mSize - 1) + 0.5) + 1,
                    (terrainHitPos.y * (float)(mSize - 1) + 0.5) + 1
                    );
                outRect.merge(mergeRect);
            }
        }

    }
    //---------------------------------------------------------------------
    PixelBox* Terrain::calculateLightmap(const Rect& rect, const Rect& extraTargetRect, Rect& outFinalRect)
    {
        // as well as calculating the lighting changes for the area that is
        // dirty, we also need to calculate the effect on casting shadow on
        // other areas. To do this, we project the dirt rect by the light direction
        // onto the minimum height


        const Vector3& lightVec = TerrainGlobalOptions::getSingleton().getLightMapDirection();
        Ogre::Rect widenedRect;
        widenRectByVector(lightVec, rect, widenedRect);

        // merge in the extra area (e.g. from neighbours)
        widenedRect.merge(extraTargetRect);

        // widenedRect now contains terrain point space version of the area we
        // need to calculate. However, we need to calculate in lightmap image space
        float terrainToLightmapScale = (float)mLightmapSizeActual / (float)mSize;
        widenedRect.left = (widenedRect.left * terrainToLightmapScale);
        widenedRect.right = (widenedRect.right * terrainToLightmapScale);
        widenedRect.top = (widenedRect.top * terrainToLightmapScale);
        widenedRect.bottom = (widenedRect.bottom * terrainToLightmapScale);

        // clamp 
        widenedRect = widenedRect.intersect(Rect(0, 0, mLightmapSizeActual, mLightmapSizeActual));

        outFinalRect = widenedRect;

        // allocate memory (L8)
        uint8* pData = static_cast<uint8*>(
            OGRE_MALLOC(widenedRect.width() * widenedRect.height(), MEMCATEGORY_GENERAL));

        PixelBox* pixbox = OGRE_NEW PixelBox(static_cast<uint32>(widenedRect.width()),
                                             static_cast<uint32>(widenedRect.height()), 1, PF_L8, pData);

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

                if (rayHit.first)
                    litVal = 0.0f;

                // encode as L8
                // invert the Y to deal with image space
                long storeX = x - widenedRect.left;
                long storeY = widenedRect.bottom - y - 1;

                uint8* pStore = pData + ((storeY * widenedRect.width()) + storeX);
                *pStore = (unsigned char)(litVal * 255.0);

            }
        }

        return pixbox;


    }
    //---------------------------------------------------------------------
    void Terrain::finaliseLightmap(const Rect& rect, PixelBox* lightmapBox)
    {
        createOrDestroyGPULightmap();
        // deal with race condition where lm has been disabled while we were working!
        if (mLightmap)
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
                Box dstBox;
                dstBox.left = static_cast<uint32>(rect.left);
                dstBox.right = static_cast<uint32>(rect.right);
                dstBox.top = static_cast<uint32>(mLightmapSizeActual - rect.bottom);
                dstBox.bottom = static_cast<uint32>(mLightmapSizeActual - rect.top);
                mLightmap->getBuffer()->blitFromMemory(*lightmapBox, dstBox);
            }
        }

        // delete memory
        OGRE_FREE(lightmapBox->data, MEMCATEGORY_GENERAL);
        OGRE_DELETE(lightmapBox);


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
                (mCompositeMapDirtyRect.width() < mSize || mCompositeMapDirtyRect.height() < mSize))
            {
                // widen the dirty rectangle since lighting makes it wider
                Rect widenedRect;
                widenRectByVector(TerrainGlobalOptions::getSingleton().getLightMapDirection(), mCompositeMapDirtyRect, widenedRect);
                // clamp
                widenedRect = widenedRect.intersect(Rect(0, 0, mSize, mSize));
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
        mCompositeMapUpdateCountdown = (long)(delay * 1000);
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
            sz = TerrainGlobalOptions::getSingleton().getDefaultGlobalColourMapSize();

        if (enabled != mGlobalColourMapEnabled ||
            (enabled && mGlobalColourMapSize != sz))
        {
            mGlobalColourMapEnabled = enabled;
            mGlobalColourMapSize = sz;

            createOrDestroyGPUColourMap();

            mMaterialDirty = true;
            mMaterialParamsDirty = true;
            mModified = true;
        }

    }
    //---------------------------------------------------------------------
    void Terrain::createOrDestroyGPUColourMap()
    {
        if (mGlobalColourMapEnabled && !mColourMap)
        {
            // create
            mColourMap = TextureManager::getSingleton().createManual(
                mMaterialName + "/cm", _getDerivedResourceGroup(), 
                TEX_TYPE_2D, mGlobalColourMapSize, mGlobalColourMapSize, MIP_DEFAULT, 
                PF_BYTE_RGB, TU_AUTOMIPMAP|TU_STATIC);

            if (mCpuColourMap.getData())
            {
                // Load cached data
                mColourMap->getBuffer()->blitFromMemory(mCpuColourMap.getPixelBox());
                // release CPU copy, don't need it anymore
                mCpuColourMap.freeMemory();
            }
        }
        else if (!mGlobalColourMapEnabled && mColourMap)
        {
            // destroy
            TextureManager::getSingleton().remove(mColourMap);
            mColourMap.reset();
        }

    }
    //---------------------------------------------------------------------
    void Terrain::createOrDestroyGPULightmap()
    {
        if (mLightMapRequired && !mLightmap)
        {
            // create
            mLightmap = TextureManager::getSingleton().createManual(
                mMaterialName + "/lm", _getDerivedResourceGroup(), 
                TEX_TYPE_2D, mLightmapSize, mLightmapSize, 0, PF_L8, TU_STATIC);

            mLightmapSizeActual = mLightmap->getWidth();

            if (mCpuLightmap.getData())
            {
                // Load cached data
                mLightmap->getBuffer()->blitFromMemory(mCpuLightmap.getPixelBox());
                // release CPU copy, don't need it anymore
                mCpuLightmap.freeMemory();
            }
            else
            {
                // initialise to full-bright
                Box box(0, 0, mLightmapSizeActual, mLightmapSizeActual);
                HardwarePixelBufferSharedPtr buf = mLightmap->getBuffer();
                uint8* pInit = buf->lock(box, HardwarePixelBuffer::HBL_DISCARD).data;
                memset(pInit, 255, mLightmapSizeActual * mLightmapSizeActual);
                buf->unlock();

            }
        }
        else if (!mLightMapRequired && mLightmap)
        {
            // destroy
            TextureManager::getSingleton().remove(mLightmap);
            mLightmap.reset();
        }

    }
    //---------------------------------------------------------------------
    void Terrain::createOrDestroyGPUCompositeMap()
    {
        if (mCompositeMapRequired && !mCompositeMap)
        {
            // create
            mCompositeMap = TextureManager::getSingleton().createManual(
                mMaterialName + "/comp", _getDerivedResourceGroup(), 
                TEX_TYPE_2D, mCompositeMapSize, mCompositeMapSize, MIP_DEFAULT, PF_BYTE_RGBA);

            mCompositeMapSizeActual = mCompositeMap->getWidth();

            if (mCpuCompositeMap.getData())
            {
                // Load cached data
                mCompositeMap->getBuffer()->blitFromMemory(mCpuCompositeMap.getPixelBox());
                // release CPU copy, don't need it anymore
                mCpuCompositeMap.freeMemory();
            }
            else
            {
                // initialise to black
                Box box(0, 0, mCompositeMapSizeActual, mCompositeMapSizeActual);
                HardwarePixelBufferSharedPtr buf = mCompositeMap->getBuffer();
                uint8* pInit = buf->lock(box, HardwarePixelBuffer::HBL_DISCARD).data;
                memset(pInit, 0, mCompositeMapSizeActual * mCompositeMapSizeActual * 4);
                buf->unlock();

            }
        }
        else if (!mCompositeMapRequired && mCompositeMap)
        {
            // destroy
            TextureManager::getSingleton().remove(mCompositeMap);
            mCompositeMap.reset();
        }

    }
    //---------------------------------------------------------------------
    Terrain* Terrain::getNeighbour(NeighbourIndex index) const
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

        // x == 0, given the check above
        if (y < 0)
            return NEIGHBOUR_SOUTH;
        else if (y > 0)
            return NEIGHBOUR_NORTH;
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
            Rect dirtyRectForNeighbours(mDirtyGeometryRectForNeighbours);
            mDirtyGeometryRectForNeighbours.setNull();
            // calculate light update rectangle
            const Vector3& lightVec = TerrainGlobalOptions::getSingleton().getLightMapDirection();
            Rect lightmapRect;
            widenRectByVector(lightVec, dirtyRectForNeighbours, getMinHeight(), getMaxHeight(), lightmapRect);

            for (int i = 0; i < (int)NEIGHBOUR_COUNT; ++i)
            {
                NeighbourIndex ni = static_cast<NeighbourIndex>(i);
                Terrain* neighbour = getNeighbour(ni);
                if (!neighbour)
                    continue;

                // Intersect the incoming rectangles with the edge regions related to this neighbour
                Rect edgeRect;
                getEdgeRect(ni, 2, &edgeRect);
                Rect heightEdgeRect = edgeRect.intersect(dirtyRectForNeighbours);
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

            for (int y = heightMatchRect.top; y < heightMatchRect.bottom; ++y)
            {
                for (int x = heightMatchRect.left; x < heightMatchRect.right; ++x)
                {
                    uint32 nx, ny;
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
            const Vector3& lightVec = TerrainGlobalOptions::getSingleton().getLightMapDirection();
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
    void Terrain::getEdgeRect(NeighbourIndex index, int32 range, Rect* outRect) const
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
        case NEIGHBOUR_COUNT:
        default:
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
        case NEIGHBOUR_COUNT:
        default:
            break;
        };
    }
    //---------------------------------------------------------------------
    void Terrain::getNeighbourEdgeRect(NeighbourIndex index, const Rect& inRect, Rect* outRect) const
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
    void Terrain::getNeighbourPoint(NeighbourIndex index, uint32 x, uint32 y, uint32 *outx, uint32 *outy) const
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
    void Terrain::getPointFromSelfOrNeighbour(int32 x, int32 y, Vector3* outpos) const
    {
        if (x >= 0 && y >=0 && x < mSize && y < mSize)
            getPoint(x, y, outpos);
        else
        {
            uint32 nx, ny;
            NeighbourIndex ni = NEIGHBOUR_EAST;
            getNeighbourPointOverflow(x, y, &ni, &nx, &ny);
            Terrain* neighbour = getNeighbour(ni);
            if (neighbour)
            {
                Vector3 neighbourPos = Vector3::ZERO;
                neighbour->getPoint(nx, ny, &neighbourPos);
                // adjust to make it relative to our position
                *outpos = neighbourPos + neighbour->getPosition() - getPosition();
            }
            else
            {
                // use our getPoint() after all, just clamp
                x = std::min(x, mSize - 1);
                y = std::min(y, mSize - 1);
                x = std::max(x, 0);
                y = std::max(y, 0);
                getPoint(x, y, outpos);
            }

        }
    }
    //---------------------------------------------------------------------
    void Terrain::getNeighbourPointOverflow(int32 x, int32 y, NeighbourIndex *outindex, uint32 *outx, uint32 *outy) const
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
        Ray modifiedRay(ray.getOrigin(), ray.getDirection());
        // Move back half a square - if we're on the edge of the AABB we might
        // miss the intersection otherwise; it's ok for everywhere else since
        // we want the far intersection anyway
        modifiedRay.setOrigin(modifiedRay.getPoint(-mWorldSize/mSize * 0.5f));

        // transform into terrain space
        Vector3 tPos, tDir;
        convertPosition(WORLD_SPACE, modifiedRay.getOrigin(), TERRAIN_SPACE, tPos);
        convertDirection(WORLD_SPACE, modifiedRay.getDirection(), TERRAIN_SPACE, tDir);
        // Discard rays with no lateral component
        if (Math::RealEqual(tDir.x, 0.0f, 1e-4) && Math::RealEqual(tDir.y, 0.0f, 1e-4))
            return 0;

        Ray terrainRay(tPos, tDir);
        // Intersect with boundary planes 
        // Only collide with the positive (exit) side of the plane, because we may be
        // querying from a point outside ourselves if we've cascaded more than once
        Real dist = std::numeric_limits<Real>::max();
        std::pair<bool, Real> intersectResult;
        if (tDir.x < 0.0f)
        {
            intersectResult = Math::intersects(terrainRay, Plane(Vector3::UNIT_X, Vector3::ZERO));
            if (intersectResult.first && intersectResult.second < dist)
                dist = intersectResult.second;
        }
        else if (tDir.x > 0.0f)
        {
            intersectResult = Math::intersects(terrainRay, Plane(Vector3::NEGATIVE_UNIT_X, Vector3(1,0,0)));
            if (intersectResult.first && intersectResult.second < dist)
                dist = intersectResult.second;
        }
        if (tDir.y < 0.0f)
        {
            intersectResult = Math::intersects(terrainRay, Plane(Vector3::UNIT_Y, Vector3::ZERO));
            if (intersectResult.first && intersectResult.second < dist)
                dist = intersectResult.second;
        }
        else if (tDir.y > 0.0f)
        {
            intersectResult = Math::intersects(terrainRay, Plane(Vector3::NEGATIVE_UNIT_Y, Vector3(0,1,0)));
            if (intersectResult.first && intersectResult.second < dist)
                dist = intersectResult.second;
        }


        // discard out of range
        if (dist * mWorldSize > distanceLimit)
            return 0;

        Vector3 terrainIntersectPos = terrainRay.getPoint(dist);
        Real x = terrainIntersectPos.x;
        Real y = terrainIntersectPos.y;
        Real dx = tDir.x;
        Real dy = tDir.y;

        // Never return diagonal directions, we will navigate those recursively anyway
        if (Math::RealEqual(x, 1.0f, 1e-4f) && dx > 0)
            return getNeighbour(NEIGHBOUR_EAST);
        else if (Math::RealEqual(x, 0.0f, 1e-4f) && dx < 0)
            return getNeighbour(NEIGHBOUR_WEST);
        else if (Math::RealEqual(y, 1.0f, 1e-4f) && dy > 0)
            return getNeighbour(NEIGHBOUR_NORTH);
        else if (Math::RealEqual(y, 0.0f, 1e-4f) && dy < 0)
            return getNeighbour(NEIGHBOUR_SOUTH);



        return 0;
    }
    //---------------------------------------------------------------------
    void Terrain::_dumpTextures(const String& prefix, const String& suffix)
    {
        if (mTerrainNormalMap)
        {
            Image img;
            mTerrainNormalMap->convertToImage(img);
            img.save(prefix + "_normalmap" + suffix);
        }

        if (mColourMap)
        {
            Image img;
            mColourMap->convertToImage(img);
            img.save(prefix + "_colourmap" + suffix);
        }

        if (mLightmap)
        {
            Image img;
            mLightmap->convertToImage(img);
            img.save(prefix + "_lightmap" + suffix);
        }

        if (mCompositeMap)
        {
            Image img;
            mCompositeMap->convertToImage(img);
            img.save(prefix + "_compositemap" + suffix);
        }

        int blendTexture = 0;
        for (TexturePtrList::iterator i = mBlendTextureList.begin(); i != mBlendTextureList.end(); ++i, ++blendTexture)
        {
            if (*i)
            {
                Image img;
                (*i)->convertToImage(img);
                img.save(prefix + "_blendtexture" + StringConverter::toString(blendTexture) + suffix);
            }
        }

    }
    //---------------------------------------------------------------------
    void Terrain::setGpuBufferAllocator(GpuBufferAllocator* alloc)
    {
        if (alloc != getGpuBufferAllocator())
        {
            if (isLoaded())
                OGRE_EXCEPT(Exception::ERR_INVALID_STATE,
                "Cannot alter the allocator when loaded!");

            mCustomGpuBufferAllocator = alloc;
        }
    }
    //---------------------------------------------------------------------
    Terrain::GpuBufferAllocator* Terrain::getGpuBufferAllocator()
    {
        if (mCustomGpuBufferAllocator)
            return mCustomGpuBufferAllocator;
        else
            return &mDefaultGpuBufferAllocator;
    }
    //---------------------------------------------------------------------
    size_t Terrain::getPositionBufVertexSize() const
    {
        size_t sz = 0;
        if (_getUseVertexCompression())
        {
            // short2 position
            sz += sizeof(short) * 2;
            // float1 height
            sz += sizeof(float);
        }
        else
        {
            // float3 position
            sz += sizeof(float) * 3;
            // float2 uv
            sz += sizeof(float) * 2;
        }

        return sz;

    }
    //---------------------------------------------------------------------
    size_t Terrain::getDeltaBufVertexSize() const
    {
        // float2(delta, deltaLODthreshold)
        return sizeof(float) * 2;
    }
    //---------------------------------------------------------------------
    size_t Terrain::_getNumIndexesForBatchSize(uint16 batchSize)
    {
        size_t mainIndexesPerRow = batchSize * 2 + 1;
        size_t numRows = batchSize - 1;
        size_t mainIndexCount = mainIndexesPerRow * numRows;
        // skirts share edges, so they take 1 less row per side than batchSize, 
        // but with 2 extra at the end (repeated) to finish the strip
        // * 2 for the vertical line, * 4 for the sides, +2 to finish
        size_t skirtIndexCount = (batchSize - 1) * 2 * 4 + 2;
        return mainIndexCount + skirtIndexCount;

    }
    //---------------------------------------------------------------------
    void Terrain::_populateIndexBuffer(uint16* pI, uint16 batchSize, 
        uint16 vdatasize, uint16 vertexIncrement, uint16 xoffset, uint16 yoffset, uint16 numSkirtRowsCols,
        uint16 skirtRowColSkip)
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

        uint16 rowSize = vdatasize * vertexIncrement;
        uint16 numRows = batchSize - 1;

        // Start on the right
        uint16 currentVertex = (batchSize - 1) * vertexIncrement;
        // but, our quad area might not start at 0 in this vertex data
        // offsets are at main terrain resolution, remember
        uint16 columnStart = xoffset;
        uint16 rowStart = yoffset;
        currentVertex += rowStart * vdatasize + columnStart;
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
            int edgeIncrement = 0, skirtIncrement = 0;
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
            uint16 skirtIndex = _calcSkirtVertexIndex(currentVertex, vdatasize, 
                (s % 2) != 0, numSkirtRowsCols, skirtRowColSkip);
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
            }
        }

    }
    //---------------------------------------------------------------------
    uint16 Terrain::_calcSkirtVertexIndex(uint16 mainIndex, uint16 vdatasize, bool isCol, 
        uint16 numSkirtRowsCols, uint16 skirtRowColSkip)
    {
        // row / col in main vertex resolution
        uint16 row = mainIndex / vdatasize;
        uint16 col = mainIndex % vdatasize;

        // skirts are after main vertices, so skip them
        uint16 base = vdatasize * vdatasize;

        // The layout in vertex data is:
        // 1. row skirts
        //    numSkirtRowsCols rows of resolution vertices each
        // 2. column skirts
        //    numSkirtRowsCols cols of resolution vertices each

        // No offsets used here, this is an index into the current vertex data, 
        // which is already relative
        if (isCol)
        {
            uint16 skirtNum = col / skirtRowColSkip;
            uint16 colbase = numSkirtRowsCols * vdatasize;
            return base + colbase + vdatasize * skirtNum + row;
        }
        else
        {
            uint16 skirtNum = row / skirtRowColSkip;
            return base + vdatasize * skirtNum + col;
        }

    }
    //---------------------------------------------------------------------
    void Terrain::setWorldSize(Real newWorldSize)
    {
        if(mWorldSize != newWorldSize)
        {
            waitForDerivedProcesses();

            mWorldSize = newWorldSize;

            updateBaseScale();

            deriveUVMultipliers();

            mMaterialParamsDirty = true;

            if(mIsLoaded)
            {
                Rect dRect(0, 0, mSize, mSize);
                dirtyRect(dRect);
                update();
            }

            mModified = true;
        }
    }
    //---------------------------------------------------------------------
    void Terrain::setSize(uint16 newSize)
    {
        if(mSize != newSize)
        {
            waitForDerivedProcesses();
            // load full HeightData
            load(0,true);

            size_t numVertices = newSize * newSize;

            PixelBox src(mSize, mSize, 1, Ogre::PF_FLOAT32_R, (void*)getHeightData());

            float* tmpData = OGRE_ALLOC_T(float, numVertices, MEMCATEGORY_GENERAL);

            PixelBox dst(newSize, newSize, 1, Ogre::PF_FLOAT32_R, tmpData);

            Image::scale(src, dst, Image::FILTER_BILINEAR);

            if (mTerrainNormalMap)
            {
                TextureManager::getSingletonPtr()->remove(mTerrainNormalMap->getHandle());
                mTerrainNormalMap.reset();
            }

            freeLodData();
            freeCPUResources();
            mLodManager = OGRE_NEW TerrainLodManager( this );
            mSize = newSize;

            determineLodLevels();

            updateBaseScale();

            deriveUVMultipliers();

            mMaterialParamsDirty = true;

            mHeightData = tmpData;
            mDeltaData = OGRE_ALLOC_T(float, numVertices, MEMCATEGORY_GEOMETRY);
            memset(mDeltaData, 0, sizeof(float) * numVertices);

            mQuadTree = OGRE_NEW TerrainQuadTreeNode(this, 0, 0, 0, mSize, mNumLodLevels - 1, 0);
            mQuadTree->prepare();

            // calculate entire terrain
            Rect rect;
            rect.top = 0; rect.bottom = mSize;
            rect.left = 0; rect.right = mSize;
            calculateHeightDeltas(rect);
            finaliseHeightDeltas(rect, true);

            if(mIsLoaded)
            {
                load(0,true);
            }

            mModified = true;
        }
    }
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    Terrain::DefaultGpuBufferAllocator::DefaultGpuBufferAllocator()
    {

    }
    //---------------------------------------------------------------------
    Terrain::DefaultGpuBufferAllocator::~DefaultGpuBufferAllocator()
    {
        freeAllBuffers();
    }
    //---------------------------------------------------------------------
    void Terrain::DefaultGpuBufferAllocator::allocateVertexBuffers(Terrain* forTerrain, 
        size_t numVertices, HardwareVertexBufferSharedPtr& destPos, HardwareVertexBufferSharedPtr& destDelta)
    {
        destPos = getVertexBuffer(mFreePosBufList, forTerrain->getPositionBufVertexSize(), numVertices);
        destDelta = getVertexBuffer(mFreeDeltaBufList, forTerrain->getDeltaBufVertexSize(), numVertices);

    }
    //---------------------------------------------------------------------
    HardwareVertexBufferSharedPtr Terrain::DefaultGpuBufferAllocator::getVertexBuffer(
        VBufList& list, size_t vertexSize, size_t numVertices)
    {
        size_t sz = vertexSize * numVertices;
        for (VBufList::iterator i = list.begin(); i != list.end(); ++i)
        {
            if ((*i)->getSizeInBytes() == sz)
            {
                HardwareVertexBufferSharedPtr ret = *i;
                list.erase(i);
                return ret;
            }
        }
        // Didn't find one?
        return HardwareBufferManager::getSingleton()
            .createVertexBuffer(vertexSize, numVertices, HardwareBuffer::HBU_STATIC_WRITE_ONLY);


    }
    //---------------------------------------------------------------------
    void Terrain::DefaultGpuBufferAllocator::freeVertexBuffers(
        const HardwareVertexBufferSharedPtr& posbuf, const HardwareVertexBufferSharedPtr& deltabuf)
    {
        mFreePosBufList.push_back(posbuf);
        mFreeDeltaBufList.push_back(deltabuf);
    }
    //---------------------------------------------------------------------
    HardwareIndexBufferSharedPtr Terrain::DefaultGpuBufferAllocator::getSharedIndexBuffer(uint16 batchSize, 
        uint16 vdatasize, size_t vertexIncrement, uint16 xoffset, uint16 yoffset, uint16 numSkirtRowsCols, 
        uint16 skirtRowColSkip)
    {
        uint32 hsh = hashIndexBuffer(batchSize, vdatasize, vertexIncrement, xoffset, yoffset, 
            numSkirtRowsCols, skirtRowColSkip);

        IBufMap::iterator i = mSharedIBufMap.find(hsh);
        if (i == mSharedIBufMap.end())
        {
            // create new
            size_t indexCount = Terrain::_getNumIndexesForBatchSize(batchSize);
            HardwareIndexBufferSharedPtr ret = HardwareBufferManager::getSingleton()
                .createIndexBuffer(HardwareIndexBuffer::IT_16BIT, indexCount, 
                HardwareBuffer::HBU_STATIC_WRITE_ONLY);
            uint16* pI = static_cast<uint16*>(ret->lock(HardwareBuffer::HBL_DISCARD));
            Terrain::_populateIndexBuffer(pI, batchSize, vdatasize, uint16(vertexIncrement), xoffset, yoffset, numSkirtRowsCols, skirtRowColSkip);
            ret->unlock();

            mSharedIBufMap[hsh] = ret;
            return ret;
        }
        else
            return i->second;

    }
    //---------------------------------------------------------------------
    void Terrain::DefaultGpuBufferAllocator::freeAllBuffers()
    {
        mFreePosBufList.clear();
        mFreeDeltaBufList.clear();
        mSharedIBufMap.clear();
    }
    //---------------------------------------------------------------------
    void Terrain::DefaultGpuBufferAllocator::warmStart(size_t numInstances, uint16 terrainSize, uint16 maxBatchSize, 
        uint16 minBatchSize)
    {
        // TODO

    }
    //---------------------------------------------------------------------
    uint32 Terrain::DefaultGpuBufferAllocator::hashIndexBuffer(uint16 batchSize, 
        uint16 vdatasize, size_t vertexIncrement, uint16 xoffset, uint16 yoffset, uint16 numSkirtRowsCols, 
        uint16 skirtRowColSkip)
    {
        uint32 ret = 0;
        ret = HashCombine(ret, batchSize);
        ret = HashCombine(ret, vdatasize);
        ret = HashCombine(ret, vertexIncrement);
        ret = HashCombine(ret, xoffset);
        ret = HashCombine(ret, yoffset);
        ret = HashCombine(ret, numSkirtRowsCols);
        ret = HashCombine(ret, skirtRowColSkip);
        return ret;

    }
    //---------------------------------------------------------------------
    void Terrain::increaseLodLevel(bool synchronous /* = false */)
    {
        int lodLevel = mLodManager->getTargetLodLevel();
        if(lodLevel<0)
            mLodManager->updateToLodLevel(-1, synchronous);
        else if( --lodLevel >= 0 )
            mLodManager->updateToLodLevel(lodLevel, synchronous);
    }
    //---------------------------------------------------------------------
    void Terrain::decreaseLodLevel()
    {
        int lodLevel = mLodManager->getTargetLodLevel() + 1;
        if( lodLevel>0 && lodLevel<mNumLodLevels )
            mLodManager->updateToLodLevel(lodLevel);
    }

    void Terrain::removeFromNeighbours()
    {
        // We are reading the list of neighbours here
        OGRE_LOCK_RW_MUTEX_READ(mNeighbourMutex);
        for (int i = 0; i < (int)NEIGHBOUR_COUNT; ++i)
        {
            NeighbourIndex ni = static_cast<NeighbourIndex>(i);
            Terrain* neighbour = getNeighbour(ni);
            if (!neighbour)
                continue;

            OGRE_LOCK_RW_MUTEX_WRITE(neighbour->mNeighbourMutex);
            // TODO: do we want to re-calculate? probably not, but not sure
            neighbour->setNeighbour(getOppositeNeighbour(ni), 0, false, false);
        }
    }
}
