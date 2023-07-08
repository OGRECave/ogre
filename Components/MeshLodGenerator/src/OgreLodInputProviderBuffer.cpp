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
    LodInputProviderBuffer::LodInputProviderBuffer( MeshPtr mesh )
    {
        mBuffer.fillBuffer(mesh);
    }

    void LodInputProviderBuffer::addVertexData(LodData* data, size_t subMeshIndex)
    {
        bool useSharedVertices = getSubMeshUseSharedVertices(subMeshIndex);

        if (useSharedVertices && !mSharedVertexLookup.empty()) {
            return; // We already loaded the shared vertex buffer.
        }

        VertexLookupList& lookup = useSharedVertices ? mSharedVertexLookup : mVertexLookup;
        lookup.clear();

        LodVertexBuffer& vertexBuffer = useSharedVertices ? mBuffer.sharedVertexBuffer : mBuffer.submesh[subMeshIndex].vertexBuffer;
        const Vector3f* pNormalOut = (Vector3f *)vertexBuffer.vertexNormalBuffer->lock(HardwareBuffer::HBL_READ_ONLY);
        data->mUseVertexNormals = data->mUseVertexNormals && (pNormalOut != NULL);

        if(!data->mUseVertexNormals)
        {
            static Vector3f zeroNormal(0, 0, 0);
            pNormalOut = &zeroNormal;
        }

        // Loop through all vertices and insert them to the Unordered Map.
        const Vector3f* pOut = (Vector3f *)vertexBuffer.vertexBuffer->lock(HardwareBuffer::HBL_READ_ONLY);
        const Vector3f* pEnd = pOut + vertexBuffer.vertexCount;
        for (; pOut < pEnd; pOut++) {
            data->mVertexList.push_back({Vector3(*pOut), Vector3(*pNormalOut)});
            LodData::Vertex* v = &data->mVertexList.back();
            std::pair<LodData::UniqueVertexSet::iterator, bool> ret;
            ret = data->mUniqueVertexSet.insert(v);
            if (!ret.second) {
                // Vertex position already exists.
                data->mVertexList.pop_back();
                v = *ret.first; // Point to the existing vertex.
                v->seam = true;
                if(data->mUseVertexNormals){
                    if(v->normal.x != (*pNormalOut)[0]){
                        v->normal += Vector3(*pNormalOut);
                        if(v->normal.isZeroLength()){
                            v->normal = Vector3(1.0, 0.0, 0.0);
                        }
                        v->normal.normalise();
                    }
                    pNormalOut++;
                }
            } else {
#if OGRE_DEBUG_MODE
                v->costHeapPosition = data->mCollapseCostHeap.end();
#endif
                v->seam = false;
                if(data->mUseVertexNormals){
                    v->normal.normalise();
                    pNormalOut++;
                }
            }
            lookup.push_back(v);
        }
        vertexBuffer.vertexBuffer->unlock();
        vertexBuffer.vertexNormalBuffer->unlock();
    }

    const IndexData* LodInputProviderBuffer::getSubMeshIndexData(size_t subMeshIndex) const
    {
        return &mBuffer.submesh[subMeshIndex].indexBuffer;
    }

    const String & LodInputProviderBuffer::getMeshName()
    {
        return mBuffer.meshName;
    }
    size_t LodInputProviderBuffer::getMeshSharedVertexCount()
    {
        return mBuffer.sharedVertexBuffer.vertexCount;
    }
    float LodInputProviderBuffer::getMeshBoundingSphereRadius()
    {
        return mBuffer.boundingSphereRadius;
    }
    size_t LodInputProviderBuffer::getSubMeshCount()
    {
        return mBuffer.submesh.size();
    }
    bool LodInputProviderBuffer::getSubMeshUseSharedVertices(size_t subMeshIndex)
    {
        return mBuffer.submesh[subMeshIndex].useSharedVertexBuffer;
    }
    size_t LodInputProviderBuffer::getSubMeshOwnVertexCount(size_t subMeshIndex)
    {
        return mBuffer.submesh[subMeshIndex].vertexBuffer.vertexCount;
    }
    size_t LodInputProviderBuffer::getSubMeshIndexCount(size_t subMeshIndex)
    {
        return mBuffer.submesh[subMeshIndex].indexBuffer.indexCount;
    }
    RenderOperation::OperationType LodInputProviderBuffer::getSubMeshRenderOp(size_t subMeshIndex)
    {
        return mBuffer.submesh[subMeshIndex].operationType;
    }
}
