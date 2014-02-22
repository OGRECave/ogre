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

#include <OgreStableHeaders.h>

#include "OgrePlatformInformation.h"

#if __OGRE_HAVE_SSE
#define __Mathlib_H__ //Neded to directly include OgreMathlibSSE2

#include "Math/Array/OgreArrayConfig.h"
#include "Math/Array/SSE2/Single/OgreMathlibSSE2.h"
#include "Math/Array/SSE2/Single/sse_mathfun.h"
#include "Math/Array/OgreBooleanMask.h"

namespace Ogre
{
    const ArrayReal MathlibSSE2::HALF       = _mm_set_ps1( 0.5f );
    const ArrayReal MathlibSSE2::ONE        = _mm_set_ps1( 1.0f );
    const ArrayReal MathlibSSE2::THREE      = _mm_set_ps1( 3.0f );
    const ArrayReal MathlibSSE2::NEG_ONE    = _mm_set_ps1( -1.0f );
    const ArrayReal MathlibSSE2::fEpsilon   = _mm_set_ps1( 1e-6f );
    const ArrayReal MathlibSSE2::fSqEpsilon = _mm_set_ps1( 1e-12f );
    const ArrayReal MathlibSSE2::OneMinusEpsilon= _mm_set_ps1( 1.0f - 1e-6f );
    const ArrayReal MathlibSSE2::FLOAT_MIN  = _mm_set_ps1( std::numeric_limits<Real>::min() );
    const ArrayReal MathlibSSE2::SIGN_MASK  = _mm_set_ps1( -0.0f );
    const ArrayReal MathlibSSE2::INFINITEA  = _mm_set_ps1( std::numeric_limits<Real>::infinity() );
    const ArrayReal MathlibSSE2::MAX_NEG    = _mm_set_ps1( -std::numeric_limits<Real>::max() );
    const ArrayReal MathlibSSE2::MAX_POS    = _mm_set_ps1( std::numeric_limits<Real>::max() );
    const ArrayReal MathlibSSE2::LAST_AFFINE_COLUMN = _mm_setr_ps( 0, 0, 0, 1 );

    static const Real _PI = Real( 4.0 * atan( 1.0 ) );
    //We can't use Math::fDeg2Rad & Math::fRad2Deg directly because
    //it's not guaranteed to have been initialized first
    const ArrayReal MathlibSSE2::PI         = _mm_set_ps1( _PI );
    const ArrayReal MathlibSSE2::TWO_PI     = _mm_set_ps1( 2.0f * _PI );
    const ArrayReal MathlibSSE2::fDeg2Rad   = _mm_set_ps1( _PI / 180.0f );
    const ArrayReal MathlibSSE2::fRad2Deg   = _mm_set_ps1( 180.0f / _PI );

    const ArrayReal MathlibSSE2::ONE_DIV_2PI= _mm_set_ps1( 1.0f / (2.0f * _PI) );

