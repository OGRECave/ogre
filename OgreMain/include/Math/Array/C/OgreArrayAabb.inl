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

        mCenter = ( max + min ) * 0.5f;
        mHalfSize   = ( max - min ) * 0.5f;
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayAabb::merge( const ArrayVector3& points )
    {
        ArrayVector3 max( mCenter + mHalfSize );
        max.makeCeil( points );

        ArrayVector3 min( mCenter - mHalfSize );
        min.makeFloor( points );

        mCenter = ( max + min ) * 0.5f;
        mHalfSize   = ( max - min ) * 0.5f;
    }
    //-----------------------------------------------------------------------------------
    inline ArrayMaskR ArrayAabb::intersects( const ArrayAabb& b2 ) const
    {
        ArrayVector3 distance( mCenter - b2.mCenter );
        ArrayVector3 sumHalfSizes( mHalfSize + b2.mHalfSize );

        // ( abs( center.x - center2.x ) <= halfSize.x + halfSize2.x &&
        //   abs( center.y - center2.y ) <= halfSize.y + halfSize2.y &&
        //   abs( center.z - center2.z ) <= halfSize.z + halfSize2.z )
        ArrayMaskR maskX = Math::Abs( distance.mChunkBase[0] ) <= sumHalfSizes.mChunkBase[0];
        ArrayMaskR maskY = Math::Abs( distance.mChunkBase[1] ) <= sumHalfSizes.mChunkBase[1];
        ArrayMaskR maskZ = Math::Abs( distance.mChunkBase[2] ) <= sumHalfSizes.mChunkBase[2];
        
        return maskX & maskY & maskZ;
    }
    //-----------------------------------------------------------------------------------
    inline ArrayReal ArrayAabb::volume(void) const
    {
        ArrayReal w = mHalfSize.mChunkBase[0] + mHalfSize.mChunkBase[0]; // x * 2
        ArrayReal h = mHalfSize.mChunkBase[1] + mHalfSize.mChunkBase[1]; // y * 2
        ArrayReal d = mHalfSize.mChunkBase[2] + mHalfSize.mChunkBase[2]; // z * 2

        return w * h * d;
    }
    //-----------------------------------------------------------------------------------
    inline ArrayMaskR ArrayAabb::contains( const ArrayAabb &other ) const
    {
        ArrayVector3 distance( mCenter - other.mCenter );

        // In theory, "abs( distance.x ) < mHalfSize - other.mHalfSize" should be more pipeline-
        // friendly because the processor can do the subtraction while the abs4() is being performed,
        // however that variation won't handle the case where both boxes are infinite (will produce
        // nan instead and return false, when it should return true)

        // ( abs( distance.x ) + other.mHalfSize.x <= mHalfSize.x &&
        //   abs( distance.y ) + other.mHalfSize.y <= mHalfSize.y &&
        //   abs( distance.z ) + other.mHalfSize.z <= mHalfSize.z )
        ArrayMaskR maskX = (Math::Abs( distance.mChunkBase[0] ) + other.mHalfSize.mChunkBase[0]) <= mHalfSize.mChunkBase[0];
        ArrayMaskR maskY = (Math::Abs( distance.mChunkBase[1] ) + other.mHalfSize.mChunkBase[1]) <= mHalfSize.mChunkBase[1];
        ArrayMaskR maskZ = (Math::Abs( distance.mChunkBase[2] ) + other.mHalfSize.mChunkBase[2]) <= mHalfSize.mChunkBase[2];

        return maskX & maskY & maskZ;
    }
    //-----------------------------------------------------------------------------------
    inline ArrayMaskR ArrayAabb::contains( const ArrayVector3 &v ) const
    {
        ArrayVector3 distance( mCenter - v );

        // ( abs( distance.x ) <= mHalfSize.x &&
        //   abs( distance.y ) <= mHalfSize.y &&
        //   abs( distance.z ) <= mHalfSize.z )
        ArrayMaskR maskX = Math::Abs( distance.mChunkBase[0] ) <= mHalfSize.mChunkBase[0];
        ArrayMaskR maskY = Math::Abs( distance.mChunkBase[1] ) <= mHalfSize.mChunkBase[1];
        ArrayMaskR maskZ = Math::Abs( distance.mChunkBase[2] ) <= mHalfSize.mChunkBase[2];

        return maskX & maskY & maskZ;
    }
    //-----------------------------------------------------------------------------------
    inline ArrayReal ArrayAabb::distance( const ArrayVector3 &v ) const
    {
        ArrayVector3 distance( mCenter - v );

        // x = abs( distance.x ) - halfSize.x
        // y = abs( distance.y ) - halfSize.y
        // z = abs( distance.z ) - halfSize.z
        // return max( min( x, y, z ), 0 ); //Return minimum between xyz, clamp to zero
        distance.mChunkBase[0] = Math::Abs( distance.mChunkBase[0] ) - mHalfSize.mChunkBase[0];
        distance.mChunkBase[1] = Math::Abs( distance.mChunkBase[1] ) - mHalfSize.mChunkBase[1];
        distance.mChunkBase[2] = Math::Abs( distance.mChunkBase[2] ) - mHalfSize.mChunkBase[2];

        return Ogre::max( Ogre::min( Ogre::min(
                distance.mChunkBase[0], distance.mChunkBase[1] ), distance.mChunkBase[2] ), Real(0.0) );
    }
    //-----------------------------------------------------------------------------------
    inline void ArrayAabb::transformAffine( const ArrayMatrix4 &m )
    {
        assert( m.isAffine() );

        mCenter = m * mCenter;

        ArrayReal x = Math::Abs( m.mChunkBase[2] ) * mHalfSize.mChunkBase[2];       // abs( m02 ) * z +
        x = ogre_madd( Math::Abs( m.mChunkBase[1] ), mHalfSize.mChunkBase[1], x );  // abs( m01 ) * y +
        x = ogre_madd( Math::Abs( m.mChunkBase[0] ), mHalfSize.mChunkBase[0], x );  // abs( m00 ) * x

        ArrayReal y = Math::Abs( m.mChunkBase[6] ) * mHalfSize.mChunkBase[2];       // abs( m12 ) * z +
        y = ogre_madd( Math::Abs( m.mChunkBase[5] ), mHalfSize.mChunkBase[1], y );  // abs( m11 ) * y +
        y = ogre_madd( Math::Abs( m.mChunkBase[4] ), mHalfSize.mChunkBase[0], y );  // abs( m10 ) * x

        ArrayReal z = Math::Abs( m.mChunkBase[10] ) * mHalfSize.mChunkBase[2];      // abs( m22 ) * z +
        z = ogre_madd( Math::Abs( m.mChunkBase[9] ), mHalfSize.mChunkBase[1], z );  // abs( m21 ) * y +
        z = ogre_madd( Math::Abs( m.mChunkBase[8] ), mHalfSize.mChunkBase[0], z );  // abs( m20 ) * x

        //Handle infinity boxes not becoming NaN. Null boxes containing -Inf will still have NaNs
        //(which is ok since we need them to say 'false' to intersection tests)
        x = MathlibC::CmovRobust( mHalfSize.mChunkBase[0], x,
                                    Math::Abs(mHalfSize.mChunkBase[0]) == MathlibC::INFINITEA );
        y = MathlibC::CmovRobust( mHalfSize.mChunkBase[1], y,
                                    Math::Abs(mHalfSize.mChunkBase[1]) == MathlibC::INFINITEA );
        z = MathlibC::CmovRobust( mHalfSize.mChunkBase[2], z,
                                    Math::Abs(mHalfSize.mChunkBase[2]) == MathlibC::INFINITEA );

        mHalfSize = ArrayVector3( x, y, z );
    }
    //-----------------------------------------------------------------------------------
}
