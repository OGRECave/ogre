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
    // Arithmetic operations
#define DEFINE_OPERATION( leftClass, rightClass, op, op_func )\
    inline ArrayQuaternion operator op ( const leftClass &lhs, const rightClass &rhs )\
    {\
        const ArrayReal * RESTRICT_ALIAS lhsChunkBase = lhs.mChunkBase;\
        const ArrayReal * RESTRICT_ALIAS rhsChunkBase = rhs.mChunkBase;\
        return ArrayQuaternion(\
                op_func( lhsChunkBase[0], rhsChunkBase[0] ),\
                op_func( lhsChunkBase[1], rhsChunkBase[1] ),\
                op_func( lhsChunkBase[2], rhsChunkBase[2] ),\
                op_func( lhsChunkBase[3], rhsChunkBase[3] ) );\
    }
#define DEFINE_L_OPERATION( leftType, rightClass, op, op_func )\
    inline ArrayQuaternion operator op ( const leftType lhs, const rightClass &rhs )\
    {\
        return ArrayQuaternion(\
                op_func( lhs, rhs.mChunkBase[0] ),\
                op_func( lhs, rhs.mChunkBase[1] ),\
                op_func( lhs, rhs.mChunkBase[2] ),\
                op_func( lhs, rhs.mChunkBase[3] ) );\
    }
#define DEFINE_R_OPERATION( leftClass, rightType, op, op_func )\
    inline ArrayQuaternion operator op ( const leftClass &lhs, const rightType rhs )\
    {\
        return ArrayQuaternion(\
                op_func( lhs.mChunkBase[0], rhs ),\
                op_func( lhs.mChunkBase[1], rhs ),\
                op_func( lhs.mChunkBase[2], rhs ),\
                op_func( lhs.mChunkBase[3], rhs ) );\
    }

    // Update operations
#define DEFINE_UPDATE_OPERATION( leftClass, op, op_func )\
    inline void ArrayQuaternion::operator op ( const leftClass &a )\
    {\
        ArrayReal * RESTRICT_ALIAS chunkBase = mChunkBase;\
        const ArrayReal * RESTRICT_ALIAS aChunkBase = a.mChunkBase;\
        chunkBase[0] = op_func( chunkBase[0], aChunkBase[0] );\
        chunkBase[1] = op_func( chunkBase[1], aChunkBase[1] );\
        chunkBase[2] = op_func( chunkBase[2], aChunkBase[2] );\
        chunkBase[3] = op_func( chunkBase[3], aChunkBase[3] );\
    }
