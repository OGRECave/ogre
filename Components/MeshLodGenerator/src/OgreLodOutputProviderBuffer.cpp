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
#include "OgreDefaultHardwareBufferManager.h"

namespace Ogre
{
    LodOutputBuffer& LodOutputProviderBuffer::getBuffer()
    {
        return mBuffer;
    }

    void LodOutputProviderBuffer::prepare( LodData* data )
    {

        mBuffer.submesh.resize(data->mIndexBufferInfoList.size());
    }

    void LodOutputProviderBuffer::bakeManualLodLevel( LodData* data, String& manualMeshName, int lodIndex )
    {
        // placeholder dummy
        size_t submeshCount = getSubMeshCount();
        for (size_t i = 0; i < submeshCount; i++) {
            createSubMeshLodIndexData(i, lodIndex, nullptr, 0, 0);
        }
    }

    void LodOutputProviderBuffer::bakeLodLevel(LodData* data, int lodIndex)
    {
        std::vector<HardwareIndexBufferPtr> lockedBuffers;

        size_t submeshCount = getSubMeshCount();

        // Create buffers.
        for (size_t i = 0; i < submeshCount; i++) {
            size_t indexCount = data->mIndexBufferInfoList[i].indexCount;
            HardwareIndexBufferPtr indexBuffer = createIndexBuffer(indexCount);

            createSubMeshLodIndexData(i, lodIndex, indexBuffer, 0, indexBuffer->getNumIndexes());

            data->mIndexBufferInfoList[i].buf.pshort = (unsigned short*) indexBuffer->lock(HardwareBuffer::HBL_NORMAL);

            lockedBuffers.push_back(indexBuffer);
        }

        // Fill buffers.
        size_t triangleCount = data->mTriangleList.size();
        for (size_t i = 0; i < triangleCount; i++) {
            if (!data->mTriangleList[i].isRemoved) {
                assert(data->mIndexBufferInfoList[data->mTriangleList[i].submeshID].indexCount != 0);
                if (data->mIndexBufferInfoList[data->mTriangleList[i].submeshID].indexSize == 2) {
                    for (unsigned int m : data->mTriangleList[i].vertexID) {
                        *(data->mIndexBufferInfoList[data->mTriangleList[i].submeshID].buf.pshort++) =
                            static_cast<unsigned short>(m);
                    }
                } else {
                    for (unsigned int m : data->mTriangleList[i].vertexID) {
                        *(data->mIndexBufferInfoList[data->mTriangleList[i].submeshID].buf.pint++) =
                            static_cast<unsigned int>(m);
                    }
                }
            }
        }

        // Close buffers.
        for (auto & buffer : lockedBuffers) {
            buffer->unlock();
        }
    }

void LodOutputProviderBuffer::inject()
{
    ushort submeshCount = Math::uint16Cast(mBuffer.submesh.size());
    OgreAssert(mMesh->getNumSubMeshes() == submeshCount, "");
    mMesh->removeLodLevels();
    for (unsigned short i = 0; i < submeshCount; i++) {
        SubMesh::LODFaceList& lods = mMesh->getSubMesh(i)->mLodFaceList;
        typedef std::vector<LodIndexBuffer> GenBuffers;
        GenBuffers& buffers = mBuffer.submesh[i].genIndexBuffers;

        size_t buffCount = buffers.size();
        for (size_t n=0; n<buffCount;n++) {
            LodIndexBuffer& buff = buffers[n];
            size_t indexCount = (buff.indexBufferSize ? buff.indexBufferSize : buff.indexCount);
            OgreAssert((int)buff.indexCount >= 0, "");
            lods.push_back(OGRE_NEW IndexData());
            lods.back()->indexStart = buff.indexStart;
            lods.back()->indexCount = buff.indexCount;
            if(indexCount != 0) {
                if(n > 0 && buffers[n-1].indexBuffer == buff.indexBuffer){
                    lods.back()->indexBuffer = (*(++lods.rbegin()))->indexBuffer;
                } else {
                    lods.back()->indexBuffer = mMesh->getHardwareBufferManager()->createIndexBuffer(
                        buff.indexSize == 2 ?
                        HardwareIndexBuffer::IT_16BIT : HardwareIndexBuffer::IT_32BIT,
                        indexCount, mMesh->getIndexBufferUsage(), mMesh->isIndexBufferShadowed());
                    lods.back()->indexBuffer->copyData(*buff.indexBuffer);
                }
            }
        }
    }
}
    
    size_t LodOutputProviderBuffer::getSubMeshCount()
    {
        return mBuffer.submesh.size();
    }

    HardwareIndexBufferPtr LodOutputProviderBuffer::createIndexBufferImpl(size_t indexCount)
    {
        auto indexType = indexCount - 1 <= std::numeric_limits<uint16>::max() ? HardwareIndexBuffer::IT_16BIT : HardwareIndexBuffer::IT_32BIT;
        DefaultHardwareBufferManagerBase bfrMgr;
        return bfrMgr.createIndexBuffer(indexType, indexCount, HBU_CPU_ONLY);
    }

    void LodOutputProviderBuffer::createSubMeshLodIndexData(size_t subMeshIndex, int lodIndex, const HardwareIndexBufferPtr & indexBuffer, size_t indexStart, size_t indexCount)
    {
        std::vector<LodIndexBuffer>& lods = mBuffer.submesh[subMeshIndex].genIndexBuffers;
        lods.reserve(lods.size() + 1);
        LodIndexBuffer * curLod;

        // I don't know what the negative lodIndex should mean but this logic
        // was present in the original code.
        if (lodIndex < 0)
        {
            lods.push_back(LodIndexBuffer());
            curLod = &lods.back();
        }
        else
        {
            curLod = &*lods.insert(lods.begin() + lodIndex, LodIndexBuffer());
        }

        curLod->indexStart = indexStart;
        curLod->indexCount = indexCount;
        curLod->indexBuffer = indexBuffer;

        if (indexBuffer)
        {
            curLod->indexSize = indexBuffer->getIndexSize();
            curLod->indexBufferSize = indexBuffer->getNumIndexes();
        }
        else
        {
            curLod->indexSize = 2;
            curLod->indexBufferSize = 0;
        }
    }
}
