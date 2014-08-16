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
#ifndef __C_Aabb_H__
#define __C_Aabb_H__

#ifndef __Aabb_H__
    #error "Don't include this file directly. include Math/Simple/OgreAabb.h"
#endif

#include "OgreVector3.h"
#include "OgreMatrix4.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Math
    *  @{
    */
    /** AoS (array of structures) version of ArrayAabb. This class also deprecates
        AxisAlignedBox. It's A 3D box aligned with the x/y/z axes.
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
        This class represents a simple box which is aligned with the
        axes. Internally it only stores 2 points as the center of
        the box, and the half of the width, height, and depth. This class
        is typically used for an axis-aligned bounding box (AABB) for
        collision and visibility determination.
        @par
        Main differences with AxisAlignedBox:
            * Aabb doesn't support null boxes as AxisAlignedBox did. Although
              if the center iszero and half size is set to negative infinity,
              it may mimic the behavior. Another possibility is to just use NaNs,
              because they would cause false on all tests. However this would
              be horrible slow.
            * BOX_INFINITE represents a truly infinite box and behaves exactly
              the same as an infinite AxisAlignedBox. However, setting the latter
              to infinite still holds information in min & max variables, while
              aabb destroys all information present.
        @note
        This is a POD data structure.
    */

    struct _OgreExport Aabb
    {
        Ogre::Vector3       mCenter;
        Ogre::Vector3       mHalfSize;

        Aabb() :
            mCenter( Vector3::ZERO ),
            mHalfSize( Vector3::ZERO )
        {
        }
        /*Aabb( const AxisAlignedBox &aab ) :
                    mCenter( aab.getCenter() ),
                    m_index( aab.getHalfSize() )
        {
        }*/
        Aabb( const Vector3 &center, const Vector3 &halfSize ) :
                    mCenter( center ),
                    mHalfSize( halfSize )
        {
        }

        /// Sets both minimum and maximum extents at once.
        inline void setExtents( const Vector3& min, const Vector3& max );

        /// Sets both minimum and maximum extents at once (static version).
        inline static Aabb newFromExtents( const Vector3& min, const Vector3& max );

        /// Gets the minimum corner of the box.
        inline Vector3 getMinimum() const;

        /// Gets the maximum corner of the box.
        inline Vector3 getMaximum() const;

        /// Gets the size of the box
        inline Vector3 getSize(void) const;

        /** Merges the passed in box into the current box. The result is the
            box which encompasses both.
        */
        inline void merge( const Aabb& rhs );

        /// Extends the box to encompass the specified point (if needed).
        inline void merge( const Vector3& points );

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
        inline void transformAffine( const Matrix4 &matrix );

        /// Returns whether or not this box intersects another.
        inline bool intersects( const Aabb& b2 ) const;

        /// Calculate the area of intersection of this box and another
        inline Aabb intersection( const Aabb& b2 ) const;

        /// Calculate the volume of this box
        inline Real volume(void) const;

        /// Tests whether another box contained by this box.
        inline bool contains( const Aabb &other ) const;

        /// Tests whether the given point contained by this box.
        inline bool contains( const Vector3 &v ) const;

        /// Returns the minimum distance between a given point and any part of the box.
        inline Real distance( const Vector3 &v ) const;

        /// Returns the radius of a sphere enclosing the aabb from the outside at center mCenter
        inline Real getRadius() const;

        /// Returns the radius of a sphere enclosing the aabb from origin as center
        inline Real getRadiusOrigin() const;

        inline bool operator == ( const Aabb &_r ) const;
        inline bool operator != ( const Aabb &_r ) const;

        static const Aabb BOX_INFINITE;
        static const Aabb BOX_NULL;

        //Contains all zeroes. Used for inactive objects to avoid causing unnecessary NaNs
        static const Aabb BOX_ZERO;
    };
    /** @} */
    /** @} */

}

#include "OgreAabb.inl"

#endif
