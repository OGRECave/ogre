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
#ifndef __REFAPP_JOINT_H__
#define __REFAPP_JOINT_H__

#include "OgreRefAppPrerequisites.h"

namespace OgreRefApp {

    /** Represents a linkage between application objects or between them and the world, enforcing 
        certain constraints.
    @remarks
        Joints can be used to link application objects together, or to link them to a static point
        in the world according to certain constraints. You should create joints using 
        World::createJoint. You should then set certain global options, like it's world
        anchor position, before attaching it to application objects. You application objects
        should already be positioned how you would like relative to the joint anchor, since once
        they are attached, they will be constrained by it.
    */
    class _OgreRefAppExport Joint 
    {
    public:
        /// The type of joint
        enum JointType {
            /// Ball & socket joint, has 3 degrees of freedom
            JT_BALL,
            /// Sliding joint, 1 degree of freedom (in-out)
            JT_SLIDER,
            /// Hinge joint, 1 degree of freedom
            JT_HINGE,
            /// Universal joint, like a double-hinge, 2 degrees of freedom
            JT_UNIVERSAL,
            /// 2 hinges in series, like the axel of a car
            JT_HINGE2
        };
        /** Constructor, however you should use World::createJoint(type, obj1, obj2). */
        Joint(JointType jtype);
        virtual ~Joint() { if (mOdeJoint) delete mOdeJoint; }


        /** Returns the type of this joint. */
        JointType getType(void);
        /** Set the anchor point of this joint.
        @remarks
            Sets the location, in world space, of the anchor point of this joint, which is usually 
            the hinge point or just the origin of joint. It has no meaning for JT_SLIDER and thus
            you don't need to call it for that.
        */
        virtual void setAnchorPosition(const Vector3& point) = 0;

        /** Gets the anchor position of this joint. */
        virtual const Vector3& getAnchorPosition(void);


        /** Gets the attached objects, a NULL means no object ie a static attachment. */
        virtual const std::pair<ApplicationObject*, ApplicationObject*>& getAttachments(void);

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
        virtual void setAxes(const Vector3& primaryAxis, const Vector3& secondaryAxis = Vector3::ZERO) = 0;

        /** Gets the axes of this joint. */
        virtual const std::pair<Vector3, Vector3>& getAxes(void);

    protected:

        /** Sets the objects attached to this joint.
        @remarks
            It appears that this has to be set before other joint params like
            anchor etc, otherwise the joint does not behave. Therefore it is internal
            and is called during construction.
        */
        void setAttachments(ApplicationObject* obj1, ApplicationObject* obj2);

        JointType mType;
        Vector3 mAnchor;
        std::pair<ApplicationObject*, ApplicationObject*> mAttachedObjects;
        std::pair<Vector3, Vector3> mAxes;

        // ODE object
        dJoint* mOdeJoint;









    };
}

#endif
