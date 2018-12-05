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

#include "Animation/OgreSkeletonAnimation.h"
#include "Animation/OgreSkeletonAnimationDef.h"
#include "Animation/OgreSkeletonInstance.h"

namespace Ogre
{
    SkeletonAnimation::SkeletonAnimation( const SkeletonAnimationDef *definition,
                                            const FastArray<size_t> *_slotStarts,
                                            SkeletonInstance *owner ) :
        mDefinition( definition ),
        mBoneWeights( 0 ),
        mCurrentFrame( 0 ),
        mFrameRate( definition->mOriginalFrameRate ),
        mWeight( 1.0f ),
        mSlotStarts( _slotStarts ),
        mLoop( true ),
        mEnabled( false ),
        mOwner( owner ),
        mName( definition->mName )
    {
        mLastKnownKeyFrames.reserve( definition->mTracks.size() );
#ifndef NDEBUG
        for( size_t i=0; i<mSlotStarts->size(); ++i )
            assert( (*mSlotStarts)[i] < ARRAY_PACKED_REALS );
#endif
    }
    //-----------------------------------------------------------------------------------
    void SkeletonAnimation::_initialize(void)
    {
        const FastArray<size_t> &slotStarts = *mSlotStarts;

        mBoneWeights = RawSimdUniquePtr<ArrayReal, MEMCATEGORY_ANIMATION>( mDefinition->mTracks.size() );
        ArrayReal *boneWeights = mBoneWeights.get();
        Real *boneWeightsScalar = reinterpret_cast<Real*>( mBoneWeights.get() );

        SkeletonTrackVec::const_iterator itor = mDefinition->mTracks.begin();
        SkeletonTrackVec::const_iterator end  = mDefinition->mTracks.end();

        while( itor != end )
        {
            if( itor->getUsedSlots() <= (ARRAY_PACKED_REALS >> 1) )
            {
                size_t level = itor->getBoneBlockIdx() >> 24;
                size_t slotStart = slotStarts[level];

                for( size_t i=0; i<slotStart; ++i )
                    boneWeightsScalar[i] = 0.0f;
                for( size_t i=slotStart; i<slotStart + itor->getUsedSlots(); ++i )
                    boneWeightsScalar[i] = 1.0f;
                for( size_t i=slotStart + itor->getUsedSlots(); i<ARRAY_PACKED_REALS; ++i )
                    boneWeightsScalar[i] = 0.0f;
            }
            else
            {
                *boneWeights = Mathlib::ONE;
            }

            mLastKnownKeyFrames.push_back( itor->getKeyFrames().begin() );
            boneWeightsScalar += ARRAY_PACKED_REALS;
            ++boneWeights;
            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void SkeletonAnimation::addFrame( Real frames )
    {
        mCurrentFrame += frames;
        Real maxFrame = mDefinition->mNumFrames;

        if( !mLoop )
        {
            mCurrentFrame = Ogre::max( mCurrentFrame, 0 );
            mCurrentFrame = Ogre::min( mCurrentFrame, maxFrame );
        }
        else
        {
            mCurrentFrame = fmod( mCurrentFrame, maxFrame );
            if( mCurrentFrame < 0 )
                mCurrentFrame = maxFrame + mCurrentFrame;
        }
    }
    //-----------------------------------------------------------------------------------
    void SkeletonAnimation::setFrame( Real frames )
    {
        mCurrentFrame = frames;
        Real maxFrame = mDefinition->mNumFrames;

        if( !mLoop )
        {
            mCurrentFrame = Ogre::max( mCurrentFrame, 0 );
            mCurrentFrame = Ogre::min( mCurrentFrame, maxFrame );
        }
        else
        {
            mCurrentFrame = fmod( mCurrentFrame, maxFrame );
            if( mCurrentFrame < 0 )
                mCurrentFrame = maxFrame + mCurrentFrame;
        }
    }
    //-----------------------------------------------------------------------------------
    Real SkeletonAnimation::getNumFrames(void) const
    {
        return mDefinition->mNumFrames;
    }
    //-----------------------------------------------------------------------------------
    Real SkeletonAnimation::getDuration(void) const
    {
        return mDefinition->mNumFrames / mFrameRate;
    }
    //-----------------------------------------------------------------------------------
    void SkeletonAnimation::setBoneWeight( IdString boneName, Real weight )
    {
        map<IdString, size_t>::type::const_iterator itor = mDefinition->mBoneToWeights.find( boneName );
        if( itor != mDefinition->mBoneToWeights.end() )
        {
            size_t level    = itor->second >> 24;
            size_t offset   = itor->second & 0x00FFFFFF;
            Real *aliasedBoneWeights = reinterpret_cast<Real*>( mBoneWeights.get() ) +
                                                offset + (*mSlotStarts)[level];
            *aliasedBoneWeights = weight;
        }
    }
    //-----------------------------------------------------------------------------------
    Real SkeletonAnimation::getBoneWeight( IdString boneName ) const
    {
        Real retVal = 0.0f;

        map<IdString, size_t>::type::const_iterator itor = mDefinition->mBoneToWeights.find( boneName );
        if( itor != mDefinition->mBoneToWeights.end() )
        {
            size_t level    = itor->second >> 24;
            size_t offset   = itor->second & 0x00FFFFFF;
            const Real *aliasedBoneWeights = reinterpret_cast<const Real*>( mBoneWeights.get() ) +
                                                            offset + (*mSlotStarts)[level];
            retVal = *aliasedBoneWeights;
        }
        return retVal;
    }
    //-----------------------------------------------------------------------------------
    Real* SkeletonAnimation::getBoneWeightPtr( IdString boneName )
    {
        Real* retVal = 0;

        map<IdString, size_t>::type::const_iterator itor = mDefinition->mBoneToWeights.find( boneName );
        if( itor != mDefinition->mBoneToWeights.end() )
        {
            size_t level    = itor->second >> 24;
            size_t offset   = itor->second & 0x00FFFFFF;
            retVal = reinterpret_cast<Real*>( mBoneWeights.get() ) + offset + (*mSlotStarts)[level];
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void SkeletonAnimation::setEnabled( bool bEnable )
    {
        if( mEnabled != bEnable )
        {
            if( bEnable )
                mOwner->_enableAnimation( this );
            else
                mOwner->_disableAnimation( this );

            mEnabled = bEnable;
        }
    }
    //-----------------------------------------------------------------------------------
    void SkeletonAnimation::_applyAnimation( const TransformArray &boneTransforms )
    {
        SkeletonTrackVec::const_iterator itor = mDefinition->mTracks.begin();
        SkeletonTrackVec::const_iterator end  = mDefinition->mTracks.end();

        KnownKeyFramesVec::iterator itLastKnownKeyFrame = mLastKnownKeyFrames.begin();

        ArrayReal simdWeight = Mathlib::SetAll( mWeight );
        ArrayReal * RESTRICT_ALIAS boneWeights = mBoneWeights.get();

        while( itor != end )
        {
            itor->applyKeyFrameRigAt( *itLastKnownKeyFrame, mCurrentFrame, simdWeight,
                                        boneWeights, boneTransforms );
            ++itLastKnownKeyFrame;
            ++boneWeights;
            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void SkeletonAnimation::_swapBoneWeightsUniquePtr( RawSimdUniquePtr<ArrayReal, MEMCATEGORY_ANIMATION>
                                                       &inOutBoneWeights )
    {
        inOutBoneWeights.swap( mBoneWeights );
    }
}
