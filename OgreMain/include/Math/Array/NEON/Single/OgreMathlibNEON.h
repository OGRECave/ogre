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

#ifndef __MathlibNEON_H__
#define __MathlibNEON_H__

#if __OGRE_HAVE_NEON

#ifndef __Mathlib_H__
    #error "Don't include this file directly. include Math/Array/OgreMathlib.h"
#endif

#include "OgrePrerequisites.h"

namespace Ogre
{
    // A few shortcut functions to emulate instructions included with SSE2 that NEON lacks.
    // See http://rcl-rs-vvg.blogspot.com/2010/08/simd-etudes.html

    // Similar to _mm_movemask_ps
    static inline uint32 vmovemaskq_u32( uint32x4_t conditions )
    {
        const uint32x4_t qMask = { 1, 2, 4, 8 };
        const uint32x4_t qAnded = vandq_u32( conditions, qMask );

        // These two are no-ops, they only tell compiler to treat Q register as two D regs
        const uint32x2_t dHigh = vget_high_u32( qAnded );
        const uint32x2_t dLow = vget_low_u32( qAnded );

        const uint32x2_t dOred = vorr_u32( dHigh, dLow );
        const uint32x2_t dOred2 = dOred;
        // For some reason vpadd_u32 doesn't work right if both arguments
        // are the same variable so we use a copy.
        const uint32x2_t dMask = vpadd_u32( dOred, dOred2 );
        return vget_lane_u32( dMask, 0 );
    }

    static inline ArrayInt vnand_s32(ArrayInt a, ArrayInt b)
    {
        return vandq_s32(vmvnq_s32(a), b);
    }

    static inline ArrayReal vnand_f32(ArrayReal a, ArrayReal b)
    {
        return vreinterpretq_f32_u32( vandq_u32( vmvnq_u32(vreinterpretq_u32_f32(a)),
                                                 vreinterpretq_u32_f32(b) ) );
    }

#define vnand_u32( a, b ) vandq_u32( vmvnq_u32(a), b )

    static inline ArrayReal vorrq_f32(ArrayReal a, ArrayReal b)
    {
        return (ArrayReal)vorrq_s32((ArrayInt)a, (ArrayInt)b);
    }

    static inline ArrayReal vdivq_f32( ArrayReal num, ArrayReal den )
    {
        const ArrayReal inv0 = vrecpeq_f32(den);
        const ArrayReal step0 = vrecpsq_f32(inv0, den);

        const ArrayReal inv1 = vmulq_f32(step0, inv0);
        return vmulq_f32( num, inv1 );
    }

#define vandq_f32( a, b ) vreinterpretq_f32_u32( vandq_u32( vreinterpretq_u32_f32( a ), vreinterpretq_u32_f32( b ) ) )
#define veorq_f32( a, b ) vreinterpretq_f32_u32( veorq_u32( vreinterpretq_u32_f32( a ), vreinterpretq_u32_f32( b ) ) )
#define vandq_f32u32( a, b ) vreinterpretq_f32_u32( vandq_u32( vreinterpretq_u32_f32( a ), b ) )

#define _MM_SHUFFLE(z, y, x, w) (((z) << 6) | ((y) << 4) | ((x) << 2) | (w))

