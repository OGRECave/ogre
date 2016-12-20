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

#ifndef __MathlibC_H__
#define __MathlibC_H__

#ifndef __Mathlib_H__
    #error "Don't include this file directly. include Math/Array/OgreMathlib.h"
#endif

#include "OgrePrerequisites.h"
#include "OgreCommon.h"
#include "OgreMath.h"

namespace Ogre
{
    class _OgreExport MathlibC
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
        static const ArrayReal INFINITEA;   //Inf, Inf, Inf, Inf
        static const ArrayReal MAX_NEG;     //Max negative number (x4)
        static const ArrayReal MAX_POS;     //Max negative number (x4)

        /** Returns the absolute values of each 4 floats
            @param
                4 floating point values
            @return
                abs( a )
        */
        static inline ArrayReal Abs4( ArrayReal a )
        {
            return Math::Abs( a );
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
            assert( !Math::isNaN( arg1 ) && !Math::isNaN( arg2 ) &&
                    "Passing NaN values to CMov4" );
#ifndef  NDEBUG
            ArrayReal newNan1 = arg1 * 0; //+-Inf * 0 = nan
            ArrayReal newNan2 = arg2 * 0; //+-Inf * 0 = nan
            assert( !Math::isNaN( newNan1 ) && !Math::isNaN( newNan2 ) &&
                    "Passing +/- Infinity values to CMov4" );
#endif

            return mask ? arg1 : arg2;
        }

        /** Robust, branchless conditional move for a 128-bit value.
            @remarks
                If you're looking to copy 4 floating point values that do
                not contain Inf or Nans, @see Cmov4 which is faster.
                This is because switching between registers flagged as
                floating point to integer and back has a latency delay

                For more information refer to Chapter 3.5.2.3
                Bypass between Execution Domains, Intel® 64 and IA-32
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
            return mask ? arg1 : arg2;
        }
        static inline ArrayInt CmovRobust( ArrayInt arg1, ArrayInt arg2, ArrayMaskI mask )
        {
            return mask ? arg1 : arg2;
        }

        /** Returns the result of "a & b"
        @return
            r[i] = a[i] & b[i];
        */
        static inline ArrayInt And( ArrayInt a, ArrayInt b )
        {
            return a & b;
        }

        static inline ArrayMaskI And( ArrayMaskI a, ArrayInt b )
        {
            return ((a ? 0xffffffff : 0) & b) != 0;
        }

        static inline ArrayMaskI And( ArrayInt a, ArrayMaskI b )
        {
            return (a & (b ? 0xffffffff : 0)) != 0;
        }

        static inline ArrayMaskI And( ArrayMaskI a, ArrayMaskI b )
        {
            return a & b;
        }

        /** Test if "a AND b" will result in non-zero, returning 0xffffffff on those cases
        @return
            r[i] = (a[i] & b[i]) ? 0xffffffff : 0;
        */
        static inline ArrayMaskI TestFlags4( ArrayInt a, ArrayInt b )
        {
            return (a & b) != 0;
        }

        static inline ArrayMaskI TestFlags4( ArrayMaskI a, ArrayInt b )
        {
            return ( (a ? 0xffffffff : 0) & b ) != 0;
        }

        static inline ArrayMaskI TestFlags4( ArrayInt a,  ArrayMaskI b )
        {
            return ( a & (b ? 0xffffffff : 0) ) != 0;
        }

        /** Returns the result of "a & ~b"
        @return
            r[i] = a[i] & ~b[i];
        */
        static inline ArrayInt AndNot( ArrayInt a, ArrayInt b )
        {
            return a & ~b;
        }

        static inline ArrayMaskI AndNot( ArrayMaskI a, ArrayInt b )
        {
            return ((a ? 0xffffffff : 0) & ~b) != 0;
        }

        static inline ArrayMaskI AndNot( ArrayInt a, ArrayMaskI b )
        {
            return (a & (b ? 0 : 0xffffffff)) != 0;
        }

        static inline ArrayMaskI AndNot( ArrayMaskI a, ArrayMaskI b )
        {
            return a & (!b);
        }

        /** Returns the result of "a | b"
        @return
            r[i] = a[i] | b[i];
        */
        static inline ArrayInt Or( ArrayInt a, ArrayInt b )
        {
            return a | b;
        }
        static inline ArrayMaskI Or( ArrayMaskI a, ArrayMaskI b )
        {
            return a | b;
        }

