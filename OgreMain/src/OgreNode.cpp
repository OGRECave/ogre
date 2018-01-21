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
#include "OgreNode.h"

#include "OgreCamera.h"
#include "OgreException.h"
#include "OgreMath.h"
#include "OgreStringConverter.h"

#include "Math/Array/OgreNodeMemoryManager.h"
#include "Math/Array/OgreBooleanMask.h"

#if OGRE_DEBUG_MODE
    #define CACHED_TRANSFORM_OUT_OF_DATE() this->_setCachedTransformOutOfDate()
#else
    #define CACHED_TRANSFORM_OUT_OF_DATE() ((void)0)
#endif

namespace Ogre {
    //-----------------------------------------------------------------------
    Node::Node( IdType id, NodeMemoryManager *nodeMemoryManager, Node *parent ) :
        IdObject( id ),
        mDepthLevel( 0 ),
        mIndestructibleByClearScene( false ),
        mParent( parent ),
		mName( "" ),
#if OGRE_DEBUG_MODE
        mCachedTransformOutOfDate( true ),
#endif
        mListener( 0 ),
        mNodeMemoryManager( nodeMemoryManager ),
        mGlobalIndex( -1 ),
        mParentIndex( -1 )
    {
        if( mParent )
            mDepthLevel = mParent->mDepthLevel + 1;

        //Will initialize mTransform
        mNodeMemoryManager->nodeCreated( mTransform, mDepthLevel );
        mTransform.mOwner[mTransform.mIndex] = this;
        if( mParent )
            mTransform.mParents[mTransform.mIndex] = mParent;
    }
    //-----------------------------------------------------------------------
    Node::Node( const Transform &transformPtrs ) :
        IdObject( 0 ),
        mDepthLevel( 0 ),
        mParent( 0 ),
		mName( "Dummy Node" ),
#if OGRE_DEBUG_MODE
        mCachedTransformOutOfDate( true ),
#endif
        mListener( 0 ),
        mNodeMemoryManager( 0 ),
        mGlobalIndex( -1 ),
        mParentIndex( -1 )
    {
        mTransform = transformPtrs;
    }
    //-----------------------------------------------------------------------
    Node::~Node()
    {
        // Call listener (note, only called if there's something to do)
        if( mListener )
        {
            mListener->nodeDestroyed(this);
        }

        //Calling removeAllChildren or mParent->removeChild before nodeDestroyed may
        //result in an enlargement of the array memory buffer or in a cleanup.
        //Either way, the memory manager could end up calling a virtual function;
        //but our vtable is wrong by now. So we destroy our slot first.
        if( mNodeMemoryManager )
            mNodeMemoryManager->nodeDestroyed( mTransform, mDepthLevel );
        mDepthLevel = 0;

        removeAllChildren();
        if( mParent )
        {
            Node *parent = mParent;
            mParent = 0; //We've already called mNodeMemoryManager->nodeDestroyed.
            parent->removeChild( this );
        }
    }
    //-----------------------------------------------------------------------
    Node* Node::getParent(void) const
    {
        return mParent;
    }
    //-----------------------------------------------------------------------
    void Node::setIndestructibleByClearScene( bool indestructible )
    {
        mIndestructibleByClearScene = indestructible;
    }
    //-----------------------------------------------------------------------
    bool Node::getIndestructibleByClearScene(void) const
    {
        return mIndestructibleByClearScene;
    }
    //-----------------------------------------------------------------------
    void Node::migrateTo( NodeMemoryManager *nodeMemoryManager )
    {
        NodeVec::const_iterator itor = mChildren.begin();
        NodeVec::const_iterator end  = mChildren.end();

        while( itor != end )
        {
            (*itor)->migrateTo( nodeMemoryManager );
            ++itor;
        }

        mNodeMemoryManager->migrateTo( mTransform, mDepthLevel, nodeMemoryManager );
        mNodeMemoryManager = nodeMemoryManager;
        _callMemoryChangeListeners();
    }
    //-----------------------------------------------------------------------
    bool Node::isStatic() const
    {
        return mNodeMemoryManager->getMemoryManagerType() == SCENE_STATIC;
    }
    //-----------------------------------------------------------------------
    bool Node::setStatic( bool bStatic )
    {
        bool retVal = false;
        if( mNodeMemoryManager->getTwin() &&
           ((mNodeMemoryManager->getMemoryManagerType() == SCENE_STATIC && !bStatic) ||
            (mNodeMemoryManager->getMemoryManagerType() == SCENE_DYNAMIC && bStatic)))
        {
            mNodeMemoryManager->migrateTo( mTransform, mDepthLevel, mNodeMemoryManager->getTwin() );
            mNodeMemoryManager = mNodeMemoryManager->getTwin();
            _callMemoryChangeListeners();
            retVal = true;
        }

        return retVal;
    }
    //-----------------------------------------------------------------------
    void Node::setParent( Node* parent )
    {
        if( mParent != parent )
        {
            mParent = parent;

            // Call listener
            if( mListener )
                mListener->nodeAttached(this);

            size_t oldDepthLevel = mDepthLevel;

            //NodeMemoryManager will set mTransform.mParents to a dummy parent node
            //(as well as transfering the memory)
            mNodeMemoryManager->nodeDettached( mTransform, mDepthLevel );

            mDepthLevel = mParent->mDepthLevel + 1;
            mTransform.mParents[mTransform.mIndex] = parent;

            const bool differentNodeMemoryManager = mParent->mNodeMemoryManager != mNodeMemoryManager;

            if( differentNodeMemoryManager )
            {
                mNodeMemoryManager->migrateToAndAttach( mTransform, mDepthLevel,
                                                        mParent->mNodeMemoryManager );
                mNodeMemoryManager = mParent->mNodeMemoryManager;
            }
            else
            {
                mNodeMemoryManager->nodeAttached( mTransform, mDepthLevel );
            }

            if( oldDepthLevel != mDepthLevel || differentNodeMemoryManager )
            {
                //Propagate the change to our children
                NodeVec::const_iterator itor = mChildren.begin();
                NodeVec::const_iterator end  = mChildren.end();

                while( itor != end )
                {
                    (*itor)->parentDepthLevelChanged();
                    ++itor;
                }

                _callMemoryChangeListeners();
            }
        }
    }
    //-----------------------------------------------------------------------
    void Node::unsetParent(void)
    {
        if( mParent )
        {
            // Call listener
            if( mListener )
                mListener->nodeDetached(this);

            mParent = 0;

            NodeMemoryManager *defaultNodeMemoryManager =
                    getDefaultNodeMemoryManager( mNodeMemoryManager->getMemoryManagerType() );

            const bool differentNodeMemoryManager = defaultNodeMemoryManager != mNodeMemoryManager;

            if( differentNodeMemoryManager )
            {
                mNodeMemoryManager->migrateToAndDetach( mTransform, mDepthLevel,
                                                        defaultNodeMemoryManager );
                mNodeMemoryManager = defaultNodeMemoryManager;
            }
            else
            {
                //NodeMemoryManager will set mTransform.mParents to a dummy parent node
                //(as well as transfering the memory)
                mNodeMemoryManager->nodeDettached( mTransform, mDepthLevel );
            }

            if( mDepthLevel != 0 || differentNodeMemoryManager )
            {
                mDepthLevel = 0;

                //Propagate the change to our children
                NodeVec::const_iterator itor = mChildren.begin();
                NodeVec::const_iterator end  = mChildren.end();

                while( itor != end )
                {
                    (*itor)->parentDepthLevelChanged();
                    ++itor;
                }

                _callMemoryChangeListeners();
            }
        }
    }
    //-----------------------------------------------------------------------
    void Node::parentDepthLevelChanged(void)
    {
        if( mNodeMemoryManager != mParent->mNodeMemoryManager )
        {
            mNodeMemoryManager->migrateTo( mTransform, mDepthLevel, mParent->mDepthLevel + 1,
                                           mParent->mNodeMemoryManager );
            mNodeMemoryManager = mParent->mNodeMemoryManager;
        }
        else
        {
            mNodeMemoryManager->nodeMoved( mTransform, mDepthLevel, mParent->mDepthLevel + 1 );
        }

        mDepthLevel = mParent->mDepthLevel + 1;

        _callMemoryChangeListeners();

        //Keep propagating changes to our children
        NodeVec::const_iterator itor = mChildren.begin();
        NodeVec::const_iterator end  = mChildren.end();

        while( itor != end )
        {
            (*itor)->parentDepthLevelChanged();
            ++itor;
        }
    }
    //-----------------------------------------------------------------------
    /*const Matrix4& Node::_getFullTransform(void) const
    {
        assert( !mCachedTransformOutOfDate );
        return mTransform.mDerivedTransform[mTransform.mIndex];
    }*/
    //-----------------------------------------------------------------------
    const Matrix4& Node::_getFullTransformUpdated(void)
    {
        _updateFromParent();
        return mTransform.mDerivedTransform[mTransform.mIndex];
    }
    //-----------------------------------------------------------------------
    void Node::_updateFromParent(void)
    {
        if( mParent )
            mParent->_updateFromParent();

        updateFromParentImpl();

        // Call listener (note, this method only called if there's something to do)
        if (mListener)
        {
            mListener->nodeUpdated(this);
        }
    }
    //-----------------------------------------------------------------------
    void Node::updateFromParentImpl(void)
    {
        //Retrieve from parents. Unfortunately we need to do SoA -> AoS -> SoA conversion
        ArrayVector3 parentPos, parentScale;
        ArrayQuaternion parentRot;

        for( size_t j=0; j<ARRAY_PACKED_REALS; ++j )
        {
            Vector3 pos, nodeScale;
            Quaternion qRot;
            const Transform &parentTransform = mTransform.mParents[j]->mTransform;
            parentTransform.mDerivedPosition->getAsVector3( pos, parentTransform.mIndex );
            parentTransform.mDerivedOrientation->getAsQuaternion( qRot, parentTransform.mIndex );
            parentTransform.mDerivedScale->getAsVector3( nodeScale, parentTransform.mIndex );

            parentPos.setFromVector3( pos, j );
            parentRot.setFromQuaternion( qRot, j );
            parentScale.setFromVector3( nodeScale, j );
        }

        parentRot.Cmov4( BooleanMask4::getMask( mTransform.mInheritOrientation ),
                         ArrayQuaternion::IDENTITY );
        parentScale.Cmov4( BooleanMask4::getMask( mTransform.mInheritScale ),
                            ArrayVector3::UNIT_SCALE );

        // Scale own position by parent scale, NB just combine
        // as equivalent axes, no shearing
        *mTransform.mDerivedScale = parentScale * (*mTransform.mScale);

        // Combine orientation with that of parent
        *mTransform.mDerivedOrientation = parentRot * (*mTransform.mOrientation);

        // Change position vector based on parent's orientation & scale
        *mTransform.mDerivedPosition = parentRot * (parentScale * (*mTransform.mPosition));

        // Add altered position vector to parents
        *mTransform.mDerivedPosition += parentPos;

        ArrayMatrix4 derivedTransform;
        derivedTransform.makeTransform( *mTransform.mDerivedPosition,
                                         *mTransform.mDerivedScale,
                                         *mTransform.mDerivedOrientation );
        derivedTransform.storeToAoS( mTransform.mDerivedTransform );
#if OGRE_DEBUG_MODE
        for( size_t j=0; j<ARRAY_PACKED_REALS; ++j )
        {
            if( mTransform.mOwner[j] )
                mTransform.mOwner[j]->mCachedTransformOutOfDate = false;
        }
#endif
    }
    //-----------------------------------------------------------------------
    void Node::updateAllTransforms( const size_t numNodes, Transform t )
    {
        ArrayMatrix4 derivedTransform;
        for( size_t i=0; i<numNodes; i += ARRAY_PACKED_REALS )
        {
            //Retrieve from parents. Unfortunately we need to do SoA -> AoS -> SoA conversion
            ArrayVector3 parentPos, parentScale;
            ArrayQuaternion parentRot;

            for( size_t j=0; j<ARRAY_PACKED_REALS; ++j )
            {
                Vector3 pos, scale;
                Quaternion qRot;
                const Transform &parentTransform = t.mParents[j]->mTransform;
                parentTransform.mDerivedPosition->getAsVector3( pos, parentTransform.mIndex );
                parentTransform.mDerivedOrientation->getAsQuaternion( qRot, parentTransform.mIndex );
                parentTransform.mDerivedScale->getAsVector3( scale, parentTransform.mIndex );

                parentPos.setFromVector3( pos, j );
                parentRot.setFromQuaternion( qRot, j );
                parentScale.setFromVector3( scale, j );
            }

            // Change position vector based on parent's orientation & scale
            *t.mDerivedPosition = parentRot * (parentScale * (*t.mPosition));

            // Combine orientation with that of parent
            *t.mDerivedOrientation = ArrayQuaternion::Cmov4( parentRot * (*t.mOrientation),
                                                             *t.mOrientation,
                                                        BooleanMask4::getMask( t.mInheritOrientation ) );

            // Scale own position by parent scale, NB just combine
            // as equivalent axes, no shearing
            *t.mDerivedScale = ArrayVector3::Cmov4( parentScale * (*t.mScale), *t.mScale,
                                                    BooleanMask4::getMask( t.mInheritScale ) );

            // Add altered position vector to parents
            *t.mDerivedPosition += parentPos;

            derivedTransform.makeTransform( *t.mDerivedPosition,
                                            *t.mDerivedScale,
                                            *t.mDerivedOrientation );
            derivedTransform.storeToAoS( t.mDerivedTransform );
#if OGRE_DEBUG_MODE
            for( size_t j=0; j<ARRAY_PACKED_REALS; ++j )
            {
                if( t.mOwner[j] )
                    t.mOwner[j]->mCachedTransformOutOfDate = false;
            }
#endif

            t.advancePack();
        }
    }
    //-----------------------------------------------------------------------
    Node* Node::createChild( SceneMemoryMgrTypes sceneType,
                             const Vector3& inTranslate, const Quaternion& inRotate )
    {
        Node* newNode = createChildImpl( sceneType );
        newNode->setPosition( inTranslate );
        newNode->setOrientation( inRotate );

        //createChildImpl must have passed us as parent. It's a special
        //case to improve memory usage (avoid transfering mTransform)
        mChildren.push_back( newNode );
        newNode->mParentIndex = mChildren.size() - 1;

        return newNode;
    }
    //-----------------------------------------------------------------------
    void Node::addChild(Node* child)
    {
        if( child->mParent )
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "Node ID: " + StringConverter::toString( child->getId() ) + ", named '" +
                child->getName() + "' already was a child of Node ID: " +
                StringConverter::toString( child->mParent->getId() ) + ", named '" +
                child->mParent->getName() + "'.", "Node::addChild");
        }

        mChildren.push_back( child );
        child->mParentIndex = mChildren.size() - 1;
        child->setParent(this);
    }
    //-----------------------------------------------------------------------
    void Node::removeChild( Node* child )
    {
        //child->getParent() can be null if we're calling this
        //function from the destructor. See ~Node notes.
        assert( (!child->getParent() || child->getParent() == this) && "Node says it's not our child" );
        assert( child->mParentIndex < mChildren.size() && "mParentIndex was out of date!!!" );

        if( child->mParentIndex < mChildren.size() )
        {
            NodeVec::iterator itor = mChildren.begin() + child->mParentIndex;

            assert( child == *itor && "mParentIndex was out of date!!!" );

            if( child == *itor )
            {
                itor = efficientVectorRemove( mChildren, itor );
                child->unsetParent();
                child->mParentIndex = -1;

                //The node that was at the end got swapped and has now a different index
                if( itor != mChildren.end() )
                    (*itor)->mParentIndex = itor - mChildren.begin();
            }
        }
    }
    //-----------------------------------------------------------------------
    void Node::_notifyStaticDirty(void) const
    {
        NodeVec::const_iterator itor = mChildren.begin();
        NodeVec::const_iterator end  = mChildren.end();

        while( itor != end )
        {
            (*itor)->_notifyStaticDirty();
            ++itor;
        }
    }
    //-----------------------------------------------------------------------
    Quaternion Node::getOrientation() const
    {
        return mTransform.mOrientation->getAsQuaternion( mTransform.mIndex );
    }
    //-----------------------------------------------------------------------
    void Node::setOrientation( Quaternion q )
    {
        assert(!q.isNaN() && "Invalid orientation supplied as parameter");
        q.normalise();
        mTransform.mOrientation->setFromQuaternion( q, mTransform.mIndex );
        CACHED_TRANSFORM_OUT_OF_DATE();
    }
    //-----------------------------------------------------------------------
    void Node::setOrientation( Real w, Real x, Real y, Real z )
    {
        setOrientation( Quaternion(w, x, y, z) );
    }
    //-----------------------------------------------------------------------
    void Node::resetOrientation(void)
    {
        mTransform.mOrientation->setFromQuaternion( Quaternion::IDENTITY, mTransform.mIndex );
    }

    //-----------------------------------------------------------------------
    void Node::setPosition(const Vector3& pos)
    {
        assert(!pos.isNaN() && "Invalid vector supplied as parameter");
        mTransform.mPosition->setFromVector3( pos, mTransform.mIndex );
        CACHED_TRANSFORM_OUT_OF_DATE();
    }
    //-----------------------------------------------------------------------
    void Node::setPosition(Real x, Real y, Real z)
    {
        setPosition( Vector3( x, y, z ) );
    }
    //-----------------------------------------------------------------------
    Vector3 Node::getPosition(void) const
    {
        return mTransform.mPosition->getAsVector3( mTransform.mIndex );
    }
    //-----------------------------------------------------------------------
    Matrix3 Node::getLocalAxes(void) const
    {
        Quaternion q;
        mTransform.mOrientation->getAsQuaternion( q, mTransform.mIndex );
        Matrix3 retVal;
        q.ToRotationMatrix( retVal );

        /* Equivalent code (easier to visualize):
        axisX = q.xAxis();
        axisY = q.yAxis();
        axisZ = q.zAxis();

        return Matrix3(axisX.x, axisY.x, axisZ.x,
                       axisX.y, axisY.y, axisZ.y,
                       axisX.z, axisY.z, axisZ.z);*/

        return retVal;
    }

    //-----------------------------------------------------------------------
    void Node::translate(const Vector3& d, TransformSpace relativeTo)
    {
        Vector3 position;
        mTransform.mPosition->getAsVector3( position, mTransform.mIndex );

        switch(relativeTo)
        {
        case TS_LOCAL:
            // position is relative to parent so transform downwards
            position += mTransform.mOrientation->getAsQuaternion( mTransform.mIndex ) * d;
            break;
        case TS_WORLD:
            // position is relative to parent so transform upwards
            if (mParent)
            {
                position += (mParent->_getDerivedOrientation().Inverse() * d)
                    / mParent->_getDerivedScale();
            }
            else
            {
                position += d;
            }
            break;
        case TS_PARENT:
            position += d;
            break;
        }

        mTransform.mPosition->setFromVector3( position, mTransform.mIndex );
        CACHED_TRANSFORM_OUT_OF_DATE();
    }
    //-----------------------------------------------------------------------
    void Node::translate(Real x, Real y, Real z, TransformSpace relativeTo)
    {
        Vector3 v(x,y,z);
        translate(v, relativeTo);
    }
    //-----------------------------------------------------------------------
    void Node::translate(const Matrix3& axes, const Vector3& move, TransformSpace relativeTo)
    {
        Vector3 derived = axes * move;
        translate(derived, relativeTo);
    }
    //-----------------------------------------------------------------------
    void Node::translate(const Matrix3& axes, Real x, Real y, Real z, TransformSpace relativeTo)
    {
        Vector3 d(x,y,z);
        translate(axes,d,relativeTo);
    }
    //-----------------------------------------------------------------------
    void Node::roll(const Radian& angle, TransformSpace relativeTo)
    {
        rotate(Vector3::UNIT_Z, angle, relativeTo);
    }
    //-----------------------------------------------------------------------
    void Node::pitch(const Radian& angle, TransformSpace relativeTo)
    {
        rotate(Vector3::UNIT_X, angle, relativeTo);
    }
    //-----------------------------------------------------------------------
    void Node::yaw(const Radian& angle, TransformSpace relativeTo)
    {
        rotate(Vector3::UNIT_Y, angle, relativeTo);

    }
    //-----------------------------------------------------------------------
    void Node::rotate(const Vector3& axis, const Radian& angle, TransformSpace relativeTo)
    {
        Quaternion q;
        q.FromAngleAxis(angle,axis);
        rotate(q, relativeTo);
    }

    //-----------------------------------------------------------------------
    void Node::rotate(const Quaternion& q, TransformSpace relativeTo)
    {
        // Normalise quaternion to avoid drift
        Quaternion qnorm = q;
        qnorm.normalise();

        Quaternion orientation;
        mTransform.mOrientation->getAsQuaternion( orientation, mTransform.mIndex );

        switch(relativeTo)
        {
        case TS_PARENT:
            // Rotations are normally relative to local axes, transform up
            orientation = qnorm * orientation;
            break;
        case TS_WORLD:
            // Rotations are normally relative to local axes, transform up
            orientation = orientation * _getDerivedOrientation().Inverse()
                * qnorm * _getDerivedOrientation();
            break;
        case TS_LOCAL:
            // Note the order of the mult, i.e. q comes after
            orientation = orientation * qnorm;
            break;
        }

        //It should be already normalized, but floating point can cause
        //successive calls to rotate to accumulate error.
        orientation.normalise();

        mTransform.mOrientation->setFromQuaternion( orientation, mTransform.mIndex );
        CACHED_TRANSFORM_OUT_OF_DATE();
    }

    
    //-----------------------------------------------------------------------
    void Node::_setDerivedPosition( const Vector3& pos )
    {
        //find where the node would end up in parent's local space
        if(mParent)
            setPosition( mParent->convertWorldToLocalPosition( pos ) );
    }
    //-----------------------------------------------------------------------
    void Node::_setDerivedOrientation( const Quaternion& q )
    {
        //find where the node would end up in parent's local space
        if(mParent)
            setOrientation( mParent->convertWorldToLocalOrientation( q ) );
    }

    //-----------------------------------------------------------------------
    Quaternion Node::_getDerivedOrientation(void) const
    {
#if OGRE_DEBUG_MODE
        assert( !mCachedTransformOutOfDate );
#endif
        return mTransform.mDerivedOrientation->getAsQuaternion( mTransform.mIndex );
    }
    //-----------------------------------------------------------------------
    Quaternion Node::_getDerivedOrientationUpdated(void)
    {
        _updateFromParent();
        return mTransform.mDerivedOrientation->getAsQuaternion( mTransform.mIndex );
    }
    //-----------------------------------------------------------------------
    Vector3 Node::_getDerivedPosition(void) const
    {
#if OGRE_DEBUG_MODE
        assert( !mCachedTransformOutOfDate );
#endif
        return mTransform.mDerivedPosition->getAsVector3( mTransform.mIndex );
    }
    //-----------------------------------------------------------------------
    Vector3 Node::_getDerivedPositionUpdated(void)
    {
        _updateFromParent();
        return mTransform.mDerivedPosition->getAsVector3( mTransform.mIndex );
    }
    //-----------------------------------------------------------------------
    Vector3 Node::_getDerivedScale(void) const
    {
#if OGRE_DEBUG_MODE
        assert( !mCachedTransformOutOfDate );
#endif
        return mTransform.mDerivedScale->getAsVector3( mTransform.mIndex );
    }
    //-----------------------------------------------------------------------
    Vector3 Node::_getDerivedScaleUpdated(void)
    {
        _updateFromParent();
        return mTransform.mDerivedScale->getAsVector3( mTransform.mIndex );
    }
    //-----------------------------------------------------------------------
    Vector3 Node::convertWorldToLocalPosition( const Vector3 &worldPos )
    {
#if OGRE_DEBUG_MODE
        assert( !mCachedTransformOutOfDate );
#endif
        ArrayVector3 arrayWorldPos;
        arrayWorldPos.setAll( worldPos );
        arrayWorldPos = mTransform.mDerivedOrientation->Inverse() *
                            (arrayWorldPos - (*mTransform.mDerivedPosition)) /
                            (*mTransform.mDerivedScale);

        Vector3 retVal;
        arrayWorldPos.getAsVector3( retVal, mTransform.mIndex );
        return retVal;
    }
    //-----------------------------------------------------------------------
    Vector3 Node::convertLocalToWorldPosition( const Vector3 &localPos )
    {
#if OGRE_DEBUG_MODE
        assert( !mCachedTransformOutOfDate );
#endif
        ArrayVector3 arrayLocalPos;
        arrayLocalPos.setAll( localPos );
        arrayLocalPos = ( (*mTransform.mDerivedOrientation) *
                            (arrayLocalPos * (*mTransform.mDerivedScale)) ) +
                            (*mTransform.mDerivedPosition);

        Vector3 retVal;
        arrayLocalPos.getAsVector3( retVal, mTransform.mIndex );
        return retVal;
    }
    //-----------------------------------------------------------------------
    Quaternion Node::convertWorldToLocalOrientation( const Quaternion &worldOrientation )
    {
#if OGRE_DEBUG_MODE
        assert( !mCachedTransformOutOfDate );
#endif
        return mTransform.mDerivedOrientation->getAsQuaternion( mTransform.mIndex ).Inverse() *
                worldOrientation;
    }
    //-----------------------------------------------------------------------
    Quaternion Node::convertLocalToWorldOrientation( const Quaternion &localOrientation )
    {
#if OGRE_DEBUG_MODE
        assert( !mCachedTransformOutOfDate );
#endif
        return mTransform.mDerivedOrientation->getAsQuaternion( mTransform.mIndex ) * localOrientation;
    }
    //-----------------------------------------------------------------------
    void Node::removeAllChildren(void)
    {
        NodeVec::iterator itor = mChildren.begin();
        NodeVec::iterator end  = mChildren.end();
        while( itor != end )
        {
            (*itor)->unsetParent();
            (*itor)->mParentIndex = -1;
            ++itor;
        }
        mChildren.clear();
    }
    //-----------------------------------------------------------------------
    void Node::setScale(const Vector3& inScale)
    {
        assert(!inScale.isNaN() && "Invalid vector supplied as parameter");
        mTransform.mScale->setFromVector3( inScale, mTransform.mIndex );
        CACHED_TRANSFORM_OUT_OF_DATE();
    }
    //-----------------------------------------------------------------------
    void Node::setScale(Real x, Real y, Real z)
    {
        setScale(Vector3(x, y, z));
    }
    //-----------------------------------------------------------------------
    Vector3 Node::getScale(void) const
    {
        return mTransform.mScale->getAsVector3( mTransform.mIndex );
    }
    //-----------------------------------------------------------------------
    void Node::setInheritOrientation(bool inherit)
    {
        mTransform.mInheritOrientation[mTransform.mIndex] = inherit;
        CACHED_TRANSFORM_OUT_OF_DATE();
    }
    //-----------------------------------------------------------------------
    bool Node::getInheritOrientation(void) const
    {
        return mTransform.mInheritOrientation[mTransform.mIndex];
    }
    //-----------------------------------------------------------------------
    void Node::setInheritScale(bool inherit)
    {
        mTransform.mInheritScale[mTransform.mIndex] = inherit;
        CACHED_TRANSFORM_OUT_OF_DATE();
    }
    //-----------------------------------------------------------------------
    bool Node::getInheritScale(void) const
    {
        return mTransform.mInheritScale[mTransform.mIndex];
    }
    //-----------------------------------------------------------------------
    void Node::scale(const Vector3& inScale)
    {
        mTransform.mScale->setFromVector3( mTransform.mScale->getAsVector3( mTransform.mIndex ) *
                                            inScale, mTransform.mIndex );
        CACHED_TRANSFORM_OUT_OF_DATE();
    }
    //-----------------------------------------------------------------------
    void Node::scale(Real x, Real y, Real z)
    {
        scale( Vector3( x, y, z ) );
	}
    //-----------------------------------------------------------------------
	Node::NodeVecIterator Node::getChildIterator(void)
    {
        return NodeVecIterator(mChildren.begin(), mChildren.end());
    }
    //-----------------------------------------------------------------------
    Node::ConstNodeVecIterator Node::getChildIterator(void) const
    {
        return ConstNodeVecIterator(mChildren.begin(), mChildren.end());
    }
    //-----------------------------------------------------------------------
    Real Node::getSquaredViewDepth(const Camera* cam) const
    {
        Vector3 diff = _getDerivedPosition() - cam->getDerivedPosition();

        // NB use squared length rather than real depth to avoid square root
        return diff.squaredLength();
    }
    //---------------------------------------------------------------------
