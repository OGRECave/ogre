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
#include "OgreOldNode.h"

#include "OgreException.h"
#include "OgreMath.h"
#include "OgreCamera.h"

namespace Ogre {
namespace v1 {

    OldNode::QueuedUpdates OldNode::msQueuedUpdates;
    //-----------------------------------------------------------------------
    OldNode::OldNode()
        :mParent(0),
        mNeedParentUpdate(false),
        mNeedChildUpdate(false),
        mParentNotified(false),
        mQueuedForUpdate(false),
        mOrientation(Quaternion::IDENTITY),
        mPosition(Vector3::ZERO),
        mScale(Vector3::UNIT_SCALE),
        mInheritOrientation(true),
        mInheritScale(true),
        mDerivedOrientation(Quaternion::IDENTITY),
        mDerivedPosition(Vector3::ZERO),
        mDerivedScale(Vector3::UNIT_SCALE),
        mInitialPosition(Vector3::ZERO),
        mInitialOrientation(Quaternion::IDENTITY),
        mInitialScale(Vector3::UNIT_SCALE),
        mCachedTransformOutOfDate(true),
        mListener(0)
    {
        // Generate a name
        mName = "";

        needUpdate();

    }
    //-----------------------------------------------------------------------
    OldNode::OldNode(const String& name)
        :
        mParent(0),
        mNeedParentUpdate(false),
        mNeedChildUpdate(false),
        mParentNotified(false),
        mQueuedForUpdate(false),
        mName(name),
        mOrientation(Quaternion::IDENTITY),
        mPosition(Vector3::ZERO),
        mScale(Vector3::UNIT_SCALE),
        mInheritOrientation(true),
        mInheritScale(true),
        mDerivedOrientation(Quaternion::IDENTITY),
        mDerivedPosition(Vector3::ZERO),
        mDerivedScale(Vector3::UNIT_SCALE),
        mInitialPosition(Vector3::ZERO),
        mInitialOrientation(Quaternion::IDENTITY),
        mInitialScale(Vector3::UNIT_SCALE),
        mCachedTransformOutOfDate(true),
        mListener(0)

    {

        needUpdate();

    }

    //-----------------------------------------------------------------------
    OldNode::~OldNode()
    {
        // Call listener (note, only called if there's something to do)
        if (mListener)
        {
            mListener->OldNodeDestroyed(this);
        }

        removeAllChildren();
        if(mParent)
            mParent->removeChild(this);

        if (mQueuedForUpdate)
        {
            // Erase from queued updates
            QueuedUpdates::iterator it =
                std::find(msQueuedUpdates.begin(), msQueuedUpdates.end(), this);
            assert(it != msQueuedUpdates.end());
            if (it != msQueuedUpdates.end())
            {
                // Optimised algorithm to erase an element from unordered vector.
                *it = msQueuedUpdates.back();
                msQueuedUpdates.pop_back();
            }
        }

    }
    //-----------------------------------------------------------------------
    OldNode* OldNode::getParent(void) const
    {
        return mParent;
    }

    //-----------------------------------------------------------------------
    void OldNode::setParent(OldNode* parent)
    {
        bool different = (parent != mParent);

        mParent = parent;
        // Request update from parent
        mParentNotified = false ;
        needUpdate();

        // Call listener (note, only called if there's something to do)
        if (mListener && different)
        {
            if (mParent)
                mListener->OldNodeAttached(this);
            else
                mListener->OldNodeDetached(this);
        }

    }

