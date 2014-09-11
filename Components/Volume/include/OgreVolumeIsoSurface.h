/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

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

    /** Abstract IsoSurface.
     */
    class _OgreVolumeExport IsoSurface : public UtilityAlloc
    {
    protected:
        
        /// The value where our isosurface is.
        static const Real ISO_LEVEL;
        
        /// To get the isovalue and normal.
        const Source *mSrc;

        /** Constructor.
        @param src
            The source to use.
        */
        explicit IsoSurface(const Source *src);

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

        virtual ~IsoSurface(void);
        
        /** Adds triangles to a MeshBuilder via Marching Cubes.
        @param corners
            The corners of the cube to triangulate via Marching Cubes.
        @param volumeValues
            The cached volume values, one Vector4 consists of gradient (x, y, z) and density (w). If 0 is given, it will be calculated.
        @param mb
            The MeshBuilder to add the triangles to.
        */
        virtual void addMarchingCubesTriangles(const Vector3 *corners, const Vector4 *volumeValues, MeshBuilder *mb) const = 0;

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
        virtual void addMarchingSquaresTriangles(const Vector3 *corners, const Vector4 *volumeValues, const size_t *indices, const Real maxDistance, MeshBuilder *mb) const = 0;
    };
}
}

#endif
