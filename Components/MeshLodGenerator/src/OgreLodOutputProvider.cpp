/*
 * -----------------------------------------------------------------------------
 * This source file is part of OGRE
 * (Object-oriented Graphics Rendering Engine)
 * For the latest info, see http://www.ogre3d.org/
 *
 * Copyright (c) 2000-2014 Torus Knot Software Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * -----------------------------------------------------------------------------
 */

#include "OgreLodOutputProvider.h"

namespace Ogre
{
    LodOutputProvider::LodOutputProvider(bool useCompression)
    : mUseCompression(useCompression)
    , mFirstBufferPass(true)
    , mLastIndexBufferID(0)
    {}

    void LodOutputProvider::prepare(LodData *data)
    {
        if (mUseCompression)
        {
            mFirstBufferPass = true;
            mTriangleCacheList.resize(data->mTriangleList.size());
            mLineCacheList.resize(data->mLineList.size());
        }
    }

    void LodOutputProvider::finalize(LodData *data)
    {
        if (mUseCompression)
        {
            if (!mFirstBufferPass)
            {
                // Uneven number of Lod levels. We need to bake the last one separately.
                bakeUncompressed(data, mLastIndexBufferID);
            }
        }
    }

    void LodOutputProvider::bakeManualLodLevel( LodData* data, String& manualMeshName, int lodIndex)
    {
        if (mUseCompression && !mFirstBufferPass)
        {
            // If we are using compression then pairs of lod levels share the same index buffer
            // and are both created after the second one is processed.
            // If this manual lod level comes after the first of a pair but before the second,
            // then we haven't yet created the lod level for that first lod and a slot in the
            // vector is missing. Instead of skipping the slot, we decrement the lodIndex here
            // so that this manual level still goes at the end of the vector.
            // When we eventually do create that first lod level it will get inserted in the
            // correct spot, pushing this one into its correct spot.
            lodIndex--;
        }

        // placeholder dummy
        size_t submeshCount = getSubMeshCount();

        for (size_t i = 0; i < submeshCount; i++) {
            createSubMeshLodIndexData(i, lodIndex, nullptr, 0, 0);
        }
    }

    void LodOutputProvider::bakeLodLevel(LodData* data, int lodIndex)
    {
        if (mUseCompression)
        {
            if (mFirstBufferPass)
            {
                bakeFirstPass(data, lodIndex);
            }
            else
            {
                bakeSecondPass(data, lodIndex);
            }
            mFirstBufferPass = !mFirstBufferPass;
        }
        else
        {
            bakeUncompressed(data, lodIndex);
        }
    }

    inline void writeTriangle(LodData * data, size_t i)
    {
        if (data->mIndexBufferInfoList[data->mTriangleList[i].submeshID].indexSize == 2) {
            for (unsigned int m : data->mTriangleList[i].vertexID) {
                *(data->mIndexBufferInfoList[data->mTriangleList[i].submeshID].buf.pshort++) =
                    static_cast<uint16>(m);
            }
        } else {
            for (unsigned int m : data->mTriangleList[i].vertexID) {
                *(data->mIndexBufferInfoList[data->mTriangleList[i].submeshID].buf.pint++) =
                    static_cast<uint32>(m);
            }
        }
    }

    inline void writeLine(LodData * data, size_t i)
    {
        if (data->mIndexBufferInfoList[data->mLineList[i].submeshID].indexSize == 2) {
            for (unsigned int m : data->mLineList[i].vertexID) {
                *(data->mIndexBufferInfoList[data->mLineList[i].submeshID].buf.pshort++) =
                    static_cast<uint16>(m);
            }
        } else {
            for (unsigned int m : data->mLineList[i].vertexID) {
                *(data->mIndexBufferInfoList[data->mLineList[i].submeshID].buf.pint++) =
                    static_cast<uint32>(m);
            }
        }
    }

    void LodOutputProvider::bakeUncompressed(LodData* data, int lodIndex)
    {
        std::vector<HardwareIndexBufferPtr> lockedBuffers;

        size_t submeshCount = getSubMeshCount();

        // Create buffers.
        for (size_t i = 0; i < submeshCount; i++) {
            size_t indexCount = data->mIndexBufferInfoList[i].indexCount;
            HardwareIndexBufferPtr indexBuffer = createIndexBuffer(indexCount);

            createSubMeshLodIndexData(i, lodIndex, indexBuffer, 0, indexBuffer->getNumIndexes());

            data->mIndexBufferInfoList[i].buf.pshort = (unsigned short*) indexBuffer->lock(HardwareBuffer::HBL_DISCARD);

            lockedBuffers.push_back(indexBuffer);
        }

        // Fill buffers.
        size_t triangleCount = data->mTriangleList.size();
        for (size_t i = 0; i < triangleCount; i++) {
            if (!data->mTriangleList[i].isRemoved) {
                assert(data->mIndexBufferInfoList[data->mTriangleList[i].submeshID].indexCount != 0);
                writeTriangle(data, i);
            }
        }
        size_t lineCount = data->mLineList.size();
        for (size_t i = 0; i < lineCount; i++) {
            if (!data->mLineList[i].isRemoved) {
                assert(data->mIndexBufferInfoList[data->mLineList[i].submeshID].indexCount != 0);
                writeLine(data, i);
            }
        }

        // Close buffers.
        for (auto & buffer : lockedBuffers) {
            buffer->unlock();
        }
    }

