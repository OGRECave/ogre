/*
-----------------------------------------------------------------------------
This source file is part of the OGRE Reference Application, a layer built
on top of OGRE(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#ifndef __REFAPP_COLLIDECAMERA_H__
#define __REFAPP_COLLIDECAMERA_H__

#include "OgreRefAppPrerequisites.h"
#include "OgreCamera.h"
#include "OgreRefAppApplicationObject.h"

namespace OgreRefApp {

    /** A camera which can interact with the world. */
    class _OgreRefAppExport CollideCamera : public ApplicationObject
    {
    protected:
        /// Contained camera
        /* Note that I choose to contain Camera rather than subclass because the
           multiple inheritence would get very nasty (lots of method name clashes)
           and it's better that we can hide a few non-relevant methods this way.
           ApplicationObject needs to be the top-level interface anyway. */
        Camera *mCamera;
        /// Set up
        void setUp(const String& name);
        /// Triggers recacl of collison bounds
        void nearDistChanged(void);
    public:
        CollideCamera(const String& name);

        /** Gets the internal Camera object. */
        Camera* getRealCamera(void) { return mCamera; }

        // ----------------------------------------------
        // Overridden methods from ApplicationObject
        // Note that we position the camera using the node
        // But we orient the camera on it's own rotation
        // ----------------------------------------------
       
        /** This method is called automatically if testCollide indicates a real collision. 
        */
        void _notifyCollided(SceneQuery::WorldFragment* wf, const CollisionInfo& info);
        /** Sets the orientation of this object. */
        void setOrientation(const Quaternion& orientation);
        /** Gets the current orientation of this object. */
        const Quaternion& getOrientation(void);

        /** Moves the object along it's local  axes.
            @par
                This method moves the object by the supplied vector along the
                local axes of the obect.
            @param 
                d Vector with x,y,z values representing the translation.
        */
        void translate(const Vector3& d);
        /** Rotate the object around the local Z-axis.
        */
        void roll(const Radian& angle);

        /** Rotate the object around the local X-axis.
        */
        void pitch(const Radian& angle);

        /** Rotate the object around the local Y-axis.
        */
        void yaw(const Radian& angle);

        /** Rotate the object around an arbitrary axis.
        */
        void rotate(const Vector3& axis, const Radian& angle);

        /** Rotate the object around an aritrary axis using a Quarternion.
        */
        void rotate(const Quaternion& q);

        // ----------------------------------------------
        // The following are methods delegated to Camera
        // ----------------------------------------------

        /** Sets the type of projection to use (orthographic or perspective). Default is perspective.
        */
        void setProjectionType(ProjectionType pt);

        /** Retrieves info on the type of projection used (orthographic or perspective).
        */
        ProjectionType getProjectionType(void) const;

        /** Sets the level of rendering detail required from this camera.
            @remarks
                Each camera is set to render at full detail by default, that is
                with full texturing, lighting etc. This method lets you change
                that behaviour, allowing you to make the camera just render a
                wireframe view, for example.
        */
        void setPolygonMode(PolygonMode sd);

        /** Retrieves the level of detail that the camera will render.
        */
        PolygonMode getPolygonMode(void) const;

        /** Sets the camera's direction vector.
            @remarks
                Note that the 'up' vector for the camera will automatically be recalculated based on the
                current 'up' vector (i.e. the roll will remain the same).
        */
        void setDirection(Real x, Real y, Real z);

        /** Sets the camera's direction vector.
        */
        void setDirection(const Vector3& vec);

        /* Gets the camera's direction.
        */
        Vector3 getDirection(void) const;


        /** Points the camera at a location in worldspace.
            @remarks
                This is a helper method to automatically generate the
                direction vector for the camera, based on it's current position
                and the supplied look-at point.
            @param
                targetPoint A vector specifying the look at point.
        */
        void lookAt( const Vector3& targetPoint );
        /** Points the camera at a location in worldspace.
            @remarks
                This is a helper method to automatically generate the
                direction vector for the camera, based on it's current position
                and the supplied look-at point.
            @param
                x
            @param
                y
            @param
                z Co-ordinates of the point to look at.
        */
        void lookAt(Real x, Real y, Real z);

        /** Tells the camera whether to yaw around it's own local Y axis or a fixed axis of choice.
            @remarks
                This method allows you to change the yaw behaviour of the camera - by default, the camera
                yaws around it's own local Y axis. This is often what you want - for example a flying camera
                - but sometimes this produces unwanted effects. For example, if you're making a first-person
                shooter, you really don't want the yaw axis to reflect the local camera Y, because this would
                mean a different yaw axis if the player is looking upwards rather than when they are looking
                straight ahead. You can change this behaviour by setting the yaw to a fixed axis (say, the world Y).
            @param
                useFixed If true, the axis passed in the second parameter will always be the yaw axis no
                matter what the camera orientation. If false, the camera returns to it's default behaviour.
            @param
                fixedAxis The axis to use if the first parameter is true.
        */
        void setFixedYawAxis( bool useFixed, const Vector3& fixedAxis = Vector3::UNIT_Y );

        /** Sets the Y-dimension Field Of View (FOV) of the camera.
            @remarks
                Field Of View (FOV) is the angle made between the camera's position, and the left & right edges
                of the 'screen' onto which the scene is projected. High values (90+) result in a wide-angle,
                fish-eye kind of view, low values (30-) in a stretched, telescopic kind of view. Typical values
                are between 45 and 60.
            @par
                This value represents the HORIZONTAL field-of-view. The vertical field of view is calculated from
                this depending on the dimensions of the viewport (they will only be the same if the viewport is square).
            @note
                Setting the FOV overrides the value supplied for Camera::setNearClipPlane.
         */
        void setFOVy(const Radian& fovy);

        /** Retrieves the cameras Y-dimension Field Of View (FOV).
        */
        const Radian& getFOVy(void) const;

        /** Sets the position of the near clipping plane.
            @remarks
                The position of the near clipping plane is the distance from the cameras position to the screen
                on which the world is projected. The near plane distance, combined with the field-of-view and the
                aspect ratio, determines the size of the viewport through which the world is viewed (in world
                co-ordinates). Note that this world viewport is different to a screen viewport, which has it's
                dimensions expressed in pixels. The cameras viewport should have the same aspect ratio as the
                screen viewport it renders into to avoid distortion.
            @param
                near The distance to the near clipping plane from the camera in world coordinates.
         */
        void setNearClipDistance(Real nearDist);

        /** Sets the position of the near clipping plane.
        */
        Real getNearClipDistance(void) const;

        /** Sets the distance to the far clipping plane.
            @remarks
                The view frustrum is a pyramid created from the camera position and the edges of the viewport.
                This frustrum does not extend to infinity - it is cropped near to the camera and there is a far
                plane beyond which nothing is displayed. This method sets the distance for the far plane. Different
                applications need different values: e.g. a flight sim needs a much further far clipping plane than
                a first-person shooter. An important point here is that the larger the gap between near and far
                clipping planes, the lower the accuracy of the Z-buffer used to depth-cue pixels. This is because the
                Z-range is limited to the size of the Z buffer (16 or 32-bit) and the max values must be spread over
                the gap between near and far clip planes. The bigger the range, the more the Z values will
                be approximated which can cause artifacts when lots of objects are close together in the Z-plane. So
                make sure you clip as close to the camera as you can - don't set a huge value for the sake of
                it.
            @param
                far The distance to the far clipping plane from the camera in world coordinates.
        */
        void setFarClipDistance(Real farDist);

        /** Retrieves the distance from the camera to the far clipping plane.
        */
        Real getFarClipDistance(void) const;

        /** Sets the aspect ratio for the camera viewport.
            @remarks
                The ratio between the x and y dimensions of the rectangular area visible through the camera
                is known as aspect ratio: aspect = width / height .
            @par
                The default for most fullscreen windows is 1.3333 - this is also assumed by Ogre unless you
                use this method to state otherwise.
        */
        void setAspectRatio(Real ratio);

        /** Retreives the current aspect ratio.
        */
        Real getAspectRatio(void) const;

        /** Retrieves a specified plane of the frustum.
            @remarks
                Gets a reference to one of the planes which make up the camera frustum, e.g. for clipping purposes.
        */
        const Plane& getFrustumPlane( FrustumPlane plane );

        /** Tests whether the given container is visible in the Frustum.
            @param
                bound Bounding box to be checked
            @param
                culledBy Optional pointer to an int which will be filled by the plane number which culled
                the box if the result was false;
            @returns
                If the box was visible, true is returned.
            @par
                Otherwise, false is returned.
        */
        bool isVisible(const AxisAlignedBox& bound, FrustumPlane* culledBy = 0);

        /** Tests whether the given container is visible in the Frustum.
            @param
                bound Bounding sphere to be checked
            @param
                culledBy Optional pointer to an int which will be filled by the plane number which culled
                the box if the result was false;
            @returns
                If the sphere was visible, true is returned.
            @par
                Otherwise, false is returned.
        */
        bool isVisible(const Sphere& bound, FrustumPlane* culledBy = 0);

        /** Tests whether the given vertex is visible in the Frustum.
            @param
                vert Vertex to be checked
            @param
                culledBy Optional pointer to an int which will be filled by the plane number which culled
                the box if the result was false;
            @returns
                If the box was visible, true is returned.
            @par
                Otherwise, false is returned.
        */
        bool isVisible(const Vector3& vert, FrustumPlane* culledBy = 0);

    };

}

#endif