    //-----------------------------------------------------------------------
    const Matrix4& OldNode::_getFullTransform(void) const
    {
        if (mCachedTransformOutOfDate)
        {
            // Use derived values
            mCachedTransform.makeTransform(
                _getDerivedPosition(),
                _getDerivedScale(),
                _getDerivedOrientation());
            mCachedTransformOutOfDate = false;
        }
        return mCachedTransform;
    }
    //-----------------------------------------------------------------------
    void OldNode::_update(bool updateChildren, bool parentHasChanged)
    {
        // always clear information about parent notification
        mParentNotified = false;

        // See if we should process everyone
        if (mNeedParentUpdate || parentHasChanged)
        {
            // Update transforms from parent
            _updateFromParent();
        }

        if(updateChildren)
        {
            if (mNeedChildUpdate || parentHasChanged)
            {
                ChildOldNodeMap::iterator it, itend;
                itend = mChildren.end();
                for (it = mChildren.begin(); it != itend; ++it)
                {
                    OldNode* child = it->second;
                    child->_update(true, true);
                }
            }
            else
            {
                // Just update selected children
                ChildUpdateSet::iterator it, itend;
                itend = mChildrenToUpdate.end();
                for(it = mChildrenToUpdate.begin(); it != itend; ++it)
                {
                    OldNode* child = *it;
                    child->_update(true, false);
                }

            }

            mChildrenToUpdate.clear();
            mNeedChildUpdate = false;
        }
    }
    //-----------------------------------------------------------------------
    void OldNode::_updateFromParent(void) const
    {
        updateFromParentImpl();

        // Call listener (note, this method only called if there's something to do)
        if (mListener)
        {
            mListener->OldNodeUpdated(this);
        }
    }
    //-----------------------------------------------------------------------
    void OldNode::updateFromParentImpl(void) const
    {
        if (mParent)
        {
            // Update orientation
            const Quaternion& parentOrientation = mParent->_getDerivedOrientation();
            if (mInheritOrientation)
            {
                // Combine orientation with that of parent
                mDerivedOrientation = parentOrientation * mOrientation;
            }
            else
            {
                // No inheritance
                mDerivedOrientation = mOrientation;
            }

            // Update scale
            const Vector3& parentScale = mParent->_getDerivedScale();
            if (mInheritScale)
            {
                // Scale own position by parent scale, NB just combine
                // as equivalent axes, no shearing
                mDerivedScale = parentScale * mScale;
            }
            else
            {
                // No inheritance
                mDerivedScale = mScale;
            }

            // Change position vector based on parent's orientation & scale
            mDerivedPosition = parentOrientation * (parentScale * mPosition);

            // Add altered position vector to parents
            mDerivedPosition += mParent->_getDerivedPosition();
        }
        else
        {
            // Root OldNode, no parent
            mDerivedOrientation = mOrientation;
            mDerivedPosition = mPosition;
            mDerivedScale = mScale;
        }

        mCachedTransformOutOfDate = true;
        mNeedParentUpdate = false;

    }
    //-----------------------------------------------------------------------
    OldNode* OldNode::createChild(const Vector3& inTranslate, const Quaternion& inRotate)
    {
        OldNode* newOldNode = createChildImpl();
        newOldNode->translate(inTranslate);
        newOldNode->rotate(inRotate);
        this->addChild(newOldNode);

        return newOldNode;
    }
    //-----------------------------------------------------------------------
    OldNode* OldNode::createChild(const String& name, const Vector3& inTranslate, const Quaternion& inRotate)
    {
        OldNode* newOldNode = createChildImpl(name);
        newOldNode->translate(inTranslate);
        newOldNode->rotate(inRotate);
        this->addChild(newOldNode);

        return newOldNode;
    }
    //-----------------------------------------------------------------------
    void OldNode::addChild(OldNode* child)
    {
        if (child->mParent)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "OldNode '" + child->getName() + "' already was a child of '" +
                child->mParent->getName() + "'.",
                "OldNode::addChild");
        }

