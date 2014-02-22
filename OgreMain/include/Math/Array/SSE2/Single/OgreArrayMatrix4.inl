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
            _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[0], rhsChunkBase[0] ),
                _mm_mul_ps( lhsChunkBase[1], rhsChunkBase[4] ) ),
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[2], rhsChunkBase[8] ),
                _mm_mul_ps( lhsChunkBase[3], rhsChunkBase[12] ) ) );
        outChunkBase[1] =
            _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[0], rhsChunkBase[1] ),
                _mm_mul_ps( lhsChunkBase[1], rhsChunkBase[5] ) ),
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[2], rhsChunkBase[9] ),
                _mm_mul_ps( lhsChunkBase[3], rhsChunkBase[13] ) ) );
        outChunkBase[2] =
            _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[0], rhsChunkBase[2] ),
                _mm_mul_ps( lhsChunkBase[1], rhsChunkBase[6] ) ),
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[2], rhsChunkBase[10] ),
                _mm_mul_ps( lhsChunkBase[3], rhsChunkBase[14] ) ) );
        outChunkBase[3] =
            _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[0], rhsChunkBase[3] ),
                _mm_mul_ps( lhsChunkBase[1], rhsChunkBase[7] ) ),
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[2], rhsChunkBase[11] ),
                _mm_mul_ps( lhsChunkBase[3], rhsChunkBase[15] ) ) );

        /* Next row (1) */
        outChunkBase[4] =
            _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[4], rhsChunkBase[0] ),
                _mm_mul_ps( lhsChunkBase[5], rhsChunkBase[4] ) ),
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[6], rhsChunkBase[8] ),
                _mm_mul_ps( lhsChunkBase[7], rhsChunkBase[12] ) ) );
        outChunkBase[5] =
            _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[4], rhsChunkBase[1] ),
                _mm_mul_ps( lhsChunkBase[5], rhsChunkBase[5] ) ),
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[6], rhsChunkBase[9] ),
                _mm_mul_ps( lhsChunkBase[7], rhsChunkBase[13] ) ) );
        outChunkBase[6] =
            _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[4], rhsChunkBase[2] ),
                _mm_mul_ps( lhsChunkBase[5], rhsChunkBase[6] ) ),
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[6], rhsChunkBase[10] ),
                _mm_mul_ps( lhsChunkBase[7], rhsChunkBase[14] ) ) );
        outChunkBase[7] =
            _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[4], rhsChunkBase[3] ),
                _mm_mul_ps( lhsChunkBase[5], rhsChunkBase[7] ) ),
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[6], rhsChunkBase[11] ),
                _mm_mul_ps( lhsChunkBase[7], rhsChunkBase[15] ) ) );

        /* Next row (2) */
        outChunkBase[8] =
            _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[8], rhsChunkBase[0] ),
                _mm_mul_ps( lhsChunkBase[9], rhsChunkBase[4] ) ),
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[10], rhsChunkBase[8] ),
                _mm_mul_ps( lhsChunkBase[11], rhsChunkBase[12] ) ) );
        outChunkBase[9] =
            _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[8], rhsChunkBase[1] ),
                _mm_mul_ps( lhsChunkBase[9], rhsChunkBase[5] ) ),
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[10], rhsChunkBase[9] ),
                _mm_mul_ps( lhsChunkBase[11], rhsChunkBase[13] ) ) );
        outChunkBase[10] =
            _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[8], rhsChunkBase[2] ),
                _mm_mul_ps( lhsChunkBase[9], rhsChunkBase[6] ) ),
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[10], rhsChunkBase[10] ),
                _mm_mul_ps( lhsChunkBase[11], rhsChunkBase[14] ) ) );
        outChunkBase[11] =
            _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[8], rhsChunkBase[3] ),
                _mm_mul_ps( lhsChunkBase[9], rhsChunkBase[7] ) ),
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[10], rhsChunkBase[11] ),
                _mm_mul_ps( lhsChunkBase[11], rhsChunkBase[15] ) ) );

        /* Next row (3) */
        outChunkBase[12] =
            _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[12], rhsChunkBase[0] ),
                _mm_mul_ps( lhsChunkBase[13], rhsChunkBase[4] ) ),
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[14], rhsChunkBase[8] ),
                _mm_mul_ps( lhsChunkBase[15], rhsChunkBase[12] ) ) );
        outChunkBase[13] =
            _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[12], rhsChunkBase[1] ),
                _mm_mul_ps( lhsChunkBase[13], rhsChunkBase[5] ) ),
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[14], rhsChunkBase[9] ),
                _mm_mul_ps( lhsChunkBase[15], rhsChunkBase[13] ) ) );
        outChunkBase[14] =
            _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[12], rhsChunkBase[2] ),
                _mm_mul_ps( lhsChunkBase[13], rhsChunkBase[6] ) ),
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[14], rhsChunkBase[10] ),
                _mm_mul_ps( lhsChunkBase[15], rhsChunkBase[14] ) ) );
        outChunkBase[15] =
            _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[12], rhsChunkBase[3] ),
                _mm_mul_ps( lhsChunkBase[13], rhsChunkBase[7] ) ),
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[14], rhsChunkBase[11] ),
                _mm_mul_ps( lhsChunkBase[15], rhsChunkBase[15] ) ) );
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
            _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[0], rhsChunkBase[0] ),
                _mm_mul_ps( lhsChunkBase[1], rhsChunkBase[4] ) ),
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[2], rhsChunkBase[8] ),
                _mm_mul_ps( lhsChunkBase[3], rhsChunkBase[12] ) ) );
        ArrayReal lhs1 = lhsChunkBase[1];
        lhsChunkBase[1] =
            _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps( lhs0, rhsChunkBase[1] ),
                _mm_mul_ps( lhsChunkBase[1], rhsChunkBase[5] ) ),
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[2], rhsChunkBase[9] ),
                _mm_mul_ps( lhsChunkBase[3], rhsChunkBase[13] ) ) );
        ArrayReal lhs2 = lhsChunkBase[2];
        lhsChunkBase[2] =
            _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps( lhs0, rhsChunkBase[2] ),
                _mm_mul_ps( lhs1, rhsChunkBase[6] ) ),
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[2], rhsChunkBase[10] ),
                _mm_mul_ps( lhsChunkBase[3], rhsChunkBase[14] ) ) );
        lhsChunkBase[3] =
            _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps( lhs0, rhsChunkBase[3] ),
                _mm_mul_ps( lhs1, rhsChunkBase[7] ) ),
            _mm_add_ps(
                _mm_mul_ps( lhs2, rhsChunkBase[11] ),
                _mm_mul_ps( lhsChunkBase[3], rhsChunkBase[15] ) ) );

        /* Next row (1) */
        lhs0 = lhsChunkBase[4];
        lhsChunkBase[4] =
            _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[4], rhsChunkBase[0] ),
                _mm_mul_ps( lhsChunkBase[5], rhsChunkBase[4] ) ),
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[6], rhsChunkBase[8] ),
                _mm_mul_ps( lhsChunkBase[7], rhsChunkBase[12] ) ) );
        lhs1 = lhsChunkBase[5];
        lhsChunkBase[5] =
            _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps( lhs0, rhsChunkBase[1] ),
                _mm_mul_ps( lhsChunkBase[5], rhsChunkBase[5] ) ),
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[6], rhsChunkBase[9] ),
                _mm_mul_ps( lhsChunkBase[7], rhsChunkBase[13] ) ) );
        lhs2 = lhsChunkBase[6];
        lhsChunkBase[6] =
            _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps( lhs0, rhsChunkBase[2] ),
                _mm_mul_ps( lhs1, rhsChunkBase[6] ) ),
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[6], rhsChunkBase[10] ),
                _mm_mul_ps( lhsChunkBase[7], rhsChunkBase[14] ) ) );
        lhsChunkBase[7] =
            _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps( lhs0, rhsChunkBase[3] ),
                _mm_mul_ps( lhs1, rhsChunkBase[7] ) ),
            _mm_add_ps(
                _mm_mul_ps( lhs2, rhsChunkBase[11] ),
                _mm_mul_ps( lhsChunkBase[7], rhsChunkBase[15] ) ) );

        /* Next row (2) */
        lhs0 = lhsChunkBase[8];
        lhsChunkBase[8] =
            _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[8], rhsChunkBase[0] ),
                _mm_mul_ps( lhsChunkBase[9], rhsChunkBase[4] ) ),
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[10], rhsChunkBase[8] ),
                _mm_mul_ps( lhsChunkBase[11], rhsChunkBase[12] ) ) );
        lhs1 = lhsChunkBase[9];
        lhsChunkBase[9] =
            _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps( lhs0, rhsChunkBase[1] ),
                _mm_mul_ps( lhsChunkBase[9], rhsChunkBase[5] ) ),
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[10], rhsChunkBase[9] ),
                _mm_mul_ps( lhsChunkBase[11], rhsChunkBase[13] ) ) );
        lhs2 = lhsChunkBase[10];
        lhsChunkBase[10] =
            _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps( lhs0, rhsChunkBase[2] ),
                _mm_mul_ps( lhs1, rhsChunkBase[6] ) ),
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[10], rhsChunkBase[10] ),
                _mm_mul_ps( lhsChunkBase[11], rhsChunkBase[14] ) ) );
        lhsChunkBase[11] =
            _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps( lhs0, rhsChunkBase[3] ),
                _mm_mul_ps( lhs1, rhsChunkBase[7] ) ),
            _mm_add_ps(
                _mm_mul_ps( lhs2, rhsChunkBase[11] ),
                _mm_mul_ps( lhsChunkBase[11], rhsChunkBase[15] ) ) );

        /* Next row (3) */
        lhs0 = lhsChunkBase[12];
        lhsChunkBase[12] =
            _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[12], rhsChunkBase[0] ),
                _mm_mul_ps( lhsChunkBase[13], rhsChunkBase[4] ) ),
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[14], rhsChunkBase[8] ),
                _mm_mul_ps( lhsChunkBase[15], rhsChunkBase[12] ) ) );
        lhs1 = lhsChunkBase[13];
        lhsChunkBase[13] =
            _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps( lhs0, rhsChunkBase[1] ),
                _mm_mul_ps( lhsChunkBase[13], rhsChunkBase[5] ) ),
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[14], rhsChunkBase[9] ),
                _mm_mul_ps( lhsChunkBase[15], rhsChunkBase[13] ) ) );
        lhs2 = lhsChunkBase[14];
        lhsChunkBase[14] =
            _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps( lhs0, rhsChunkBase[2] ),
                _mm_mul_ps( lhs1, rhsChunkBase[6] ) ),
            _mm_add_ps(
                _mm_mul_ps( lhsChunkBase[14], rhsChunkBase[10] ),
                _mm_mul_ps( lhsChunkBase[15], rhsChunkBase[14] ) ) );
        lhsChunkBase[15] =
            _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps( lhs0, rhsChunkBase[3] ),
                _mm_mul_ps( lhs1, rhsChunkBase[7] ) ),
            _mm_add_ps(
                _mm_mul_ps( lhs2, rhsChunkBase[11] ),
                _mm_mul_ps( lhsChunkBase[15], rhsChunkBase[15] ) ) );
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
        ArrayReal invW = _mm_add_ps( _mm_add_ps(
                                _mm_mul_ps( mChunkBase[12], rhs.mChunkBase[0] ),
                                _mm_mul_ps( mChunkBase[13], rhs.mChunkBase[1] ) ),
                            _mm_add_ps(
                                _mm_mul_ps( mChunkBase[14], rhs.mChunkBase[2] ),
                                mChunkBase[15] ) );
        invW = MathlibSSE2::Inv4( invW );

        return ArrayVector3(
            //X
            _mm_mul_ps(
            _mm_add_ps( _mm_add_ps(
                _mm_mul_ps( mChunkBase[0], rhs.mChunkBase[0] ), //( m00 * v.x   +
                _mm_mul_ps( mChunkBase[1], rhs.mChunkBase[1] ) ),   //  m01 * v.y ) +
            _mm_add_ps(
                _mm_mul_ps( mChunkBase[2], rhs.mChunkBase[2] ), //( m02 * v.z   +
                mChunkBase[3] ) ) , invW ),                     //  m03 ) * fInvW
            //Y
            _mm_mul_ps(
            _mm_add_ps( _mm_add_ps(
                _mm_mul_ps( mChunkBase[4], rhs.mChunkBase[0] ), //( m10 * v.x   +
                _mm_mul_ps( mChunkBase[5], rhs.mChunkBase[1] ) ),   //  m11 * v.y ) +
            _mm_add_ps(
                _mm_mul_ps( mChunkBase[6], rhs.mChunkBase[2] ), //( m12 * v.z   +
                mChunkBase[7] ) ), invW ),                          //  m13 ) * fInvW
            //Z
            _mm_mul_ps(
            _mm_add_ps( _mm_add_ps(
                _mm_mul_ps( mChunkBase[8], rhs.mChunkBase[0] ), //( m20 * v.x   +
                _mm_mul_ps( mChunkBase[9], rhs.mChunkBase[1] ) ),   //  m21 * v.y ) +
            _mm_add_ps(
                _mm_mul_ps( mChunkBase[10], rhs.mChunkBase[2] ),    //( m22 * v.z   +
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
        ArrayReal fTx  = _mm_add_ps( qChunkBase[1], qChunkBase[1] );        // 2 * x
        ArrayReal fTy  = _mm_add_ps( qChunkBase[2], qChunkBase[2] );        // 2 * y
        ArrayReal fTz  = _mm_add_ps( qChunkBase[3], qChunkBase[3] );        // 2 * z
        ArrayReal fTwx = _mm_mul_ps( fTx, qChunkBase[0] );                  // fTx*w;
        ArrayReal fTwy = _mm_mul_ps( fTy, qChunkBase[0] );                  // fTy*w;
        ArrayReal fTwz = _mm_mul_ps( fTz, qChunkBase[0] );                  // fTz*w;
        ArrayReal fTxx = _mm_mul_ps( fTx, qChunkBase[1] );                  // fTx*x;
        ArrayReal fTxy = _mm_mul_ps( fTy, qChunkBase[1] );                  // fTy*x;
        ArrayReal fTxz = _mm_mul_ps( fTz, qChunkBase[1] );                  // fTz*x;
        ArrayReal fTyy = _mm_mul_ps( fTy, qChunkBase[2] );                  // fTy*y;
        ArrayReal fTyz = _mm_mul_ps( fTz, qChunkBase[2] );                  // fTz*y;
        ArrayReal fTzz = _mm_mul_ps( fTz, qChunkBase[3] );                  // fTz*z;

        chunkBase[0] = _mm_sub_ps( MathlibSSE2::ONE, _mm_add_ps( fTyy, fTzz ) );
        chunkBase[1] = _mm_sub_ps( fTxy, fTwz );
        chunkBase[2] = _mm_add_ps( fTxz, fTwy );
        chunkBase[4] = _mm_add_ps( fTxy, fTwz );
        chunkBase[5] = _mm_sub_ps( MathlibSSE2::ONE, _mm_add_ps( fTxx, fTzz ) );
        chunkBase[6] = _mm_sub_ps( fTyz, fTwx );
        chunkBase[8] = _mm_sub_ps( fTxz, fTwy );
        chunkBase[9] = _mm_add_ps( fTyz, fTwx );
        chunkBase[10]= _mm_sub_ps( MathlibSSE2::ONE, _mm_add_ps( fTxx, fTyy ) );
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayMatrix4::makeTransform( const ArrayVector3 &position, const ArrayVector3 &scale,
                                             const ArrayQuaternion &orientation )
    {
        ArrayReal * RESTRICT_ALIAS chunkBase            = mChunkBase;
        const ArrayReal * RESTRICT_ALIAS posChunkBase   = position.mChunkBase;
        const ArrayReal * RESTRICT_ALIAS scaleChunkBase = scale.mChunkBase;
        this->fromQuaternion( orientation );
        chunkBase[0] = _mm_mul_ps( chunkBase[0], scaleChunkBase[0] );   //m00 * scale.x
        chunkBase[1] = _mm_mul_ps( chunkBase[1], scaleChunkBase[1] );   //m01 * scale.y
        chunkBase[2] = _mm_mul_ps( chunkBase[2], scaleChunkBase[2] );   //m02 * scale.z
        chunkBase[3] =  posChunkBase[0];                                //m03 * pos.x

        chunkBase[4] = _mm_mul_ps( chunkBase[4], scaleChunkBase[0] );   //m10 * scale.x
        chunkBase[5] = _mm_mul_ps( chunkBase[5], scaleChunkBase[1] );   //m11 * scale.y
        chunkBase[6] = _mm_mul_ps( chunkBase[6], scaleChunkBase[2] );   //m12 * scale.z
        chunkBase[7] =  posChunkBase[1];                                //m13 * pos.y

        chunkBase[8] = _mm_mul_ps( chunkBase[8], scaleChunkBase[0] );   //m20 * scale.x
        chunkBase[9] = _mm_mul_ps( chunkBase[9], scaleChunkBase[1] );   //m21 * scale.y
        chunkBase[10]= _mm_mul_ps( chunkBase[10],scaleChunkBase[2] );   //m22 * scale.z
        chunkBase[11]=  posChunkBase[2];                                //m23 * pos.z

        // No projection term
        chunkBase[12] = mChunkBase[13] = mChunkBase[14] = _mm_setzero_ps();
        chunkBase[15] = MathlibSSE2::ONE;
    }
    //-----------------------------------------------------------------------------------
    inline bool ArrayMatrix4::isAffine() const
    {
        ArrayReal mask = 
            _mm_and_ps(
                _mm_and_ps( _mm_cmpeq_ps( mChunkBase[12], _mm_setzero_ps() ),
                    _mm_cmpeq_ps( mChunkBase[13], _mm_setzero_ps() ) ),
                _mm_and_ps( _mm_cmpeq_ps( mChunkBase[14], _mm_setzero_ps() ),
                    _mm_cmpeq_ps( mChunkBase[15], MathlibSSE2::ONE ) ) );

        return _mm_movemask_ps( mask ) == 0x0f;
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayMatrix4::storeToAoS( Matrix4 * RESTRICT_ALIAS dst ) const
    {
        //Do not use the unpack version, use the shuffle. Shuffle is faster in k10 processors
        //("The conceptual shuffle" http://developer.amd.com/community/blog/the-conceptual-shuffle/)
        //and the unpack version uses 64-bit moves, which can cause store forwarding issues when
        //then loading them with 128-bit movaps
#define _MM_TRANSPOSE4_SRC_DST_PS(row0, row1, row2, row3, dst0, dst1, dst2, dst3) { \
            __m128 tmp3, tmp2, tmp1, tmp0;                          \
                                                                    \
            tmp0   = _mm_shuffle_ps((row0), (row1), 0x44);          \
            tmp2   = _mm_shuffle_ps((row0), (row1), 0xEE);          \
            tmp1   = _mm_shuffle_ps((row2), (row3), 0x44);          \
            tmp3   = _mm_shuffle_ps((row2), (row3), 0xEE);          \
                                                                    \
            (dst0) = _mm_shuffle_ps(tmp0, tmp1, 0x88);              \
            (dst1) = _mm_shuffle_ps(tmp0, tmp1, 0xDD);              \
            (dst2) = _mm_shuffle_ps(tmp2, tmp3, 0x88);              \
            (dst3) = _mm_shuffle_ps(tmp2, tmp3, 0xDD);              \
        }
        register ArrayReal m0, m1, m2, m3;

        _MM_TRANSPOSE4_SRC_DST_PS(
                            this->mChunkBase[0], this->mChunkBase[1],
                            this->mChunkBase[2], this->mChunkBase[3],
                            m0, m1, m2, m3 );
        _mm_stream_ps( dst[0]._m, m0 );
        _mm_stream_ps( dst[1]._m, m1 );
        _mm_stream_ps( dst[2]._m, m2 );
        _mm_stream_ps( dst[3]._m, m3 );
        _MM_TRANSPOSE4_SRC_DST_PS(
                            this->mChunkBase[4], this->mChunkBase[5],
                            this->mChunkBase[6], this->mChunkBase[7],
                            m0, m1, m2, m3 );
        _mm_stream_ps( dst[0]._m+4, m0 );
        _mm_stream_ps( dst[1]._m+4, m1 );
        _mm_stream_ps( dst[2]._m+4, m2 );
        _mm_stream_ps( dst[3]._m+4, m3 );
        _MM_TRANSPOSE4_SRC_DST_PS(
                            this->mChunkBase[8], this->mChunkBase[9],
                            this->mChunkBase[10], this->mChunkBase[11],
                            m0, m1, m2, m3 );
        _mm_stream_ps( dst[0]._m+8, m0 );
        _mm_stream_ps( dst[1]._m+8, m1 );
        _mm_stream_ps( dst[2]._m+8, m2 );
        _mm_stream_ps( dst[3]._m+8, m3 );
        _MM_TRANSPOSE4_SRC_DST_PS(
                            this->mChunkBase[12], this->mChunkBase[13],
                            this->mChunkBase[14], this->mChunkBase[15],
                            m0, m1, m2, m3 );
        _mm_stream_ps( dst[0]._m+12, m0 );
        _mm_stream_ps( dst[1]._m+12, m1 );
        _mm_stream_ps( dst[2]._m+12, m2 );
        _mm_stream_ps( dst[3]._m+12, m3 );
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayMatrix4::loadFromAoS( const Matrix4 * RESTRICT_ALIAS src )
    {
        _MM_TRANSPOSE4_SRC_DST_PS(
                            _mm_load_ps( src[0]._m ), _mm_load_ps( src[1]._m ), 
                            _mm_load_ps( src[2]._m ), _mm_load_ps( src[3]._m ),
                            this->mChunkBase[0], this->mChunkBase[1],
                            this->mChunkBase[2], this->mChunkBase[3] );
        _MM_TRANSPOSE4_SRC_DST_PS(
                            _mm_load_ps( src[0]._m+4 ), _mm_load_ps( src[1]._m+4 ), 
                            _mm_load_ps( src[2]._m+4 ), _mm_load_ps( src[3]._m+4 ),
                            this->mChunkBase[4], this->mChunkBase[5],
                            this->mChunkBase[6], this->mChunkBase[7] );
        _MM_TRANSPOSE4_SRC_DST_PS(
                            _mm_load_ps( src[0]._m+8 ), _mm_load_ps( src[1]._m+8 ), 
                            _mm_load_ps( src[2]._m+8 ), _mm_load_ps( src[3]._m+8 ),
                            this->mChunkBase[8], this->mChunkBase[9],
                            this->mChunkBase[10], this->mChunkBase[11] );
        _MM_TRANSPOSE4_SRC_DST_PS(
                            _mm_load_ps( src[0]._m+12 ), _mm_load_ps( src[1]._m+12 ), 
                            _mm_load_ps( src[2]._m+12 ), _mm_load_ps( src[3]._m+12 ),
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
}
