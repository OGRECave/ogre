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

#include "OgreMeshLodPrecompiledHeaders.h"

namespace Ogre
{
    LodOutputProviderCompressedMesh::LodOutputProviderCompressedMesh( MeshPtr mesh ) :
        mFirstBufferPass(0), mMesh(mesh), mLastIndexBufferID(0)
    {
        fallback = new LodOutputProviderMesh(mesh);
    }

    LodOutputProviderCompressedMesh::LodOutputProviderCompressedMesh()
    {

    }


    LodOutputProviderCompressedMesh::~LodOutputProviderCompressedMesh()
    {
        delete fallback;
    }

    void LodOutputProviderCompressedMesh::prepare( LodData* data )
    {
        mFirstBufferPass = true;
        mTriangleCacheList.resize(data->mTriangleList.size());
        fallback->prepare(data);
    }

    void LodOutputProviderCompressedMesh::finalize( LodData* data )
    {
        if(!mFirstBufferPass){
            // Uneven number of Lod levels. We need to bake the last one separately.
            fallback->bakeLodLevel(data, mLastIndexBufferID);
        }
        fallback->finalize(data);
    }

    void LodOutputProviderCompressedMesh::bakeManualLodLevel( LodData* data, String& manualMeshName, int lodIndex )
    {
        if(!mFirstBufferPass){
            lodIndex--;
        }
        fallback->bakeManualLodLevel(data, manualMeshName, lodIndex);
    }

    void LodOutputProviderCompressedMesh::inject()
    {
        fallback->inject();
    }

    void LodOutputProviderCompressedMesh::bakeLodLevel(LodData* data, int lodIndex)
    {
        if(mFirstBufferPass){
            bakeFirstPass(data, lodIndex);
        } else {
            bakeSecondPass(data, lodIndex);
        }
        mFirstBufferPass = !mFirstBufferPass;
    }
    void LodOutputProviderCompressedMesh::bakeFirstPass(LodData* data, int lodIndex) {
        size_t submeshCount = mMesh->getNumSubMeshes();
        assert(mTriangleCacheList.size() == data->mTriangleList.size());
        mLastIndexBufferID =  lodIndex;

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
    }
    void LodOutputProviderCompressedMesh::bakeSecondPass(LodData* data, int lodIndex) {
        size_t submeshCount = mMesh->getNumSubMeshes();
        assert(mTriangleCacheList.size() == data->mTriangleList.size());
        assert(lodIndex > mLastIndexBufferID); // Implementation limitation
        // Create buffers.
        for (size_t i = 0; i < submeshCount; i++) {
            SubMesh::LODFaceList& lods = mMesh->getSubMesh(i)->mLodFaceList;
            lods.reserve(lods.size() + 2);
            size_t indexCount = data->mIndexBufferInfoList[i].indexCount + data->mIndexBufferInfoList[i].prevOnlyIndexCount;
            assert(data->mIndexBufferInfoList[i].prevIndexCount >= data->mIndexBufferInfoList[i].indexCount);
            assert(data->mIndexBufferInfoList[i].prevIndexCount >= data->mIndexBufferInfoList[i].prevOnlyIndexCount);
            
            IndexData* prevLod = *lods.insert(lods.begin() + mLastIndexBufferID, OGRE_NEW IndexData());
            prevLod->indexStart = 0;

            //If the index is empty we need to create a "dummy" triangle, just to keep the index buffer from being empty.
            //The main reason for this is that the OpenGL render system will crash with a segfault unless the index has some values.
            //This should hopefully be removed with future versions of Ogre. The most preferred solution would be to add the
            //ability for a submesh to be excluded from rendering for a given LOD (which isn't possible currently 2012-12-09).
            indexCount = std::max<size_t>(indexCount, 3);
            prevLod->indexCount = std::max<size_t>(data->mIndexBufferInfoList[i].prevIndexCount, 3u);

            prevLod->indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(
                data->mIndexBufferInfoList[i].indexSize == 2 ?
                HardwareIndexBuffer::IT_16BIT : HardwareIndexBuffer::IT_32BIT,
                indexCount, mMesh->getIndexBufferUsage(), mMesh->isIndexBufferShadowed());

            data->mIndexBufferInfoList[i].buf.pshort =
                static_cast<unsigned short*>(prevLod->indexBuffer->lock(0, prevLod->indexBuffer->getSizeInBytes(),
                HardwareBuffer::HBL_DISCARD));

            //Check if we should fill it with a "dummy" triangle.
            if (indexCount == 3) {
                memset(data->mIndexBufferInfoList[i].buf.pshort, 0, 3 * data->mIndexBufferInfoList[i].indexSize);
            }

            // Set up the other Lod
            IndexData* curLod = *lods.insert(lods.begin() + lodIndex, OGRE_NEW IndexData());
            curLod->indexStart = indexCount - data->mIndexBufferInfoList[i].indexCount;
            curLod->indexCount = data->mIndexBufferInfoList[i].indexCount;
            if(curLod->indexCount == 0){
                curLod->indexStart-=3;
                curLod->indexCount=3;
            }
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
                if (data->mIndexBufferInfoList[data->mTriangleList[i].submeshID].indexSize == 2) {
                    for (int m = 0; m < 3; m++) {
                        *(data->mIndexBufferInfoList[data->mTriangleList[i].submeshID].buf.pshort++) =
                            static_cast<unsigned short>(mTriangleCacheList[i].vertexID[m]);
                    }
                } else {
                    for (int m = 0; m < 3; m++) {
                        *(data->mIndexBufferInfoList[data->mTriangleList[i].submeshID].buf.pint++) =
                            static_cast<unsigned int>(mTriangleCacheList[i].vertexID[m]);
                    }
                }
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
                if (data->mIndexBufferInfoList[data->mTriangleList[i].submeshID].indexSize == 2) {
                    for (int m = 0; m < 3; m++) {
                        *(data->mIndexBufferInfoList[data->mTriangleList[i].submeshID].buf.pshort++) =
                            static_cast<unsigned short>(data->mTriangleList[i].vertexID[m]);
                    }
                } else {
                    for (int m = 0; m < 3; m++) {
                        *(data->mIndexBufferInfoList[data->mTriangleList[i].submeshID].buf.pint++) =
                            static_cast<unsigned int>(data->mTriangleList[i].vertexID[m]);
                    }
                }
            }
        }

