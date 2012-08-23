/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2012 Torus Knot Software Ltd
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#include "OgreVolumeIsoSurface.h"

#include "OgreVolumeIsoSurfaceTables.h"

namespace Ogre {
namespace Volume {

    const Real IsoSurface::ISO_LEVEL = (Real)0.0;

    const size_t IsoSurface::MS_CORNERS_FRONT[4] = {7, 6, 2, 3};
    const size_t IsoSurface::MS_CORNERS_BACK[4] = {5, 4, 0, 1};
    const size_t IsoSurface::MS_CORNERS_LEFT[4] = {4, 7, 3, 0};
    const size_t IsoSurface::MS_CORNERS_RIGHT[4] = {6, 5, 1, 2};
    const size_t IsoSurface::MS_CORNERS_TOP[4] = {4, 5, 6, 7};
    const size_t IsoSurface::MS_CORNERS_BOTTOM[4] = {3, 2, 1, 0};
    
    //-----------------------------------------------------------------------

    IsoSurface::IsoSurface(const Source *src) : mSrc(src)
    {
    }
    
    //-----------------------------------------------------------------------

    void IsoSurface::addMarchingCubesTriangles(const Vector3 *corners, const Vector4 *volumeValues, MeshBuilder *mb) const
    {
        unsigned char cubeIndex = 0;
        Vector3 normals[8];
        Real values[8];

        // Find out the case.
        for (size_t i = 0; i < 8; ++i)
        {
            if (volumeValues)
            {
                normals[i].x = volumeValues[i].x;
                normals[i].y = volumeValues[i].y;
                normals[i].z = volumeValues[i].z;
                values[i] = volumeValues[i].w;
            }
            else
            {
                values[i] = mSrc->getValueAndGradient(corners[i], normals[i]);
            }
            if (values[i] >= ISO_LEVEL)
            {
                cubeIndex |= 1 << i;
                normals[i].normalise();
            }
        }

        int edge = mcEdges[cubeIndex];

        // Nothing intersects.
        if (!edge)
        {
            return;
        }

        // Find the intersection vertices.
        Vector3 intersectionPoints[12];
        Vector3 intersectionNormals[12];
        if (edge & 1)
        {
            intersectionPoints[0] = interpolate(corners[0], corners[1], values[0], values[1],
                normals[0], normals[1], intersectionNormals[0]);
        }
        if (edge & 2)
        {
            intersectionPoints[1] = interpolate(corners[1], corners[2], values[1], values[2],
                normals[1], normals[2], intersectionNormals[1]);
        }
        if (edge & 4)
        {
            intersectionPoints[2] = interpolate(corners[2], corners[3], values[2], values[3],
                normals[2], normals[3], intersectionNormals[2]);
        }
        if (edge & 8)
        {
            intersectionPoints[3] = interpolate(corners[3], corners[0], values[3], values[0],
                normals[3], normals[0], intersectionNormals[3]);
        }
        if (edge & 16)
        {
            intersectionPoints[4] = interpolate(corners[4], corners[5], values[4], values[5],
                normals[4], normals[5], intersectionNormals[4]);
        }
        if (edge & 32)
        {
            intersectionPoints[5] = interpolate(corners[5], corners[6], values[5], values[6],
                normals[5], normals[6], intersectionNormals[5]);
        }
        if (edge & 64)
        {
            intersectionPoints[6] = interpolate(corners[6], corners[7], values[6], values[7],
                normals[6], normals[7], intersectionNormals[6]);
        }
        if (edge & 128)
        {
            intersectionPoints[7] = interpolate(corners[7], corners[4], values[7], values[4],
                normals[7], normals[4], intersectionNormals[7]);
        }
        if (edge & 256)
        {
            intersectionPoints[8] = interpolate(corners[0], corners[4], values[0], values[4],
                normals[0], normals[4], intersectionNormals[8]);
        }
        if (edge & 512)
        {
            intersectionPoints[9] = interpolate(corners[1], corners[5], values[1], values[5],
                normals[1], normals[5], intersectionNormals[9]);
        }
        if (edge & 1024)
        {
            intersectionPoints[10] = interpolate(corners[2], corners[6], values[2], values[6],
                normals[2], normals[6], intersectionNormals[10]);
        }
        if (edge & 2048)
        {
            intersectionPoints[11] = interpolate(corners[3], corners[7], values[3], values[7],
                normals[3], normals[7], intersectionNormals[11]);
        }

        // Create the triangles according to the table.
        for (int i = 0; mcTriangles[cubeIndex][i] != -1; i += 3)
        {
            mb->addTriangle(Triangle(
                intersectionPoints[mcTriangles[cubeIndex][i]], intersectionNormals[mcTriangles[cubeIndex][i]],
                intersectionPoints[mcTriangles[cubeIndex][i + 1]], intersectionNormals[mcTriangles[cubeIndex][i + 1]],
                intersectionPoints[mcTriangles[cubeIndex][i + 2]], intersectionNormals[mcTriangles[cubeIndex][i + 2]]));
        }
    }
    
