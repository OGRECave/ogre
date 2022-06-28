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
#include "OgreRoot.h"
#include "OgreTerrainQuadTreeNode.h"
#include "OgreTerrainLodManager.h"
#include "OgreStreamSerialiser.h"
#include "OgreLogManager.h"
#include "OgreTerrain.h"

namespace Ogre
{
    const uint16 TerrainLodManager::WORKQUEUE_LOAD_LOD_DATA_REQUEST = 1;
    const uint32 TerrainLodManager::TERRAINLODDATA_CHUNK_ID = StreamSerialiser::makeIdentifier("TLDA");
    const uint16 TerrainLodManager::TERRAINLODDATA_CHUNK_VERSION = 1;

    TerrainLodManager::TerrainLodManager(Terrain* t, DataStreamPtr& stream)
        : mTerrain(t)
    {
        init();
        mDataStream = stream;
        mStreamOffset = !mDataStream ? 0 : mDataStream->tell();
    }

    TerrainLodManager::TerrainLodManager(Terrain* t, const String& filename)
        : mTerrain(t), mStreamOffset(0)
    {
        init();
        open(filename);
    }

    void TerrainLodManager::open(const String& filename)
    {
        if(!filename.empty())
            mDataStream = Root::getSingleton().openFileStream(filename, mTerrain->_getDerivedResourceGroup());
    }

    void TerrainLodManager::close()
    {
        mDataStream.reset();
    }

    bool TerrainLodManager::isOpen() const
    {
        return mDataStream.get() != 0;
    }

    void TerrainLodManager::init()
    {
        mHighestLodPrepared = -1;
        mHighestLodLoaded = -1;
        mTargetLodLevel = -1;
        mIncreaseLodLevelInProgress = false;
        mLastRequestSynchronous = false;
        mLodInfoTable = 0;

        WorkQueue* wq = Root::getSingleton().getWorkQueue();
        mWorkQueueChannel = wq->getChannel("Ogre/TerrainLodManager");
        wq->addRequestHandler(mWorkQueueChannel, this);
        wq->addResponseHandler(mWorkQueueChannel, this);
    }

    TerrainLodManager::~TerrainLodManager()
    {
        waitForDerivedProcesses();
        WorkQueue* wq = Root::getSingleton().getWorkQueue();
        wq->removeRequestHandler(mWorkQueueChannel, this);
        wq->removeResponseHandler(mWorkQueueChannel, this);

        if(mLodInfoTable)
            OGRE_FREE(mLodInfoTable,MEMCATEGORY_GENERAL);
    }

    bool TerrainLodManager::canHandleRequest(const WorkQueue::Request* req, const WorkQueue* srcQ)
    {
        LoadLodRequest lreq = any_cast<LoadLodRequest>(req->getData());
        if (lreq.requestee != this)
            return false;
        return RequestHandler::canHandleRequest(req, srcQ);
    }
    //---------------------------------------------------------------------
    bool TerrainLodManager::canHandleResponse(const WorkQueue::Response* res, const WorkQueue* srcQ)
    {
        LoadLodRequest lreq = any_cast<LoadLodRequest>(res->getRequest()->getData());
        return (lreq.requestee == this);
    }

    WorkQueue::Response* TerrainLodManager::handleRequest(const WorkQueue::Request* req, const WorkQueue* srcQ)
    {
        LoadLodRequest lreq = any_cast<LoadLodRequest>(req->getData());
        // read data from file into temporary height & delta buffer
        try {
            if(lreq.currentPreparedLod>lreq.requestedLod)
                readLodData(lreq.currentPreparedLod-1, lreq.requestedLod);
        } catch (Exception& e) {
            return OGRE_NEW WorkQueue::Response(req, false, Any(), e.getFullDescription());
        }

        int lastTreeStart = -1;
        for( int level=lreq.currentLoadedLod-1; level>=lreq.requestedLod; --level )
        {
            LodInfo& lodinfo = getLodInfo(level);
            // skip re-assign
            if(lastTreeStart != (int)lodinfo.treeStart)
            {
                mTerrain->getQuadTree()->assignVertexData(lodinfo.treeStart, lodinfo.treeEnd,
                        lodinfo.resolution, lodinfo.size);
                lastTreeStart = lodinfo.treeStart;
            }
        }
        return OGRE_NEW WorkQueue::Response(req, true, Any());
    }

