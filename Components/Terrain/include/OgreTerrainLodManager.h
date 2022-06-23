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

#ifndef __Ogre_TerrainLodManager_H__
#define __Ogre_TerrainLodManager_H__

#include "OgreTerrainPrerequisites.h"
#include "OgreWorkQueue.h"


namespace Ogre
{
    class Terrain;
    /** \addtogroup Optional
    *  @{
    */
    /** \addtogroup Terrain
    *  Some details on the terrain LOD manager
    *  @{
    */

    /** Terrain LOD data manager
    @par
        This class is used for managing terrain LOD data's loading, unloading.
    */

    class _OgreTerrainExport TerrainLodManager
    {
    public:
        static const uint32 TERRAINLODDATA_CHUNK_ID;
        static const uint16 TERRAINLODDATA_CHUNK_VERSION;
        typedef std::vector<float> LodData;
        typedef std::vector<LodData> LodsData;

        struct LoadLodRequest
        {
            LoadLodRequest( TerrainLodManager* r, uint16 preparedLod, uint16 loadedLod, uint16 target )
                : requestee(r)
                , currentPreparedLod(preparedLod)
                , currentLoadedLod(loadedLod)
                , requestedLod(target)
            {
            }
            TerrainLodManager* requestee;
            uint16 currentPreparedLod;
            uint16 currentLoadedLod;
            uint16 requestedLod;
        };

        struct LodInfo
        {
            uint treeStart;
            uint treeEnd;
            bool isLast;
            uint16 resolution;
            uint size;
        };
    public:
        TerrainLodManager(Terrain* t, DataStreamPtr& stream);
        TerrainLodManager(Terrain* t, const String& filename = "");
        virtual ~TerrainLodManager();

        void open(const String& filename);
        void close();
        bool isOpen() const;

        void updateToLodLevel(int lodLevel, bool synchronous = false);
        /// Save each LOD level separately compressed so seek is possible
        static void saveLodData(StreamSerialiser& stream, Terrain* terrain);

        /** Copy geometry data from buffer to mHeightData/mDeltaData
          @param lodLevel A LOD level to work with
          @param data, dataSize Buffer which holds geometry data if separated form
          @remarks Data in buffer has to be both height and delta data. First half is height data.
                Seconds half is delta data.
          */
        void fillBufferAtLod(uint lodLevel, const float* data, uint dataSize );
        /** Read separated geometry data from file into allocated memory
          @param lowerLodBound Lower bound of LOD levels to load
          @param higherLodBound Upper bound of LOD levels to load
          @remarks Geometry data are uncompressed using inflate() and stored into
                allocated buffer
          */
        void readLodData(uint16 lowerLodBound, uint16 higherLodBound);
        void waitForDerivedProcesses();

        int getHighestLodPrepared(){ return mHighestLodPrepared; }
        int getHighestLodLoaded(){ return mHighestLodLoaded; }
        int getTargetLodLevel(){ return mTargetLodLevel; }

        LodInfo& getLodInfo(uint lodLevel)
        {
            if(!mLodInfoTable)
                buildLodInfoTable();
            return mLodInfoTable[lodLevel];
        }
    private:
        WorkQueue::Response* handleRequest(const WorkQueue::Request* req, const WorkQueue* srcQ);
        void handleResponse(const WorkQueue::Response* res, const WorkQueue* srcQ);

        void init();
        void buildLodInfoTable();

        /** Separate geometry data by LOD level
        @param data A geometry data to separate i.e. mHeightData/mDeltaData
        @param size Dimension of the input data
        @param numLodLevels Number of LOD levels in the input data
        @param lods The separated LOD data
        @remarks Allocates new array and fills it with geometry data coupled by LOD level from lowest LOD level to highest. Example:
                before separation:
                00 01 02 03 04
                05 06 07 08 09
                10 11 12 13 14
                15 16 17 18 19
                20 21 22 23 24
                after separation:
                2: 00 04 20 24
                1: 02 10 12 14 22
                0: 01 03 05 06 07 08 09 11 13 15 16 17 18 19 21 23
          */
        static void separateData(float* data, uint16 size, uint16 numLodLevels, LodsData& lods );
    private:
        Terrain* mTerrain;
        DataStreamPtr mDataStream;
        size_t mStreamOffset;

        LodInfo* mLodInfoTable;
        int mTargetLodLevel;    /// Which LOD level is demanded
        int mHighestLodPrepared;  /// Highest LOD level stored in memory i.e. mHeightData/mDeltaData
        int mHighestLodLoaded;  /// Highest LOD level loaded in GPU

        bool mIncreaseLodLevelInProgress;  /// Is increaseLodLevel() running?
        bool mLastRequestSynchronous;
    };
    /** @} */
    /** @} */
}

#endif
