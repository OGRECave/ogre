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
    
    void LodInputProvider::printTriangle(LodData::Triangle* triangle, std::ostream& str)
{
    for (int i = 0; i < 3; i++) {
        str << (i + 1) << ". vertex position: ("
            << triangle->vertex[i]->position.x << ", "
            << triangle->vertex[i]->position.y << ", "
            << triangle->vertex[i]->position.z << ") "
            << "vertex ID: " << triangle->vertexID[i] << std::endl;
    }
}

bool LodInputProvider::isDuplicateTriangle(LodData::Triangle* triangle, LodData::Triangle* triangle2)
{
    for (int i = 0; i < 3; i++) {
        if (triangle->vertex[i] != triangle2->vertex[0] ||
            triangle->vertex[i] != triangle2->vertex[1] ||
            triangle->vertex[i] != triangle2->vertex[2]) {
                return false;
        }
    }
    return true;
}

LodData::Triangle* LodInputProvider::isDuplicateTriangle(LodData::Triangle* triangle)
{
    // duplicate triangle detection (where all vertices has the same position)
    LodData::VTriangles::iterator itEnd = triangle->vertex[0]->triangles.end();
    LodData::VTriangles::iterator it = triangle->vertex[0]->triangles.begin();
    for (; it != itEnd; ++it) {
        LodData::Triangle* t = *it;
        if (isDuplicateTriangle(triangle, t)) {
            return *it;
        }
    }
    return NULL;
}
void LodInputProvider::addTriangleToEdges(LodData* data, LodData::Triangle* triangle)
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
    for (int i = 0; i < 3; i++) {
        triangle->vertex[i]->triangles.addNotExists(triangle);
    }
    for (int i = 0; i < 3; i++) {
        for (int n = 0; n < 3; n++) {
            if (i != n) {
                triangle->vertex[i]->addEdge(LodData::Edge(triangle->vertex[n]));
            }
        }
    }
}

}
