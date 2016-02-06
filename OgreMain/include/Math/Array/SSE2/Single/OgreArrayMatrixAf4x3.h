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
#ifndef _SSE2_ArrayMatrixAf4x3_H_
#define _SSE2_ArrayMatrixAf4x3_H_

#ifndef _ArrayMatrixAf4x3_H_
    #error "Don't include this file directly. include Math/Array/OgreArrayMatrix4.h"
#endif

#include "OgreMatrix4.h"

#include "Math/Array/OgreMathlib.h"
#include "Math/Array/OgreArrayVector3.h"
#include "Math/Array/OgreArrayQuaternion.h"

namespace Ogre
{
    class SimpleMatrixAf4x3;

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Math
    *  @{
    */
    /** Cache-friendly container of AFFINE 4x4 matrices represented as a SoA array.
        @remarks
            ArrayMatrix4 is a SIMD & cache-friendly version of Matrix4.
            An operation on an ArrayMatrix4 is done on 4 vectors at a
            time (the actual amount is defined by ARRAY_PACKED_REALS)
            Assuming ARRAY_PACKED_REALS == 4, the memory layout will
            be as following:
              mChunkBase     mChunkBase + 4
             a00b00c00d00    a01b01c01d01
            Extracting one Matrix4 needs 256 bytes, which needs 4 line
            fetches for common cache lines of 64 bytes.
            Make sure extractions are made sequentially to avoid cache
            trashing and excessive bandwidth consumption, and prefer
            working on @See ArrayVector3 & @See ArrayQuaternion instead
            Architectures where the cache line == 32 bytes may want to
            set ARRAY_PACKED_REALS = 2 depending on their needs
    */

    class _OgreExport ArrayMatrixAf4x3
    {
    public:
        ArrayReal       mChunkBase[12];

        ArrayMatrixAf4x3() {}
        ArrayMatrixAf4x3( const ArrayMatrixAf4x3 &copy )
        {
            //Using a loop minimizes instruction count (better i-cache)
            //Doing 4 at a time per iteration maximizes instruction pairing
            //Unrolling the whole loop is i-cache unfriendly and
            //becomes unmaintainable (16 lines!?)
            for( size_t i=0; i<12; i+=4 )
            {
                mChunkBase[i  ] = copy.mChunkBase[i  ];
                mChunkBase[i+1] = copy.mChunkBase[i+1];
                mChunkBase[i+2] = copy.mChunkBase[i+2];
                mChunkBase[i+3] = copy.mChunkBase[i+3];
            }
        }

        /// Sets all packed matrices to the same value as the scalar input matrix
        void setAll( const Matrix4 &m )
        {
            mChunkBase[0]  = _mm_set_ps1( m._m[0] );
            mChunkBase[1]  = _mm_set_ps1( m._m[1] );
            mChunkBase[2]  = _mm_set_ps1( m._m[2] );
            mChunkBase[3]  = _mm_set_ps1( m._m[3] );
            mChunkBase[4]  = _mm_set_ps1( m._m[4] );
            mChunkBase[5]  = _mm_set_ps1( m._m[5] );
            mChunkBase[6]  = _mm_set_ps1( m._m[6] );
            mChunkBase[7]  = _mm_set_ps1( m._m[7] );
            mChunkBase[8]  = _mm_set_ps1( m._m[8] );
            mChunkBase[9]  = _mm_set_ps1( m._m[9] );
            mChunkBase[10] = _mm_set_ps1( m._m[10] );
            mChunkBase[11] = _mm_set_ps1( m._m[11] );
        }

        static ArrayMatrixAf4x3 createAllFromMatrix4( const Matrix4 &m )
        {
            ArrayMatrixAf4x3 retVal;
            retVal.setAll( m );
            return retVal;
        }

        // Concatenation
        FORCEINLINE friend ArrayMatrixAf4x3 operator * ( const ArrayMatrixAf4x3 &lhs, const ArrayMatrixAf4x3 &rhs );

        inline ArrayVector3 operator * ( const ArrayVector3 &rhs ) const;

        /// Prefer the update version 'a *= b' A LOT over 'a = a * b'
        /// (copying from an ArrayMatrix4 is 256 bytes!)
        FORCEINLINE void operator *= ( const ArrayMatrixAf4x3 &rhs );

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

        /// @copydoc Matrix4::decomposition()
        inline void decomposition( ArrayVector3 &position, ArrayVector3 &scale,
                                   ArrayQuaternion &orientation ) const;

        /** Calculates the inverse of the matrix. If used against degenerate matrices,
            it may cause NaNs and Infs on those. Use @setToInverseDegeneratesAsIdentity
            if you want to deal with degenerate matrices.
        */
        inline void setToInverse(void);

        /** Calculates the inverse of the matrix. If one (or more) of the matrices are
            degenerate (don't have an inverse), those are set to identity.
        */
        inline void setToInverseDegeneratesAsIdentity(void);

        /** Strips orientation and/or scale components out of this matrix based on the input using
            branchless selection.
        @remarks
            Scale is always assumed to be positive. Negating the scale is the same as rotating
            180° and/or skewing. If negative scale was applied, it is assumed
            it was done using orientation/skewing alone (if orientation is stripped, the matrix will
            look in the opposite direction as if scale was positive, if scale is stripped, the
            matrix will keep looking in the opposite direction as if the scale were still negative)
            This behavior mimics that of major modeling tools.
        */
        inline void retain( ArrayMaskR orientation, ArrayMaskR scale );

        /** Converts these matrices contained in this ArrayMatrix to AoS form and stores them in dst
        @remarks
            'dst' must be aligned and assumed to have enough memory for ARRAY_PACKED_REALS matrices
        */
        inline void streamToAoS( Matrix4 * RESTRICT_ALIAS dst ) const;
        inline void storeToAoS( SimpleMatrixAf4x3 * RESTRICT_ALIAS src ) const;
        inline void streamToAoS( SimpleMatrixAf4x3 * RESTRICT_ALIAS src ) const;

        /** Converts ARRAY_PACKED_REALS matrices into this ArrayMatrix
        @remarks
            'src' must be aligned and assumed to have enough memory for ARRAY_PACKED_REALS matrices
        */
        inline void loadFromAoS( const Matrix4 * RESTRICT_ALIAS src );
        inline void loadFromAoS( const Matrix4 * RESTRICT_ALIAS * src );
        inline void loadFromAoS( const SimpleMatrixAf4x3 * RESTRICT_ALIAS src );
        inline void loadFromAoS( const SimpleMatrixAf4x3 * RESTRICT_ALIAS * src );

        static const ArrayMatrixAf4x3 IDENTITY;
    };

    /** Simple wrap up to load an AoS matrix 4x3 using SSE. The main reason of this class
        is to force MSVC to use 3 movaps to load arrays of MatrixAf4x3s (which are waaay more
        efficient that whatever lea+mov junk it tries to produce)
    */
    class _OgreExport SimpleMatrixAf4x3
    {
    public:
        ArrayReal       mChunkBase[3];

        SimpleMatrixAf4x3() {}
        SimpleMatrixAf4x3( ArrayReal row0, ArrayReal row1, ArrayReal row2 )
        {
            mChunkBase[0] = row0;
            mChunkBase[1] = row1;
            mChunkBase[2] = row2;
        }

        /// Assumes src is aligned
        void load( const Matrix4 &src )
        {
            mChunkBase[0] = _mm_load_ps( src._m );
            mChunkBase[1] = _mm_load_ps( src._m+4 );
            mChunkBase[2] = _mm_load_ps( src._m+8 );
        }

        /// Assumes dst is aligned
        void store( Matrix4 *dst ) const
        {
            float * RESTRICT_ALIAS dstPtr = reinterpret_cast<float*>( dst );

            _mm_store_ps( dstPtr, mChunkBase[0] );
            _mm_store_ps( dstPtr + 4, mChunkBase[1] );
            _mm_store_ps( dstPtr + 8, mChunkBase[2] );
            dstPtr += 12;
            *dstPtr++ = 0;
            *dstPtr++ = 0;
            *dstPtr++ = 0;
            *dstPtr++ = 1;
        }

        /// Assumes dst is aligned
        void store4x3( Matrix4 *dst ) const
        {
            float * RESTRICT_ALIAS dstPtr = reinterpret_cast<float*>( dst );

            _mm_store_ps( dstPtr, mChunkBase[0] );
            _mm_store_ps( dstPtr + 4, mChunkBase[1] );
            _mm_store_ps( dstPtr + 8, mChunkBase[2] );
        }

        /// Assumes dst is aligned
        void store4x3( float * RESTRICT_ALIAS dst ) const
        {
            _mm_store_ps( dst, mChunkBase[0] );
            _mm_store_ps( dst + 4, mChunkBase[1] );
            _mm_store_ps( dst + 8, mChunkBase[2] );
        }

        /// Copies our 4x3 contents using memory write combining when possible.
        void streamTo4x3( float * RESTRICT_ALIAS dst ) const
        {
#ifndef OGRE_RENDERSYSTEM_API_ALIGN_COMPATIBILITY
            _mm_stream_ps( dst,    mChunkBase[0] );
            _mm_stream_ps( dst+4,  mChunkBase[1] );
            _mm_stream_ps( dst+8,  mChunkBase[2] );
#else
            _mm_storeu_ps( dst,    mChunkBase[0] );
            _mm_storeu_ps( dst+4,  mChunkBase[1] );
            _mm_storeu_ps( dst+8,  mChunkBase[2] );
#endif
        }

        static const SimpleMatrixAf4x3 IDENTITY;
    };

    /** @} */
    /** @} */

}

#include "OgreArrayMatrixAf4x3.inl"

#endif
