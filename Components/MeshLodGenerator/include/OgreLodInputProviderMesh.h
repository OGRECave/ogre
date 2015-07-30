
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

#ifndef _LodInputProviderMesh_H__
#define _LodInputProviderMesh_H__

#include "OgreLodPrerequisites.h"
#include "OgreLodInputProvider.h"
#include "OgreLodData.h"
#include "OgreSharedPtr.h"
#include "OgreLogManager.h"

namespace Ogre
{

    class _OgreLodExport LodInputProviderMesh :
        public LodInputProvider
    {
    public:
        LodInputProviderMesh(v1::MeshPtr mesh);
        /// Called when the data should be filled with the input.
        virtual void initData(LodData* data);

    protected:
        typedef vector<LodData::Vertex*>::type VertexLookupList;
        // This helps to find the vertex* in LodData for index buffer indices
        VertexLookupList mSharedVertexLookup;
        VertexLookupList mVertexLookup;
        v1::MeshPtr mMesh;

        void tuneContainerSize(LodData* data);
        void initialize(LodData* data);
        void addIndexData(LodData* data, v1::IndexData* indexData, bool useSharedVertexLookup, unsigned short submeshID);
        void addVertexData(LodData* data, v1::VertexData* vertexData, bool useSharedVertexLookup);
        template<typename IndexType>
        void addIndexDataImpl(LodData* data, IndexType* iPos, const IndexType* iEnd,
                              VertexLookupList& lookup,
                              unsigned short submeshID)
        {
            // Loop through all triangles and connect them to the vertices.
            for (; iPos < iEnd; iPos += 3)
            {
                // It should never reallocate or every pointer will be invalid.
                OgreAssert(data->mTriangleList.capacity() > data->mTriangleList.size(), "");
                data->mTriangleList.push_back(LodData::Triangle());
                LodData::Triangle* tri = &data->mTriangleList.back();
                tri->isRemoved = false;
                tri->submeshID = submeshID;
                for (int i = 0; i < 3; i++)
                {
                    // Invalid index: Index is bigger then vertex buffer size.
                    OgreAssert(iPos[i] < lookup.size(), "");
                    tri->vertexID[i] = iPos[i];
                    tri->vertex[i] = lookup[iPos[i]];
                }
                if (tri->isMalformed())
                {
#if OGRE_DEBUG_MODE
                    stringstream str;
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
    };

}
#endif


