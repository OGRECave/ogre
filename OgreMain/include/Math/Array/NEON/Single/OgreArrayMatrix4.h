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
#ifndef __NEON_ArrayMatrix4_H__
#define __NEON_ArrayMatrix4_H__

#ifndef __ArrayMatrix4_H__
    #error "Don't include this file directly. include Math/Array/OgreArrayMatrix4.h"
#endif

#include "OgreMatrix4.h"

#include "Math/Array/OgreMathlib.h"
#include "Math/Array/OgreArrayVector3.h"
#include "Math/Array/OgreArrayQuaternion.h"

namespace Ogre
{
    class SimpleMatrix4;

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Math
    *  @{
    */
    /** Cache-friendly container of 4x4 matrices represented as a SoA array.
        @remarks
            ArrayMatrix4 is a SIMD & cache-friendly version of Matrix4.
            An operation on an ArrayMatrix4 is done on 4 vectors at a
            time (the actual amount is defined by ARRAY_PACKED_REALS)
            Assuming ARRAY_PACKED_REALS == 4, the memory layout will
            be as following:
             mChunkBase mChunkBase + 3
             a00b00c00d00    a01b01c01d01
            Extracting one Matrix4 needs 256 bytes, which needs 4 line
            fetches for common cache lines of 64 bytes.
            Make sure extractions are made sequentially to avoid cache
            trashing and excessive bandwidth consumption, and prefer
            working on @See ArrayVector3 & @See ArrayQuaternion instead
            Architectures where the cache line == 32 bytes may want to
            set ARRAY_PACKED_REALS = 2 depending on their needs
    */

    class _OgreExport ArrayMatrix4
    {
    public:
        ArrayReal       mChunkBase[16];

        ArrayMatrix4() {}
        ArrayMatrix4( const ArrayMatrix4 &copy )
        {
            //Using a loop minimizes instruction count (better i-cache)
            //Doing 4 at a time per iteration maximizes instruction pairing
            //Unrolling the whole loop is i-cache unfriendly and
            //becomes unmaintainable (16 lines!?)
            for( size_t i=0; i<16; i+=4 )
            {
                mChunkBase[i  ] = copy.mChunkBase[i  ];
                mChunkBase[i+1] = copy.mChunkBase[i+1];
                mChunkBase[i+2] = copy.mChunkBase[i+2];
                mChunkBase[i+3] = copy.mChunkBase[i+3];
            }
        }

        void getAsMatrix4( Matrix4 &out, size_t index ) const
        {
            //Be careful of not writing to these regions or else strict aliasing rule gets broken!!!
            const Real * RESTRICT_ALIAS aliasedReal = reinterpret_cast<const Real*>( mChunkBase );
            Real * RESTRICT_ALIAS matrix = reinterpret_cast<Real*>( out._m );
            for( size_t i=0; i<16; i+=4 )
            {
                matrix[i  ] = aliasedReal[ARRAY_PACKED_REALS * (i  ) + index];
                matrix[i+1] = aliasedReal[ARRAY_PACKED_REALS * (i+1) + index];
                matrix[i+2] = aliasedReal[ARRAY_PACKED_REALS * (i+2) + index];
                matrix[i+3] = aliasedReal[ARRAY_PACKED_REALS * (i+3) + index];
            }
        }

        /// STRONGLY Prefer using @see getAsMatrix4() because this function may have more
        /// overhead (the other one is faster)
        Matrix4 getAsMatrix4( size_t index ) const
        {
            Matrix4 retVal;
            getAsMatrix4( retVal, index );

            return retVal;
        }

        void setFromMatrix4( const Matrix4 &m, size_t index )
        {
            Real * RESTRICT_ALIAS aliasedReal = reinterpret_cast<Real*>( mChunkBase );
            const Real * RESTRICT_ALIAS matrix = reinterpret_cast<const Real*>( m._m );
            for( size_t i=0; i<16; i+=4 )
            {
                aliasedReal[ARRAY_PACKED_REALS * (i  ) + index] = matrix[i  ];
                aliasedReal[ARRAY_PACKED_REALS * (i+1) + index] = matrix[i+1];
                aliasedReal[ARRAY_PACKED_REALS * (i+2) + index] = matrix[i+2];
                aliasedReal[ARRAY_PACKED_REALS * (i+3) + index] = matrix[i+3];
            }
        }

        /// Sets all packed matrices to the same value as the scalar input matrix
        void setAll( const Matrix4 &m )
        {
            mChunkBase[0]  = vdupq_n_f32( m._m[0] );
            mChunkBase[1]  = vdupq_n_f32( m._m[1] );
            mChunkBase[2]  = vdupq_n_f32( m._m[2] );
            mChunkBase[3]  = vdupq_n_f32( m._m[3] );
            mChunkBase[4]  = vdupq_n_f32( m._m[4] );
            mChunkBase[5]  = vdupq_n_f32( m._m[5] );
            mChunkBase[6]  = vdupq_n_f32( m._m[6] );
            mChunkBase[7]  = vdupq_n_f32( m._m[7] );
            mChunkBase[8]  = vdupq_n_f32( m._m[8] );
            mChunkBase[9]  = vdupq_n_f32( m._m[9] );
            mChunkBase[10] = vdupq_n_f32( m._m[10] );
            mChunkBase[11] = vdupq_n_f32( m._m[11] );
            mChunkBase[12] = vdupq_n_f32( m._m[12] );
            mChunkBase[13] = vdupq_n_f32( m._m[13] );
            mChunkBase[14] = vdupq_n_f32( m._m[14] );
            mChunkBase[15] = vdupq_n_f32( m._m[15] );
        }

        static ArrayMatrix4 createAllFromMatrix4( const Matrix4 &m )
        {
            ArrayMatrix4 retVal;
            retVal.mChunkBase[0]  = vdupq_n_f32( m._m[0] );
            retVal.mChunkBase[1]  = vdupq_n_f32( m._m[1] );
            retVal.mChunkBase[2]  = vdupq_n_f32( m._m[2] );
            retVal.mChunkBase[3]  = vdupq_n_f32( m._m[3] );
            retVal.mChunkBase[4]  = vdupq_n_f32( m._m[4] );
            retVal.mChunkBase[5]  = vdupq_n_f32( m._m[5] );
            retVal.mChunkBase[6]  = vdupq_n_f32( m._m[6] );
            retVal.mChunkBase[7]  = vdupq_n_f32( m._m[7] );
            retVal.mChunkBase[8]  = vdupq_n_f32( m._m[8] );
            retVal.mChunkBase[9]  = vdupq_n_f32( m._m[9] );
            retVal.mChunkBase[10] = vdupq_n_f32( m._m[10] );
            retVal.mChunkBase[11] = vdupq_n_f32( m._m[11] );
            retVal.mChunkBase[12] = vdupq_n_f32( m._m[12] );
            retVal.mChunkBase[13] = vdupq_n_f32( m._m[13] );
            retVal.mChunkBase[14] = vdupq_n_f32( m._m[14] );
            retVal.mChunkBase[15] = vdupq_n_f32( m._m[15] );
            return retVal;
        }

        /** Assigns the value of the other matrix. Does not reference the
            ptr address, but rather perform a memory copy
            @param
                rkmatrix The other matrix
        */
        inline ArrayMatrix4& operator = ( const ArrayMatrix4& rkMatrix )
        {
            for( size_t i=0; i<16; i+=4 )
            {
                mChunkBase[i  ] = rkMatrix.mChunkBase[i  ];
                mChunkBase[i+1] = rkMatrix.mChunkBase[i+1];
                mChunkBase[i+2] = rkMatrix.mChunkBase[i+2];
                mChunkBase[i+3] = rkMatrix.mChunkBase[i+3];
            }
            return *this;
        }

        // Concatenation
        inline friend ArrayMatrix4 operator * ( const ArrayMatrix4 &lhs, const ArrayMatrix4 &rhs );

        inline ArrayVector3 operator * ( const ArrayVector3 &rhs ) const;

        /// Prefer the update version 'a *= b' A LOT over 'a = a * b'
        /// (copying from an ArrayMatrix4 is 256 bytes!)
        inline void operator *= ( const ArrayMatrix4 &rhs );

        /** Converts the given quaternion to a 3x3 matrix representation and fill our values
            @remarks
                Similar to @see Quaternion::ToRotationMatrix, this function will take the input
                quaternion and overwrite the first 3x3 subset of this matrix. The 4th row &
                columns are left untouched.
                This function is defined in ArrayMatrix4 to avoid including this header into
                ArrayQuaternion. The idea is that ArrayMatrix4 requires ArrayQuaternion, and
                ArrayQuaternion requires ArrayVector3. Simple dependency order
            @param
                The quaternion to convert from.
        */
        inline void fromQuaternion( const ArrayQuaternion &q );

        /// @copydoc Matrix4::makeTransform()
        inline void makeTransform( const ArrayVector3 &position, const ArrayVector3 &scale,
                                    const ArrayQuaternion &orientation );

        /** Converts these matrices contained in this ArrayMatrix to AoS form and stores them in dst
        @remarks
            'dst' must be aligned and assumed to have enough memory for ARRAY_PACKED_REALS matrices
        */
        inline void storeToAoS( Matrix4 * RESTRICT_ALIAS dst ) const;

        /** Converts ARRAY_PACKED_REALS matrices into this ArrayMatrix
        @remarks
            'src' must be aligned and assumed to have enough memory for ARRAY_PACKED_REALS matrices
        */
        inline void loadFromAoS( const Matrix4 * RESTRICT_ALIAS src );
        inline void loadFromAoS( const SimpleMatrix4 * RESTRICT_ALIAS src );

        /// @copydoc Matrix4::isAffine()
        inline bool isAffine() const;

        static const ArrayMatrix4 IDENTITY;
    };

    /** Simple wrap up to load an AoS matrix 4x4 using SSE. The main reason of this class
        is to force MSVC to use 4 movaps to load arrays of Matrix4s (which are waaay more
        efficient that whatever lea+mov junk it tries to produce)
    */
    class _OgreExport SimpleMatrix4
    {
    public:
        ArrayReal       mChunkBase[4];

        /// Assumes src is aligned
        void load( const Matrix4 &src )
        {
            mChunkBase[0] = vld1q_f32( src._m );
            mChunkBase[1] = vld1q_f32( src._m+4 );
            mChunkBase[2] = vld1q_f32( src._m+8 );
            mChunkBase[3] = vld1q_f32( src._m+12 );
        }
    };

    /** @} */
    /** @} */

}

#include "OgreArrayMatrix4.inl"

#endif
