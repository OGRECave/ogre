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
                ( lhsChunkBase[0] * rhsChunkBase[0] + lhsChunkBase[1] * rhsChunkBase[4] ) +
                ( lhsChunkBase[2] * rhsChunkBase[8] + lhsChunkBase[3] * rhsChunkBase[12] );
        outChunkBase[1] =
                ( lhsChunkBase[0] * rhsChunkBase[1] + lhsChunkBase[1] * rhsChunkBase[5] ) +
                ( lhsChunkBase[2] * rhsChunkBase[9] + lhsChunkBase[3] * rhsChunkBase[13] );
        outChunkBase[2] =
                ( lhsChunkBase[0] * rhsChunkBase[2] + lhsChunkBase[1] * rhsChunkBase[6] ) +
                ( lhsChunkBase[2] * rhsChunkBase[10] + lhsChunkBase[3] * rhsChunkBase[14] );
        outChunkBase[3] =
                ( lhsChunkBase[0] * rhsChunkBase[3] + lhsChunkBase[1] * rhsChunkBase[7] ) +
                ( lhsChunkBase[2] * rhsChunkBase[11] + lhsChunkBase[3] * rhsChunkBase[15] );

        /* Next row (1) */
        outChunkBase[4] =
                ( lhsChunkBase[4] * rhsChunkBase[0] + lhsChunkBase[5] * rhsChunkBase[4] ) +
                ( lhsChunkBase[6] * rhsChunkBase[8] + lhsChunkBase[7] * rhsChunkBase[12] );
        outChunkBase[5] =
                ( lhsChunkBase[4] * rhsChunkBase[1] + lhsChunkBase[5] * rhsChunkBase[5] ) +
                ( lhsChunkBase[6] * rhsChunkBase[9] + lhsChunkBase[7] * rhsChunkBase[13] );
        outChunkBase[6] =
                ( lhsChunkBase[4] * rhsChunkBase[2] + lhsChunkBase[5] * rhsChunkBase[6] ) +
                ( lhsChunkBase[6] * rhsChunkBase[10] + lhsChunkBase[7] * rhsChunkBase[14] );
        outChunkBase[7] =
                ( lhsChunkBase[4] * rhsChunkBase[3] + lhsChunkBase[5] * rhsChunkBase[7] ) +
                ( lhsChunkBase[6] * rhsChunkBase[11] + lhsChunkBase[7] * rhsChunkBase[15] );

        /* Next row (2) */
        outChunkBase[8] =
                ( lhsChunkBase[8] * rhsChunkBase[0] + lhsChunkBase[9] * rhsChunkBase[4] ) +
                ( lhsChunkBase[10] * rhsChunkBase[8] + lhsChunkBase[11] * rhsChunkBase[12] );
        outChunkBase[9] =
                ( lhsChunkBase[8] * rhsChunkBase[1] + lhsChunkBase[9] * rhsChunkBase[5] ) +
                ( lhsChunkBase[10] * rhsChunkBase[9] + lhsChunkBase[11] * rhsChunkBase[13] );
        outChunkBase[10] =
                ( lhsChunkBase[8] * rhsChunkBase[2] + lhsChunkBase[9] * rhsChunkBase[6] ) +
                ( lhsChunkBase[10] * rhsChunkBase[10] + lhsChunkBase[11] * rhsChunkBase[14] );
        outChunkBase[11] =
                ( lhsChunkBase[8] * rhsChunkBase[3] + lhsChunkBase[9] * rhsChunkBase[7] ) +
                ( lhsChunkBase[10] * rhsChunkBase[11] + lhsChunkBase[11] * rhsChunkBase[15] );

        /* Next row (3) */
        outChunkBase[12] =
                ( lhsChunkBase[12] * rhsChunkBase[0] + lhsChunkBase[13] * rhsChunkBase[4] ) +
                ( lhsChunkBase[14] * rhsChunkBase[8] + lhsChunkBase[15] * rhsChunkBase[12] );
        outChunkBase[13] =
                ( lhsChunkBase[12] * rhsChunkBase[1] + lhsChunkBase[13] * rhsChunkBase[5] ) +
                ( lhsChunkBase[14] * rhsChunkBase[9] + lhsChunkBase[15] * rhsChunkBase[13] );
        outChunkBase[14] =
                ( lhsChunkBase[12] * rhsChunkBase[2] + lhsChunkBase[13] * rhsChunkBase[6] ) +
                ( lhsChunkBase[14] * rhsChunkBase[10] + lhsChunkBase[15] * rhsChunkBase[14] );
        outChunkBase[15] =
                ( lhsChunkBase[12] * rhsChunkBase[3] + lhsChunkBase[13] * rhsChunkBase[7] ) +
                ( lhsChunkBase[14] * rhsChunkBase[11] + lhsChunkBase[15] * rhsChunkBase[15] );
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
                ( lhsChunkBase[0] * rhsChunkBase[0] + lhsChunkBase[1] * rhsChunkBase[4] ) +
                ( lhsChunkBase[2] * rhsChunkBase[8] + lhsChunkBase[3] * rhsChunkBase[12] );
        ArrayReal lhs1 = lhsChunkBase[1];
        lhsChunkBase[1] =
                ( lhs0 * rhsChunkBase[1] + lhsChunkBase[1] * rhsChunkBase[5] ) +
                ( lhsChunkBase[2] * rhsChunkBase[9] + lhsChunkBase[3] * rhsChunkBase[13] );
        ArrayReal lhs2 = lhsChunkBase[2];
        lhsChunkBase[2] =
                ( lhs0 * rhsChunkBase[2] + lhs1 * rhsChunkBase[6] ) +
                ( lhsChunkBase[2] * rhsChunkBase[10] + lhsChunkBase[3] * rhsChunkBase[14] );
        lhsChunkBase[3] =
                ( lhs0 * rhsChunkBase[3] + lhs1 * rhsChunkBase[7] ) +
                ( lhs2 * rhsChunkBase[11] + lhsChunkBase[3] * rhsChunkBase[15] );

        /* Next row (1) */
        lhs0 = lhsChunkBase[4];
        lhsChunkBase[4] =
                ( lhsChunkBase[4] * rhsChunkBase[0] + lhsChunkBase[5] * rhsChunkBase[4] ) +
                ( lhsChunkBase[6] * rhsChunkBase[8] + lhsChunkBase[7] * rhsChunkBase[12] );
        lhs1 = lhsChunkBase[5];
        lhsChunkBase[5] =
                ( lhs0 * rhsChunkBase[1] + lhsChunkBase[5] * rhsChunkBase[5] ) +
                ( lhsChunkBase[6] * rhsChunkBase[9] + lhsChunkBase[7] * rhsChunkBase[13] );
        lhs2 = lhsChunkBase[6];
        lhsChunkBase[6] =
                ( lhs0 * rhsChunkBase[2] + lhs1 * rhsChunkBase[6] ) +
                ( lhsChunkBase[6] * rhsChunkBase[10] + lhsChunkBase[7] * rhsChunkBase[14] );
        lhsChunkBase[7] =
                ( lhs0 * rhsChunkBase[3] + lhs1 * rhsChunkBase[7] ) +
                ( lhs2 * rhsChunkBase[11] + lhsChunkBase[7] * rhsChunkBase[15] );

        /* Next row (2) */
        lhs0 = lhsChunkBase[8];
        lhsChunkBase[8] =
                ( lhsChunkBase[8] * rhsChunkBase[0] + lhsChunkBase[9] * rhsChunkBase[4] ) +
                ( lhsChunkBase[10] * rhsChunkBase[8] + lhsChunkBase[11] * rhsChunkBase[12] );
        lhs1 = lhsChunkBase[9];
        lhsChunkBase[9] =
                ( lhs0 * rhsChunkBase[1] + lhsChunkBase[9] * rhsChunkBase[5] ) +
                ( lhsChunkBase[10] * rhsChunkBase[9] + lhsChunkBase[11] * rhsChunkBase[13] );
        lhs2 = lhsChunkBase[10];
        lhsChunkBase[10] =
                ( lhs0 * rhsChunkBase[2] + lhs1 * rhsChunkBase[6] ) +
                ( lhsChunkBase[10] * rhsChunkBase[10] + lhsChunkBase[11] * rhsChunkBase[14] );
        lhsChunkBase[11] =
                ( lhs0 * rhsChunkBase[3] + lhs1 * rhsChunkBase[7] ) +
                ( lhs2 * rhsChunkBase[11] + lhsChunkBase[11] * rhsChunkBase[15] );

        /* Next row (3) */
        lhs0 = lhsChunkBase[12];
        lhsChunkBase[12] =
                ( lhsChunkBase[12] * rhsChunkBase[0] + lhsChunkBase[13] * rhsChunkBase[4] ) +
                ( lhsChunkBase[14] * rhsChunkBase[8] + lhsChunkBase[15] * rhsChunkBase[12] );
        lhs1 = lhsChunkBase[13];
        lhsChunkBase[13] =
                ( lhs0 * rhsChunkBase[1] + lhsChunkBase[13] * rhsChunkBase[5] ) +
                ( lhsChunkBase[14] * rhsChunkBase[9] + lhsChunkBase[15] * rhsChunkBase[13] );
        lhs2 = lhsChunkBase[14];
        lhsChunkBase[14] =
                ( lhs0 * rhsChunkBase[2] + lhs1 * rhsChunkBase[6] ) +
                ( lhsChunkBase[14] * rhsChunkBase[10] + lhsChunkBase[15] * rhsChunkBase[14] );
        lhsChunkBase[15] =
                ( lhs0 * rhsChunkBase[3] + lhs1 * rhsChunkBase[7] ) +
                ( lhs2 * rhsChunkBase[11] + lhsChunkBase[15] * rhsChunkBase[15] );
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
        ArrayReal invW = ( mChunkBase[12] * rhs.mChunkBase[0] + mChunkBase[13] * rhs.mChunkBase[1] )
                            + ( mChunkBase[14] * rhs.mChunkBase[2] + mChunkBase[15] );
        invW = MathlibC::Inv4( invW );

        return ArrayVector3(
            //X = ( m00 * v.x + m01 * v.y + m02 * v.z + m03 ) * fInvW
            ( mChunkBase[0] * rhs.mChunkBase[0] + mChunkBase[1] * rhs.mChunkBase[1] +
              mChunkBase[2] * rhs.mChunkBase[2] + mChunkBase[3] ) * invW,
            //Y = ( m10 * v.x + m11 * v.y + m12 * v.z + m13 ) * fInvW
            ( mChunkBase[4] * rhs.mChunkBase[0] + mChunkBase[5] * rhs.mChunkBase[1] +
              mChunkBase[6] * rhs.mChunkBase[2] + mChunkBase[7] ) * invW,
            //Z = ( m20 * v.x + m21 * v.y + m22 * v.z + m23 ) * fInvW
            ( mChunkBase[8] * rhs.mChunkBase[0] + mChunkBase[9] * rhs.mChunkBase[1] +
              mChunkBase[10] * rhs.mChunkBase[2] + mChunkBase[11] ) * invW );
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
    inline void ArrayMatrix4::makeTransform( const ArrayVector3 &position, const ArrayVector3 &scale,
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

        // No projection term
        chunkBase[12] = mChunkBase[13] = mChunkBase[14] = 0.0f;
        chunkBase[15] = 1.0f;
    }
    //-----------------------------------------------------------------------------------
    inline bool ArrayMatrix4::isAffine() const
    {
        return (mChunkBase[12] == 0) & (mChunkBase[13] == 0) &
                (mChunkBase[14] == 0) & (mChunkBase[15] == 1.0f);
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayMatrix4::storeToAoS( Matrix4 * RESTRICT_ALIAS dst ) const
    {
        this->getAsMatrix4( *dst, 0 );
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayMatrix4::loadFromAoS( const Matrix4 * RESTRICT_ALIAS src )
    {
        this->setFromMatrix4( *src, 0 );
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayMatrix4::loadFromAoS( const SimpleMatrix4 * RESTRICT_ALIAS src )
    {
        mChunkBase[0]  = src->mChunkBase[0];
        mChunkBase[1]  = src->mChunkBase[1];
        mChunkBase[2]  = src->mChunkBase[2];
        mChunkBase[3]  = src->mChunkBase[3];
        mChunkBase[4]  = src->mChunkBase[4];
        mChunkBase[5]  = src->mChunkBase[5];
        mChunkBase[6]  = src->mChunkBase[6];
        mChunkBase[7]  = src->mChunkBase[7];
        mChunkBase[8]  = src->mChunkBase[8];
        mChunkBase[9]  = src->mChunkBase[9];
        mChunkBase[10] = src->mChunkBase[10];
        mChunkBase[11] = src->mChunkBase[11];
        mChunkBase[12] = src->mChunkBase[12];
        mChunkBase[13] = src->mChunkBase[13];
        mChunkBase[14] = src->mChunkBase[14];
        mChunkBase[15] = src->mChunkBase[15];
    }
    //-----------------------------------------------------------------------------------
}
