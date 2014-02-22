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

namespace Ogre
{
    /// Concatenates two 4x4 array matrices.
    /// @remarks
    ///     99.99% of the cases the matrix isn't concatenated with itself, therefore it's
    ///     safe to assume &lhs != &rhs. RESTRICT_ALIAS modifier is used (a non-standard
    ///     C++ extension) is used when available to dramatically improve performance,
    ///     particularly of the update operations ( a *= b )
    ///     This function will assert if OGRE_RESTRICT_ALIASING is enabled and any of the
    ///     given pointers point to the same location
    inline void concatArrayMat4 ( ArrayReal * RESTRICT_ALIAS outChunkBase,
                                    const ArrayReal * RESTRICT_ALIAS lhsChunkBase,
                                    const ArrayReal * RESTRICT_ALIAS rhsChunkBase )
    {
#if OGRE_RESTRICT_ALIASING != 0
        assert( outChunkBase != lhsChunkBase && outChunkBase != rhsChunkBase &&
                lhsChunkBase != rhsChunkBase &&
                "Re-strict aliasing rule broken. Compile without OGRE_RESTRICT_ALIASING" );
#endif
        outChunkBase[0] =
            vaddq_f32(
            vaddq_f32(
                vmulq_f32( lhsChunkBase[0], rhsChunkBase[0] ),
                vmulq_f32( lhsChunkBase[1], rhsChunkBase[4] ) ),
            vaddq_f32(
                vmulq_f32( lhsChunkBase[2], rhsChunkBase[8] ),
                vmulq_f32( lhsChunkBase[3], rhsChunkBase[12] ) ) );
        outChunkBase[1] =
            vaddq_f32(
            vaddq_f32(
                vmulq_f32( lhsChunkBase[0], rhsChunkBase[1] ),
                vmulq_f32( lhsChunkBase[1], rhsChunkBase[5] ) ),
            vaddq_f32(
                vmulq_f32( lhsChunkBase[2], rhsChunkBase[9] ),
                vmulq_f32( lhsChunkBase[3], rhsChunkBase[13] ) ) );
        outChunkBase[2] =
            vaddq_f32(
            vaddq_f32(
                vmulq_f32( lhsChunkBase[0], rhsChunkBase[2] ),
                vmulq_f32( lhsChunkBase[1], rhsChunkBase[6] ) ),
            vaddq_f32(
                vmulq_f32( lhsChunkBase[2], rhsChunkBase[10] ),
                vmulq_f32( lhsChunkBase[3], rhsChunkBase[14] ) ) );
        outChunkBase[3] =
            vaddq_f32(
            vaddq_f32(
                vmulq_f32( lhsChunkBase[0], rhsChunkBase[3] ),
                vmulq_f32( lhsChunkBase[1], rhsChunkBase[7] ) ),
            vaddq_f32(
                vmulq_f32( lhsChunkBase[2], rhsChunkBase[11] ),
                vmulq_f32( lhsChunkBase[3], rhsChunkBase[15] ) ) );

        /* Next row (1) */
        outChunkBase[4] =
            vaddq_f32(
            vaddq_f32(
                vmulq_f32( lhsChunkBase[4], rhsChunkBase[0] ),
                vmulq_f32( lhsChunkBase[5], rhsChunkBase[4] ) ),
            vaddq_f32(
                vmulq_f32( lhsChunkBase[6], rhsChunkBase[8] ),
                vmulq_f32( lhsChunkBase[7], rhsChunkBase[12] ) ) );
        outChunkBase[5] =
            vaddq_f32(
            vaddq_f32(
                vmulq_f32( lhsChunkBase[4], rhsChunkBase[1] ),
                vmulq_f32( lhsChunkBase[5], rhsChunkBase[5] ) ),
            vaddq_f32(
                vmulq_f32( lhsChunkBase[6], rhsChunkBase[9] ),
                vmulq_f32( lhsChunkBase[7], rhsChunkBase[13] ) ) );
        outChunkBase[6] =
            vaddq_f32(
            vaddq_f32(
                vmulq_f32( lhsChunkBase[4], rhsChunkBase[2] ),
                vmulq_f32( lhsChunkBase[5], rhsChunkBase[6] ) ),
            vaddq_f32(
                vmulq_f32( lhsChunkBase[6], rhsChunkBase[10] ),
                vmulq_f32( lhsChunkBase[7], rhsChunkBase[14] ) ) );
        outChunkBase[7] =
            vaddq_f32(
            vaddq_f32(
                vmulq_f32( lhsChunkBase[4], rhsChunkBase[3] ),
                vmulq_f32( lhsChunkBase[5], rhsChunkBase[7] ) ),
            vaddq_f32(
                vmulq_f32( lhsChunkBase[6], rhsChunkBase[11] ),
                vmulq_f32( lhsChunkBase[7], rhsChunkBase[15] ) ) );

        /* Next row (2) */
        outChunkBase[8] =
            vaddq_f32(
            vaddq_f32(
                vmulq_f32( lhsChunkBase[8], rhsChunkBase[0] ),
                vmulq_f32( lhsChunkBase[9], rhsChunkBase[4] ) ),
            vaddq_f32(
                vmulq_f32( lhsChunkBase[10], rhsChunkBase[8] ),
                vmulq_f32( lhsChunkBase[11], rhsChunkBase[12] ) ) );
        outChunkBase[9] =
            vaddq_f32(
            vaddq_f32(
                vmulq_f32( lhsChunkBase[8], rhsChunkBase[1] ),
                vmulq_f32( lhsChunkBase[9], rhsChunkBase[5] ) ),
            vaddq_f32(
                vmulq_f32( lhsChunkBase[10], rhsChunkBase[9] ),
                vmulq_f32( lhsChunkBase[11], rhsChunkBase[13] ) ) );
        outChunkBase[10] =
            vaddq_f32(
            vaddq_f32(
                vmulq_f32( lhsChunkBase[8], rhsChunkBase[2] ),
                vmulq_f32( lhsChunkBase[9], rhsChunkBase[6] ) ),
            vaddq_f32(
                vmulq_f32( lhsChunkBase[10], rhsChunkBase[10] ),
                vmulq_f32( lhsChunkBase[11], rhsChunkBase[14] ) ) );
        outChunkBase[11] =
            vaddq_f32(
            vaddq_f32(
                vmulq_f32( lhsChunkBase[8], rhsChunkBase[3] ),
                vmulq_f32( lhsChunkBase[9], rhsChunkBase[7] ) ),
            vaddq_f32(
                vmulq_f32( lhsChunkBase[10], rhsChunkBase[11] ),
                vmulq_f32( lhsChunkBase[11], rhsChunkBase[15] ) ) );

        /* Next row (3) */
        outChunkBase[12] =
            vaddq_f32(
            vaddq_f32(
                vmulq_f32( lhsChunkBase[12], rhsChunkBase[0] ),
                vmulq_f32( lhsChunkBase[13], rhsChunkBase[4] ) ),
            vaddq_f32(
                vmulq_f32( lhsChunkBase[14], rhsChunkBase[8] ),
                vmulq_f32( lhsChunkBase[15], rhsChunkBase[12] ) ) );
        outChunkBase[13] =
            vaddq_f32(
            vaddq_f32(
                vmulq_f32( lhsChunkBase[12], rhsChunkBase[1] ),
                vmulq_f32( lhsChunkBase[13], rhsChunkBase[5] ) ),
            vaddq_f32(
                vmulq_f32( lhsChunkBase[14], rhsChunkBase[9] ),
                vmulq_f32( lhsChunkBase[15], rhsChunkBase[13] ) ) );
        outChunkBase[14] =
            vaddq_f32(
            vaddq_f32(
                vmulq_f32( lhsChunkBase[12], rhsChunkBase[2] ),
                vmulq_f32( lhsChunkBase[13], rhsChunkBase[6] ) ),
            vaddq_f32(
                vmulq_f32( lhsChunkBase[14], rhsChunkBase[10] ),
                vmulq_f32( lhsChunkBase[15], rhsChunkBase[14] ) ) );
        outChunkBase[15] =
            vaddq_f32(
            vaddq_f32(
                vmulq_f32( lhsChunkBase[12], rhsChunkBase[3] ),
                vmulq_f32( lhsChunkBase[13], rhsChunkBase[7] ) ),
            vaddq_f32(
                vmulq_f32( lhsChunkBase[14], rhsChunkBase[11] ),
                vmulq_f32( lhsChunkBase[15], rhsChunkBase[15] ) ) );
    }