    static inline ArrayReal vshuf_f32(ArrayReal a, ArrayReal b, unsigned int idx)
    {
        float x, y, z, w;

        // First index used to retrieve from a
        switch((idx >> 6) & 0x03)
        {
            case 0:
                z = vgetq_lane_f32(a, 0);
                break;
            case 1:
                z = vgetq_lane_f32(a, 1);
                break;
            case 2:
                z = vgetq_lane_f32(a, 2);
                break;
            case 3:
                z = vgetq_lane_f32(a, 3);
                break;
        }

        // Second index used to retrieve from a
        switch((idx >> 4) & 0x03)
        {
            case 0:
                y = vgetq_lane_f32(a, 0);
                break;
            case 1:
                y = vgetq_lane_f32(a, 1);
                break;
            case 2:
                y = vgetq_lane_f32(a, 2);
                break;
            case 3:
                y = vgetq_lane_f32(a, 3);
                break;
        }

        // Third index used to retrieve from b
        switch((idx >> 2) & 0x03)
        {
            case 0:
                x = vgetq_lane_f32(b, 0);
                break;
            case 1:
                x = vgetq_lane_f32(b, 1);
                break;
            case 2:
                x = vgetq_lane_f32(b, 2);
                break;
            case 3:
                x = vgetq_lane_f32(b, 3);
                break;
        }

        // Four index used to retrieve from b
        switch(idx & 0x03)
        {
            case 0:
                w = vgetq_lane_f32(b, 0);
                break;
            case 1:
                w = vgetq_lane_f32(b, 1);
                break;
            case 2:
                w = vgetq_lane_f32(b, 2);
                break;
            case 3:
                w = vgetq_lane_f32(b, 3);
                break;
        }

        return (ArrayReal) { x, y, z, w };
    }

    inline uint32x4_t vcneqq_f32(ArrayReal a, ArrayReal b)
    {
        return vceqq_u32(vceqq_f32(a, b), vdupq_n_u32( 0 ));
    }

    class ArrayRadian
    {
        ArrayReal mRad;

    public:
        explicit ArrayRadian ( ArrayReal r ) : mRad( r ) {}
        //ArrayRadian ( const ArrayDegree& d );
        ArrayRadian& operator = ( const ArrayReal &f )      { mRad = f; return *this; }
        ArrayRadian& operator = ( const ArrayRadian &r )    { mRad = r.mRad; return *this; }
        //ArrayRadian& operator = ( const ArrayDegree& d );

        //ArrayReal valueDegrees() const; // see bottom of this file
        ArrayReal valueRadians() const                      { return mRad; }

        inline const ArrayRadian& operator + () const;
        inline ArrayRadian operator + ( const ArrayRadian& r ) const;
        //inline ArrayRadian operator + ( const ArrayDegree& d ) const;
        inline ArrayRadian& operator += ( const ArrayRadian& r );
        //inline ArrayRadian& operator += ( const ArrayDegree& d );
        inline ArrayRadian operator - () const;
        inline ArrayRadian operator - ( const ArrayRadian& r ) const;
        //inline ArrayRadian operator - ( const ArrayDegree& d ) const;
        inline ArrayRadian& operator -= ( const ArrayRadian& r );
        //inline ArrayRadian& operator -= ( const ArrayDegree& d );
        inline ArrayRadian operator * ( ArrayReal f ) const;
        inline ArrayRadian operator * ( const ArrayRadian& f ) const;
        inline ArrayRadian& operator *= ( ArrayReal f );
        inline ArrayRadian operator / ( ArrayReal f ) const;
        inline ArrayRadian& operator /= ( ArrayReal f );

        inline ArrayMaskR operator <  ( const ArrayRadian& r ) const;
        inline ArrayMaskR operator <= ( const ArrayRadian& r ) const;
        inline ArrayMaskR operator == ( const ArrayRadian& r ) const;
        inline ArrayMaskR operator != ( const ArrayRadian& r ) const;
        inline ArrayMaskR operator >= ( const ArrayRadian& r ) const;
        inline ArrayMaskR operator >  ( const ArrayRadian& r ) const;
    };

