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
#include "Animation/OgreBone.h"
#include "Animation/OgreTagPoint.h"
#include "OgreNode.h"
#include "OgreLogManager.h"

#include "Math/Array/OgreBoneMemoryManager.h"
#include "Math/Array/OgreKfTransform.h"
#include "Math/Array/OgreBooleanMask.h"

#if OGRE_DEBUG_MODE
    #define CACHED_TRANSFORM_OUT_OF_DATE() this->setCachedTransformOutOfDate()
#else
    #define CACHED_TRANSFORM_OUT_OF_DATE() ((void)0)
#endif

namespace Ogre {
    //-----------------------------------------------------------------------
    Bone::Bone() :
        IdObject( 0 ),
        mReverseBind( 0 ),
        mTransform( BoneTransform() ),
#if OGRE_DEBUG_MODE
        mCachedTransformOutOfDate( true ),
        mDebugParentNode( 0 ),
        mInitialized( false ),
#endif
        mDepthLevel( 0 ),
        mParent( 0 ),
        mName( "@Dummy Bone" ),
        mBoneMemoryManager( 0 ),
        mGlobalIndex( -1 ),
        mParentIndex( -1 )
    {
    }
    //-----------------------------------------------------------------------
    Bone::~Bone()
    {
#if OGRE_DEBUG_MODE
        assert( !mInitialized && "Must call _deinitialize() before destructor!" );
#endif
    }
    //-----------------------------------------------------------------------
    void Bone::_initialize( IdType id, BoneMemoryManager *boneMemoryManager,
                            Bone *parent, ArrayMatrixAf4x3 const * RESTRICT_ALIAS reverseBind )
    {
#if OGRE_DEBUG_MODE
        assert( !mInitialized );
        mInitialized = true;
#endif

        this->_setId( id );
        mReverseBind        = reverseBind;
        mParent             = parent;
        mBoneMemoryManager  = boneMemoryManager;

        if( mParent )
            mDepthLevel = mParent->mDepthLevel + 1;

        //Will initialize mTransform
        mBoneMemoryManager->nodeCreated( mTransform, mDepthLevel );
        mTransform.mOwner[mTransform.mIndex] = this;
        if( mParent )
        {
            const BoneTransform parentTransform = mParent->mTransform;
            mTransform.mParentTransform[mTransform.mIndex] =
                                &parentTransform.mDerivedTransform[parentTransform.mIndex];

            mParent->mChildren.push_back( this );
            this->mParentIndex = mParent->mChildren.size() - 1;
        }
    }
    //-----------------------------------------------------------------------
    void Bone::_deinitialize( bool debugCheckLifoOrder )
    {
        TagPointVec::const_iterator itor = mTagPointChildren.begin();
        TagPointVec::const_iterator end  = mTagPointChildren.end();

        while( itor != end )
        {
            (*itor)->_unsetParentBone();
            (*itor)->mParentIndex = -1;
            ++itor;
        }

        mTagPointChildren.clear();

#if OGRE_DEBUG_MODE
        //Calling mParent->removeChild() is not necessary during Release mode at all,
        //However we need to call this->_deinitialize in LIFO order (children first,
        //then parents). We check that here via this assert.
        //And for the assert to work, we need to call removeChild.
        if( mParent )
            mParent->removeChild( this );
        assert( mChildren.empty() || !debugCheckLifoOrder );
#endif

        if( mBoneMemoryManager )
            mBoneMemoryManager->nodeDestroyed( mTransform, mDepthLevel );

        mReverseBind        = 0;
        mBoneMemoryManager  = 0;

#if OGRE_DEBUG_MODE
        mInitialized = false;
#endif
    }
    //-----------------------------------------------------------------------
    void Bone::setCachedTransformOutOfDate(void) const
    {
#if OGRE_DEBUG_MODE
        mCachedTransformOutOfDate = true;

        BoneVec::const_iterator itor = mChildren.begin();
        BoneVec::const_iterator end  = mChildren.end();

        while( itor != end )
        {
            (*itor)->setCachedTransformOutOfDate();
            ++itor;
        }
#else
        OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR,
                    "Do not call setCachedTransformOutOfDate in Release builds.\n"
                    "Use CACHED_TRANSFORM_OUT_OF_DATE macro instead!",
                    "Bone::setCachedTransformOutOfDate" );
#endif
    }
    //-----------------------------------------------------------------------
    void Bone::resetParentTransformPtr(void)
    {
        if( mParent )
        {
            const BoneTransform parentTransform = mParent->mTransform;
            mTransform.mParentTransform[mTransform.mIndex] =
                                    &parentTransform.mDerivedTransform[parentTransform.mIndex];
        }

        _memoryRebased();
    }
    //-----------------------------------------------------------------------
    void Bone::_memoryRebased(void)
    {
        BoneVec::iterator itor = mChildren.begin();
        BoneVec::iterator end  = mChildren.end();
        while( itor != end )
        {
            (*itor)->resetParentTransformPtr();
            ++itor;
        }
    }
    //-----------------------------------------------------------------------
    void Bone::addTagPoint( TagPoint *tagPoint )
    {
        tagPoint->_setParentBone( this );
        mTagPointChildren.push_back( tagPoint );
        tagPoint->mParentIndex = mTagPointChildren.size() - 1u;
    }
    //-----------------------------------------------------------------------
    void Bone::removeTagPoint( TagPoint *child )
    {
        assert( child->getParentBone() == this && "TagPoint says it's not our child (We're Bone)" );
        assert( child->mParentIndex < mTagPointChildren.size() && "mParentIndex was out of date!!!" );

        if( child->mParentIndex < mChildren.size() )
        {
            TagPointVec::iterator itor = mTagPointChildren.begin() + child->mParentIndex;

            assert( child == *itor && "mParentIndex was out of date!!!" );

            if( child == *itor )
            {
                itor = efficientVectorRemove( mTagPointChildren, itor );
                child->_unsetParentBone();
                child->mParentIndex = -1;

                //The node that was at the end got swapped and has now a different index
                if( itor != mTagPointChildren.end() )
                    (*itor)->mParentIndex = itor - mTagPointChildren.begin();
            }
        }
    }
    //-----------------------------------------------------------------------
    void Bone::_setNodeParent( Node *nodeParent )
    {
#if OGRE_DEBUG_MODE
        mDebugParentNode = nodeParent;
#endif
        if( nodeParent )
        {
            //This "Hack" just works. Don't ask. And it's fast!
            //(we're responsible for ensuring the memory layout matches)
            Transform parentTransf = nodeParent->_getTransform();
            mTransform.mParentNodeTransform[mTransform.mIndex] = reinterpret_cast<SimpleMatrixAf4x3*>(
                                                &parentTransf.mDerivedTransform[parentTransf.mIndex] );
        }
        else
        {
            mTransform.mParentNodeTransform[mTransform.mIndex] = &SimpleMatrixAf4x3::IDENTITY;
        }
    }
    //-----------------------------------------------------------------------
    void Bone::setInheritOrientation(bool inherit)
    {
        mTransform.mInheritOrientation[mTransform.mIndex] = inherit;
        CACHED_TRANSFORM_OUT_OF_DATE();
    }
    //-----------------------------------------------------------------------
    bool Bone::getInheritOrientation(void) const
    {
        return mTransform.mInheritOrientation[mTransform.mIndex];
    }
    //-----------------------------------------------------------------------
    void Bone::setInheritScale(bool inherit)
    {
        mTransform.mInheritScale[mTransform.mIndex] = inherit;
        CACHED_TRANSFORM_OUT_OF_DATE();
    }
    //-----------------------------------------------------------------------
    bool Bone::getInheritScale(void) const
    {
        return mTransform.mInheritScale[mTransform.mIndex];
    }
    //-----------------------------------------------------------------------
    Matrix4 Bone::_getDerivedTransform(void) const
    {
#if OGRE_DEBUG_MODE
        assert( !mCachedTransformOutOfDate );
#endif
        OGRE_ALIGNED_DECL( Matrix4, localSpaceBone, OGRE_SIMD_ALIGNMENT );
        OGRE_ALIGNED_DECL( Matrix4, parentNodeTransform, OGRE_SIMD_ALIGNMENT );

        mTransform.mDerivedTransform[mTransform.mIndex].store4x3( &localSpaceBone );
        mTransform.mParentNodeTransform[mTransform.mIndex]->store4x3( &parentNodeTransform );

        parentNodeTransform.concatenateAffine( localSpaceBone );

        return parentNodeTransform;
    }
    //-----------------------------------------------------------------------
    const SimpleMatrixAf4x3& Bone::_getFullTransformUpdated(void)
    {
        _updateFromParent();
        return mTransform.mDerivedTransform[mTransform.mIndex];
    }
    //-----------------------------------------------------------------------
    void Bone::_updateFromParent(void)
    {
        if( mParent )
            mParent->_updateFromParent();

        updateFromParentImpl();
    }
    //-----------------------------------------------------------------------
    void Bone::updateFromParentImpl(void)
    {
        //Retrieve from parents. Unfortunately we need to do AoS -> SoA -> AoS conversion
        /*ArrayMatrixAf4x3 nodeMat;
        ArrayMatrixAf4x3 parentMat;
        nodeMat.loadFromAoS( mTransform.mParentNodeTransform );
        parentMat.loadFromAoS( mTransform.mParentTransform );

        //ArrayMatrixAf4x3::retain is quite lengthy in instruction count, and the
        //general case is to inherit both attributes. This branch is justified.
        if( BooleanMask4::allBitsSet( mTransform.mInheritOrientation, mTransform.mInheritScale ) )
        {
            ArrayMaskR inheritOrientation   = BooleanMask4::getMask( mTransform.mInheritOrientation );
            ArrayMaskR inheritScale         = BooleanMask4::getMask( mTransform.mInheritScale );
            parentMat.retain( inheritOrientation, inheritScale );
        }

        ArrayMatrixAf4x3 mat;
        mat.makeTransform( *mTransform.mPosition, *mTransform.mScale, *mTransform.mOrientation );
        mat = parentMat * mat;
        */
        /*
            Calculating the bone matrices
            -----------------------------
            Now that we have the derived scaling factors, orientations & positions matrices,
            we have to compute the Matrix4x3 to apply to the vertices of a mesh.
            Because any modification of a vertex has to be relative to the bone, we must
            first reverse transform by the Bone's original derived position/orientation/scale,
            then transform by the new derived position/orientation/scale.
        */
        /*
        mat.storeToAoS( mTransform.mDerivedTransform );

#if OGRE_DEBUG_MODE
        for( size_t j=0; j<ARRAY_PACKED_REALS; ++j )
        {
            if( mTransform.mOwner[j] )
                mTransform.mOwner[j]->mCachedTransformOutOfDate = false;
        }
#endif*/
    }
    //-----------------------------------------------------------------------
    void Bone::updateAllTransforms( const size_t numNodes, BoneTransform t,
                                    ArrayMatrixAf4x3 const * RESTRICT_ALIAS _reverseBind,
                                    size_t numBinds )
    {
        size_t currentBind = 0;
        numBinds = (numBinds + ARRAY_PACKED_REALS - 1) / ARRAY_PACKED_REALS;

        ArrayMatrixAf4x3 derivedTransform;
        for( size_t i=0; i<numNodes; i += ARRAY_PACKED_REALS )
        {
            //Retrieve from parents. Unfortunately we need to do SoA -> AoS -> SoA conversion
            ArrayMatrixAf4x3 nodeMat;
            ArrayMatrixAf4x3 parentMat;

            nodeMat.loadFromAoS( t.mParentNodeTransform );
            parentMat.loadFromAoS( t.mParentTransform );

            //ArrayMatrixAf4x3::retain is quite lengthy in instruction count, and the
            //general case is to inherit both attributes. This branch is justified.
            if( !BooleanMask4::allBitsSet( t.mInheritOrientation, t.mInheritScale ) )
            {
                ArrayMaskR inheritOrientation   = BooleanMask4::getMask( t.mInheritOrientation );
                ArrayMaskR inheritScale         = BooleanMask4::getMask( t.mInheritScale );
                parentMat.retain( inheritOrientation, inheritScale );
            }

            const ArrayMatrixAf4x3 * RESTRICT_ALIAS reverseBind = _reverseBind + currentBind;

            derivedTransform.makeTransform( *t.mPosition, *t.mScale, *t.mOrientation );
            derivedTransform = parentMat * derivedTransform;
            derivedTransform.storeToAoS( t.mDerivedTransform );

            /*
                Calculating the bone matrices
                -----------------------------
                Now that we have the derived scaling factors, orientations & positions matrices,
                we have to compute the Matrix4x3 to apply to the vertices of a mesh.
                Because any modification of a vertex has to be relative to the bone, we must
                first reverse transform by the Bone's original derived position/orientation/scale,
                then transform by the new derived position/orientation/scale.
            */
            //derivedTransform = nodeMat * ( derivedTransform * (*reverseBind) );
            derivedTransform *= *reverseBind;
            derivedTransform = nodeMat * derivedTransform;
            derivedTransform.streamToAoS( t.mFinalTransform );

#if OGRE_DEBUG_MODE
            for( size_t j=0; j<ARRAY_PACKED_REALS; ++j )
            {
                if( t.mOwner[j] )
                    t.mOwner[j]->mCachedTransformOutOfDate = false;
            }
#endif

            t.advancePack();
            currentBind = ( currentBind + 1 ) % numBinds;
        }
    }
    //-----------------------------------------------------------------------
    void Bone::removeChild( Bone* child )
    {
        assert( child->getParent() == this && "Node says it's not our child" );
        assert( child->mParentIndex < mChildren.size() && "mParentIndex was out of date!!!" );

        if( child->mParentIndex < mChildren.size() )
        {
            BoneVec::iterator itor = mChildren.begin() + child->mParentIndex;

            assert( child == *itor && "mParentIndex was out of date!!!" );

            if( child == *itor )
            {
                itor = efficientVectorRemove( mChildren, itor );
                child->mParentIndex = -1;

                //The node that was at the end got swapped and has now a different index
                if( itor != mChildren.end() )
                    (*itor)->mParentIndex = itor - mChildren.begin();
            }
        }
    }
    //-----------------------------------------------------------------------
#if OGRE_DEBUG_MODE
    void Bone::_setCachedTransformOutOfDate(void)
    {
        mCachedTransformOutOfDate = true;
    }
#endif
}

#undef CACHED_TRANSFORM_OUT_OF_DATE