    void LodOutputProvider::bakeFirstPass(LodData* data, int lodIndex)
    {
        size_t submeshCount = getSubMeshCount();
        assert(mTriangleCacheList.size() == data->mTriangleList.size());
        assert(mLineCacheList.size() == data->mLineList.size());
        mLastIndexBufferID = lodIndex;

        for (size_t i = 0; i < submeshCount; i++) {
            data->mIndexBufferInfoList[i].prevIndexCount = data->mIndexBufferInfoList[i].indexCount;
            data->mIndexBufferInfoList[i].prevOnlyIndexCount = 0;
        }

        size_t triangleCount = mTriangleCacheList.size();
        for (size_t i = 0; i < triangleCount; i++) {
            mTriangleCacheList[i].vertexChanged = false;
            if (!data->mTriangleList[i].isRemoved) {
                mTriangleCacheList[i].vertexID[0] = data->mTriangleList[i].vertexID[0];
                mTriangleCacheList[i].vertexID[1] = data->mTriangleList[i].vertexID[1];
                mTriangleCacheList[i].vertexID[2] = data->mTriangleList[i].vertexID[2];
            }
        }

        size_t lineCount = mLineCacheList.size();
        for (size_t i = 0; i < lineCount; i++) {
            mLineCacheList[i].vertexChanged = false;
            if (!data->mLineList[i].isRemoved) {
                mLineCacheList[i].vertexID[0] = data->mLineList[i].vertexID[0];
                mLineCacheList[i].vertexID[1] = data->mLineList[i].vertexID[1];
            }
        }
    }

    void LodOutputProvider::bakeSecondPass(LodData* data, int lodIndex)
    {
        std::vector<HardwareIndexBufferPtr> lockedBuffers;

        size_t submeshCount = getSubMeshCount();
        assert(mTriangleCacheList.size() == data->mTriangleList.size());
        assert(mLineCacheList.size() == data->mLineList.size());
        assert(lodIndex > mLastIndexBufferID); // Implementation limitation

        // Create buffers.
        for (size_t i = 0; i < submeshCount; i++) {
            assert(data->mIndexBufferInfoList[i].prevIndexCount >= data->mIndexBufferInfoList[i].indexCount);
            assert(data->mIndexBufferInfoList[i].prevIndexCount >= data->mIndexBufferInfoList[i].prevOnlyIndexCount);

            HardwareIndexBufferPtr indexBuffer = createIndexBuffer(data->mIndexBufferInfoList[i].indexCount + data->mIndexBufferInfoList[i].prevOnlyIndexCount);
            lockedBuffers.push_back(indexBuffer);

            data->mIndexBufferInfoList[i].buf.pshort = (unsigned short*) indexBuffer->lock(HardwareBuffer::HBL_DISCARD);

            // Set up one Lod
            size_t indexCount = std::max<size_t>(data->mIndexBufferInfoList[i].prevIndexCount, 3u);
            createSubMeshLodIndexData(i, mLastIndexBufferID, indexBuffer, 0, indexCount);

            // Set up the other Lod
            indexCount = std::max<size_t>(data->mIndexBufferInfoList[i].indexCount, 3u);
            createSubMeshLodIndexData(i, lodIndex, indexBuffer, indexBuffer->getNumIndexes() - indexCount, indexCount);
        }

        // Fill buffers.
        // Filling will be done in 3 parts.
        // 1. prevLod only indices.
        size_t triangleCount = mTriangleCacheList.size();
        for (size_t i = 0; i < triangleCount; i++) {
            if (mTriangleCacheList[i].vertexChanged) {
                assert(data->mIndexBufferInfoList[data->mTriangleList[i].submeshID].prevIndexCount != 0);
                assert(mTriangleCacheList[i].vertexID[0] != mTriangleCacheList[i].vertexID[1]);
                assert(mTriangleCacheList[i].vertexID[1] != mTriangleCacheList[i].vertexID[2]);
                assert(mTriangleCacheList[i].vertexID[2] != mTriangleCacheList[i].vertexID[0]);
                writeTriangle(data, i);
            }
        }
        size_t lineCount = mLineCacheList.size();
        for (size_t i = 0; i < lineCount; i++) {
            if (mLineCacheList[i].vertexChanged) {
                assert(data->mIndexBufferInfoList[data->mLineList[i].submeshID].prevIndexCount != 0);
                assert(mLineCacheList[i].vertexID[0] != mLineCacheList[i].vertexID[1]);
                writeLine(data, i);
            }
        }

        // 2. shared indices.
        for (size_t i = 0; i < triangleCount; i++) {
            if (!data->mTriangleList[i].isRemoved && !mTriangleCacheList[i].vertexChanged) {
                assert(mTriangleCacheList[i].vertexID[0] == data->mTriangleList[i].vertexID[0]);
                assert(mTriangleCacheList[i].vertexID[1] == data->mTriangleList[i].vertexID[1]);
                assert(mTriangleCacheList[i].vertexID[2] == data->mTriangleList[i].vertexID[2]);

                assert(data->mIndexBufferInfoList[data->mTriangleList[i].submeshID].indexCount != 0);
                assert(data->mIndexBufferInfoList[data->mTriangleList[i].submeshID].prevIndexCount != 0);
                writeTriangle(data, i);
            }
        }
        for (size_t i = 0; i < lineCount; i++) {
            if (!data->mLineList[i].isRemoved && !mLineCacheList[i].vertexChanged) {
                assert(mLineCacheList[i].vertexID[0] == data->mLineList[i].vertexID[0]);
                assert(mLineCacheList[i].vertexID[1] == data->mLineList[i].vertexID[1]);

                assert(data->mIndexBufferInfoList[data->mLineList[i].submeshID].indexCount != 0);
                assert(data->mIndexBufferInfoList[data->mLineList[i].submeshID].prevIndexCount != 0);
                writeLine(data, i);
            }
        }

        // 3. curLod indices only.
        for (size_t i = 0; i < triangleCount; i++) {
            if (!data->mTriangleList[i].isRemoved && mTriangleCacheList[i].vertexChanged) {
                assert(data->mIndexBufferInfoList[data->mTriangleList[i].submeshID].indexCount != 0);
                writeTriangle(data, i);
            }
        }
        for (size_t i = 0; i < lineCount; i++) {
            if (!data->mLineList[i].isRemoved && mLineCacheList[i].vertexChanged) {
                assert(data->mIndexBufferInfoList[data->mLineList[i].submeshID].indexCount != 0);
                writeLine(data, i);
            }
        }

        // Close buffers.
        for (auto & buffer : lockedBuffers)
        {
            buffer->unlock();
        }
    }

