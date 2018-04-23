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
    class _OgreExport Vector3
    {
    public:
        Real x, y, z;

    public:
        /** Default constructor.
            @note
                It does <b>NOT</b> initialize the vector for efficiency.
        */
        inline Vector3()
        {
        }

        inline Vector3( const Real fX, const Real fY, const Real fZ )
            : x( fX ), y( fY ), z( fZ )
        {
        }

        inline explicit Vector3( const Real afCoordinate[3] )
            : x( afCoordinate[0] ),
              y( afCoordinate[1] ),
              z( afCoordinate[2] )
        {
        }

        inline explicit Vector3( const int afCoordinate[3] )
        {
            x = (Real)afCoordinate[0];
            y = (Real)afCoordinate[1];
            z = (Real)afCoordinate[2];
        }

        inline explicit Vector3( const Real scaler )
            : x( scaler )
            , y( scaler )
            , z( scaler )
        {
        }

        /** Swizzle-like narrowing operations
        */
        inline Vector2 xy() const
        {
            return Vector2(x, y);
        }

        inline Real operator [] ( const size_t i ) const
        {
            assert( i < 3 );

            return *(&x+i);
        }

        inline Real& operator [] ( const size_t i )
        {
            assert( i < 3 );

            return *(&x+i);
        }
        /// Pointer accessor for direct copying
        inline Real* ptr()
        {
            return &x;
        }
        /// Pointer accessor for direct copying
        inline const Real* ptr() const
        {
            return &x;
        }

        inline bool operator == ( const Vector3& rkVector ) const
        {
            return ( x == rkVector.x && y == rkVector.y && z == rkVector.z );
        }

        inline bool operator != ( const Vector3& rkVector ) const
        {
            return ( x != rkVector.x || y != rkVector.y || z != rkVector.z );
        }

        // arithmetic operations
        inline Vector3 operator + ( const Vector3& rkVector ) const
        {
            return Vector3(
                x + rkVector.x,
                y + rkVector.y,
                z + rkVector.z);
        }

        inline Vector3 operator - ( const Vector3& rkVector ) const
        {
            return Vector3(
                x - rkVector.x,
                y - rkVector.y,
                z - rkVector.z);
        }

        inline Vector3 operator * ( const Real fScalar ) const
        {
            return Vector3(
                x * fScalar,
                y * fScalar,
                z * fScalar);
        }

        inline Vector3 operator * ( const Vector3& rhs) const
        {
            return Vector3(
                x * rhs.x,
                y * rhs.y,
                z * rhs.z);
        }

        inline Vector3 operator / ( const Real fScalar ) const
        {
            assert( fScalar != 0.0 );

            Real fInv = 1.0f / fScalar;

            return Vector3(
                x * fInv,
                y * fInv,
                z * fInv);
        }

        inline Vector3 operator / ( const Vector3& rhs) const
        {
            return Vector3(
                x / rhs.x,
                y / rhs.y,
                z / rhs.z);
        }

        inline const Vector3& operator + () const
        {
            return *this;
        }

        inline Vector3 operator - () const
        {
            return Vector3(-x, -y, -z);
        }

        // overloaded operators to help Vector3
        inline friend Vector3 operator * ( const Real fScalar, const Vector3& rkVector )
        {
            return Vector3(
                fScalar * rkVector.x,
                fScalar * rkVector.y,
                fScalar * rkVector.z);
        }

        inline friend Vector3 operator / ( const Real fScalar, const Vector3& rkVector )
        {
            return Vector3(
                fScalar / rkVector.x,
                fScalar / rkVector.y,
                fScalar / rkVector.z);
        }

        inline friend Vector3 operator + (const Vector3& lhs, const Real rhs)
        {
            return Vector3(
                lhs.x + rhs,
                lhs.y + rhs,
                lhs.z + rhs);
        }

        inline friend Vector3 operator + (const Real lhs, const Vector3& rhs)
        {
            return Vector3(
                lhs + rhs.x,
                lhs + rhs.y,
                lhs + rhs.z);
        }

        inline friend Vector3 operator - (const Vector3& lhs, const Real rhs)
        {
            return Vector3(
                lhs.x - rhs,
                lhs.y - rhs,
                lhs.z - rhs);
        }

        inline friend Vector3 operator - (const Real lhs, const Vector3& rhs)
        {
            return Vector3(
                lhs - rhs.x,
                lhs - rhs.y,
                lhs - rhs.z);
        }

        // arithmetic updates
        inline Vector3& operator += ( const Vector3& rkVector )
        {
            x += rkVector.x;
            y += rkVector.y;
            z += rkVector.z;

            return *this;
        }

        inline Vector3& operator += ( const Real fScalar )
        {
            x += fScalar;
            y += fScalar;
            z += fScalar;
            return *this;
        }

        inline Vector3& operator -= ( const Vector3& rkVector )
        {
            x -= rkVector.x;
            y -= rkVector.y;
            z -= rkVector.z;

            return *this;
        }

        inline Vector3& operator -= ( const Real fScalar )
        {
            x -= fScalar;
            y -= fScalar;
            z -= fScalar;
            return *this;
        }

        inline Vector3& operator *= ( const Real fScalar )
        {
            x *= fScalar;
            y *= fScalar;
            z *= fScalar;
            return *this;
        }

        inline Vector3& operator *= ( const Vector3& rkVector )
        {
            x *= rkVector.x;
            y *= rkVector.y;
            z *= rkVector.z;

            return *this;
        }

        inline Vector3& operator /= ( const Real fScalar )
        {
            assert( fScalar != 0.0 );

            Real fInv = 1.0f / fScalar;

            x *= fInv;
            y *= fInv;
            z *= fInv;

            return *this;
        }

        inline Vector3& operator /= ( const Vector3& rkVector )
        {
            x /= rkVector.x;
            y /= rkVector.y;
            z /= rkVector.z;

            return *this;
        }


        /** Returns the length (magnitude) of the vector.
            @warning
                This operation requires a square root and is expensive in
                terms of CPU operations. If you don't need to know the exact
                length (e.g. for just comparing lengths) use squaredLength()
                instead.
        */
        inline Real length () const
        {
            return Math::Sqrt( x * x + y * y + z * z );
        }

        /** Returns the square of the length(magnitude) of the vector.
            @remarks
                This  method is for efficiency - calculating the actual
                length of a vector requires a square root, which is expensive
                in terms of the operations required. This method returns the
                square of the length of the vector, i.e. the same as the
                length but before the square root is taken. Use this if you
                want to find the longest / shortest vector without incurring
                the square root.
        */
        inline Real squaredLength () const
        {
            return x * x + y * y + z * z;
        }

        /** Returns the distance to another vector.
            @warning
                This operation requires a square root and is expensive in
                terms of CPU operations. If you don't need to know the exact
                distance (e.g. for just comparing distances) use squaredDistance()
                instead.
        */
        inline Real distance(const Vector3& rhs) const
        {
            return (*this - rhs).length();
        }

        /** Returns the square of the distance to another vector.
            @remarks
                This method is for efficiency - calculating the actual
                distance to another vector requires a square root, which is
                expensive in terms of the operations required. This method
                returns the square of the distance to another vector, i.e.
                the same as the distance but before the square root is taken.
                Use this if you want to find the longest / shortest distance
                without incurring the square root.
        */
        inline Real squaredDistance(const Vector3& rhs) const
        {
            return (*this - rhs).squaredLength();
        }

        /** Calculates the dot (scalar) product of this vector with another.
            @remarks
                The dot product can be used to calculate the angle between 2
                vectors. If both are unit vectors, the dot product is the
                cosine of the angle; otherwise the dot product must be
                divided by the product of the lengths of both vectors to get
                the cosine of the angle. This result can further be used to
                calculate the distance of a point from a plane.
            @param
                vec Vector with which to calculate the dot product (together
                with this one).
            @return
                A float representing the dot product value.
        */
        inline Real dotProduct(const Vector3& vec) const
        {
            return x * vec.x + y * vec.y + z * vec.z;
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

        /** Normalises the vector.
            @remarks
                This method normalises the vector such that it's
                length / magnitude is 1. The result is called a unit vector.
            @note
                This function will not crash for zero-sized vectors, but there
                will be no changes made to their components.
            @return The previous length of the vector.
        */
        inline Real normalise()
        {
            Real fLength = Math::Sqrt( x * x + y * y + z * z );

            // Will also work for zero-sized vectors, but will change nothing
            // We're not using epsilons because we don't need to.
            // Read http://www.ogre3d.org/forums/viewtopic.php?f=4&t=61259
            if ( fLength > Real(0.0f) )
            {
                Real fInvLength = 1.0f / fLength;
                x *= fInvLength;
                y *= fInvLength;
                z *= fInvLength;
            }

            return fLength;
        }

        /** Calculates the cross-product of 2 vectors, i.e. the vector that
            lies perpendicular to them both.
            @remarks
                The cross-product is normally used to calculate the normal
                vector of a plane, by calculating the cross-product of 2
                non-equivalent vectors which lie on the plane (e.g. 2 edges
                of a triangle).
            @param rkVector
                Vector which, together with this one, will be used to
                calculate the cross-product.
            @return
                A vector which is the result of the cross-product. This
                vector will <b>NOT</b> be normalised, to maximise efficiency
                - call Vector3::normalise on the result if you wish this to
                be done. As for which side the resultant vector will be on, the
                returned vector will be on the side from which the arc from 'this'
                to rkVector is anticlockwise, e.g. UNIT_Y.crossProduct(UNIT_Z)
                = UNIT_X, whilst UNIT_Z.crossProduct(UNIT_Y) = -UNIT_X.
                This is because OGRE uses a right-handed coordinate system.
            @par
                For a clearer explanation, look a the left and the bottom edges
                of your monitor's screen. Assume that the first vector is the
                left edge and the second vector is the bottom edge, both of
                them starting from the lower-left corner of the screen. The
                resulting vector is going to be perpendicular to both of them
                and will go <i>inside</i> the screen, towards the cathode tube
                (assuming you're using a CRT monitor, of course).
        */
        inline Vector3 crossProduct( const Vector3& rkVector ) const
        {
            return Vector3(
                y * rkVector.z - z * rkVector.y,
                z * rkVector.x - x * rkVector.z,
                x * rkVector.y - y * rkVector.x);
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

        /** Returns true if the vector's scalar components are all greater
            that the ones of the vector it is compared against.
        */
        inline bool operator < ( const Vector3& rhs ) const
        {
            if( x < rhs.x && y < rhs.y && z < rhs.z )
                return true;
            return false;
        }

        /** Returns true if the vector's scalar components are all smaller
            that the ones of the vector it is compared against.
        */
        inline bool operator > ( const Vector3& rhs ) const
        {
            if( x > rhs.x && y > rhs.y && z > rhs.z )
                return true;
            return false;
        }

        /** Sets this vector's components to the minimum of its own and the
            ones of the passed in vector.
            @remarks
                'Minimum' in this case means the combination of the lowest
                value of x, y and z from both vectors. Lowest is taken just
                numerically, not magnitude, so -1 < 0.
        */
        inline void makeFloor( const Vector3& cmp )
        {
            if( cmp.x < x ) x = cmp.x;
            if( cmp.y < y ) y = cmp.y;
            if( cmp.z < z ) z = cmp.z;
        }

        /** Sets this vector's components to the maximum of its own and the
            ones of the passed in vector.
            @remarks
                'Maximum' in this case means the combination of the highest
                value of x, y and z from both vectors. Highest is taken just
                numerically, not magnitude, so 1 > -3.
        */
        inline void makeCeil( const Vector3& cmp )
        {
            if( cmp.x > x ) x = cmp.x;
            if( cmp.y > y ) y = cmp.y;
            if( cmp.z > z ) z = cmp.z;
        }

        /** Generates a vector perpendicular to this vector (eg an 'up' vector).
            @remarks
                This method will return a vector which is perpendicular to this
                vector. There are an infinite number of possibilities but this
                method will guarantee to generate one of them. If you need more
                control you should use the Quaternion class.
        */
        inline Vector3 perpendicular(void) const
        {
            // From Sam Hocevar's article "On picking an orthogonal
            // vector (and combing coconuts)"
            Vector3 perp = Math::Abs(x) > Math::Abs(z)
                         ? Vector3(-y, x, 0.0) : Vector3(0.0, -z, y);
            return perp.normalisedCopy();
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

        /** Gets the angle between 2 vectors.
        @remarks
            Vectors do not have to be unit-length but must represent directions.
        */
        inline Radian angleBetween(const Vector3& dest) const
        {
            Real lenProduct = length() * dest.length();

            // Divide by zero check
            if(lenProduct < 1e-6f)
                lenProduct = 1e-6f;

            Real f = dotProduct(dest) / lenProduct;

            f = Math::Clamp(f, (Real)-1.0, (Real)1.0);
            return Math::ACos(f);

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

        /** Returns true if this vector is zero length. */
        inline bool isZeroLength(void) const
        {
            Real sqlen = (x * x) + (y * y) + (z * z);
            return (sqlen < (1e-06 * 1e-06));

        }

        /** As normalise, except that this vector is unaffected and the
            normalised vector is returned as a copy. */
        inline Vector3 normalisedCopy(void) const
        {
            Vector3 ret = *this;
            ret.normalise();
            return ret;
        }

        /** Calculates a reflection vector to the plane with the given normal .
        @remarks NB assumes 'this' is pointing AWAY FROM the plane, invert if it is not.
        */
        inline Vector3 reflect(const Vector3& normal) const
        {
            return Vector3( *this - ( 2 * this->dotProduct(normal) * normal ) );
        }

        /** Returns whether this vector is within a positional tolerance
            of another vector.
        @param rhs The vector to compare with
        @param tolerance The amount that each element of the vector may vary by
            and still be considered equal
        */
        inline bool positionEquals(const Vector3& rhs, Real tolerance = 1e-03) const
        {
            return Math::RealEqual(x, rhs.x, tolerance) &&
                Math::RealEqual(y, rhs.y, tolerance) &&
                Math::RealEqual(z, rhs.z, tolerance);

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

        /// Check whether this vector contains valid values
        inline bool isNaN() const
        {
            return Math::isNaN(x) || Math::isNaN(y) || Math::isNaN(z);
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

        /** Function for writing to a stream.
        */
        inline friend std::ostream& operator <<
            ( std::ostream& o, const Vector3& v )
        {
            o << "Vector3(" << v.x << ", " << v.y << ", " << v.z << ")";
            return o;
        }
    };
    /** @} */
    /** @} */

}
#endif
