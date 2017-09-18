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
#include "Animation/OgreTagPoint.h"
#include "Animation/OgreBone.h"

#include "Math/Array/OgreBooleanMask.h"
#include "Math/Array/OgreNodeMemoryManager.h"
#include "OgreSceneManager.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    TagPoint::TagPoint( IdType id, SceneManager* creator, NodeMemoryManager *nodeMemoryManager,
                        SceneNode *parent ) :
        SceneNode( id, creator, nodeMemoryManager, parent ),
        mParentBone( 0 )
    {
    }
    //-----------------------------------------------------------------------
    TagPoint::~TagPoint()
    {
        if( mParentBone )
            mParentBone->removeTagPoint( this );
    }
    //-----------------------------------------------------------------------
    void TagPoint::_setParentBone( Bone *bone )
    {
        if( this->mParentBone )
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "Node ID: " + StringConverter::toString( this->getId() ) + ", named '" +
                this->getName() + "' already was a child of Bone ID: " +
                StringConverter::toString( this->mParentBone->getId() ) + ", named '" +
                this->mParentBone->getName() + "'.", "TagPoint::_setBoneParent");
        }

        if( this->mParent )
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "Node ID: " + StringConverter::toString( this->getId() ) + ", named '" +
                this->getName() + "' already was a child of Node ID: " +
                StringConverter::toString( this->mParent->getId() ) + ", named '" +
                this->mParent->getName() + "'.", "TagPoint::_setBoneParent");
        }

        mParentBone = bone;

        // Call listener
        if( mListener )
            mListener->nodeAttached(this);

        assert( mDepthLevel == 0 );

        NodeMemoryManager *nodeMemoryManager = &mCreator->_getTagPointNodeMemoryManager();
        if( mNodeMemoryManager != nodeMemoryManager )
            this->migrateTo( nodeMemoryManager );

        // Somewhat hacky way to mark our base classes that we can't be attached.
        mParent = mCreator->getDummySceneNode();
    }
    //-----------------------------------------------------------------------
    void TagPoint::_unsetParentBone(void)
    {
        if( mParentBone )
        {
            mParentBone = 0;
            this->unsetParent();
        }
    }
    //-----------------------------------------------------------------------
    Matrix3 TagPoint::_getDerivedOrientationMatrix(void) const
    {
#if OGRE_DEBUG_MODE
        assert( !mCachedTransformOutOfDate );
#endif
        Matrix3 retVal;
        mTransform.mDerivedTransform[mTransform.mIndex].extract3x3Matrix( retVal );
        return retVal;
    }
    //-----------------------------------------------------------------------
    void TagPoint::updateFromParentImpl(void)
    {
        assert( false && "Not implemented" );
        //I'm lazy, but before you implement it, remember that the skeleton needs to be updated as well.
    }
    //-----------------------------------------------------------------------
    void TagPoint::updateAllTransformsBoneToTag( const size_t numNodes, Transform t )
    {
        SimpleMatrixAf4x3 const * RESTRICT_ALIAS parentBoneParentNodeTransform[ARRAY_PACKED_REALS];
        SimpleMatrixAf4x3 const * RESTRICT_ALIAS parentBoneTransform[ARRAY_PACKED_REALS];

        for( size_t j=0; j<ARRAY_PACKED_REALS; ++j )
        {
            parentBoneParentNodeTransform[j]    = &SimpleMatrixAf4x3::IDENTITY;
            parentBoneTransform[j]              = &SimpleMatrixAf4x3::IDENTITY;
        }

        for( size_t i=0; i<numNodes; i += ARRAY_PACKED_REALS )
        {
            //Retrieve from parents. Unfortunately we need to do SoA -> AoS -> SoA conversion
            ArrayMatrixAf4x3 finalMat;
            ArrayMatrixAf4x3 parentBone;

            for( size_t j=0; j<ARRAY_PACKED_REALS; ++j )
            {
                if( t.mOwner[j] )
                {
                    Bone *parentBonePtr = static_cast<TagPoint*>( t.mOwner[j] )->mParentBone;
                    const BoneTransform &boneTransform = parentBonePtr->_getTransform();
                    parentBoneParentNodeTransform[j] =
                            boneTransform.mParentNodeTransform[boneTransform.mIndex];
                    parentBoneTransform[j] = &boneTransform.mDerivedTransform[boneTransform.mIndex];
                }
            }

            finalMat.loadFromAoS( parentBoneParentNodeTransform );
            parentBone.loadFromAoS( parentBoneTransform );

            finalMat *= parentBone; //finalMat = parentBoneParentNodeTransform * parentBone;

            //ArrayMatrixAf4x3::retain is quite lengthy in instruction count, and the
            //general case is to inherit both attributes. This branch is justified.
            if( !BooleanMask4::allBitsSet( t.mInheritOrientation, t.mInheritScale ) )
            {
                ArrayMaskR inheritOrientation   = BooleanMask4::getMask( t.mInheritOrientation );
                ArrayMaskR inheritScale         = BooleanMask4::getMask( t.mInheritScale );
                finalMat.retain( inheritOrientation, inheritScale );
            }

            ArrayMatrixAf4x3 baseTransform;
            baseTransform.makeTransform( *t.mPosition, *t.mScale, *t.mOrientation );

            finalMat *= baseTransform; //finalMat = parentMat * baseTransform;

            finalMat.streamToAoS( t.mDerivedTransform );

            finalMat.decomposition( *t.mDerivedPosition,
                                         *t.mDerivedScale,
                                         *t.mDerivedOrientation );

#if OGRE_DEBUG_MODE
            for( size_t j=0; j<ARRAY_PACKED_REALS; ++j )
            {
                if( t.mOwner[j] )
                    static_cast<TagPoint*>(t.mOwner[j])->mCachedTransformOutOfDate = false;
            }
#endif

            t.advancePack();
        }
    }
    //-----------------------------------------------------------------------
    void TagPoint::updateAllTransformsTagOnTag( const size_t numNodes, Transform t )
    {
        for( size_t i=0; i<numNodes; i += ARRAY_PACKED_REALS )
        {
            //Retrieve from parents. Unfortunately we need to do SoA -> AoS -> SoA conversion
            ArrayMatrixAf4x3 finalMat;

            Matrix4 const * RESTRICT_ALIAS parentBoneTransform[ARRAY_PACKED_REALS];

            for( size_t j=0; j<ARRAY_PACKED_REALS; ++j )
            {
                Transform &parentTransform = t.mParents[j]->_getTransform();
                parentBoneTransform[j] = &parentTransform.mDerivedTransform[parentTransform.mIndex];
            }

            finalMat.loadFromAoS( parentBoneTransform );

            //ArrayMatrixAf4x3::retain is quite lengthy in instruction count, and the
            //general case is to inherit both attributes. This branch is justified.
            if( !BooleanMask4::allBitsSet( t.mInheritOrientation, t.mInheritScale ) )
            {
                ArrayMaskR inheritOrientation   = BooleanMask4::getMask( t.mInheritOrientation );
                ArrayMaskR inheritScale         = BooleanMask4::getMask( t.mInheritScale );
                finalMat.retain( inheritOrientation, inheritScale );
            }

            ArrayMatrixAf4x3 baseTransform;
            baseTransform.makeTransform( *t.mPosition, *t.mScale, *t.mOrientation );

            finalMat *= baseTransform; //finalMat = parentMat * baseTransform;

            finalMat.streamToAoS( t.mDerivedTransform );

            finalMat.decomposition( *t.mDerivedPosition,
                                         *t.mDerivedScale,
                                         *t.mDerivedOrientation );

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
    TagPoint* TagPoint::createChildTagPoint( const Vector3& vPos, const Quaternion& qRot )
    {
        TagPoint *newNode = mCreator->_createTagPoint( this, mNodeMemoryManager );
        newNode->setPosition( vPos );
        newNode->setOrientation( qRot );

        //_createTagPoint must have passed us as parent. It's a special
        //case to improve memory usage (avoid transfering mTransform)
        mChildren.push_back( newNode );
        newNode->mParentIndex = mChildren.size() - 1;

        return newNode;
    }
}
