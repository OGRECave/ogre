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

#include "OgreLodInputProviderMesh.h"
#include "OgreLodData.h"
#include "OgreMesh.h"
#include "OgreSubMesh.h"

namespace Ogre
{
    LodInputProviderMesh::LodInputProviderMesh( v1::MeshPtr mesh ) : mMesh(mesh)
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
        size_t vertexCount = 0;
        size_t vertexLookupSize = 0;
        size_t sharedVertexLookupSize = 0;
        unsigned short submeshCount = mMesh->getNumSubMeshes();
        for (unsigned short i = 0; i < submeshCount; i++)
        {
            const v1::SubMesh* submesh = mMesh->getSubMesh(i);
            if (!submesh->useSharedVertices)
            {
                size_t count = submesh->vertexData[VpNormal]->vertexCount;
                vertexLookupSize = std::max<size_t>(vertexLookupSize, count);
                vertexCount += count;
            }
            else if (!sharedVerticesAdded)
            {
                sharedVerticesAdded = true;
                sharedVertexLookupSize = mMesh->sharedVertexData[VpNormal]->vertexCount;
                vertexCount += sharedVertexLookupSize;
            }
        }

        // Tune containers:
        data->mUniqueVertexSet.rehash(4 * vertexCount); // less then 0.25 item/bucket for low collision rate

        // There are less triangles then 2 * vertexCount. Except if there are bunch of triangles,
        // where all vertices have the same position, but that would not make much sense.
        data->mTriangleList.reserve(2 * vertexCount);

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
        unsigned short submeshCount = mMesh->getNumSubMeshes();
        for (unsigned short i = 0; i < submeshCount; ++i)
        {
            const v1::SubMesh* submesh = mMesh->getSubMesh(i);
            v1::VertexData* vertexData = (submesh->useSharedVertices ? mMesh->sharedVertexData[VpNormal] :
                                          submesh->vertexData[VpNormal]);
            addVertexData(data, vertexData, submesh->useSharedVertices);
            if(submesh->indexData[VpNormal]->indexCount > 0)
                addIndexData(data, submesh->indexData[VpNormal], submesh->useSharedVertices, i);
        }