    //-----------------------------------------------------------------------------------
    ArrayReal MathlibSSE2::Sin4( ArrayReal x )
    {
        // Map arbitrary angle x to the range [-pi; +pi] without using division.
        // Code taken from MSDN's HLSL trick. Architectures with fused mad (i.e. NEON)
        // can replace the add, the sub, & the two muls for two mad
        ArrayReal integralPart;
        x = _mm_add_ps( _mm_mul_ps( x, ONE_DIV_2PI ), HALF );
        x = Modf4( x, integralPart );
        x = _mm_sub_ps( _mm_mul_ps( x, TWO_PI ), PI );

        return sin_ps( x );
    }
    //-----------------------------------------------------------------------------------
    ArrayReal MathlibSSE2::Cos4( ArrayReal x )
    {
        // Map arbitrary angle x to the range [-pi; +pi] without using division.
        // Code taken from MSDN's HLSL trick. Architectures with fused mad (i.e. NEON)
        // can replace the add, the sub, & the two muls for two mad
        ArrayReal integralPart;
        x = _mm_add_ps( _mm_mul_ps( x, ONE_DIV_2PI ), HALF );
        x = Modf4( x, integralPart );
        x = _mm_sub_ps( _mm_mul_ps( x, TWO_PI ), PI );

        return cos_ps( x );
    }
    //-----------------------------------------------------------------------------------
    void MathlibSSE2::SinCos4( ArrayReal x, ArrayReal &outSin, ArrayReal &outCos )
    {
        // TODO: Improve accuracy by mapping to the range [-pi/4, pi/4] and swap
        // between cos & sin depending on which quadrant it fell:
        // Quadrant | sin     |  cos
        // n = 0 ->  sin( x ),  cos( x )
        // n = 1 ->  cos( x ), -sin( x )
        // n = 2 -> -sin( x ), -cos( x )
        // n = 3 -> -sin( x ),  sin( x )
        // See ARGUMENT REDUCTION FOR HUGE ARGUMENTS:
        // Good to the Last Bit
        // K. C. Ng and themembers of the FP group of SunPro
        // http://www.derekroconnor.net/Software/Ng--ArgReduction.pdf

        // -- Perhaps we can leave this to GSoC students? --

        // Map arbitrary angle x to the range [-pi; +pi] without using division.
        // Code taken from MSDN's HLSL trick. Architectures with fused mad (i.e. NEON)
        // can replace the add, the sub, & the two muls for two mad
        ArrayReal integralPart;
        x = _mm_add_ps( _mm_mul_ps( x, ONE_DIV_2PI ), HALF );
        x = Modf4( x, integralPart );
        x = _mm_sub_ps( _mm_mul_ps( x, TWO_PI ), PI );

        sincos_ps( x, &outSin, &outCos );
    }

    const ArrayReal BooleanMask4::mMasks[NUM_MASKS] =
    {
        _mm_castsi128_ps(_mm_set_epi32( 0x00000000, 0x00000000, 0x00000000, 0x00000000 )),//MASK_NONE
        _mm_castsi128_ps(_mm_set_epi32( 0x00000000, 0x00000000, 0x00000000, 0xffffffff )),//MASK_X
        _mm_castsi128_ps(_mm_set_epi32( 0x00000000, 0x00000000, 0xffffffff, 0x00000000 )),//MASK_Y
        _mm_castsi128_ps(_mm_set_epi32( 0x00000000, 0x00000000, 0xffffffff, 0xffffffff )),//MASK_XY
        _mm_castsi128_ps(_mm_set_epi32( 0x00000000, 0xffffffff, 0x00000000, 0x00000000 )),//MASK_Z
        _mm_castsi128_ps(_mm_set_epi32( 0x00000000, 0xffffffff, 0x00000000, 0xffffffff )),//MASK_XZ
        _mm_castsi128_ps(_mm_set_epi32( 0x00000000, 0xffffffff, 0xffffffff, 0x00000000 )),//MASK_YZ
        _mm_castsi128_ps(_mm_set_epi32( 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff )),//MASK_XYZ
        _mm_castsi128_ps(_mm_set_epi32( 0xffffffff, 0x00000000, 0x00000000, 0x00000000 )),//MASK_W
        _mm_castsi128_ps(_mm_set_epi32( 0xffffffff, 0x00000000, 0x00000000, 0xffffffff )),//MASK_XW
        _mm_castsi128_ps(_mm_set_epi32( 0xffffffff, 0x00000000, 0xffffffff, 0x00000000 )),//MASK_YW
        _mm_castsi128_ps(_mm_set_epi32( 0xffffffff, 0x00000000, 0xffffffff, 0xffffffff )),//MASK_XYW
        _mm_castsi128_ps(_mm_set_epi32( 0xffffffff, 0xffffffff, 0x00000000, 0x00000000 )),//MASK_ZW
        _mm_castsi128_ps(_mm_set_epi32( 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff )),//MASK_XZW
        _mm_castsi128_ps(_mm_set_epi32( 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000 )),//MASK_YZW
        _mm_castsi128_ps(_mm_set_epi32( 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff )) //MASK_XYZW
    };
}
#endif
