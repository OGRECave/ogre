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

    void LodInputProviderBuffer::initData( LodData* data )
    {
        tuneContainerSize(data);
        initialize(data);
    }

    void LodInputProviderBuffer::tuneContainerSize(LodData* data)
    {
        // Get Vertex count for container tuning.
        bool sharedVerticesAdded = false;
        size_t trianglesCount = 0;
        size_t vertexCount = 0;
        size_t vertexLookupSize = 0;
        size_t sharedVertexLookupSize = 0;
        ushort submeshCount = Math::uint16Cast(mBuffer.submesh.size());
        for (unsigned short i = 0; i < submeshCount; i++) {
            const LodInputBuffer::Submesh& submesh = mBuffer.submesh[i];
            trianglesCount += submesh.indexBuffer.indexCount/3; //assume mBuffer provide triangle list only
            if (!submesh.useSharedVertexBuffer) {
                size_t count = submesh.vertexBuffer.vertexCount;
                vertexLookupSize = std::max<size_t>(vertexLookupSize, count);
                vertexCount += count;
            } else if (!sharedVerticesAdded) {
                sharedVerticesAdded = true;
                sharedVertexLookupSize = mBuffer.sharedVertexBuffer.vertexCount;
                vertexCount += sharedVertexLookupSize;
            }
        }

        // Tune containers:
        data->mUniqueVertexSet.rehash(4 * vertexCount); // less then 0.25 item/bucket for low collision rate

        data->mTriangleList.reserve(trianglesCount);

        data->mVertexList.reserve(vertexCount);
        mSharedVertexLookup.reserve(sharedVertexLookupSize);
        mVertexLookup.reserve(vertexLookupSize);
        data->mIndexBufferInfoList.resize(submeshCount);
    }

    void LodInputProviderBuffer::initialize( LodData* data )
    {
#if OGRE_DEBUG_MODE
        data->mMeshName = mBuffer.meshName;
#endif
        data->mMeshBoundingSphereRadius = mBuffer.boundingSphereRadius;
        ushort submeshCount = Math::uint16Cast(mBuffer.submesh.size());
        for (unsigned short i = 0; i < submeshCount; ++i) {
            LodInputBuffer::Submesh& submesh = mBuffer.submesh[i];
            LodVertexBuffer& vertexBuffer =
                (submesh.useSharedVertexBuffer ? mBuffer.sharedVertexBuffer : submesh.vertexBuffer);
            addVertexData(data, vertexBuffer, submesh.useSharedVertexBuffer);
            addIndexData(data, submesh.indexBuffer, submesh.useSharedVertexBuffer, i);
        }

        // These were only needed for addIndexData() and addVertexData().
        mSharedVertexLookup.clear();
        mVertexLookup.clear();
    }
    void LodInputProviderBuffer::addVertexData(LodData* data, LodVertexBuffer& vertexBuffer, bool useSharedVertexLookup)
    {
        if (useSharedVertexLookup && !mSharedVertexLookup.empty()) {
            return; // We already loaded the shared vertex buffer.
        }

        VertexLookupList& lookup = useSharedVertexLookup ? mSharedVertexLookup : mVertexLookup;
        lookup.clear();

        const Vector3* pNormalOut = vertexBuffer.vertexNormalBuffer.get();
        data->mUseVertexNormals = data->mUseVertexNormals && (pNormalOut != NULL);

        if(!data->mUseVertexNormals)
            pNormalOut = &Vector3::ZERO;

        // Loop through all vertices and insert them to the Unordered Map.
        Vector3* pOut = vertexBuffer.vertexBuffer.get();
        Vector3* pEnd = pOut + vertexBuffer.vertexCount;
        for (; pOut < pEnd; pOut++) {
            data->mVertexList.push_back({*pOut, *pNormalOut});
            LodData::Vertex* v = &data->mVertexList.back();
            std::pair<LodData::UniqueVertexSet::iterator, bool> ret;
            ret = data->mUniqueVertexSet.insert(v);
            if (!ret.second) {
                // Vertex position already exists.
                data->mVertexList.pop_back();
                v = *ret.first; // Point to the existing vertex.
                v->seam = true;
                if(data->mUseVertexNormals){
                    if(v->normal.x != (*pNormalOut).x){
                        v->normal += *pNormalOut;
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
    }

    void LodInputProviderBuffer::addIndexData(LodData* data, LodIndexBuffer& indexBuffer, bool useSharedVertexLookup, unsigned short submeshID)
    {
        size_t isize = indexBuffer.indexSize;
        data->mIndexBufferInfoList[submeshID].indexSize = isize;
        data->mIndexBufferInfoList[submeshID].indexCount = indexBuffer.indexCount;
        VertexLookupList& lookup = useSharedVertexLookup ? mSharedVertexLookup : mVertexLookup;

        // Lock the buffer for reading.
        unsigned char* iStart = indexBuffer.indexBuffer.get();
        if(!iStart) {
            return;
        }
        unsigned char* iEnd = iStart + indexBuffer.indexCount * isize;
        if (isize == sizeof(unsigned short)) {
            addIndexDataImpl<unsigned short>(data, (unsigned short*) iStart, (unsigned short*) iEnd, lookup, submeshID);
        } else {
            // Unsupported index size.
            OgreAssert(isize == sizeof(unsigned int), "");
            addIndexDataImpl<unsigned int>(data, (unsigned int*) iStart, (unsigned int*) iEnd, lookup, submeshID);
        }
    }

}