    class _OgreExport MathlibNEON
    {
    public:
        static const ArrayReal HALF;        //0.5f, 0.5f, 0.5f, 0.5f
        static const ArrayReal ONE;         //1.0f, 1.0f, 1.0f, 1.0f
        static const ArrayReal THREE;       //3.0f, 3.0f, 3.0f, 3.0f
        static const ArrayReal NEG_ONE;     //-1.0f, -1.0f, -1.0f, -1.0f
        static const ArrayReal PI;          //PI, PI, PI, PI
        static const ArrayReal TWO_PI;      //2*PI, 2*PI, 2*PI, 2*PI
        static const ArrayReal ONE_DIV_2PI; //1 / 2PI, 1 / 2PI, 1 / 2PI, 1 / 2PI
        static const ArrayReal fEpsilon;    //1e-6f, 1e-6f, 1e-6f, 1e-6f
        static const ArrayReal fSqEpsilon;  //1e-12f, 1e-12f, 1e-12f, 1e-12f
        static const ArrayReal OneMinusEpsilon;//1 - 1e-6f, 1 - 1e-6f, 1 - 1e-6f, 1 - 1e-6f
        static const ArrayReal fDeg2Rad;    //Math::fDeg2Rad, Math::fDeg2Rad, Math::fDeg2Rad, Math::fDeg2Rad
        static const ArrayReal fRad2Deg;    //Math::fRad2Deg, Math::fRad2Deg, Math::fRad2Deg, Math::fRad2Deg
        static const ArrayReal FLOAT_MIN;   //FLT_MIN, FLT_MIN, FLT_MIN, FLT_MIN
        static const ArrayReal SIGN_MASK;   //0x80000000, 0x80000000, 0x80000000, 0x80000000
        //INFINITE is taken in Windows, INFINITY by C99 (bloody macros). A joke on Infinite Tea
        static const ArrayReal INFINITEA;   //Inf, Inf, Inf, Inf
        static const ArrayReal MAX_NEG;     //Max negative number (x4)
        static const ArrayReal MAX_POS;     //Max negative number (x4)
        static const ArrayReal LAST_AFFINE_COLUMN;//0, 0, 0, 1

        /** Returns the absolute values of each 4 floats
            @param
                4 floating point values
            @return
                abs( a )
        */
        static inline ArrayReal Abs4( ArrayReal a )
        {
            return vnand_f32( vdupq_n_f32( -0.0f ), a );
        }

        /** Branchless conditional move for 4 floating point values
            @remarks
                Will NOT work if any of the arguments contains Infinite
                or NaNs or non-floating point values. If an exact binary
                copy is needed, @see CmovRobust
            @param
                4 floating point values. Can't be NaN or Inf
            @param
                4 floating point values. Can't be NaN or Inf
            @param
                4 values containing either 0 or 0xffffffff
                Any other value, the result is undefined
            @return
                r[i] = mask[i] != 0 ? arg1[i] : arg2[i]
                Another way to say it:
                    if( maskCondition[i] == true )
                        r[i] = arg1[i];
                    else
                        arg2[i];
        */
        static inline ArrayReal Cmov4( ArrayReal arg1, ArrayReal arg2, ArrayMaskR mask )
        {
            assert( vmovemaskq_u32( vceqq_f32( arg1, arg1 ) ) == 0x0f &&
                    vmovemaskq_u32( vceqq_f32( arg2, arg2 ) ) == 0x0f &&
                    "Passing NaN values to CMov4" );
#ifndef  NDEBUG
            ArrayReal newNan1 = vmulq_f32( arg1, vdupq_n_f32(0.0f) ); //+-Inf * 0 = nan
            ArrayReal newNan2 = vmulq_f32( arg2, vdupq_n_f32(0.0f) ); //+-Inf * 0 = nan
            assert( vmovemaskq_u32( vceqq_f32( newNan1, newNan1 ) ) == 0x0f &&
                    vmovemaskq_u32( vceqq_f32( newNan2, newNan2 ) ) == 0x0f &&
                    "Passing +/- Infinity values to CMov4" );
#endif

            ArrayReal t = vsubq_f32( arg1, arg2 );              // t = arg1 - arg2
            return vaddq_f32( arg2, vandq_f32u32( t, mask ) ); // r = arg2 + (t & mask)
        }