    void TerrainLodManager::handleResponse(const WorkQueue::Response* res, const WorkQueue* srcQ)
    {
        const WorkQueue::Request* req = res->getRequest();
        // No response data, just request
        LoadLodRequest lreq = any_cast<LoadLodRequest>(req->getData());

        mIncreaseLodLevelInProgress = false;

        if (res->succeeded())
        {
            // no others update LOD status
            if(lreq.currentPreparedLod == mHighestLodPrepared && lreq.currentLoadedLod == mHighestLodLoaded )
            {
                if( lreq.requestedLod < mHighestLodPrepared )
                    mHighestLodPrepared = lreq.requestedLod;

                int lastTreeStart = -1;
                for( int level = mHighestLodLoaded-1; level>=lreq.requestedLod && level>=mTargetLodLevel; --level )
                {
                    LodInfo& lodinfo = getLodInfo(level);
                    // skip re-load
                    if(lastTreeStart != (int)lodinfo.treeStart)
                    {
                        mTerrain->getQuadTree()->load(lodinfo.treeStart, lodinfo.treeEnd);
                        lastTreeStart = lodinfo.treeStart;
                    }
                    --mHighestLodLoaded;
                }
            }

            // has streamed in new data, should update terrain
            if(lreq.currentPreparedLod>lreq.requestedLod)
            {
                mTerrain->dirty();
                mTerrain->updateGeometryWithoutNotifyNeighbours();
            }

            // there are new requests
            if(mHighestLodLoaded != mTargetLodLevel)
                updateToLodLevel(mTargetLodLevel,mLastRequestSynchronous);
        }
        else
        {
            LogManager::getSingleton().stream(LML_CRITICAL) << "Failed to prepare and load terrain LOD: " << res->getMessages();
        }
    }
    void TerrainLodManager::buildLodInfoTable()
    {
        uint16 numLodLevels = mTerrain->getNumLodLevels();
        mLodInfoTable = OGRE_ALLOC_T(LodInfo, numLodLevels, MEMCATEGORY_GENERAL);

        uint16 size = mTerrain->getSize();
        uint16 depth = mTerrain->mTreeDepth;
        uint16 prevdepth = depth;
        uint16 last = 0;
        uint16 currresolution = size;
        uint16 bakedresolution = size;
        uint16 targetSplits = (bakedresolution - 1) / (Terrain::TERRAIN_MAX_BATCH_SIZE - 1);

        int *lodDepth = OGRE_ALLOC_T(int, numLodLevels, MEMCATEGORY_GENERAL);
        for(int level=0; level<numLodLevels; level++)
            lodDepth[level] = (level < mTerrain->getNumLodLevelsPerLeaf()) ? depth-1 : numLodLevels-level-1;

        while(depth-- && targetSplits)
        {
            uint splits = 1 << depth;
            if (splits == targetSplits)
            {
                for(uint level=0; level<numLodLevels; level++)
                {
                    if (lodDepth[level] >= depth && lodDepth[level] < prevdepth)
                    {
                        mLodInfoTable[level].treeStart = depth;
                        mLodInfoTable[level].treeEnd = prevdepth;
                        mLodInfoTable[level].isLast = level == last+prevdepth-depth-static_cast<uint>(1);
                        mLodInfoTable[level].resolution = bakedresolution;
                        mLodInfoTable[level].size = ((bakedresolution-1) / splits) + 1;
                        // this lod info has been filled
                        lodDepth[level] = -1;
                    }
                }
                // next set to look for
                bakedresolution =  ((currresolution - 1) >> 1) + 1;
                targetSplits = (bakedresolution - 1) / (Terrain::TERRAIN_MAX_BATCH_SIZE - 1);
                prevdepth = depth;
            }
            currresolution = ((currresolution - 1) >> 1) + 1;
            last++;
        }
        for(int level=0; level<numLodLevels; level++)
        {
            if (lodDepth[level]>=0 && lodDepth[level]<=prevdepth)
            {
                mLodInfoTable[level].treeStart = 0;
                mLodInfoTable[level].treeEnd = 1;
                mLodInfoTable[level].isLast = level == last+prevdepth-depth-1;
                mLodInfoTable[level].resolution = bakedresolution;
                mLodInfoTable[level].size = bakedresolution;
            }
        }
        OGRE_FREE(lodDepth,MEMCATEGORY_GENERAL);
    }

