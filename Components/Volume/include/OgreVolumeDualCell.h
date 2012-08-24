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
#ifndef __Ogre_Volume_DualCell_H__
#define __Ogre_Volume_DualCell_H__

#include "OgreManualObject.h"
#include "OgreVolumePrerequisites.h"

namespace Ogre {
namespace Volume {

    /** A single cell in the DualGrid.
     */
    class _OgreVolumeExport DualCell
    {
    protected:
        /// The corners of the cell.
        Vector3 mCorners[8];

        /// Density and normal on the corners.
        Vector4 mValues[8];
    public:
        /** Constructor.
         @param c0
            The first corner.
         @param c1
            The second corner.
         @param c2
            The third corner.
         @param c3
            The fourth corner.
         @param c4
            The fifth corner.
         @param c5
            The sixth corner.
         @param c6
            The seventh corner.
         @param c7
            The eighth corner.
         */
        DualCell(const Vector3 &c0, const Vector3 &c1, const Vector3 &c2, const Vector3 &c3, const Vector3 &c4, const Vector3 &c5, const Vector3 &c6, const Vector3 &c7);

        /** Constructor with values.
         @param c0
            The first corner.
         @param c1
            The second corner.
         @param c2
            The third corner.
         @param c3
            The fourth corner.
         @param c4
            The fifth corner.
         @param c5
            The sixth corner.
         @param c6
            The seventh corner.
         @param c7
            The eighth corner.
         @param v0
            The first of the cell, one Vector4 consists of gradient (x, y, z) and density (w).
         @param v1
            The second of the cell, one Vector4 consists of gradient (x, y, z) and density (w).
         @param v2
            The third of the cell, one Vector4 consists of gradient (x, y, z) and density (w).
         @param v3
            The fourth of the cell, one Vector4 consists of gradient (x, y, z) and density (w).
         @param v4
            The fifth of the cell, one Vector4 consists of gradient (x, y, z) and density (w).
         @param v5
            The sixth of the cell, one Vector4 consists of gradient (x, y, z) and density (w).
         @param v6
            The seventh of the cell, one Vector4 consists of gradient (x, y, z) and density (w).
         @param v7
            The eighth of the cell, one Vector4 consists of gradient (x, y, z) and density (w).
         */
        DualCell(const Vector3 &c0, const Vector3 &c1, const Vector3 &c2, const Vector3 &c3, const Vector3 &c4, const Vector3 &c5, const Vector3 &c6, const Vector3 &c7,
            const Vector4 &v0, const Vector4 &v1, const Vector4 &v2, const Vector4 &v3, const Vector4 &v4, const Vector4 &v5, const Vector4 &v6, const Vector4 &v7);

        /** Adds a line representation of this cell to a manual object.
         @param manual
            The manual object to add to.
         @param baseIndex
            The starting vertex index, will be incremented during this operation.
         */
        void addToManualObject(ManualObject *manual, size_t &baseIndex) const;

        /** Gets the corners of this cell.
        @return
            The corners.
        */
        inline const Vector3* getCorners(void) const
        {
            return mCorners;
        }
        
        /** Whether this dualcell has cached volume values.
        @return
            true if so.
        */
        inline bool hasValues(void) const
        {
            return mValues[0].x != (Real)0.0 || mValues[0].y != (Real)0.0 || mValues[0].z != (Real)0.0;
        }

        /** Gets the cached volume values.
        @return
            8 element array of the volume values, one Vector4 consists of gradient (x, y, z) and density (w).
        */
        inline const Vector4* getValues(void) const
        {
            return mValues;
        }
    };
}
}

#endif