        /** Robust, branchless conditional move for a 128-bit value.
            @remarks
                If you're looking to copy 4 floating point values that do
                not contain Inf or Nans, @see Cmov4 which is faster.
                This is because switching between registers flagged as
                floating point to integer and back has a latency delay

                For more information refer to Chapter 3.5.2.3
                Bypass between Execution Domains, Intel¨ 64 and IA-32
                Architectures Optimization Reference Manual Order
                Number: 248966-026 April (and also Table 2-12)
            @param
                A value containing 128 bits
            @param
                A value containing 128 bits
            @param
                Mask, each bit is evaluated
            @return
                For each bit:
                r[i] = mask[i] != 0 ? arg1[i] : arg2[i]
                Another way to say it:
                    if( maskCondition[i] == true )
                        r[i] = arg1[i];
                    else
                        arg2[i];
        */
        static inline ArrayReal CmovRobust( ArrayReal arg1, ArrayReal arg2, ArrayMaskR mask )
        {
            return vreinterpretq_f32_u32(
                    vorrq_u32( vandq_u32( vreinterpretq_u32_f32( arg1 ), mask ),
                               vnand_u32( mask, vreinterpretq_u32_f32( arg2 ) ) ) );
        }
        static inline ArrayInt CmovRobust( ArrayInt arg1, ArrayInt arg2, ArrayMaskI mask )
        {
            return vorrq_s32( vandq_s32( arg1, vreinterpretq_s32_u32( mask ) ),
                              vnand_s32( vreinterpretq_s32_u32( mask ), arg2 ) );
        }

        /** Returns the result of "a & b"
        @return
            r[i] = a[i] & b[i];
        */
        static inline ArrayReal And( ArrayReal a, ArrayReal b )
        {
            return vandq_f32( a, b );
        }
        static inline ArrayInt And( ArrayInt a, ArrayInt b )
        {
            return vandq_s32( a, b );
        }
        static inline ArrayInt And( ArrayInt a, ArrayMaskI b )
        {
            return vandq_s32( a, vreinterpretq_s32_u32( b ) );
        }
        static inline ArrayMaskI And( ArrayMaskI a, ArrayInt b )
        {
            return vreinterpretq_u32_s32( vandq_s32( vreinterpretq_s32_u32( a ), b ) );
        }
        static inline ArrayMaskI And( ArrayMaskI a, ArrayMaskI b )
        {
            return vandq_u32( a, b );
        }

        /** Returns the result of "a & b"
        @return
            r[i] = a[i] & b;
        */
        static inline ArrayInt And( ArrayInt a, uint32 b )
        {
            return vandq_s32( a, vdupq_n_s32( b ) );
        }

        /** Test if "a AND b" will result in non-zero, returning 0xffffffff on those cases
        @remarks
            Because there is no "not-equal" instruction in integer NEON, be need to do some
            bit flipping.
        @par
            Do not try to do a floating-point variation of this one. I already tried, but hit
            a wall: 0x80000000 & 0x80000000 => returns 0, because in floating point, -0 == 0
        @return
            r[i] = (a[i] & b[i]) ? 0xffffffff : 0;
        */
        static inline ArrayMaskI TestFlags4( ArrayInt a, ArrayInt b )
        {
            // !( (a & b) == 0 ) --> ( (a & b) == 0 ) ^ -1
            return veorq_u32( vceqq_s32( vandq_s32( a, b ), vdupq_n_s32(0) ),
                                    vdupq_n_u32( ~0 ) );
        }
        static inline ArrayMaskI TestFlags4( ArrayInt a, ArrayMaskI b )
        {
            // !( (a & b) == 0 ) --> ( (a & b) == 0 ) ^ -1
            return veorq_u32( vceqq_u32( vandq_u32( vreinterpretq_u32_s32( a ), b ),
                                     vdupq_n_u32(0) ), vdupq_n_u32( ~0 ) );
        }
        static inline ArrayMaskI TestFlags4( ArrayMaskI a,  ArrayInt b )
        {
            // !( (a & b) == 0 ) --> ( (a & b) == 0 ) ^ -1
            return veorq_u32( vceqq_u32( vandq_u32( a, vreinterpretq_u32_s32( b ) ),
                                     vdupq_n_u32(0) ), vdupq_n_u32( ~0 ) );
        }


