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
        ushort submeshCount = Math::uint16Cast(mBuffer.submesh.size());
        LodIndexBuffer buffer;
        buffer.indexSize = 2;
        buffer.indexCount = 0;
        buffer.indexStart = 0;
        buffer.indexBufferSize = 0;
        if(lodIndex < 0) {
            for (unsigned short i = 0; i < submeshCount; i++) {
                mBuffer.submesh[i].genIndexBuffers.push_back(buffer);
            }
        } else {
            for (unsigned short i = 0; i < submeshCount; i++) {
                mBuffer.submesh[i].genIndexBuffers.insert(mBuffer.submesh[i].genIndexBuffers.begin() + lodIndex, buffer);
            }
        }
    }

    void LodOutputProviderBuffer::bakeLodLevel(LodData* data, int lodIndex)
    {
        ushort submeshCount = Math::uint16Cast(mBuffer.submesh.size());

        // Create buffers.
        for (unsigned short i = 0; i < submeshCount; i++) {
            std::vector<LodIndexBuffer>& lods = mBuffer.submesh[i].genIndexBuffers;
            size_t indexCount = data->mIndexBufferInfoList[i].indexCount;
            lods.reserve(lods.size() + 1);
            LodIndexBuffer& curLod = *lods.insert(lods.begin() + lodIndex, LodIndexBuffer());
            if (indexCount == 0) {
                curLod.indexCount = 3;
            } else {
                curLod.indexCount = indexCount;
            }
            curLod.indexStart = 0;
            curLod.indexSize = data->mIndexBufferInfoList[i].indexSize;
            curLod.indexBufferSize = 0; // It means same as index count
            curLod.indexBuffer = Ogre::SharedPtr<unsigned char>(new unsigned char[curLod.indexCount * curLod.indexSize]);
            // buf is an union, so pint=pshort
            data->mIndexBufferInfoList[i].buf.pshort = (unsigned short*) curLod.indexBuffer.get();

            if (indexCount == 0) {
                memset(data->mIndexBufferInfoList[i].buf.pshort, 0, 3 * data->mIndexBufferInfoList[i].indexSize);
            }
        }

        // Fill buffers.
        size_t triangleCount = data->mTriangleList.size();
        for (size_t i = 0; i < triangleCount; i++) {
            if (!data->mTriangleList[i].isRemoved) {
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
                    size_t sizeInBytes = lods.back()->indexBuffer->getSizeInBytes();
                    void* pOutBuff = lods.back()->indexBuffer->lock(0, sizeInBytes, HardwareBuffer::HBL_DISCARD);
                    memcpy(pOutBuff, buff.indexBuffer.get(), sizeInBytes);
                    lods.back()->indexBuffer->unlock();
                }
            }
        }
    }
}
}