    /// Update version
    inline void concatArrayMat4 ( ArrayReal * RESTRICT_ALIAS lhsChunkBase,
                                    const ArrayReal * RESTRICT_ALIAS rhsChunkBase )
    {
#if OGRE_RESTRICT_ALIASING != 0
        assert( lhsChunkBase != rhsChunkBase &&
                "Re-strict aliasing rule broken. Compile without OGRE_RESTRICT_ALIASING" );
#endif
        ArrayReal lhs0 = lhsChunkBase[0];
        lhsChunkBase[0] =
            vaddq_f32(
            vaddq_f32(
                vmulq_f32( lhsChunkBase[0], rhsChunkBase[0] ),
                vmulq_f32( lhsChunkBase[1], rhsChunkBase[4] ) ),
            vaddq_f32(
                vmulq_f32( lhsChunkBase[2], rhsChunkBase[8] ),
                vmulq_f32( lhsChunkBase[3], rhsChunkBase[12] ) ) );
        ArrayReal lhs1 = lhsChunkBase[1];
        lhsChunkBase[1] =
            vaddq_f32(
            vaddq_f32(
                vmulq_f32( lhs0, rhsChunkBase[1] ),
                vmulq_f32( lhsChunkBase[1], rhsChunkBase[5] ) ),
            vaddq_f32(
                vmulq_f32( lhsChunkBase[2], rhsChunkBase[9] ),
                vmulq_f32( lhsChunkBase[3], rhsChunkBase[13] ) ) );
        ArrayReal lhs2 = lhsChunkBase[2];
        lhsChunkBase[2] =
            vaddq_f32(
            vaddq_f32(
                vmulq_f32( lhs0, rhsChunkBase[2] ),
                vmulq_f32( lhs1, rhsChunkBase[6] ) ),
            vaddq_f32(
                vmulq_f32( lhsChunkBase[2], rhsChunkBase[10] ),
                vmulq_f32( lhsChunkBase[3], rhsChunkBase[14] ) ) );
        lhsChunkBase[3] =
            vaddq_f32(
            vaddq_f32(
                vmulq_f32( lhs0, rhsChunkBase[3] ),
                vmulq_f32( lhs1, rhsChunkBase[7] ) ),
            vaddq_f32(
                vmulq_f32( lhs2, rhsChunkBase[11] ),
                vmulq_f32( lhsChunkBase[3], rhsChunkBase[15] ) ) );

        /* Next row (1) */
        lhs0 = lhsChunkBase[4];
        lhsChunkBase[4] =
            vaddq_f32(
            vaddq_f32(
                vmulq_f32( lhsChunkBase[4], rhsChunkBase[0] ),
                vmulq_f32( lhsChunkBase[5], rhsChunkBase[4] ) ),
            vaddq_f32(
                vmulq_f32( lhsChunkBase[6], rhsChunkBase[8] ),
                vmulq_f32( lhsChunkBase[7], rhsChunkBase[12] ) ) );
        lhs1 = lhsChunkBase[5];
        lhsChunkBase[5] =
            vaddq_f32(
            vaddq_f32(
                vmulq_f32( lhs0, rhsChunkBase[1] ),
                vmulq_f32( lhsChunkBase[5], rhsChunkBase[5] ) ),
            vaddq_f32(
                vmulq_f32( lhsChunkBase[6], rhsChunkBase[9] ),
                vmulq_f32( lhsChunkBase[7], rhsChunkBase[13] ) ) );
        lhs2 = lhsChunkBase[6];
        lhsChunkBase[6] =
            vaddq_f32(
            vaddq_f32(
                vmulq_f32( lhs0, rhsChunkBase[2] ),
                vmulq_f32( lhs1, rhsChunkBase[6] ) ),
            vaddq_f32(
                vmulq_f32( lhsChunkBase[6], rhsChunkBase[10] ),
                vmulq_f32( lhsChunkBase[7], rhsChunkBase[14] ) ) );
        lhsChunkBase[7] =
            vaddq_f32(
            vaddq_f32(
                vmulq_f32( lhs0, rhsChunkBase[3] ),
                vmulq_f32( lhs1, rhsChunkBase[7] ) ),
            vaddq_f32(
                vmulq_f32( lhs2, rhsChunkBase[11] ),
                vmulq_f32( lhsChunkBase[7], rhsChunkBase[15] ) ) );

        /* Next row (2) */
        lhs0 = lhsChunkBase[8];
        lhsChunkBase[8] =
            vaddq_f32(
            vaddq_f32(
                vmulq_f32( lhsChunkBase[8], rhsChunkBase[0] ),
                vmulq_f32( lhsChunkBase[9], rhsChunkBase[4] ) ),
            vaddq_f32(
                vmulq_f32( lhsChunkBase[10], rhsChunkBase[8] ),
                vmulq_f32( lhsChunkBase[11], rhsChunkBase[12] ) ) );
        lhs1 = lhsChunkBase[9];
        lhsChunkBase[9] =
            vaddq_f32(
            vaddq_f32(
                vmulq_f32( lhs0, rhsChunkBase[1] ),
                vmulq_f32( lhsChunkBase[9], rhsChunkBase[5] ) ),
            vaddq_f32(
                vmulq_f32( lhsChunkBase[10], rhsChunkBase[9] ),
                vmulq_f32( lhsChunkBase[11], rhsChunkBase[13] ) ) );
        lhs2 = lhsChunkBase[10];
        lhsChunkBase[10] =
            vaddq_f32(
            vaddq_f32(
                vmulq_f32( lhs0, rhsChunkBase[2] ),
                vmulq_f32( lhs1, rhsChunkBase[6] ) ),
            vaddq_f32(
                vmulq_f32( lhsChunkBase[10], rhsChunkBase[10] ),
                vmulq_f32( lhsChunkBase[11], rhsChunkBase[14] ) ) );
        lhsChunkBase[11] =
            vaddq_f32(
            vaddq_f32(
                vmulq_f32( lhs0, rhsChunkBase[3] ),
                vmulq_f32( lhs1, rhsChunkBase[7] ) ),
            vaddq_f32(
                vmulq_f32( lhs2, rhsChunkBase[11] ),
                vmulq_f32( lhsChunkBase[11], rhsChunkBase[15] ) ) );

        /* Next row (3) */
        lhs0 = lhsChunkBase[12];
        lhsChunkBase[12] =
            vaddq_f32(
            vaddq_f32(
                vmulq_f32( lhsChunkBase[12], rhsChunkBase[0] ),
                vmulq_f32( lhsChunkBase[13], rhsChunkBase[4] ) ),
            vaddq_f32(
                vmulq_f32( lhsChunkBase[14], rhsChunkBase[8] ),
                vmulq_f32( lhsChunkBase[15], rhsChunkBase[12] ) ) );
        lhs1 = lhsChunkBase[13];
        lhsChunkBase[13] =
            vaddq_f32(
            vaddq_f32(
                vmulq_f32( lhs0, rhsChunkBase[1] ),
                vmulq_f32( lhsChunkBase[13], rhsChunkBase[5] ) ),
            vaddq_f32(
                vmulq_f32( lhsChunkBase[14], rhsChunkBase[9] ),
                vmulq_f32( lhsChunkBase[15], rhsChunkBase[13] ) ) );
        lhs2 = lhsChunkBase[14];
        lhsChunkBase[14] =
            vaddq_f32(
            vaddq_f32(
                vmulq_f32( lhs0, rhsChunkBase[2] ),
                vmulq_f32( lhs1, rhsChunkBase[6] ) ),
            vaddq_f32(
                vmulq_f32( lhsChunkBase[14], rhsChunkBase[10] ),
                vmulq_f32( lhsChunkBase[15], rhsChunkBase[14] ) ) );
        lhsChunkBase[15] =
            vaddq_f32(
            vaddq_f32(
                vmulq_f32( lhs0, rhsChunkBase[3] ),
                vmulq_f32( lhs1, rhsChunkBase[7] ) ),
            vaddq_f32(
                vmulq_f32( lhs2, rhsChunkBase[11] ),
                vmulq_f32( lhsChunkBase[15], rhsChunkBase[15] ) ) );
    }