        /** Returns the result of "a & ~b"
        @return
            r[i] = a[i] & ~b[i];
        */
        /*static inline ArrayInt AndNot( ArrayInt a, ArrayInt b )
        {
            return vnand_s32( b, a );
        }*/
        static inline ArrayMaskI AndNot( ArrayMaskI a, ArrayMaskI b )
        {
            return vnand_u32( b, a );
        }

        /** Returns the result of "a | b"
        @return
            r[i] = a[i] | b[i];
        */
        static inline ArrayReal Or( ArrayReal a, ArrayReal b )
        {
            return vorrq_f32( a, b );
        }
        static inline ArrayInt Or( ArrayInt a, ArrayInt b )
        {
            return vorrq_s32( a, b );
        }
        static inline ArrayMaskI Or( ArrayMaskI a, ArrayMaskI b )
        {
            return vorrq_u32( a, b );
        }

        /** Returns the result of "a < b"
        @return
            r[i] = a[i] < b[i] ? 0xffffffff : 0;
        */
        static inline ArrayMaskR CompareLess( ArrayReal a, ArrayReal b )
        {
            return vcltq_f32( a, b );
        }

        /** Returns the result of "a <= b"
         @return
         r[i] = a[i] <= b[i] ? 0xffffffff : 0;
         */
        static inline ArrayMaskR CompareLessEqual( ArrayReal a, ArrayReal b )
        {
            return vcleq_f32( a, b );
        }

        /** Returns the result of "a > b"
        @return
            r[i] = a[i] > b[i] ? 0xffffffff : 0;
        */
        static inline ArrayMaskR CompareGreater( ArrayReal a, ArrayReal b )
        {
            return vcgtq_f32( a, b );
        }

        /** Returns the result of "a >= b"
        @return
            r[i] = a[i] >= b[i] ? 0xffffffff : 0;
        */
        static inline ArrayMaskR CompareGreaterEqual( ArrayReal a, ArrayReal b )
        {
            return vcgeq_f32( a, b );
        }

        static inline ArrayReal SetAll( Real val )
        {
            return vdupq_n_f32( val );
        }

        static inline ArrayInt SetAll( uint32 val )
        {
            return vdupq_n_s32( val );
        }

        static inline void Set( ArrayReal &_dst, Real val, size_t index )
        {
            float *dst = reinterpret_cast<float*>( &_dst );
            dst[index] = val;
        }

        /** Returns the result of "a == std::numeric_limits<float>::infinity()"
        @return
            r[i] = a[i] == Inf ? 0xffffffff : 0;
        */
        static inline ArrayMaskR isInfinity( ArrayReal a )
        {
            return vceqq_f32( a, MathlibNEON::INFINITEA );
        }

        /// Returns the maximum value between a and b
        static inline ArrayReal Max( ArrayReal a, ArrayReal b )
        {
            return vmaxq_f32( a, b );
        }

		/// Returns the minimum value between a and b
        static inline ArrayReal Min( ArrayReal a, ArrayReal b )
        {
            return vminq_f32( a, b );
        }

        /** Returns the minimum value of all elements in a
        @return
            r[0] = min( a[0], a[1], a[2], a[3] )
        */
        static inline Real CollapseMin( ArrayReal a )
        {
            float32x2_t a_lo, a_hi, min;
            a_lo = vget_low_f32(a);
            a_hi = vget_high_f32(a);
            min = vpmin_f32(a_lo, a_hi);
            min = vpmin_f32(min, min);

            return vget_lane_f32(min, 0);
        }

