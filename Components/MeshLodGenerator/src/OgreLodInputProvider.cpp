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
#if OGRE_DEBUG_MODE
    static void printTriangle(LodData::Triangle* triangle, std::ostream& str)
    {
        for (int i = 0; i < 3; i++) {
            str << (i + 1) << ". vertex position: ("
                << triangle->vertex[i]->position.x << ", "
                << triangle->vertex[i]->position.y << ", "
                << triangle->vertex[i]->position.z << ") "
                << "vertex ID: " << triangle->vertexID[i] << std::endl;
        }
    }
#endif

    static bool isDuplicateTriangle(LodData::Triangle* triangle, LodData::Triangle* triangle2)
    {
        for (auto & i : triangle->vertex) {
            if (i != triangle2->vertex[0] ||
                i != triangle2->vertex[1] ||
                i != triangle2->vertex[2]) {
                    return false;
            }
        }
        return true;
    }

    static LodData::Triangle* isDuplicateTriangle(LodData::Triangle* triangle)
    {
        // duplicate triangle detection (where all vertices has the same position)
        for (const auto& t : triangle->vertex[0]->triangles) {
            if (isDuplicateTriangle(triangle, t)) {
                return t;
            }
        }
        return NULL;
    }

    static void addTriangleToEdges(LodData* data, LodData::Triangle* triangle)
    {
        if(MESHLOD_QUALITY >= 3) {
            LodData::Triangle* duplicate = isDuplicateTriangle(triangle);
            if (duplicate != NULL) {
#if OGRE_DEBUG_MODE
                std::stringstream str;
                str << "In " << data->mMeshName << " duplicate triangle found." << std::endl;
                str << "Triangle " << LodData::getVectorIDFromPointer(data->mTriangleList, triangle) << " positions:" << std::endl;
                printTriangle(triangle, str);
                str << "Triangle " << LodData::getVectorIDFromPointer(data->mTriangleList, duplicate) << " positions:" << std::endl;
                printTriangle(duplicate, str);
                str << "Triangle " << LodData::getVectorIDFromPointer(data->mTriangleList, triangle) << " will be excluded from Lod level calculations.";
                LogManager::getSingleton().stream() << str.str();
#endif
                triangle->isRemoved = true;
                data->mIndexBufferInfoList[triangle->submeshID].indexCount -= 3;
                return;
            }
        }
        for (auto & i : triangle->vertex) {
            i->triangles.addNotExists(triangle);
        }
        for (int i = 0; i < 3; i++) {
            for (int n = 0; n < 3; n++) {
                if (i != n) {
                    triangle->vertex[i]->addEdge(LodData::Edge(triangle->vertex[n]));
                }
            }
        }
    }

    static void addLineToVertices(LodData* data, LodData::Line* line)
    {
        for (auto & i : line->vertex) {
            i->lines.addNotExists(line);
        }
    }

    inline size_t getTriangleCount(RenderOperation::OperationType renderOp, size_t indexCount)
    {
        if(renderOp == RenderOperation::OT_TRIANGLE_LIST)
            return indexCount / 3;
        else if(renderOp == RenderOperation::OT_TRIANGLE_STRIP || renderOp == RenderOperation::OT_TRIANGLE_FAN)
            return indexCount >= 3 ? indexCount - 2 : 0;
        return 0;
    }

    inline size_t getLineCount(RenderOperation::OperationType renderOp, size_t indexCount)
    {
        if(renderOp == RenderOperation::OT_LINE_LIST)
            return indexCount / 2;
        else if(renderOp == RenderOperation::OT_LINE_STRIP)
            return indexCount >= 2 ? indexCount - 1 : 0;
        return 0;
    }

    void LodInputProvider::initData( LodData* data )
    {
        tuneContainerSize(data);
        initialize(data);
    }
    void LodInputProvider::tuneContainerSize(LodData* data)
    {
        // Get Vertex count for container tuning.
        bool sharedVerticesAdded = false;
        size_t triangleCount = 0;
        size_t lineCount = 0;
        size_t vertexCount = 0;
        size_t vertexLookupSize = 0;
        size_t sharedVertexLookupSize = 0;
        size_t submeshCount = getSubMeshCount();
        for (size_t i = 0; i < submeshCount; i++) {
            triangleCount += getTriangleCount(getSubMeshRenderOp(i), getSubMeshIndexCount(i));
            lineCount += getLineCount(getSubMeshRenderOp(i), getSubMeshIndexCount(i));
            if (!getSubMeshUseSharedVertices(i)) {
                size_t count = getSubMeshOwnVertexCount(i);
                vertexLookupSize = std::max(vertexLookupSize, count);
                vertexCount += count;
            } else if (!sharedVerticesAdded) {
                sharedVerticesAdded = true;
                sharedVertexLookupSize = getMeshSharedVertexCount();
                vertexCount += sharedVertexLookupSize;
            }
        }

        // Tune containers:
        data->mUniqueVertexSet.rehash(4 * vertexCount); // less then 0.25 item/bucket for low collision rate

        data->mTriangleList.reserve(triangleCount);
        data->mLineList.reserve(lineCount);

        data->mVertexList.reserve(vertexCount);
        mSharedVertexLookup.reserve(sharedVertexLookupSize);
        mVertexLookup.reserve(vertexLookupSize);
        data->mIndexBufferInfoList.resize(submeshCount);
    }
    void LodInputProvider::initialize( LodData* data )
    {
#if OGRE_DEBUG_MODE
        data->mMeshName = getMeshName();
#endif
        data->mMeshBoundingSphereRadius = getMeshBoundingSphereRadius();
        size_t submeshCount = getSubMeshCount();
        for (size_t i = 0; i < submeshCount; ++i) {
            addVertexData(data, i);
            addIndexData(data, i);
        }

        // These were only needed for addIndexData() and addVertexData().
        mSharedVertexLookup.clear();
        mVertexLookup.clear();
    }


    template <typename IndexType>
    void addTriangleIndices(LodData* data, IndexType* iPos, const IndexType* iEnd,
                            std::vector<LodData::Vertex*>& lookup, ushort submeshID,
                            RenderOperation::OperationType renderOp)
    {
        IndexType i0 = iPos[0], i1 = iPos[1], i2 = iPos[2];
        unsigned inc = (renderOp==RenderOperation::OT_TRIANGLE_LIST) ? 3 : 1;
        unsigned triangleIdx = 0;

        // Loop through all triangles and connect them to the vertices.
        for (iPos += (3 - inc); iPos < iEnd; iPos += inc, ++triangleIdx) {
            // It should never reallocate or every pointer will be invalid.
            OgreAssertDbg(data->mTriangleList.capacity() > data->mTriangleList.size(), "");
            data->mTriangleList.push_back(LodData::Triangle());
            LodData::Triangle* tri = &data->mTriangleList.back();
            tri->isRemoved = false;
            tri->submeshID = submeshID;

            if(triangleIdx > 0)
            {
                switch(renderOp)
                {
                case RenderOperation::OT_TRIANGLE_LIST:
                    i0 = iPos[0];
                    i1 = iPos[1];
                    i2 = iPos[2];
                    break;
                case RenderOperation::OT_TRIANGLE_STRIP:
                    if(triangleIdx & 1)
                        i0 = i2;
                    else
                        i1 = i2;
                    i2 = iPos[0];
                    break;
                case RenderOperation::OT_TRIANGLE_FAN:
                    i1 = i2;
                    i2 = iPos[0];
                    break;
                default:
                    OgreAssert(false, "Invalid RenderOperation");
                    break;
                }
            }

            // Invalid index: Index is bigger then vertex buffer size.
            OgreAssertDbg(i0 < lookup.size() && i1 < lookup.size() && i2 < lookup.size(), "");
            tri->vertexID[0] = i0;
            tri->vertexID[1] = i1;
            tri->vertexID[2] = i2;
            tri->vertex[0] = lookup[i0];
            tri->vertex[1] = lookup[i1];
            tri->vertex[2] = lookup[i2];

            if (tri->isMalformed()) {
#if OGRE_DEBUG_MODE
                std::stringstream str;
                str << "In " << data->mMeshName << " malformed triangle found with ID: " << LodData::getVectorIDFromPointer(data->mTriangleList, tri) << ". " <<
                std::endl;
                printTriangle(tri, str);
                str << "It will be excluded from Lod level calculations.";
                LogManager::getSingleton().stream() << str.str();
#endif
                tri->isRemoved = true;
                data->mIndexBufferInfoList[tri->submeshID].indexCount -= 3;
                continue;
            }
            tri->computeNormal();
            addTriangleToEdges(data, tri);
        }
    }

    template <typename IndexType>
    void addLineIndices(LodData* data, IndexType* iPos, const IndexType* iEnd,
                        std::vector<LodData::Vertex*>& lookup, ushort submeshID,
                        RenderOperation::OperationType renderOp)
    {
        IndexType i0 = iPos[0], i1 = iPos[1];
        unsigned inc = (renderOp==RenderOperation::OT_LINE_LIST) ? 2 : 1;
        unsigned lineIdx = 0;

        // Loop through all lines and connect them to the vertices.
        for (iPos += (2 - inc); iPos < iEnd; iPos += inc, ++lineIdx) {
            // It should never reallocate or every pointer will be invalid.
            OgreAssertDbg(data->mLineList.capacity() > data->mLineList.size(), "");
            data->mLineList.push_back(LodData::Line());
            LodData::Line* line = &data->mLineList.back();
            line->isRemoved = false;
            line->submeshID = submeshID;

            if(lineIdx > 0)
            {
                switch(renderOp)
                {
                case RenderOperation::OT_LINE_LIST:
                    i0 = iPos[0];
                    i1 = iPos[1];
                    break;
                case RenderOperation::OT_LINE_STRIP:
                    i0 = i1;
                    i1 = iPos[0];
                    break;
                default:
                    OgreAssert(false, "Invalid RenderOperation");
                    break;
                }
            }

            // Invalid index: Index is bigger then vertex buffer size.
            OgreAssertDbg(i0 < lookup.size() && i1 < lookup.size(), "");
            line->vertexID[0] = i0;
            line->vertexID[1] = i1;
            line->vertex[0] = lookup[i0];
            line->vertex[1] = lookup[i1];

            if (line->isMalformed()) {
#if OGRE_DEBUG_MODE
                std::stringstream str;
                str << "In " << data->mMeshName << " malformed line found with ID: " << LodData::getVectorIDFromPointer(data->mLineList, line) << ". " <<
                std::endl;
//                printLine(tri, str);
                str << "It will be excluded from Lod level calculations.";
                LogManager::getSingleton().stream() << str.str();
#endif
                line->isRemoved = true;
                data->mIndexBufferInfoList[line->submeshID].indexCount -= 2;
                continue;
            }
            addLineToVertices(data, line);
        }
    }

    void LodInputProvider::addIndexData(LodData* data, size_t subMeshIndex)
    {
        const IndexData* id = getSubMeshIndexData(subMeshIndex);

        if (!id->indexCount || !id->indexBuffer) {
            // Locking a zero length buffer on Linux with nvidia cards fails.
            return;
        }

        data->mIndexBufferInfoList[subMeshIndex].indexSize = id->indexBuffer->getIndexSize();
        data->mIndexBufferInfoList[subMeshIndex].indexCount = id->indexCount;

        bool useSharedVertexLookup = getSubMeshUseSharedVertices(subMeshIndex);
        auto renderOp = getSubMeshRenderOp(subMeshIndex);
        VertexLookupList& lookup = useSharedVertexLookup ? mSharedVertexLookup : mVertexLookup;

        // Lock the buffer for reading.
        HardwareBufferLockGuard lock(id->indexBuffer, HardwareBuffer::HBL_READ_ONLY);
        size_t isize = id->indexBuffer->getIndexSize();
        uchar* iStart = (uchar*)lock.pData + id->indexStart * isize;
        uchar* iEnd = iStart + id->indexCount * isize;

        if (renderOp == RenderOperation::OT_TRIANGLE_LIST ||
            renderOp == RenderOperation::OT_TRIANGLE_STRIP ||
            renderOp == RenderOperation::OT_TRIANGLE_FAN)
        {
            if (isize == sizeof(unsigned short)) {
                addTriangleIndices(data, (unsigned short*) iStart, (const unsigned short*) iEnd, lookup, subMeshIndex, renderOp);
            } else {
                // Unsupported index size.
                OgreAssert(isize == sizeof(unsigned int), "");
                addTriangleIndices(data, (unsigned int*) iStart, (const unsigned int*) iEnd, lookup, subMeshIndex, renderOp);
            }
        }
        else if (renderOp == RenderOperation::OT_LINE_LIST ||
                 renderOp == RenderOperation::OT_LINE_STRIP)
        {
            if (isize == sizeof(unsigned short)) {
                addLineIndices(data, (unsigned short*) iStart, (const unsigned short*) iEnd, lookup, subMeshIndex, renderOp);
            } else {
                // Unsupported index size.
                OgreAssert(isize == sizeof(unsigned int), "");
                addLineIndices(data, (unsigned int*) iStart, (const unsigned int*) iEnd, lookup, subMeshIndex, renderOp);
            }
        }
    }
}
