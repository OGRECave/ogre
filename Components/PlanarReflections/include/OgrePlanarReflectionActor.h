/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2017 Torus Knot Software Ltd

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
#ifndef _OgrePlanarReflectionActor_H_
#define _OgrePlanarReflectionActor_H_

#include "OgrePlanarReflectionsPrerequisites.h"
#include "OgreIdString.h"
#include "OgrePlane.h"
#include "OgreVector2.h"
#include "Math/Array/OgreArrayConfig.h"
#include "Math/Array/OgreArrayVector3.h"
#include "Math/Array/OgreArrayQuaternion.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Component
    *  @{
    */
    /** \addtogroup Material
    *  @{
    */

    struct ActiveActorData;

    /** Actors are defined by a plane and a rectangle that limits that plane.
        Planes are normally infinite, but Actors limit them to a rectangle.
        In 3D, this means a plane has 4 additional planes to limit them
        (north, west, south & east)
        To hold those 5 planes, we only need 5 "d" from Plane, and 5 normals
        (one for the main plane, 2 for north & south, 2 for west & east).
        However a Quaternion can represent all 5 normals: zAxis is normal,
        +/- yAxis is north & south, +/- xAxis is west & east.
    */
    struct ArrayActorPlane
    {
        ArrayQuaternion planeNormals;
        /// Arrangement is:
        /// 0 = main plane
        /// 1 = +Y plane (north)
        /// 2 = -Y plane (south)
        /// 3 = +X plane (east)
        /// 4 = -X plane (west)
        ArrayReal       planeNegD[5];
        ArrayVector3    center;
        ArrayReal       xyHalfSize[2];
    };

    class _OgrePlanarReflectionsExport PlanarReflectionActor
    {
        friend class PlanarReflections;
    protected:
        Plane       mPlane;
        Vector3     mCenter;
        Vector2     mHalfSize;
        Quaternion  mOrientation;

        bool        mHasReservation;
        uint8       mCurrentBoundSlot;
        /** Ogre tries to activate visible planar reflections, sorting those
            that are closest to the camera, as long as they have equal priority.
            However distance to camera may not be convenient in all cases (such as when you
            have four small mirrors close and one huge mirror a bit further away).
            For such cases, you can use priority to allow a particular actor "to win" against
            others. It's also very possible you want to only set priority to 0 for actors
            that have reserved slots.
        @par
            There is no particular overhead/performance cost associated with this call.
        @remarks
            If the actor fails frustum culling, it won't be activated even if priority == 0.
            This shouldn't matter.
        @par
            Value is in range [0; 255]. Default value is 127.
            A priority of 0 means always activate first.
            A priority of 255 means activate last.
        */
        public: uint8   mActivationPriority;
    protected:
        uint8           mIndex;
        ArrayActorPlane *mActorPlane;

        void updateArrayActorPlane(void);

    public:
        PlanarReflectionActor() :
            mCenter( Vector3::ZERO ), mHalfSize( Vector2::UNIT_SCALE ),
            mOrientation( Quaternion::IDENTITY ), mHasReservation( false ),
            mCurrentBoundSlot( 0xFF ), mActivationPriority( 127 ), mIndex( 0 ), mActorPlane( 0 )
        {
        }

        PlanarReflectionActor( const Vector3 &center, const Vector2 &halfSize,
                               const Quaternion orientation ) :
            mPlane( orientation.zAxis(), center ), mCenter( center ),
            mHalfSize( halfSize ), mOrientation( orientation ), mHasReservation( false ),
            mCurrentBoundSlot( 0xFF ), mActivationPriority( 127 ), mIndex( 0 ), mActorPlane( 0 )
        {
        }

        /** Sets the plane's position, size and orientation
        @param center
            XYZ position, in world coordinates
        @param halfSize
            2D half size of the rectangle that this plane creates, in local space.
        @param orientation
            Quaternion containing the normal and roll of the plane. Assumed to be unit-length
            The plane's normal is orientation.zAxis();
        */
        void setPlane( const Vector3 &center, const Vector2 &halfSize, const Quaternion &orientation );

        const Vector3& getCenter(void) const;
        const Vector2& getHalfSize(void) const;
        const Quaternion& getOrientation(void) const;
        const Vector3& getNormal(void) const;
        const Plane& getPlane(void) const;
        /// See PlanarReflections::reserve
        bool hasReservation(void) const;
        /// This value may have some meaning even if there is no reservation.
        uint8 getCurrentBoundSlot(void) const;

        Real getSquaredDistanceTo( const Vector3 &pos ) const;
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
