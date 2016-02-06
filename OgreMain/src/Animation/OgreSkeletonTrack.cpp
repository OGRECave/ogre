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

#include "Animation/OgreSkeletonTrack.h"

#include "Math/Array/OgreMathlib.h"
#include "Math/Array/OgreBoneTransform.h"
#include "Math/Array/OgreKfTransformArrayMemoryManager.h"

#include "OgreException.h"

namespace Ogre
{
    SkeletonTrack::SkeletonTrack( uint32 boneBlockIdx,
                                    KfTransformArrayMemoryManager *kfTransformMemoryManager ) :
        mKeyFrameRigs( 0 ),
        mNumFrames( 0 ),
        mBoneBlockIdx( boneBlockIdx ),
        mUsedSlots( 0 ),
        mLocalMemoryManager( kfTransformMemoryManager )
    {
    }
    //-----------------------------------------------------------------------------------
    SkeletonTrack::~SkeletonTrack()
    {
    }
    //-----------------------------------------------------------------------------------
    void SkeletonTrack::setNumKeyFrame( size_t numKeyFrames )
    {
        mKeyFrameRigs.reserve( numKeyFrames );
    }
    //-----------------------------------------------------------------------------------
    void SkeletonTrack::addKeyFrame( Real timestamp, Real frameRate )
    {
        assert( mKeyFrameRigs.empty() || timestamp > mKeyFrameRigs.back().mFrame );

        mKeyFrameRigs.push_back( KeyFrameRig() );
        KeyFrameRig &keyFrame = mKeyFrameRigs.back();
        keyFrame.mFrame = timestamp * frameRate;
        keyFrame.mInvNextFrameDistance = 1.0f;
        if( mKeyFrameRigs.size() > 1 )
        {
            KeyFrameRig &prevKeyFrame = mKeyFrameRigs[mKeyFrameRigs.size()-2];
            prevKeyFrame.mInvNextFrameDistance = 1.0f / (keyFrame.mFrame - prevKeyFrame.mFrame);
        }

        mLocalMemoryManager->createNewNode( (KfTransform**)(&keyFrame.mBoneTransform) );
    }
    //-----------------------------------------------------------------------------------
    void SkeletonTrack::setKeyFrameTransform( Real frame, uint32 slot, const Vector3 &vPos,
                                                const Quaternion &qRot, const Vector3 vScale )
    {
        KeyFrameRigVec::iterator itor = mKeyFrameRigs.begin();
        KeyFrameRigVec::iterator end  = mKeyFrameRigs.end();

        while( itor != end && Math::Abs( itor->mFrame - frame ) < 1e-6f )
            ++itor;

        if( itor == mKeyFrameRigs.end() )
        {
            OGRE_EXCEPT( Exception::ERR_ITEM_NOT_FOUND, "Frame not found.",
                         "SkeletonTrack::setKeyFrameTransform" );
        }

        itor->mBoneTransform->mPosition.setFromVector3( vPos, slot );
        itor->mBoneTransform->mOrientation.setFromQuaternion( qRot, slot );
        itor->mBoneTransform->mScale.setFromVector3( vScale, slot );

        mUsedSlots = std::max( slot+1, mUsedSlots );
    }
    //-----------------------------------------------------------------------------------
    inline void SkeletonTrack::getKeyFrameRigAt( KeyFrameRigVec::const_iterator &inOutPrevFrame,
                                                    KeyFrameRigVec::const_iterator &outNextFrame,
                                                    Real frame ) const
    {
        KeyFrameRigVec::const_iterator prevFrame = inOutPrevFrame;
        KeyFrameRigVec::const_iterator nextFrame = inOutPrevFrame;

        if( frame >= nextFrame->mFrame )
        {
            while( nextFrame != (mKeyFrameRigs.end() - 1) && nextFrame->mFrame <= frame )
                prevFrame = nextFrame++;
        }
        else
        {
            while( prevFrame != mKeyFrameRigs.begin() && prevFrame->mFrame > frame )
                nextFrame = prevFrame--;
        }

        inOutPrevFrame  = prevFrame;
        outNextFrame    = nextFrame;
    }
    //-----------------------------------------------------------------------------------
    void SkeletonTrack::applyKeyFrameRigAt( KeyFrameRigVec::const_iterator &inOutLastKnownKeyFrameRig,
                                            float frame, ArrayReal animWeight,
                                            const ArrayReal * RESTRICT_ALIAS perBoneWeights,
                                            const TransformArray &boneTransforms ) const
    {
        KeyFrameRigVec::const_iterator prevFrame = inOutLastKnownKeyFrameRig;
        KeyFrameRigVec::const_iterator nextFrame;
        getKeyFrameRigAt( prevFrame, nextFrame, frame );

        const Real scalarW = (frame - prevFrame->mFrame) * prevFrame->mInvNextFrameDistance;
        ArrayReal fTimeW = Mathlib::SetAll( scalarW );

        size_t level    = mBoneBlockIdx >> 24;
        size_t offset   = mBoneBlockIdx & 0x00FFFFFF;

        ArrayVector3 * RESTRICT_ALIAS finalPos      = boneTransforms[level].mPosition + offset;
        ArrayVector3 * RESTRICT_ALIAS finalScale    = boneTransforms[level].mScale + offset;
        ArrayQuaternion * RESTRICT_ALIAS finalRot   = boneTransforms[level].mOrientation + offset;

        KfTransform * RESTRICT_ALIAS prevTransf = prevFrame->mBoneTransform;
        KfTransform * RESTRICT_ALIAS nextTransf = nextFrame->mBoneTransform;

        ArrayVector3 interpPos, interpScale;
        ArrayQuaternion interpRot;
        //Interpolate keyframes' rotation not using shortestPath to respect the original animation
        interpPos   = Math::lerp( prevTransf->mPosition, nextTransf->mPosition, fTimeW );
        interpRot   = ArrayQuaternion::nlerpShortest( fTimeW,
                                                      prevTransf->mOrientation,
                                                      nextTransf->mOrientation );
        interpScale = Math::lerp( prevTransf->mScale, nextTransf->mScale, fTimeW );

        //Combine our internal flag (that prevents blending
        //unanimated bones) with user's custom weights
        ArrayReal fW = (*perBoneWeights) * animWeight;

        //When mixing, also interpolate rotation not using shortest path; as this is usually desired
        *finalPos   += interpPos * fW;
        *finalScale *= Math::lerp( ArrayVector3::UNIT_SCALE, interpScale, fW );
        *finalRot   = (*finalRot) * ArrayQuaternion::nlerpShortest( fW, ArrayQuaternion::IDENTITY,
                                                                    interpRot );

        inOutLastKnownKeyFrameRig = prevFrame;
    }
    //-----------------------------------------------------------------------------------
    void SkeletonTrack::_bakeUnusedSlots(void)
    {
        assert( mUsedSlots <= ARRAY_PACKED_REALS );

        if( mUsedSlots <= (ARRAY_PACKED_REALS >> 1) )
        {
            KeyFrameRigVec::const_iterator itor = mKeyFrameRigs.begin();
            KeyFrameRigVec::const_iterator end  = mKeyFrameRigs.end();

            while( itor != end )
            {
                size_t j=0;
                for( size_t i=mUsedSlots; i<ARRAY_PACKED_REALS; ++i )
                {
                    Vector3 vTmp;
                    Quaternion qTmp;
                    itor->mBoneTransform->mPosition.getAsVector3( vTmp, j );
                    itor->mBoneTransform->mPosition.setFromVector3( vTmp, i );
                    itor->mBoneTransform->mOrientation.getAsQuaternion( qTmp, j );
                    itor->mBoneTransform->mOrientation.setFromQuaternion( qTmp, i );
                    itor->mBoneTransform->mScale.getAsVector3( vTmp, j );
                    itor->mBoneTransform->mScale.setFromVector3( vTmp, i );

                    j = (j+1) % mUsedSlots;
                }
                ++itor;
            }
        }
    }
}
