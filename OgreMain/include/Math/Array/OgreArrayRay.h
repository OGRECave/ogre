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
#ifndef _Ogre_ArrayRay_H_
#define _Ogre_ArrayRay_H_

#include "OgreArrayVector3.h"
#include "OgreArrayAabb.h"

namespace Ogre
{
    class ArrayRay
    {
    public:
        ArrayVector3    mOrigin;
        ArrayVector3    mDirection;

        ArrayRay() : mOrigin( ArrayVector3::ZERO ), mDirection( ArrayVector3::UNIT_Z ) {}
        ArrayRay( const ArrayVector3 &origin, const ArrayVector3 &direction) :
            mOrigin( origin ), mDirection( direction ) {}

        /// SLAB method
        /// See https://tavianator.com/fast-branchless-raybounding-box-intersections-part-2-nans/
        ArrayMaskR intersects( const ArrayAabb &aabb ) const
        {
            ArrayVector3 invDir = Mathlib::SetAll( 1.0f ) / mDirection;
            ArrayVector3 intersectAtMinPlane = (aabb.getMinimum() - mOrigin) * invDir;
            ArrayVector3 intersectAtMaxPlane = (aabb.getMaximum() - mOrigin) * invDir;

            ArrayVector3 minIntersect = intersectAtMinPlane;
            minIntersect.makeFloor( intersectAtMaxPlane );
            ArrayVector3 maxIntersect = intersectAtMinPlane;
            maxIntersect.makeCeil( intersectAtMaxPlane );

            ArrayReal tmin, tmax;
            tmin = minIntersect.mChunkBase[0];
            tmax = maxIntersect.mChunkBase[0];

#if OGRE_CPU == OGRE_CPU_ARM && OGRE_USE_SIMD == 1
            //ARM's NEON behaves the way we want, by propagating NaNs. No need to do anything weird
            tmin = Mathlib::Max( Mathlib::Max( minIntersect.mChunkBase[0],
                                               minIntersect.mChunkBase[1] ),
                                               minIntersect.mChunkBase[2] );
            tmax = Mathlib::Min( Mathlib::Min( maxIntersect.mChunkBase[0],
                                               maxIntersect.mChunkBase[1] ),
                                               maxIntersect.mChunkBase[2] );
#else
            //We need to do weird min/max so that NaNs get propagated
            //(happens if mOrigin is in the AABB's border)
            tmin = Mathlib::Max( tmin, Mathlib::Min( minIntersect.mChunkBase[1], tmax ) );
            tmax = Mathlib::Min( tmax, Mathlib::Max( maxIntersect.mChunkBase[1], tmin ) );

            tmin = Mathlib::Max( tmin, Mathlib::Min( minIntersect.mChunkBase[2], tmax ) );
            tmax = Mathlib::Min( tmax, Mathlib::Max( maxIntersect.mChunkBase[2], tmin ) );
#endif
            //tmax >= max( tmin, 0 )
            return Mathlib::CompareGreaterEqual( tmax, Mathlib::Max( tmin, ARRAY_REAL_ZERO ) );
        }
    };
}

#endif
