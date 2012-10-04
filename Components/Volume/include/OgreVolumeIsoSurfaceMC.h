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
#ifndef __Ogre_Volume_IsoSurfaceMC_H__
#define __Ogre_Volume_IsoSurfaceMC_H__

#include "OgreVolumeIsoSurface.h"

namespace Ogre {
namespace Volume {

    /** Marching Cubes implementation like at
        http://local.wasp.uwa.edu.au/~pbourke/geometry/polygonise/
     */
    class _OgreVolumeExport IsoSurfaceMC : public IsoSurface
    {
    protected:
        

        /** Linear interpolation between two vectors based on some values associated to them.
        @param v1
            The first vector.
        @param v2
            The second vector.
        @param val1
            The value for the first vector.
        @param val2
            The value for the second vector.
        @param normal
            Reference to a vector where the normal will be stored.
        @return
            The interpolated position.
        */
        inline Vector3 interpolate(const Vector3 &v1, const Vector3 &v2, const Vector4 val1, const Vector4 val2, Vector3 &normal) const
        {
            // Don't use Math::RealEqual here as it isn't inlined and this function is performance critical.
            if (fabs(val1.w - ISO_LEVEL) <= FLT_EPSILON)
            {
                normal.x = val1.x;
                normal.y = val1.y;
                normal.z = val1.z;
                return v1;
            }
            if (fabs(val2.w - ISO_LEVEL) <= FLT_EPSILON)
            {
                normal.x = val2.x;
                normal.y = val2.y;
                normal.z = val2.z;
                return v2;
            }
            if (fabs(val2.w - val1.w) <= FLT_EPSILON)
            {
                normal.x = val1.x;
                normal.y = val1.y;
                normal.z = val1.z;
                return v1;
            }
            Real mu = (ISO_LEVEL - val1.w) / (val2.w - val1.w);
            Vector4 normal4 = val1 + mu * (val2 - val1);
            normal.x = normal4.x;
            normal.y = normal4.y;
            normal.z = normal4.z;
            normal.normalise();
            return v1 + mu * (v2 - v1);
        }

    public:

        /** Constructor.
        @param src
            The source for the isovalues and normals there.
        */
        explicit IsoSurfaceMC(const Source *src);
        
        /** Overridden from IsoSurface.
        */
        virtual void addMarchingCubesTriangles(const Vector3 *corners, const Vector4 *volumeValues, MeshBuilder *mb) const;
        
        /** Overridden from IsoSurface.
        */
        virtual void addMarchingSquaresTriangles(const Vector3 *corners, const Vector4 *volumeValues, const size_t *indices, const Real maxDistance, MeshBuilder *mb) const;
    };
}
}

#endif