        /** Returns the maximum value of all elements in a
        @return
            r[0] = max( a[0], a[1], a[2], a[3] )
        */
        static inline Real CollapseMax( ArrayReal a )
        {
            float32x2_t a_lo, a_hi, max;
            a_lo = vget_low_f32(a);
            a_hi = vget_high_f32(a);
            max = vpmax_f32(a_lo, a_hi);
            max = vpmax_f32(max, max);

            return vget_lane_f32(max, 0);
        }

        /** Returns the reciprocal of x
            @remarks
                If you have a very rough guarantees that you won't be feeding a zero,
                consider using @see InvNonZero4 because it's faster.

                Use SSE Newton-Raphson reciprocal estimate, accurate to 23 significant
                bits of the mantissa after an extra iteration, instead of the little
                12 bits of accuracy that vrecpsq_f32 gives us 
                In short, one Newton-Raphson Iteration:
                 f( i+1 ) = 2 * rcp( f ) - f * rcp( f ) * rcp( f )
                See Intel AP-803 (Application note), Order N¡ 243637-002 (if you can get it!)
                 "x0 = RCPSS(d)
                x1 = x0 * (2 - d * x0) = 2 * x0 - d * x0 * x0
                where x0 is the first approximation to the reciprocal of the divisor d, and x1 is a
                better approximation. You must use this formula before multiplying with the dividend."
            @param val
                4 floating point values. If it's zero, the returned value will be infinite,
                which is the correct result, but it's slower than InvNonZero4
            @return
                1 / x (packed as 4 floats)
        */
        static inline ArrayReal Inv4( ArrayReal val )
        {
            ArrayReal inv = vrecpeq_f32( val );
            ArrayReal twoRcp    = vaddq_f32( inv, inv );                    //2 * rcp( f )
            ArrayReal rightSide = vmulq_f32( val, vmulq_f32( inv, inv ) );  //f * rcp( f ) * rcp( f )
            rightSide = vreinterpretq_f32_u32(
                         vandq_u32( vreinterpretq_u32_f32( rightSide ),
                                    vcneqq_f32( val, vdupq_n_f32(0.0f) ) ) ); //Nuke this NaN
            return vsubq_f32( twoRcp, rightSide );
        }

        /** Returns the reciprocal of x
            @remarks
                If the input is zero, it will produce a NaN!!! (but it's faster)
                Note: Some architectures may slowdown when a NaN is produced, making this
                function slower than Inv4 for those cases
                @see Inv4

                Use SSE Newton-Raphson reciprocal estimate, accurate to 23 significant
                bits of the mantissa after an extra iteration, instead of the little
                12 bits of accuracy that vrecpsq_f32 gives us 
                In short, one Newton-Raphson Iteration:
                 f( i+1 ) = 2 * rcp( f ) - f * rcp( f ) * rcp( f )
                See Intel AP-803 (Application note), Order N¡ 243637-002 (if you can get it!)
                 "x0 = RCPSS(d)
                x1 = x0 * (2 - d * x0) = 2 * x0 - d * x0 * x0
                where x0 is the first approximation to the reciprocal of the divisor d, and x1 is a
                better approximation. You must use this formula before multiplying with the dividend."
            @param val
                4 floating point values. If it's zero, the returned value will be NaN
            @return
                1 / x (packed as 4 floats)
        */
        static inline ArrayReal InvNonZero4( ArrayReal val )
        {
            ArrayReal inv = vrecpeq_f32( val );
            ArrayReal twoRcp    = vaddq_f32( inv, inv );                    //2 * rcp( f )
            ArrayReal rightSide= vmulq_f32( val, vmulq_f32( inv, inv ) );   //f * rcp( f ) * rcp( f )
            return vsubq_f32( twoRcp, rightSide );
        }

