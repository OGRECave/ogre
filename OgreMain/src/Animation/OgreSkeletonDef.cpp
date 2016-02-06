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

#include "Animation/OgreSkeletonDef.h"
#include "Animation/OgreSkeletonAnimationDef.h"
#include "Animation/OgreBone.h"
#include "Math/Array/OgreBoneMemoryManager.h"
#include "Math/Array/OgreKfTransformArrayMemoryManager.h"

#include "OgreId.h"

#include "OgreOldBone.h"
#include "OgreSkeleton.h"

namespace Ogre
{
    SkeletonDef::SkeletonDef( const v1::Skeleton *originalSkeleton, Real frameRate ) :
        mNumUnusedSlots( 0 ),
        mName( originalSkeleton->getName() )
    {
        mBones.reserve( originalSkeleton->getNumBones() );

        //Clone the bone data
        size_t numDepthLevels = 1;
        v1::Skeleton::ConstBoneIterator itor = originalSkeleton->getBoneIteratorConst();
        while( itor.hasMoreElements() )
        {
            v1::OldBone *bone = itor.getNext();
            size_t parentIndex = -1;
            if( bone->getParent() )
            {
                assert( !bone->getParent() || dynamic_cast<v1::OldBone*>( bone->getParent() ) );
                v1::OldBone *parent = static_cast<v1::OldBone*>( bone->getParent() );
                parentIndex = parent->getHandle();

                size_t tmpDepthLevels = 2;
                v1::OldNode *tmpParent = parent;
                while( (tmpParent = tmpParent->getParent()) )
                    ++tmpDepthLevels;
                numDepthLevels = std::max( numDepthLevels, tmpDepthLevels );
            }

            BoneData boneData( bone->getHandle(), parentIndex, bone->getPosition(),
                                bone->getOrientation(), bone->getScale(),
                                bone->getName(), bone->getInheritOrientation(),
                                bone->getInheritScale() );
            mBoneIndexByName[bone->getName()] = mBones.size();
            mBones.push_back( boneData );
        }

        mBonesPerDepth.resize( numDepthLevels );
        mDepthLevelInfoVec.resize( numDepthLevels );
        for( size_t i=0; i<mBones.size(); ++i )
        {
            size_t currentDepthLevel = 0;
            BoneData const *tmpBone = &mBones[i];
            while( tmpBone->parent != std::numeric_limits<size_t>::max() )
            {
                tmpBone = &mBones[tmpBone->parent];
                ++currentDepthLevel;
            }

            DepthLevelInfo &info = mDepthLevelInfoVec[currentDepthLevel];
            info.firstBoneIndex = std::min( i, info.firstBoneIndex );
            ++info.numBonesInLevel;

            mBonesPerDepth[currentDepthLevel].push_back( i );
        }

        // Populate both mBoneToSlot and mSlotToBone
        mBoneToSlot.reserve( originalSkeleton->getNumBones() );
        for( size_t i=0; i<originalSkeleton->getNumBones(); ++i )
        {
            const v1::OldBone *bone = originalSkeleton->getBone( i );

            size_t depthLevel = 0;
            v1::OldNode const *parentBone = bone;
            while( (parentBone = parentBone->getParent()) )
                ++depthLevel;

            size_t offset = 0;
            BoneToSlotVec::const_iterator itBoneToSlot = mBoneToSlot.begin();
            BoneToSlotVec::const_iterator enBoneToSlot  = mBoneToSlot.end();
            while( itBoneToSlot != enBoneToSlot )
            {
                if( (*itBoneToSlot >> 24) == depthLevel )
                    ++offset;
                ++itBoneToSlot;
            }

            //Build the map that lets us know the final slot bone index that will be
            //assigned to this bone (to get the block we still need to divide by ARRAY_PACKED_REALS)
            mBoneToSlot.push_back( static_cast<uint>((depthLevel << 24) | (offset & 0x00FFFFFF)) );
            mSlotToBone[mBoneToSlot.back()] = static_cast<uint>(i);
        }

        //Clone the animations
        mAnimationDefs.resize( originalSkeleton->getNumAnimations() );
        for( size_t i=0; i<originalSkeleton->getNumAnimations(); ++i )
        {
            mAnimationDefs[i]._setSkeletonDef( this );
            mAnimationDefs[i].setName( originalSkeleton->getAnimation( i )->getName() );
            mAnimationDefs[i].build( originalSkeleton, originalSkeleton->getAnimation( i ), frameRate );
        }

        //Create the bones (just like we would for SkeletonInstance)so we can
        //get derived position/rotation/scale and then calculate its inverse
        BoneMemoryManager boneMemoryManager;
        vector<Bone>::type boneNodes( mBones.size(), Bone() );
        vector<list<size_t>::type>::type::const_iterator itDepth = mBonesPerDepth.begin();
        vector<list<size_t>::type>::type::const_iterator enDepth = mBonesPerDepth.end();

        while( itDepth != enDepth )
        {
            list<size_t>::type::const_iterator bonesItor = itDepth->begin();
            list<size_t>::type::const_iterator bonesItorEnd  = itDepth->end();

            while( bonesItor != bonesItorEnd )
            {
                Bone *parent = 0;
                size_t parentIdx = mBones[*bonesItor].parent;
                const BoneData &boneData = mBones[*bonesItor];

                if( parentIdx != std::numeric_limits<size_t>::max() )
                    parent = &boneNodes[parentIdx];

                Bone &newBone = boneNodes[*bonesItor];
                newBone._initialize( Id::generateNewId<Bone>(), &boneMemoryManager, parent, 0 );
                
                newBone.setPosition( boneData.vPos );
                newBone.setOrientation( boneData.qRot );
                newBone.setScale( boneData.vScale );
                newBone.setInheritOrientation( boneData.bInheritOrientation );
                newBone.setInheritScale( boneData.bInheritScale );
                newBone.setName( boneData.name );
                newBone.mGlobalIndex = *bonesItor;

                ++bonesItor;
            }

            ++itDepth;
        }

        //Calculate in bulk. Calling getDerivedPositionUpdated & Co like
        //we need would need, has O(N!) complexity.
        for( size_t i=0; i<boneMemoryManager.getNumDepths(); ++i )
        {
            BoneTransform t;
            const size_t numNodes = boneMemoryManager.getFirstNode( t, i );
            Bone::updateAllTransforms( numNodes, t, &ArrayMatrixAf4x3::IDENTITY, 1 );
        }

        {
            size_t numBoneBlocks = getNumberOfBoneBlocks( mDepthLevelInfoVec.size() );

            //Create initial bind pose.
            mBindPose = RawSimdUniquePtr<KfTransform, MEMCATEGORY_ANIMATION>( numBoneBlocks );
            RawSimdUniquePtr<SimpleMatrixAf4x3, MEMCATEGORY_ANIMATION> derivedPosesPtr =
                        RawSimdUniquePtr<SimpleMatrixAf4x3, MEMCATEGORY_ANIMATION>(
                                                            numBoneBlocks * ARRAY_PACKED_REALS );

            size_t bindPoseIndex = 0;
            KfTransform *bindPose = mBindPose.get();
            SimpleMatrixAf4x3 *derivedPose = derivedPosesPtr.get();

            size_t currentDepthLevel = 0;
            DepthLevelInfoVec::const_iterator bonesItor = mDepthLevelInfoVec.begin();
            DepthLevelInfoVec::const_iterator bonesItorEnd  = mDepthLevelInfoVec.end();

            while( bonesItor != bonesItorEnd )
            {
                list<size_t>::type::const_iterator itBoneIdx = mBonesPerDepth[currentDepthLevel].begin();
                list<size_t>::type::const_iterator enBoneIdx = mBonesPerDepth[currentDepthLevel].end();
                while( itBoneIdx != enBoneIdx )
                {
                    const BoneData &boneData = mBones[*itBoneIdx];
                    bindPose->mPosition.setFromVector3( boneData.vPos, bindPoseIndex );
                    bindPose->mOrientation.setFromQuaternion( boneData.qRot, bindPoseIndex );
                    bindPose->mScale.setFromVector3( boneData.vScale, bindPoseIndex );

                    const Bone &derivedBone = boneNodes[*itBoneIdx];
                    derivedPose[bindPoseIndex] = derivedBone._getLocalSpaceTransform();

                    ++bindPoseIndex;
                    if( bindPoseIndex >= ARRAY_PACKED_REALS )
                    {
                        bindPoseIndex = 0;
                        ++bindPose;
                        derivedPose += ARRAY_PACKED_REALS;
                    }

                    ++itBoneIdx;
                }

                if( bonesItor->numBonesInLevel <= (ARRAY_PACKED_REALS >> 1) )
                {
                    //Repeat the slots in patterns to save memory when instances get created
                    //Do the same in SkeletonTrack::_bakeUnusedSlots
                    size_t k=0;
                    for( size_t j=bonesItor->numBonesInLevel; j<ARRAY_PACKED_REALS; ++j )
                    {
                        Vector3 vTmp;
                        Quaternion qTmp;
                        bindPose->mPosition.getAsVector3( vTmp, k );
                        bindPose->mPosition.setFromVector3( vTmp, j );
                        bindPose->mOrientation.getAsQuaternion( qTmp, k );
                        bindPose->mOrientation.setFromQuaternion( qTmp, j );
                        bindPose->mScale.getAsVector3( vTmp, k );
                        bindPose->mScale.setFromVector3( vTmp, j );

                        derivedPose[j] = derivedPose[k];

                        k = (k+1) % bonesItor->numBonesInLevel;
                    }

                    bindPoseIndex = 0;
                    ++bindPose;
                    derivedPose += ARRAY_PACKED_REALS;
                }
                else if( bindPoseIndex != 0 )
                {
                    //We can't repeat to save memory per instance. But we still need
                    //to create a hole so the next depth level starts in the first slot
                    //of the next block
                    for( ; bindPoseIndex<ARRAY_PACKED_REALS; ++bindPoseIndex )
                    {
                        bindPose->mPosition.setFromVector3( Vector3::ZERO, bindPoseIndex );
                        bindPose->mOrientation.setFromQuaternion( Quaternion::IDENTITY, bindPoseIndex );
                        bindPose->mScale.setFromVector3( Vector3::UNIT_SCALE, bindPoseIndex );

                        derivedPose[bindPoseIndex] = SimpleMatrixAf4x3::IDENTITY;
                    }

                    bindPoseIndex = 0;
                    ++bindPose;
                    derivedPose += ARRAY_PACKED_REALS;
                }

                ++currentDepthLevel;
                ++bonesItor;
            }

            //Now set the reverse of the binding pose (so we can pass the
            //construct the derived transform matrices in the correct space)
            mReverseBindPose = RawSimdUniquePtr<ArrayMatrixAf4x3, MEMCATEGORY_ANIMATION>(numBoneBlocks);
            ArrayMatrixAf4x3 *reverseBindPose = mReverseBindPose.get();
            derivedPose = derivedPosesPtr.get();
            for( size_t i=0; i<numBoneBlocks; ++i )
            {
                reverseBindPose[i].loadFromAoS( derivedPose );
                reverseBindPose[i].setToInverseDegeneratesAsIdentity();
                derivedPose += ARRAY_PACKED_REALS;
            }
        }

        {
            //Cache the amount of unused slots for the SkeletonInstance
            mNumUnusedSlots = 0;
            SkeletonDef::DepthLevelInfoVec::const_iterator depthLevelItor = mDepthLevelInfoVec.begin();
            while( depthLevelItor != mDepthLevelInfoVec.end() )
            {
                if( depthLevelItor->numBonesInLevel > (ARRAY_PACKED_REALS >> 1) )
                {
                    size_t unusedSlots = ARRAY_PACKED_REALS -
                                            (depthLevelItor->numBonesInLevel % ARRAY_PACKED_REALS);
                    if( unusedSlots != ARRAY_PACKED_REALS )
                        mNumUnusedSlots += unusedSlots;
                }
                ++depthLevelItor;
            }
        }

        {
            vector<Bone>::type::iterator boneNodeItor = boneNodes.begin();
            vector<Bone>::type::iterator end  = boneNodes.end();
            while( boneNodeItor != end )
            {
                //Don't check LIFO Order since the BoneMemoryManager dies with us.
                const bool debugCheckLifoOrder = false;
                boneNodeItor->_deinitialize( debugCheckLifoOrder );
                ++boneNodeItor;
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void SkeletonDef::getBonesPerDepth( vector<size_t>::type &out ) const
    {
        out.clear();
        out.reserve( mDepthLevelInfoVec.size() );

        DepthLevelInfoVec::const_iterator itor = mDepthLevelInfoVec.begin();
        DepthLevelInfoVec::const_iterator end  = mDepthLevelInfoVec.end();

        while( itor != end )
        {
            out.push_back( itor->numBonesInLevel );
            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    size_t SkeletonDef::getNumberOfBoneBlocks( size_t numLevels ) const
    {
        size_t numBlocks = 0;

        DepthLevelInfoVec::const_iterator itor = mDepthLevelInfoVec.begin();
        DepthLevelInfoVec::const_iterator end  = mDepthLevelInfoVec.begin() + numLevels;

        while( itor != end )
        {
            numBlocks += (itor->numBonesInLevel - 1 + ARRAY_PACKED_REALS) / ARRAY_PACKED_REALS;
            ++itor;
        }

        return numBlocks;
    }
}