    inline ArrayMatrix4 operator * ( const ArrayMatrix4 &lhs, const ArrayMatrix4 &rhs )
    {
        ArrayMatrix4 retVal;
        concatArrayMat4( retVal.mChunkBase, lhs.mChunkBase, rhs.mChunkBase );
        return retVal;
    }
    //-----------------------------------------------------------------------------------
    inline ArrayVector3 ArrayMatrix4::operator * ( const ArrayVector3 &rhs ) const
    {
        ArrayReal invW = vaddq_f32( vaddq_f32(
                                vmulq_f32( mChunkBase[12], rhs.mChunkBase[0] ),
                                vmulq_f32( mChunkBase[13], rhs.mChunkBase[1] ) ),
                            vaddq_f32(
                                vmulq_f32( mChunkBase[14], rhs.mChunkBase[2] ),
                                mChunkBase[15] ) );
        invW = MathlibNEON::Inv4( invW );

        return ArrayVector3(
            //X
            vmulq_f32(
            vaddq_f32( vaddq_f32(
                vmulq_f32( mChunkBase[0], rhs.mChunkBase[0] ),  //( m00 * v.x   +
                vmulq_f32( mChunkBase[1], rhs.mChunkBase[1] ) ),    //  m01 * v.y ) +
            vaddq_f32(
                vmulq_f32( mChunkBase[2], rhs.mChunkBase[2] ),  //( m02 * v.z   +
                mChunkBase[3] ) ) , invW ),                     //  m03 ) * fInvW
            //Y
            vmulq_f32(
            vaddq_f32( vaddq_f32(
                vmulq_f32( mChunkBase[4], rhs.mChunkBase[0] ),  //( m10 * v.x   +
                vmulq_f32( mChunkBase[5], rhs.mChunkBase[1] ) ),    //  m11 * v.y ) +
            vaddq_f32(
                vmulq_f32( mChunkBase[6], rhs.mChunkBase[2] ),  //( m12 * v.z   +
                mChunkBase[7] ) ), invW ),                          //  m13 ) * fInvW
            //Z
            vmulq_f32(
            vaddq_f32( vaddq_f32(
                vmulq_f32( mChunkBase[8], rhs.mChunkBase[0] ),  //( m20 * v.x   +
                vmulq_f32( mChunkBase[9], rhs.mChunkBase[1] ) ),    //  m21 * v.y ) +
            vaddq_f32(
                vmulq_f32( mChunkBase[10], rhs.mChunkBase[2] ), //( m22 * v.z   +
                mChunkBase[11] ) ), invW ) );                       //  m23 ) * fInvW
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayMatrix4::operator *= ( const ArrayMatrix4 &rhs )
    {
        concatArrayMat4( mChunkBase, rhs.mChunkBase );
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayMatrix4::fromQuaternion( const ArrayQuaternion &q )
    {
        ArrayReal * RESTRICT_ALIAS chunkBase = mChunkBase;
        const ArrayReal * RESTRICT_ALIAS qChunkBase = q.mChunkBase;
        ArrayReal fTx  = vaddq_f32( qChunkBase[1], qChunkBase[1] );     // 2 * x
        ArrayReal fTy  = vaddq_f32( qChunkBase[2], qChunkBase[2] );     // 2 * y
        ArrayReal fTz  = vaddq_f32( qChunkBase[3], qChunkBase[3] );     // 2 * z
        ArrayReal fTwx = vmulq_f32( fTx, qChunkBase[0] );                   // fTx*w;
        ArrayReal fTwy = vmulq_f32( fTy, qChunkBase[0] );                   // fTy*w;
        ArrayReal fTwz = vmulq_f32( fTz, qChunkBase[0] );                   // fTz*w;
        ArrayReal fTxx = vmulq_f32( fTx, qChunkBase[1] );                   // fTx*x;
        ArrayReal fTxy = vmulq_f32( fTy, qChunkBase[1] );                   // fTy*x;
        ArrayReal fTxz = vmulq_f32( fTz, qChunkBase[1] );                   // fTz*x;
        ArrayReal fTyy = vmulq_f32( fTy, qChunkBase[2] );                   // fTy*y;
        ArrayReal fTyz = vmulq_f32( fTz, qChunkBase[2] );                   // fTz*y;
        ArrayReal fTzz = vmulq_f32( fTz, qChunkBase[3] );                   // fTz*z;

        chunkBase[0] = vsubq_f32( MathlibNEON::ONE, vaddq_f32( fTyy, fTzz ) );
        chunkBase[1] = vsubq_f32( fTxy, fTwz );
        chunkBase[2] = vaddq_f32( fTxz, fTwy );
        chunkBase[4] = vaddq_f32( fTxy, fTwz );
        chunkBase[5] = vsubq_f32( MathlibNEON::ONE, vaddq_f32( fTxx, fTzz ) );
        chunkBase[6] = vsubq_f32( fTyz, fTwx );
        chunkBase[8] = vsubq_f32( fTxz, fTwy );
        chunkBase[9] = vaddq_f32( fTyz, fTwx );
        chunkBase[10]= vsubq_f32( MathlibNEON::ONE, vaddq_f32( fTxx, fTyy ) );
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayMatrix4::makeTransform( const ArrayVector3 &position, const ArrayVector3 &scale,
                                             const ArrayQuaternion &orientation )
    {
        ArrayReal * RESTRICT_ALIAS chunkBase            = mChunkBase;
        const ArrayReal * RESTRICT_ALIAS posChunkBase   = position.mChunkBase;
        const ArrayReal * RESTRICT_ALIAS scaleChunkBase = scale.mChunkBase;
        this->fromQuaternion( orientation );
        chunkBase[0] = vmulq_f32( chunkBase[0], scaleChunkBase[0] );    //m00 * scale.x
        chunkBase[1] = vmulq_f32( chunkBase[1], scaleChunkBase[1] );    //m01 * scale.y
        chunkBase[2] = vmulq_f32( chunkBase[2], scaleChunkBase[2] );    //m02 * scale.z
        chunkBase[3] =  posChunkBase[0];                                //m03 * pos.x

        chunkBase[4] = vmulq_f32( chunkBase[4], scaleChunkBase[0] );    //m10 * scale.x
        chunkBase[5] = vmulq_f32( chunkBase[5], scaleChunkBase[1] );    //m11 * scale.y
        chunkBase[6] = vmulq_f32( chunkBase[6], scaleChunkBase[2] );    //m12 * scale.z
        chunkBase[7] =  posChunkBase[1];                                //m13 * pos.y

        chunkBase[8] = vmulq_f32( chunkBase[8], scaleChunkBase[0] );    //m20 * scale.x
        chunkBase[9] = vmulq_f32( chunkBase[9], scaleChunkBase[1] );    //m21 * scale.y
        chunkBase[10]= vmulq_f32( chunkBase[10],scaleChunkBase[2] );    //m22 * scale.z
        chunkBase[11]=  posChunkBase[2];                                //m23 * pos.z

        // No projection term
        chunkBase[12] = mChunkBase[13] = mChunkBase[14] = vdupq_n_f32(0.0f);
        chunkBase[15] = MathlibNEON::ONE;
    }
    //-----------------------------------------------------------------------------------
    inline bool ArrayMatrix4::isAffine() const
    {
        uint32x4_t mask =
            vandq_u32(
                vandq_u32( vceqq_f32( mChunkBase[12], vdupq_n_f32(0.0f) ),
                    vceqq_f32( mChunkBase[13], vdupq_n_f32(0.0f) ) ),
                vandq_u32( vceqq_f32( mChunkBase[14], vdupq_n_f32(0.0f) ),
                    vceqq_f32( mChunkBase[15], MathlibNEON::ONE ) ) );

        return vmovemaskq_u32( mask ) == 0x0f;
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayMatrix4::storeToAoS( Matrix4 * RESTRICT_ALIAS dst ) const
    {
#define _MM_TRANSPOSE4_SRC_DST_PS(row0, row1, row2, row3, dst0, dst1, dst2, dst3) { \
            float32x4x4_t tmp0, tmp1;               \
            tmp0.val[0] = row0;                     \
            tmp0.val[1] = row1;                     \
            tmp0.val[2] = row2;                     \
            tmp0.val[3] = row3;                     \
            vst4q_f32((float32_t*)&tmp1, tmp0);     \
            dst0 = tmp1.val[0];                     \
            dst1 = tmp1.val[1];                     \
            dst2 = tmp1.val[2];                     \
            dst3 = tmp1.val[3];                     \
        }

        register ArrayReal m0, m1, m2, m3;

        _MM_TRANSPOSE4_SRC_DST_PS(
                            this->mChunkBase[0], this->mChunkBase[1],
                            this->mChunkBase[2], this->mChunkBase[3],
                            m0, m1, m2, m3 );
        vst1q_f32( dst[0]._m, m0 );
        vst1q_f32( dst[1]._m, m1 );
        vst1q_f32( dst[2]._m, m2 );
        vst1q_f32( dst[3]._m, m3 );
        _MM_TRANSPOSE4_SRC_DST_PS(
                            this->mChunkBase[4], this->mChunkBase[5],
                            this->mChunkBase[6], this->mChunkBase[7],
                            m0, m1, m2, m3 );
        vst1q_f32( dst[0]._m+4, m0 );
        vst1q_f32( dst[1]._m+4, m1 );
        vst1q_f32( dst[2]._m+4, m2 );
        vst1q_f32( dst[3]._m+4, m3 );
        _MM_TRANSPOSE4_SRC_DST_PS(
                            this->mChunkBase[8], this->mChunkBase[9],
                            this->mChunkBase[10], this->mChunkBase[11],
                            m0, m1, m2, m3 );
        vst1q_f32( dst[0]._m+8, m0 );
        vst1q_f32( dst[1]._m+8, m1 );
        vst1q_f32( dst[2]._m+8, m2 );
        vst1q_f32( dst[3]._m+8, m3 );
        _MM_TRANSPOSE4_SRC_DST_PS(
                            this->mChunkBase[12], this->mChunkBase[13],
                            this->mChunkBase[14], this->mChunkBase[15],
                            m0, m1, m2, m3 );
        vst1q_f32( dst[0]._m+12, m0 );
        vst1q_f32( dst[1]._m+12, m1 );
        vst1q_f32( dst[2]._m+12, m2 );
        vst1q_f32( dst[3]._m+12, m3 );
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayMatrix4::loadFromAoS( const Matrix4 * RESTRICT_ALIAS src )
    {
        _MM_TRANSPOSE4_SRC_DST_PS(
                            vld1q_f32( src[0]._m ), vld1q_f32( src[1]._m ), 
                            vld1q_f32( src[2]._m ), vld1q_f32( src[3]._m ),
                            this->mChunkBase[0], this->mChunkBase[1],
                            this->mChunkBase[2], this->mChunkBase[3] );
        _MM_TRANSPOSE4_SRC_DST_PS(
                            vld1q_f32( src[0]._m+4 ), vld1q_f32( src[1]._m+4 ), 
                            vld1q_f32( src[2]._m+4 ), vld1q_f32( src[3]._m+4 ),
                            this->mChunkBase[4], this->mChunkBase[5],
                            this->mChunkBase[6], this->mChunkBase[7] );
        _MM_TRANSPOSE4_SRC_DST_PS(
                            vld1q_f32( src[0]._m+8 ), vld1q_f32( src[1]._m+8 ), 
                            vld1q_f32( src[2]._m+8 ), vld1q_f32( src[3]._m+8 ),
                            this->mChunkBase[8], this->mChunkBase[9],
                            this->mChunkBase[10], this->mChunkBase[11] );
        _MM_TRANSPOSE4_SRC_DST_PS(
                            vld1q_f32( src[0]._m+12 ), vld1q_f32( src[1]._m+12 ), 
                            vld1q_f32( src[2]._m+12 ), vld1q_f32( src[3]._m+12 ),
                            this->mChunkBase[12], this->mChunkBase[13],
                            this->mChunkBase[14], this->mChunkBase[15] );
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayMatrix4::loadFromAoS( const SimpleMatrix4 * RESTRICT_ALIAS src )
    {
        _MM_TRANSPOSE4_SRC_DST_PS(
                            src[0].mChunkBase[0], src[1].mChunkBase[0],
                            src[2].mChunkBase[0], src[3].mChunkBase[0],
                            this->mChunkBase[0], this->mChunkBase[1],
                            this->mChunkBase[2], this->mChunkBase[3] );
        _MM_TRANSPOSE4_SRC_DST_PS(
                            src[0].mChunkBase[1], src[1].mChunkBase[1],
                            src[2].mChunkBase[1], src[3].mChunkBase[1],
                            this->mChunkBase[4], this->mChunkBase[5],
                            this->mChunkBase[6], this->mChunkBase[7] );
        _MM_TRANSPOSE4_SRC_DST_PS(
                            src[0].mChunkBase[2], src[1].mChunkBase[2],
                            src[2].mChunkBase[2], src[3].mChunkBase[2],
                            this->mChunkBase[8], this->mChunkBase[9],
                            this->mChunkBase[10], this->mChunkBase[11] );
        _MM_TRANSPOSE4_SRC_DST_PS(
                            src[0].mChunkBase[3], src[1].mChunkBase[3],
                            src[2].mChunkBase[3], src[3].mChunkBase[3],
                            this->mChunkBase[12], this->mChunkBase[13],
                            this->mChunkBase[14], this->mChunkBase[15] );
    }
    //-----------------------------------------------------------------------------------
    #undef _MM_TRANSPOSE4_SRC_DST_PS
}
