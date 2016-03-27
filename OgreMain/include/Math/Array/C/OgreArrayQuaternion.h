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
#ifndef __C_ArrayQuaternion_H__
#define __C_ArrayQuaternion_H__

#ifndef __ArrayQuaternion_H__
    #error "Don't include this file directly. include Math/Array/OgreArrayQuaternion.h"
#endif

#include "OgreQuaternion.h"

#include "Math/Array/OgreMathlib.h"
#include "Math/Array/OgreArrayVector3.h"

#include "OgreArrayQuaternion.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Math
    *  @{
    */
    /** Cache-friendly array of Quaternion represented as a SoA array.
        @remarks
            ArrayQuaternion is a SIMD & cache-friendly version of Quaternion.
            An operation on an ArrayQuaternion is done on 4 quaternions at a
            time (the actual amount is defined by ARRAY_PACKED_REALS)
            Assuming ARRAY_PACKED_REALS == 4, the memory layout will
            be as following:
               mChunkBase             mChunkBase + 4
            WWWW XXXX YYYY ZZZZ     WWWW XXXX YYYY ZZZZ
            Extracting one quat (XYZW) needs 64 bytes, which is within
            the 64 byte size of common cache lines.
            Architectures where the cache line == 32 bytes may want to
            set ARRAY_PACKED_REALS = 2 depending on their needs
    */

    class _OgreExport ArrayQuaternion
    {
    public:
        Real w, x, y, z;

        ArrayQuaternion() {}
        ArrayQuaternion( const ArrayReal &chunkW, const ArrayReal &chunkX,
                                const ArrayReal &chunkY, const ArrayReal &chunkZ )
        {
            w = chunkW;
            x = chunkX;
            y = chunkY;
            z = chunkZ;
        }

        void getAsQuaternion( Quaternion &out, size_t index ) const
        {
            //Be careful of not writing to these regions or else strict aliasing rule gets broken!!!
            const Real *aliasedReal = reinterpret_cast<const Real*>( &w );
            out.w = aliasedReal[ARRAY_PACKED_REALS * 0 + index];        //W
            out.x = aliasedReal[ARRAY_PACKED_REALS * 1 + index];        //X
            out.y = aliasedReal[ARRAY_PACKED_REALS * 2 + index];        //Y
            out.z = aliasedReal[ARRAY_PACKED_REALS * 3 + index];        //Z
        }

        /// Prefer using @see getAsQuaternion() because this function may have more
        /// overhead (the other one is faster)
        Quaternion getAsQuaternion( size_t index ) const
        {
            //Be careful of not writing to these regions or else strict aliasing rule gets broken!!!
            const Real *aliasedReal = reinterpret_cast<const Real*>( &w );
            return Quaternion( aliasedReal[ARRAY_PACKED_REALS * 0 + index], //W
                            aliasedReal[ARRAY_PACKED_REALS * 1 + index],        //X
                            aliasedReal[ARRAY_PACKED_REALS * 2 + index],        //Y
                            aliasedReal[ARRAY_PACKED_REALS * 3 + index] );  //Z
        }

        void setFromQuaternion( const Quaternion &v, size_t index )
        {
            Real *aliasedReal = reinterpret_cast<Real*>( &w );
            aliasedReal[ARRAY_PACKED_REALS * 0 + index] = v.w;
            aliasedReal[ARRAY_PACKED_REALS * 1 + index] = v.x;
            aliasedReal[ARRAY_PACKED_REALS * 2 + index] = v.y;
            aliasedReal[ARRAY_PACKED_REALS * 3 + index] = v.z;
        }
        
        void setAll( const Quaternion &v )
        {
            w = v.w;
            x = v.x;
            y = v.y;
            z = v.z;
        }

        /** @see Quaternion::FromRotationMatrix
            This code assumes that:
                Quaternion is orthogonal
                Determinant of quaternion is 1.
        @param matrix
            9-element matrix (3x3)
        */
        inline void FromOrthoDet1RotationMatrix( const ArrayReal * RESTRICT_ALIAS matrix );

        /// @copydoc Quaternion::FromAngleAxis
        inline void FromAngleAxis( const ArrayRadian& rfAngle, const ArrayVector3& rkAxis );

        /// @copydoc Quaternion::ToAngleAxis
        inline void ToAngleAxis( ArrayRadian &rfAngle, ArrayVector3 &rkAxis ) const;

        inline friend ArrayQuaternion operator * ( const ArrayQuaternion &lhs, const ArrayQuaternion &rhs );

        inline friend ArrayQuaternion operator + ( const ArrayQuaternion &lhs, const ArrayQuaternion &rhs );
        inline friend ArrayQuaternion operator - ( const ArrayQuaternion &lhs, const ArrayQuaternion &rhs );
        inline friend ArrayQuaternion operator * ( const ArrayQuaternion &lhs, ArrayReal scalar );
        inline friend ArrayQuaternion operator * ( ArrayReal scalar, const ArrayQuaternion &lhs );
        inline void operator += ( const ArrayQuaternion &a );
        inline void operator -= ( const ArrayQuaternion &a );
        inline void operator *= ( const ArrayReal fScalar );

        /// @copydoc Quaternion::xAxis
        inline ArrayVector3 xAxis( void ) const;
        /// @copydoc Quaternion::yAxis
        inline ArrayVector3 yAxis( void ) const;
        /// @copydoc Quaternion::zAxis
        inline ArrayVector3 zAxis( void ) const;

        /// @copydoc Quaternion::Dot
        inline ArrayReal Dot( const ArrayQuaternion& rkQ ) const;

        /// @copydoc Quaternion::Norm
        inline ArrayReal Norm( void ) const; //Returns the squared length, doesn't modify

        /// Unlike Quaternion::normalise(), this function does not return the length of the vector
        /// because such value was not cached and was never available @see Quaternion::normalise()
        inline void normalise( void );

        inline ArrayQuaternion Inverse( void ) const;       // apply to non-zero quaternion
        inline ArrayQuaternion UnitInverse( void ) const;   // apply to unit-length quaternion
        inline ArrayQuaternion Exp( void ) const;
        inline ArrayQuaternion Log( void ) const;

        /// Rotation of a vector by a quaternion
        inline ArrayVector3 operator * ( const ArrayVector3 &v ) const;

        /** Rotates a vector by multiplying the quaternion to the vector, and modifies it's contents
            by storing the results there.
            @remarks
                This function is the same as doing:
                    ArrayVector v;
                    ArrayQuaternion q;
                    v = q * v;
                In fact, the operator overloading will make above code work perfectly. However, because
                we don't trust all compilers in optimizing this performance-sensitive function (in fact
                MSVC 2008 doesn't inline the op. and generates an unnecessary ArrayVector3) this
                function will take the input vector, and store the results back on that vector.
                This is very common when concatenating transformations on an ArrayVector3, whose
                memory reside in the heap (it makes better usage of the memory). Long story short,
                prefer calling this function to using an operator when just updating an ArrayVector3 is
                involved. (It's fine using operators for ArrayVector3s)
            @param
                
        */
        static inline void mul( const ArrayQuaternion &inQ, ArrayVector3 &inOutVec );

        /// @See Quaternion::Slerp
        /// @remarks
        ///     shortestPath is always true
        static inline ArrayQuaternion Slerp( ArrayReal fT, const ArrayQuaternion &rkP,
                                                    const ArrayQuaternion &rkQ );

        /// @See Quaternion::nlerp
        /// @remarks
        ///     shortestPath is always true
        static inline ArrayQuaternion nlerpShortest( ArrayReal fT, const ArrayQuaternion& rkP,
                                                    const ArrayQuaternion& rkQ );

        /// @See Quaternion::nlerp
        /// @remarks
        ///     shortestPath is always false
        static inline ArrayQuaternion nlerp( ArrayReal fT, const ArrayQuaternion& rkP, 
                                                    const ArrayQuaternion& rkQ );

        /** Conditional move update. @See MathlibC::Cmov4
            Changes each of the four vectors contained in 'this' with
            the replacement provided
            @remarks
                If mask param contains anything other than 0's or 0xffffffff's
                the result is undefined.
                Use this version if you want to decide whether to keep current
                result or overwrite with a replacement (performance optimization).
                i.e. a = Cmov4( a, b )
                If this vector hasn't been assigned yet any value and want to
                decide between two ArrayQuaternions, i.e. a = Cmov4( b, c ) then
                @see Cmov4( const ArrayQuaternion &arg1, const ArrayQuaternion &arg2, ArrayReal mask );
                instead.
            @param
                Vectors to be used as replacement if the mask is zero.
            @param
                mask filled with either 0's or 0xFFFFFFFF
            @return
                this[i] = mask[i] != 0 ? this[i] : replacement[i]
        */
        inline void Cmov4( ArrayMaskR mask, const ArrayQuaternion &replacement );

        /** Conditional move. @See MathlibC::Cmov4
            Selects between arg1 & arg2 according to mask
            @remarks
                If mask param contains anything other than 0's or 0xffffffff's
                the result is undefined.
                If you wanted to do a = cmov4( a, b ), then consider using the update version
                @see Cmov4( ArrayReal mask, const ArrayQuaternion &replacement );
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
        inline static ArrayQuaternion Cmov4( const ArrayQuaternion &arg1, const ArrayQuaternion &arg2, ArrayMaskR mask );

        static const ArrayQuaternion ZERO;
        static const ArrayQuaternion IDENTITY;
    };
    /** @} */
    /** @} */

}

#include "OgreArrayQuaternion.inl"

#endif
