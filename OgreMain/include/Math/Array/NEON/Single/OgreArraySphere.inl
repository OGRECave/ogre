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
    inline ArrayMaskR ArraySphere::intersects( const ArraySphere &s ) const
    {
        ArrayReal sqRadius  = vaddq_f32( mRadius, s.mRadius );
        sqRadius            = vmulq_f32( sqRadius, sqRadius );
        ArrayReal sqDist    = mCenter.squaredDistance( s.mCenter );

        return vcleq_f32( sqDist, sqRadius ); // sqDist <= sqRadius
    }
    //-----------------------------------------------------------------------------------
    inline ArrayMaskR ArraySphere::intersects( const ArrayAabb &aabb ) const
    {
        //Get closest point from the AABB to the sphere's center by clamping
        ArrayVector3 closestPoint = mCenter;
        closestPoint.makeFloor( aabb.getMaximum() );
        closestPoint.makeCeil( aabb.getMinimum() );

        //Test the closets point vs sphere
        return intersects( closestPoint );
    }
    //-----------------------------------------------------------------------------------
    inline ArrayMaskR ArraySphere::intersects( const ArrayVector3 &v ) const
    {
        ArrayReal sqRadius  = vmulq_f32( mRadius, mRadius );
        ArrayReal sqDist    = mCenter.squaredDistance( v );

        return vcleq_f32( sqDist, sqRadius ); // sqDist <= sqRadius
    }
}
