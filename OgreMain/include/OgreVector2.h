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
#ifndef __Vector2_H__
#define __Vector2_H__


#include "OgrePrerequisites.h"
#include "OgreMath.h"
#include "OgreMatrix.h"

namespace Ogre
{

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Math
    *  @{
    */
    /** Standard 2-dimensional vector.
        @remarks
            A direction in 2D space represented as distances along the 2
            orthogonal axes (x, y). Note that positions, directions and
            scaling factors can be represented by a vector, depending on how
            you interpret the values.
    */
    class _OgreExport Vector2 : public Vector<2>
    {
    public:
        /** Default constructor.
            @note
                It does <b>NOT</b> initialize the vector for efficiency.
        */
        Vector2() {}
        Vector2(const Real fX, const Real fY) : Vector<2>(fX, fY) {}
        explicit Vector2(const Real scaler) : Vector<2>(scaler) {}
        explicit Vector2(const Real afCoordinate[2]) : Vector<2>(afCoordinate) {}
        explicit Vector2( const int afCoordinate[2] ) : Vector<2>(afCoordinate) {}

        // transparent conversion
        operator const Vector<2>&() { return *this; }

        inline Vector2& operator = ( const Real fScalar)
        {
            x = fScalar;
            y = fScalar;

            return *this;
        }

        /** Returns a vector at a point half way between this and the passed
            in vector.
        */
        inline Vector2 midPoint( const Vector2& vec ) const
        {
            return Vector2(
                ( x + vec.x ) * 0.5f,
                ( y + vec.y ) * 0.5f );
        }

        /** Calculates the 2 dimensional cross-product of 2 vectors, which results
            in a single floating point value which is 2 times the area of the triangle.
        */
        inline Real crossProduct( const Vector2& rkVector ) const
        {
            return x * rkVector.y - y * rkVector.x;
        }

        /** Generates a new random vector which deviates from this vector by a
            given angle in a random direction.
            @remarks
                This method assumes that the random number generator has already
                been seeded appropriately.
            @param angle
                The angle at which to deviate in radians
            @return
                A random vector which deviates from this vector by angle. This
                vector will not be normalised, normalise it if you wish
                afterwards.
        */
        inline Vector2 randomDeviant(Radian angle) const
        {
            angle *= Math::RangeRandom(-1, 1);
            Real cosa = Math::Cos(angle);
            Real sina = Math::Sin(angle);
            return Vector2(cosa * x - sina * y,
                           sina * x + cosa * y);
        }

        /**  Gets the oriented angle between 2 vectors.
        @remarks
            Vectors do not have to be unit-length but must represent directions.
            The angle is comprised between 0 and 2 PI.
        */
        inline Radian angleTo(const Ogre::Vector2& other) const
        {
            Radian angle = angleBetween(other);
        
            if (crossProduct(other)<0)          
                angle = (Radian)Math::TWO_PI - angle;

            return angle;
        }

        // special points
        static const Vector2 ZERO;
        static const Vector2 UNIT_X;
        static const Vector2 UNIT_Y;
        static const Vector2 NEGATIVE_UNIT_X;
        static const Vector2 NEGATIVE_UNIT_Y;
        static const Vector2 UNIT_SCALE;
    };
    /** @} */
    /** @} */

}
#endif
