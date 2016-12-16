/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2017 Torus Knot Software Ltd

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

#include "OgreForwardClustered.h"
#include "OgreSceneManager.h"
#include "OgreRenderTarget.h"

#include "Math/Array/OgreArraySphere.h"
#include "Math/Array/OgreBooleanMask.h"

#include "Compositor/OgreCompositorShadowNode.h"

#include "Vao/OgreVaoManager.h"
#include "Vao/OgreTexBufferPacked.h"

#include "OgreHlms.h"

namespace Ogre
{
    //Six variables * 4 (padded vec3) * 4 (bytes) * numLights
    const size_t c_numBytesPerLight = 6 * 4 * 4;

    ForwardClustered::ForwardClustered( uint32 width, uint32 height,
                                        uint32 numSlices, uint32 lightsPerCell,
                                        float minDistance, float maxDistance,
                                        SceneManager *sceneManager ) :
        ForwardPlusBase( sceneManager ),
        mWidth( width ),
        mHeight( height ),
        mNumSlices( numSlices ),
        /*mWidth( 1 ),
        mHeight( 1 ),
        mNumSlices( 2 ),*/
        mLightsPerCell( lightsPerCell + 3u ),
        mGridBuffer( 0 ),
        mCurrentCamera( 0 ),
        mMinDistance( minDistance ),
        mMaxDistance( maxDistance )
    {
        assert( numSlices > 1 && "Must use at least 2 slices for ForwardClustered!" );

        //SIMD optimization restriction.
        assert( (width % ARRAY_PACKED_REALS) == 0 && "Width must be multiple of ARRAY_PACKED_REALS!" );

        mLightCountInCell.resize( mNumSlices * mWidth * mHeight, LightCount() );

        // 2^( x * mNumSlices ) + mMinDistance = mMaxDistance;
        mExponentK = Math::Log2( mMaxDistance - mMinDistance ) / (Real)mNumSlices;
        mInvExponentK = 1.0f / mExponentK;

        mFrustumRegions.resize( (mWidth / ARRAY_PACKED_REALS) * mHeight * mNumSlices );
    }
    //-----------------------------------------------------------------------------------
    ForwardClustered::~ForwardClustered()
    {
    }
    //-----------------------------------------------------------------------------------
    inline float ForwardClustered::getDepthAtSlice( uint32 uSlice ) const
    {
        return -(powf( 2.0f, mExponentK * uSlice ) + mMinDistance);
    }
    //-----------------------------------------------------------------------------------
    inline uint32 ForwardClustered::getSliceAtDepth( Real depth ) const
    {
        return static_cast<uint32>(
                    floorf( Math::Log2( Ogre::max( -depth - mMinDistance, 1 ) ) * mInvExponentK ) );
    }
    //-----------------------------------------------------------------------------------
    void ForwardClustered::execute( size_t threadId, size_t numThreads )
    {
        const size_t slicesPerThread = mNumSlices / numThreads;

        for( size_t i=0; i<slicesPerThread; ++i )
            collectLightForSlice( i + threadId * slicesPerThread );

        const size_t slicesRemainder = mNumSlices % numThreads;
        if( slicesRemainder > threadId )
            collectLightForSlice( threadId + numThreads * slicesPerThread );
    }
    //-----------------------------------------------------------------------------------
    void ForwardClustered::collectLightForSlice( size_t slice )
    {
        const size_t frustumStartIdx = slice * (mWidth / ARRAY_PACKED_REALS) * mHeight;

        Real nearDepthAtSlice = getDepthAtSlice( slice );
        Real farDepthAtSlice  = getDepthAtSlice( slice + 1 );

        if( slice == 0 )
            nearDepthAtSlice = mCurrentCamera->getNearClipDistance();

        if( slice == mNumSlices - 1u )
            farDepthAtSlice = Ogre::max( mCurrentCamera->getFarClipDistance(), farDepthAtSlice );

        const Vector3 *wsCorners = mCurrentCamera->_getCachedWorldSpaceCorners();
        const Vector3 cameraPos = mCurrentCamera->_getCachedDerivedPosition();
        const Vector3 cameraDir = mCurrentCamera->_getCachedDerivedDirection();

        for( size_t y=0; y<mHeight; ++y )
        {
            const Real fWyBottom    = y / (Real)mHeight;
            const Real fWyTop       = (y + 1u) / (Real)mHeight;

            const Vector3 leftMostBottom    = Math::lerp( wsCorners[6], wsCorners[5], fWyBottom );
            const Vector3 leftMostTop       = Math::lerp( wsCorners[6], wsCorners[5], fWyTop );
            const Vector3 rightMostBottom   = Math::lerp( wsCorners[7], wsCorners[4], fWyBottom );
            const Vector3 rightMostTop      = Math::lerp( wsCorners[7], wsCorners[4], fWyTop );

            for( size_t x=0; x<mWidth / ARRAY_PACKED_REALS; ++x )
            {
                FrustumRegion &frustumRegion = mFrustumRegions[frustumStartIdx + y *
                        (mWidth / ARRAY_PACKED_REALS) + x];

                for( size_t i=0; i<ARRAY_PACKED_REALS; ++i )
                {
                    const Real fWxLeft  = (x * ARRAY_PACKED_REALS + i) / (Real)mWidth;
                    const Real fWxRight = (x * ARRAY_PACKED_REALS + i + 1u) / (Real)mWidth;

                    Vector3 leftTop     = Math::lerp( leftMostTop,    rightMostTop,    fWxLeft );
                    Vector3 leftBottom  = Math::lerp( leftMostBottom, rightMostBottom, fWxLeft );
                    Vector3 rightTop    = Math::lerp( leftMostTop,    rightMostTop,    fWxRight );
                    Vector3 rightBottom = Math::lerp( leftMostBottom, rightMostBottom, fWxRight );

                    Vector3 vDir;

                    vDir = (leftTop - cameraPos).normalisedCopy();
                    vDir /= vDir.dotProduct( cameraDir );
                    const Vector3 leftTopNear( vDir * nearDepthAtSlice + cameraPos );
                    vDir = (leftBottom - cameraPos).normalisedCopy();
                    vDir /= vDir.dotProduct( cameraDir );
                    const Vector3 leftBottomNear( vDir * nearDepthAtSlice + cameraPos );
                    vDir = (rightTop - cameraPos).normalisedCopy();
                    vDir /= vDir.dotProduct( cameraDir );
                    const Vector3 rightTopNear( vDir * nearDepthAtSlice + cameraPos );
                    vDir = (rightBottom - cameraPos).normalisedCopy();
                    vDir /= vDir.dotProduct( cameraDir );
                    const Vector3 rightBottomNear( vDir * nearDepthAtSlice + cameraPos );

                    vDir = (leftTop - cameraPos).normalisedCopy();
                    vDir /= vDir.dotProduct( cameraDir );
                    const Vector3 leftTopFar( vDir * farDepthAtSlice + cameraPos );
                    vDir = (leftBottom - cameraPos).normalisedCopy();
                    vDir /= vDir.dotProduct( cameraDir );
                    const Vector3 leftBottomFar( vDir * farDepthAtSlice + cameraPos );
                    vDir = (rightTop - cameraPos).normalisedCopy();
                    vDir /= vDir.dotProduct( cameraDir );
                    const Vector3 rightTopFar( vDir * farDepthAtSlice + cameraPos );
                    vDir = (rightBottom - cameraPos).normalisedCopy();
                    vDir /= vDir.dotProduct( cameraDir );
                    const Vector3 rightBottomFar( vDir * farDepthAtSlice + cameraPos );

                    Plane plane[6];
                    plane[FRUSTUM_PLANE_LEFT]   = Plane( leftTopFar, leftTopNear, leftBottomFar );
                    plane[FRUSTUM_PLANE_RIGHT]  = Plane( rightTopFar, rightBottomFar, rightTopNear );
                    plane[FRUSTUM_PLANE_TOP]    = Plane( leftTopFar, rightTopFar, leftTopNear );
                    plane[FRUSTUM_PLANE_BOTTOM] = Plane( leftBottomFar, leftBottomNear, rightBottomFar );
                    plane[FRUSTUM_PLANE_NEAR]   = Plane( rightBottomNear, rightTopNear, leftBottomNear );
                    plane[FRUSTUM_PLANE_FAR]    = Plane( leftBottomFar, rightBottomFar, rightTopFar );

                    Aabb planeAabb( leftTopNear, Vector3::ZERO );
                    planeAabb.merge( leftBottomNear );
                    planeAabb.merge( rightTopNear );
                    planeAabb.merge( rightBottomNear );
                    planeAabb.merge( leftTopFar );
                    planeAabb.merge( leftBottomFar );
                    planeAabb.merge( rightTopFar );
                    planeAabb.merge( rightBottomFar );

                    for( int i=0; i<6; ++i )
                    {
                        frustumRegion.plane[i].normal.setFromVector3( plane[i].normal, i );
                        Mathlib::Set( frustumRegion.plane[i].negD, plane[i].d, i );
                    }

                    frustumRegion.aabb.setFromAabb( planeAabb, i );
                }
            }
        }

        const size_t numPackedFrustumsPerSlice = (mWidth / ARRAY_PACKED_REALS) * mHeight;

        //Initialize light counts to 0
        memset( mLightCountInCell.begin() + frustumStartIdx * ARRAY_PACKED_REALS,
                0, numPackedFrustumsPerSlice * ARRAY_PACKED_REALS * sizeof(LightCount) );

        const size_t numLights = mCurrentLightList.size();
        LightArray::const_iterator itLight = mCurrentLightList.begin();

        //Test all lights against every frustum in this slice.
        for( size_t i=0; i<numLights; ++i )
        {
            const Light::LightTypes lightType = (*itLight)->getType();

            if( lightType == Light::LT_POINT || lightType == Light::LT_VPL )
            {
                //Perform 6 planes vs sphere intersection then frustum's AABB vs sphere.
                //to rule out very big spheres behind the frustum (false positives).
                //There's still a few false positives in some edge case, but it's still very good.
                //See http://www.iquilezles.org/www/articles/frustumcorrect/frustumcorrect.htm

                Vector3 scalarLightPos = (*itLight)->getParentNode()->_getDerivedPosition();
                ArrayVector3 lightPos;
                ArrayReal lightRadius;
                lightPos.setAll( scalarLightPos );
                lightRadius = Mathlib::SetAll( (*itLight)->getWorldRadius() );

                ArraySphere sphere( lightRadius, lightPos );

                for( size_t j=0; j<numPackedFrustumsPerSlice; ++j )
                {
                    const FrustumRegion frustumRegion = mFrustumRegions[frustumStartIdx + j];

                    //Test all 6 planes and AND the dot product. If one is false, then we're not visible
                    //We perform (both lines are equivalent):
                    //  plane[i].normal.dotProduct( lightPos ) + plane[i].d > -radius;
                    //  plane[i].normal.dotProduct( lightPos ) + radius > -plane[i].d;
                    ArrayReal dotResult;
                    ArrayMaskR mask;

                    dotResult = frustumRegion.plane[0].normal.dotProduct( lightPos ) + lightRadius;
                    mask = Mathlib::CompareGreater( dotResult, frustumRegion.plane[0].negD );

                    dotResult = frustumRegion.plane[1].normal.dotProduct( lightPos ) + lightRadius;
                    mask = Mathlib::And( mask, Mathlib::CompareGreater( dotResult,
                                                                        frustumRegion.plane[1].negD ) );

                    dotResult = frustumRegion.plane[2].normal.dotProduct( lightPos ) + lightRadius;
                    mask = Mathlib::And( mask, Mathlib::CompareGreater( dotResult,
                                                                        frustumRegion.plane[2].negD ) );

                    dotResult = frustumRegion.plane[3].normal.dotProduct( lightPos ) + lightRadius;
                    mask = Mathlib::And( mask, Mathlib::CompareGreater( dotResult,
                                                                        frustumRegion.plane[3].negD ) );

                    dotResult = frustumRegion.plane[4].normal.dotProduct( lightPos ) + lightRadius;
                    mask = Mathlib::And( mask, Mathlib::CompareGreater( dotResult,
                                                                        frustumRegion.plane[4].negD ) );

                    dotResult = frustumRegion.plane[5].normal.dotProduct( lightPos ) + lightRadius;
                    mask = Mathlib::And( mask, Mathlib::CompareGreater( dotResult,
                                                                        frustumRegion.plane[5].negD ) );

                    //Test the frustum's AABB vs sphere. If they don't intersect, we're not visible.
                    ArrayMaskR aabbVsSphere = sphere.intersects( frustumRegion.aabb );

                    mask = Mathlib::And( mask, aabbVsSphere );

                    const uint32 scalarMask = BooleanMask4::getScalarMask( mask );

                    for( size_t k=0; k<ARRAY_PACKED_REALS; ++k )
                    {
                        if( IS_BIT_SET( k, scalarMask ) )
                        {
                            const size_t idx = (frustumStartIdx + j) * ARRAY_PACKED_REALS + k;
                            FastArray<LightCount>::iterator numLightsInCell =
                                    mLightCountInCell.begin() + idx;

                            //assert( numLightsInCell < mLightCountInCell.end() );

                            //mLightsPerCell - 3 because three slots is reserved
                            //for the number of lights in cell per type
                            if( numLightsInCell->lightCount[0] < mLightsPerCell - 3u )
                            {
                                uint16 * RESTRICT_ALIAS cellElem = mGridBuffer + idx * mLightsPerCell +
                                        (numLightsInCell->lightCount[0] + 3u);
                                *cellElem = i * 6;
                                ++numLightsInCell->lightCount[0];
                                ++numLightsInCell->lightCount[lightType];
                            }
                        }
                    }
                }
            }
        }

        {
            //Now write all the light counts
            FastArray<LightCount>::const_iterator itor = mLightCountInCell.begin() +
                    frustumStartIdx * ARRAY_PACKED_REALS;
            FastArray<LightCount>::const_iterator end  = mLightCountInCell.begin() +
                    (frustumStartIdx + numPackedFrustumsPerSlice) * ARRAY_PACKED_REALS;

            const size_t cellSize = mLightsPerCell;
            size_t gridIdx = frustumStartIdx * ARRAY_PACKED_REALS * cellSize;

            while( itor != end )
            {
                uint32 accumLight = itor->lightCount[1];
                mGridBuffer[gridIdx+0u] = static_cast<uint16>( accumLight );
                accumLight += itor->lightCount[2];
                mGridBuffer[gridIdx+1u] = static_cast<uint16>( accumLight );
                accumLight += itor->lightCount[3];
                mGridBuffer[gridIdx+2u] = static_cast<uint16>( accumLight );
                gridIdx += cellSize;
                ++itor;
            }
        }
    }
    //-----------------------------------------------------------------------------------
    inline bool OrderLightByDistanceToCamera( const Light *left, const Light *right )
    {
        if( left->getType() != right->getType() )
            return left->getType() < right->getType();
        return left->getCachedDistanceToCameraAsReal() < right->getCachedDistanceToCameraAsReal();
    }

