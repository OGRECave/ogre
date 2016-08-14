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
    DEFINE_OPERATION( ArrayQuaternion, ArrayQuaternion, +, vaddq_f32 );

    // - Subtraction
    DEFINE_OPERATION( ArrayQuaternion, ArrayQuaternion, -, vsubq_f32 );

    // * Multiplication (scalar only)
    DEFINE_L_OPERATION( ArrayReal, ArrayQuaternion, *, vmulq_f32 );
    DEFINE_R_OPERATION( ArrayQuaternion, ArrayReal, *, vmulq_f32 );

    // Update operations
    // +=
    DEFINE_UPDATE_OPERATION(            ArrayQuaternion,        +=, vaddq_f32 );

    // -=
    DEFINE_UPDATE_OPERATION(            ArrayQuaternion,        -=, vsubq_f32 );

    // *=
    DEFINE_UPDATE_R_OPERATION(          ArrayReal,          *=, vmulq_f32 );

    // Notes: This operator doesn't get inlined. The generated instruction count is actually high so
    // the compiler seems to be clever in not inlining. There is no gain in doing a "mul()" equivalent
    // like we did with mul( const ArrayQuaternion&, ArrayVector3& ) because we would still need
    // a temporary variable to hold all the operations (we can't overwrite to any heap value
    // since all values are used until the last operation)
    inline ArrayQuaternion operator * ( const ArrayQuaternion &lhs, const ArrayQuaternion &rhs )
    {
        return ArrayQuaternion(
            /* w = (w * rkQ.w - x * rkQ.x) - (y * rkQ.y + z * rkQ.z) */
            vsubq_f32( vsubq_f32(
                    vmulq_f32( lhs.mChunkBase[0], rhs.mChunkBase[0] ),
                    vmulq_f32( lhs.mChunkBase[1], rhs.mChunkBase[1] ) ),
            vaddq_f32(
                    vmulq_f32( lhs.mChunkBase[2], rhs.mChunkBase[2] ),
                    vmulq_f32( lhs.mChunkBase[3], rhs.mChunkBase[3] ) ) ),
            /* x = (w * rkQ.x + x * rkQ.w) + (y * rkQ.z - z * rkQ.y) */
            vaddq_f32( vaddq_f32(
                    vmulq_f32( lhs.mChunkBase[0], rhs.mChunkBase[1] ),
                    vmulq_f32( lhs.mChunkBase[1], rhs.mChunkBase[0] ) ),
            vsubq_f32(
                    vmulq_f32( lhs.mChunkBase[2], rhs.mChunkBase[3] ),
                    vmulq_f32( lhs.mChunkBase[3], rhs.mChunkBase[2] ) ) ),
            /* y = (w * rkQ.y + y * rkQ.w) + (z * rkQ.x - x * rkQ.z) */
            vaddq_f32( vaddq_f32(
                    vmulq_f32( lhs.mChunkBase[0], rhs.mChunkBase[2] ),
                    vmulq_f32( lhs.mChunkBase[2], rhs.mChunkBase[0] ) ),
            vsubq_f32(
                    vmulq_f32( lhs.mChunkBase[3], rhs.mChunkBase[1] ),
                    vmulq_f32( lhs.mChunkBase[1], rhs.mChunkBase[3] ) ) ),
            /* z = (w * rkQ.z + z * rkQ.w) + (x * rkQ.y - y * rkQ.x) */
            vaddq_f32( vaddq_f32(
                    vmulq_f32( lhs.mChunkBase[0], rhs.mChunkBase[3] ),
                    vmulq_f32( lhs.mChunkBase[3], rhs.mChunkBase[0] ) ),
            vsubq_f32(
                    vmulq_f32( lhs.mChunkBase[1], rhs.mChunkBase[2] ),
                    vmulq_f32( lhs.mChunkBase[2], rhs.mChunkBase[1] ) ) ) );
    }
    //-----------------------------------------------------------------------------------
    inline ArrayQuaternion ArrayQuaternion::Slerp( ArrayReal fT, const ArrayQuaternion &rkP,
                                                        const ArrayQuaternion &rkQ /*, bool shortestPath*/ )
    {
        ArrayReal fCos = rkP.Dot( rkQ );
        /* Clamp fCos to [-1; 1] range */
        fCos = vminq_f32( MathlibNEON::ONE, vmaxq_f32( MathlibNEON::NEG_ONE, fCos ) );

        /* Invert the rotation for shortest path? */
        /* m = fCos < 0.0f ? -1.0f : 1.0f; */
        ArrayReal m = MathlibNEON::Cmov4( MathlibNEON::NEG_ONE, MathlibNEON::ONE,
                                            vcltq_f32( fCos, vdupq_n_f32(0.0f) ) /*&& shortestPath*/ );
        ArrayQuaternion rkT(
                        vmulq_f32( rkQ.mChunkBase[0], m ),
                        vmulq_f32( rkQ.mChunkBase[1], m ),
                        vmulq_f32( rkQ.mChunkBase[2], m ),
                        vmulq_f32( rkQ.mChunkBase[3], m ) );

        ArrayReal fSin = MathlibNEON::Sqrt( vsubq_f32( MathlibNEON::ONE, vmulq_f32( fCos, fCos ) ) );

        /* ATan2 in original Quaternion is slightly absurd, because fSin was derived from
           fCos (hence never negative) which makes ACos a better replacement. ACos is much
           faster than ATan2, as long as the input is whithin range [-1; 1], otherwise the generated
           NaNs make it slower (whether clamping the input outweights the benefit is
           arguable). We use ACos4 to avoid implementing ATan2_4.
        */
        ArrayReal fAngle = MathlibNEON::ACos4( fCos );

        // mask = Abs( fCos ) < 1-epsilon
        ArrayMaskR mask    = vcltq_f32( MathlibNEON::Abs4( fCos ), MathlibNEON::OneMinusEpsilon );
        ArrayReal fInvSin = MathlibNEON::InvNonZero4( fSin );
        ArrayReal oneSubT = vsubq_f32( MathlibNEON::ONE, fT );
        // fCoeff1 = Sin( fT * fAngle ) * fInvSin
        ArrayReal fCoeff0 = vmulq_f32( MathlibNEON::Sin4( vmulq_f32( oneSubT, fAngle ) ), fInvSin );
        ArrayReal fCoeff1 = vmulq_f32( MathlibNEON::Sin4( vmulq_f32( fT, fAngle ) ), fInvSin );
        // fCoeff1 = mask ? fCoeff1 : fT; (switch to lerp when rkP & rkQ are too close->fSin=0, or 180°)
        fCoeff0 = MathlibNEON::CmovRobust( fCoeff0, oneSubT, mask );
        fCoeff1 = MathlibNEON::CmovRobust( fCoeff1, fT, mask );

        // retVal = fCoeff0 * rkP + fCoeff1 * rkT;
        rkT.mChunkBase[0] = vaddq_f32( vmulq_f32( rkP.mChunkBase[0], fCoeff0 ),
                                         vmulq_f32( rkT.mChunkBase[0], fCoeff1 ) ),
        rkT.mChunkBase[1] = vaddq_f32( vmulq_f32( rkP.mChunkBase[1], fCoeff0 ),
                                         vmulq_f32( rkT.mChunkBase[1], fCoeff1 ) ),
        rkT.mChunkBase[2] = vaddq_f32( vmulq_f32( rkP.mChunkBase[2], fCoeff0 ),
                                         vmulq_f32( rkT.mChunkBase[2], fCoeff1 ) ),
        rkT.mChunkBase[3] = vaddq_f32( vmulq_f32( rkP.mChunkBase[3], fCoeff0 ),
                                         vmulq_f32( rkT.mChunkBase[3], fCoeff1 ) );

        rkT.normalise();

        return rkT;
    }
    //-----------------------------------------------------------------------------------
    inline ArrayQuaternion ArrayQuaternion::nlerpShortest( ArrayReal fT, const ArrayQuaternion &rkP,
                                                            const ArrayQuaternion &rkQ )
    {
        //Flip the sign of rkQ when p.dot( q ) < 0 to get the shortest path
        ArrayReal signMask = vdupq_n_f32( -0.0f );
        ArrayReal sign = vandq_f32( signMask, rkP.Dot( rkQ ) );
        ArrayQuaternion tmpQ = ArrayQuaternion( veorq_f32( rkQ.mChunkBase[0], sign ),
                                                veorq_f32( rkQ.mChunkBase[1], sign ),
                                                veorq_f32( rkQ.mChunkBase[2], sign ),
                                                veorq_f32( rkQ.mChunkBase[3], sign ) );

        ArrayQuaternion retVal(
                _mm_madd_ps( fT, vsubq_f32( tmpQ.mChunkBase[0], rkP.mChunkBase[0] ), rkP.mChunkBase[0] ),
                _mm_madd_ps( fT, vsubq_f32( tmpQ.mChunkBase[1], rkP.mChunkBase[1] ), rkP.mChunkBase[1] ),
                _mm_madd_ps( fT, vsubq_f32( tmpQ.mChunkBase[2], rkP.mChunkBase[2] ), rkP.mChunkBase[2] ),
                _mm_madd_ps( fT, vsubq_f32( tmpQ.mChunkBase[3], rkP.mChunkBase[3] ), rkP.mChunkBase[3] ) );
        retVal.normalise();

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    inline ArrayQuaternion ArrayQuaternion::nlerp( ArrayReal fT, const ArrayQuaternion &rkP,
                                                        const ArrayQuaternion &rkQ )
    {
        ArrayQuaternion retVal(
                _mm_madd_ps( fT, vsubq_f32( rkQ.mChunkBase[0], rkP.mChunkBase[0] ), rkP.mChunkBase[0] ),
                _mm_madd_ps( fT, vsubq_f32( rkQ.mChunkBase[1], rkP.mChunkBase[1] ), rkP.mChunkBase[1] ),
                _mm_madd_ps( fT, vsubq_f32( rkQ.mChunkBase[2], rkP.mChunkBase[2] ), rkP.mChunkBase[2] ),
                _mm_madd_ps( fT, vsubq_f32( rkQ.mChunkBase[3], rkP.mChunkBase[3] ), rkP.mChunkBase[3] ) );
        retVal.normalise();

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    inline ArrayQuaternion ArrayQuaternion::Cmov4( const ArrayQuaternion &arg1,
                                                    const ArrayQuaternion &arg2, ArrayMaskR mask )
    {
        return ArrayQuaternion(
                MathlibNEON::Cmov4( arg1.mChunkBase[0], arg2.mChunkBase[0], mask ),
                MathlibNEON::Cmov4( arg1.mChunkBase[1], arg2.mChunkBase[1], mask ),
                MathlibNEON::Cmov4( arg1.mChunkBase[2], arg2.mChunkBase[2], mask ),
                MathlibNEON::Cmov4( arg1.mChunkBase[3], arg2.mChunkBase[3], mask ) );
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayQuaternion::mul( const ArrayQuaternion &inQ, ArrayVector3 &inOutVec )
    {
        // nVidia SDK implementation
        ArrayVector3 qVec( inQ.mChunkBase[1], inQ.mChunkBase[2], inQ.mChunkBase[3] );

        ArrayVector3 uv = qVec.crossProduct( inOutVec );
        ArrayVector3 uuv    = qVec.crossProduct( uv );

        // uv = uv * (2.0f * w)
        ArrayReal w2 = vaddq_f32( inQ.mChunkBase[0], inQ.mChunkBase[0] );
        uv.mChunkBase[0] = vmulq_f32( uv.mChunkBase[0], w2 );
        uv.mChunkBase[1] = vmulq_f32( uv.mChunkBase[1], w2 );
        uv.mChunkBase[2] = vmulq_f32( uv.mChunkBase[2], w2 );

        // uuv = uuv * 2.0f
        uuv.mChunkBase[0] = vaddq_f32( uuv.mChunkBase[0], uuv.mChunkBase[0] );
        uuv.mChunkBase[1] = vaddq_f32( uuv.mChunkBase[1], uuv.mChunkBase[1] );
        uuv.mChunkBase[2] = vaddq_f32( uuv.mChunkBase[2], uuv.mChunkBase[2] );

        //inOutVec = v + uv + uuv
        inOutVec.mChunkBase[0] = vaddq_f32( inOutVec.mChunkBase[0],
                                    vaddq_f32( uv.mChunkBase[0], uuv.mChunkBase[0] ) );
        inOutVec.mChunkBase[1] = vaddq_f32( inOutVec.mChunkBase[1],
                                    vaddq_f32( uv.mChunkBase[1], uuv.mChunkBase[1] ) );
        inOutVec.mChunkBase[2] = vaddq_f32( inOutVec.mChunkBase[2],
                                    vaddq_f32( uv.mChunkBase[2], uuv.mChunkBase[2] ) );
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
        tmp = vmaxq_f32( vdupq_n_f32(0.0f),
                          vaddq_f32( vaddq_f32( MathlibNEON::ONE, m00 ), vaddq_f32( m11, m22 ) ) );
        mChunkBase[0] = vmulq_f32( MathlibNEON::Sqrt( tmp ), MathlibNEON::HALF );

        //x = sqrt( max( 0, (1 + m00) - (m11 + m22) ) ) * 0.5f;
        tmp = vmaxq_f32( vdupq_n_f32(0.0f),
                          vsubq_f32( vaddq_f32( MathlibNEON::ONE, m00 ), vaddq_f32( m11, m22 ) ) );
        mChunkBase[1] = vmulq_f32( MathlibNEON::Sqrt( tmp ), MathlibNEON::HALF );

        //y = sqrt( max( 0, (1 - m00) + (m11 - m22) ) ) * 0.5f;
        tmp = vmaxq_f32( vdupq_n_f32(0.0f),
                          vaddq_f32( vsubq_f32( MathlibNEON::ONE, m00 ), vsubq_f32( m11, m22 ) ) );
        mChunkBase[2] = vmulq_f32( MathlibNEON::Sqrt( tmp ), MathlibNEON::HALF );

        //z = sqrt( max( 0, (1 - m00) - (m11 - m22) ) ) * 0.5f;
        tmp = vmaxq_f32( vdupq_n_f32(0.0f),
                          vsubq_f32( vsubq_f32( MathlibNEON::ONE, m00 ), vsubq_f32( m11, m22 ) ) );
        mChunkBase[3] = vmulq_f32( MathlibNEON::Sqrt( tmp ), MathlibNEON::HALF );

        //x = _copysign( x, m21 - m12 ); --> (x & 0x7FFFFFFF) | ((m21 - m12) & 0x80000000)
        //y = _copysign( y, m02 - m20 ); --> (y & 0x7FFFFFFF) | ((m02 - m20) & 0x80000000)
        //z = _copysign( z, m10 - m01 ); --> (z & 0x7FFFFFFF) | ((m10 - m01) & 0x80000000)
        tmp = vandq_f32( vsubq_f32( m21, m12 ), MathlibNEON::SIGN_MASK );
        mChunkBase[1] = vorrq_f32( vnand_f32( MathlibNEON::SIGN_MASK, mChunkBase[1] ), tmp );
        tmp = vandq_f32( vsubq_f32( m02, m20 ), MathlibNEON::SIGN_MASK );
        mChunkBase[2] = vorrq_f32( vnand_f32( MathlibNEON::SIGN_MASK, mChunkBase[2] ), tmp );
        tmp = vandq_f32( vsubq_f32( m10, m01 ), MathlibNEON::SIGN_MASK );
        mChunkBase[3] = vorrq_f32( vnand_f32( MathlibNEON::SIGN_MASK, mChunkBase[3] ), tmp );
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayQuaternion::FromAngleAxis( const ArrayRadian& rfAngle, const ArrayVector3& rkAxis )
    {
        // assert:  axis[] is unit length
        //
        // The quaternion representing the rotation is
        //   q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k)

        ArrayReal fHalfAngle( vmulq_f32( rfAngle.valueRadians(), MathlibNEON::HALF ) );

        ArrayReal fSin;
        MathlibNEON::SinCos4( fHalfAngle, fSin, mChunkBase[0] );

        ArrayReal * RESTRICT_ALIAS chunkBase = mChunkBase;
        const ArrayReal * RESTRICT_ALIAS rkAxisChunkBase = rkAxis.mChunkBase;

        chunkBase[1] = vmulq_f32( fSin, rkAxisChunkBase[0] ); //x = fSin*rkAxis.x;
        chunkBase[2] = vmulq_f32( fSin, rkAxisChunkBase[1] ); //y = fSin*rkAxis.y;
        chunkBase[3] = vmulq_f32( fSin, rkAxisChunkBase[2] ); //z = fSin*rkAxis.z;
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayQuaternion::ToAngleAxis( ArrayRadian &rfAngle, ArrayVector3 &rkAxis ) const
    {
        // The quaternion representing the rotation is
        //   q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k)
        ArrayReal sqLength = vaddq_f32( vaddq_f32(
                                vmulq_f32( mChunkBase[1], mChunkBase[1] ),  //(x * x +
                                vmulq_f32( mChunkBase[2], mChunkBase[2] ) ),    //y * y) +
                                vmulq_f32( mChunkBase[3], mChunkBase[3] ) );    //z * z )

        ArrayMaskR mask     = vcgtq_f32( sqLength, vdupq_n_f32(0.0f) ); //mask = sqLength > 0

        //sqLength = sqLength > 0 ? sqLength : 1; so that invSqrt doesn't give NaNs or Infs
        //when 0 (to avoid using CmovRobust just to select the non-nan results)
        sqLength = MathlibNEON::Cmov4( sqLength, MathlibNEON::ONE,
                                        vcgtq_f32( sqLength, MathlibNEON::FLOAT_MIN ) );
        ArrayReal fInvLength = MathlibNEON::InvSqrtNonZero4( sqLength );

        const ArrayReal acosW = MathlibNEON::ACos4( mChunkBase[0] );
        rfAngle = MathlibNEON::Cmov4( //sqLength > 0 ? (2 * ACos(w)) : 0
                    vaddq_f32( acosW, acosW ),
                    vdupq_n_f32(0.0f), mask );

        rkAxis.mChunkBase[0] = MathlibNEON::Cmov4(  //sqLength > 0 ? (x * fInvLength) : 1
                                    vmulq_f32( mChunkBase[1], fInvLength ), MathlibNEON::ONE, mask );
        rkAxis.mChunkBase[1] = MathlibNEON::Cmov4(  //sqLength > 0 ? (y * fInvLength) : 0
                                    vmulq_f32( mChunkBase[2], fInvLength ), vdupq_n_f32(0.0f), mask );
        rkAxis.mChunkBase[2] = MathlibNEON::Cmov4(  //sqLength > 0 ? (y * fInvLength) : 0
                                    vmulq_f32( mChunkBase[3], fInvLength ), vdupq_n_f32(0.0f), mask );
    }
    //-----------------------------------------------------------------------------------
    inline ArrayVector3 ArrayQuaternion::xAxis( void ) const
    {
        ArrayReal fTy  = vaddq_f32( mChunkBase[2], mChunkBase[2] );     // 2 * y
        ArrayReal fTz  = vaddq_f32( mChunkBase[3], mChunkBase[3] );     // 2 * z
        ArrayReal fTwy = vmulq_f32( fTy, mChunkBase[0] );                   // fTy*w;
        ArrayReal fTwz = vmulq_f32( fTz, mChunkBase[0] );                   // fTz*w;
        ArrayReal fTxy = vmulq_f32( fTy, mChunkBase[1] );                   // fTy*x;
        ArrayReal fTxz = vmulq_f32( fTz, mChunkBase[1] );                   // fTz*x;
        ArrayReal fTyy = vmulq_f32( fTy, mChunkBase[2] );                   // fTy*y;
        ArrayReal fTzz = vmulq_f32( fTz, mChunkBase[3] );                   // fTz*z;

        return ArrayVector3(
                vsubq_f32( MathlibNEON::ONE, vaddq_f32( fTyy, fTzz ) ),
                vaddq_f32( fTxy, fTwz ),
                vsubq_f32( fTxz, fTwy ) );
    }
    //-----------------------------------------------------------------------------------
    inline ArrayVector3 ArrayQuaternion::yAxis( void ) const
    {
        ArrayReal fTx  = vaddq_f32( mChunkBase[1], mChunkBase[1] );     // 2 * x
        ArrayReal fTy  = vaddq_f32( mChunkBase[2], mChunkBase[2] );     // 2 * y
        ArrayReal fTz  = vaddq_f32( mChunkBase[3], mChunkBase[3] );     // 2 * z
        ArrayReal fTwx = vmulq_f32( fTx, mChunkBase[0] );                   // fTx*w;
        ArrayReal fTwz = vmulq_f32( fTz, mChunkBase[0] );                   // fTz*w;
        ArrayReal fTxx = vmulq_f32( fTx, mChunkBase[1] );                   // fTx*x;
        ArrayReal fTxy = vmulq_f32( fTy, mChunkBase[1] );                   // fTy*x;
        ArrayReal fTyz = vmulq_f32( fTz, mChunkBase[2] );                   // fTz*y;
        ArrayReal fTzz = vmulq_f32( fTz, mChunkBase[3] );                   // fTz*z;

        return ArrayVector3(
                vsubq_f32( fTxy, fTwz ),
                vsubq_f32( MathlibNEON::ONE, vaddq_f32( fTxx, fTzz ) ),
                vaddq_f32( fTyz, fTwx ) );
    }
    //-----------------------------------------------------------------------------------
    inline ArrayVector3 ArrayQuaternion::zAxis( void ) const
    {
        ArrayReal fTx  = vaddq_f32( mChunkBase[1], mChunkBase[1] );     // 2 * x
        ArrayReal fTy  = vaddq_f32( mChunkBase[2], mChunkBase[2] );     // 2 * y
        ArrayReal fTz  = vaddq_f32( mChunkBase[3], mChunkBase[3] );     // 2 * z
        ArrayReal fTwx = vmulq_f32( fTx, mChunkBase[0] );                   // fTx*w;
        ArrayReal fTwy = vmulq_f32( fTy, mChunkBase[0] );                   // fTy*w;
        ArrayReal fTxx = vmulq_f32( fTx, mChunkBase[1] );                   // fTx*x;
        ArrayReal fTxz = vmulq_f32( fTz, mChunkBase[1] );                   // fTz*x;
        ArrayReal fTyy = vmulq_f32( fTy, mChunkBase[2] );                   // fTy*y;
        ArrayReal fTyz = vmulq_f32( fTz, mChunkBase[2] );                   // fTz*y;

        return ArrayVector3(
                vaddq_f32( fTxz, fTwy ),
                vsubq_f32( fTyz, fTwx ),
                vsubq_f32( MathlibNEON::ONE, vaddq_f32( fTxx, fTyy ) ) );
    }
    //-----------------------------------------------------------------------------------
    inline ArrayReal ArrayQuaternion::Dot( const ArrayQuaternion& rkQ ) const
    {
        return
        vaddq_f32( vaddq_f32( vaddq_f32(
            vmulq_f32( mChunkBase[0], rkQ.mChunkBase[0] ) , //((w * vec.w   +
            vmulq_f32( mChunkBase[1], rkQ.mChunkBase[1] ) ),    //  x * vec.x ) +
            vmulq_f32( mChunkBase[2], rkQ.mChunkBase[2] ) ), //  y * vec.y ) +
            vmulq_f32( mChunkBase[3], rkQ.mChunkBase[3] ) );    //  z * vec.z
    }
    //-----------------------------------------------------------------------------------
    inline ArrayReal ArrayQuaternion::Norm( void ) const
    {
        return
        vaddq_f32( vaddq_f32( vaddq_f32(
            vmulq_f32( mChunkBase[0], mChunkBase[0] ) , //((w * w   +
            vmulq_f32( mChunkBase[1], mChunkBase[1] ) ),    //  x * x ) +
            vmulq_f32( mChunkBase[2], mChunkBase[2] ) ), //  y * y ) +
            vmulq_f32( mChunkBase[3], mChunkBase[3] ) );    //  z * z
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayQuaternion::normalise( void )
    {
        ArrayReal sqLength = vaddq_f32( vaddq_f32( vaddq_f32(
            vmulq_f32( mChunkBase[0], mChunkBase[0] ) , //((w * w   +
            vmulq_f32( mChunkBase[1], mChunkBase[1] ) ),    //  x * x ) +
            vmulq_f32( mChunkBase[2], mChunkBase[2] ) ), //  y * y ) +
            vmulq_f32( mChunkBase[3], mChunkBase[3] ) );    //  z * z

        //Convert sqLength's 0s into 1, so that zero vectors remain as zero
        //Denormals are treated as 0 during the check.
        //Note: We could create a mask now and nuke nans after InvSqrt, however
        //generating the nans could impact performance in some architectures
        sqLength = MathlibNEON::Cmov4( sqLength, MathlibNEON::ONE,
                                        vcgtq_f32( sqLength, MathlibNEON::FLOAT_MIN ) );
        ArrayReal invLength = MathlibNEON::InvSqrtNonZero4( sqLength );
        mChunkBase[0] = vmulq_f32( mChunkBase[0], invLength ); //w * invLength
        mChunkBase[1] = vmulq_f32( mChunkBase[1], invLength ); //x * invLength
        mChunkBase[2] = vmulq_f32( mChunkBase[2], invLength ); //y * invLength
        mChunkBase[3] = vmulq_f32( mChunkBase[3], invLength ); //z * invLength
    }
    //-----------------------------------------------------------------------------------
    inline ArrayQuaternion ArrayQuaternion::Inverse( void ) const
    {
        ArrayReal fNorm = vaddq_f32( vaddq_f32( vaddq_f32(
            vmulq_f32( mChunkBase[0], mChunkBase[0] ) , //((w * w   +
            vmulq_f32( mChunkBase[1], mChunkBase[1] ) ),    //  x * x ) +
            vmulq_f32( mChunkBase[2], mChunkBase[2] ) ), //  y * y ) +
            vmulq_f32( mChunkBase[3], mChunkBase[3] ) );    //  z * z;

        //Will return a zero Quaternion if original is zero length (Quaternion's behavior)
        fNorm = MathlibNEON::Cmov4( fNorm, MathlibNEON::ONE,
                                    vcgtq_f32( fNorm, MathlibNEON::fEpsilon ) );
        ArrayReal invNorm    = MathlibNEON::Inv4( fNorm );
        ArrayReal negInvNorm = vmulq_f32( invNorm, MathlibNEON::NEG_ONE );

        return ArrayQuaternion(
            vmulq_f32( mChunkBase[0], invNorm ),        //w * invNorm
            vmulq_f32( mChunkBase[1], negInvNorm ), //x * -invNorm
            vmulq_f32( mChunkBase[2], negInvNorm ), //y * -invNorm
            vmulq_f32( mChunkBase[3], negInvNorm ) );   //z * -invNorm
    }
    //-----------------------------------------------------------------------------------
    inline ArrayQuaternion ArrayQuaternion::UnitInverse( void ) const
    {
        return ArrayQuaternion(
            mChunkBase[0],                                          //w
            vmulq_f32( mChunkBase[1], MathlibNEON::NEG_ONE ),       //-x
            vmulq_f32( mChunkBase[2], MathlibNEON::NEG_ONE ),       //-y
            vmulq_f32( mChunkBase[3], MathlibNEON::NEG_ONE ) ); //-z
    }
    //-----------------------------------------------------------------------------------
    inline ArrayQuaternion ArrayQuaternion::Exp( void ) const
    {
        // If q = A*(x*i+y*j+z*k) where (x,y,z) is unit length, then
        // exp(q) = cos(A)+sin(A)*(x*i+y*j+z*k).  If sin(A) is near zero,
        // use exp(q) = cos(A)+A*(x*i+y*j+z*k) since A/sin(A) has limit 1.

        ArrayReal fAngle = MathlibNEON::Sqrt( vaddq_f32( vaddq_f32(				//sqrt(
                                vmulq_f32( mChunkBase[1], mChunkBase[1] ),      //(x * x +
                                vmulq_f32( mChunkBase[2], mChunkBase[2] ) ),    // y * y) +
                                vmulq_f32( mChunkBase[3], mChunkBase[3] ) ) );  // z * z )

        ArrayReal w, fSin;
        MathlibNEON::SinCos4( fAngle, fSin, w );

        //coeff = Abs(fSin) >= msEpsilon ? (fSin / fAngle) : 1.0f;
        ArrayReal coeff = MathlibNEON::CmovRobust( vdivq_f32( fSin, fAngle ), MathlibNEON::ONE,
                                vcgeq_f32( MathlibNEON::Abs4( fSin ), MathlibNEON::fEpsilon ) );
        return ArrayQuaternion(
            w,                                          //cos( fAngle )
            vmulq_f32( mChunkBase[1], coeff ),      //x * coeff
            vmulq_f32( mChunkBase[2], coeff ),      //y * coeff
            vmulq_f32( mChunkBase[3], coeff ) );        //z * coeff
    }
    //-----------------------------------------------------------------------------------
    inline ArrayQuaternion ArrayQuaternion::Log( void ) const
    {
        // If q = cos(A)+sin(A)*(x*i+y*j+z*k) where (x,y,z) is unit length, then
        // log(q) = A*(x*i+y*j+z*k).  If sin(A) is near zero, use log(q) =
        // sin(A)*(x*i+y*j+z*k) since sin(A)/A has limit 1.

        ArrayReal fAngle    = MathlibNEON::ACos4( mChunkBase[0] );
        ArrayReal fSin      = MathlibNEON::Sin4( fAngle );

        //mask = Math::Abs(w) < 1.0 && Math::Abs(fSin) >= msEpsilon
        ArrayMaskR mask = vandq_u32(
                            vcltq_f32( MathlibNEON::Abs4( mChunkBase[0] ), MathlibNEON::ONE ),
                            vcgeq_f32( MathlibNEON::Abs4( fSin ), MathlibNEON::fEpsilon ) );

        //coeff = mask ? (fAngle / fSin) : 1.0
        //Unlike Exp(), we can use InvNonZero4 (which is faster) instead of div because we know for
        //sure CMov will copy the 1 instead of the NaN when fSin is close to zero, guarantee we might
        //not have in Exp()
        ArrayReal coeff = MathlibNEON::CmovRobust( vmulq_f32( fAngle, MathlibNEON::InvNonZero4( fSin ) ),
                                                    MathlibNEON::ONE, mask );

        return ArrayQuaternion(
            vdupq_n_f32(0.0f),                          //w = 0
            vmulq_f32( mChunkBase[1], coeff ),      //x * coeff
            vmulq_f32( mChunkBase[2], coeff ),      //y * coeff
            vmulq_f32( mChunkBase[3], coeff ) );        //z * coeff
    }
    //-----------------------------------------------------------------------------------
    inline ArrayVector3 ArrayQuaternion::operator * ( const ArrayVector3 &v ) const
    {
        // nVidia SDK implementation
        ArrayVector3 qVec( mChunkBase[1], mChunkBase[2], mChunkBase[3] );

        ArrayVector3 uv = qVec.crossProduct( v );
        ArrayVector3 uuv    = qVec.crossProduct( uv );

        // uv = uv * (2.0f * w)
        ArrayReal w2 = vaddq_f32( mChunkBase[0], mChunkBase[0] );
        uv.mChunkBase[0] = vmulq_f32( uv.mChunkBase[0], w2 );
        uv.mChunkBase[1] = vmulq_f32( uv.mChunkBase[1], w2 );
        uv.mChunkBase[2] = vmulq_f32( uv.mChunkBase[2], w2 );

        // uuv = uuv * 2.0f
        uuv.mChunkBase[0] = vaddq_f32( uuv.mChunkBase[0], uuv.mChunkBase[0] );
        uuv.mChunkBase[1] = vaddq_f32( uuv.mChunkBase[1], uuv.mChunkBase[1] );
        uuv.mChunkBase[2] = vaddq_f32( uuv.mChunkBase[2], uuv.mChunkBase[2] );

        //uv = v + uv + uuv
        uv.mChunkBase[0] = vaddq_f32( v.mChunkBase[0],
                                vaddq_f32( uv.mChunkBase[0], uuv.mChunkBase[0] ) );
        uv.mChunkBase[1] = vaddq_f32( v.mChunkBase[1],
                                vaddq_f32( uv.mChunkBase[1], uuv.mChunkBase[1] ) );
        uv.mChunkBase[2] = vaddq_f32( v.mChunkBase[2],
                                vaddq_f32( uv.mChunkBase[2], uuv.mChunkBase[2] ) );

        return uv;
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayQuaternion::Cmov4( ArrayMaskR mask, const ArrayQuaternion &replacement )
    {
        ArrayReal * RESTRICT_ALIAS aChunkBase = mChunkBase;
        const ArrayReal * RESTRICT_ALIAS bChunkBase = replacement.mChunkBase;
        aChunkBase[0] = MathlibNEON::Cmov4( aChunkBase[0], bChunkBase[0], mask );
        aChunkBase[1] = MathlibNEON::Cmov4( aChunkBase[1], bChunkBase[1], mask );
        aChunkBase[2] = MathlibNEON::Cmov4( aChunkBase[2], bChunkBase[2], mask );
        aChunkBase[3] = MathlibNEON::Cmov4( aChunkBase[3], bChunkBase[3], mask );
    }
}
