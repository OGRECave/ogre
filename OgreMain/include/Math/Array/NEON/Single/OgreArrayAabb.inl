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
    //-----------------------------------------------------------------------------------
    inline ArrayVector3 ArrayAabb::getMinimum() const
    {
        return mCenter - mHalfSize;
    }
    //-----------------------------------------------------------------------------------
    inline ArrayVector3 ArrayAabb::getMaximum() const
    {
        return mCenter + mHalfSize;
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayAabb::merge( const ArrayAabb& rhs )
    {
        ArrayVector3 max( mCenter + mHalfSize );
        max.makeCeil( rhs.mCenter + rhs.mHalfSize );

        ArrayVector3 min( mCenter - mHalfSize );
        min.makeFloor( rhs.mCenter - rhs.mHalfSize );

        mCenter = ( max + min ) * Mathlib::HALF;
        mHalfSize   = ( max - min ) * Mathlib::HALF;
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayAabb::merge( const ArrayVector3& points )
    {
        ArrayVector3 max( mCenter + mHalfSize );
        max.makeCeil( points );

        ArrayVector3 min( mCenter - mHalfSize );
        min.makeFloor( points );

        mCenter = ( max + min ) * Mathlib::HALF;
        mHalfSize   = ( max - min ) * Mathlib::HALF;
    }
    //-----------------------------------------------------------------------------------
    inline ArrayMaskR ArrayAabb::intersects( const ArrayAabb& b2 ) const
    {
        ArrayVector3 dist( mCenter - b2.mCenter );
        ArrayVector3 sumHalfSizes( mHalfSize + b2.mHalfSize );

        // ( abs( center.x - center2.x ) <= halfSize.x + halfSize2.x &&
        //   abs( center.y - center2.y ) <= halfSize.y + halfSize2.y &&
        //   abs( center.z - center2.z ) <= halfSize.z + halfSize2.z )
        ArrayMaskR maskX = vcleq_f32( MathlibNEON::Abs4( dist.mChunkBase[0] ),
                                        sumHalfSizes.mChunkBase[0] );
        ArrayMaskR maskY = vcleq_f32( MathlibNEON::Abs4( dist.mChunkBase[1] ),
                                        sumHalfSizes.mChunkBase[1] );
        ArrayMaskR maskZ = vcleq_f32( MathlibNEON::Abs4( dist.mChunkBase[2] ),
                                        sumHalfSizes.mChunkBase[2] );
        
        return vandq_u32( vandq_u32( maskX, maskY ), maskZ );
    }
    //-----------------------------------------------------------------------------------
    inline ArrayReal ArrayAabb::volume(void) const
    {
        ArrayReal w = vaddq_f32( mHalfSize.mChunkBase[0], mHalfSize.mChunkBase[0] ); // x * 2
        ArrayReal h = vaddq_f32( mHalfSize.mChunkBase[1], mHalfSize.mChunkBase[1] ); // y * 2
        ArrayReal d = vaddq_f32( mHalfSize.mChunkBase[2], mHalfSize.mChunkBase[2] ); // z * 2

        return vmulq_f32( vmulq_f32( w, h ), d ); // w * h * d
    }
    //-----------------------------------------------------------------------------------
    inline ArrayMaskR ArrayAabb::contains( const ArrayAabb &other ) const
    {
        ArrayVector3 dist( mCenter - other.mCenter );

        // In theory, "abs( dist.x ) < mHalfSize - other.mHalfSize" should be more pipeline-
        // friendly because the processor can do the subtraction while the abs4() is being performed,
        // however that variation won't handle the case where both boxes are infinite (will produce
        // nan instead and return false, when it should return true)

        // ( abs( dist.x ) + other.mHalfSize.x <= mHalfSize.x &&
        //   abs( dist.y ) + other.mHalfSize.y <= mHalfSize.y &&
        //   abs( dist.z ) + other.mHalfSize.z <= mHalfSize.z )
        ArrayMaskR maskX = vcleq_f32( vaddq_f32( MathlibNEON::Abs4( dist.mChunkBase[0] ),
                                        other.mHalfSize.mChunkBase[0] ), mHalfSize.mChunkBase[0] );
        ArrayMaskR maskY = vcleq_f32( vaddq_f32( MathlibNEON::Abs4( dist.mChunkBase[1] ),
                                        other.mHalfSize.mChunkBase[1] ), mHalfSize.mChunkBase[1] );
        ArrayMaskR maskZ = vcleq_f32( vaddq_f32( MathlibNEON::Abs4( dist.mChunkBase[2] ),
                                        other.mHalfSize.mChunkBase[2] ), mHalfSize.mChunkBase[2] );

        return vandq_u32( vandq_u32( maskX, maskY ), maskZ );
    }
    //-----------------------------------------------------------------------------------
    inline ArrayMaskR ArrayAabb::contains( const ArrayVector3 &v ) const
    {
        ArrayVector3 dist( mCenter - v );

        // ( abs( dist.x ) <= mHalfSize.x &&
        //   abs( dist.y ) <= mHalfSize.y &&
        //   abs( dist.z ) <= mHalfSize.z )
        ArrayMaskR maskX = vcleq_f32( MathlibNEON::Abs4( dist.mChunkBase[0] ),
                                        mHalfSize.mChunkBase[0] );
        ArrayMaskR maskY = vcleq_f32( MathlibNEON::Abs4( dist.mChunkBase[1] ),
                                        mHalfSize.mChunkBase[1] );
        ArrayMaskR maskZ = vcleq_f32( MathlibNEON::Abs4( dist.mChunkBase[2] ),
                                        mHalfSize.mChunkBase[2] );

        return vandq_u32( vandq_u32( maskX, maskY ), maskZ );
    }
    //-----------------------------------------------------------------------------------
    inline ArrayReal ArrayAabb::distance( const ArrayVector3 &v ) const
    {
        ArrayVector3 dist( mCenter - v );

        // x = abs( dist.x ) - halfSize.x
        // y = abs( dist.y ) - halfSize.y
        // z = abs( dist.z ) - halfSize.z
        // return max( min( x, y, z ), 0 ); //Return minimum between xyz, clamp to zero
        dist.mChunkBase[0] = vsubq_f32( MathlibNEON::Abs4( dist.mChunkBase[0] ),
                                                mHalfSize.mChunkBase[0] );
        dist.mChunkBase[1] = vsubq_f32( MathlibNEON::Abs4( dist.mChunkBase[1] ),
                                                mHalfSize.mChunkBase[1] );
        dist.mChunkBase[2] = vsubq_f32( MathlibNEON::Abs4( dist.mChunkBase[2] ),
                                                mHalfSize.mChunkBase[2] );

        return vmaxq_f32( vminq_f32( vminq_f32( dist.mChunkBase[0],
                    dist.mChunkBase[1] ), dist.mChunkBase[2] ), vdupq_n_f32(0.0f) );
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayAabb::transformAffine( const ArrayMatrix4 &m )
    {
        assert( m.isAffine() );

        mCenter = m * mCenter;

        ArrayReal x = vmulq_f32( Mathlib::Abs4( m.mChunkBase[2] ), mHalfSize.mChunkBase[2] );       // abs( m02 ) * z +
        x = _mm_madd_ps( Mathlib::Abs4( m.mChunkBase[1] ), mHalfSize.mChunkBase[1], x );            // abs( m01 ) * y +
        x = _mm_madd_ps( Mathlib::Abs4( m.mChunkBase[0] ), mHalfSize.mChunkBase[0], x );            // abs( m00 ) * x

        ArrayReal y = vmulq_f32( Mathlib::Abs4( m.mChunkBase[6] ), mHalfSize.mChunkBase[2] );       // abs( m12 ) * z +
        y = _mm_madd_ps( Mathlib::Abs4( m.mChunkBase[5] ), mHalfSize.mChunkBase[1], y );            // abs( m11 ) * y +
        y = _mm_madd_ps( Mathlib::Abs4( m.mChunkBase[4] ), mHalfSize.mChunkBase[0], y );            // abs( m10 ) * x

        ArrayReal z = vmulq_f32( Mathlib::Abs4( m.mChunkBase[10] ), mHalfSize.mChunkBase[2] );      // abs( m22 ) * z +
        z = _mm_madd_ps( Mathlib::Abs4( m.mChunkBase[9] ), mHalfSize.mChunkBase[1], z );            // abs( m21 ) * y +
        z = _mm_madd_ps( Mathlib::Abs4( m.mChunkBase[8] ), mHalfSize.mChunkBase[0], z );            // abs( m20 ) * x

        //Handle infinity & null boxes becoming NaN; leaving the original value instead.
        x = MathlibNEON::CmovRobust( mHalfSize.mChunkBase[0], x,
                                     vceqq_f32( Mathlib::Abs4(mHalfSize.mChunkBase[0]),
                                                MathlibNEON::INFINITEA ) );
        y = MathlibNEON::CmovRobust( mHalfSize.mChunkBase[1], y,
                                     vceqq_f32( Mathlib::Abs4(mHalfSize.mChunkBase[1]),
                                                MathlibNEON::INFINITEA ) );
        z = MathlibNEON::CmovRobust( mHalfSize.mChunkBase[2], z,
                                     vceqq_f32( Mathlib::Abs4(mHalfSize.mChunkBase[2]),
                                                MathlibNEON::INFINITEA ) );

        mHalfSize = ArrayVector3( x, y, z );
    }
    //-----------------------------------------------------------------------------------
}