        // 3. curLod indices only.
        for (size_t i = 0; i < triangleCount; i++) {
            if (!data->mTriangleList[i].isRemoved && mTriangleCacheList[i].vertexChanged) {
                assert(data->mIndexBufferInfoList[data->mTriangleList[i].submeshID].indexCount != 0);
                if (data->mIndexBufferInfoList[data->mTriangleList[i].submeshID].indexSize == 2) {
                    for (int m = 0; m < 3; m++) {
                        *(data->mIndexBufferInfoList[data->mTriangleList[i].submeshID].buf.pshort++) =
                            static_cast<unsigned short>(data->mTriangleList[i].vertexID[m]);
                    }
                } else {
                    for (int m = 0; m < 3; m++) {
                        *(data->mIndexBufferInfoList[data->mTriangleList[i].submeshID].buf.pint++) =
                            static_cast<unsigned int>(data->mTriangleList[i].vertexID[m]);
                    }
                }
            }
        }

        // Close buffers.
        for (unsigned short i = 0; i < submeshCount; i++) {
            SubMesh::LODFaceList& lods = mMesh->getSubMesh(i)->mLodFaceList;
            IndexData* prevLod = lods[mLastIndexBufferID];
            IndexData* curLod = lods[lodIndex];
            prevLod->indexBuffer->unlock();
            curLod->indexBuffer = prevLod->indexBuffer;
        }
    }

    void LodOutputProviderCompressedMesh::triangleRemoved( LodData* data, LodData::Triangle* tri )
    {
        triangleChanged(data, tri);
    }

    void LodOutputProviderCompressedMesh::triangleChanged( LodData* data, LodData::Triangle* tri )
    {
        assert(!tri->isRemoved);
        TriangleCache& cache = mTriangleCacheList[LodData::getVectorIDFromPointer(data->mTriangleList, tri)];
        if(!cache.vertexChanged){
            cache.vertexChanged = true;
            data->mIndexBufferInfoList[tri->submeshID].prevOnlyIndexCount += 3;
        }
    }

}
