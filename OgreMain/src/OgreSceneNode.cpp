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
#include "OgreSceneNode.h"

#include "OgreException.h"
#include "OgreEntity.h"
#include "OgreCamera.h"
#include "OgreLight.h"
#include "OgreMath.h"
#include "OgreSceneManager.h"
#include "OgreMovableObject.h"
#include "OgreWireBoundingBox.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    SceneNode::SceneNode(SceneManager* creator)
        : Node()
        , mWireBoundingBox(0)
        , mShowBoundingBox(false)
        , mCreator(creator)
        , mYawFixed(false)
        , mAutoTrackTarget(0)
        , mIsInSceneGraph(false)
    {
        needUpdate();
    }
    //-----------------------------------------------------------------------
    SceneNode::SceneNode(SceneManager* creator, const String& name)
        : Node(name)
        , mWireBoundingBox(0)
        , mShowBoundingBox(false)
        , mCreator(creator)
        , mYawFixed(false)
        , mAutoTrackTarget(0)
        , mIsInSceneGraph(false)
    {
        needUpdate();
    }
    //-----------------------------------------------------------------------
    SceneNode::~SceneNode()
    {
        // Detach all objects, do this manually to avoid needUpdate() call 
        // which can fail because of deleted items
		ObjectMap::iterator itr;
		MovableObject* ret;
		for ( itr = mObjectsByName.begin(); itr != mObjectsByName.end(); itr++ )
		{
		  ret = itr->second;
		  ret->_notifyAttached((SceneNode*)0);
		}
        mObjectsByName.clear();

        if (mWireBoundingBox) {
			OGRE_DELETE mWireBoundingBox;
		}
    }
    //-----------------------------------------------------------------------
    void SceneNode::_update(bool updateChildren, bool parentHasChanged)
    {
        Node::_update(updateChildren, parentHasChanged);
        _updateBounds();
    }
    //-----------------------------------------------------------------------
	void SceneNode::setParent(Node* parent)
	{
		Node::setParent(parent);

		if (parent)
		{
			SceneNode* sceneParent = static_cast<SceneNode*>(parent);
			setInSceneGraph(sceneParent->isInSceneGraph());
		}
		else
		{
			setInSceneGraph(false);
		}
	}
    //-----------------------------------------------------------------------
	void SceneNode::setInSceneGraph(bool inGraph)
	{
		if (inGraph != mIsInSceneGraph)
		{
			mIsInSceneGraph = inGraph;
			// Tell children
	        ChildNodeMap::iterator child;
    	    for (child = mChildren.begin(); child != mChildren.end(); ++child)
        	{
            	SceneNode* sceneChild = static_cast<SceneNode*>(child->second);
				sceneChild->setInSceneGraph(inGraph);
			}
		}
	}
    //-----------------------------------------------------------------------
    void SceneNode::attachObject(MovableObject* obj)
    {
        if (obj->isAttached())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "Object already attached to a SceneNode or a Bone",
                "SceneNode::attachObject");
        }

        obj->_notifyAttached(this);

        // Also add to name index
        std::pair<ObjectMap::iterator, bool> insresult = 
            mObjectsByName.insert(ObjectMap::value_type(obj->getName(), obj));
        assert(insresult.second && "Object was not attached because an object of the "
            "same name was already attached to this node.");

        // Make sure bounds get updated (must go right to the top)
        needUpdate();
    }
    //-----------------------------------------------------------------------
    unsigned short SceneNode::numAttachedObjects(void) const
    {
        return static_cast< unsigned short >( mObjectsByName.size() );
    }
    //-----------------------------------------------------------------------
    MovableObject* SceneNode::getAttachedObject(unsigned short index)
    {
        if (index < mObjectsByName.size())
        {
            ObjectMap::iterator i = mObjectsByName.begin();
            // Increment (must do this one at a time)            
            while (index--)++i;

            return i->second;
        }
        else
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Object index out of bounds.", "SceneNode::getAttachedObject");
        }
        return 0;
    }
    //-----------------------------------------------------------------------
    MovableObject* SceneNode::getAttachedObject(const String& name)
    {
        // Look up 
        ObjectMap::iterator i = mObjectsByName.find(name);

        if (i == mObjectsByName.end())
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Attached object " + 
                name + " not found.", "SceneNode::getAttachedObject");
        }

        return i->second;

    }
    //-----------------------------------------------------------------------
    MovableObject* SceneNode::detachObject(unsigned short index)
    {
        MovableObject* ret;
        if (index < mObjectsByName.size())
        {

            ObjectMap::iterator i = mObjectsByName.begin();
            // Increment (must do this one at a time)            
            while (index--)++i;

            ret = i->second;
            mObjectsByName.erase(i);
            ret->_notifyAttached((SceneNode*)0);

            // Make sure bounds get updated (must go right to the top)
            needUpdate();

            return ret;

        }
        else
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Object index out of bounds.", "SceneNode::getAttchedEntity");
        }
        return 0;

    }
    //-----------------------------------------------------------------------
    MovableObject* SceneNode::detachObject(const String& name)
    {
        ObjectMap::iterator it = mObjectsByName.find(name);
        if (it == mObjectsByName.end())
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Object " + name + " is not attached "
                "to this node.", "SceneNode::detachObject");
        }
        MovableObject* ret = it->second;
        mObjectsByName.erase(it);
        ret->_notifyAttached((SceneNode*)0);
        // Make sure bounds get updated (must go right to the top)
        needUpdate();
        
        return ret;

    }
    //-----------------------------------------------------------------------
    void SceneNode::detachObject(MovableObject* obj)
    {
        ObjectMap::iterator i, iend;
        iend = mObjectsByName.end();
        for (i = mObjectsByName.begin(); i != iend; ++i)
        {
            if (i->second == obj)
            {
                mObjectsByName.erase(i);
                break;
            }
        }
        obj->_notifyAttached((SceneNode*)0);

        // Make sure bounds get updated (must go right to the top)
        needUpdate();

    }
    //-----------------------------------------------------------------------
    void SceneNode::detachAllObjects(void)
    {
		ObjectMap::iterator itr;
		MovableObject* ret;
		for ( itr = mObjectsByName.begin(); itr != mObjectsByName.end(); itr++ )
		{
		  ret = itr->second;
		  ret->_notifyAttached((SceneNode*)0);
		}
        mObjectsByName.clear();
        // Make sure bounds get updated (must go right to the top)
        needUpdate();
    }
    //-----------------------------------------------------------------------
    void SceneNode::_updateBounds(void)
    {
        // Reset bounds first
        mWorldAABB.setNull();

        // Update bounds from own attached objects
        ObjectMap::iterator i;
        for (i = mObjectsByName.begin(); i != mObjectsByName.end(); ++i)
        {
            // Merge world bounds of each object
            mWorldAABB.merge(i->second->getWorldBoundingBox(true));
        }

        // Merge with children
        ChildNodeMap::iterator child;
        for (child = mChildren.begin(); child != mChildren.end(); ++child)
        {
            SceneNode* sceneChild = static_cast<SceneNode*>(child->second);
            mWorldAABB.merge(sceneChild->mWorldAABB);
        }

    }
    //-----------------------------------------------------------------------
    void SceneNode::_findVisibleObjects(Camera* cam, RenderQueue* queue, 
		VisibleObjectsBoundsInfo* visibleBounds, bool includeChildren, 
		bool displayNodes, bool onlyShadowCasters)
    {
        // Check self visible
        if (!cam->isVisible(mWorldAABB))
            return;

        // Add all entities
        ObjectMap::iterator iobj;
        ObjectMap::iterator iobjend = mObjectsByName.end();
        for (iobj = mObjectsByName.begin(); iobj != iobjend; ++iobj)
        {
            // Tell attached objects about camera position (incase any extra processing they want to do)
            iobj->second->_notifyCurrentCamera(cam);
            if (iobj->second->isVisible() && 
                (!onlyShadowCasters || iobj->second->getCastShadows()))
            {
                iobj->second->_updateRenderQueue(queue);

				// update visible boundaries aab
				if (visibleBounds)
				{
					visibleBounds->merge(iobj->second->getWorldBoundingBox(true), 
						iobj->second->getWorldBoundingSphere(true), cam, 
						queue->getQueueGroup(iobj->second->getRenderQueueGroup())->getShadowsEnabled());
				}
            }
        }

        if (includeChildren)
        {
            ChildNodeMap::iterator child, childend;
            childend = mChildren.end();
            for (child = mChildren.begin(); child != childend; ++child)
            {
                SceneNode* sceneChild = static_cast<SceneNode*>(child->second);
                sceneChild->_findVisibleObjects(cam, queue, visibleBounds, includeChildren, 
					displayNodes, onlyShadowCasters);
            }
        }

        if (displayNodes)
        {
            // Include self in the render queue
            queue->addRenderable(this);
        }

		// Check if the bounding box should be shown.
		// See if our flag is set or if the scene manager flag is set.
		if (mShowBoundingBox || (mCreator && mCreator->getShowBoundingBoxes())) 
		{ 
			_addBoundingBoxToQueue(queue);
		}


    }


	void SceneNode::_addBoundingBoxToQueue(RenderQueue* queue) {
		// Create a WireBoundingBox if needed.
		if (mWireBoundingBox == NULL) {
			mWireBoundingBox = OGRE_NEW WireBoundingBox();
		}
		mWireBoundingBox->setupBoundingBox(mWorldAABB);
		queue->addRenderable(mWireBoundingBox);
	}

	void SceneNode::showBoundingBox(bool bShow) {
		mShowBoundingBox = bShow;
	}

	bool SceneNode::getShowBoundingBox() const {
		return mShowBoundingBox;
	}


    //-----------------------------------------------------------------------
    void SceneNode::updateFromParentImpl(void) const
    {
        Node::updateFromParentImpl();

        // Notify objects that it has been moved
        ObjectMap::const_iterator i;
        for (i = mObjectsByName.begin(); i != mObjectsByName.end(); ++i)
        {
            MovableObject* object = i->second;
            object->_notifyMoved();
        }
    }
    //-----------------------------------------------------------------------
    Node* SceneNode::createChildImpl(void)
    {
        assert(mCreator);
        return mCreator->createSceneNode();
    }
    //-----------------------------------------------------------------------
    Node* SceneNode::createChildImpl(const String& name)
    {
        assert(mCreator);
        return mCreator->createSceneNode(name);
    }
    //-----------------------------------------------------------------------
    const AxisAlignedBox& SceneNode::_getWorldAABB(void) const
    {
        return mWorldAABB;
    }
    //-----------------------------------------------------------------------
    SceneNode::ObjectIterator SceneNode::getAttachedObjectIterator(void)
    {
        return ObjectIterator(mObjectsByName.begin(), mObjectsByName.end());
    }
	//-----------------------------------------------------------------------
	SceneNode::ConstObjectIterator SceneNode::getAttachedObjectIterator(void) const
	{
		return ConstObjectIterator(mObjectsByName.begin(), mObjectsByName.end());
	}
    //-----------------------------------------------------------------------
    void SceneNode::removeAndDestroyChild(const String& name)
    {
        SceneNode* pChild = static_cast<SceneNode*>(getChild(name));
        pChild->removeAndDestroyAllChildren();

        removeChild(name);
        pChild->getCreator()->destroySceneNode(name);

    }
    //-----------------------------------------------------------------------
    void SceneNode::removeAndDestroyChild(unsigned short index)
    {
        SceneNode* pChild = static_cast<SceneNode*>(getChild(index));
        pChild->removeAndDestroyAllChildren();

        removeChild(index);
        pChild->getCreator()->destroySceneNode(pChild->getName());
    }
    //-----------------------------------------------------------------------
    void SceneNode::removeAndDestroyAllChildren(void)
    {
        ChildNodeMap::iterator i, iend;
        iend = mChildren.end();
        for (i = mChildren.begin(); i != iend;)
        {
            SceneNode* sn = static_cast<SceneNode*>(i->second);
			// increment iterator before destroying (iterator invalidated by 
			// SceneManager::destroySceneNode because it causes removal from parent)
			++i;
            sn->removeAndDestroyAllChildren();
            sn->getCreator()->destroySceneNode(sn->getName());
        }
	    mChildren.clear();
        needUpdate();
    }
    //-----------------------------------------------------------------------
	SceneNode* SceneNode::createChildSceneNode(const Vector3& translate, 
        const Quaternion& rotate)
	{
		return static_cast<SceneNode*>(this->createChild(translate, rotate));
	}
    //-----------------------------------------------------------------------
    SceneNode* SceneNode::createChildSceneNode(const String& name, const Vector3& translate, 
		const Quaternion& rotate)
	{
		return static_cast<SceneNode*>(this->createChild(name, translate, rotate));
	}
    //-----------------------------------------------------------------------
    void SceneNode::findLights(LightList& destList, Real radius, uint32 lightMask) const
    {
        // No any optimisation here, hope inherits more smart for that.
        //
        // If a scene node is static and lights have moved, light list won't change
        // can't use a simple global boolean flag since this is only called for
        // visible nodes, so temporarily visible nodes will not be updated
        // Since this is only called for visible nodes, skip the check for now
        //
        if (mCreator)
        {
            // Use SceneManager to calculate
            mCreator->_populateLightList(this, radius, destList, lightMask);
        }
        else
        {
            destList.clear();
        }
    }
    //-----------------------------------------------------------------------
    void SceneNode::setAutoTracking(bool enabled, SceneNode* target, 
        const Vector3& localDirectionVector,
        const Vector3& offset)
    {
        if (enabled)
        {
            mAutoTrackTarget = target;
            mAutoTrackOffset = offset;
            mAutoTrackLocalDirection = localDirectionVector;
        }
        else
        {
            mAutoTrackTarget = 0;
        }
        if (mCreator)
            mCreator->_notifyAutotrackingSceneNode(this, enabled);
    }
    //-----------------------------------------------------------------------
    void SceneNode::setFixedYawAxis(bool useFixed, const Vector3& fixedAxis)
    {
        mYawFixed = useFixed;
        mYawFixedAxis = fixedAxis;
    }

	//-----------------------------------------------------------------------
	void SceneNode::yaw(const Radian& angle, TransformSpace relativeTo)
	{
		if (mYawFixed)
		{
			rotate(mYawFixedAxis, angle, relativeTo);
		}
		else
		{
			rotate(Vector3::UNIT_Y, angle, relativeTo);
		}

	}
    //-----------------------------------------------------------------------
    void SceneNode::setDirection(Real x, Real y, Real z, TransformSpace relativeTo, 
        const Vector3& localDirectionVector)
    {
        setDirection(Vector3(x,y,z), relativeTo, localDirectionVector);
    }

    //-----------------------------------------------------------------------
    void SceneNode::setDirection(const Vector3& vec, TransformSpace relativeTo, 
        const Vector3& localDirectionVector)
    {
        // Do nothing if given a zero vector
        if (vec == Vector3::ZERO) return;

        // The direction we want the local direction point to
        Vector3 targetDir = vec.normalisedCopy();

        // Transform target direction to world space
        switch (relativeTo)
        {
        case TS_PARENT:
            if (mInheritOrientation)
            {
                if (mParent)
                {
                    targetDir = mParent->_getDerivedOrientation() * targetDir;
                }
            }
            break;
        case TS_LOCAL:
            targetDir = _getDerivedOrientation() * targetDir;
            break;
        case TS_WORLD:
            // default orientation
            break;
        }

        // Calculate target orientation relative to world space
        Quaternion targetOrientation;
        if( mYawFixed )
        {
            // Calculate the quaternion for rotate local Z to target direction
            Vector3 xVec = mYawFixedAxis.crossProduct(targetDir);
            xVec.normalise();
            Vector3 yVec = targetDir.crossProduct(xVec);
            yVec.normalise();
            Quaternion unitZToTarget = Quaternion(xVec, yVec, targetDir);

            if (localDirectionVector == Vector3::NEGATIVE_UNIT_Z)
            {
                // Specail case for avoid calculate 180 degree turn
                targetOrientation =
                    Quaternion(-unitZToTarget.y, -unitZToTarget.z, unitZToTarget.w, unitZToTarget.x);
            }
            else
            {
                // Calculate the quaternion for rotate local direction to target direction
                Quaternion localToUnitZ = localDirectionVector.getRotationTo(Vector3::UNIT_Z);
                targetOrientation = unitZToTarget * localToUnitZ;
            }
        }
        else
        {
            const Quaternion& currentOrient = _getDerivedOrientation();

            // Get current local direction relative to world space
            Vector3 currentDir = currentOrient * localDirectionVector;

            if ((currentDir+targetDir).squaredLength() < 0.00005f)
            {
                // Oops, a 180 degree turn (infinite possible rotation axes)
                // Default to yaw i.e. use current UP
                targetOrientation =
                    Quaternion(-currentOrient.y, -currentOrient.z, currentOrient.w, currentOrient.x);
            }
            else
            {
                // Derive shortest arc to new direction
                Quaternion rotQuat = currentDir.getRotationTo(targetDir);
                targetOrientation = rotQuat * currentOrient;
            }
        }

        // Set target orientation, transformed to parent space
        if (mParent && mInheritOrientation)
            setOrientation(mParent->_getDerivedOrientation().UnitInverse() * targetOrientation);
        else
            setOrientation(targetOrientation);
    }
    //-----------------------------------------------------------------------
    void SceneNode::lookAt( const Vector3& targetPoint, TransformSpace relativeTo, 
        const Vector3& localDirectionVector)
    {
        // Calculate ourself origin relative to the given transform space
        Vector3 origin;
        switch (relativeTo)
        {
        default:    // Just in case
        case TS_WORLD:
            origin = _getDerivedPosition();
            break;
        case TS_PARENT:
            origin = mPosition;
            break;
        case TS_LOCAL:
            origin = Vector3::ZERO;
            break;
        }

        setDirection(targetPoint - origin, relativeTo, localDirectionVector);
    }
    //-----------------------------------------------------------------------
    void SceneNode::_autoTrack(void)
    {
        // NB assumes that all scene nodes have been updated
        if (mAutoTrackTarget)
        {
            lookAt(mAutoTrackTarget->_getDerivedPosition() + mAutoTrackOffset, 
                TS_WORLD, mAutoTrackLocalDirection);
            // update self & children
            _update(true, true);
        }
    }
    //-----------------------------------------------------------------------
    SceneNode* SceneNode::getParentSceneNode(void) const
    {
        return static_cast<SceneNode*>(getParent());
    }
    //-----------------------------------------------------------------------
    void SceneNode::setVisible(bool visible, bool cascade)
    {
        ObjectMap::iterator oi, oiend;
        oiend = mObjectsByName.end();
        for (oi = mObjectsByName.begin(); oi != oiend; ++oi)
        {
            oi->second->setVisible(visible);
        }

        if (cascade)
        {
            ChildNodeMap::iterator i, iend;
            iend = mChildren.end();
            for (i = mChildren.begin(); i != iend; ++i)
            {
                static_cast<SceneNode*>(i->second)->setVisible(visible, cascade);
            }
        }
    }
	//-----------------------------------------------------------------------
	void SceneNode::setDebugDisplayEnabled(bool enabled, bool cascade)
	{
		ObjectMap::iterator oi, oiend;
		oiend = mObjectsByName.end();
		for (oi = mObjectsByName.begin(); oi != oiend; ++oi)
		{
			oi->second->setDebugDisplayEnabled(enabled);
		}

		if (cascade)
		{
			ChildNodeMap::iterator i, iend;
			iend = mChildren.end();
			for (i = mChildren.begin(); i != iend; ++i)
			{
				static_cast<SceneNode*>(i->second)->setDebugDisplayEnabled(enabled, cascade);
			}
		}
	}
    //-----------------------------------------------------------------------
    void SceneNode::flipVisibility(bool cascade)
    {
        ObjectMap::iterator oi, oiend;
        oiend = mObjectsByName.end();
        for (oi = mObjectsByName.begin(); oi != oiend; ++oi)
        {
            oi->second->setVisible(!oi->second->getVisible());
        }

        if (cascade)
        {
            ChildNodeMap::iterator i, iend;
            iend = mChildren.end();
            for (i = mChildren.begin(); i != iend; ++i)
            {
                static_cast<SceneNode*>(i->second)->flipVisibility(cascade);
            }
        }
    }



}
