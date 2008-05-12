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
#include "OgreRefAppJointSubtypes.h"
#include "OgreRefAppWorld.h"

namespace OgreRefApp {

    //-------------------------------------------------------------------------
    BallJoint::BallJoint(Joint::JointType jtype, ApplicationObject* obj1, ApplicationObject* obj2)
        : Joint(jtype)
    {
        mOdeJoint = new dBallJoint(World::getSingleton().getOdeWorld()->id());
        setAttachments(obj1, obj2);
    }
    //-------------------------------------------------------------------------
    void BallJoint::setAnchorPosition(const Vector3& point)
    {
        dBallJoint* ballJoint = static_cast<dBallJoint*>(mOdeJoint);

        ballJoint->setAnchor(point.x, point.y, point.z);
        mAnchor = point;
    }
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    SliderJoint::SliderJoint(Joint::JointType jtype, ApplicationObject* obj1, ApplicationObject* obj2)
        : Joint(jtype)
    {
        mOdeJoint = new dSliderJoint(World::getSingleton().getOdeWorld()->id());
        setAttachments(obj1, obj2);

    }
    //-------------------------------------------------------------------------
    void SliderJoint::setAxes(const Vector3& a1, const Vector3& na)
    {
        dSliderJoint* sliderJoint = static_cast<dSliderJoint*>(mOdeJoint);
        sliderJoint->setAxis(a1.x, a1.y, a1.z);
        mAxes.first = a1;

    }
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    HingeJoint::HingeJoint(Joint::JointType jtype, ApplicationObject* obj1, ApplicationObject* obj2)
        : Joint(jtype)
    {
        mOdeJoint = new dHingeJoint(World::getSingleton().getOdeWorld()->id());
        setAttachments(obj1, obj2);
    }
    //-------------------------------------------------------------------------
    void HingeJoint::setAnchorPosition(const Vector3& point)
    {
        dHingeJoint* hinge = static_cast<dHingeJoint*>(mOdeJoint);
        hinge->setAnchor(point.x, point.y, point.z);
        mAnchor = point;
    }
    //-------------------------------------------------------------------------
    void HingeJoint::setAxes(const Vector3& a1, const Vector3& na)
    {
        dHingeJoint* hinge = static_cast<dHingeJoint*>(mOdeJoint);
        hinge->setAxis(a1.x, a1.y, a1.z);
        mAxes.first = a1;
    }
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    UniversalJoint::UniversalJoint(Joint::JointType jtype, ApplicationObject* obj1, ApplicationObject* obj2)
        : Joint(jtype)
    {
        mOdeJoint = new dUniversalJoint(World::getSingleton().getOdeWorld()->id());
        setAttachments(obj1, obj2);
    }
    //-------------------------------------------------------------------------
    void UniversalJoint::setAnchorPosition(const Vector3& point)
    {
        dUniversalJoint* univ = static_cast<dUniversalJoint*>(mOdeJoint);
        univ->setAnchor(point.x, point.y, point.z);
        mAnchor = point;
    }
    //-------------------------------------------------------------------------
    void UniversalJoint::setAxes(const Vector3& a1, const Vector3& a2)
    {
        dUniversalJoint* univ = static_cast<dUniversalJoint*>(mOdeJoint);
        univ->setAxis1(a1.x, a1.y, a1.z);
        univ->setAxis2(a2.x, a2.y, a2.z);
        mAxes.first = a1;
        mAxes.second = a2;
    }
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    Hinge2Joint::Hinge2Joint(Joint::JointType jtype, ApplicationObject* obj1, ApplicationObject* obj2)
        : Joint(jtype)
    {
        mOdeJoint = new dHinge2Joint(World::getSingleton().getOdeWorld()->id());
        setAttachments(obj1, obj2);
    }
    //-------------------------------------------------------------------------
    void Hinge2Joint::setAnchorPosition(const Vector3& point)
    {
        dHinge2Joint* hinge = static_cast<dHinge2Joint*>(mOdeJoint);
        hinge->setAnchor(point.x, point.y, point.z);
        mAnchor = point;
    }
    //-------------------------------------------------------------------------
    void Hinge2Joint::setAxes(const Vector3& a1, const Vector3& a2)
    {
        dHinge2Joint* hinge = static_cast<dHinge2Joint*>(mOdeJoint);
        hinge->setAxis1(a1.x, a1.y, a1.z);
        hinge->setAxis2(a2.x, a2.y, a2.z);
        mAxes.first = a1;
        mAxes.second = a2;
    }



}
