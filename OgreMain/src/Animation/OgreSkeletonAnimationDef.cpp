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

#include "Animation/OgreSkeletonAnimationDef.h"
#include "Animation/OgreSkeletonDef.h"

#include "Math/Array/OgreMathlib.h"
#include "Math/Array/OgreTransform.h"
#include "Math/Array/OgreKfTransformArrayMemoryManager.h"
#include "Math/Array/OgreKfTransform.h"

#include "OgreAnimation.h"
#include "OgreOldBone.h"
#include "OgreKeyFrame.h"
#include "OgreSkeleton.h"
#include "OgreStringConverter.h"

namespace Ogre
{
    SkeletonAnimationDef::SkeletonAnimationDef() :
        mNumFrames( 0 ),
        mOriginalFrameRate( 25.0f ),
        mSkeletonDef( 0 ),
        mKfTransformMemoryManager( 0 )
    {
    }
    //-----------------------------------------------------------------------------------
    SkeletonAnimationDef::~SkeletonAnimationDef()
    {
        mTracks.clear();

        if( mKfTransformMemoryManager )
        {
            mKfTransformMemoryManager->destroy();
            delete mKfTransformMemoryManager;
            mKfTransformMemoryManager = 0;
        }
    }
    //-----------------------------------------------------------------------------------
    void SkeletonAnimationDef::build( const v1::Skeleton *skeleton, const v1::Animation *animation,
                                      Real frameRate )
    {
        mOriginalFrameRate = frameRate;
        mNumFrames = animation->getLength() * frameRate;

        //Converts Bone Index -> Slot Index
        const SkeletonDef::BoneToSlotVec &boneToSlot    = mSkeletonDef->getBoneToSlot();
        const SkeletonDef::IndexToIndexMap &slotToBone  = mSkeletonDef->getSlotToBone();

        //1st Pass: Count the number of keyframes, so we know how
        //much memory to allocate, as we don't listen for resizes.
        //We also build a list of unique keyframe timestamps per block
        //(i.e. merge the keyframes from two bones that the same block)
        v1::Animation::OldNodeTrackIterator itor = animation->getOldNodeTrackIterator();
        {
            //Count the number of blocks needed by counting the number of unique keyframes per block.
            //i.e. When ARRAY_PACKED_REALS = 4; if 2 bones are in the same block and have the same
            //keyframe time value, we'll need 4 slots (aka. 1 block). If those 2 bones have two
            //different time values, we'll need 8 slots.
            TimestampVec emptyVec;
            TimestampsPerBlock timestampsByBlock;

            while( itor.hasMoreElements() )
            {
                size_t boneIdx                      = itor.peekNextKey();
                v1::OldNodeAnimationTrack *track    = itor.getNext();

                if( track->getNumKeyFrames() > 0 )
                {
                    uint32 slotIdx = boneToSlot[boneIdx];
                    uint32 blockIdx = SkeletonDef::slotToBlockIdx( slotIdx );

                    TimestampsPerBlock::iterator itKeyframes = timestampsByBlock.find( blockIdx );
                    if( itKeyframes == timestampsByBlock.end() )
                    {
                        itKeyframes = timestampsByBlock.insert(
                                            std::make_pair( (size_t)blockIdx, emptyVec ) ).first;
                    }

                    itKeyframes->second.reserve( track->getNumKeyFrames() );

                    for( size_t i=0; i<track->getNumKeyFrames(); ++i )
                    {
                        Real timestamp = track->getKeyFrame(i)->getTime();
                        TimestampVec::iterator it = std::lower_bound( itKeyframes->second.begin(),
                                                                      itKeyframes->second.end(),
                                                                      timestamp );
                        if( it == itKeyframes->second.end() || *it != timestamp )
                            itKeyframes->second.insert( it, timestamp );
                    }
                }
            }

            //We need to iterate again because 'std::distance( timestampsByBlock.begin(), itKeyframes )'
            //would be bogus while we were still inserting to timestampsByBlock thus altering the order.
            itor = animation->getOldNodeTrackIterator();

            while( itor.hasMoreElements() )
            {
                size_t boneIdx                      = itor.peekNextKey();
                v1::OldNodeAnimationTrack *track    = itor.getNext();

                if( track->getNumKeyFrames() > 0 )
                {
                    uint32 slotIdx = boneToSlot[boneIdx];
                    uint32 blockIdx = SkeletonDef::slotToBlockIdx( slotIdx );

                    TimestampsPerBlock::iterator itKeyframes = timestampsByBlock.find( blockIdx );
                    size_t trackDiff = std::distance( timestampsByBlock.begin(), itKeyframes );
                    mBoneToWeights[skeleton->getBone( boneIdx )->getName()] =
                                        (slotIdx & 0xFF000000) |
                                        ( (trackDiff * ARRAY_PACKED_REALS +
                                          (slotIdx & 0x00FFFFFF) % ARRAY_PACKED_REALS) );
                }
            }

            //Now that we've built the list of unique keyframes, allocate the mTracks and its keyframes
            allocateCacheFriendlyKeyframes( timestampsByBlock, frameRate );
        }

        //Set now all the transforms the allocated space
        SkeletonTrackVec::iterator itTrack = mTracks.begin();
        SkeletonTrackVec::iterator enTrack = mTracks.end();

        while( itTrack != enTrack )
        {
            KeyFrameRigVec &keyFrames = itTrack->_getKeyFrames();
            KeyFrameRigVec::iterator itKeys = keyFrames.begin();
            KeyFrameRigVec::iterator enKeys = keyFrames.end();

            uint32 blockIdx = itTrack->getBoneBlockIdx();
            uint32 slotStart= SkeletonDef::blockIdxToSlotStart( blockIdx );

            while( itKeys != enKeys )
            {
                Real fTime = itKeys->mFrame * frameRate;

                for( uint32 i=0; i<ARRAY_PACKED_REALS; ++i )
                {
                    uint32 slotIdx = slotStart + i;

                    uint32 boneIdx = -1;
                    map<uint32, uint32>::type::const_iterator it = slotToBone.find( slotIdx );
                    if( it != slotToBone.end() )
                        boneIdx = it->second;

					if( animation->hasOldNodeTrack( boneIdx ) )
                    {
                        v1::OldNodeAnimationTrack *oldTrack = animation->getOldNodeTrack( boneIdx );

                        v1::TransformKeyFrame originalKF( 0, fTime );
                        getInterpolatedUnnormalizedKeyFrame( oldTrack, animation->_getTimeIndex( fTime ),
                                                             &originalKF );

                        itKeys->mBoneTransform->mPosition.setFromVector3( originalKF.getTranslate(), i );
                        itKeys->mBoneTransform->mOrientation.setFromQuaternion( originalKF.getRotation(),
                                                                                i );
                        itKeys->mBoneTransform->mScale.setFromVector3( originalKF.getScale(), i );
                        itTrack->_setMaxUsedSlot( i );
                    }
                    else
                    {
                        //This bone is not affected at all by the animation, but is
                        //part of the block. Fill with identity transform
                        itKeys->mBoneTransform->mPosition.setFromVector3( Vector3::ZERO, i );
                        itKeys->mBoneTransform->mOrientation.setFromQuaternion( Quaternion::IDENTITY, i );
                        itKeys->mBoneTransform->mScale.setFromVector3( Vector3::UNIT_SCALE, i );
                    }
                }

                ++itKeys;
            }

            itTrack->_bakeUnusedSlots();

            ++itTrack;
        }
    }
    //-----------------------------------------------------------------------------------
    void SkeletonAnimationDef::getInterpolatedUnnormalizedKeyFrame( v1::OldNodeAnimationTrack *oldTrack,
                                                                    const v1::TimeIndex& timeIndex,
                                                                    v1::TransformKeyFrame* kf )
    {
        v1::KeyFrame *kBase1, *kBase2;
        v1::TransformKeyFrame *k1, *k2;
        unsigned short firstKeyIndex;

        Real t = oldTrack->getKeyFramesAtTime( timeIndex, &kBase1, &kBase2, &firstKeyIndex);
        k1 = static_cast<v1::TransformKeyFrame*>(kBase1);
        k2 = static_cast<v1::TransformKeyFrame*>(kBase2);

        if (t == 0.0)
        {
            // Just use k1
            kf->setRotation(k1->getRotation());
            kf->setTranslate(k1->getTranslate());
            kf->setScale(k1->getScale());
        }
        else
        {
            // Interpolate by t
            Vector3 base;
            // Translation
            base = k1->getTranslate();
            kf->setTranslate( base + ((k2->getTranslate() - base) * t) );

            // Scale
            base = k1->getScale();
            kf->setScale( base + ((k2->getScale() - base) * t) );

            Quaternion qBase = k1->getRotation();
            kf->setRotation( qBase + ((k2->getRotation() - qBase) * t) );
        }
    }
    //-----------------------------------------------------------------------------------
    void SkeletonAnimationDef::allocateCacheFriendlyKeyframes(
                                            const TimestampsPerBlock &timestampsByBlock, Real frameRate )
    {
        assert( !mKfTransformMemoryManager );

        size_t numKeyFrames = 0;
        TimestampsPerBlock::const_iterator itor = timestampsByBlock.begin();
        TimestampsPerBlock::const_iterator end  = timestampsByBlock.end();

        while( itor != end )
        {
            numKeyFrames += itor->second.size();
            ++itor;
        }

        mKfTransformMemoryManager = new KfTransformArrayMemoryManager(
                                            0, numKeyFrames * ARRAY_PACKED_REALS,
                                            -1, numKeyFrames * ARRAY_PACKED_REALS );
        mKfTransformMemoryManager->initialize();

        mTracks.reserve( timestampsByBlock.size() );

        //1st pass: All first keyframes.
        itor = timestampsByBlock.begin();
        while( itor != end )
        {
            size_t blockIdx = itor->first;
            mTracks.push_back( SkeletonTrack( static_cast<uint32>(blockIdx), mKfTransformMemoryManager ) );

            mTracks.back().addKeyFrame( *itor->second.begin(), frameRate );
            ++itor;
        }

        vector<size_t>::type keyframesDone; //One per block
        keyframesDone.resize( timestampsByBlock.size(), 1 );

        //2nd pass: The in-between keyframes.
        bool addedAny = true;
        while( addedAny )
        {
            addedAny = false;

            //Keep adding keyframes until we're done with them
            size_t i = 0;
            itor = timestampsByBlock.begin();
            while( itor != end )
            {
                if( keyframesDone[i] < itor->second.size() - 1 )
                {
                    //First find if there are key frames (J0) from the next blocks whose
                    //next keyframe (J1) is before ours (I0), and add those instead (J0).
                    //If not, add ours.
                    Real timestampI0 = itor->second[keyframesDone[i]];

                    bool subAddedAny = false;
                    size_t j = i+1;
                    TimestampsPerBlock::const_iterator itor2 = itor;
                    ++itor2;
                    while( itor2 != end )
                    {
                        if( keyframesDone[j] < itor2->second.size() - 1 )
                        {
                            Real timestampJ1 = itor2->second[keyframesDone[j] + 1];
                            if( timestampJ1 <= timestampI0 )
                            {
                                Real timestampJ0 = itor2->second[keyframesDone[j]];
                                mTracks[j].addKeyFrame( timestampJ0, frameRate );
                                ++keyframesDone[j];
                                addedAny = subAddedAny = true;
                            }
                        }

                        ++j;
                        ++itor2;
                    }

                    //Don't add ours if a previous track's keyframe (H0) is between
                    //our keyframe I0 and our next keyframe I1. We'll add both together
                    //in the next iteration
                    bool dontAdd = false;
                    Real timestampI1 = itor->second[keyframesDone[i] + 1];
                    j = 0;
                    itor2 = timestampsByBlock.begin();
                    while( itor2 != itor && !dontAdd )
                    {
                        if( keyframesDone[j] < itor2->second.size() - 1 )
                        {
                            Real timestampJ0 = itor2->second[keyframesDone[j]];
                            if( timestampI1 > timestampJ0 && timestampI0 < timestampJ0 )
                            {
                                dontAdd = true;
                                addedAny = true; //There still a reason to iterate again
                            }
                        }
                        ++j;
                        ++itor2;
                    }

                    if( !subAddedAny && !dontAdd )
                    {
                        mTracks[i].addKeyFrame( timestampI0, frameRate );
                        ++keyframesDone[i];
                        addedAny = true;
                    }
                }

                ++i;
                ++itor;
            }
        }

        //3rd Pass: Create the last key frames
        size_t i = 0;
        itor = timestampsByBlock.begin();
        while( itor != end )
        {
            if( itor->second.size() > 1 )
            {
                mTracks[i].addKeyFrame( *(itor->second.end()-1), frameRate );
                ++keyframesDone[i];
            }
            ++i;
            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void SkeletonAnimationDef::_dumpCsvTracks( String &outText ) const
    {
        const SkeletonDef::BoneDataVec &mBones = mSkeletonDef->getBones();
        const SkeletonDef::IndexToIndexMap &slotToBone = mSkeletonDef->getSlotToBone();

        SkeletonTrackVec::const_iterator itor = mTracks.begin();
        SkeletonTrackVec::const_iterator end  = mTracks.end();

        while( itor != end )
        {
            const SkeletonTrack &track = *itor;
            uint32 blockIdx = track.getBoneBlockIdx();

            const KeyFrameRigVec &keyFrames = track.getKeyFrames();

            for( size_t i=0; i<ARRAY_PACKED_REALS; ++i )
            {
                uint32 slotIdx = SkeletonDef::blockIdxToSlotStart( blockIdx ) + i;

                SkeletonDef::IndexToIndexMap::const_iterator itSlotToBone = slotToBone.find( slotIdx );

                if( itSlotToBone != slotToBone.end() )
                {
                    uint32 boneIdx = itSlotToBone->second;

                    const SkeletonDef::BoneData &boneDef = mBones[boneIdx];
                    outText += boneDef.name;
                    outText += ",";

                    KeyFrameRigVec::const_iterator itKeyFrames = keyFrames.begin();
                    KeyFrameRigVec::const_iterator enKeyFrames = keyFrames.end();

                    while( itKeyFrames != enKeyFrames )
                    {
                        outText += StringConverter::toString( itKeyFrames->mFrame );
                        outText += ",";

                        const KfTransform * RESTRICT_ALIAS boneTransform = itKeyFrames->mBoneTransform;

                        Vector3 vPos, vScale;
                        Quaternion qRot;

                        boneTransform->mPosition.getAsVector3( vPos, i );
                        boneTransform->mOrientation.getAsQuaternion( qRot, i );
                        boneTransform->mScale.getAsVector3( vScale, i );

                        outText += StringConverter::toString( vPos.x ) + ",";
                        outText += StringConverter::toString( vPos.y ) + ",";
                        outText += StringConverter::toString( vPos.z ) + ",";
                        outText += StringConverter::toString( qRot.x ) + ",";
                        outText += StringConverter::toString( qRot.y ) + ",";
                        outText += StringConverter::toString( qRot.z ) + ",";
                        outText += StringConverter::toString( qRot.w ) + ",";
                        outText += StringConverter::toString( vScale.x ) + ",";
                        outText += StringConverter::toString( vScale.y ) + ",";
                        outText += StringConverter::toString( vScale.z ) + ",";
                        ++itKeyFrames;
                    }

                    outText += "\n";
                }
            }

            ++itor;
        }
    }
}