    // data are reorganized from lowest lod level(mNumLodLevels-1) to highest(0)
    void TerrainLodManager::separateData(float* data, uint16 size, uint16 numLodLevels, LodsData& lods)
    {
        lods.resize(numLodLevels);
        for (int level = numLodLevels - 1; level >= 0; level--)
        {
            unsigned int inc = 1 << level;
            unsigned int prev = 1 << (level + 1);

            for (uint16 y = 0; y < size; y += inc)
            {
                for (uint16 x = 0; x < size-1; x += inc)
                    if ((level == numLodLevels - 1) || ((x % prev != 0) || (y % prev != 0)))
                        lods[level].push_back( data[y*size + x] );
                if ((level == numLodLevels -1) || (y % prev) != 0)
                    lods[level].push_back( data[y*size + size-1] );
                if (y+inc > size)
                    break;
            }
        }
    }
    //---------------------------------------------------------------------
    void TerrainLodManager::updateToLodLevel(int lodLevel, bool synchronous /* = false */)
    {
        //init
        if(mHighestLodPrepared==-1)
            mHighestLodPrepared = mTerrain->getNumLodLevels();
        if(mHighestLodLoaded==-1)
            mHighestLodLoaded = mTerrain->getNumLodLevels();

        lodLevel = mTerrain->getPositiveLodLevel(lodLevel);

        mTargetLodLevel = lodLevel;
        mLastRequestSynchronous = synchronous;

        // need loading
        if(mTargetLodLevel<mHighestLodLoaded)
        {
            // no task is running
            if (!mIncreaseLodLevelInProgress)
            {
                mIncreaseLodLevelInProgress = true;
                LoadLodRequest req(this,mHighestLodPrepared,mHighestLodLoaded,mTargetLodLevel);
                Root::getSingleton().getWorkQueue()->addRequest(
                    mWorkQueueChannel, WORKQUEUE_LOAD_LOD_DATA_REQUEST,
                    req, 0, synchronous);
            }
            else if(synchronous)
                waitForDerivedProcesses();
        }
        // need unloading
        else if(mTargetLodLevel>mHighestLodLoaded)
        {
            for( int level=mHighestLodLoaded; level<mTargetLodLevel; level++ )
            {
                LodInfo& lod = getLodInfo(level);
                if(lod.isLast)
                {
                    mTerrain->getQuadTree()->unload(lod.treeStart, lod.treeEnd);
                    mHighestLodLoaded = level+1;
                }
            }
        }
    }

    // save each LOD level separately compressed so seek is possible
    void TerrainLodManager::saveLodData(StreamSerialiser& stream, Terrain* terrain)
    {
        uint16 numLodLevels = terrain->getNumLodLevels();

        LodsData lods;
        separateData(terrain->mHeightData, terrain->getSize(), numLodLevels, lods);
        separateData(terrain->mDeltaData, terrain->getSize(), numLodLevels, lods);

        for (int level = numLodLevels - 1; level >=0; level--)
        {
            stream.writeChunkBegin(TERRAINLODDATA_CHUNK_ID, TERRAINLODDATA_CHUNK_VERSION);
            stream.startDeflate();
            stream.write(&(lods[level][0]), lods[level].size());
            stream.stopDeflate();
            stream.writeChunkEnd(TERRAINLODDATA_CHUNK_ID);
        }
    }

