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
        LodOutputProvider::prepare(data);

        size_t submeshCount = mMesh->getNumSubMeshes();

        // Create buffers.
        for (size_t i = 0; i < submeshCount; i++) {
            SubMesh::LODFaceList& lods = mMesh->getSubMesh(i)->mLodFaceList;
            lods.resize(1);
        }
    }

    size_t LodOutputProviderMesh::getSubMeshCount()
    {
        return mMesh->getNumSubMeshes();
    }

    HardwareIndexBufferPtr LodOutputProviderMesh::createIndexBufferImpl(size_t indexCount)
    {
        auto indexType = indexCount - 1 <= std::numeric_limits<uint16>::max() ? HardwareIndexBuffer::IT_16BIT : HardwareIndexBuffer::IT_32BIT;

        return mMesh->getHardwareBufferManager()->createIndexBuffer(indexType, indexCount, mMesh->getIndexBufferUsage(), mMesh->isIndexBufferShadowed());
    }

    void LodOutputProviderMesh::createSubMeshLodIndexData(size_t subMeshIndex, int lodIndex, const HardwareIndexBufferPtr & indexBuffer, size_t indexStart, size_t indexCount)
    {
        IndexData* curLod = OGRE_NEW IndexData();
        SubMesh::LODFaceList& lods = mMesh->getSubMesh(subMeshIndex)->mLodFaceList;
        lods.insert(lods.begin() + lodIndex, curLod);

        curLod->indexStart = 0;
        curLod->indexCount = indexCount;
        curLod->indexBuffer = indexBuffer;
    }
}