    //-----------------------------------------------------------------------

    void IsoSurface::addMarchingSquaresTriangles(const Vector3 *corners, const Vector4 *volumeValues, const size_t *indices, const Real maxDistance, MeshBuilder *mb) const
    {
        unsigned char squareIndex = 0;
        Vector3 normals[4];
        Real values[4];

        // Find out the case.
        for (size_t i = 0; i < 4; ++i)
        {
            if (volumeValues)
            {
                normals[i].x = volumeValues[indices[i]].x;
                normals[i].y = volumeValues[indices[i]].y;
                normals[i].z = volumeValues[indices[i]].z;
                values[i] = volumeValues[indices[i]].w;
            }
            else
            {
                values[i] = mSrc->getValueAndGradient(corners[indices[i]], normals[i]);
            }
            if (values[i] >= ISO_LEVEL)
            {
                squareIndex |= 1 << i;
                normals[i].normalise();
            }
        }
    
        // Don't generate triangles if we are completly inside and far enough away from the surfacec
        if (squareIndex == 15 && values[0] >= maxDistance && values[1] >= maxDistance && values[2] >= maxDistance && values[3] >= maxDistance)
        {
            return;
        }

        int edge = msEdges[squareIndex];

        // Find the intersection vertices.
        Vector3 intersectionPoints[8];
        Vector3 intersectionNormals[8];
        intersectionPoints[0] = corners[indices[0]];
        intersectionPoints[2] = corners[indices[1]];
        intersectionPoints[4] = corners[indices[2]];
        intersectionPoints[6] = corners[indices[3]];

        Real innerVal = mSrc->getValueAndGradient(intersectionPoints[0], intersectionNormals[0]);
        intersectionNormals[0].normalise();
        intersectionNormals[0] *= innerVal + (Real)1.0;
        innerVal = mSrc->getValueAndGradient(intersectionPoints[2], intersectionNormals[2]);
        intersectionNormals[2].normalise();
        intersectionNormals[2] *= innerVal + (Real)1.0;
        innerVal = mSrc->getValueAndGradient(intersectionPoints[4], intersectionNormals[4]);
        intersectionNormals[4].normalise();
        intersectionNormals[4] *= innerVal + (Real)1.0;
        innerVal = mSrc->getValueAndGradient(intersectionPoints[6], intersectionNormals[6]);
        intersectionNormals[6].normalise();
        intersectionNormals[6] *= innerVal + (Real)1.0;

        if (edge & 1)
        {
            intersectionPoints[1] = interpolate(corners[indices[0]], corners[indices[1]], values[0], values[1],
                normals[0], normals[1], intersectionNormals[1]);
        }
        if (edge & 2)
        {
            intersectionPoints[3] = interpolate(corners[indices[1]], corners[indices[2]], values[1], values[2],
                normals[1], normals[2], intersectionNormals[3]);
        }
        if (edge & 4)
        {
            intersectionPoints[5] = interpolate(corners[indices[2]], corners[indices[3]], values[2], values[3],
                normals[2], normals[3], intersectionNormals[5]);
        }
        if (edge & 8)
        {
            intersectionPoints[7] = interpolate(corners[indices[3]], corners[indices[0]], values[3], values[0],
                normals[3], normals[0], intersectionNormals[7]);
        }
        
        // Ambigous case handling, 5 = 0 2 and 10 = 1 3
        /*if (squareIndex == 5 || squareIndex == 10)
        {
            Vector3 avg = (corners[indices[0]] + corners[indices[1]] + corners[indices[2]] + corners[indices[3]]) / (Real)4.0;
            // Lets take the alternative.
            if (mSrc->getValue(avg) >= ISO_LEVEL)
            {
                squareIndex = squareIndex == 5 ? 16 : 17;
            }
        }*/

        // Create the triangles according to the table.
        for (int i = 0; mcTriangles[squareIndex][i] != -1; i += 3)
        {
            mb->addTriangle(Triangle(
                intersectionPoints[msTriangles[squareIndex][i]], intersectionNormals[msTriangles[squareIndex][i]],
                intersectionPoints[msTriangles[squareIndex][i + 1]], intersectionNormals[msTriangles[squareIndex][i + 1]],
                intersectionPoints[msTriangles[squareIndex][i + 2]], intersectionNormals[msTriangles[squareIndex][i + 2]]));
        }
    }

}
}