    void ForwardClustered::collectLights( Camera *camera )
    {
        CachedGrid *cachedGrid = 0;
        if( getCachedGridFor( camera, &cachedGrid ) )
            return; //Up to date.

        //Cull the lights against the camera. Get non-directional, non-shadow-casting lights
        //(lights set to cast shadows but currently not casting shadows are also included)
        if( mSceneManager->getCurrentShadowNode() )
        {
            const LightListInfo &globalLightList = mSceneManager->getGlobalLightList();
            const CompositorShadowNode *shadowNode = mSceneManager->getCurrentShadowNode();

            //Exclude shadow casting lights
            const LightClosestArray &shadowCastingLights = shadowNode->getShadowCastingLights();
            LightClosestArray::const_iterator itor = shadowCastingLights.begin();
            LightClosestArray::const_iterator end  = shadowCastingLights.end();

            while( itor != end )
            {
                globalLightList.lights[itor->globalIndex]->setVisible( false );
                ++itor;
            }

            mSceneManager->cullLights( camera, Light::LT_POINT, Light::NUM_LIGHT_TYPES, mCurrentLightList );

            //Restore shadow casting lights
            itor = shadowCastingLights.begin();
            end  = shadowCastingLights.end();

            while( itor != end )
            {
                globalLightList.lights[itor->globalIndex]->setVisible( true );
                ++itor;
            }
        }
        else
        {
            mSceneManager->cullLights( camera, Light::LT_POINT, Light::NUM_LIGHT_TYPES, mCurrentLightList );
        }

        const size_t numLights = mCurrentLightList.size();

        //Sort by distance to camera
        std::sort( mCurrentLightList.begin(), mCurrentLightList.end(), OrderLightByDistanceToCamera );

        //Allocate the buffers if not already.
        if( !cachedGrid->gridBuffer )
        {
            cachedGrid->gridBuffer = mVaoManager->createTexBuffer( PF_R16_UINT,
                                                                   mWidth * mHeight * mNumSlices *
                                                                   mLightsPerCell * sizeof(uint16),
                                                                   BT_DYNAMIC_PERSISTENT, 0, false );
        }

        if( !cachedGrid->globalLightListBuffer ||
            cachedGrid->globalLightListBuffer->getNumElements() < c_numBytesPerLight * numLights )
        {
            if( cachedGrid->globalLightListBuffer )
            {
                if( cachedGrid->globalLightListBuffer->getMappingState() != MS_UNMAPPED )
                    cachedGrid->globalLightListBuffer->unmap( UO_UNMAP_ALL );
                mVaoManager->destroyTexBuffer( cachedGrid->globalLightListBuffer );
            }

            cachedGrid->globalLightListBuffer = mVaoManager->createTexBuffer(
                                                                    PF_FLOAT32_RGBA,
                                                                    c_numBytesPerLight *
                                                                    std::max<size_t>( numLights, 96 ),
                                                                    BT_DYNAMIC_PERSISTENT, 0, false );
        }

        //Fill the first buffer with the light. The other buffer contains indexes into this list.
        fillGlobalLightListBuffer( camera, cachedGrid->globalLightListBuffer );

        //Fill the indexes buffer
        mGridBuffer = reinterpret_cast<uint16 * RESTRICT_ALIAS>(
                    cachedGrid->gridBuffer->map( 0, cachedGrid->gridBuffer->getNumElements() ) );

        //memset( mLightCountInCell.begin(), 0, mLightCountInCell.size() * sizeof(LightCount) );

        mCurrentCamera = camera;
        //Make sure these are up to date when calling the cached versions from multiple threads.
        mCurrentCamera->getDerivedPosition();
        mCurrentCamera->getWorldSpaceCorners();

        mSceneManager->executeUserScalableTask( this, true );

        cachedGrid->gridBuffer->unmap( UO_KEEP_PERSISTENT );
        mGridBuffer = 0;

        {
            //Check if some of the caches are really old and delete them
            CachedGridVec::iterator itor = mCachedGrid.begin();
            CachedGridVec::iterator end  = mCachedGrid.end();

            const uint32 currentFrame = mVaoManager->getFrameCount();

            while( itor != end )
            {
                if( itor->lastFrame + 3 < currentFrame )
                {
                    if( itor->gridBuffer )
                    {
                        if( itor->gridBuffer->getMappingState() != MS_UNMAPPED )
                            itor->gridBuffer->unmap( UO_UNMAP_ALL );
                        mVaoManager->destroyTexBuffer( itor->gridBuffer );
                        itor->gridBuffer = 0;
                    }

                    if( itor->globalLightListBuffer )
                    {
                        if( itor->globalLightListBuffer->getMappingState() != MS_UNMAPPED )
                            itor->globalLightListBuffer->unmap( UO_UNMAP_ALL );
                        mVaoManager->destroyTexBuffer( itor->globalLightListBuffer );
                        itor->globalLightListBuffer = 0;
                    }

                    itor = efficientVectorRemove( mCachedGrid, itor );
                    end  = mCachedGrid.end();
                }
                else
                {
                    ++itor;
                }
            }
        }
    }
    //-----------------------------------------------------------------------------------
    size_t ForwardClustered::getConstBufferSize(void) const
    {
        // (4 (vec4) + vec4 fwdScreenToGrid) * 4 bytes = 16
        return (4 + 4) * 4;
    }
    //-----------------------------------------------------------------------------------
    void ForwardClustered::fillConstBufferData( RenderTarget *renderTarget,
                                         float * RESTRICT_ALIAS passBufferPtr ) const
    {
        const float renderTargetWidth = static_cast<float>( renderTarget->getWidth() );
        const float renderTargetHeight = static_cast<float>( renderTarget->getHeight() );

        //vec4 f3dData;
        *passBufferPtr++ = mMinDistance;
        *passBufferPtr++ = mInvExponentK;
        *passBufferPtr++ = static_cast<float>( mNumSlices - 1 );
        *passBufferPtr++ = static_cast<float>( renderTargetHeight );

        //vec4 fwdScreenToGrid
        *passBufferPtr++ = static_cast<float>( mWidth ) / renderTargetWidth;
        *passBufferPtr++ = static_cast<float>( mHeight ) / renderTargetHeight;
        *passBufferPtr++ = 0;
        *passBufferPtr++ = 0;
    }
    //-----------------------------------------------------------------------------------
    void ForwardClustered::setHlmsPassProperties( Hlms *hlms )
    {
        ForwardPlusBase::setHlmsPassProperties( hlms );

        hlms->_setProperty( HlmsBaseProp::ForwardPlus, HlmsBaseProp::ForwardClustered.mHash );

        hlms->_setProperty( HlmsBaseProp::FwdClusteredWidthxHeight, mWidth * mHeight );
        hlms->_setProperty( HlmsBaseProp::FwdClusteredWidth,        mWidth );
        hlms->_setProperty( HlmsBaseProp::FwdClusteredLightsPerCell,mLightsPerCell );
    }
}