        // These were only needed for addIndexData() and addVertexData().
        mSharedVertexLookup.clear();
        mVertexLookup.clear();
    }
    void LodInputProviderMesh::addVertexData(LodData* data, v1::VertexData* vertexData, bool useSharedVertexLookup)
    {
        if ((useSharedVertexLookup && !mSharedVertexLookup.empty()))   // We already loaded the shared vertex buffer.
        {
            return;
        }
        OgreAssert(vertexData->vertexCount != 0, "");

        // Locate position element and the buffer to go with it.
        const v1::VertexElement* elemPos = vertexData->vertexDeclaration->
                                           findElementBySemantic(VES_POSITION);

        // Only float supported.
        OgreAssert(elemPos->getSize() == 12, "");

        v1::HardwareVertexBufferSharedPtr vbuf = vertexData->vertexBufferBinding->getBuffer(
                    elemPos->getSource() );

        // Lock the buffer for reading.
        unsigned char* vStart = static_cast<unsigned char*>(vbuf->lock(v1::HardwareBuffer::HBL_READ_ONLY));
        unsigned char* vertex = vStart;
        size_t vSize = vbuf->getVertexSize();
        unsigned char* vEnd = vertex + vertexData->vertexCount * vSize;

        VertexLookupList& lookup = useSharedVertexLookup ? mSharedVertexLookup : mVertexLookup;
        lookup.clear();

        v1::HardwareVertexBufferSharedPtr vNormalBuf;
        unsigned char* vNormal = NULL;
        size_t vNormSize = 0;
        const v1::VertexElement* elemNormal = vertexData->vertexDeclaration->
                                              findElementBySemantic(VES_NORMAL);

        data->mUseVertexNormals &= (elemNormal != NULL);
        if(data->mUseVertexNormals)
        {
            if(elemNormal->getSource() == elemPos->getSource())
            {
                // Don't open the same buffer twice. Just copy the pointer.
                vNormalBuf = vbuf;
                vNormal = vStart;
            }
            else
            {
                vNormalBuf = vertexData->vertexBufferBinding->getBuffer(elemNormal->getSource());
                vNormal = static_cast<unsigned char*>(
                              vNormalBuf->lock(v1::HardwareBuffer::HBL_READ_ONLY) );
            }
            vNormSize = vNormalBuf->getVertexSize();
        }


        // Loop through all vertices and insert them to the Unordered Map.
        for (; vertex < vEnd; vertex += vSize)
        {
            float* pFloat;
            elemPos->baseVertexPointerToElement(vertex, &pFloat);
            data->mVertexList.push_back(LodData::Vertex());
            LodData::Vertex* v = &data->mVertexList.back();
            v->position.x = pFloat[0];
            v->position.y = pFloat[1];
            v->position.z = pFloat[2];
            std::pair<LodData::UniqueVertexSet::iterator, bool> ret;
            ret = data->mUniqueVertexSet.insert(v);
            if (!ret.second)
            {
                // Vertex position already exists.
                data->mVertexList.pop_back();
                v = *ret.first; // Point to the existing vertex.
                v->seam = true;
            }
            else
            {
#if OGRE_DEBUG_MODE
                // Needed for an assert, don't remove it.
                v->costHeapPosition = data->mCollapseCostHeap.end();
#endif
                v->seam = false;
            }
            lookup.push_back(v);

            if(data->mUseVertexNormals)
            {
                elemNormal->baseVertexPointerToElement(vNormal, &pFloat);
                if (!ret.second)
                {
                    if(v->normal.x != pFloat[0])
                    {
                        v->normal.x += pFloat[0];
                        v->normal.y += pFloat[1];
                        v->normal.z += pFloat[2];
                        if(v->normal.isZeroLength())
                        {
                            v->normal = Vector3(1.0, 0.0, 0.0);
                        }
                        v->normal.normalise();
                    }
                }
                else
                {
                    v->normal.x = pFloat[0];
                    v->normal.y = pFloat[1];
                    v->normal.z = pFloat[2];
                    v->normal.normalise();
                }
                vNormal += vNormSize;
            }
        }
        vbuf->unlock();
        if(data->mUseVertexNormals && elemNormal->getSource() != elemPos->getSource())
        {
            vNormalBuf->unlock();
        }
    }
    void LodInputProviderMesh::addIndexData( LodData* data, v1::IndexData* indexData,
            bool useSharedVertexLookup, unsigned short submeshID )
    {
        const v1::HardwareIndexBufferSharedPtr& ibuf = indexData->indexBuffer;
        size_t isize = ibuf->getIndexSize();
        data->mIndexBufferInfoList[submeshID].indexSize = isize;
        data->mIndexBufferInfoList[submeshID].indexCount = indexData->indexCount;
        if (indexData->indexCount == 0)
        {
            // Locking a zero length buffer on Linux with nvidia cards fails.
            return;
        }
        VertexLookupList& lookup = useSharedVertexLookup ? mSharedVertexLookup : mVertexLookup;

        // Lock the buffer for reading.
        char* iStart = static_cast<char*>(ibuf->lock(v1::HardwareBuffer::HBL_READ_ONLY));
        char* iEnd = iStart + ibuf->getSizeInBytes();
        if (isize == sizeof(unsigned short))
        {
            addIndexDataImpl<unsigned short>(data, (unsigned short*) iStart, (unsigned short*) iEnd, lookup, submeshID);
        }
        else
        {
            // Unsupported index size.
            OgreAssert(isize == sizeof(unsigned int), "");
            addIndexDataImpl<unsigned int>(data, (unsigned int*) iStart, (unsigned int*) iEnd, lookup, submeshID);
        }
        ibuf->unlock();
    }

}