        /** Returns the squared root of the reciprocal of x
            @remarks
                Use SSE Newton-Raphson reciprocal estimate, accurate to 23 significant
                bits of the mantissa after an extra iteration, instead of the little
                12 bits of accuracy that vrecpsq_f32 gives us 
                In short, one Newton-Raphson Iteration:
                     f( i+1 ) = 0.5 * rsqrt( f ) * ( 3 - f * rsqrt( f ) * rsqrt( f ) )
                See Intel AP-803 (Application note), Order N¡ 243637-002 (if you can get it!)
                "x0 = RSQRTSS(a)
                x1 = 0.5 * x0 * ( 3 - ( a * x0 ) * x0 )
                where x0 is the first approximation to the reciprocal square root of a, and x1 is a
                better approximation. The order of evaluation is important. You must use this formula
                before multiplying with a to get the square root."
            @param f
                4 floating point values
            @return
                1 / sqrt( x ) (packed as 4 floats)
        */
        static inline ArrayReal InvSqrt4( ArrayReal f )
        {
            //TODO NEON has vrsqrtsq_f32 to do this job
            ArrayReal invSqrt   = vrsqrteq_f32( f );

            ArrayReal halfInvSqrt= vmulq_f32( HALF, invSqrt );                      //0.5 * rsqrt( f )
            ArrayReal rightSide  = vmulq_f32( invSqrt, vmulq_f32( f, invSqrt ) );   //f * rsqrt( f ) * rsqrt( f )
            rightSide = vreinterpretq_f32_u32(
                            vandq_u32( vreinterpretq_u32_f32( rightSide ),
                                        vcneqq_f32( f, vdupq_n_f32(0.0f) ) ));//Nuke this NaN
            return vmulq_f32( halfInvSqrt, vsubq_f32( THREE, rightSide ) );     //halfInvSqrt*(3 - rightSide)
        }

        /** Returns the squared root of the reciprocal of x
            @remarks
                Use SSE Newton-Raphson reciprocal estimate, accurate to 23 significant
                bits of the mantissa after an extra iteration, instead of the little
                12 bits of accuracy that vrecpsq_f32 gives us
                In short, one Newton-Raphson Iteration:
                     f( i+1 ) = 0.5 * rsqrt( f ) * ( 3 - f * rsqrt( f ) * rsqrt( f ) )
                See Intel AP-803 (Application note), Order N¡ 243637-002 (if you can get it!)
                "x0 = RSQRTSS(a)
                x1 = 0.5 * x0 * ( 3 - ( a * x0 ) * x0 )
                where x0 is the first approximation to the reciprocal square root of a, and x1 is a
                better approximation. The order of evaluation is important. You must use this formula
                before multiplying with a to get the square root."

                Warning: Passing a zero will return a NaN instead of infinity
            @param f
                4 floating point values
            @return
                1 / sqrt( x ) (packed as 4 floats)
        */
        static inline ArrayReal InvSqrtNonZero4( ArrayReal f )
        {
            //TODO NEON has vrsqrtsq_f32 to do this job
            ArrayReal invSqrt = vrsqrteq_f32( f );

            ArrayReal halfInvSqrt= vmulq_f32( HALF, invSqrt );                      //0.5 * rsqrt( f )
            ArrayReal rightSide  = vmulq_f32( invSqrt, vmulq_f32( f, invSqrt ) );   //f * rsqrt( f ) * rsqrt( f )
            return vmulq_f32( halfInvSqrt, vsubq_f32( THREE, rightSide ) );     //halfInvSqrt*(3 - rightSide)
        }

        static inline ArrayReal Sqrt( ArrayReal f )
        {
            //Netwon-Raphson, 2 iterations.
            ArrayReal fStep0 = vrsqrteq_f32( f );
            //Nuke NaN when f == 0
            fStep0 = vreinterpretq_f32_u32( vandq_u32( vtstq_u32( f, f ),
                                                       vreinterpretq_u32_f32( fStep0 ) ) );
            // step fStep0 = 1 / sqrt(x)
            const ArrayReal fStepParm0  = vmulq_f32( f, fStep0 );
            const ArrayReal fStepResult0= vrsqrtsq_f32( fStepParm0, fStep0 );
            // step fStep1 = 1 / sqrt(x)
            const ArrayReal fStep1      = vmulq_f32( fStep0, fStepResult0 );
            const ArrayReal fStepParm1  = vmulq_f32( f, fStep1 );
            const ArrayReal fStepResult1= vrsqrtsq_f32( fStepParm1, fStep1 );
            // take the res. fStep2 = 1 / sqrt(x)
            const ArrayReal fStep2      = vmulq_f32( fStep1, fStepResult1 );
            // mul by x to get sqrt, not rsqrt
            return vmulq_f32( f, fStep2 );
        }