#if OGRE_DEBUG_MODE
    void Node::_setCachedTransformOutOfDate(void)
    {
        mCachedTransformOutOfDate = true;

        NodeVec::const_iterator itor = mChildren.begin();
        NodeVec::const_iterator end  = mChildren.end();

        while( itor != end )
        {
            (*itor)->_setCachedTransformOutOfDate();
            ++itor;
        }
    }
#endif
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    /*Node::DebugRenderable::DebugRenderable(Node* parent)
        : mParent(parent)
    {
        String matName = "Ogre/Debug/AxesMat";
        mMat = MaterialManager::getSingleton().getByName(matName);
        if (mMat.isNull())
        {
            mMat = MaterialManager::getSingleton().create(matName, ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME);
            Pass* p = mMat->getTechnique(0)->getPass(0);
            p->setLightingEnabled(false);
            p->setPolygonModeOverrideable(false);
            p->setVertexColourTracking(TVC_AMBIENT);
            p->setSceneBlending(SBT_TRANSPARENT_ALPHA);
            p->setCullingMode(CULL_NONE);
            p->setDepthWriteEnabled(false);
        }

        String meshName = "Ogre/Debug/AxesMesh";
        mMeshPtr = MeshManager::getSingleton().getByName(meshName);
        if (mMeshPtr.isNull())
        {
            ManualObject mo( 0, 0 );
            mo.begin(mMat->getName());*/
            /* 3 axes, each made up of 2 of these (base plane = XY)
             *   .------------|\
             *   '------------|/
             *//*
            mo.estimateVertexCount(7 * 2 * 3);
            mo.estimateIndexCount(3 * 2 * 3);
            Quaternion quat[6];
            ColourValue col[3];

            // x-axis
            quat[0] = Quaternion::IDENTITY;
            quat[1].FromAxes(Vector3::UNIT_X, Vector3::NEGATIVE_UNIT_Z, Vector3::UNIT_Y);
            col[0] = ColourValue::Red;
            col[0].a = 0.8;
            // y-axis
            quat[2].FromAxes(Vector3::UNIT_Y, Vector3::NEGATIVE_UNIT_X, Vector3::UNIT_Z);
            quat[3].FromAxes(Vector3::UNIT_Y, Vector3::UNIT_Z, Vector3::UNIT_X);
            col[1] = ColourValue::Green;
            col[1].a = 0.8;
            // z-axis
            quat[4].FromAxes(Vector3::UNIT_Z, Vector3::UNIT_Y, Vector3::NEGATIVE_UNIT_X);
            quat[5].FromAxes(Vector3::UNIT_Z, Vector3::UNIT_X, Vector3::UNIT_Y);
            col[2] = ColourValue::Blue;
            col[2].a = 0.8;

            Vector3 basepos[7] = 
            {
                // stalk
                Vector3(0, 0.05, 0), 
                Vector3(0, -0.05, 0),
                Vector3(0.7, -0.05, 0),
                Vector3(0.7, 0.05, 0),
                // head
                Vector3(0.7, -0.15, 0),
                Vector3(1, 0, 0),
                Vector3(0.7, 0.15, 0)
            };


            // vertices
            // 6 arrows
            for (size_t i = 0; i < 6; ++i)
            {
                // 7 points
                for (size_t p = 0; p < 7; ++p)
                {
                    Vector3 pos = quat[i] * basepos[p];
                    mo.position(pos);
                    mo.colour(col[i / 2]);
                }
            }

            // indices
            // 6 arrows
            for (uint32 i = 0; i < 6; ++i)
            {
                uint32 base = i * 7;
                mo.triangle(base + 0, base + 1, base + 2);
                mo.triangle(base + 0, base + 2, base + 3);
                mo.triangle(base + 4, base + 5, base + 6);
            }

            mo.end();

            mMeshPtr = mo.convertToMesh(meshName, ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME);

        }

    }
    //---------------------------------------------------------------------
    Node::DebugRenderable::~DebugRenderable()
    {
    }
    //-----------------------------------------------------------------------
    const MaterialPtr& Node::DebugRenderable::getMaterial(void) const
    {
        return mMat;
    }
    //---------------------------------------------------------------------
    void Node::DebugRenderable::getRenderOperation(RenderOperation& op)
    {
        return mMeshPtr->getSubMesh(0)->_getRenderOperation(op);
    }
    //-----------------------------------------------------------------------
    void Node::DebugRenderable::getWorldTransforms(Matrix4* xform) const
    {
        // Assumes up to date
        *xform = mParent->_getFullTransform();
        if (!Math::RealEqual(mScaling, 1.0))
        {
            Matrix4 m = Matrix4::IDENTITY;
            Vector3 s(mScaling, mScaling, mScaling);
            m.setScale(s);
            *xform = (*xform) * m;
        }
    }
    //-----------------------------------------------------------------------
    Real Node::DebugRenderable::getSquaredViewDepth(const Camera* cam) const
    {
        return mParent->getSquaredViewDepth(cam);
    }
    //-----------------------------------------------------------------------
    const LightList& Node::DebugRenderable::getLights(void) const
    {
        // Nodes should not be lit by the scene, this will not get called
        static LightList ll;
        return ll;
    }*/
}

#undef CACHED_TRANSFORM_OUT_OF_DATE
