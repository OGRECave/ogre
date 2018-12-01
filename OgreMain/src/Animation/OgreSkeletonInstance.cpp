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

#include "Animation/OgreSkeletonInstance.h"
#include "Animation/OgreSkeletonDef.h"
#include "Animation/OgreSkeletonAnimationDef.h"
#include "Animation/OgreSkeletonManager.h"

#include "OgreId.h"

#include "OgreOldBone.h"
#include "OgreSceneNode.h"
#include "OgreSkeleton.h"

namespace Ogre
{
    SkeletonInstance::SkeletonInstance( const SkeletonDef *skeletonDef,
                                        BoneMemoryManager *boneMemoryManager ) :
            mDefinition( skeletonDef ),
            mParentNode( 0 ),
            mRefCount( 1 )
    {
        mBones.resize( mDefinition->getBones().size(), Bone() );

        vector<list<size_t>::type>::type::const_iterator itDepth = mDefinition->mBonesPerDepth.begin();
        vector<list<size_t>::type>::type::const_iterator enDepth = mDefinition->mBonesPerDepth.end();

        while( itDepth != enDepth )
        {
            list<size_t>::type::const_iterator itor = itDepth->begin();
            list<size_t>::type::const_iterator end  = itDepth->end();

            while( itor != end )
            {
                Bone *parent = 0;
                size_t parentIdx = mDefinition->mBones[*itor].parent;
                const SkeletonDef::BoneData &boneData = mDefinition->mBones[*itor];

                if( parentIdx != std::numeric_limits<size_t>::max() )
                    parent = &mBones[parentIdx];

                Bone &newBone = mBones[*itor];
                newBone._initialize( Id::generateNewId<Node>(), boneMemoryManager, parent, 0 );
                newBone.setPosition( boneData.vPos );
                newBone.setOrientation( boneData.qRot );
                newBone.setScale( boneData.vScale );
                newBone.setInheritOrientation( boneData.bInheritOrientation );
                newBone.setInheritScale( boneData.bInheritScale );
                newBone.setName( boneData.name );
                newBone.mGlobalIndex = *itor;

                ++itor;
            }

            ++itDepth;
        }

        {
            const SkeletonDef::DepthLevelInfoVec &depthLevelInfo = mDefinition->getDepthLevelInfo();
            mSlotStarts.reserve( depthLevelInfo.size() );
            mBoneStartTransforms.reserve( depthLevelInfo.size() );

            mManualBones = RawSimdUniquePtr<ArrayReal, MEMCATEGORY_ANIMATION>(
                                mDefinition->getNumberOfBoneBlocks( depthLevelInfo.size() ) );
            Real *manualBones = reinterpret_cast<Real*>( mManualBones.get() );

            // FIXME: manualBones could possibly be null. What to do?

            size_t currentUnusedSlotIdx = 0;
            mUnusedNodes.resize( mDefinition->mNumUnusedSlots, Bone() );

            ArrayMatrixAf4x3 const *reverseBindPose = mDefinition->mReverseBindPose.get();

            SkeletonDef::DepthLevelInfoVec::const_iterator itor = depthLevelInfo.begin();
            SkeletonDef::DepthLevelInfoVec::const_iterator end  = depthLevelInfo.end();

            while( itor != end )
            {
                const BoneTransform &firstBoneTransform = mBones[itor->firstBoneIndex]._getTransform();
                mBoneStartTransforms.push_back( firstBoneTransform );
                mSlotStarts.push_back( firstBoneTransform.mIndex );

                assert( (itor->numBonesInLevel <= (ARRAY_PACKED_REALS >> 1)) ||
                        !firstBoneTransform.mIndex );

                if( itor->numBonesInLevel > (ARRAY_PACKED_REALS >> 1) )
                {
                    //TODO: Reserve enough space in mUnusedNodes (amount can be cached in SkeletonDef)
                    size_t unusedSlots = ARRAY_PACKED_REALS -
                                            (itor->numBonesInLevel % ARRAY_PACKED_REALS);

                    // When x is a multiple of ARRAY_PACKED_REALS, then the formula gives:
                    //      ARRAY_PACKED_REALS - x % ARRAY_PACKED_REALS = ARRAY_PACKED_REALS;
                    // but we don't need to create any unused node, as the slots are already
                    // aligned to the block
                    if( unusedSlots != ARRAY_PACKED_REALS )
                    {
                        for( size_t i=0; i<unusedSlots; ++i )
                        {
                            //Dummy bones need the right parent so they
                            //consume memory from the right depth level
                            Bone *parent = 0;
                            if( itor != depthLevelInfo.begin() )
                                parent = &mBones[(itor-1)->firstBoneIndex];

                            Bone &unused = mUnusedNodes[currentUnusedSlotIdx];
                            unused._initialize( Id::generateNewId<Node>(), boneMemoryManager,
                                                parent, 0 );
                            unused.setName( "Unused" );
                            unused.mGlobalIndex = i;

                            ++currentUnusedSlotIdx;
                        }
                    }
                }

                //Prepare for default pose, 0.0f for manually animated or a slot that
                //doesn't belong to us, 1.0f when we should apply animation
                size_t slotStart = firstBoneTransform.mIndex;
                size_t remainder = (slotStart + itor->numBonesInLevel) % ARRAY_PACKED_REALS;
                for( size_t i=0; i<slotStart; ++i )
                    *manualBones++ = 0.0f;
                for( size_t i=slotStart; i<slotStart + itor->numBonesInLevel; ++i )
                    *manualBones++ = 1.0f;
                if( remainder != 0 )
                {
                    for( size_t i=0; i<ARRAY_PACKED_REALS - remainder; ++i )
                        *manualBones++ = 0.0f;
                }

                //Take advantage that all Bones in mOwner are planar in memory
                Bone **bonesPtr = firstBoneTransform.mOwner;
                for( size_t i=slotStart; i<slotStart + itor->numBonesInLevel; ++i )
                {
                    bonesPtr[i]->_setReverseBindPtr( reverseBindPose );
                    if( !( (i+1) % ARRAY_PACKED_REALS) )
                        ++reverseBindPose;
                }

                if( remainder != 0 )
                    ++reverseBindPose;

                ++itor;
            }
        }

        const SkeletonAnimationDefVec &animationDefs = mDefinition->getAnimationDefs();
        mAnimations.reserve( animationDefs.size() );

        SkeletonAnimationDefVec::const_iterator itor = animationDefs.begin();
        SkeletonAnimationDefVec::const_iterator end  = animationDefs.end();

        while( itor != end )
        {
            SkeletonAnimation animation( &(*itor), &mSlotStarts, this );
            mAnimations.push_back( animation );
            mAnimations.back()._initialize();
            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    SkeletonInstance::~SkeletonInstance()
    {
        {
            SceneNodeBonePairVec::iterator itor = mCustomParentSceneNodes.begin();
            SceneNodeBonePairVec::iterator end  = mCustomParentSceneNodes.end();
            while( itor != end )
            {
                itor->sceneNodeParent->_detachAllBones( this );
                ++itor;
            }
        }

        //Detach all bones in the reverse order they were attached (LIFO!!!)
        size_t currentDepth = mDefinition->mBonesPerDepth.size() - 1;
        vector<list<size_t>::type>::type::const_reverse_iterator ritDepth = mDefinition->mBonesPerDepth.rbegin();
        vector<list<size_t>::type>::type::const_reverse_iterator renDepth = mDefinition->mBonesPerDepth.rend();

        BoneVec::reverse_iterator ritUnusedNodes = mUnusedNodes.rbegin();
        BoneVec::reverse_iterator renUnusedNodes = mUnusedNodes.rend();

        while( ritDepth != renDepth )
        {
            while( ritUnusedNodes != renUnusedNodes && ritUnusedNodes->getDepthLevel() == currentDepth )
            {
                ritUnusedNodes->_deinitialize();
                ++ritUnusedNodes;
            }

            list<size_t>::type::const_reverse_iterator ritor = ritDepth->rbegin();
            list<size_t>::type::const_reverse_iterator rend  = ritDepth->rend();
            while( ritor != rend )
            {
                mBones[*ritor]._deinitialize();
                ++ritor;
            }

            --currentDepth;
            ++ritDepth;
        }

        mAnimations.clear();

        mUnusedNodes.clear();
        mBones.clear();
    }
    //-----------------------------------------------------------------------------------
    void SkeletonInstance::update(void)
    {
        if( !mActiveAnimations.empty() )
            resetToPose();

        ActiveAnimationsVec::iterator itor = mActiveAnimations.begin();
        ActiveAnimationsVec::iterator end  = mActiveAnimations.end();

        while( itor != end )
        {
            (*itor)->_applyAnimation( mBoneStartTransforms );
            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void SkeletonInstance::resetToPose(void)
    {
        KfTransform const * RESTRICT_ALIAS bindPose = mDefinition->getBindPose();
        ArrayReal const * RESTRICT_ALIAS manualBones = mManualBones.get();

        SkeletonDef::DepthLevelInfoVec::const_iterator itDepthLevelInfo =
                                                mDefinition->getDepthLevelInfo().begin();

        TransformArray::iterator itor = mBoneStartTransforms.begin();
        TransformArray::iterator end  = mBoneStartTransforms.end();

        while( itor != end )
        {
            BoneTransform t = *itor;
            for( size_t i=0; i<itDepthLevelInfo->numBonesInLevel; i += ARRAY_PACKED_REALS )
            {
                OGRE_PREFETCH_T0( (const char*)(t.mPosition + 4) );
                OGRE_PREFETCH_T0( (const char*)(t.mOrientation + 4) );
                OGRE_PREFETCH_T0( (const char*)(t.mScale + 4) );
                OGRE_PREFETCH_T0( (const char*)(t.mPosition + 8) );
                OGRE_PREFETCH_T0( (const char*)(t.mOrientation + 8) );
                OGRE_PREFETCH_T0( (const char*)(t.mScale + 8) );

                *t.mPosition = Math::lerp( *t.mPosition, bindPose->mPosition, *manualBones );
                *t.mOrientation = Math::lerp( *t.mOrientation, bindPose->mOrientation, *manualBones );
                *t.mScale = Math::lerp( *t.mScale, bindPose->mScale, *manualBones );
                t.advancePack();

                ++bindPose;
                ++manualBones;
            }

            ++itor;
            ++itDepthLevelInfo;
        }
    }
    //-----------------------------------------------------------------------------------
    void SkeletonInstance::setManualBone( Bone *bone, bool isManual )
    {
        assert( &mBones[bone->mGlobalIndex] == bone && "The bone doesn't belong to this instance!" );

        uint32 depthLevel = bone->getDepthLevel();
        Bone &firstBone = mBones[mDefinition->getDepthLevelInfo()[depthLevel].firstBoneIndex];

        const BoneTransform &boneTransf      = bone->_getTransform();
        const BoneTransform &firstBoneTransf = firstBone._getTransform();

        //Get bone offset relative to the beginning of its depth level.
        uintptr_t diff = (boneTransf.mOwner - firstBoneTransf.mOwner) + boneTransf.mIndex;
        //Offset by all past bones from parent levels.
        diff += mDefinition->getNumberOfBoneBlocks( depthLevel ) * ARRAY_PACKED_REALS;

        assert( diff < mManualBones.size() * ARRAY_PACKED_REALS &&
                "Offset incorrectly calculated. manualBones[diff] will overflow!" );

        Real *manualBones = reinterpret_cast<Real*>( mManualBones.get() );
        manualBones[diff] = isManual ? 0.0f : 1.0f;
    }
    //-----------------------------------------------------------------------------------
    bool SkeletonInstance::isManualBone( Bone *bone )
    {
        assert( &mBones[bone->mGlobalIndex] == bone && "The bone doesn't belong to this instance!" );

        uint32 depthLevel = bone->getDepthLevel();
        Bone &firstBone = mBones[mDefinition->getDepthLevelInfo()[depthLevel].firstBoneIndex];

        const BoneTransform &boneTransf      = bone->_getTransform();
        const BoneTransform &firstBoneTransf = firstBone._getTransform();

        //Get bone offset relative to the beginning of its depth level.
        uintptr_t diff = (boneTransf.mOwner - firstBoneTransf.mOwner) + boneTransf.mIndex;
        //Offset by all past bones from parent levels.
        diff += mDefinition->getNumberOfBoneBlocks( depthLevel ) * ARRAY_PACKED_REALS;

        assert( diff < mManualBones.size() * ARRAY_PACKED_REALS &&
                "Offset incorrectly calculated. manualBones[diff] will overflow!" );

        const Real *manualBones = reinterpret_cast<const Real*>( mManualBones.get() );
        return manualBones[diff] == 0.0f;
    }
    //-----------------------------------------------------------------------------------
    void SkeletonInstance::setSceneNodeAsParentOfBone( Bone *bone, SceneNode *nodeParent )
    {
        assert( &mBones[bone->mGlobalIndex] == bone && "The bone doesn't belong to this instance!" );

        SceneNodeBonePairVec::iterator itor = mCustomParentSceneNodes.begin();
        SceneNodeBonePairVec::iterator end  = mCustomParentSceneNodes.end();
        while( itor != end && itor->boneChild != bone )
            ++itor;

        if( itor != end && itor->boneChild == bone )
        {
            itor->sceneNodeParent->_detachBone( this, itor->boneChild );
            efficientVectorRemove( mCustomParentSceneNodes, itor );
        }

        if( nodeParent )
        {
            bone->_setNodeParent( nodeParent );
            mCustomParentSceneNodes.push_back( SceneNodeBonePair( bone, nodeParent ) );
            nodeParent->_attachBone( this, bone );
        }
        else
        {
            bone->_setNodeParent( mParentNode );
        }
    }
    //-----------------------------------------------------------------------------------
    bool SkeletonInstance::hasBone( IdString boneName ) const
    {
        return mDefinition->mBoneIndexByName.find( boneName ) != mDefinition->mBoneIndexByName.end();
    }      
    //-----------------------------------------------------------------------------------
    Bone* SkeletonInstance::getBone( IdString boneName )
    {
        SkeletonDef::BoneNameMap::const_iterator itor = mDefinition->mBoneIndexByName.find( boneName );

        if( itor == mDefinition->mBoneIndexByName.end() )
        {
            OGRE_EXCEPT( Exception::ERR_ITEM_NOT_FOUND,
                             "Can't find bone with name '" + boneName.getFriendlyText() + "'",
                             "SkeletonInstance::getBone" );
        }

        return &mBones[itor->second];
    }
    //-----------------------------------------------------------------------------------
    Bone* SkeletonInstance::getBone( size_t index )
    {
        return &mBones[index];
    }
    //-----------------------------------------------------------------------------------
    size_t SkeletonInstance::getNumBones(void) const
    {
        return mBones.size();
    }
    //-----------------------------------------------------------------------------------
    bool SkeletonInstance::hasAnimation( IdString name ) const
    {
        SkeletonAnimationVec::const_iterator itor = mAnimations.begin();
        SkeletonAnimationVec::const_iterator end  = mAnimations.end();

        while( itor != end && itor->getName() != name )
            ++itor;

        return itor != end;
    }
    //-----------------------------------------------------------------------------------
    SkeletonAnimation* SkeletonInstance::getAnimation( IdString name )
    {
        SkeletonAnimationVec::iterator itor = mAnimations.begin();
        SkeletonAnimationVec::iterator end  = mAnimations.end();

        while( itor != end )
        {
            if( itor->getName() == name )
                return &(*itor);
            ++itor;
        }

        OGRE_EXCEPT( Exception::ERR_ITEM_NOT_FOUND,
                     "Can't find animation '" + name.getFriendlyText() + "'",
                     "SkeletonInstance::getAnimation" );
        return 0;    
    }
    //-----------------------------------------------------------------------------------
    void SkeletonInstance::addAnimationsFromSkeleton( const String &skelName, const String &groupName )
    {
        //First save BoneWeightPtr which would otherwise be freed during mAnimations' resize
        //SkeletonAnimation does not follow the rule of 3
        //https://en.wikipedia.org/wiki/Rule_of_three_(C%2B%2B_programming)
        typedef vector< RawSimdUniquePtr<ArrayReal, MEMCATEGORY_ANIMATION> >::type BoneWeightPtrVec;
        const size_t oldNumAnimations = mAnimations.size();
        BoneWeightPtrVec boneWeightPtrs( mAnimations.size() );

        for( size_t i=0; i<oldNumAnimations; ++i )
            mAnimations[i]._swapBoneWeightsUniquePtr( boneWeightPtrs[i] );

        SkeletonDefPtr definition = SkeletonManager::getSingleton().getSkeletonDef(skelName, groupName);

        const SkeletonAnimationDefVec &animationDefs = definition->getAnimationDefs();
        mAnimations.reserve( mAnimations.size() + animationDefs.size() );

        SkeletonAnimationDefVec::const_iterator itor = animationDefs.begin();
        SkeletonAnimationDefVec::const_iterator end  = animationDefs.end();
        while( itor != end )
        {
            SkeletonAnimation animation( &(*itor), &mSlotStarts, this );
            mAnimations.push_back( animation );
            mAnimations.back()._initialize();
            ++itor;
        }

        //Restore the BoneWeightPtr
        for( size_t i=0; i<oldNumAnimations; ++i )
            mAnimations[i]._swapBoneWeightsUniquePtr( boneWeightPtrs[i] );
    }
    //-----------------------------------------------------------------------------------
    void SkeletonInstance::_enableAnimation( SkeletonAnimation *animation )
    {
        mActiveAnimations.push_back( animation );
    }
    //-----------------------------------------------------------------------------------
    void SkeletonInstance::_disableAnimation( SkeletonAnimation *animation )
    {
        ActiveAnimationsVec::iterator it = std::find( mActiveAnimations.begin(), mActiveAnimations.end(),
                                                        animation );
        if( it != mActiveAnimations.end() )
            efficientVectorRemove( mActiveAnimations, it );
    }
    //-----------------------------------------------------------------------------------
    void SkeletonInstance::setParentNode( Node *parentNode )
    {
        mParentNode = parentNode;

        {
            BoneVec::iterator itor = mBones.begin();
            BoneVec::iterator end  = mBones.end();

            while( itor != end )
            {
                itor->_setNodeParent( mParentNode );
                ++itor;
            }
        }

        {
            //Restore the bones with custom scene nodes.
            SceneNodeBonePairVec::iterator itor = mCustomParentSceneNodes.begin();
            SceneNodeBonePairVec::iterator end  = mCustomParentSceneNodes.end();
            while( itor != end )
            {
                itor->boneChild->_setNodeParent( itor->sceneNodeParent );
                ++itor;
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void SkeletonInstance::getTransforms( SimpleMatrixAf4x3 * RESTRICT_ALIAS outTransform,
                                            const FastArray<unsigned short> &usedBones ) const
    {
        FastArray<unsigned short>::const_iterator itor = usedBones.begin();
        FastArray<unsigned short>::const_iterator end  = usedBones.end();

        while( itor != end )
        {
            *outTransform++ = mBones[*itor]._getFullTransform();
            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void SkeletonInstance::_updateBoneStartTransforms(void)
    {
        const SkeletonDef::DepthLevelInfoVec &depthLevelInfo = mDefinition->getDepthLevelInfo();

        assert( mBoneStartTransforms.size() == depthLevelInfo.size() );

        TransformArray::iterator itBoneStartTr = mBoneStartTransforms.begin();

        SkeletonDef::DepthLevelInfoVec::const_iterator itor = depthLevelInfo.begin();
        SkeletonDef::DepthLevelInfoVec::const_iterator end  = depthLevelInfo.end();

        while( itor != end )
        {
            *itBoneStartTr++ = mBones[itor->firstBoneIndex]._getTransform();
            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    const void* SkeletonInstance::_getMemoryBlock(void) const
    {
        return reinterpret_cast<const void*>( mBoneStartTransforms[0].mOwner );
    }
    //-----------------------------------------------------------------------------------
    const void* SkeletonInstance::_getMemoryUniqueOffset(void) const
    {
        return reinterpret_cast<const void*>(
                        mBoneStartTransforms[0].mOwner + mBoneStartTransforms[0].mIndex );
    }
    //-----------------------------------------------------------------------------------
    void SkeletonInstance::_incrementRefCount(void) 
    {
        mRefCount++;
    }
    //-----------------------------------------------------------------------------------
    void SkeletonInstance::_decrementRefCount(void) 
    {
        mRefCount--;
    }
    //-----------------------------------------------------------------------------------
    uint16 SkeletonInstance::_getRefCount(void) const 
    {
        return mRefCount;
    }
}
