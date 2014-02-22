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

Where EXPLICITLY stated, the following license will apply (it's from the
Sony's SIMD Math library):

Copyright (C) 2006, 2007 Sony Computer Entertainment Inc.
All rights reserved.

Redistribution and use in source and binary forms,
with or without modification, are permitted provided that the
following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Sony Computer Entertainment Inc nor the names
      of its contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

namespace Ogre
{
    /*inline ArrayReal ArrayRadian::valueDegrees() const
    {
        return _mm_mul_ps( mRad, MathlibSSE2::fRad2Deg );
    }*/
    //-----------------------------------------------------------------------------------
    inline const ArrayRadian& ArrayRadian::operator + () const
    {
        return *this;
    }
    //-----------------------------------------------------------------------------------
    inline ArrayRadian ArrayRadian::operator + ( const ArrayRadian& r ) const
    {
        return ArrayRadian( _mm_add_ps( mRad, r.mRad ) );
    }
    //-----------------------------------------------------------------------------------
    /*inline ArrayRadian ArrayRadian::operator + ( const ArrayDegree& d ) const
    {
        return ArrayRadian( _mm_add_ps( mRad, r.valueRadians() ) );
    }*/
    //-----------------------------------------------------------------------------------
    inline ArrayRadian& ArrayRadian::operator += ( const ArrayRadian& r )
    {
        mRad = _mm_add_ps( mRad, r.mRad );
        return *this;
    }
    //-----------------------------------------------------------------------------------
    /*inline ArrayRadian& ArrayRadian::operator += ( const ArrayDegree& r )
    {
        mRad = _mm_add_ps( mRad, r.valueRadians() );
        return *this;
    }*/
    //-----------------------------------------------------------------------------------
    inline ArrayRadian ArrayRadian::operator - () const
    {
        return ArrayRadian( _mm_xor_ps( mRad, MathlibSSE2::SIGN_MASK ) );
    }
    //-----------------------------------------------------------------------------------
    inline ArrayRadian ArrayRadian::operator - ( const ArrayRadian& r ) const
    {
        return ArrayRadian( _mm_sub_ps( mRad, r.mRad ) );
    }
    //-----------------------------------------------------------------------------------
    /*inline ArrayRadian ArrayRadian::operator - ( const ArrayDegree& d ) const
    {
        return ArrayRadian( _mm_sub_ps( mRad, r.valueRadians() ) );
    }*/
    //-----------------------------------------------------------------------------------
    inline ArrayRadian& ArrayRadian::operator -= ( const ArrayRadian& r )
    {
        mRad = _mm_sub_ps( mRad, r.mRad );
        return *this;
    }
    //-----------------------------------------------------------------------------------
    /*inline ArrayRadian& ArrayRadian::operator -= ( const ArrayDegree& r )
    {
        mRad = _mm_sub_ps( mRad, r.valueRadians() );
        return *this;
    }*/
    //-----------------------------------------------------------------------------------
    inline ArrayRadian ArrayRadian::operator * ( const ArrayRadian& r ) const
    {
        return ArrayRadian( _mm_mul_ps( mRad, r.mRad ) );
    }
    //-----------------------------------------------------------------------------------
    inline ArrayRadian ArrayRadian::operator / ( ArrayReal r ) const
    {
        return ArrayRadian( _mm_div_ps( mRad, r ) );
    }
    //-----------------------------------------------------------------------------------
    inline ArrayRadian& ArrayRadian::operator /= ( ArrayReal r )
    {
        mRad = _mm_div_ps( mRad, r );
        return *this;
    }

    inline ArrayReal ArrayRadian::operator <  ( const ArrayRadian& r ) const { return _mm_cmplt_ps( mRad, r.mRad ); }
    inline ArrayReal ArrayRadian::operator <= ( const ArrayRadian& r ) const { return _mm_cmple_ps( mRad, r.mRad ); }
    inline ArrayReal ArrayRadian::operator == ( const ArrayRadian& r ) const { return _mm_cmpeq_ps( mRad, r.mRad ); }
    inline ArrayReal ArrayRadian::operator != ( const ArrayRadian& r ) const { return _mm_cmpneq_ps( mRad, r.mRad );}
    inline ArrayReal ArrayRadian::operator >= ( const ArrayRadian& r ) const { return _mm_cmpge_ps( mRad, r.mRad ); }
    inline ArrayReal ArrayRadian::operator >  ( const ArrayRadian& r ) const { return _mm_cmpgt_ps( mRad, r.mRad ); }

    //-----------------------------------------------------------------------------------
    inline ArrayReal MathlibSSE2::Modf4( ArrayReal x, ArrayReal &outIntegral )
    {
        outIntegral = _mm_cvtepi32_ps( _mm_cvttps_epi32( x ) ); //outIntegral = floor( x )
        return _mm_sub_ps( x, outIntegral );
    }
    //-----------------------------------------------------------------------------------
    inline ArrayReal MathlibSSE2::ACos4( ArrayReal x)
    {
        // This function, ACos4, is under Copyright (C) 2006, 2007
        // Sony Computer Entertainment Inc. (BSD style license) See
        // header for details. Adapted/ported to Ogre intrinsics.
        __m128 xabs = Abs4( x );
        __m128 select = _mm_cmplt_ps( x, _mm_setzero_ps() );
        __m128 t1 = _mm_sqrt_ps( _mm_sub_ps( ONE, xabs ) );
        
        /* Instruction counts can be reduced if the polynomial was
         * computed entirely from nested (dependent) fma's. However, 
         * to reduce the number of pipeline stalls, the polygon is evaluated 
         * in two halves (hi amd lo). 
         */
        __m128 xabs2 = _mm_mul_ps( xabs,  xabs );
        __m128 xabs4 = _mm_mul_ps( xabs2, xabs2 );
        __m128 hi = _mm_madd_ps(_mm_madd_ps(_mm_madd_ps(_mm_set1_ps(-0.0012624911f),
            xabs, _mm_set1_ps(0.0066700901f)),
                xabs, _mm_set1_ps(-0.0170881256f)),
                    xabs, _mm_set1_ps( 0.0308918810f));
        __m128 lo = _mm_madd_ps(_mm_madd_ps(_mm_madd_ps(_mm_set1_ps(-0.0501743046f),
            xabs, _mm_set1_ps(0.0889789874f)),
                xabs, _mm_set1_ps(-0.2145988016f)),
                    xabs, _mm_set1_ps( 1.5707963050f));
        
        __m128 result = _mm_madd_ps( hi, xabs4, lo );
        
        // Adjust the result if x is negactive.
        return Cmov4(   _mm_nmsub_ps( t1, result, PI ), // Negative
                        _mm_mul_ps( t1, result ),       // Positive
                        select );
    }

}
