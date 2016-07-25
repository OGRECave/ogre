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
    /// Concatenates two 4x3 array matrices.
    /// @remarks
    ///     99.99% of the cases the matrix isn't concatenated with itself, therefore it's
    ///     safe to assume &lhs != &rhs. RESTRICT_ALIAS modifier is used (a non-standard
    ///     C++ extension) is used when available to dramatically improve performance,
    ///     particularly of the update operations ( a *= b )
    ///     This function will assert if OGRE_RESTRICT_ALIASING is enabled and any of the
    ///     given pointers point to the same location
    inline void concatArrayMatAf4x3 ( ArrayReal * RESTRICT_ALIAS outChunkBase,
                                        const ArrayReal * RESTRICT_ALIAS lhsChunkBase,
                                        const ArrayReal * RESTRICT_ALIAS rhsChunkBase )
    {
#if OGRE_RESTRICT_ALIASING != 0
        assert( outChunkBase != lhsChunkBase && outChunkBase != rhsChunkBase &&
                lhsChunkBase != rhsChunkBase &&
                "Re-strict aliasing rule broken. Compile without OGRE_RESTRICT_ALIASING" );
#endif
        outChunkBase[0] =
                ( lhsChunkBase[0] * rhsChunkBase[0] + lhsChunkBase[1] * rhsChunkBase[4] ) +
                ( lhsChunkBase[2] * rhsChunkBase[8] );
        outChunkBase[1] =
                ( lhsChunkBase[0] * rhsChunkBase[1] + lhsChunkBase[1] * rhsChunkBase[5] ) +
                ( lhsChunkBase[2] * rhsChunkBase[9] );
        outChunkBase[2] =
                ( lhsChunkBase[0] * rhsChunkBase[2] + lhsChunkBase[1] * rhsChunkBase[6] ) +
                ( lhsChunkBase[2] * rhsChunkBase[10] );
        outChunkBase[3] =
                ( lhsChunkBase[0] * rhsChunkBase[3] + lhsChunkBase[1] * rhsChunkBase[7] ) +
                ( lhsChunkBase[2] * rhsChunkBase[11] + lhsChunkBase[3] );

        /* Next row (1) */
        outChunkBase[4] =
                ( lhsChunkBase[4] * rhsChunkBase[0] + lhsChunkBase[5] * rhsChunkBase[4] ) +
                ( lhsChunkBase[6] * rhsChunkBase[8] );
        outChunkBase[5] =
                ( lhsChunkBase[4] * rhsChunkBase[1] + lhsChunkBase[5] * rhsChunkBase[5] ) +
                ( lhsChunkBase[6] * rhsChunkBase[9] );
        outChunkBase[6] =
                ( lhsChunkBase[4] * rhsChunkBase[2] + lhsChunkBase[5] * rhsChunkBase[6] ) +
                ( lhsChunkBase[6] * rhsChunkBase[10] );
        outChunkBase[7] =
                ( lhsChunkBase[4] * rhsChunkBase[3] + lhsChunkBase[5] * rhsChunkBase[7] ) +
                ( lhsChunkBase[6] * rhsChunkBase[11] + lhsChunkBase[7] );

        /* Next row (2) */
        outChunkBase[8] =
                ( lhsChunkBase[8] * rhsChunkBase[0] + lhsChunkBase[9] * rhsChunkBase[4] ) +
                ( lhsChunkBase[10] * rhsChunkBase[8] );
        outChunkBase[9] =
                ( lhsChunkBase[8] * rhsChunkBase[1] + lhsChunkBase[9] * rhsChunkBase[5] ) +
                ( lhsChunkBase[10] * rhsChunkBase[9] );
        outChunkBase[10] =
                ( lhsChunkBase[8] * rhsChunkBase[2] + lhsChunkBase[9] * rhsChunkBase[6] ) +
                ( lhsChunkBase[10] * rhsChunkBase[10] );
        outChunkBase[11] =
                ( lhsChunkBase[8] * rhsChunkBase[3] + lhsChunkBase[9] * rhsChunkBase[7] ) +
                ( lhsChunkBase[10] * rhsChunkBase[11] + lhsChunkBase[11] );
    }

    /// Update version
    inline void concatArrayMatAf4x3( ArrayReal * RESTRICT_ALIAS lhsChunkBase,
                                     const ArrayReal * RESTRICT_ALIAS rhsChunkBase )
    {
#if OGRE_RESTRICT_ALIASING != 0
        assert( lhsChunkBase != rhsChunkBase &&
                "Re-strict aliasing rule broken. Compile without OGRE_RESTRICT_ALIASING" );
#endif
        ArrayReal lhs0 = lhsChunkBase[0];
        lhsChunkBase[0] =
                ( lhsChunkBase[0] * rhsChunkBase[0] + lhsChunkBase[1] * rhsChunkBase[4] ) +
                ( lhsChunkBase[2] * rhsChunkBase[8] );
        ArrayReal lhs1 = lhsChunkBase[1];
        lhsChunkBase[1] =
                ( lhs0 * rhsChunkBase[1] + lhsChunkBase[1] * rhsChunkBase[5] ) +
                ( lhsChunkBase[2] * rhsChunkBase[9] );
        ArrayReal lhs2 = lhsChunkBase[2];
        lhsChunkBase[2] =
                ( lhs0 * rhsChunkBase[2] + lhs1 * rhsChunkBase[6] ) +
                ( lhsChunkBase[2] * rhsChunkBase[10] );
        lhsChunkBase[3] =
                ( lhs0 * rhsChunkBase[3] + lhs1 * rhsChunkBase[7] ) +
                ( lhs2 * rhsChunkBase[11] + lhsChunkBase[3] );

        /* Next row (1) */
        lhs0 = lhsChunkBase[4];
        lhsChunkBase[4] =
                ( lhsChunkBase[4] * rhsChunkBase[0] + lhsChunkBase[5] * rhsChunkBase[4] ) +
                ( lhsChunkBase[6] * rhsChunkBase[8] );
        lhs1 = lhsChunkBase[5];
        lhsChunkBase[5] =
                ( lhs0 * rhsChunkBase[1] + lhsChunkBase[5] * rhsChunkBase[5] ) +
                ( lhsChunkBase[6] * rhsChunkBase[9] );
        lhs2 = lhsChunkBase[6];
        lhsChunkBase[6] =
                ( lhs0 * rhsChunkBase[2] + lhs1 * rhsChunkBase[6] ) +
                ( lhsChunkBase[6] * rhsChunkBase[10] );
        lhsChunkBase[7] =
                ( lhs0 * rhsChunkBase[3] + lhs1 * rhsChunkBase[7] ) +
                ( lhs2 * rhsChunkBase[11] + lhsChunkBase[7] );

        /* Next row (2) */
        lhs0 = lhsChunkBase[8];
        lhsChunkBase[8] =
                ( lhsChunkBase[8] * rhsChunkBase[0] + lhsChunkBase[9] * rhsChunkBase[4] ) +
                ( lhsChunkBase[10] * rhsChunkBase[8] );
        lhs1 = lhsChunkBase[9];
        lhsChunkBase[9] =
                ( lhs0 * rhsChunkBase[1] + lhsChunkBase[9] * rhsChunkBase[5] ) +
                ( lhsChunkBase[10] * rhsChunkBase[9] );
        lhs2 = lhsChunkBase[10];
        lhsChunkBase[10] =
                ( lhs0 * rhsChunkBase[2] + lhs1 * rhsChunkBase[6] ) +
                ( lhsChunkBase[10] * rhsChunkBase[10] );
        lhsChunkBase[11] =
                ( lhs0 * rhsChunkBase[3] + lhs1 * rhsChunkBase[7] ) +
                ( lhs2 * rhsChunkBase[11] + lhsChunkBase[11] );
    }

    inline ArrayMatrixAf4x3 operator * ( const ArrayMatrixAf4x3 &lhs, const ArrayMatrixAf4x3 &rhs )
    {
        ArrayMatrixAf4x3 retVal;
        concatArrayMatAf4x3( retVal.mChunkBase, lhs.mChunkBase, rhs.mChunkBase );
        return retVal;
    }
    //-----------------------------------------------------------------------------------
    inline ArrayVector3 ArrayMatrixAf4x3::operator * ( const ArrayVector3 &rhs ) const
    {
        return ArrayVector3(
            //X = ( m00 * v.x + m01 * v.y + m02 * v.z + m03 )
            ( mChunkBase[0] * rhs.mChunkBase[0] + mChunkBase[1] * rhs.mChunkBase[1] +
              mChunkBase[2] * rhs.mChunkBase[2] + mChunkBase[3] ),
            //Y = ( m10 * v.x + m11 * v.y + m12 * v.z + m13 )
            ( mChunkBase[4] * rhs.mChunkBase[0] + mChunkBase[5] * rhs.mChunkBase[1] +
              mChunkBase[6] * rhs.mChunkBase[2] + mChunkBase[7] ),
            //Z = ( m20 * v.x + m21 * v.y + m22 * v.z + m23 )
            ( mChunkBase[8] * rhs.mChunkBase[0] + mChunkBase[9] * rhs.mChunkBase[1] +
              mChunkBase[10] * rhs.mChunkBase[2] + mChunkBase[11] ) );
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayMatrixAf4x3::operator *= ( const ArrayMatrixAf4x3 &rhs )
    {
        concatArrayMatAf4x3( mChunkBase, rhs.mChunkBase );
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayMatrixAf4x3::fromQuaternion( const ArrayQuaternion &q )
    {
        ArrayReal * RESTRICT_ALIAS chunkBase = mChunkBase;
        const ArrayReal * RESTRICT_ALIAS qChunkBase = &q.w;
        ArrayReal fTx  = qChunkBase[1] + qChunkBase[1];         // 2 * x
        ArrayReal fTy  = qChunkBase[2] + qChunkBase[2];         // 2 * y
        ArrayReal fTz  = qChunkBase[3] + qChunkBase[3];         // 2 * z
        ArrayReal fTwx = fTx * qChunkBase[0];                   // fTx*w;
        ArrayReal fTwy = fTy * qChunkBase[0];                   // fTy*w;
        ArrayReal fTwz = fTz * qChunkBase[0];                   // fTz*w;
        ArrayReal fTxx = fTx * qChunkBase[1];                   // fTx*x;
        ArrayReal fTxy = fTy * qChunkBase[1];                   // fTy*x;
        ArrayReal fTxz = fTz * qChunkBase[1];                   // fTz*x;
        ArrayReal fTyy = fTy * qChunkBase[2];                   // fTy*y;
        ArrayReal fTyz = fTz * qChunkBase[2];                   // fTz*y;
        ArrayReal fTzz = fTz * qChunkBase[3];                   // fTz*z;

        chunkBase[0] = 1.0f - ( fTyy + fTzz );
        chunkBase[1] = fTxy - fTwz;
        chunkBase[2] = fTxz + fTwy;
        chunkBase[4] = fTxy + fTwz;
        chunkBase[5] = 1.0f - ( fTxx + fTzz );
        chunkBase[6] = fTyz - fTwx;
        chunkBase[8] = fTxz - fTwy;
        chunkBase[9] = fTyz + fTwx;
        chunkBase[10]= 1.0f - ( fTxx + fTyy );
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayMatrixAf4x3::makeTransform( const ArrayVector3 &position, const ArrayVector3 &scale,
                                             const ArrayQuaternion &orientation )
    {
        ArrayReal * RESTRICT_ALIAS chunkBase            = mChunkBase;
        const ArrayReal * RESTRICT_ALIAS posChunkBase   = position.mChunkBase;
        const ArrayReal * RESTRICT_ALIAS scaleChunkBase = scale.mChunkBase;
        this->fromQuaternion( orientation );
        chunkBase[0] = chunkBase[0] * scaleChunkBase[0];    //m00 * scale.x
        chunkBase[1] = chunkBase[1] * scaleChunkBase[1];    //m01 * scale.y
        chunkBase[2] = chunkBase[2] * scaleChunkBase[2];    //m02 * scale.z
        chunkBase[3] = posChunkBase[0];                     //m03 * pos.x

        chunkBase[4] = chunkBase[4] * scaleChunkBase[0];    //m10 * scale.x
        chunkBase[5] = chunkBase[5] * scaleChunkBase[1];    //m11 * scale.y
        chunkBase[6] = chunkBase[6] * scaleChunkBase[2];    //m12 * scale.z
        chunkBase[7] = posChunkBase[1];                     //m13 * pos.y

        chunkBase[8] = chunkBase[8] * scaleChunkBase[0];    //m20 * scale.x
        chunkBase[9] = chunkBase[9] * scaleChunkBase[1];    //m21 * scale.y
        chunkBase[10]= chunkBase[10]* scaleChunkBase[2];    //m22 * scale.z
        chunkBase[11]= posChunkBase[2];                     //m23 * pos.z
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayMatrixAf4x3::decomposition( ArrayVector3 &position, ArrayVector3 &scale,
                                                 ArrayQuaternion &orientation ) const
    {
        const ArrayReal * RESTRICT_ALIAS chunkBase  = mChunkBase;
        // build orthogonal matrix Q

        ArrayReal m00 = mChunkBase[0], m01 = mChunkBase[1], m02 = mChunkBase[2];
        ArrayReal m10 = mChunkBase[4], m11 = mChunkBase[5], m12 = mChunkBase[6];
        ArrayReal m20 = mChunkBase[8], m21 = mChunkBase[9], m22 = mChunkBase[10];

        ArrayReal fInvLength = 1.0f / sqrt( m00 * m00 + m10 * m10 + m20 * m20 );

        ArrayReal q00, q01, q02,
                  q10, q11, q12,
                  q20, q21, q22; //3x3 matrix
        q00 = m00 * fInvLength;
        q10 = m10 * fInvLength;
        q20 = m20 * fInvLength;

        ArrayReal fDot = q00 * m01 + q10 * m11 + q20 * m21;
        q01 = m01 - fDot * q00;
        q11 = m11 - fDot * q10;
        q21 = m21 - fDot * q20;

        fInvLength = 1.0f / sqrt( q01 * q01 + q11 * q11 + q21 * q21 );

        q01 *= fInvLength;
        q11 *= fInvLength;
        q21 *= fInvLength;

        fDot = q00 * m02 + q10 * m12 + q20 * m22;
        q02 = m02 - fDot * q00;
        q12 = m12 - fDot * q10;
        q22 = m22 - fDot * q20;

        fDot = q01 * m02 + q11 * m12 + q21 * m22;
        q02 = q02 - fDot * q01;
        q12 = q12 - fDot * q11;
        q22 = q22 - fDot * q21;

        fInvLength = 1.0f / sqrt( q02 * q02 + q12 * q12 + q22 * q22 );

        q02 *= fInvLength;
        q12 *= fInvLength;
        q22 *= fInvLength;

        // guarantee that orthogonal matrix has determinant 1 (no reflections)
        //fDet = q00*q11*q22 + q01*q12*q20 +
        //       q02*q10*q21 - q02*q11*q20 -
        //       q01*q10*q22 - q00*q12*q21;
        ArrayReal fDet = (q00*q11*q22 + q01*q12*q20 + q02*q10*q21) -
                         (q02*q11*q20 + q01*q10*q22 + q00*q12*q21);

        //if ( fDet < 0.0 )
        //{
        //    for (size_t iRow = 0; iRow < 3; iRow++)
        //        for (size_t iCol = 0; iCol < 3; iCol++)
        //            kQ[iRow][iCol] = -kQ[iRow][iCol];
        //}
        fDet = fDet < 0 ? -1.0f : 1.0f;
        q00 = q00 * fDet;
        q01 = q01 * fDet;
        q02 = q02 * fDet;
        q10 = q10 * fDet;
        q11 = q11 * fDet;
        q12 = q12 * fDet;
        q20 = q20 * fDet;
        q21 = q21 * fDet;
        q22 = q22 * fDet;

        const ArrayReal matrix[9] = { q00, q01, q02,
                                      q10, q11, q12,
                                      q20, q21, q22 };
        orientation.FromOrthoDet1RotationMatrix( matrix );

        ArrayReal * RESTRICT_ALIAS scaleChunkBase = scale.mChunkBase;
        scaleChunkBase[0] = q00 * m00 + q10 * m10 + q20 * m20;
        scaleChunkBase[1] = q01 * m01 + q11 * m11 + q21 * m21;
        scaleChunkBase[2] = q02 * m02 + q12 * m12 + q22 * m22;

        ArrayReal * RESTRICT_ALIAS posChunkBase = position.mChunkBase;
        posChunkBase[0] = chunkBase[3];
        posChunkBase[1] = chunkBase[7];
        posChunkBase[2] = chunkBase[11];
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayMatrixAf4x3::setToInverse(void)
    {
        ArrayReal m10 = mChunkBase[4], m11 = mChunkBase[5], m12 = mChunkBase[6];
        ArrayReal m20 = mChunkBase[8], m21 = mChunkBase[9], m22 = mChunkBase[10];

        ArrayReal t00 = m22 * m11 - m21 * m12;
        ArrayReal t10 = m20 * m12 - m22 * m10;
        ArrayReal t20 = m21 * m10 - m20 * m11;

        ArrayReal m00 = mChunkBase[0], m01 = mChunkBase[1], m02 = mChunkBase[2];

        //det = m00 * t00 + m01 * t10 + m02 * t20
        ArrayReal det   = m00 * t00 + m01 * t10 + m02 * t20;
        ArrayReal invDet= 1.0f / det; //High precision division

        t00 = t00 * invDet;
        t10 = t10 * invDet;
        t20 = t20 * invDet;

        m00 = m00 * invDet;
        m01 = m01 * invDet;
        m02 = m02 * invDet;

        ArrayReal r00 = t00;
        ArrayReal r01 = m02 * m21 - m01 * m22;
        ArrayReal r02 = m01 * m12 - m02 * m11;

        ArrayReal r10 = t10;
        ArrayReal r11 = m00 * m22 - m02 * m20;
        ArrayReal r12 = m02 * m10 - m00 * m12;

        ArrayReal r20 = t20;
        ArrayReal r21 = m01 * m20 - m00 * m21;
        ArrayReal r22 = m00 * m11 - m01 * m10;

        ArrayReal m03 = mChunkBase[3], m13 = mChunkBase[7], m23 = mChunkBase[11];

        ArrayReal r03 = -(r00 * m03 + r01 * m13 + r02 * m23);
        ArrayReal r13 = -(r10 * m03 + r11 * m13 + r12 * m23);
        ArrayReal r23 = -(r20 * m03 + r21 * m13 + r22 * m23);

        mChunkBase[0] = r00;
        mChunkBase[1] = r01;
        mChunkBase[2] = r02;
        mChunkBase[3] = r03;

        mChunkBase[4] = r10;
        mChunkBase[5] = r11;
        mChunkBase[6] = r12;
        mChunkBase[7] = r13;

        mChunkBase[8] = r20;
        mChunkBase[9] = r21;
        mChunkBase[10]= r22;
        mChunkBase[11]= r23;
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayMatrixAf4x3::setToInverseDegeneratesAsIdentity(void)
    {
        ArrayReal m10 = mChunkBase[4], m11 = mChunkBase[5], m12 = mChunkBase[6];
        ArrayReal m20 = mChunkBase[8], m21 = mChunkBase[9], m22 = mChunkBase[10];

        ArrayReal t00 = m22 * m11 - m21 * m12;
        ArrayReal t10 = m20 * m12 - m22 * m10;
        ArrayReal t20 = m21 * m10 - m20 * m11;

        ArrayReal m00 = mChunkBase[0], m01 = mChunkBase[1], m02 = mChunkBase[2];

        //det = m00 * t00 + m01 * t10 + m02 * t20
        ArrayReal det   = m00 * t00 + m01 * t10 + m02 * t20;

        if( MathlibC::Abs4( det ) <= MathlibC::fEpsilon )
        {
            mChunkBase[0] = 1;
            mChunkBase[1] = 0;
            mChunkBase[2] = 0;
            mChunkBase[3] = 0;

            mChunkBase[4] = 0;
            mChunkBase[5] = 1;
            mChunkBase[6] = 0;
            mChunkBase[7] = 0;

            mChunkBase[8]  = 0;
            mChunkBase[9]  = 0;
            mChunkBase[10] = 1;
            mChunkBase[11] = 0;
        }
        else
        {
            ArrayReal invDet = 1.0f / det; //High precision division

            t00 = t00 * invDet;
            t10 = t10 * invDet;
            t20 = t20 * invDet;

            m00 = m00 * invDet;
            m01 = m01 * invDet;
            m02 = m02 * invDet;

            ArrayReal r00 = t00;
            ArrayReal r01 = m02 * m21 - m01 * m22;
            ArrayReal r02 = m01 * m12 - m02 * m11;

            ArrayReal r10 = t10;
            ArrayReal r11 = m00 * m22 - m02 * m20;
            ArrayReal r12 = m02 * m10 - m00 * m12;

            ArrayReal r20 = t20;
            ArrayReal r21 = m01 * m20 - m00 * m21;
            ArrayReal r22 = m00 * m11 - m01 * m10;

            ArrayReal m03 = mChunkBase[3], m13 = mChunkBase[7], m23 = mChunkBase[11];

            ArrayReal r03 = -(r00 * m03 + r01 * m13 + r02 * m23);
            ArrayReal r13 = -(r10 * m03 + r11 * m13 + r12 * m23);
            ArrayReal r23 = -(r20 * m03 + r21 * m13 + r22 * m23);

            mChunkBase[0] = r00;
            mChunkBase[1] = r01;
            mChunkBase[2] = r02;
            mChunkBase[3] = r03;

            mChunkBase[4] = r10;
            mChunkBase[5] = r11;
            mChunkBase[6] = r12;
            mChunkBase[7] = r13;

            mChunkBase[8] = r20;
            mChunkBase[9] = r21;
            mChunkBase[10] = r22;
            mChunkBase[11] = r23;
        }
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayMatrixAf4x3::retain( ArrayMaskR orientation, ArrayMaskR scale )
    {
        ArrayVector3 row0( mChunkBase[0], mChunkBase[1], mChunkBase[2] );
        ArrayVector3 row1( mChunkBase[4], mChunkBase[5], mChunkBase[6] );
        ArrayVector3 row2( mChunkBase[8], mChunkBase[9], mChunkBase[10] );

        ArrayVector3 vScale( row0.length(), row1.length(), row2.length() );
        ArrayVector3 vInvScale( vScale );
        vInvScale.inverseLeaveZeroes();

        row0 *= vInvScale.mChunkBase[0];
        row1 *= vInvScale.mChunkBase[1];
        row2 *= vInvScale.mChunkBase[2];

        vScale.Cmov4( scale, ArrayVector3::UNIT_SCALE );

        row0.Cmov4( orientation, ArrayVector3::UNIT_X );
        row1.Cmov4( orientation, ArrayVector3::UNIT_Y );
        row2.Cmov4( orientation, ArrayVector3::UNIT_Z );

        row0 *= vScale.mChunkBase[0];
        row1 *= vScale.mChunkBase[1];
        row2 *= vScale.mChunkBase[2];

        mChunkBase[0] = row0.mChunkBase[0];
        mChunkBase[1] = row0.mChunkBase[1];
        mChunkBase[2] = row0.mChunkBase[2];

        mChunkBase[4] = row1.mChunkBase[0];
        mChunkBase[5] = row1.mChunkBase[1];
        mChunkBase[6] = row1.mChunkBase[2];

        mChunkBase[8] = row2.mChunkBase[0];
        mChunkBase[9] = row2.mChunkBase[1];
        mChunkBase[10]= row2.mChunkBase[2];
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayMatrixAf4x3::streamToAoS( Matrix4 * RESTRICT_ALIAS dst ) const
    {
        for( size_t i=0; i<12; i+=4 )
        {
            dst->_m[i  ] = mChunkBase[ARRAY_PACKED_REALS * (i  )];
            dst->_m[i+1] = mChunkBase[ARRAY_PACKED_REALS * (i+1)];
            dst->_m[i+2] = mChunkBase[ARRAY_PACKED_REALS * (i+2)];
            dst->_m[i+3] = mChunkBase[ARRAY_PACKED_REALS * (i+3)];
        }

        dst->_m[12] = 0;
        dst->_m[13] = 0;
        dst->_m[14] = 0;
        dst->_m[15] = 1;
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayMatrixAf4x3::storeToAoS( SimpleMatrixAf4x3 * RESTRICT_ALIAS dst ) const
    {
        for( int i=0; i<12; i+= 4 )
        {
            dst->mChunkBase[i  ] = mChunkBase[i  ];
            dst->mChunkBase[i+1] = mChunkBase[i+1];
            dst->mChunkBase[i+2] = mChunkBase[i+2];
            dst->mChunkBase[i+3] = mChunkBase[i+3];
        }
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayMatrixAf4x3::streamToAoS( SimpleMatrixAf4x3 * RESTRICT_ALIAS dst ) const
    {
        for( int i=0; i<12; i+= 4 )
        {
            dst->mChunkBase[i  ] = mChunkBase[i  ];
            dst->mChunkBase[i+1] = mChunkBase[i+1];
            dst->mChunkBase[i+2] = mChunkBase[i+2];
            dst->mChunkBase[i+3] = mChunkBase[i+3];
        }
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayMatrixAf4x3::loadFromAoS( const Matrix4 * RESTRICT_ALIAS src )
    {
        for( int i=0; i<12; i+= 4 )
        {
            mChunkBase[i  ] = src->_m[i  ];
            mChunkBase[i+1] = src->_m[i+1];
            mChunkBase[i+2] = src->_m[i+2];
            mChunkBase[i+3] = src->_m[i+3];
        }
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayMatrixAf4x3::loadFromAoS( const Matrix4 * RESTRICT_ALIAS * src )
    {
        for( int i=0; i<12; i+= 4 )
        {
            mChunkBase[i  ] = src[0]->_m[i  ];
            mChunkBase[i+1] = src[0]->_m[i+1];
            mChunkBase[i+2] = src[0]->_m[i+2];
            mChunkBase[i+3] = src[0]->_m[i+3];
        }
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayMatrixAf4x3::loadFromAoS( const SimpleMatrixAf4x3 * RESTRICT_ALIAS src )
    {
        for( int i=0; i<12; i+= 4 )
        {
            mChunkBase[i  ] = src->mChunkBase[i  ];
            mChunkBase[i+1] = src->mChunkBase[i+1];
            mChunkBase[i+2] = src->mChunkBase[i+2];
            mChunkBase[i+3] = src->mChunkBase[i+3];
        }
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayMatrixAf4x3::loadFromAoS( const SimpleMatrixAf4x3 * RESTRICT_ALIAS * src )
    {
        for( int i=0; i<12; i+= 4 )
        {
            mChunkBase[i  ] = src[0]->mChunkBase[i  ];
            mChunkBase[i+1] = src[0]->mChunkBase[i+1];
            mChunkBase[i+2] = src[0]->mChunkBase[i+2];
            mChunkBase[i+3] = src[0]->mChunkBase[i+3];
        }
    }
    //-----------------------------------------------------------------------------------
}
