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

    void LodOutputProviderMesh::prepare( LodData* data )
    {
        size_t submeshCount = mMesh->getNumSubMeshes();

        // Create buffers.
        for (size_t i = 0; i < submeshCount; i++) {
            SubMesh::LODFaceList& lods = mMesh->getSubMesh(i)->mLodFaceList;
            lods.resize(1);
        }
    }

    void LodOutputProviderMesh::bakeManualLodLevel( LodData* data, String& manualMeshName, int lodIndex)
    {
        // placeholder dummy
        size_t submeshCount = mMesh->getNumSubMeshes();
        for (size_t i = 0; i < submeshCount; i++) {
            SubMesh::LODFaceList& lods = mMesh->getSubMesh(i)->mLodFaceList;
            lods.insert(lods.begin() + lodIndex, OGRE_NEW IndexData());
        }
    }

    void LodOutputProviderMesh::bakeLodLevel(LodData* data, int lodIndex)
    {
        size_t submeshCount = mMesh->getNumSubMeshes();

        // Create buffers.
        for (size_t i = 0; i < submeshCount; i++) {
            SubMesh::LODFaceList& lods = mMesh->getSubMesh(i)->mLodFaceList;
            size_t indexCount = data->mIndexBufferInfoList[i].indexCount;
            IndexData* curLod = OGRE_NEW IndexData();
            curLod->indexStart = 0;
            lods.insert(lods.begin() + lodIndex, curLod);
            if (indexCount == 0) {
                //If the index is empty we need to create a "dummy" triangle, just to keep the index buffer from being empty.
                //The main reason for this is that the OpenGL render system will crash with a segfault unless the index has some values.
                //This should hopefully be removed with future versions of Ogre. The most preferred solution would be to add the
                //ability for a submesh to be excluded from rendering for a given LOD (which isn't possible currently 2012-12-09).
                curLod->indexCount = 3;
            } else {
                curLod->indexCount = indexCount;
            }

            curLod->indexBuffer = mMesh->getHardwareBufferManager()->createIndexBuffer(
                data->mIndexBufferInfoList[i].indexSize == 2 ?
                HardwareIndexBuffer::IT_16BIT : HardwareIndexBuffer::IT_32BIT,
                curLod->indexCount, mMesh->getIndexBufferUsage(), mMesh->isIndexBufferShadowed());

            data->mIndexBufferInfoList[i].buf.pshort =
                static_cast<unsigned short*>(curLod->indexBuffer->lock(0, curLod->indexBuffer->getSizeInBytes(),
                HardwareBuffer::HBL_DISCARD));

            //Check if we should fill it with a "dummy" triangle.
            if (indexCount == 0) {
                memset(data->mIndexBufferInfoList[i].buf.pshort, 0, 3 * data->mIndexBufferInfoList[i].indexSize);
            }
        }

        // Fill buffers.
        size_t triangleCount = data->mTriangleList.size();
        for (size_t i = 0; i < triangleCount; i++) {
            if (!data->mTriangleList[i].isRemoved) {
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
            lods[lodIndex]->indexBuffer->unlock();
        }
    }

}
