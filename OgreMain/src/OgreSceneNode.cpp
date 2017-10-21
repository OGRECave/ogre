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
#include "OgreSceneNode.h"

#include "OgreException.h"
#include "OgreEntity.h"
#include "OgreCamera.h"
#include "OgreLight.h"
#include "OgreMath.h"
#include "OgreSceneManager.h"
#include "OgreMovableObject.h"
#include "OgreWireBoundingBox.h"

#include "Animation/OgreSkeletonInstance.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    SceneNode::SceneNode( IdType id, SceneManager* creator, NodeMemoryManager *nodeMemoryManager,
                            SceneNode *parent )
        : Node( id, nodeMemoryManager, parent )
        , mCreator(creator)
		, mYawFixed(false)
    {
    }
    //-----------------------------------------------------------------------
    SceneNode::SceneNode( const Transform &transformPtrs )
        : Node( transformPtrs )
        , mCreator(0)
		, mYawFixed(false)
    {
    }
    //-----------------------------------------------------------------------
    SceneNode::~SceneNode()
    {
        if( mListener )
            mCreator->unregisterSceneNodeListener( this );

        // Detach all objects
        detachAllObjects();
        detachAllBones();
    }
    //-----------------------------------------------------------------------
    bool SceneNode::setStatic( bool bStatic )
    {
        bool retVal = Node::setStatic( bStatic );

        bool ourCurrentStatus = isStatic();

        if( retVal )
        {
            if( mCreator && bStatic )
                mCreator->notifyStaticDirty( this );

            //Now apply the same state to all our attachments.
            ObjectVec::const_iterator itor = mAttachments.begin();
            ObjectVec::const_iterator end  = mAttachments.end();

            while( itor != end )
            {
                MovableObject *obj = *itor;
                if( obj->isStatic() != ourCurrentStatus )
                {
                    bool result = obj->setStatic( bStatic );
                    if( !result )
                    {
                        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                            "Calling SceneNode::setStatic but attachment ID: " +
                            StringConverter::toString( obj->getId() ) + ", named '" + obj->getName() +
                            "' can't switch after creation. This entity must be created in the given"
                            " state before making the node switch", "SceneNode::setStatic");
                    }
                }
                ++itor;
            }
        }

        return retVal;
    }
    //-----------------------------------------------------------------------
    void SceneNode::_notifyStaticDirty(void) const
    {
        if( mCreator )
        {
            //All our attachments are dirty now.
            ObjectVec::const_iterator itor = mAttachments.begin();
            ObjectVec::const_iterator end  = mAttachments.end();

            while( itor != end )
                mCreator->notifyStaticAabbDirty( *itor++ );
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
        mAttachments.push_back( obj );
        obj->mParentIndex = mAttachments.size() - 1;
        
        //Do this after attaching to allow proper cleanup in cases
        //where object assumes it always has a scene node attached
        if( obj->isStatic() != this->isStatic() )
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "Object is static while Node isn't, or viceversa",
                "SceneNode::attachObject");
        }
    }
    //-----------------------------------------------------------------------
    SceneNode::ObjectVec::iterator SceneNode::getAttachedObjectIt( const String& name )
    {
        ObjectVec::iterator itor = mAttachments.begin();
        ObjectVec::iterator end  = mAttachments.end();

        while( itor != end )
        {
            if( (*itor)->getName() == name )
                return itor;
            ++itor;
        }

        return end;
    }
    //-----------------------------------------------------------------------
    SceneNode::ObjectVec::const_iterator SceneNode::getAttachedObjectIt( const String& name ) const
    {
        ObjectVec::const_iterator itor = mAttachments.begin();
        ObjectVec::const_iterator end  = mAttachments.end();

        while( itor != end )
        {
            if( (*itor)->getName() == name )
                return itor;
            ++itor;
        }

        return end;
    }
    //-----------------------------------------------------------------------
    MovableObject* SceneNode::getAttachedObject( const String& name )
    {
        ObjectVec::const_iterator itor = getAttachedObjectIt( name );

        if( itor == mAttachments.end() )
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Attached object " + 
                    name + " not found.", "SceneNode::getAttachedObject");
        }

        return *itor;
    }
    //-----------------------------------------------------------------------
    void SceneNode::detachObject( MovableObject* obj )
    {
        if( obj->getParentNode() != this )
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "MovableObject ID: " +
                StringConverter::toString( obj->getId() ) + ", named '" + obj->getName() +
                "' is not attached to this SceneNode!", "SceneNode::detachObject");
        }
        else if( obj->mParentIndex >= mAttachments.size() ||
                 obj != *(mAttachments.begin() + obj->mParentIndex) )
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "MovableObject ID: " +
                StringConverter::toString( obj->getId() ) + ", named '" + obj->getName() +
                "' had it's mParentIndex out of date!!! (or the MovableObject wasn't "
                "attached to this SceneNode)", "SceneNode::detachObject");
        }

        ObjectVec::iterator itor = mAttachments.begin() + obj->mParentIndex;

        (*itor)->_notifyAttached( (SceneNode*)0 );
        itor = efficientVectorRemove( mAttachments, itor );

        if( itor != mAttachments.end() )
        {
            //The object that was at the end got swapped and has now a different index
            (*itor)->mParentIndex = itor - mAttachments.begin();
        }
    }
    //-----------------------------------------------------------------------
    void SceneNode::detachAllObjects(void)
    {
        ObjectVec::iterator itr;
        for ( itr = mAttachments.begin(); itr != mAttachments.end(); ++itr )
            (*itr)->_notifyAttached( (SceneNode*)0 );
        mAttachments.clear();
    }
    //-----------------------------------------------------------------------
    void SceneNode::_attachBone( SkeletonInstance *skeletonInstance, Bone *bone )
    {
        mBoneChildren[skeletonInstance].push_back( bone );
    }
    //-----------------------------------------------------------------------
    void SceneNode::_detachBone( SkeletonInstance *skeletonInstance, Bone *bone )
    {
        BoneVec::iterator itor = std::find( mBoneChildren[skeletonInstance].begin(),
                                            mBoneChildren[skeletonInstance].end(),
                                            bone );

        assert( itor != mBoneChildren[skeletonInstance].end() );

        efficientVectorRemove( mBoneChildren[skeletonInstance], itor );
    }
    //-----------------------------------------------------------------------
    void SceneNode::_detachAllBones( SkeletonInstance *skeletonInstance )
    {
        BonesPerSkeletonInstance::iterator itor = mBoneChildren.find( skeletonInstance );
        if( itor != mBoneChildren.end() )
            mBoneChildren.erase( itor );
    }
    //-----------------------------------------------------------------------
    void SceneNode::detachAllBones(void)
    {
        BonesPerSkeletonInstance::const_iterator itSkeletons = mBoneChildren.begin();
        BonesPerSkeletonInstance::const_iterator enSkeletons = mBoneChildren.end();

        while( itSkeletons != enSkeletons )
        {
            BoneVec::const_iterator itBone = itSkeletons->second.begin();
            BoneVec::const_iterator enBone = itSkeletons->second.end();

            while( itBone != enBone )
            {
                itSkeletons->first->setSceneNodeAsParentOfBone( *itBone, 0 );
                ++itBone;
            }

            ++itSkeletons;
        }

        mBoneChildren.clear();
    }
    //-----------------------------------------------------------------------
    void SceneNode::_callMemoryChangeListeners(void)
    {
        ObjectVec::iterator itor = mAttachments.begin();
        ObjectVec::iterator end  = mAttachments.end();

        while( itor != end )
        {
            (*itor)->_notifyParentNodeMemoryChanged();
            ++itor;
        }

        BonesPerSkeletonInstance::const_iterator itSkeletons = mBoneChildren.begin();
        BonesPerSkeletonInstance::const_iterator enSkeletons = mBoneChildren.end();

        while( itSkeletons != enSkeletons )
        {
            BoneVec::const_iterator itBone = itSkeletons->second.begin();
            BoneVec::const_iterator enBone = itSkeletons->second.end();

            while( itBone != enBone )
            {
                (*itBone)->_setNodeParent( this );
                ++itBone;
            }

            ++itSkeletons;
        }
    }
    /*TODO
    Node::DebugRenderable* SceneNode::getDebugRenderable()
    {
        Vector3 hs = mWorldAABB.getHalfSize();
        Real sz = std::min(hs.x, hs.y);
        sz = std::min(sz, hs.z);
        sz = std::max(sz, (Real)1.0);
        return Node::getDebugRenderable(sz);
    }*/
    //-----------------------------------------------------------------------
    Node* SceneNode::createChildImpl( SceneMemoryMgrTypes sceneType )
    {
        assert(mCreator);

        NodeMemoryManager *nodeMemoryManager = mNodeMemoryManager;
        if( mNodeMemoryManager->getTwin() && mNodeMemoryManager->getMemoryManagerType() != sceneType )
            nodeMemoryManager = mNodeMemoryManager->getTwin();
        return mCreator->_createSceneNode( this, nodeMemoryManager );
    }
    //-----------------------------------------------------------------------
    SceneNode::ObjectIterator SceneNode::getAttachedObjectIterator(void)
    {
        return ObjectIterator( mAttachments.begin(), mAttachments.end() );
    }
    //-----------------------------------------------------------------------
    SceneNode::ConstObjectIterator SceneNode::getAttachedObjectIterator(void) const
    {
        return ConstObjectIterator( mAttachments.begin(), mAttachments.end() );
    }
    //-----------------------------------------------------------------------
    void SceneNode::removeAndDestroyChild( SceneNode *sceneNode )
    {
        assert( sceneNode->getParent() == this );
        sceneNode->removeAndDestroyAllChildren();

        removeChild( sceneNode );
        sceneNode->getCreator()->destroySceneNode( sceneNode );
    }
    //-----------------------------------------------------------------------
    void SceneNode::removeAndDestroyAllChildren(void)
    {
        NodeVec::iterator itor = mChildren.begin();
        NodeVec::iterator end  = mChildren.end();
        while( itor != end )
        {
            SceneNode *sceneNode = static_cast<SceneNode*>( *itor );
            sceneNode->removeAndDestroyAllChildren();
            sceneNode->unsetParent();
            mCreator->destroySceneNode( sceneNode );
            ++itor;
        }

        mChildren.clear();
    }
    //-----------------------------------------------------------------------
    SceneNode* SceneNode::createChildSceneNode( SceneMemoryMgrTypes sceneType,
                                                const Vector3& inTranslate,
                                                const Quaternion& inRotate )
    {
        return static_cast<SceneNode*>(this->createChild(sceneType, inTranslate, inRotate));
    }
    //-----------------------------------------------------------------------
    void SceneNode::setListener( Listener* listener )
    {
        Listener *oldListener = mListener;
        Node::setListener( listener );

        //Un/Register ourselves as a listener in the scene manager
        if( oldListener )
            mCreator->unregisterSceneNodeListener( this );
        if( mListener )
            mCreator->registerSceneNodeListener( this );
    }
    //-----------------------------------------------------------------------
	void SceneNode::setAutoTracking( bool enabled, SceneNode* const target,
									 const Vector3& localDirectionVector, const Vector3& offset )
    {
		assert( mCreator && "Auto-Tracking only works with SceneNodes created by a SceneManager" );
		if( enabled )
			mCreator->_addAutotrackingSceneNode( this, target, offset, localDirectionVector );
		else
			mCreator->_removeAutotrackingSceneNode( this );
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

        _updateFromParent();

        // The direction we want the local direction point to
        Vector3 targetDir = vec.normalisedCopy();

        const bool inheritOrientation = mTransform.mInheritOrientation[mTransform.mIndex];

        // Transform target direction to world space
        switch (relativeTo)
        {
        case TS_PARENT:
            if( inheritOrientation )
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
            Vector3 yawAxis = mYawFixedAxis;

            if (inheritOrientation && mParent) {
                yawAxis = mParent->_getDerivedOrientation() * yawAxis;
            }

            Vector3 xVec = yawAxis.crossProduct(targetDir);
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
        if( mParent && inheritOrientation )
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
            mTransform.mPosition->getAsVector3( origin, mTransform.mIndex );
            break;
        case TS_LOCAL:
            origin = Vector3::ZERO;
            break;
        }

        setDirection( targetPoint - origin, relativeTo, localDirectionVector );
	}
    //-----------------------------------------------------------------------
    SceneNode* SceneNode::getParentSceneNode(void) const
    {
        return static_cast<SceneNode*>(getParent());
    }
    //-----------------------------------------------------------------------
    void SceneNode::setVisible( bool visible, bool cascade )
    {
        ObjectVec::iterator itor = mAttachments.begin();
        ObjectVec::iterator end  = mAttachments.end();

        while( itor != end )
        {
            (*itor)->setVisible( visible );
            ++itor;
        }

        if (cascade)
        {
            NodeVec::iterator childItor = mChildren.begin();
            NodeVec::iterator childItorEnd  = mChildren.end();
            while( childItor != childItorEnd )
            {
                static_cast<SceneNode*>( *childItor )->setVisible( visible, cascade );
                ++childItor;
            }
        }
    }
    //-----------------------------------------------------------------------
    void SceneNode::flipVisibility(bool cascade)
    {
        ObjectVec::iterator itor = mAttachments.begin();
        ObjectVec::iterator end  = mAttachments.end();

        while( itor != end )
        {
            (*itor)->setVisible( !(*itor)->getVisible() );
            ++itor;
        }

        if (cascade)
        {
            NodeVec::iterator childItor = mChildren.begin();
            NodeVec::iterator childItorEnd  = mChildren.end();
            while( childItor != childItorEnd )
            {
                static_cast<SceneNode*>( *childItor )->flipVisibility( cascade );
                ++childItor;
            }
        }
    }
    //-----------------------------------------------------------------------
    NodeMemoryManager* SceneNode::getDefaultNodeMemoryManager( SceneMemoryMgrTypes sceneType )
    {
        return &mCreator->_getNodeMemoryManager( sceneType );
    }
    //-----------------------------------------------------------------------
#if OGRE_DEBUG_MODE
    void SceneNode::_setCachedTransformOutOfDate(void)
    {
        Node::_setCachedTransformOutOfDate();

        ObjectVec::const_iterator itor = mAttachments.begin();
        ObjectVec::const_iterator end  = mAttachments.end();

        while( itor != end )
        {
            (*itor)->_setCachedAabbOutOfDate();
            ++itor;
        }

        BonesPerSkeletonInstance::const_iterator itSkeletons = mBoneChildren.begin();
        BonesPerSkeletonInstance::const_iterator enSkeletons = mBoneChildren.end();

        while( itSkeletons != enSkeletons )
        {
            BoneVec::const_iterator itBone = itSkeletons->second.begin();
            BoneVec::const_iterator enBone = itSkeletons->second.end();

            while( itBone != enBone )
            {
                (*itBone)->_setCachedTransformOutOfDate();
                ++itBone;
            }

            ++itSkeletons;
        }
    }
#endif
}
