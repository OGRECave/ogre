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
#ifndef __Vector3_H__
#define __Vector3_H__

#include "OgrePrerequisites.h"
#include "OgreQuaternion.h"
#include "OgreVector2.h"

namespace Ogre
{

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Math
    *  @{
    */
    /** Standard 3-dimensional vector.
        @remarks
            A direction in 3D space represented as distances along the 3
            orthogonal axes (x, y, z). Note that positions, directions and
            scaling factors can be represented by a vector, depending on how
            you interpret the values.
    */
    class _OgreExport Vector3 : public Vector<3>
    {
    public:
        /** Default constructor.
            @note
                It does <b>NOT</b> initialize the vector for efficiency.
        */
        Vector3() {}
        Vector3(const Real fX, const Real fY, const Real fZ) : Vector<3>(fX, fY, fZ) {}

        explicit Vector3(const Real afCoordinate[3]) : Vector<3>(afCoordinate) {}
        explicit Vector3(const int aiCoordinate[3]) : Vector<3>(aiCoordinate) {}
        explicit Vector3(const Real scaler) : Vector<3>(scaler) {}

        // transparent conversion
        operator const Vector<3>&() { return *this; }

        inline Vector<3>& operator = ( const Real fScaler )
        {
            x = fScaler;
            y = fScaler;
            z = fScaler;

            return *this;
        }

        /** Calculates the absolute dot (scalar) product of this vector with another.
            @remarks
                This function work similar dotProduct, except it use absolute value
                of each component of the vector to computing.
            @param
                vec Vector with which to calculate the absolute dot product (together
                with this one).
            @return
                A Real representing the absolute dot product value.
        */
        inline Real absDotProduct(const Vector3& vec) const
        {
            return Math::Abs(x * vec.x) + Math::Abs(y * vec.y) + Math::Abs(z * vec.z);
        }

        /** Returns a vector at a point half way between this and the passed
            in vector.
        */
        inline Vector3 midPoint( const Vector3& vec ) const
        {
            return Vector3(
                ( x + vec.x ) * 0.5f,
                ( y + vec.y ) * 0.5f,
                ( z + vec.z ) * 0.5f );
        }

        /** Generates a new random vector which deviates from this vector by a
            given angle in a random direction.
            @remarks
                This method assumes that the random number generator has already
                been seeded appropriately.
            @param
                angle The angle at which to deviate
            @param
                up Any vector perpendicular to this one (which could generated
                by cross-product of this vector and any other non-colinear
                vector). If you choose not to provide this the function will
                derive one on it's own, however if you provide one yourself the
                function will be faster (this allows you to reuse up vectors if
                you call this method more than once)
            @return
                A random vector which deviates from this vector by angle. This
                vector will not be normalised, normalise it if you wish
                afterwards.
        */
        inline Vector3 randomDeviant(
            const Radian& angle,
            const Vector3& up = Vector3::ZERO ) const
        {
            Vector3 newUp;

            if (up == Vector3::ZERO)
            {
                // Generate an up vector
                newUp = this->perpendicular();
            }
            else
            {
                newUp = up;
            }

            // Rotate up vector by random amount around this
            Quaternion q;
            q.FromAngleAxis( Radian(Math::UnitRandom() * Math::TWO_PI), *this );
            newUp = q * newUp;

            // Finally rotate this by given angle around randomised up
            q.FromAngleAxis( angle, newUp );
            return q * (*this);
        }

        /** Gets the shortest arc quaternion to rotate this vector to the destination
            vector.
        @remarks
            If you call this with a dest vector that is close to the inverse
            of this vector, we will rotate 180 degrees around the 'fallbackAxis'
            (if specified, or a generated axis if not) since in this case
            ANY axis of rotation is valid.
        */
        Quaternion getRotationTo(const Vector3& dest,
            const Vector3& fallbackAxis = Vector3::ZERO) const
        {
            // From Sam Hocevar's article "Quaternion from two vectors:
            // the final version"
            Real a = Math::Sqrt(this->squaredLength() * dest.squaredLength());
            Real b = a + this->dotProduct(dest);
            Vector3 axis;

            if (b < (Real)1e-06 * a)
            {
                b = (Real)0.0;
                axis = fallbackAxis != Vector3::ZERO ? fallbackAxis
                     : Math::Abs(x) > Math::Abs(z) ? Vector3(-y, x, (Real)0.0)
                     : Vector3((Real)0.0, -z, y);
            }
            else
            {
                axis = this->crossProduct(dest);
            }

            Quaternion q(b, axis.x, axis.y, axis.z);
            q.normalise();
            return q;
        }

        /** Returns whether this vector is within a positional tolerance
            of another vector, also take scale of the vectors into account.
        @param rhs The vector to compare with
        @param tolerance The amount (related to the scale of vectors) that distance
            of the vector may vary by and still be considered close
        */
        inline bool positionCloses(const Vector3& rhs, Real tolerance = 1e-03f) const
        {
            return squaredDistance(rhs) <=
                (squaredLength() + rhs.squaredLength()) * tolerance;
        }

        /** Returns whether this vector is within a directional tolerance
            of another vector.
        @param rhs The vector to compare with
        @param tolerance The maximum angle by which the vectors may vary and
            still be considered equal
        @note Both vectors should be normalised.
        */
        inline bool directionEquals(const Vector3& rhs,
            const Radian& tolerance) const
        {
            Real dot = dotProduct(rhs);
            Radian angle = Math::ACos(dot);

            return Math::Abs(angle.valueRadians()) <= tolerance.valueRadians();

        }

        /// Extract the primary (dominant) axis from this direction vector
        inline Vector3 primaryAxis() const
        {
            Real absx = Math::Abs(x);
            Real absy = Math::Abs(y);
            Real absz = Math::Abs(z);
            if (absx > absy)
                if (absx > absz)
                    return x > 0 ? Vector3::UNIT_X : Vector3::NEGATIVE_UNIT_X;
                else
                    return z > 0 ? Vector3::UNIT_Z : Vector3::NEGATIVE_UNIT_Z;
            else // absx <= absy
                if (absy > absz)
                    return y > 0 ? Vector3::UNIT_Y : Vector3::NEGATIVE_UNIT_Y;
                else
                    return z > 0 ? Vector3::UNIT_Z : Vector3::NEGATIVE_UNIT_Z;
        }

        // special points
        static const Vector3 ZERO;
        static const Vector3 UNIT_X;
        static const Vector3 UNIT_Y;
        static const Vector3 UNIT_Z;
        static const Vector3 NEGATIVE_UNIT_X;
        static const Vector3 NEGATIVE_UNIT_Y;
        static const Vector3 NEGATIVE_UNIT_Z;
        static const Vector3 UNIT_SCALE;
    };
    /** @} */
    /** @} */

}
#endif