        static inline ArrayMaskI Or( ArrayMaskI a, ArrayInt b )
        {
            return ( (a ? 0xffffffff : 0) | b ) != 0;
        }
        static inline ArrayMaskI Or( ArrayInt a,  ArrayMaskI b )
        {
            return ( a | (b ? 0xffffffff : 0) ) != 0;
        }

        /** Returns the result of "a < b"
        @return
            r[i] = a[i] < b[i] ? 0xffffffff : 0;
        */
        static inline ArrayMaskR CompareLess( ArrayReal a, ArrayReal b )
        {
            return a < b;
        }

        /** Returns the result of "a <= b"
        @return
            r[i] = a[i] <= b[i] ? 0xffffffff : 0;
        */
        static inline ArrayMaskR CompareLessEqual( ArrayReal a, ArrayReal b )
        {
            return a <= b;
        }

        /** Returns the result of "a > b"
        @return
            r[i] = a[i] > b[i] ? 0xffffffff : 0;
        */
        static inline ArrayMaskR CompareGreater( ArrayReal a, ArrayReal b )
        {
            return a > b;
        }

        /** Returns the result of "a >= b"
        @return
            r[i] = a[i] >= b[i] ? 0xffffffff : 0;
        */
        static inline ArrayMaskR CompareGreaterEqual( ArrayReal a, ArrayReal b )
        {
            return a >= b;
        }

        static inline ArrayReal SetAll( Real val )
        {
            return val;
        }

        static inline ArrayInt SetAll( uint32 val )
        {
            return val;
        }

        static inline void Set( ArrayReal &dst, Real val, size_t index )
        {
            dst = val;
        }

        /** Returns the result of "a == std::numeric_limits<float>::infinity()"
        @return
            r[i] = a[i] == Inf ? 0xffffffff : 0;
        */
        static inline ArrayMaskR isInfinity( ArrayReal a )
        {
            return a == std::numeric_limits<float>::infinity();
        }

        /// Returns the maximum value between a and b
        static inline ArrayReal Max( ArrayReal a, ArrayReal b )
        {
            return Ogre::max( a, b );
        }

        /// Returns the minimum value between a and b
        static inline ArrayReal Min( ArrayReal a, ArrayReal b )
        {
            return Ogre::min( a, b );
        }
        
        /** Returns the minimum value of all elements in a
        @return
            r[0] = min( a[0], a[1], a[2], a[3] )
        */
        static inline Real ColapseMin( ArrayReal a )
        {
            return a;
        }

        /** Returns the maximum value of all elements in a
        @return
            r[0] = max( a[0], a[1], a[2], a[3] )
        */
        static inline Real ColapseMax( ArrayReal a )
        {
            return a;
        }

        /** Returns the reciprocal of x
            @remarks
                If you have a very rough guarantees that you won't be feeding a zero,
                consider using @see InvNonZero4 because it's faster.
                See MathlibSSE2 implementation
            @return
                1 / x (packed as 4 floats)
        */
        static inline ArrayReal Inv4( ArrayReal val )
        {
            return 1.0f / val;
        }

        /** Returns the reciprocal of x
            @remarks
                If the input is zero, it will produce a NaN!!! (but it's faster)
                Note: Some architectures may slowdown when a NaN is produced, making this
                function slower than Inv4 for those cases
                @see Inv4
            @param val
                If it's zero, the returned value could be NaN depending on implementation
            @return
                1 / x
        */
        static inline ArrayReal InvNonZero4( ArrayReal val )
        {
            return 1.0f / val;
        }

        /** Returns the squared root of the reciprocal of x
            @return
                1 / sqrt( x )
        */
        static inline ArrayReal InvSqrt4( ArrayReal f )
        {
            return 1.0f / sqrt( f );
        }

        /** Returns the squared root of the reciprocal of x
            @return
                1 / sqrt( x ) (packed as 4 floats)
        */
        static inline ArrayReal InvSqrtNonZero4( ArrayReal f )
        {
            return 1.0f / sqrt( f );
        }

        /** Break x into fractional and integral parts
            @param
                4 floating point values. i.e. "2.57" (x4)
            @param x
                The integral part of x. i.e. 2
            @return outIntegral
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
        static inline ArrayReal Sin4( ArrayReal x );

        /** Returns the cosine of x
            @param x
                4 floating point values
            @return
                cos( x ) (packed as 4 floats)
        */
        static inline ArrayReal Cos4( ArrayReal x );

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
}

#include "OgreMathlibC.inl"

#endif
