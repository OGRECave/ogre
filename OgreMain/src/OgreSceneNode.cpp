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
#include "OgreStableHeaders.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    SceneNode::SceneNode(SceneManager* creator) : SceneNode(creator, BLANKSTRING)
    {
    }
    //-----------------------------------------------------------------------
    SceneNode::SceneNode(SceneManager* creator, const String& name)
        : Node(name)
        , mCreator(creator)
        , mAutoTrackTarget(0)
        , mGlobalIndex(-1)
        , mYawFixed(false)
        , mIsInSceneGraph(false)
        , mShowBoundingBox(false)
    {
        needUpdate();
    }
    //-----------------------------------------------------------------------
    SceneNode::~SceneNode()
    {
        // Detach all objects, do this manually to avoid needUpdate() call 
        // which can fail because of deleted items
        for (auto & itr : mObjectsByName)
        {
            itr->_notifyAttached((SceneNode*)0);
        }
        mObjectsByName.clear();
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
            for (auto child : getChildren())
            {
                SceneNode* sceneChild = static_cast<SceneNode*>(child);
                sceneChild->setInSceneGraph(inGraph);
            }
        }
    }
    //-----------------------------------------------------------------------
    struct MovableObjectNameExists {
        const String& name;
        bool operator()(const MovableObject* mo) {
            return mo->getName() == name;
        }
    };
    void SceneNode::attachObject(MovableObject* obj)
    {
        OgreAssert(!obj->isAttached(), "Object already attached to a SceneNode or a Bone");

        obj->_notifyAttached(this);

        // Also add to name index
        MovableObjectNameExists pred = {obj->getName()};
        ObjectMap::iterator it = std::find_if(mObjectsByName.begin(), mObjectsByName.end(), pred);
        if (it != mObjectsByName.end())
            OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM,
                        "An object named '" + obj->getName() + "' already attached to this SceneNode");
        mObjectsByName.push_back(obj);

        // Make sure bounds get updated (must go right to the top)
        needUpdate();
    }
    //-----------------------------------------------------------------------
    MovableObject* SceneNode::getAttachedObject(const String& name) const
    {
        // Look up 
        MovableObjectNameExists pred = {name};
        auto i = std::find_if(mObjectsByName.begin(), mObjectsByName.end(), pred);

        if (i == mObjectsByName.end())
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Attached object " + 
                name + " not found.", "SceneNode::getAttachedObject");
        }

        return *i;
    }
    //-----------------------------------------------------------------------
    MovableObject* SceneNode::detachObject(unsigned short index)
    {
        OgreAssert(index < mObjectsByName.size(), "out of bounds");
        ObjectMap::iterator i = mObjectsByName.begin();
        i += index;

        MovableObject* ret = *i;
        std::swap(*i, mObjectsByName.back());
        mObjectsByName.pop_back();

        ret->_notifyAttached((SceneNode*)0);

        // Make sure bounds get updated (must go right to the top)
        needUpdate();

        return ret;
    }
    //-----------------------------------------------------------------------
    MovableObject* SceneNode::detachObject(const String& name)
    {
        MovableObjectNameExists pred = {name};
        ObjectMap::iterator it = std::find_if(mObjectsByName.begin(), mObjectsByName.end(), pred);

        if (it == mObjectsByName.end())
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Object " + name + " is not attached "
                "to this node.", "SceneNode::detachObject");
        }

        MovableObject* ret = *it;
        std::swap(*it, mObjectsByName.back());
        mObjectsByName.pop_back();

        ret->_notifyAttached((SceneNode*)0);
        // Make sure bounds get updated (must go right to the top)
        needUpdate();
        
        return ret;

    }
    //-----------------------------------------------------------------------
    void SceneNode::detachObject(MovableObject* obj)
    {
        auto it = std::find(mObjectsByName.begin(), mObjectsByName.end(), obj);
        OgreAssert(it != mObjectsByName.end(), "Object is not attached to this node");
        std::swap(*it, mObjectsByName.back());
        mObjectsByName.pop_back();
        obj->_notifyAttached((SceneNode*)0);

        // Make sure bounds get updated (must go right to the top)
        needUpdate();
    }
    //-----------------------------------------------------------------------
    void SceneNode::detachAllObjects(void)
    {
        for (auto & itr : mObjectsByName)
        {
            itr->_notifyAttached((SceneNode*)0);
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
        for (auto *o : mObjectsByName)
        {
            // Merge world bounds of each object
            mWorldAABB.merge(o->getWorldBoundingBox(true));
        }

        // Merge with children
        for (auto child : getChildren())
        {
            SceneNode* sceneChild = static_cast<SceneNode*>(child);
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
        for (auto *o : mObjectsByName)
        {
            queue->processVisibleObject(o, cam, onlyShadowCasters, visibleBounds);
        }

        if (includeChildren)
        {
            for (auto child : getChildren())
            {
                SceneNode* sceneChild = static_cast<SceneNode*>(child);
                sceneChild->_findVisibleObjects(cam, queue, visibleBounds, includeChildren, 
                    displayNodes, onlyShadowCasters);
            }
        }

        if (mCreator && mCreator->getDebugDrawer())
        {
            mCreator->getDebugDrawer()->drawSceneNode(this);
        }
    }

    SceneNode::ObjectIterator SceneNode::getAttachedObjectIterator(void) {
        return ObjectIterator(mObjectsByName.begin(), mObjectsByName.end());
    }
    SceneNode::ConstObjectIterator SceneNode::getAttachedObjectIterator(void) const {
        return ConstObjectIterator(mObjectsByName.begin(), mObjectsByName.end());
    }

    //-----------------------------------------------------------------------
    void SceneNode::updateFromParentImpl(void) const
    {
        Node::updateFromParentImpl();

        // Notify objects that it has been moved
        for (auto o : mObjectsByName)
        {
            o->_notifyMoved();
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
        SceneNode* pChild = static_cast<SceneNode*>(getChildren()[index]);
        pChild->removeAndDestroyAllChildren();

        removeChild(index);
        pChild->getCreator()->destroySceneNode(pChild);
    }
    //-----------------------------------------------------------------------
    void SceneNode::removeAndDestroyChild(SceneNode* child)
    {
        auto it = std::find(getChildren().begin(), getChildren().end(), child);
        OgreAssert(it != getChildren().end(), "Not a child of this SceneNode");
        removeAndDestroyChild(it - getChildren().begin());
    }
    //-----------------------------------------------------------------------
    void SceneNode::removeAndDestroyAllChildren(void)
    {
        // do not store iterators (invalidated by
        // SceneManager::destroySceneNode because it causes removal from parent)
        while(!getChildren().empty()) {
            SceneNode* sn = static_cast<SceneNode*>(getChildren().front());
            sn->removeAndDestroyAllChildren();
            sn->getCreator()->destroySceneNode(sn);
        }

        mChildren.clear();
        needUpdate();
    }
    void SceneNode::loadChildren(const String& filename)
    {
        String baseName, strExt;
        StringUtil::splitBaseFilename(filename, baseName, strExt);
        auto codec = Codec::getCodec(strExt);
        if (!codec)
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "No codec found to load " + filename);

        auto stream = Root::openFileStream(
            filename, ResourceGroupManager::getSingleton().getWorldResourceGroupName());
        codec->decode(stream, this);
    }
    void SceneNode::saveChildren(const String& filename)
    {
        String baseName, strExt;
        StringUtil::splitBaseFilename(filename, baseName, strExt);
        auto codec = Codec::getCodec(strExt);
        codec->encodeToFile(this, filename);
    }
    //-----------------------------------------------------------------------
    SceneNode* SceneNode::createChildSceneNode(const Vector3& inTranslate, 
        const Quaternion& inRotate)
    {
        return static_cast<SceneNode*>(this->createChild(inTranslate, inRotate));
    }
    //-----------------------------------------------------------------------
    SceneNode* SceneNode::createChildSceneNode(const String& name, const Vector3& inTranslate, 
        const Quaternion& inRotate)
    {
        return static_cast<SceneNode*>(this->createChild(name, inTranslate, inRotate));
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
    void SceneNode::setAutoTracking(bool enabled, SceneNode* const target, 
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
            if (getInheritOrientation())
            {
                if (getParent())
                {
                    targetDir = getParent()->_getDerivedOrientation() * targetDir;
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
            Vector3 yawAxis = mYawFixedAxis;

            if (getInheritOrientation() && getParent())
            {
                yawAxis = getParent()->_getDerivedOrientation() * yawAxis;
            }

            Quaternion unitZToTarget = Math::lookRotation(targetDir, yawAxis);

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
        if (getParent() && getInheritOrientation())
            setOrientation(getParent()->_getDerivedOrientation().UnitInverse() * targetOrientation);
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
            origin = getPosition();
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
    void SceneNode::setVisible(bool visible, bool cascade) const
    {
        for (auto o : mObjectsByName)
        {
            o->setVisible(visible);
        }

        if (cascade)
        {
            for (auto c : getChildren())
            {
                static_cast<SceneNode*>(c)->setVisible(visible, cascade);
            }
        }
    }
    //-----------------------------------------------------------------------
    void SceneNode::setDebugDisplayEnabled(bool enabled, bool cascade) const
    {
        for (auto o : mObjectsByName)
        {
            o->setDebugDisplayEnabled(enabled);
        }

        if (cascade)
        {
            for (auto c : getChildren())
            {
                static_cast<SceneNode*>(c)->setDebugDisplayEnabled(enabled, cascade);
            }
        }
    }
    //-----------------------------------------------------------------------
    void SceneNode::flipVisibility(bool cascade) const
    {
        for (auto o : mObjectsByName)
        {
            o->setVisible(!o->getVisible());
        }

        if (cascade)
        {
            for (auto c : getChildren())
            {
                static_cast<SceneNode*>(c)->flipVisibility(cascade);
            }
        }
    }
}
