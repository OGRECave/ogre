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
#include "OgreRefAppJoint.h"
#include "OgreRefAppApplicationObject.h"

namespace OgreRefApp {

    //-------------------------------------------------------------------------
    Joint::Joint(Joint::JointType jtype) 
        : mType(jtype), mAnchor(Vector3::ZERO), mOdeJoint(NULL)
    {
        mAxes.first = Vector3::UNIT_X;
        mAxes.second = Vector3::UNIT_Z;

        // Subclasses must set attachment since mOdeJoint must be created
        
    }
    //-------------------------------------------------------------------------
    Joint::JointType Joint::getType(void)
    {
        return mType;
    }
    //-------------------------------------------------------------------------
    const Vector3& Joint::getAnchorPosition(void)
    {
        return mAnchor;
    }
    //-------------------------------------------------------------------------
    void Joint::setAttachments(ApplicationObject* obj1, ApplicationObject* obj2)
    {
        dBodyID b1, b2;
        b1 = b2 = 0;
        if (obj1)
        {
            assert(obj1->getOdeBody() && "Cannot attach objects which do not have ODE bodies.");
            b1 = obj1->getOdeBody()->id();
        }
        if (obj2)
        {
            assert(obj2->getOdeBody() && "Cannot attach objects which do not have ODE bodies.");
            b2 = obj2->getOdeBody()->id();
        }

        mOdeJoint->attach(b1, b2);
        mAttachedObjects.first = obj1;
        mAttachedObjects.second = obj2;

    }
    //-------------------------------------------------------------------------
    const std::pair<ApplicationObject*, ApplicationObject*>& 
    Joint::getAttachments(void)
    {
        return mAttachedObjects;
    }
    //-------------------------------------------------------------------------
    const std::pair<Vector3, Vector3>& 
    Joint::getAxes(void)
    {
        return mAxes;
    }
    //-------------------------------------------------------------------------



}