    void LodOutputProvider::triangleRemoved( LodData* data, LodData::Triangle* tri )
    {
        if (mUseCompression)
        {
            triangleChanged(data, tri);
        }
    }

    void LodOutputProvider::triangleChanged( LodData* data, LodData::Triangle* tri )
    {
        if (mUseCompression)
        {
            assert(!tri->isRemoved);
            TriangleCache& cache = mTriangleCacheList[LodData::getVectorIDFromPointer(data->mTriangleList, tri)];
            if(!cache.vertexChanged){
                cache.vertexChanged = true;
                data->mIndexBufferInfoList[tri->submeshID].prevOnlyIndexCount += 3;
            }
        }
    }

    void LodOutputProvider::lineRemoved( LodData* data, LodData::Line* line )
    {
        if (mUseCompression)
        {
            lineChanged(data, line);
        }
    }

    void LodOutputProvider::lineChanged( LodData* data, LodData::Line* line )
    {
        if (mUseCompression)
        {
            assert(!line->isRemoved);
            LineCache& cache = mLineCacheList[LodData::getVectorIDFromPointer(data->mLineList, line)];
            if(!cache.vertexChanged){
                cache.vertexChanged = true;
                data->mIndexBufferInfoList[line->submeshID].prevOnlyIndexCount += 2;
            }
        }
    }

    HardwareIndexBufferPtr LodOutputProvider::createIndexBuffer(size_t indexCount)
    {
        //If the index is empty we need to create a "dummy" triangle, just to keep the index buffer from being empty.
        //The main reason for this is that the OpenGL render system will crash with a segfault unless the index has some values.
        //This should hopefully be removed with future versions of Ogre. The most preferred solution would be to add the
        //ability for a submesh to be excluded from rendering for a given LOD (which isn't possible currently 2012-12-09).
        HardwareIndexBufferPtr buffer = createIndexBufferImpl(indexCount ? indexCount : 3);

        //Check if we should fill it with a "dummy" triangle.
        if (indexCount == 0)
        {
            void * addr = buffer->lock(HardwareBuffer::HBL_DISCARD);
            memset(addr, 0, 3 * buffer->getIndexSize());
            buffer->unlock();
        }

        return buffer;
    }

}
