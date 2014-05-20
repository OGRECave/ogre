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
#ifndef __NEON_ArrayVector3_H__
#define __NEON_ArrayVector3_H__

#ifndef __ArrayVector3_H__
    #error "Don't include this file directly. include Math/Array/OgreArrayVector3.h"
#endif

#include "OgreVector3.h"

#include "Math/Array/OgreMathlib.h"

namespace Ogre
{
    class ArrayInterQuaternion;

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Math
    *  @{
    */
    /** Cache-friendly array of 3-dimensional represented as a SoA array.
        @remarks
            ArrayVector3 is a SIMD & cache-friendly version of Vector3.
            An operation on an ArrayVector3 is done on 4 vectors at a
            time (the actual amount is defined by ARRAY_PACKED_REALS)
            Assuming ARRAY_PACKED_REALS == 4, the memory layout will
            be as following:
             mChunkBase     mChunkBase + 3
            XXXX YYYY ZZZZ      XXXX YYYY ZZZZ
            Extracting one vector (XYZ) needs 48 bytes, which is within
            the 64 byte size of common cache lines.
            Architectures where the cache line == 32 bytes may want to
            set ARRAY_PACKED_REALS = 2 depending on their needs
    */

    class _OgreExport ArrayVector3
    {
    public:
        ArrayReal       mChunkBase[3];

        ArrayVector3() {}
        ArrayVector3( ArrayReal chunkX, ArrayReal chunkY, ArrayReal chunkZ )
        {
            mChunkBase[0] = chunkX;
            mChunkBase[1] = chunkY;
            mChunkBase[2] = chunkZ;
        }

        void getAsVector3( Vector3 &out, size_t index ) const
        {
            //Be careful of not writing to these regions or else strict aliasing rule gets broken!!!
            const Real *aliasedReal = reinterpret_cast<const Real*>( mChunkBase );
            out.x = aliasedReal[ARRAY_PACKED_REALS * 0 + index];        //X
            out.y = aliasedReal[ARRAY_PACKED_REALS * 1 + index];        //Y
            out.z = aliasedReal[ARRAY_PACKED_REALS * 2 + index];        //Z
        }

        /// Prefer using @see getAsVector3() because this function may have more
        /// overhead (the other one is faster)
        Vector3 getAsVector3( size_t index ) const
        {
            //Be careful of not writing to these regions or else strict aliasing rule gets broken!!!
            const Real *aliasedReal = reinterpret_cast<const Real*>( mChunkBase );
            return Vector3( aliasedReal[ARRAY_PACKED_REALS * 0 + index],        //X
                            aliasedReal[ARRAY_PACKED_REALS * 1 + index],        //Y
                            aliasedReal[ARRAY_PACKED_REALS * 2 + index] );  //Z
        }

        void setFromVector3( const Vector3 &v, size_t index )
        {
            Real *aliasedReal = reinterpret_cast<Real*>( mChunkBase );
            aliasedReal[ARRAY_PACKED_REALS * 0 + index] = v.x;
            aliasedReal[ARRAY_PACKED_REALS * 1 + index] = v.y;
            aliasedReal[ARRAY_PACKED_REALS * 2 + index] = v.z;
        }

        /// Sets all packed vectors to the same value as the scalar input vector
        void setAll( const Vector3 &v )
        {
            mChunkBase[0] = vdupq_n_f32( v.x );
            mChunkBase[1] = vdupq_n_f32( v.y );
            mChunkBase[2] = vdupq_n_f32( v.z );
        }

        /// Copies only one vector, by looking at the indexes
        /*void copyScalar( size_t ourIndex, const ArrayVector3 &copy, size_t copyIndex )
        {
            Vector3 tmp;
            copy.getAsVector3( tmp );
            this->setFromVector3( tmp );
        }*/

        inline ArrayVector3& operator = ( const Real fScalar )
        {
            //set1_ps is a composite instrinsic using shuffling instructions.
            //Store the actual result in a tmp variable and copy. We don't
            //do mChunkBase[1] = mChunkBase[0]; because of a potential LHS
            //depending on how smart the compiler was
            ArrayReal tmp = vdupq_n_f32( fScalar );
            mChunkBase[0] = tmp;
            mChunkBase[1] = tmp;
            mChunkBase[2] = tmp;

            return *this;
        }

        // Arithmetic operations
        inline const ArrayVector3& operator + () const;
        inline ArrayVector3 operator - () const;

        inline friend ArrayVector3 operator + ( const ArrayVector3 &lhs, const ArrayVector3 &rhs );
        inline friend ArrayVector3 operator + ( Real fScalar, const ArrayVector3 &rhs );
        inline friend ArrayVector3 operator + ( const ArrayVector3 &lhs, Real fScalar );

        inline friend ArrayVector3 operator + ( ArrayReal fScalar, const ArrayVector3 &rhs );
        inline friend ArrayVector3 operator + ( const ArrayVector3 &lhs, ArrayReal fScalar );

        inline friend ArrayVector3 operator - ( const ArrayVector3 &lhs, const ArrayVector3 &rhs );
        inline friend ArrayVector3 operator - ( Real fScalar, const ArrayVector3 &rhs );
        inline friend ArrayVector3 operator - ( const ArrayVector3 &lhs, Real fScalar );

        inline friend ArrayVector3 operator - ( ArrayReal fScalar, const ArrayVector3 &rhs );
        inline friend ArrayVector3 operator - ( const ArrayVector3 &lhs, ArrayReal fScalar );

        inline friend ArrayVector3 operator * ( const ArrayVector3 &lhs, const ArrayVector3 &rhs );
        inline friend ArrayVector3 operator * ( Real fScalar, const ArrayVector3 &rhs );
        inline friend ArrayVector3 operator * ( const ArrayVector3 &lhs, Real fScalar );

        inline friend ArrayVector3 operator * ( ArrayReal fScalar, const ArrayVector3 &rhs );
        inline friend ArrayVector3 operator * ( const ArrayVector3 &lhs, ArrayReal fScalar );

        inline friend ArrayVector3 operator / ( const ArrayVector3 &lhs, const ArrayVector3 &rhs );
        inline friend ArrayVector3 operator / ( Real fScalar, const ArrayVector3 &rhs );
        inline friend ArrayVector3 operator / ( const ArrayVector3 &lhs, Real fScalar );

        inline friend ArrayVector3 operator / ( ArrayReal fScalar, const ArrayVector3 &rhs );
        inline friend ArrayVector3 operator / ( const ArrayVector3 &lhs, ArrayReal fScalar );

        inline void operator += ( const ArrayVector3 &a );
        inline void operator += ( const Real fScalar );
        inline void operator += ( const ArrayReal fScalar );

        inline void operator -= ( const ArrayVector3 &a );
        inline void operator -= ( const Real fScalar );
        inline void operator -= ( const ArrayReal fScalar );

        inline void operator *= ( const ArrayVector3 &a );
        inline void operator *= ( const Real fScalar );
        inline void operator *= ( const ArrayReal fScalar );

        inline void operator /= ( const ArrayVector3 &a );
        inline void operator /= ( const Real fScalar );
        inline void operator /= ( const ArrayReal fScalar );

        /// @copydoc Vector3::length()
        inline ArrayReal length() const;

        /// @copydoc Vector3::squaredLength()
        inline ArrayReal squaredLength() const;

        /// @copydoc Vector3::distance()
        inline ArrayReal distance( const ArrayVector3& rhs ) const;

        /// @copydoc Vector3::squaredDistance()
        inline ArrayReal squaredDistance( const ArrayVector3& rhs ) const;

        /// @copydoc Vector3::dotProduct()
        inline ArrayReal dotProduct( const ArrayVector3& vec ) const;

        /// @copydoc Vector3::absDotProduct()
        inline ArrayReal absDotProduct( const ArrayVector3& vec ) const;

        /// Unlike Vector3::normalise(), this function does not return the length of the vector
        /// because such value was not cached and was never available @see Vector3::normalise()
        inline void normalise( void );

        /// @copydoc Vector3::crossProduct()
        inline ArrayVector3 crossProduct( const ArrayVector3& rkVector ) const;

        /// @copydoc Vector3::midPoint()
        inline ArrayVector3 midPoint( const ArrayVector3& vec ) const;

        /// @copydoc Vector3::makeFloor()
        inline void makeFloor( const ArrayVector3& cmp );

        /// @copydoc Vector3::makeCeil()
        inline void makeCeil( const ArrayVector3& cmp );

        /// Returns the smallest value between x, y, z; min( x, y, z )
        inline ArrayReal getMinComponent() const;

        /// Returns the biggest value between x, y, z; max( x, y, z )
        inline ArrayReal getMaxComponent() const;

        /** Converts the vector to (sign(x), sign(y), sign(z))
        @remarks
            For reference, sign( x ) = x >= 0 ? 1.0 : -1.0
            sign( -0.0f ) may return 1 or -1 depending on implementation
            @par
            SSE2 implementation: Does distinguish between -0 & 0
            C implementation: Does not distinguish between -0 & 0
        */
        inline void setToSign();

        /// @copydoc Vector3::perpendicular()
        inline ArrayVector3 perpendicular( void ) const;

        /// @copydoc Vector3::normalisedCopy()
        inline ArrayVector3 normalisedCopy( void ) const;

        /// @copydoc Vector3::reflect()
        inline ArrayVector3 reflect( const ArrayVector3& normal ) const;

        /** Calculates the inverse of the vectors: 1.0f / v;
         But if original is zero, the zero is left (0 / 0 = 0).
         Example:
         Bfore inverseLeaveZero:
         x = 0; y = 2; z = 3;
         After inverseLeaveZero
         x = 0; y = 0.5; z = 0.3333;
         */
        inline void inverseLeaveZeroes( void );

        /// @see Vector3::isNaN()
        /// @return
        ///     Return value differs from Vector3's counterpart. We return an int
        ///     bits 0-4 are set for each NaN of each vector inside.
        ///     if the int is non-zero, there is a NaN.
        inline int isNaN( void ) const;

        /// @copydoc Vector3::primaryAxis()
        inline ArrayVector3 primaryAxis( void ) const;

        /** Takes each Vector and returns one returns a single vector
        @remarks
            This is useful when calculating bounding boxes, since it can be done independently
            in SIMD form, and once it is done, merge the results from the simd vectors into one
        @return
            Vector.x = min( vector[0].x, vector[1].x, vector[2].x, vector[3].x )
            Vector.y = min( vector[0].y, vector[1].y, vector[2].y, vector[3].y )
            Vector.z = min( vector[0].z, vector[1].z, vector[2].z, vector[3].z )
        */
        inline Vector3 collapseMin( void ) const;

        /** Takes each Vector and returns one returns a single vector
        @remarks
            This is useful when calculating bounding boxes, since it can be done independently
            in SIMD form, and once it is done, merge the results from the simd vectors into one
        @return
            Vector.x = max( vector[0].x, vector[1].x, vector[2].x, vector[3].x )
            Vector.y = max( vector[0].y, vector[1].y, vector[2].y, vector[3].y )
            Vector.z = max( vector[0].z, vector[1].z, vector[2].z, vector[3].z )
        */
        inline Vector3 collapseMax( void ) const;

        /** Conditional move update. @See MathlibNEON::Cmov4
            Changes each of the four vectors contained in 'this' with
            the replacement provided
            @remarks
                If mask param contains anything other than 0's or 0xffffffff's
                the result is undefined.
                Use this version if you want to decide whether to keep current
                result or overwrite with a replacement (performance optimization).
                i.e. a = Cmov4( a, b )
                If this vector hasn't been assigned yet any value and want to
                decide between two ArrayVector3s, i.e. a = Cmov4( b, c ) then
                @see Cmov4( const ArrayVector3 &arg1, const ArrayVector3 &arg2, ArrayReal mask );
                instead.
            @param
                Vectors to be used as replacement if the mask is zero.
            @param
                mask filled with either 0's or 0xFFFFFFFF
            @return
                this[i] = mask[i] != 0 ? this[i] : replacement[i]
        */
        inline void Cmov4( ArrayMaskR mask, const ArrayVector3 &replacement );

        /** Conditional move update. @See MathlibNEON::CmovRobust
            Changes each of the four vectors contained in 'this' with
            the replacement provided
            @remarks
                If mask param contains anything other than 0's or 0xffffffff's
                the result is undefined.
                Use this version if you want to decide whether to keep current
                result or overwrite with a replacement (performance optimization).
                i.e. a = CmovRobust( a, b )
                If this vector hasn't been assigned yet any value and want to
                decide between two ArrayVector3s, i.e. a = Cmov4( b, c ) then
                @see Cmov4( const ArrayVector3 &arg1, const ArrayVector3 &arg2, ArrayReal mask );
                instead.
            @param
                Vectors to be used as replacement if the mask is zero.
            @param
                mask filled with either 0's or 0xFFFFFFFF
            @return
                this[i] = mask[i] != 0 ? this[i] : replacement[i]
        */
        inline void CmovRobust( ArrayMaskR mask, const ArrayVector3 &replacement );

        /** Conditional move. @See MathlibNEON::Cmov4
            Selects between arg1 & arg2 according to mask
            @remarks
                If mask param contains anything other than 0's or 0xffffffff's
                the result is undefined.
                If you wanted to do a = cmov4( a, b ), then consider using the update version
                @see Cmov4( ArrayReal mask, const ArrayVector3 &replacement );
                instead.
            @param
                First array of Vectors
            @param
                Second array of Vectors
            @param
                mask filled with either 0's or 0xFFFFFFFF
            @return
                this[i] = mask[i] != 0 ? arg1[i] : arg2[i]
        */
        inline static ArrayVector3 Cmov4( const ArrayVector3 &arg1, const ArrayVector3 &arg2, ArrayMaskR mask );

		/** Converts 4 ARRAY_PACKED_REALS reals into this ArrayVector3
        @remarks
            'src' must be aligned and assumed to have enough memory for ARRAY_PACKED_REALS Vector3
            i.e. on SSE2 you can construct src as:
                OGRE_ALIGNED_DECL( Real, vals[ARRAY_PACKED_REALS * 4], OGRE_SIMD_ALIGNMENT ) =
                {
                    x0, y0, z0, 0,
                    x1, y1, z1, 0,
                    x2, y2, z2, 0,
                    x3, y3, z3, 0,
                }
            @See Frustum::getCustomWorldSpaceCorners implementation for an actual, advanced use case.
        */
        inline void loadFromAoS( const Real * RESTRICT_ALIAS src );

        static const ArrayVector3 ZERO;
        static const ArrayVector3 UNIT_X;
        static const ArrayVector3 UNIT_Y;
        static const ArrayVector3 UNIT_Z;
        static const ArrayVector3 NEGATIVE_UNIT_X;
        static const ArrayVector3 NEGATIVE_UNIT_Y;
        static const ArrayVector3 NEGATIVE_UNIT_Z;
        static const ArrayVector3 UNIT_SCALE;
    };
    /** @} */
    /** @} */

}

#include "OgreArrayVector3.inl"

#endif
