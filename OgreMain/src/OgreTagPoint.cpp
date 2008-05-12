/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
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
#include "OgreStableHeaders.h"

#include "OgreTagPoint.h"
#include "OgreMatrix4.h"
#include "OgreMatrix3.h"
#include "OgreEntity.h"
#include "OgreSceneNode.h"
#include "OgreSkeleton.h"
#include "OgreQuaternion.h"


namespace Ogre {

    //-----------------------------------------------------------------------------
    TagPoint::TagPoint(unsigned short handle, Skeleton* creator)
        : Bone(handle, creator)
        , mParentEntity(0)
        , mChildObject(0)
        , mInheritParentEntityOrientation(true)
        , mInheritParentEntityScale(true)
    {
    }
    //-----------------------------------------------------------------------------
    TagPoint::~TagPoint()
    {
    }
    //-----------------------------------------------------------------------------
    Entity *TagPoint::getParentEntity(void) const
    {
        return mParentEntity;
    }
    //-----------------------------------------------------------------------------
    MovableObject* TagPoint::getChildObject(void) const
    {
        return mChildObject;
    }
    //-----------------------------------------------------------------------------
    void TagPoint::setParentEntity(Entity *pEntity)
    {
        mParentEntity = pEntity;
    }
    //-----------------------------------------------------------------------------
    void TagPoint::setChildObject(MovableObject *pObject)
    {
        mChildObject = pObject;
    }
    //-----------------------------------------------------------------------------
    void TagPoint::setInheritParentEntityOrientation(bool inherit)
    {
        mInheritParentEntityOrientation = inherit;
        needUpdate();
    }
    //-----------------------------------------------------------------------------
    bool TagPoint::getInheritParentEntityOrientation(void) const
    {
        return mInheritParentEntityOrientation;
    }
    //-----------------------------------------------------------------------------
    void TagPoint::setInheritParentEntityScale(bool inherit)
    {
        mInheritParentEntityScale = inherit;
        needUpdate();
    }
    //-----------------------------------------------------------------------------
    bool TagPoint::getInheritParentEntityScale(void) const
    {
        return mInheritParentEntityScale;
    }
    //-----------------------------------------------------------------------------
    const Matrix4& TagPoint::_getFullLocalTransform(void) const
    {
        return mFullLocalTransform;
    }
    //-----------------------------------------------------------------------------
    const Matrix4& TagPoint::getParentEntityTransform(void) const
    {

        return mParentEntity->_getParentNodeFullTransform();
    }
    //-----------------------------------------------------------------------------
    void TagPoint::needUpdate(bool forceParentUpdate)
    {
		Bone::needUpdate(forceParentUpdate);

        // We need to tell parent entities node
        if (mParentEntity)
        {
            Node* n = mParentEntity->getParentNode();
            if (n)
            {
                n->needUpdate();
            }

        }

    }
    //-----------------------------------------------------------------------------
    void TagPoint::updateFromParentImpl(void) const
    {
        // Call superclass
        Bone::updateFromParentImpl();

        // Save transform for local skeleton
        mFullLocalTransform.makeTransform(
            mDerivedPosition,
            mDerivedScale,
            mDerivedOrientation);

        // Include Entity transform
        if (mParentEntity)
        {
            Node* entityParentNode = mParentEntity->getParentNode();
            if (entityParentNode)
            {
                // Note: orientation/scale inherits from parent node already take care with
                // Bone::_updateFromParent, don't do that with parent entity transform.

                // Combine orientation with that of parent entity
                const Quaternion& parentOrientation = entityParentNode->_getDerivedOrientation();
                if (mInheritParentEntityOrientation)
                {
                    mDerivedOrientation = parentOrientation * mDerivedOrientation;
                }

                // Incorporate parent entity scale
                const Vector3& parentScale = entityParentNode->_getDerivedScale();
                if (mInheritParentEntityScale)
                {
                    mDerivedScale *= parentScale;
                }

                // Change position vector based on parent entity's orientation & scale
                mDerivedPosition = parentOrientation * (parentScale * mDerivedPosition);

                // Add altered position vector to parent entity
                mDerivedPosition += entityParentNode->_getDerivedPosition();
            }
        }

        if (mChildObject)
        {
            mChildObject->_notifyMoved();
        }
    }
    //-----------------------------------------------------------------------------
    const LightList& TagPoint::getLights(void) const
    {
        return mParentEntity->queryLights();
    }

}