        /** Break x into fractional and integral parts
            @param x
                4 floating point values. i.e. "2.57" (x4)
            @param outIntegral
                The integral part of x. i.e. 2
            @return
                The fractional part of x. i.e. 0.57
        */
        static inline ArrayReal Modf4( ArrayReal x, ArrayReal &outIntegral );

        /** Returns the arccos of x
            @param x
                4 floating point values
            @return
                arccos( x ) (packed as 4 floats)
        */
        static inline ArrayReal ACos4( ArrayReal x );

        /** Returns the sine of x
            @param x
                4 floating point values
            @return
                sin( x ) (packed as 4 floats)
        */
        static ArrayReal Sin4( ArrayReal x );

        /** Returns the cosine of x
            @param x
                4 floating point values
            @return
                cos( x ) (packed as 4 floats)
        */
        static ArrayReal Cos4( ArrayReal x );

        /** Calculates the cosine & sine of x. Use this function if you need to calculate
            both, as it is faster than calling Cos4 & Sin4 together.
            @param x
                4 floating point values
            @param outSin
                Output value, sin( x ) (packed as 4 floats)
            @param outCos
                Output value, cos( x ) (packed as 4 floats)
        */
        static void SinCos4( ArrayReal x, ArrayReal &outSin, ArrayReal &outCos );
    };

#if OGRE_COMPILER != OGRE_COMPILER_CLANG && OGRE_COMPILER != OGRE_COMPILER_GNUC
//  inline ArrayReal operator - ( ArrayReal l )                 { return _mm_xor_ps( l, MathlibNEON::SIGN_MASK ); }
//  inline ArrayReal operator + ( ArrayReal l, Real r )         { return vaddq_f32( l, vdupq_n_f32( r ) ); }
//  inline ArrayReal operator + ( Real l, ArrayReal r )         { return vaddq_f32( vdupq_n_f32( l ), r ); }
//  inline ArrayReal operator + ( ArrayReal l, ArrayReal r )    { return vaddq_f32( l, r ); }
//  inline ArrayReal operator - ( ArrayReal l, Real r )         { return vsubq_f32( l, vdupq_n_f32( r ) ); }
//  inline ArrayReal operator - ( Real l, ArrayReal r )         { return vsubq_f32( vdupq_n_f32( l ), r ); }
    inline ArrayReal operator - ( ArrayReal l, ArrayReal r )    { return vsubq_f32( l, r ); }
//  inline ArrayReal operator * ( ArrayReal l, Real r )         { return vmulq_f32( l, vdupq_n_f32( r ) ); }
//  inline ArrayReal operator * ( Real l, ArrayReal r )         { return vmulq_f32( vdupq_n_f32( l ), r ); }
    inline ArrayReal operator * ( ArrayReal l, ArrayReal r )    { return vmulq_f32( l, r ); }
//  inline ArrayReal operator / ( ArrayReal l, Real r )         { return _mm_div_ps( l, vdupq_n_f32( r ) ); }
//  inline ArrayReal operator / ( Real l, ArrayReal r )         { return _mm_div_ps( vdupq_n_f32( l ), r ); }
//  inline ArrayReal operator / ( ArrayReal l, ArrayReal r )    { return _mm_div_ps( l, r ); }
#endif
}

#include "OgreMathlibNEON.inl"

#endif
#endif