        mChildren.insert(ChildOldNodeMap::value_type(child->getName(), child));
        child->setParent(this);

    }
    //-----------------------------------------------------------------------
    unsigned short OldNode::numChildren(void) const
    {
        return static_cast< unsigned short >( mChildren.size() );
    }
    //-----------------------------------------------------------------------
    OldNode* OldNode::getChild(unsigned short index) const
    {
        if( index < mChildren.size() )
        {
            ChildOldNodeMap::const_iterator i = mChildren.begin();
            while (index--) ++i;
            return i->second;
        }
        else
            return NULL;
    }
    //-----------------------------------------------------------------------
    OldNode* OldNode::removeChild(unsigned short index)
    {
        OldNode* ret;
        if (index < mChildren.size())
        {
            ChildOldNodeMap::iterator i = mChildren.begin();
            while (index--) ++i;
            ret = i->second;
            // cancel any pending update
            cancelUpdate(ret);

            mChildren.erase(i);
            ret->setParent(NULL);
            return ret;
        }
        else
        {
            OGRE_EXCEPT(
                Exception::ERR_INVALIDPARAMS,
                "Child index out of bounds.",
                "OldNode::getChild" );
        }
        return 0;
    }
    //-----------------------------------------------------------------------
    OldNode* OldNode::removeChild(OldNode* child)
    {
        if (child)
        {
            ChildOldNodeMap::iterator i = mChildren.find(child->getName());
            // ensure it's our child
            if (i != mChildren.end() && i->second == child)
            {
                // cancel any pending update
                cancelUpdate(child);

                mChildren.erase(i);
                child->setParent(NULL);
            }
        }
        return child;
    }
    //-----------------------------------------------------------------------
    const Quaternion& OldNode::getOrientation() const
    {
        return mOrientation;
    }

    //-----------------------------------------------------------------------
    void OldNode::setOrientation( const Quaternion & q )
    {
        assert(!q.isNaN() && "Invalid orientation supplied as parameter");
        mOrientation = q;
        mOrientation.normalise();
        needUpdate();
    }
    //-----------------------------------------------------------------------
    void OldNode::setOrientation( Real w, Real x, Real y, Real z)
    {
        setOrientation(Quaternion(w, x, y, z));
    }
    //-----------------------------------------------------------------------
    void OldNode::resetOrientation(void)
    {
        mOrientation = Quaternion::IDENTITY;
        needUpdate();
    }

    //-----------------------------------------------------------------------
    void OldNode::setPosition(const Vector3& pos)
    {
        assert(!pos.isNaN() && "Invalid vector supplied as parameter");
        mPosition = pos;
        needUpdate();
    }


    //-----------------------------------------------------------------------
    void OldNode::setPosition(Real x, Real y, Real z)
    {
        Vector3 v(x,y,z);
        setPosition(v);
    }

    //-----------------------------------------------------------------------
    const Vector3 & OldNode::getPosition(void) const
    {
        return mPosition;
    }
    //-----------------------------------------------------------------------
    Matrix3 OldNode::getLocalAxes(void) const
    {
        Vector3 axisX = Vector3::UNIT_X;
        Vector3 axisY = Vector3::UNIT_Y;
        Vector3 axisZ = Vector3::UNIT_Z;

        axisX = mOrientation * axisX;
        axisY = mOrientation * axisY;
        axisZ = mOrientation * axisZ;

        return Matrix3(axisX.x, axisY.x, axisZ.x,
                       axisX.y, axisY.y, axisZ.y,
                       axisX.z, axisY.z, axisZ.z);
    }

    //-----------------------------------------------------------------------
    void OldNode::translate(const Vector3& d, TransformSpace relativeTo)
    {
        switch(relativeTo)
        {
        case TS_LOCAL:
            // position is relative to parent so transform downwards
            mPosition += mOrientation * d;
            break;
        case TS_WORLD:
            // position is relative to parent so transform upwards
            if (mParent)
            {
                mPosition += (mParent->_getDerivedOrientation().Inverse() * d)
                    / mParent->_getDerivedScale();
            }
            else
            {
                mPosition += d;
            }
            break;
        case TS_PARENT:
            mPosition += d;
            break;
        }
        needUpdate();

    }
    //-----------------------------------------------------------------------
    void OldNode::translate(Real x, Real y, Real z, TransformSpace relativeTo)
    {
        Vector3 v(x,y,z);
        translate(v, relativeTo);
    }
    //-----------------------------------------------------------------------
    void OldNode::translate(const Matrix3& axes, const Vector3& move, TransformSpace relativeTo)
    {
        Vector3 derived = axes * move;
        translate(derived, relativeTo);
    }
    //-----------------------------------------------------------------------
    void OldNode::translate(const Matrix3& axes, Real x, Real y, Real z, TransformSpace relativeTo)
    {
        Vector3 d(x,y,z);
        translate(axes,d,relativeTo);
    }
    //-----------------------------------------------------------------------
    void OldNode::roll(const Radian& angle, TransformSpace relativeTo)
    {
        rotate(Vector3::UNIT_Z, angle, relativeTo);
    }
    //-----------------------------------------------------------------------
    void OldNode::pitch(const Radian& angle, TransformSpace relativeTo)
    {
        rotate(Vector3::UNIT_X, angle, relativeTo);
    }
    //-----------------------------------------------------------------------
    void OldNode::yaw(const Radian& angle, TransformSpace relativeTo)
    {
        rotate(Vector3::UNIT_Y, angle, relativeTo);

    }
    //-----------------------------------------------------------------------
    void OldNode::rotate(const Vector3& axis, const Radian& angle, TransformSpace relativeTo)
    {
        Quaternion q;
        q.FromAngleAxis(angle,axis);
        rotate(q, relativeTo);
    }

    //-----------------------------------------------------------------------
    void OldNode::rotate(const Quaternion& q, TransformSpace relativeTo)
    {
        // Normalise quaternion to avoid drift
        Quaternion qnorm = q;
        qnorm.normalise();

        switch(relativeTo)
        {
        case TS_PARENT:
            // Rotations are normally relative to local axes, transform up
            mOrientation = qnorm * mOrientation;
            break;
        case TS_WORLD:
            // Rotations are normally relative to local axes, transform up
            mOrientation = mOrientation * _getDerivedOrientation().Inverse()
                * qnorm * _getDerivedOrientation();
            break;
        case TS_LOCAL:
            // Note the order of the mult, i.e. q comes after
            mOrientation = mOrientation * qnorm;
            break;
        }
        mOrientation.normalise();
        needUpdate();
    }

    
    //-----------------------------------------------------------------------
    void OldNode::_setDerivedPosition( const Vector3& pos )
    {
        //find where the OldNode would end up in parent's local space
        setPosition( mParent->convertWorldToLocalPosition( pos ) );
    }
    //-----------------------------------------------------------------------
    void OldNode::_setDerivedOrientation( const Quaternion& q )
    {
        //find where the OldNode would end up in parent's local space
        setOrientation( mParent->convertWorldToLocalOrientation( q ) );
    }

    //-----------------------------------------------------------------------
    const Quaternion & OldNode::_getDerivedOrientation(void) const
    {
        if (mNeedParentUpdate)
        {
            _updateFromParent();
        }
        return mDerivedOrientation;
    }
    //-----------------------------------------------------------------------
    const Vector3 & OldNode::_getDerivedPosition(void) const
    {
        if (mNeedParentUpdate)
        {
            _updateFromParent();
        }
        return mDerivedPosition;
    }
    //-----------------------------------------------------------------------
    const Vector3 & OldNode::_getDerivedScale(void) const
    {
        if (mNeedParentUpdate)
        {
            _updateFromParent();
        }
        return mDerivedScale;
    }
    //-----------------------------------------------------------------------
    Vector3 OldNode::convertWorldToLocalPosition( const Vector3 &worldPos )
    {
        if (mNeedParentUpdate)
        {
            _updateFromParent();
        }
        return mDerivedOrientation.Inverse() * (worldPos - mDerivedPosition) / mDerivedScale;
    }
    //-----------------------------------------------------------------------
    Vector3 OldNode::convertLocalToWorldPosition( const Vector3 &localPos )
    {
        if (mNeedParentUpdate)
        {
            _updateFromParent();
        }
        return (mDerivedOrientation * (localPos * mDerivedScale)) + mDerivedPosition;
    }
    //-----------------------------------------------------------------------
    Quaternion OldNode::convertWorldToLocalOrientation( const Quaternion &worldOrientation )
    {
        if (mNeedParentUpdate)
        {
            _updateFromParent();
        }
        return mDerivedOrientation.Inverse() * worldOrientation;
    }
    //-----------------------------------------------------------------------
    Quaternion OldNode::convertLocalToWorldOrientation( const Quaternion &localOrientation )
    {
        if (mNeedParentUpdate)
        {
            _updateFromParent();
        }
        return mDerivedOrientation * localOrientation;

    }
    //-----------------------------------------------------------------------
    void OldNode::removeAllChildren(void)
    {
        ChildOldNodeMap::iterator i, iend;
        iend = mChildren.end();
        for (i = mChildren.begin(); i != iend; ++i)
        {
            i->second->setParent(0);
        }
        mChildren.clear();
        mChildrenToUpdate.clear();
    }
    //-----------------------------------------------------------------------
    void OldNode::setScale(const Vector3& inScale)
    {
        assert(!inScale.isNaN() && "Invalid vector supplied as parameter");
        mScale = inScale;
        needUpdate();
    }
    //-----------------------------------------------------------------------
    void OldNode::setScale(Real x, Real y, Real z)
    {
        setScale(Vector3(x, y, z));
    }
    //-----------------------------------------------------------------------
    const Vector3 & OldNode::getScale(void) const
    {
        return mScale;
    }
    //-----------------------------------------------------------------------
    void OldNode::setInheritOrientation(bool inherit)
    {
        mInheritOrientation = inherit;
        needUpdate();
    }
    //-----------------------------------------------------------------------
    bool OldNode::getInheritOrientation(void) const
    {
        return mInheritOrientation;
    }
    //-----------------------------------------------------------------------
    void OldNode::setInheritScale(bool inherit)
    {
        mInheritScale = inherit;
        needUpdate();
    }
    //-----------------------------------------------------------------------
    bool OldNode::getInheritScale(void) const
    {
        return mInheritScale;
    }
    //-----------------------------------------------------------------------
    void OldNode::scale(const Vector3& inScale)
    {
        mScale = mScale * inScale;
        needUpdate();

    }
    //-----------------------------------------------------------------------
    void OldNode::scale(Real x, Real y, Real z)
    {
        mScale.x *= x;
        mScale.y *= y;
        mScale.z *= z;
        needUpdate();

    }
    //-----------------------------------------------------------------------
    const String& OldNode::getName(void) const
    {
        return mName;
    }
    //-----------------------------------------------------------------------
    void OldNode::setInitialState(void)
    {
        mInitialPosition = mPosition;
        mInitialOrientation = mOrientation;
        mInitialScale = mScale;
    }
    //-----------------------------------------------------------------------
    void OldNode::resetToInitialState(void)
    {
        mPosition = mInitialPosition;
        mOrientation = mInitialOrientation;
        mScale = mInitialScale;

        needUpdate();
    }
    //-----------------------------------------------------------------------
    const Vector3& OldNode::getInitialPosition(void) const
    {
        return mInitialPosition;
    }
    //-----------------------------------------------------------------------
    const Quaternion& OldNode::getInitialOrientation(void) const
    {
        return mInitialOrientation;

    }
    //-----------------------------------------------------------------------
    const Vector3& OldNode::getInitialScale(void) const
    {
        return mInitialScale;
    }
    //-----------------------------------------------------------------------
    OldNode* OldNode::getChild(const String& name) const
    {
        ChildOldNodeMap::const_iterator i = mChildren.find(name);

        if (i == mChildren.end())
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Child OldNode named " + name +
                " does not exist.", "OldNode::getChild");
        }
        return i->second;

    }
    //-----------------------------------------------------------------------
    OldNode* OldNode::removeChild(const String& name)
    {
        ChildOldNodeMap::iterator i = mChildren.find(name);

        if (i == mChildren.end())
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Child OldNode named " + name +
                " does not exist.", "OldNode::removeChild");
        }

        OldNode* ret = i->second;
        // Cancel any pending update
        cancelUpdate(ret);

        mChildren.erase(i);
        ret->setParent(NULL);

        return ret;


    }
    //-----------------------------------------------------------------------
    OldNode::ChildOldNodeIterator OldNode::getChildIterator(void)
    {
        return ChildOldNodeIterator(mChildren.begin(), mChildren.end());
    }
    //-----------------------------------------------------------------------
    OldNode::ConstChildOldNodeIterator OldNode::getChildIterator(void) const
    {
        return ConstChildOldNodeIterator(mChildren.begin(), mChildren.end());
    }
    //-----------------------------------------------------------------------
    Real OldNode::getSquaredViewDepth(const Camera* cam) const
    {
        Vector3 diff = _getDerivedPosition() - cam->getDerivedPosition();

        // NB use squared length rather than real depth to avoid square root
        return diff.squaredLength();
    }
    //-----------------------------------------------------------------------
    void OldNode::needUpdate(bool forceParentUpdate)
    {

        mNeedParentUpdate = true;
        mNeedChildUpdate = true;
        mCachedTransformOutOfDate = true;

        // Make sure we're not root and parent hasn't been notified before
        if (mParent && (!mParentNotified || forceParentUpdate))
        {
            mParent->requestUpdate(this, forceParentUpdate);
            mParentNotified = true ;
        }

        // all children will be updated
        mChildrenToUpdate.clear();
    }
    //-----------------------------------------------------------------------
    void OldNode::requestUpdate(OldNode* child, bool forceParentUpdate)
    {
        // If we're already going to update everything this doesn't matter
        if (mNeedChildUpdate)
        {
            return;
        }

        mChildrenToUpdate.insert(child);
        // Request selective update of me, if we didn't do it before
        if (mParent && (!mParentNotified || forceParentUpdate))
        {
            mParent->requestUpdate(this, forceParentUpdate);
            mParentNotified = true ;
        }

    }
    //-----------------------------------------------------------------------
    void OldNode::cancelUpdate(OldNode* child)
    {
        mChildrenToUpdate.erase(child);

        // Propagate this up if we're done
        if (mChildrenToUpdate.empty() && mParent && !mNeedChildUpdate)
        {
            mParent->cancelUpdate(this);
            mParentNotified = false ;
        }
    }
    //-----------------------------------------------------------------------
    void OldNode::queueNeedUpdate(OldNode* n)
    {
        // Don't queue the OldNode more than once
        if (!n->mQueuedForUpdate)
        {
            n->mQueuedForUpdate = true;
            msQueuedUpdates.push_back(n);
        }
    }
    //-----------------------------------------------------------------------
    void OldNode::processQueuedUpdates(void)
    {
        for (QueuedUpdates::iterator i = msQueuedUpdates.begin();
            i != msQueuedUpdates.end(); ++i)
        {
            // Update, and force parent update since chances are we've ended
            // up with some mixed state in there due to re-entrancy
            OldNode* n = *i;
            n->mQueuedForUpdate = false;
            n->needUpdate(true);
        }
        msQueuedUpdates.clear();
    }
    //---------------------------------------------------------------------
}
}

