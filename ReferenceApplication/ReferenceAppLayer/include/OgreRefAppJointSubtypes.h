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
#ifndef __REFAPP_JOINTSUBTYPES_H__
#define __REFAPP_JOINTSUBTYPES_H__

#include "OgreRefAppPrerequisites.h"
#include "OgreRefAppJoint.h"

namespace OgreRefApp {

    /** Implementation of ball joint. */
    class _OgreRefAppExport BallJoint : public Joint
    {
    public:
        BallJoint(Joint::JointType jtype, ApplicationObject* obj1, ApplicationObject* obj2);
        ~BallJoint() {}
        /** Set the anchor point of this joint.
        @remarks
            Sets the location, in world space, of the anchor point of this joint, which can be
            the hinge point or just the origin of joint.
        */
        void setAnchorPosition(const Vector3& point);

        /** Sets the axes for this joint.
        @remarks
            Has no meaning for this type of joint, so does nothing.
        */
        void setAxes(const Vector3& primaryAxis, const Vector3& secondaryAxis = Vector3::ZERO) {}
    protected:
    };

    /** Implementation of slider joint. */
    class _OgreRefAppExport SliderJoint : public Joint
    {
    public:
        SliderJoint(Joint::JointType jtype, ApplicationObject* obj1, ApplicationObject* obj2);
        ~SliderJoint() {}
        /** Set the anchor point of this joint.
        @remarks
            Has no meaning for a slider, thus unimplemented.
        */
        void setAnchorPosition(const Vector3& point) {}

        /** Sets the axes for this joint.
        @remarks
            The meaning of axes for a joint depends on it's type:
            <ul>
            <li>For JT_BALL, it has no meaning and you don't need to call it.</li>
            <li>For JT_SLIDER, only one is applicable and it's the axis along which the slide occurs. </li>
            <li>For JT_HINGE, only one is applicable and it's the hinge axis. </li>
            <li>For JT_UNIVERSAL, and JT_HINGE2 it's the 2 hinge axes.</li>
            </ul>
        */
        void setAxes(const Vector3& primaryAxis, const Vector3& secondaryAxis = Vector3::ZERO);
    protected:
    };

    /** Implementation of hinge joint. */
    class _OgreRefAppExport HingeJoint : public Joint
    {
    public:
        HingeJoint(Joint::JointType jtype, ApplicationObject* obj1, ApplicationObject* obj2);
        ~HingeJoint() {}
        /** Set the anchor point of this joint.
        */
        void setAnchorPosition(const Vector3& point);

        /** Sets the axes for this joint.
        @remarks
            The meaning of axes for a joint depends on it's type:
            <ul>
            <li>For JT_BALL, it has no meaning and you don't need to call it.</li>
            <li>For JT_SLIDER, only one is applicable and it's the axis along which the slide occurs. </li>
            <li>For JT_HINGE, only one is applicable and it's the hinge axis. </li>
            <li>For JT_UNIVERSAL, and JT_HINGE2 it's the 2 hinge axes.</li>
            </ul>
        */
        void setAxes(const Vector3& primaryAxis, const Vector3& secondaryAxis = Vector3::ZERO);
    protected:
    };

    /** Implementation of universal joint. */
    class _OgreRefAppExport UniversalJoint : public Joint
    {
    public:
        UniversalJoint(Joint::JointType jtype, ApplicationObject* obj1, ApplicationObject* obj2);
        ~UniversalJoint() {}
        /** Set the anchor point of this joint.
        */
        void setAnchorPosition(const Vector3& point);

        /** Sets the axes for this joint.
        @remarks
            The meaning of axes for a joint depends on it's type:
            <ul>
            <li>For JT_BALL, it has no meaning and you don't need to call it.</li>
            <li>For JT_SLIDER, only one is applicable and it's the axis along which the slide occurs. </li>
            <li>For JT_HINGE, only one is applicable and it's the hinge axis. </li>
            <li>For JT_UNIVERSAL, and JT_HINGE2 it's the 2 hinge axes.</li>
            </ul>
        */
        void setAxes(const Vector3& primaryAxis, const Vector3& secondaryAxis = Vector3::ZERO);
    protected:
    };

    /** Implementation of hinge2 joint. */
    class _OgreRefAppExport Hinge2Joint : public Joint
    {
    public:
        Hinge2Joint(Joint::JointType jtype, ApplicationObject* obj1, ApplicationObject* obj2);
        ~Hinge2Joint() {}
        /** Set the anchor point of this joint.
        */
        void setAnchorPosition(const Vector3& point);

        /** Sets the axes for this joint.
        @remarks
            The meaning of axes for a joint depends on it's type:
            <ul>
            <li>For JT_BALL, it has no meaning and you don't need to call it.</li>
            <li>For JT_SLIDER, only one is applicable and it's the axis along which the slide occurs. </li>
            <li>For JT_HINGE, only one is applicable and it's the hinge axis. </li>
            <li>For JT_UNIVERSAL, and JT_HINGE2 it's the 2 hinge axes.</li>
            </ul>
        */
        void setAxes(const Vector3& primaryAxis, const Vector3& secondaryAxis = Vector3::ZERO);
    protected:
    };

}

#endif
