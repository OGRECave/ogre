/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#ifndef __Ogre_Volume_IsoSurface_H__
#define __Ogre_Volume_IsoSurface_H__

#include "OgreVolumeMeshBuilder.h"
#include "OgreVolumeSource.h"
#include "OgreVolumePrerequisites.h"

namespace Ogre {
namespace Volume {

    /** Marching Cubes implementation like at
        http://local.wasp.uwa.edu.au/~pbourke/geometry/polygonise/
     */
    class _OgreVolumeExport IsoSurface
    {
    protected:
        
        /// The value where our isosurface is.
        static const Real ISO_LEVEL;
        
        /// To get the isovalue and normal.
        const Source *mSrc;

        /** Linear interpolation between two vectors based on some values associated to them.
        @param v1
            The first vector.
        @param v2
            The second vector.
        @param val1
            The value for the first vector.
        @param val2
            The value for the second vector.
        @param normal1
            The normal of the first vector.
        @param normal2
            The normal of the second vector.
        @param normal
            Reference to a vector where the normal will be stored.
        */
        inline Vector3 interpolate(const Vector3 &v1, const Vector3 &v2, const Real val1, const Real val2, const Vector3 &normal1, const Vector3 &normal2, Vector3 &normal) const
        {
            // Don't use Math::RealEqual here as it isn't inlined and this function is performance critical.
            if (fabs(val1 - ISO_LEVEL) <= FLT_EPSILON)
            {
                normal = normal1;
                return v1;
            }
            if (fabs(val2 - ISO_LEVEL) <= FLT_EPSILON)
            {
                normal = normal2;
                return v2;
            }
            if (fabs(val2 - val1) <= FLT_EPSILON)
            {
                normal = normal1;
                return v1;
            }
            Real mu = (ISO_LEVEL - val1) / (val2 - val1);
            normal = normal1 + mu * (normal2 - normal1);
            normal.normalise();
            return v1 + mu * (v2 - v1);
        }

    public:

        /// To call Marching Squares with a cube on its front.
        static const size_t MS_CORNERS_FRONT[4];

        /// To call Marching Squares with a cube on its back.
        static const size_t MS_CORNERS_BACK[4];
        
        /// To call Marching Squares with a cube on its left.
        static const size_t MS_CORNERS_LEFT[4];
        
        /// To call Marching Squares with a cube on its right.
        static const size_t MS_CORNERS_RIGHT[4];
                
        /// To call Marching Squares with a cube on its top.
        static const size_t MS_CORNERS_TOP[4];
                
        /// To call Marching Squares with a cube on its bottom.
        static const size_t MS_CORNERS_BOTTOM[4];
        
        /** Constructor.
        @param src
            The source for the isovalues and normals there.
        */
        explicit IsoSurface(const Source *src);

        /** Adds triangles to a MeshBuilder via Marching Cubes.
        @param corners
            The corners of the cube to triangulate via Marching Cubes.
        @param volumeValues
            The cached volume values, one Vector4 consists of gradient (x, y, z) and density (w). If 0 is given, it will be calculated.
        @param mb
            The MeshBuilder to add the triangles to.
        */
        void addMarchingCubesTriangles(const Vector3 *corners, const Vector4 *volumeValues, MeshBuilder *mb) const;

        /** Adds triangles to a MeshBuilder via Marching Squares.
        @param corners
            The corners of the cube where one side is to be triangulated.
        @param volumeValues
            The cached volume values, one Vector4 consists of gradient (x, y, z) and density (w). If 0 is given, it will be calculated.
        @param indices
            The four corners of the cube (== one side) to triangulate.
        @param maxDistance
            The maximum distance to the surface where triangles are generated.
        @param mb
            The MeshBuilder to add the triangles to.
        */
        void addMarchingSquaresTriangles(const Vector3 *corners, const Vector4 *volumeValues, const size_t *indices, const Real maxDistance, MeshBuilder *mb) const;
    };
}
}

#endif