#define DEFINE_UPDATE_R_OPERATION( rightType, op, op_func )\
    inline void ArrayQuaternion::operator op ( const rightType a )\
    {\
        mChunkBase[0] = op_func( mChunkBase[0], a );\
        mChunkBase[1] = op_func( mChunkBase[1], a );\
        mChunkBase[2] = op_func( mChunkBase[2], a );\
        mChunkBase[3] = op_func( mChunkBase[3], a );\
    }

    // + Addition
    DEFINE_OPERATION( ArrayQuaternion, ArrayQuaternion, +, _mm_add_ps );

    // - Subtraction
    DEFINE_OPERATION( ArrayQuaternion, ArrayQuaternion, -, _mm_sub_ps );

    // * Multiplication (scalar only)
    DEFINE_L_OPERATION( ArrayReal, ArrayQuaternion, *, _mm_mul_ps );
    DEFINE_R_OPERATION( ArrayQuaternion, ArrayReal, *, _mm_mul_ps );

    // Update operations
    // +=
    DEFINE_UPDATE_OPERATION(            ArrayQuaternion,        +=, _mm_add_ps );

    // -=
    DEFINE_UPDATE_OPERATION(            ArrayQuaternion,        -=, _mm_sub_ps );

    // *=
    DEFINE_UPDATE_R_OPERATION(          ArrayReal,          *=, _mm_mul_ps );

    // Notes: This operator doesn't get inlined. The generated instruction count is actually high so
    // the compiler seems to be clever in not inlining. There is no gain in doing a "mul()" equivalent
    // like we did with mul( const ArrayQuaternion&, ArrayVector3& ) because we would still need
    // a temporary variable to hold all the operations (we can't overwrite to any heap value
    // since all values are used until the last operation)
    inline ArrayQuaternion operator * ( const ArrayQuaternion &lhs, const ArrayQuaternion &rhs )
    {
        return ArrayQuaternion(
            /* w = (w * rkQ.w - x * rkQ.x) - (y * rkQ.y + z * rkQ.z) */
            _mm_sub_ps( _mm_sub_ps(
                    _mm_mul_ps( lhs.mChunkBase[0], rhs.mChunkBase[0] ),
                    _mm_mul_ps( lhs.mChunkBase[1], rhs.mChunkBase[1] ) ),
            _mm_add_ps(
                    _mm_mul_ps( lhs.mChunkBase[2], rhs.mChunkBase[2] ),
                    _mm_mul_ps( lhs.mChunkBase[3], rhs.mChunkBase[3] ) ) ),
            /* x = (w * rkQ.x + x * rkQ.w) + (y * rkQ.z - z * rkQ.y) */
            _mm_add_ps( _mm_add_ps(
                    _mm_mul_ps( lhs.mChunkBase[0], rhs.mChunkBase[1] ),
                    _mm_mul_ps( lhs.mChunkBase[1], rhs.mChunkBase[0] ) ),
            _mm_sub_ps(
                    _mm_mul_ps( lhs.mChunkBase[2], rhs.mChunkBase[3] ),
                    _mm_mul_ps( lhs.mChunkBase[3], rhs.mChunkBase[2] ) ) ),
            /* y = (w * rkQ.y + y * rkQ.w) + (z * rkQ.x - x * rkQ.z) */
            _mm_add_ps( _mm_add_ps(
                    _mm_mul_ps( lhs.mChunkBase[0], rhs.mChunkBase[2] ),
                    _mm_mul_ps( lhs.mChunkBase[2], rhs.mChunkBase[0] ) ),
            _mm_sub_ps(
                    _mm_mul_ps( lhs.mChunkBase[3], rhs.mChunkBase[1] ),
                    _mm_mul_ps( lhs.mChunkBase[1], rhs.mChunkBase[3] ) ) ),
            /* z = (w * rkQ.z + z * rkQ.w) + (x * rkQ.y - y * rkQ.x) */
            _mm_add_ps( _mm_add_ps(
                    _mm_mul_ps( lhs.mChunkBase[0], rhs.mChunkBase[3] ),
                    _mm_mul_ps( lhs.mChunkBase[3], rhs.mChunkBase[0] ) ),
            _mm_sub_ps(
                    _mm_mul_ps( lhs.mChunkBase[1], rhs.mChunkBase[2] ),
                    _mm_mul_ps( lhs.mChunkBase[2], rhs.mChunkBase[1] ) ) ) );
    }
    //-----------------------------------------------------------------------------------
    inline ArrayQuaternion ArrayQuaternion::Slerp( ArrayReal fT, const ArrayQuaternion &rkP,
                                                        const ArrayQuaternion &rkQ /*, bool shortestPath*/ )
    {
        ArrayReal fCos = rkP.Dot( rkQ );
        /* Clamp fCos to [-1; 1] range */
        fCos = _mm_min_ps( MathlibSSE2::ONE, _mm_max_ps( MathlibSSE2::NEG_ONE, fCos ) );
        
        /* Invert the rotation for shortest path? */
        /* m = fCos < 0.0f ? -1.0f : 1.0f; */
        ArrayReal m = MathlibSSE2::Cmov4( MathlibSSE2::NEG_ONE, MathlibSSE2::ONE,
                                            _mm_cmplt_ps( fCos, _mm_setzero_ps() ) /*&& shortestPath*/ );
        ArrayQuaternion rkT(
                        _mm_mul_ps( rkQ.mChunkBase[0], m ),
                        _mm_mul_ps( rkQ.mChunkBase[1], m ),
                        _mm_mul_ps( rkQ.mChunkBase[2], m ),
                        _mm_mul_ps( rkQ.mChunkBase[3], m ) );
        
        ArrayReal fSin = _mm_sqrt_ps( _mm_sub_ps( MathlibSSE2::ONE, _mm_mul_ps( fCos, fCos ) ) );
        
        /* ATan2 in original Quaternion is slightly absurd, because fSin was derived from
           fCos (hence never negative) which makes ACos a better replacement. ACos is much
           faster than ATan2, as long as the input is whithin range [-1; 1], otherwise the generated
           NaNs make it slower (whether clamping the input outweights the benefit is
           arguable). We use ACos4 to avoid implementing ATan2_4.
        */
        ArrayReal fAngle = MathlibSSE2::ACos4( fCos );

        // mask = Abs( fCos ) < 1-epsilon
        ArrayReal mask    = _mm_cmplt_ps( MathlibSSE2::Abs4( fCos ), MathlibSSE2::OneMinusEpsilon );
        ArrayReal fInvSin = MathlibSSE2::InvNonZero4( fSin );
        ArrayReal oneSubT = _mm_sub_ps( MathlibSSE2::ONE, fT );
        // fCoeff1 = Sin( fT * fAngle ) * fInvSin
        ArrayReal fCoeff0 = _mm_mul_ps( MathlibSSE2::Sin4( _mm_mul_ps( oneSubT, fAngle ) ), fInvSin );
        ArrayReal fCoeff1 = _mm_mul_ps( MathlibSSE2::Sin4( _mm_mul_ps( fT, fAngle ) ), fInvSin );
        // fCoeff1 = mask ? fCoeff1 : fT; (switch to lerp when rkP & rkQ are too close->fSin=0, or 180°)
        fCoeff0 = MathlibSSE2::CmovRobust( fCoeff0, oneSubT, mask );
        fCoeff1 = MathlibSSE2::CmovRobust( fCoeff1, fT, mask );

        // retVal = fCoeff0 * rkP + fCoeff1 * rkT;
        rkT.mChunkBase[0] = _mm_add_ps( _mm_mul_ps( rkP.mChunkBase[0], fCoeff0 ),
                                         _mm_mul_ps( rkT.mChunkBase[0], fCoeff1 ) ),
        rkT.mChunkBase[1] = _mm_add_ps( _mm_mul_ps( rkP.mChunkBase[1], fCoeff0 ),
                                         _mm_mul_ps( rkT.mChunkBase[1], fCoeff1 ) ),
        rkT.mChunkBase[2] = _mm_add_ps( _mm_mul_ps( rkP.mChunkBase[2], fCoeff0 ),
                                         _mm_mul_ps( rkT.mChunkBase[2], fCoeff1 ) ),
        rkT.mChunkBase[3] = _mm_add_ps( _mm_mul_ps( rkP.mChunkBase[3], fCoeff0 ),
                                         _mm_mul_ps( rkT.mChunkBase[3], fCoeff1 ) );

        rkT.normalise();

        return rkT;
    }
    //-----------------------------------------------------------------------------------
    inline ArrayQuaternion ArrayQuaternion::nlerpShortest( ArrayReal fT, const ArrayQuaternion &rkP,
                                                            const ArrayQuaternion &rkQ )
    {
        //Flip the sign of rkQ when p.dot( q ) < 0 to get the shortest path
        ArrayReal signMask = _mm_set1_ps( -0.0f );
        ArrayReal sign = _mm_and_ps( signMask, rkP.Dot( rkQ ) );
        ArrayQuaternion tmpQ = ArrayQuaternion( _mm_xor_ps( rkQ.mChunkBase[0], sign ),
                                                _mm_xor_ps( rkQ.mChunkBase[1], sign ),
                                                _mm_xor_ps( rkQ.mChunkBase[2], sign ),
                                                _mm_xor_ps( rkQ.mChunkBase[3], sign ) );

        ArrayQuaternion retVal(
                _mm_madd_ps( fT, _mm_sub_ps( tmpQ.mChunkBase[0], rkP.mChunkBase[0] ), rkP.mChunkBase[0] ),
                _mm_madd_ps( fT, _mm_sub_ps( tmpQ.mChunkBase[1], rkP.mChunkBase[1] ), rkP.mChunkBase[1] ),
                _mm_madd_ps( fT, _mm_sub_ps( tmpQ.mChunkBase[2], rkP.mChunkBase[2] ), rkP.mChunkBase[2] ),
                _mm_madd_ps( fT, _mm_sub_ps( tmpQ.mChunkBase[3], rkP.mChunkBase[3] ), rkP.mChunkBase[3] ) );
        retVal.normalise();

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    inline ArrayQuaternion ArrayQuaternion::nlerp( ArrayReal fT, const ArrayQuaternion &rkP,
                                                        const ArrayQuaternion &rkQ )
    {
        ArrayQuaternion retVal(
                _mm_madd_ps( fT, _mm_sub_ps( rkQ.mChunkBase[0], rkP.mChunkBase[0] ), rkP.mChunkBase[0] ),
                _mm_madd_ps( fT, _mm_sub_ps( rkQ.mChunkBase[1], rkP.mChunkBase[1] ), rkP.mChunkBase[1] ),
                _mm_madd_ps( fT, _mm_sub_ps( rkQ.mChunkBase[2], rkP.mChunkBase[2] ), rkP.mChunkBase[2] ),
                _mm_madd_ps( fT, _mm_sub_ps( rkQ.mChunkBase[3], rkP.mChunkBase[3] ), rkP.mChunkBase[3] ) );
        retVal.normalise();

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    inline ArrayQuaternion ArrayQuaternion::Cmov4( const ArrayQuaternion &arg1,
                                                    const ArrayQuaternion &arg2, ArrayMaskR mask )
    {
        return ArrayQuaternion(
                MathlibSSE2::Cmov4( arg1.mChunkBase[0], arg2.mChunkBase[0], mask ),
                MathlibSSE2::Cmov4( arg1.mChunkBase[1], arg2.mChunkBase[1], mask ),
                MathlibSSE2::Cmov4( arg1.mChunkBase[2], arg2.mChunkBase[2], mask ),
                MathlibSSE2::Cmov4( arg1.mChunkBase[3], arg2.mChunkBase[3], mask ) );
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayQuaternion::mul( const ArrayQuaternion &inQ, ArrayVector3 &inOutVec )
    {
        // nVidia SDK implementation
        ArrayVector3 qVec( inQ.mChunkBase[1], inQ.mChunkBase[2], inQ.mChunkBase[3] );

        ArrayVector3 uv = qVec.crossProduct( inOutVec );
        ArrayVector3 uuv    = qVec.crossProduct( uv );

        // uv = uv * (2.0f * w)
        ArrayReal w2 = _mm_add_ps( inQ.mChunkBase[0], inQ.mChunkBase[0] );
        uv.mChunkBase[0] = _mm_mul_ps( uv.mChunkBase[0], w2 );
        uv.mChunkBase[1] = _mm_mul_ps( uv.mChunkBase[1], w2 );
        uv.mChunkBase[2] = _mm_mul_ps( uv.mChunkBase[2], w2 );

        // uuv = uuv * 2.0f
        uuv.mChunkBase[0] = _mm_add_ps( uuv.mChunkBase[0], uuv.mChunkBase[0] );
        uuv.mChunkBase[1] = _mm_add_ps( uuv.mChunkBase[1], uuv.mChunkBase[1] );
        uuv.mChunkBase[2] = _mm_add_ps( uuv.mChunkBase[2], uuv.mChunkBase[2] );

        //inOutVec = v + uv + uuv
        inOutVec.mChunkBase[0] = _mm_add_ps( inOutVec.mChunkBase[0],
                                    _mm_add_ps( uv.mChunkBase[0], uuv.mChunkBase[0] ) );
        inOutVec.mChunkBase[1] = _mm_add_ps( inOutVec.mChunkBase[1],
                                    _mm_add_ps( uv.mChunkBase[1], uuv.mChunkBase[1] ) );
        inOutVec.mChunkBase[2] = _mm_add_ps( inOutVec.mChunkBase[2],
                                    _mm_add_ps( uv.mChunkBase[2], uuv.mChunkBase[2] ) );
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayQuaternion::FromOrthoDet1RotationMatrix( const ArrayReal * RESTRICT_ALIAS matrix )
    {
        ArrayReal m00 = matrix[0], m01 = matrix[1], m02 = matrix[2],
                  m10 = matrix[3], m11 = matrix[4], m12 = matrix[5],
                  m20 = matrix[6], m21 = matrix[7], m22 = matrix[8];

        //To deal with matrices that don't have determinant = 1
        //absQ2 = det( matrix )^(1/3)
        // quaternion.w = sqrt( max( 0, absQ2 + m00 + m11 + m22 ) ) / 2; ... etc

        //w = sqrt( max( 0, 1 + m00 + m11 + m22 ) ) / 2;
        //x = sqrt( max( 0, 1 + m00 - m11 - m22 ) ) / 2;
        //y = sqrt( max( 0, 1 - m00 + m11 - m22 ) ) / 2;
        //z = sqrt( max( 0, 1 - m00 - m11 + m22 ) ) / 2;
        //x = _copysign( x, m21 - m12 );
        //y = _copysign( y, m02 - m20 );
        //z = _copysign( z, m10 - m01 );
        ArrayReal tmp;

        //w = sqrt( max( 0, (1 + m00) + (m11 + m22) ) ) * 0.5f;
        tmp = _mm_max_ps( _mm_setzero_ps(),
                          _mm_add_ps( _mm_add_ps( MathlibSSE2::ONE, m00 ), _mm_add_ps( m11, m22 ) ) );
        mChunkBase[0] = _mm_mul_ps( _mm_sqrt_ps( tmp ), MathlibSSE2::HALF );

        //x = sqrt( max( 0, (1 + m00) - (m11 + m22) ) ) * 0.5f;
        tmp = _mm_max_ps( _mm_setzero_ps(),
                          _mm_sub_ps( _mm_add_ps( MathlibSSE2::ONE, m00 ), _mm_add_ps( m11, m22 ) ) );
        mChunkBase[1] = _mm_mul_ps( _mm_sqrt_ps( tmp ), MathlibSSE2::HALF );

        //y = sqrt( max( 0, (1 - m00) + (m11 - m22) ) ) * 0.5f;
        tmp = _mm_max_ps( _mm_setzero_ps(),
                          _mm_add_ps( _mm_sub_ps( MathlibSSE2::ONE, m00 ), _mm_sub_ps( m11, m22 ) ) );
        mChunkBase[2] = _mm_mul_ps( _mm_sqrt_ps( tmp ), MathlibSSE2::HALF );

        //z = sqrt( max( 0, (1 - m00) - (m11 - m22) ) ) * 0.5f;
        tmp = _mm_max_ps( _mm_setzero_ps(),
                          _mm_sub_ps( _mm_sub_ps( MathlibSSE2::ONE, m00 ), _mm_sub_ps( m11, m22 ) ) );
        mChunkBase[3] = _mm_mul_ps( _mm_sqrt_ps( tmp ), MathlibSSE2::HALF );

        //x = _copysign( x, m21 - m12 ); --> (x & 0x7FFFFFFF) | ((m21 - m12) & 0x80000000)
        //y = _copysign( y, m02 - m20 ); --> (y & 0x7FFFFFFF) | ((m02 - m20) & 0x80000000)
        //z = _copysign( z, m10 - m01 ); --> (z & 0x7FFFFFFF) | ((m10 - m01) & 0x80000000)
        tmp = _mm_and_ps( _mm_sub_ps( m21, m12 ), MathlibSSE2::SIGN_MASK );
        mChunkBase[1] = _mm_or_ps( _mm_andnot_ps( MathlibSSE2::SIGN_MASK, mChunkBase[1] ), tmp );
        tmp = _mm_and_ps( _mm_sub_ps( m02, m20 ), MathlibSSE2::SIGN_MASK );
        mChunkBase[2] = _mm_or_ps( _mm_andnot_ps( MathlibSSE2::SIGN_MASK, mChunkBase[2] ), tmp );
        tmp = _mm_and_ps( _mm_sub_ps( m10, m01 ), MathlibSSE2::SIGN_MASK );
        mChunkBase[3] = _mm_or_ps( _mm_andnot_ps( MathlibSSE2::SIGN_MASK, mChunkBase[3] ), tmp );
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayQuaternion::FromAngleAxis( const ArrayRadian& rfAngle, const ArrayVector3& rkAxis )
    {
        // assert:  axis[] is unit length
        //
        // The quaternion representing the rotation is
        //   q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k)

        ArrayReal fHalfAngle( _mm_mul_ps( rfAngle.valueRadians(), MathlibSSE2::HALF ) );

        ArrayReal fSin;
        MathlibSSE2::SinCos4( fHalfAngle, fSin, mChunkBase[0] );

        ArrayReal * RESTRICT_ALIAS chunkBase = mChunkBase;
        const ArrayReal * RESTRICT_ALIAS rkAxisChunkBase = rkAxis.mChunkBase;

        chunkBase[1] = _mm_mul_ps( fSin, rkAxisChunkBase[0] ); //x = fSin*rkAxis.x;
        chunkBase[2] = _mm_mul_ps( fSin, rkAxisChunkBase[1] ); //y = fSin*rkAxis.y;
        chunkBase[3] = _mm_mul_ps( fSin, rkAxisChunkBase[2] ); //z = fSin*rkAxis.z;
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayQuaternion::ToAngleAxis( ArrayRadian &rfAngle, ArrayVector3 &rkAxis ) const
    {
        // The quaternion representing the rotation is
        //   q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k)
        ArrayReal sqLength = _mm_add_ps( _mm_add_ps(
                                _mm_mul_ps( mChunkBase[1], mChunkBase[1] ), //(x * x +
                                _mm_mul_ps( mChunkBase[2], mChunkBase[2] ) ),   //y * y) +
                                _mm_mul_ps( mChunkBase[3], mChunkBase[3] ) );   //z * z )

        ArrayReal mask      = _mm_cmpgt_ps( sqLength, _mm_setzero_ps() ); //mask = sqLength > 0

        //sqLength = sqLength > 0 ? sqLength : 1; so that invSqrt doesn't give NaNs or Infs
        //when 0 (to avoid using CmovRobust just to select the non-nan results)
        sqLength = MathlibSSE2::Cmov4( sqLength, MathlibSSE2::ONE,
                                        _mm_cmpgt_ps( sqLength, MathlibSSE2::FLOAT_MIN ) );
        ArrayReal fInvLength = MathlibSSE2::InvSqrtNonZero4( sqLength );

        const ArrayReal acosW = MathlibSSE2::ACos4( mChunkBase[0] );
        rfAngle = MathlibSSE2::Cmov4( //sqLength > 0 ? (2 * ACos(w)) : 0
                    _mm_add_ps( acosW, acosW ),
                    _mm_setzero_ps(), mask );

        rkAxis.mChunkBase[0] = MathlibSSE2::Cmov4(  //sqLength > 0 ? (x * fInvLength) : 1
                                    _mm_mul_ps( mChunkBase[1], fInvLength ), MathlibSSE2::ONE, mask );
        rkAxis.mChunkBase[1] = MathlibSSE2::Cmov4(  //sqLength > 0 ? (y * fInvLength) : 0
                                    _mm_mul_ps( mChunkBase[2], fInvLength ), _mm_setzero_ps(), mask );
        rkAxis.mChunkBase[2] = MathlibSSE2::Cmov4(  //sqLength > 0 ? (y * fInvLength) : 0
                                    _mm_mul_ps( mChunkBase[3], fInvLength ), _mm_setzero_ps(), mask );
    }
    //-----------------------------------------------------------------------------------
    inline ArrayVector3 ArrayQuaternion::xAxis( void ) const
    {
        ArrayReal fTy  = _mm_add_ps( mChunkBase[2], mChunkBase[2] );        // 2 * y
        ArrayReal fTz  = _mm_add_ps( mChunkBase[3], mChunkBase[3] );        // 2 * z
        ArrayReal fTwy = _mm_mul_ps( fTy, mChunkBase[0] );                  // fTy*w;
        ArrayReal fTwz = _mm_mul_ps( fTz, mChunkBase[0] );                  // fTz*w;
        ArrayReal fTxy = _mm_mul_ps( fTy, mChunkBase[1] );                  // fTy*x;
        ArrayReal fTxz = _mm_mul_ps( fTz, mChunkBase[1] );                  // fTz*x;
        ArrayReal fTyy = _mm_mul_ps( fTy, mChunkBase[2] );                  // fTy*y;
        ArrayReal fTzz = _mm_mul_ps( fTz, mChunkBase[3] );                  // fTz*z;

        return ArrayVector3(
                _mm_sub_ps( MathlibSSE2::ONE, _mm_add_ps( fTyy, fTzz ) ),
                _mm_add_ps( fTxy, fTwz ),
                _mm_sub_ps( fTxz, fTwy ) );
    }
    //-----------------------------------------------------------------------------------
    inline ArrayVector3 ArrayQuaternion::yAxis( void ) const
    {
        ArrayReal fTx  = _mm_add_ps( mChunkBase[1], mChunkBase[1] );        // 2 * x
        ArrayReal fTy  = _mm_add_ps( mChunkBase[2], mChunkBase[2] );        // 2 * y
        ArrayReal fTz  = _mm_add_ps( mChunkBase[3], mChunkBase[3] );        // 2 * z
        ArrayReal fTwx = _mm_mul_ps( fTx, mChunkBase[0] );                  // fTx*w;
        ArrayReal fTwz = _mm_mul_ps( fTz, mChunkBase[0] );                  // fTz*w;
        ArrayReal fTxx = _mm_mul_ps( fTx, mChunkBase[1] );                  // fTx*x;
        ArrayReal fTxy = _mm_mul_ps( fTy, mChunkBase[1] );                  // fTy*x;
        ArrayReal fTyz = _mm_mul_ps( fTz, mChunkBase[2] );                  // fTz*y;
        ArrayReal fTzz = _mm_mul_ps( fTz, mChunkBase[3] );                  // fTz*z;

        return ArrayVector3(
                _mm_sub_ps( fTxy, fTwz ),
                _mm_sub_ps( MathlibSSE2::ONE, _mm_add_ps( fTxx, fTzz ) ),
                _mm_add_ps( fTyz, fTwx ) );
    }
    //-----------------------------------------------------------------------------------
    inline ArrayVector3 ArrayQuaternion::zAxis( void ) const
    {
        ArrayReal fTx  = _mm_add_ps( mChunkBase[1], mChunkBase[1] );        // 2 * x
        ArrayReal fTy  = _mm_add_ps( mChunkBase[2], mChunkBase[2] );        // 2 * y
        ArrayReal fTz  = _mm_add_ps( mChunkBase[3], mChunkBase[3] );        // 2 * z
        ArrayReal fTwx = _mm_mul_ps( fTx, mChunkBase[0] );                  // fTx*w;
        ArrayReal fTwy = _mm_mul_ps( fTy, mChunkBase[0] );                  // fTy*w;
        ArrayReal fTxx = _mm_mul_ps( fTx, mChunkBase[1] );                  // fTx*x;
        ArrayReal fTxz = _mm_mul_ps( fTz, mChunkBase[1] );                  // fTz*x;
        ArrayReal fTyy = _mm_mul_ps( fTy, mChunkBase[2] );                  // fTy*y;
        ArrayReal fTyz = _mm_mul_ps( fTz, mChunkBase[2] );                  // fTz*y;

        return ArrayVector3(
                _mm_add_ps( fTxz, fTwy ),
                _mm_sub_ps( fTyz, fTwx ),
                _mm_sub_ps( MathlibSSE2::ONE, _mm_add_ps( fTxx, fTyy ) ) );
    }
    //-----------------------------------------------------------------------------------
    inline ArrayReal ArrayQuaternion::Dot( const ArrayQuaternion& rkQ ) const
    {
        return
        _mm_add_ps( _mm_add_ps( _mm_add_ps(
            _mm_mul_ps( mChunkBase[0], rkQ.mChunkBase[0] ) ,    //((w * vec.w   +
            _mm_mul_ps( mChunkBase[1], rkQ.mChunkBase[1] ) ),   //  x * vec.x ) +
            _mm_mul_ps( mChunkBase[2], rkQ.mChunkBase[2] ) ), //  y * vec.y ) +
            _mm_mul_ps( mChunkBase[3], rkQ.mChunkBase[3] ) );   //  z * vec.z
    }
    //-----------------------------------------------------------------------------------
    inline ArrayReal ArrayQuaternion::Norm( void ) const
    {
        return
        _mm_add_ps( _mm_add_ps( _mm_add_ps(
            _mm_mul_ps( mChunkBase[0], mChunkBase[0] ) ,    //((w * w   +
            _mm_mul_ps( mChunkBase[1], mChunkBase[1] ) ),   //  x * x ) +
            _mm_mul_ps( mChunkBase[2], mChunkBase[2] ) ), //  y * y ) +
            _mm_mul_ps( mChunkBase[3], mChunkBase[3] ) );   //  z * z
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayQuaternion::normalise( void )
    {
        ArrayReal sqLength = _mm_add_ps( _mm_add_ps( _mm_add_ps(
            _mm_mul_ps( mChunkBase[0], mChunkBase[0] ) ,    //((w * w   +
            _mm_mul_ps( mChunkBase[1], mChunkBase[1] ) ),   //  x * x ) +
            _mm_mul_ps( mChunkBase[2], mChunkBase[2] ) ), //  y * y ) +
            _mm_mul_ps( mChunkBase[3], mChunkBase[3] ) );   //  z * z

        //Convert sqLength's 0s into 1, so that zero vectors remain as zero
        //Denormals are treated as 0 during the check.
        //Note: We could create a mask now and nuke nans after InvSqrt, however
        //generating the nans could impact performance in some architectures
        sqLength = MathlibSSE2::Cmov4( sqLength, MathlibSSE2::ONE,
                                        _mm_cmpgt_ps( sqLength, MathlibSSE2::FLOAT_MIN ) );
        ArrayReal invLength = MathlibSSE2::InvSqrtNonZero4( sqLength );
        mChunkBase[0] = _mm_mul_ps( mChunkBase[0], invLength ); //w * invLength
        mChunkBase[1] = _mm_mul_ps( mChunkBase[1], invLength ); //x * invLength
        mChunkBase[2] = _mm_mul_ps( mChunkBase[2], invLength ); //y * invLength
        mChunkBase[3] = _mm_mul_ps( mChunkBase[3], invLength ); //z * invLength
    }
    //-----------------------------------------------------------------------------------
    inline ArrayQuaternion ArrayQuaternion::Inverse( void ) const
    {
        ArrayReal fNorm = _mm_add_ps( _mm_add_ps( _mm_add_ps(
            _mm_mul_ps( mChunkBase[0], mChunkBase[0] ) ,    //((w * w   +
            _mm_mul_ps( mChunkBase[1], mChunkBase[1] ) ),   //  x * x ) +
            _mm_mul_ps( mChunkBase[2], mChunkBase[2] ) ), //  y * y ) +
            _mm_mul_ps( mChunkBase[3], mChunkBase[3] ) );   //  z * z;

        //Will return a zero Quaternion if original is zero length (Quaternion's behavior)
        fNorm = MathlibSSE2::Cmov4( fNorm, MathlibSSE2::ONE,
                                    _mm_cmpgt_ps( fNorm, MathlibSSE2::fEpsilon ) );
        ArrayReal invNorm    = MathlibSSE2::Inv4( fNorm );
        ArrayReal negInvNorm = _mm_mul_ps( invNorm, MathlibSSE2::NEG_ONE );

        return ArrayQuaternion(
            _mm_mul_ps( mChunkBase[0], invNorm ),       //w * invNorm
            _mm_mul_ps( mChunkBase[1], negInvNorm ),    //x * -invNorm
            _mm_mul_ps( mChunkBase[2], negInvNorm ),    //y * -invNorm
            _mm_mul_ps( mChunkBase[3], negInvNorm ) );  //z * -invNorm
    }
    //-----------------------------------------------------------------------------------
    inline ArrayQuaternion ArrayQuaternion::UnitInverse( void ) const
    {
        return ArrayQuaternion(
            mChunkBase[0],                                          //w
            _mm_mul_ps( mChunkBase[1], MathlibSSE2::NEG_ONE ),      //-x
            _mm_mul_ps( mChunkBase[2], MathlibSSE2::NEG_ONE ),      //-y
            _mm_mul_ps( mChunkBase[3], MathlibSSE2::NEG_ONE ) );    //-z
    }
    //-----------------------------------------------------------------------------------
    inline ArrayQuaternion ArrayQuaternion::Exp( void ) const
    {
        // If q = A*(x*i+y*j+z*k) where (x,y,z) is unit length, then
        // exp(q) = cos(A)+sin(A)*(x*i+y*j+z*k).  If sin(A) is near zero,
        // use exp(q) = cos(A)+A*(x*i+y*j+z*k) since A/sin(A) has limit 1.

        ArrayReal fAngle = _mm_sqrt_ps( _mm_add_ps( _mm_add_ps(                     //sqrt(
                                _mm_mul_ps( mChunkBase[1], mChunkBase[1] ),     //(x * x +
                                _mm_mul_ps( mChunkBase[2], mChunkBase[2] ) ),       //y * y) +
                                _mm_mul_ps( mChunkBase[3], mChunkBase[3] ) ) ); //z * z )

        ArrayReal w, fSin;
        MathlibSSE2::SinCos4( fAngle, fSin, w );

        //coeff = Abs(fSin) >= msEpsilon ? (fSin / fAngle) : 1.0f;
        ArrayReal coeff = MathlibSSE2::CmovRobust( _mm_div_ps( fSin, fAngle ), MathlibSSE2::ONE,
                                _mm_cmpge_ps( MathlibSSE2::Abs4( fSin ), MathlibSSE2::fEpsilon ) );
        return ArrayQuaternion(
            w,                                          //cos( fAngle )
            _mm_mul_ps( mChunkBase[1], coeff ),     //x * coeff
            _mm_mul_ps( mChunkBase[2], coeff ),     //y * coeff
            _mm_mul_ps( mChunkBase[3], coeff ) );       //z * coeff
    }
    //-----------------------------------------------------------------------------------
    inline ArrayQuaternion ArrayQuaternion::Log( void ) const
    {
        // If q = cos(A)+sin(A)*(x*i+y*j+z*k) where (x,y,z) is unit length, then
        // log(q) = A*(x*i+y*j+z*k).  If sin(A) is near zero, use log(q) =
        // sin(A)*(x*i+y*j+z*k) since sin(A)/A has limit 1.

        ArrayReal fAngle    = MathlibSSE2::ACos4( mChunkBase[0] );
        ArrayReal fSin      = MathlibSSE2::Sin4( fAngle );

        //mask = Math::Abs(w) < 1.0 && Math::Abs(fSin) >= msEpsilon
        ArrayReal mask = _mm_and_ps(
                            _mm_cmplt_ps( MathlibSSE2::Abs4( mChunkBase[0] ), MathlibSSE2::ONE ),
                            _mm_cmpge_ps( MathlibSSE2::Abs4( fSin ), MathlibSSE2::fEpsilon ) );

        //coeff = mask ? (fAngle / fSin) : 1.0
        //Unlike Exp(), we can use InvNonZero4 (which is faster) instead of div because we know for
        //sure CMov will copy the 1 instead of the NaN when fSin is close to zero, guarantee we might
        //not have in Exp()
        ArrayReal coeff = MathlibSSE2::CmovRobust( _mm_mul_ps( fAngle, MathlibSSE2::InvNonZero4( fSin ) ),
                                                    MathlibSSE2::ONE, mask );

        return ArrayQuaternion(
            _mm_setzero_ps(),                           //w = 0
            _mm_mul_ps( mChunkBase[1], coeff ),     //x * coeff
            _mm_mul_ps( mChunkBase[2], coeff ),     //y * coeff
            _mm_mul_ps( mChunkBase[3], coeff ) );       //z * coeff
    }
    //-----------------------------------------------------------------------------------
    inline ArrayVector3 ArrayQuaternion::operator * ( const ArrayVector3 &v ) const
    {
        // nVidia SDK implementation
        ArrayVector3 qVec( mChunkBase[1], mChunkBase[2], mChunkBase[3] );

        ArrayVector3 uv = qVec.crossProduct( v );
        ArrayVector3 uuv    = qVec.crossProduct( uv );

        // uv = uv * (2.0f * w)
        ArrayReal w2 = _mm_add_ps( mChunkBase[0], mChunkBase[0] );
        uv.mChunkBase[0] = _mm_mul_ps( uv.mChunkBase[0], w2 );
        uv.mChunkBase[1] = _mm_mul_ps( uv.mChunkBase[1], w2 );
        uv.mChunkBase[2] = _mm_mul_ps( uv.mChunkBase[2], w2 );

        // uuv = uuv * 2.0f
        uuv.mChunkBase[0] = _mm_add_ps( uuv.mChunkBase[0], uuv.mChunkBase[0] );
        uuv.mChunkBase[1] = _mm_add_ps( uuv.mChunkBase[1], uuv.mChunkBase[1] );
        uuv.mChunkBase[2] = _mm_add_ps( uuv.mChunkBase[2], uuv.mChunkBase[2] );

        //uv = v + uv + uuv
        uv.mChunkBase[0] = _mm_add_ps( v.mChunkBase[0],
                                _mm_add_ps( uv.mChunkBase[0], uuv.mChunkBase[0] ) );
        uv.mChunkBase[1] = _mm_add_ps( v.mChunkBase[1],
                                _mm_add_ps( uv.mChunkBase[1], uuv.mChunkBase[1] ) );
        uv.mChunkBase[2] = _mm_add_ps( v.mChunkBase[2],
                                _mm_add_ps( uv.mChunkBase[2], uuv.mChunkBase[2] ) );

        return uv;
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayQuaternion::Cmov4( ArrayMaskR mask, const ArrayQuaternion &replacement )
    {
        ArrayReal * RESTRICT_ALIAS aChunkBase = mChunkBase;
        const ArrayReal * RESTRICT_ALIAS bChunkBase = replacement.mChunkBase;
        aChunkBase[0] = MathlibSSE2::Cmov4( aChunkBase[0], bChunkBase[0], mask );
        aChunkBase[1] = MathlibSSE2::Cmov4( aChunkBase[1], bChunkBase[1], mask );
        aChunkBase[2] = MathlibSSE2::Cmov4( aChunkBase[2], bChunkBase[2], mask );
        aChunkBase[3] = MathlibSSE2::Cmov4( aChunkBase[3], bChunkBase[3], mask );
    }
}
