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
#ifndef __Vector4_H__
#define __Vector4_H__

#include "OgrePrerequisites.h"
#include "OgreVector3.h"

namespace Ogre
{

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Math
    *  @{
    */
    /** 4-dimensional homogeneous vector.
    */
    class _OgreExport Vector4 : public Vector<4>
    {
    public:
        /** Default constructor.
            @note
                It does <b>NOT</b> initialize the vector for efficiency.
        */
        Vector4() {}
        Vector4(const Vector<4>& o) : Vector<4>(o) {}
        Vector4(const Real fX, const Real fY, const Real fZ, const Real fW) : Vector<4>(fX, fY, fZ, fW) {}

        explicit Vector4(const Real afCoordinate[4]) : Vector<4>(afCoordinate) {}
        explicit Vector4(const int aiCoordinate[4] ) : Vector<4>(aiCoordinate) {}
        explicit Vector4(const Real scaler) : Vector<4>(scaler) {}
        explicit Vector4(const Vector3& rhs) : Vector<4>(rhs.x, rhs.y, rhs.z, 1.0f) {}

        inline Vector<4>& operator = ( const Real fScalar)
        {
            x = fScalar;
            y = fScalar;
            z = fScalar;
            w = fScalar;
            return *this;
        }

        inline Vector<4>& operator = (const Vector<3>& rhs)
        {
            x = rhs.x;
            y = rhs.y;
            z = rhs.z;
            w = 1.0f;
            return *this;
        }

        // special
        static const Vector<4> ZERO;
    };
    /** @} */
    /** @} */

}
#endif