    void TerrainLodManager::readLodData(uint16 lowerLodBound, uint16 higherLodBound)
    {
        if(!mDataStream) // No file to read from
            return;

        uint16 numLodLevels = mTerrain->getNumLodLevels();
        mDataStream->seek(mStreamOffset);
        StreamSerialiser stream(mDataStream);

        const StreamSerialiser::Chunk *mainChunk = stream.readChunkBegin(Terrain::TERRAIN_CHUNK_ID, Terrain::TERRAIN_CHUNK_VERSION);

        if(mainChunk->version > 1)
        {
            // skip the general information
            stream.readChunkBegin(Terrain::TERRAINGENERALINFO_CHUNK_ID, Terrain::TERRAINGENERALINFO_CHUNK_VERSION);
            stream.readChunkEnd(Terrain::TERRAINGENERALINFO_CHUNK_ID);

            // skip the previous lod data
            for(int skip=numLodLevels-1-lowerLodBound; skip>0; skip--)
            {
                stream.readChunkBegin(TERRAINLODDATA_CHUNK_ID, TERRAINLODDATA_CHUNK_VERSION);
                stream.readChunkEnd(TERRAINLODDATA_CHUNK_ID);
            }

            // uncompress
            uint maxSize = 2 * mTerrain->getGeoDataSizeAtLod(higherLodBound);
            float *lodData = OGRE_ALLOC_T(float, maxSize, MEMCATEGORY_GENERAL);

            for(int level=lowerLodBound; level>=higherLodBound; level-- )
            {
                // both height data and delta data
                uint dataSize = 2 * mTerrain->getGeoDataSizeAtLod(level);

                // reach and read the target lod data
                const StreamSerialiser::Chunk *c = stream.readChunkBegin(TERRAINLODDATA_CHUNK_ID,
                        TERRAINLODDATA_CHUNK_VERSION);
                stream.startDeflate(c->length);
                stream.read(lodData, dataSize);
                stream.stopDeflate();
                stream.readChunkEnd(TERRAINLODDATA_CHUNK_ID);

                fillBufferAtLod(level, lodData, dataSize);
            }
            stream.readChunkEnd(Terrain::TERRAIN_CHUNK_ID);

            OGRE_FREE(lodData, MEMCATEGORY_GENERAL);
        }
    }
    void TerrainLodManager::fillBufferAtLod(uint lodLevel, const float* data, uint dataSize )
    {
        unsigned int inc = 1 << lodLevel;
        unsigned int prev = 1 << (lodLevel + 1);
        uint16 numLodLevels = mTerrain->getNumLodLevels();
        uint16 size = mTerrain->getSize();

        const float* heightDataPtr = data;
        const float* deltaDataPtr = data+dataSize/2;

        for (uint16 y = 0; y < size; y += inc)
        {
            for (uint16 x = 0; x < size-1; x += inc)
                if ((lodLevel == numLodLevels - static_cast<uint>(1)) || (x % prev) || (y % prev))
                {
                    mTerrain->mHeightData[y*size + x] = *(heightDataPtr++);
                    mTerrain->mDeltaData[y*size + x] = *(deltaDataPtr++);
                }
            if ((lodLevel == numLodLevels - static_cast<uint>(1)) || (y % prev))
            {
                mTerrain->mHeightData[y*size + size-1] = *(heightDataPtr++);
                mTerrain->mDeltaData[y*size + size-1] = *(deltaDataPtr++);
            }
            if (y+inc > size)
                break;
        }
    }
    void TerrainLodManager::waitForDerivedProcesses()
    {
        while (mIncreaseLodLevelInProgress)
        {
            // we need to wait for this to finish
            OGRE_THREAD_SLEEP(50);
            Root::getSingleton().getWorkQueue()->processResponses();
        }
    }
}
