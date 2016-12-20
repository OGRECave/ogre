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
#ifndef __SSE2_ArrayAabb_H__
#define __SSE2_ArrayAabb_H__

#ifndef __ArrayAabb_H__
    #error "Don't include this file directly. include Math/Array/OgreArrayAabb.h"
#endif

#include "Math/Array/OgreMathlib.h"
#include "Math/Array/OgreArrayVector3.h"
#include "Math/Array/OgreArrayMatrix4.h"
#include "Math/Simple/OgreAabb.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Math
    *  @{
    */
    /** Cache-friendly array of Aabb represented as a SoA array.
        @remarks
            ArrayAabb is a SIMD & cache-friendly version of AxisAlignedBox.
            (AABB = Axis aligned bounding box) @See ArrayVector3 for
            more information.
        @par
            For performance reasons given the mathematical properties,
            this version stores the box in the form "center + halfSize"
            instead of the form "minimum, maximum" that is present in
            AxisAlignedBox:
                * Merging is slightly more expensive
                * intersects() is much cheaper
                * Naturally deals with infinite boxes (no need for branches)
                * Transform is cheaper (a common operation)
        @par
            Extracting one aabb needs 84 bytes, while all 4 aabbs
            need 96 bytes, both cases are always two cache lines.
            Architectures where the cache line == 32 bytes may want to
            set ARRAY_PACKED_REALS = 2 depending on their needs
    */

    class _OgreExport ArrayAabb
    {
    public:
        ArrayVector3    mCenter;
        ArrayVector3    mHalfSize;

        ArrayAabb() {}

        ArrayAabb( const ArrayVector3 &center, const ArrayVector3 &halfSize ) :
                mCenter( center ), mHalfSize( halfSize )
        {
        }

        void getAsAabb( Aabb &out, size_t index ) const
        {
            //Be careful of not writing to these regions or else strict aliasing rule gets broken!!!
            const Real *aliasedReal = reinterpret_cast<const Real*>( &mCenter );
            out.mCenter.x = aliasedReal[ARRAY_PACKED_REALS * 0 + index];        //X
            out.mCenter.y = aliasedReal[ARRAY_PACKED_REALS * 1 + index];        //Y
            out.mCenter.z = aliasedReal[ARRAY_PACKED_REALS * 2 + index];        //Z
            out.mHalfSize.x = aliasedReal[ARRAY_PACKED_REALS * 3 + index];      //X
            out.mHalfSize.y = aliasedReal[ARRAY_PACKED_REALS * 4 + index];      //Y
            out.mHalfSize.z = aliasedReal[ARRAY_PACKED_REALS * 5 + index];      //Z
        }

        /// Prefer using @see getAsAabb() because this function may have more
        /// overhead (the other one is faster)
        Aabb getAsAabb( size_t index ) const
        {
            Aabb retVal;
            getAsAabb( retVal, index );
            return retVal;
        }

        void setFromAabb( const Aabb &aabb, size_t index )
        {
            Real *aliasedReal = reinterpret_cast<Real*>( &mCenter );
            aliasedReal[ARRAY_PACKED_REALS * 0 + index] = aabb.mCenter.x;       //X
            aliasedReal[ARRAY_PACKED_REALS * 1 + index] = aabb.mCenter.y;       //Y
            aliasedReal[ARRAY_PACKED_REALS * 2 + index] = aabb.mCenter.z;       //Z
            aliasedReal[ARRAY_PACKED_REALS * 3 + index] = aabb.mHalfSize.x;     //X
            aliasedReal[ARRAY_PACKED_REALS * 4 + index] = aabb.mHalfSize.y;     //Y
            aliasedReal[ARRAY_PACKED_REALS * 5 + index] = aabb.mHalfSize.z;     //Z
        }

        void setAll( const Aabb &aabb )
        {
            mCenter.setAll( aabb.mCenter );
            mHalfSize.setAll( aabb.mHalfSize );
        }

        /// Gets the minimum corner of the box.
        inline ArrayVector3 getMinimum() const;

        /// Gets the maximum corner of the box.
        inline ArrayVector3 getMaximum() const;

        /** Merges the passed in box into the current box. The result is the
            box which encompasses both.
        */
        inline void merge( const ArrayAabb& rhs );

        /// Extends the box to encompass the specified point (if needed).
        inline void merge( const ArrayVector3& points );

        /** Transforms the box according to the matrix supplied.
        @remarks
        By calling this method you get the axis-aligned box which
        surrounds the transformed version of this box. Therefore each
        corner of the box is transformed by the matrix, then the
        extents are mapped back onto the axes to produce another
        AABB. Useful when you have a local AABB for an object which
        is then transformed.
        @note
        The matrix must be an affine matrix. @see Matrix4::isAffine.
        */
        inline void transformAffine( const ArrayMatrix4 &matrix );

        /// Returns whether or not this box intersects another.
        inline ArrayMaskR intersects( const ArrayAabb& b2 ) const;

        /// Calculate the area of intersection of this box and another
        inline ArrayAabb intersection( const ArrayAabb& b2 ) const;

        /// Calculate the volume of this box
        inline ArrayReal volume(void) const;

        /// Tests whether another box contained by this box.
        inline ArrayMaskR contains( const ArrayAabb &other ) const;

        /// Tests whether the given point contained by this box.
        inline ArrayMaskR contains( const ArrayVector3 &v ) const;

        /// Returns the minimum distance between a given point and any part of the box.
        inline ArrayReal distance( const ArrayVector3 &v ) const;

        static const ArrayAabb BOX_INFINITE;

        //Contains all zeroes. Used for inactive objects to avoid causing unnecessary NaNs
        static const ArrayAabb BOX_ZERO;
    };
    /** @} */
    /** @} */

}

#include "OgreArrayAabb.inl"

#endif
