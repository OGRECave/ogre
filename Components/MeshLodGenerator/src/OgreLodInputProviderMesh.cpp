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
    inline size_t getTriangleCount(RenderOperation::OperationType renderOp, size_t indexCount)
    {
        if(renderOp == RenderOperation::OT_TRIANGLE_LIST)
            return indexCount / 3;
        else if(renderOp == RenderOperation::OT_TRIANGLE_STRIP || renderOp == RenderOperation::OT_TRIANGLE_FAN)
            return indexCount >= 3 ? indexCount - 2 : 0;
        return 0;
    }
    
    LodInputProviderMesh::LodInputProviderMesh( MeshPtr mesh ) : mMesh(mesh)
    {

    }

    void LodInputProviderMesh::initData( LodData* data )
    {
        tuneContainerSize(data);
        initialize(data);
    }
    void LodInputProviderMesh::tuneContainerSize(LodData* data)
    {
        // Get Vertex count for container tuning.
        bool sharedVerticesAdded = false;
        size_t trianglesCount = 0;
        size_t vertexCount = 0;
        size_t vertexLookupSize = 0;
        size_t sharedVertexLookupSize = 0;
        size_t submeshCount = mMesh->getNumSubMeshes();
        for (size_t i = 0; i < submeshCount; i++) {
            const SubMesh* submesh = mMesh->getSubMesh(i);
            trianglesCount += getTriangleCount(submesh->operationType , submesh->indexData->indexCount);
            if (!submesh->useSharedVertices) {
                size_t count = submesh->vertexData->vertexCount;
                vertexLookupSize = std::max(vertexLookupSize, count);
                vertexCount += count;
            } else if (!sharedVerticesAdded) {
                sharedVerticesAdded = true;
                sharedVertexLookupSize = mMesh->sharedVertexData->vertexCount;
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

    void LodInputProviderMesh::initialize( LodData* data )
    {
#if OGRE_DEBUG_MODE
        data->mMeshName = mMesh->getName();
#endif
        data->mMeshBoundingSphereRadius = mMesh->getBoundingSphereRadius();
        size_t submeshCount = mMesh->getNumSubMeshes();
        for (size_t i = 0; i < submeshCount; ++i) {
            const SubMesh* submesh = mMesh->getSubMesh(i);
            VertexData* vertexData = (submesh->useSharedVertices ? mMesh->sharedVertexData : submesh->vertexData);
            addVertexData(data, vertexData, submesh->useSharedVertices);
            if(submesh->indexData->indexCount > 0)
                addIndexData(data, submesh->indexData, submesh->useSharedVertices, i, submesh->operationType);
        }

        // These were only needed for addIndexData() and addVertexData().
        mSharedVertexLookup.clear();
        mVertexLookup.clear();
    }
    void LodInputProviderMesh::addVertexData(LodData* data, VertexData* vertexData, bool useSharedVertexLookup)
    {
        if ((useSharedVertexLookup && !mSharedVertexLookup.empty())) { // We already loaded the shared vertex buffer.
            return;
        }
        OgreAssert(vertexData->vertexCount != 0, "");

        // Locate position element and the buffer to go with it.
        const VertexElement* elemPos = vertexData->vertexDeclaration->findElementBySemantic(VES_POSITION);

        // Only float supported.
        OgreAssert(elemPos->getSize() == 12, "");

        HardwareVertexBufferSharedPtr vbuf = vertexData->vertexBufferBinding->getBuffer(elemPos->getSource());

        // Lock the buffer for reading.
        unsigned char* vStart = static_cast<unsigned char*>(vbuf->lock(HardwareBuffer::HBL_READ_ONLY));
        unsigned char* vertex = vStart;
        size_t vSize = vbuf->getVertexSize();
        unsigned char* vEnd = vertex + vertexData->vertexCount * vSize;

        VertexLookupList& lookup = useSharedVertexLookup ? mSharedVertexLookup : mVertexLookup;
        lookup.clear();

        HardwareVertexBufferSharedPtr vNormalBuf;
        unsigned char* vNormal = NULL;
        const VertexElement* elemNormal = vertexData->vertexDeclaration->findElementBySemantic(VES_NORMAL);

        data->mUseVertexNormals &= (elemNormal != NULL);
        if(data->mUseVertexNormals){
            if(elemNormal->getSource() == elemPos->getSource()){
                // Don't open the same buffer twice. Just copy the pointer.
                vNormalBuf = vbuf;
                vNormal = vStart;
            }  else {
                vNormalBuf = vertexData->vertexBufferBinding->getBuffer(elemNormal->getSource());
                vNormal = static_cast<unsigned char*>(vNormalBuf->lock(HardwareBuffer::HBL_READ_ONLY));
            }
        }


        // Loop through all vertices and insert them to the Unordered Map.
        for (; vertex < vEnd; vertex += vSize) {
            float* pFloat;
            elemPos->baseVertexPointerToElement(vertex, &pFloat);
            data->mVertexList.push_back(LodData::Vertex());
            LodData::Vertex* v = &data->mVertexList.back();
            v->position.x = pFloat[0];
            v->position.y = pFloat[1];
            v->position.z = pFloat[2];
            std::pair<LodData::UniqueVertexSet::iterator, bool> ret;
            ret = data->mUniqueVertexSet.insert(v);
            if (!ret.second) {
                // Vertex position already exists.
                data->mVertexList.pop_back();
                v = *ret.first; // Point to the existing vertex.
                v->seam = true;
            } else {
#if OGRE_DEBUG_MODE
                // Needed for an assert, don't remove it.
                v->costHeapPosition = data->mCollapseCostHeap.end();
#endif
                v->seam = false;
            }
            lookup.push_back(v);

            if(data->mUseVertexNormals){
                elemNormal->baseVertexPointerToElement(vNormal, &pFloat);
                if (!ret.second) {
                    if(v->normal.x != pFloat[0]){
                        v->normal.x += pFloat[0];
                        v->normal.y += pFloat[1];
                        v->normal.z += pFloat[2];
                        if(v->normal.isZeroLength()){
                            v->normal = Vector3(1.0, 0.0, 0.0);
                        }
                        v->normal.normalise();
                    }
                } else {
                    v->normal.x = pFloat[0];
                    v->normal.y = pFloat[1];
                    v->normal.z = pFloat[2];
                    v->normal.normalise();
                }
                vNormal += vNormalBuf->getVertexSize();;
            }
        }
        vbuf->unlock();
        if(data->mUseVertexNormals && elemNormal->getSource() != elemPos->getSource()){
            vNormalBuf->unlock();
        }
    }
    void LodInputProviderMesh::addIndexData(LodData* data, IndexData* indexData, bool useSharedVertexLookup, ushort submeshID, RenderOperation::OperationType renderOp)
    {
        const HardwareIndexBufferSharedPtr& ibuf = indexData->indexBuffer;
        size_t isize = ibuf->getIndexSize();
        data->mIndexBufferInfoList[submeshID].indexSize = isize;
        data->mIndexBufferInfoList[submeshID].indexCount = indexData->indexCount;
        if (indexData->indexCount == 0) {
            // Locking a zero length buffer on Linux with nvidia cards fails.
            return;
        }
        VertexLookupList& lookup = useSharedVertexLookup ? mSharedVertexLookup : mVertexLookup;

        // Lock the buffer for reading.
        char* iStart = static_cast<char*>(ibuf->lock(HardwareBuffer::HBL_READ_ONLY));
        char* iEnd = iStart + ibuf->getSizeInBytes();
        if (isize == sizeof(unsigned short)) {
            addIndexDataImpl<unsigned short>(data, (unsigned short*) iStart, (unsigned short*) iEnd, lookup, submeshID, renderOp);
        } else {
            // Unsupported index size.
            OgreAssert(isize == sizeof(unsigned int), "");
            addIndexDataImpl<unsigned int>(data, (unsigned int*) iStart, (unsigned int*) iEnd, lookup, submeshID, renderOp);
        }
        ibuf->unlock();
    }   

}
