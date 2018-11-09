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
#include "OgreViewport.h"
#include "OgreCamera.h"
#include "OgreDecal.h"

#include "Math/Array/OgreArraySphere.h"
#include "Math/Array/OgreBooleanMask.h"

#include "Compositor/OgreCompositorShadowNode.h"

#include "Vao/OgreVaoManager.h"
#include "Vao/OgreTexBufferPacked.h"

#include "Math/Array/OgreObjectMemoryManager.h"

#include "OgreHlms.h"
#include "OgreWireAabb.h"

#include "OgreProfiler.h"

namespace Ogre
{
    static const size_t c_reservedLightSlotsPerCell     = 3u;
    static const size_t c_reservedDecalsSlotsPerCell    = 1u;

    ForwardClustered::ForwardClustered( uint32 width, uint32 height,
                                        uint32 numSlices, uint32 lightsPerCell,
                                        uint32 decalsPerCell,
                                        float minDistance, float maxDistance,
                                        SceneManager *sceneManager ) :
        ForwardPlusBase( sceneManager, decalsPerCell > 0u ),
        mWidth( width ),
        mHeight( height ),
        mNumSlices( numSlices ),
        /*mWidth( 1 ),
        mHeight( 1 ),
        mNumSlices( 2 ),*/
        mReservedSlotsPerCell( ((lightsPerCell > 0u) ? 3u : 0u) + ((decalsPerCell > 0u) ? 1u : 0u) ),
        mObjsPerCell( lightsPerCell + decalsPerCell + mReservedSlotsPerCell ),
        mLightsPerCell( lightsPerCell ),
        mDecalsPerCell( decalsPerCell ),
        mGridBuffer( 0 ),
        mCurrentCamera( 0 ),
        mMinDistance( minDistance ),
        mMaxDistance( maxDistance ),
        mObjectMemoryManager( 0 ),
        mNodeMemoryManager( 0 ),
        mDebugWireAabbFrozen( false )
    {
        //SIMD optimization restriction.
        assert( (width % ARRAY_PACKED_REALS) == 0 && "Width must be multiple of ARRAY_PACKED_REALS!" );

        mLightCountInCell.resize( mNumSlices * mWidth * mHeight, LightCount() );

        // 2^( x * mNumSlices ) + mMinDistance = mMaxDistance;
        mExponentK = Math::Log2( mMaxDistance - mMinDistance ) / (Real)mNumSlices;
        mInvExponentK = 1.0f / mExponentK;

        mFrustumRegions = RawSimdUniquePtr<FrustumRegion, MEMCATEGORY_SCENE_CONTROL>(
                    (mWidth / ARRAY_PACKED_REALS) * mHeight * mNumSlices );

        mObjectMemoryManager = new ObjectMemoryManager();
        mNodeMemoryManager = new NodeMemoryManager();

        mThreadCameras.reserve( mSceneManager->getNumWorkerThreads() );
        for( size_t i=0; i<mSceneManager->getNumWorkerThreads(); ++i )
        {
            SceneNode *sceneNode = OGRE_NEW SceneNode( i, 0, mNodeMemoryManager, 0 );
            Camera *newCamera = OGRE_NEW Camera( i, mObjectMemoryManager, 0 );
            sceneNode->attachObject( newCamera );
            mThreadCameras.push_back( newCamera );
        }
    }
    //-----------------------------------------------------------------------------------
    ForwardClustered::~ForwardClustered()
    {
        setDebugFrustum( false );

        for( size_t i=mThreadCameras.size(); i--; )
        {
            SceneNode *sceneNode = mThreadCameras[i]->getParentSceneNode();
            sceneNode->detachAllObjects();
            OGRE_DELETE sceneNode;
            OGRE_DELETE mThreadCameras[i];
        }
        mThreadCameras.clear();

        delete mObjectMemoryManager;
        delete mNodeMemoryManager;

        mObjectMemoryManager = 0;
        mNodeMemoryManager = 0;
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
            collectLightForSlice( i + threadId * slicesPerThread, threadId );

        const size_t slicesRemainder = mNumSlices % numThreads;
        if( slicesRemainder > threadId )
            collectLightForSlice( threadId + numThreads * slicesPerThread, threadId );
    }
    //-----------------------------------------------------------------------------------
    void ForwardClustered::collectObjsForSlice( const size_t numPackedFrustumsPerSlice,
                                                const size_t frustumStartIdx,
                                                uint16 offsetStart,
                                                size_t minRq, size_t maxRq,
                                                size_t currObjsPerCell,
                                                size_t cellOffsetStart,
                                                ObjTypes objType,
                                                uint16 numFloat4PerObj )
    {
        const VisibleObjectsPerRq &objsPerRqInThread0 = mSceneManager->_getTmpVisibleObjectsList()[0];
        const size_t actualMaxRq = std::min( maxRq, objsPerRqInThread0.size() );
        for( size_t rqId=minRq; rqId<=actualMaxRq; ++rqId )
        {
            MovableObject::MovableObjectArray::const_iterator itor = objsPerRqInThread0[rqId].begin();
            MovableObject::MovableObjectArray::const_iterator end  = objsPerRqInThread0[rqId].end();

            while( itor != end )
            {
                MovableObject *decal = *itor;

                Node *node = decal->getParentNode();

                //Aabb localAabbScalar = decal->getLocalAabb();
                Aabb localAabbScalar;
                localAabbScalar.mCenter    = node->_getDerivedPosition();
                localAabbScalar.mHalfSize  = node->_getDerivedScale() * 0.5f;

                ArrayQuaternion objOrientation;
                objOrientation.setAll( node->_getDerivedOrientation() );

                ArrayAabb localObb;
                localObb.setAll( localAabbScalar );

                ArrayVector3 orientedHalfSize = objOrientation * localObb.mHalfSize;

                ArrayPlane obbPlane[6];
                obbPlane[0].normal= objOrientation.xAxis();
                obbPlane[0].negD  = obbPlane[0].normal.dotProduct( localObb.mCenter - orientedHalfSize );
                obbPlane[1].normal= -obbPlane[0].normal;
                obbPlane[1].negD  = obbPlane[1].normal.dotProduct( localObb.mCenter + orientedHalfSize );
                obbPlane[2].normal= objOrientation.yAxis();
                obbPlane[2].negD  = obbPlane[2].normal.dotProduct( localObb.mCenter - orientedHalfSize );
                obbPlane[3].normal= -obbPlane[2].normal;
                obbPlane[3].negD  = obbPlane[3].normal.dotProduct( localObb.mCenter + orientedHalfSize );
                obbPlane[4].normal= objOrientation.zAxis();
                obbPlane[4].negD  = obbPlane[4].normal.dotProduct( localObb.mCenter - orientedHalfSize );
                obbPlane[5].normal= -obbPlane[4].normal;
                obbPlane[5].negD  = obbPlane[5].normal.dotProduct( localObb.mCenter + orientedHalfSize );

                objOrientation = objOrientation.Inverse();

                for( size_t j=0; j<numPackedFrustumsPerSlice; ++j )
                {
                    const FrustumRegion * RESTRICT_ALIAS frustumRegion =
                            mFrustumRegions.get() + frustumStartIdx + j;

                    ArrayReal dotResult;
                    ArrayMaskR mask;
                    ArrayVector3 newPlaneNormal;

                    newPlaneNormal = objOrientation * frustumRegion->plane[0].normal;
                    dotResult = frustumRegion->plane[0].normal.dotProduct( localObb.mCenter ) +
                                newPlaneNormal.absDotProduct( localObb.mHalfSize );
                    mask = Mathlib::CompareGreater( dotResult, frustumRegion->plane[0].negD );

                    newPlaneNormal = objOrientation * frustumRegion->plane[1].normal;
                    dotResult = frustumRegion->plane[1].normal.dotProduct( localObb.mCenter ) +
                                newPlaneNormal.absDotProduct( localObb.mHalfSize );
                    mask = Mathlib::And( mask, Mathlib::CompareGreater( dotResult,
                                                                        frustumRegion->plane[1].negD ) );

                    newPlaneNormal = objOrientation * frustumRegion->plane[2].normal;
                    dotResult = frustumRegion->plane[2].normal.dotProduct( localObb.mCenter ) +
                                newPlaneNormal.absDotProduct( localObb.mHalfSize );
                    mask = Mathlib::And( mask, Mathlib::CompareGreater( dotResult,
                                                                        frustumRegion->plane[2].negD ) );

                    newPlaneNormal = objOrientation * frustumRegion->plane[3].normal;
                    dotResult = frustumRegion->plane[3].normal.dotProduct( localObb.mCenter ) +
                                newPlaneNormal.absDotProduct( localObb.mHalfSize );
                    mask = Mathlib::And( mask, Mathlib::CompareGreater( dotResult,
                                                                        frustumRegion->plane[3].negD ) );

                    newPlaneNormal = objOrientation * frustumRegion->plane[4].normal;
                    dotResult = frustumRegion->plane[4].normal.dotProduct( localObb.mCenter ) +
                                newPlaneNormal.absDotProduct( localObb.mHalfSize );
                    mask = Mathlib::And( mask, Mathlib::CompareGreater( dotResult,
                                                                        frustumRegion->plane[4].negD ) );

                    newPlaneNormal = objOrientation * frustumRegion->plane[5].normal;
                    dotResult = frustumRegion->plane[5].normal.dotProduct( localObb.mCenter ) +
                                newPlaneNormal.absDotProduct( localObb.mHalfSize );
                    mask = Mathlib::And( mask, Mathlib::CompareGreater( dotResult,
                                                                        frustumRegion->plane[5].negD ) );

                    if( BooleanMask4::getScalarMask( mask ) != 0 )
                    {
                        //Test all 8 frustum corners against each of the 6 obb planes.
                        for( int k=0; k<6; ++k )
                        {
                            ArrayMaskR vertexMask = ARRAY_MASK_ZERO;

                            for( int l=0; l<8; ++l )
                            {
                                dotResult = obbPlane[k].normal.dotProduct(
                                                frustumRegion->corners[l] ) - obbPlane[k].negD;
                                vertexMask = Mathlib::Or( vertexMask,
                                                          Mathlib::CompareGreater( dotResult,
                                                                                   ARRAY_REAL_ZERO ) );
                            }

                            mask = Mathlib::And( mask, vertexMask );
                        }
                    }

                    const uint32 scalarMask = BooleanMask4::getScalarMask( mask );

                    for( size_t k=0; k<ARRAY_PACKED_REALS; ++k )
                    {
                        if( IS_BIT_SET( k, scalarMask ) )
                        {
                            const size_t idx = (frustumStartIdx + j) * ARRAY_PACKED_REALS + k;
                            FastArray<LightCount>::iterator numLightsInCell =
                                    mLightCountInCell.begin() + idx;

                            //assert( numLightsInCell < mLightCountInCell.end() );

                            if( numLightsInCell->objCount[objType] < currObjsPerCell )
                            {
                                uint16 * RESTRICT_ALIAS cellElem = mGridBuffer + idx * mObjsPerCell +
                                                                   cellOffsetStart +
                                                                   numLightsInCell->objCount[objType];
                                *cellElem = offsetStart;
                                ++numLightsInCell->objCount[objType];
                            }
                        }
                    }
                }

                offsetStart += numFloat4PerObj;
                ++itor;
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void ForwardClustered::collectLightForSlice( size_t slice, size_t threadId )
    {
        const size_t frustumStartIdx = slice * (mWidth / ARRAY_PACKED_REALS) * mHeight;

        Real nearDepthAtSlice = -getDepthAtSlice( slice );
        Real farDepthAtSlice  = -getDepthAtSlice( slice + 1 );

        if( slice == 0 )
            nearDepthAtSlice = mCurrentCamera->getNearClipDistance();

        if( slice == mNumSlices - 1u )
            farDepthAtSlice = Ogre::max( mCurrentCamera->getFarClipDistance(), farDepthAtSlice );

        Camera *camera = mThreadCameras[threadId];

        camera->resetFrustumExtents();
        camera->setPosition( mCurrentCamera->_getCachedRealPosition() );
        camera->setOrientation( mCurrentCamera->_getCachedRealOrientation() );

        camera->setProjectionType( mCurrentCamera->getProjectionType() );
        camera->setAspectRatio( mCurrentCamera->getAspectRatio() );
        camera->setOrthoWindowHeight( mCurrentCamera->getOrthoWindowHeight() );
#if OGRE_NO_VIEWPORT_ORIENTATIONMODE == 0
        camera->setOrientationMode( mCurrentCamera->getOrientationMode() );
#endif
        if( !mCurrentCamera->isReflected() && camera->isReflected() )
            camera->disableReflection();
        else if( mCurrentCamera->isReflected() )
            camera->enableReflection( mCurrentCamera->getReflectionPlane() );

        Real origFrustumLeft, origFrustumRight, origFrustumTop, origFrustumBottom;
        mCurrentCamera->getFrustumExtents(origFrustumLeft, origFrustumRight,
                                          origFrustumTop, origFrustumBottom, FET_TAN_HALF_ANGLES);
        camera->setFrustumExtents(origFrustumLeft, origFrustumRight,
                                          origFrustumTop, origFrustumBottom, FET_TAN_HALF_ANGLES);

        camera->setNearClipDistance( nearDepthAtSlice );
        camera->setFarClipDistance( farDepthAtSlice );

        camera->getFrustumExtents( origFrustumLeft, origFrustumRight,
                                   origFrustumTop, origFrustumBottom, FET_PROJ_PLANE_POS );

        const Real frustumHorizLength = (origFrustumRight - origFrustumLeft) / (Real)mWidth;
        const Real frustumVertLength = (origFrustumTop - origFrustumBottom) / (Real)mHeight;

        for( size_t y=0; y<mHeight; ++y )
        {
            const Real yStep = static_cast<Real>( y );

            for( size_t x=0; x<mWidth / ARRAY_PACKED_REALS; ++x )
            {
                for( size_t i=0; i<ARRAY_PACKED_REALS; ++i )
                {
                    const Real xStep = static_cast<Real>( x * ARRAY_PACKED_REALS + i);

                    const Real newLeft   = origFrustumLeft + xStep * frustumHorizLength;
                    const Real newRight  = newLeft + frustumHorizLength;
                    const Real newBottom = origFrustumBottom + yStep * frustumVertLength;
                    const Real newTop    = newBottom + frustumVertLength;

                    camera->setFrustumExtents( newLeft, newRight, newTop, newBottom, FET_PROJ_PLANE_POS );

                    const Vector3 *wsCorners = camera->getWorldSpaceCorners();

                    FrustumRegion &frustumRegion = mFrustumRegions.get()[frustumStartIdx + y *
                            (mWidth / ARRAY_PACKED_REALS) + x];
                    {
                        Aabb planeAabb( wsCorners[0], Vector3::ZERO );
                        frustumRegion.corners[0].setAll( wsCorners[0] );
                        for( int j=1; j<8; ++j )
                        {
                            planeAabb.merge( wsCorners[j] );
                            frustumRegion.corners[j].setFromVector3( wsCorners[j], i );
                        }
                        frustumRegion.aabb.setFromAabb( planeAabb, i );
                    }

                    const Plane *planes = camera->getFrustumPlanes();
                    for( int j=0; j<6; ++j )
                    {
                        frustumRegion.plane[j].normal.setFromVector3( planes[j].normal, i );
                        Mathlib::Set( frustumRegion.plane[j].negD, -planes[j].d, i );
                    }
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
                lightRadius = Mathlib::SetAll( (*itLight)->getAttenuationRange() );

                ArraySphere sphere( lightRadius, lightPos );

                for( size_t j=0; j<numPackedFrustumsPerSlice; ++j )
                {
                    const FrustumRegion * RESTRICT_ALIAS frustumRegion =
                            mFrustumRegions.get() + frustumStartIdx + j;

                    //Test all 6 planes and AND the dot product. If one is false, then we're not visible
                    //We perform (both lines are equivalent):
                    //  plane[i].normal.dotProduct( lightPos ) + plane[i].d > -radius;
                    //  plane[i].normal.dotProduct( lightPos ) + radius > -plane[i].d;
                    ArrayReal dotResult;
                    ArrayMaskR mask;

                    dotResult = frustumRegion->plane[0].normal.dotProduct( lightPos ) + lightRadius;
                    mask = Mathlib::CompareGreater( dotResult, frustumRegion->plane[0].negD );

                    dotResult = frustumRegion->plane[1].normal.dotProduct( lightPos ) + lightRadius;
                    mask = Mathlib::And( mask, Mathlib::CompareGreater( dotResult,
                                                                        frustumRegion->plane[1].negD ) );

                    dotResult = frustumRegion->plane[2].normal.dotProduct( lightPos ) + lightRadius;
                    mask = Mathlib::And( mask, Mathlib::CompareGreater( dotResult,
                                                                        frustumRegion->plane[2].negD ) );

                    dotResult = frustumRegion->plane[3].normal.dotProduct( lightPos ) + lightRadius;
                    mask = Mathlib::And( mask, Mathlib::CompareGreater( dotResult,
                                                                        frustumRegion->plane[3].negD ) );

                    dotResult = frustumRegion->plane[4].normal.dotProduct( lightPos ) + lightRadius;
                    mask = Mathlib::And( mask, Mathlib::CompareGreater( dotResult,
                                                                        frustumRegion->plane[4].negD ) );

                    dotResult = frustumRegion->plane[5].normal.dotProduct( lightPos ) + lightRadius;
                    mask = Mathlib::And( mask, Mathlib::CompareGreater( dotResult,
                                                                        frustumRegion->plane[5].negD ) );

                    //Test the frustum's AABB vs sphere. If they don't intersect, we're not visible.
                    ArrayMaskR aabbVsSphere = sphere.intersects( frustumRegion->aabb );

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

                            if( numLightsInCell->lightCount[0] < mLightsPerCell )
                            {
                                uint16 * RESTRICT_ALIAS cellElem = mGridBuffer + idx * mObjsPerCell +
                                                                   (numLightsInCell->lightCount[0] +
                                                                   c_reservedLightSlotsPerCell);
                                *cellElem = static_cast<uint16>( i * c_ForwardPlusNumFloat4PerLight );
                                ++numLightsInCell->lightCount[0];
                                ++numLightsInCell->lightCount[lightType];
                            }
                        }
                    }
                }
            }
            else
            {
                //Spotlight. Do pyramid vs frustum intersection. This pyramid
                //has 5 sides and encloses the spotlight's cone.
                //See www.yosoygames.com.ar/wp/2016/12/
                //frustum-vs-pyramid-intersection-also-frustum-vs-frustum/

                Node *lightNode = (*itLight)->getParentNode();

                //Generate the 5 pyramid vertices
                const Real lightRange = (*itLight)->getAttenuationRange();
                const Real lenOpposite = (*itLight)->getSpotlightTanHalfAngle() * lightRange;

                Vector3 leftCorner = lightNode->_getDerivedOrientation() *
                        Vector3( -lenOpposite, lenOpposite, 0 );
                Vector3 rightCorner = lightNode->_getDerivedOrientation() *
                        Vector3( lenOpposite, lenOpposite, 0 );

                Vector3 scalarLightPos = (*itLight)->getParentNode()->_getDerivedPosition();
                Vector3 scalarLightDir = (*itLight)->getDerivedDirection() * lightRange;

                Plane scalarPlane[6];

                scalarPlane[FRUSTUM_PLANE_FAR] = Plane( scalarLightPos + scalarLightDir + leftCorner,
                                                        scalarLightPos + scalarLightDir,
                                                        scalarLightPos + scalarLightDir + rightCorner );
                scalarPlane[FRUSTUM_PLANE_NEAR] = Plane( -scalarPlane[FRUSTUM_PLANE_FAR].normal,
                                                         scalarLightPos );

                scalarPlane[FRUSTUM_PLANE_LEFT] = Plane( scalarLightPos + scalarLightDir - rightCorner,
                                                         scalarLightPos + scalarLightDir + leftCorner,
                                                         scalarLightPos );
                scalarPlane[FRUSTUM_PLANE_RIGHT]= Plane( scalarLightPos + scalarLightDir + rightCorner,
                                                         scalarLightPos + scalarLightDir - leftCorner,
                                                         scalarLightPos );

                scalarPlane[FRUSTUM_PLANE_TOP]  = Plane( scalarLightPos + scalarLightDir + leftCorner,
                                                         scalarLightPos + scalarLightDir + rightCorner,
                                                         scalarLightPos );
                scalarPlane[FRUSTUM_PLANE_BOTTOM]= Plane( scalarLightPos + scalarLightDir - leftCorner,
                                                          scalarLightPos + scalarLightDir - rightCorner,
                                                          scalarLightPos );

                ArrayPlane pyramidPlane[6];
                pyramidPlane[0].normal.setAll( scalarPlane[0].normal );
                pyramidPlane[0].negD = Mathlib::SetAll( -scalarPlane[0].d );
                pyramidPlane[1].normal.setAll( scalarPlane[1].normal );
                pyramidPlane[1].negD = Mathlib::SetAll( -scalarPlane[1].d );
                pyramidPlane[2].normal.setAll( scalarPlane[2].normal );
                pyramidPlane[2].negD = Mathlib::SetAll( -scalarPlane[2].d );
                pyramidPlane[3].normal.setAll( scalarPlane[3].normal );
                pyramidPlane[3].negD = Mathlib::SetAll( -scalarPlane[3].d );
                pyramidPlane[4].normal.setAll( scalarPlane[4].normal );
                pyramidPlane[4].negD = Mathlib::SetAll( -scalarPlane[4].d );
                pyramidPlane[5].normal.setAll( scalarPlane[5].normal );
                pyramidPlane[5].negD = Mathlib::SetAll( -scalarPlane[5].d );

                ArrayVector3 pyramidVertex[5];

                pyramidVertex[0].setAll( scalarLightPos );
                pyramidVertex[1].setAll( scalarLightPos + scalarLightDir + leftCorner );
                pyramidVertex[2].setAll( scalarLightPos + scalarLightDir + rightCorner );
                pyramidVertex[3].setAll( scalarLightPos + scalarLightDir - leftCorner );
                pyramidVertex[4].setAll( scalarLightPos + scalarLightDir - rightCorner );

                for( size_t j=0; j<numPackedFrustumsPerSlice; ++j )
                {
                    const FrustumRegion * RESTRICT_ALIAS frustumRegion =
                            mFrustumRegions.get() + frustumStartIdx + j;

                    ArrayReal dotResult;
                    ArrayMaskR mask;

                    mask = BooleanMask4::getAllSetMask();

                    //There is no intersection if for at least one of the 12 planes
                    //(6+6) all the vertices (5+8 verts.) are on the negative side.

                    //Test all 5 pyramid vertices against each of the 6 frustum planes.
                    for( int k=0; k<6; ++k )
                    {
                        ArrayMaskR vertexMask = ARRAY_MASK_ZERO;

                        for( int l=0; l<5; ++l )
                        {
                            dotResult = frustumRegion->plane[k].normal.dotProduct( pyramidVertex[l] ) -
                                        frustumRegion->plane[k].negD;
                            vertexMask = Mathlib::Or( vertexMask,
                                                      Mathlib::CompareGreater( dotResult,
                                                                               ARRAY_REAL_ZERO ) );
                        }

                        mask = Mathlib::And( mask, vertexMask );
                    }

                    if( BooleanMask4::getScalarMask( mask ) != 0 )
                    {
                        //Test all 8 frustum corners against each of the 6 pyramid planes.
                        for( int k=0; k<6; ++k )
                        {
                            ArrayMaskR vertexMask = ARRAY_MASK_ZERO;

                            for( int l=0; l<8; ++l )
                            {
                                dotResult = pyramidPlane[k].normal.dotProduct(
                                            frustumRegion->corners[l] ) - pyramidPlane[k].negD;
                                vertexMask = Mathlib::Or( vertexMask,
                                                          Mathlib::CompareGreater( dotResult,
                                                                                   ARRAY_REAL_ZERO ) );
                            }

                            mask = Mathlib::And( mask, vertexMask );
                        }
                    }

                    const uint32 scalarMask = BooleanMask4::getScalarMask( mask );

                    for( size_t k=0; k<ARRAY_PACKED_REALS; ++k )
                    {
                        if( IS_BIT_SET( k, scalarMask ) )
                        {
                            const size_t idx = (frustumStartIdx + j) * ARRAY_PACKED_REALS + k;
                            FastArray<LightCount>::iterator numLightsInCell =
                                    mLightCountInCell.begin() + idx;

                            //assert( numLightsInCell < mLightCountInCell.end() );

                            if( numLightsInCell->lightCount[0] < mLightsPerCell )
                            {
                                uint16 * RESTRICT_ALIAS cellElem = mGridBuffer + idx * mObjsPerCell +
                                                                   (numLightsInCell->lightCount[0] +
                                                                   c_reservedLightSlotsPerCell);
                                *cellElem = static_cast<uint16>( i * c_ForwardPlusNumFloat4PerLight );
                                ++numLightsInCell->lightCount[0];
                                ++numLightsInCell->lightCount[lightType];
                            }
                        }
                    }
                }
            }

            ++itLight;
        }

        const bool hasDecals = mDecalsEnabled;
        const size_t decalOffsetStart = mLightsPerCell + c_reservedLightSlotsPerCell;

        const VisibleObjectsPerRq &objsPerRqInThread0 = mSceneManager->_getTmpVisibleObjectsList()[0];
        const size_t actualMaxDecalRq = std::min( MaxDecalRq, objsPerRqInThread0.size() );
        collectObjsForSlice( numPackedFrustumsPerSlice, frustumStartIdx,
                             mDecalFloat4Offset, MinDecalRq, actualMaxDecalRq,
                             mDecalsPerCell,
                             decalOffsetStart + c_reservedDecalsSlotsPerCell,
                             ObjType_Decal, (uint16)c_ForwardPlusNumFloat4PerDecal );

        {
            //Now write all the light counts
            FastArray<LightCount>::const_iterator itor = mLightCountInCell.begin() +
                    frustumStartIdx * ARRAY_PACKED_REALS;
            FastArray<LightCount>::const_iterator end  = mLightCountInCell.begin() +
                    (frustumStartIdx + numPackedFrustumsPerSlice) * ARRAY_PACKED_REALS;

            const size_t cellSize = mObjsPerCell;
            //const bool hasLights = mLightsPerCell > 0u;
            const bool hasLights = true;
            size_t gridIdx = frustumStartIdx * ARRAY_PACKED_REALS * cellSize;

            while( itor != end )
            {
                uint32 accumLight = itor->lightCount[1];
                if( hasLights )
                {
                    mGridBuffer[gridIdx+0u] = static_cast<uint16>( accumLight );
                    accumLight += itor->lightCount[2];
                    mGridBuffer[gridIdx+1u] = static_cast<uint16>( accumLight );
                    accumLight += itor->lightCount[3];
                    mGridBuffer[gridIdx+2u] = static_cast<uint16>( accumLight );
                }
                if( hasDecals )
                {
                    mGridBuffer[gridIdx+decalOffsetStart+0u] =
                            static_cast<uint16>( itor->objCount[ObjType_Decal] );
                }
                gridIdx += cellSize;
                ++itor;
            }
        }
    }
    //-----------------------------------------------------------------------------------
    inline bool OrderObjsByDistanceToCamera( const MovableObject *left, const MovableObject *right )
    {
        return left->getCachedDistanceToCameraAsReal() < right->getCachedDistanceToCameraAsReal();
    }

    void ForwardClustered::collectObjs( const Camera *camera, size_t &outNumDecals )
    {
        size_t numDecals = 0;

        const bool didCollect = mSceneManager->_collectForwardPlusObjects( camera );

        VisibleObjectsPerThreadArray &objsPerThread = mSceneManager->_getTmpVisibleObjectsList();
        VisibleObjectsPerRq &objsPerRqInThread0 = *objsPerThread.begin();

        if( didCollect )
        {
            //Merge objects collected in all threads into just thread0
            VisibleObjectsPerThreadArray::const_iterator itor = objsPerThread.begin() + 1u;
            VisibleObjectsPerThreadArray::const_iterator end  = objsPerThread.end();

            while( itor != end )
            {
                const size_t numRqs = objsPerRqInThread0.size();

                const VisibleObjectsPerRq &objsPerRq = *itor;

                OGRE_ASSERT_MEDIUM( numRqs == objsPerRq.size() );

                for( size_t rqId=0; rqId<numRqs; ++rqId )
                {
                    objsPerRqInThread0[rqId].appendPOD( objsPerRq[rqId].begin(),
                                                        objsPerRq[rqId].end() );
                }

                ++itor;
            }

            //Sort the objects by distance to camera
            const size_t numRqs = objsPerRqInThread0.size();
            for( size_t rqId=0; rqId<numRqs; ++rqId )
            {
                if( MinDecalRq >= rqId && rqId <= MaxDecalRq )
                    numDecals += objsPerRqInThread0[rqId].size();

                std::sort( objsPerRqInThread0[rqId].begin(), objsPerRqInThread0[rqId].end(),
                           OrderObjsByDistanceToCamera );
            }
        }
        else
        {
            const size_t numRqs = objsPerRqInThread0.size();
            for( size_t rqId=0; rqId<numRqs; ++rqId )
                objsPerRqInThread0[rqId].clear();
        }

        outNumDecals = numDecals;
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

        OgreProfile( "Forward Clustered Light Collect" );

        //Cull the lights against the camera. Get non-directional, non-shadow-casting lights
        //(lights set to cast shadows but currently not casting shadows are also included)
        if( mSceneManager->getCurrentShadowNode() )
        {
            //const LightListInfo &globalLightList = mSceneManager->getGlobalLightList();
            const CompositorShadowNode *shadowNode = mSceneManager->getCurrentShadowNode();

            //Exclude shadow casting lights
            const LightClosestArray &shadowCastingLights = shadowNode->getShadowCastingLights();
            LightClosestArray::const_iterator itor = shadowCastingLights.begin();
            LightClosestArray::const_iterator end  = shadowCastingLights.end();

            while( itor != end )
            {
                if( itor->light )
                    itor->light->setVisible( false );
                ++itor;
            }

            mSceneManager->cullLights( camera, Light::LT_POINT,
                                       Light::MAX_FORWARD_PLUS_LIGHTS, mCurrentLightList );

            //Restore shadow casting lights
            itor = shadowCastingLights.begin();
            end  = shadowCastingLights.end();

            while( itor != end )
            {
                if( itor->light )
                    itor->light->setVisible( true );
                ++itor;
            }
        }
        else
        {
            mSceneManager->cullLights( camera, Light::LT_POINT,
                                       Light::MAX_FORWARD_PLUS_LIGHTS, mCurrentLightList );
        }

        size_t numDecals;
        collectObjs( camera, numDecals );

        const size_t numLights = mCurrentLightList.size();

        //Sort by distance to camera
        std::sort( mCurrentLightList.begin(), mCurrentLightList.end(), OrderLightByDistanceToCamera );

        //Allocate the buffers if not already.
        CachedGridBuffer &gridBuffers = cachedGrid->gridBuffers[cachedGrid->currentBufIdx];
        if( !gridBuffers.gridBuffer )
        {
            gridBuffers.gridBuffer = mVaoManager->createTexBuffer( PF_R16_UINT,
                                                                   mWidth * mHeight * mNumSlices *
                                                                   mObjsPerCell * sizeof(uint16),
                                                                   BT_DYNAMIC_PERSISTENT, 0, false );
        }

        const size_t bufferBytesNeeded = calculateBytesNeeded( std::max<size_t>( numLights, 96u ),
                                                               std::max<size_t>( numDecals, 16u ) );
        if( !gridBuffers.globalLightListBuffer ||
            gridBuffers.globalLightListBuffer->getNumElements() < bufferBytesNeeded )
        {
            if( gridBuffers.globalLightListBuffer )
            {
                if( gridBuffers.globalLightListBuffer->getMappingState() != MS_UNMAPPED )
                    gridBuffers.globalLightListBuffer->unmap( UO_UNMAP_ALL );
                mVaoManager->destroyTexBuffer( gridBuffers.globalLightListBuffer );
            }

            gridBuffers.globalLightListBuffer = mVaoManager->createTexBuffer(
                                                                    PF_FLOAT32_RGBA,
                                                                    bufferBytesNeeded,
                                                                    BT_DYNAMIC_PERSISTENT, 0, false );
        }

        //Fill the first buffer with the light. The other buffer contains indexes into this list.
        fillGlobalLightListBuffer( camera, gridBuffers.globalLightListBuffer );

        //Fill the indexes buffer
        mGridBuffer = reinterpret_cast<uint16 * RESTRICT_ALIAS>(
                    gridBuffers.gridBuffer->map( 0, gridBuffers.gridBuffer->getNumElements() ) );

        //memset( mLightCountInCell.begin(), 0, mLightCountInCell.size() * sizeof(LightCount) );

        mCurrentCamera = camera;
        //Make sure these are up to date when calling the cached versions from multiple threads.
        mCurrentCamera->getDerivedPosition();
        mCurrentCamera->getWorldSpaceCorners();

        mSceneManager->executeUserScalableTask( this, true );

        if( !mDebugWireAabb.empty() && !mDebugWireAabbFrozen )
        {
            //std::cout << "Start" << std::endl;
            const size_t numFrustumRegions = mFrustumRegions.size();
            for( size_t i=0; i<numFrustumRegions; ++i )
            {
                for( size_t j=0; j<ARRAY_PACKED_REALS; ++j )
                {
                    Aabb aabb = mFrustumRegions.get()[i].aabb.getAsAabb( j );
                    mDebugWireAabb[i*ARRAY_PACKED_REALS+j]->setToAabb( aabb );
                    mDebugWireAabb[i*ARRAY_PACKED_REALS+j]->getParentNode()->_getFullTransformUpdated();

                    //std::cout << aabb.mCenter << aabb.mHalfSize << std::endl;
                }
            }
        }

        gridBuffers.gridBuffer->unmap( UO_KEEP_PERSISTENT );
        mGridBuffer = 0;

        deleteOldGridBuffers();
    }
    //-----------------------------------------------------------------------------------
    size_t ForwardClustered::getConstBufferSize(void) const
    {
        // (4 (vec4) + vec4 fwdScreenToGrid) * 4 bytes = 16
        return (4 + 4) * 4;
    }
    //-----------------------------------------------------------------------------------
    void ForwardClustered::fillConstBufferData( Viewport *viewport, RenderTarget* renderTarget,
                                                IdString shaderSyntax,
                                                float * RESTRICT_ALIAS passBufferPtr ) const
    {
        const float viewportWidth = static_cast<float>( viewport->getActualWidth());
        const float viewportHeight = static_cast<float>( viewport->getActualHeight() );
        const float viewportWidthOffset = static_cast<float>( viewport->getActualLeft() );
        float viewportHeightOffset = static_cast<float>( viewport->getActualTop() );

        //The way ogre represents viewports is top = 0 bottom = 1. As a result if 'texture flipping'
        //is required then all is ok. However if it is not required then viewport offsets are
        //actually represented from the bottom up.
        //As a result we need convert our viewport height offsets to work bottom up instead of top down;
        //This is compounded by OpenGL standard being different to DirectX and Metal
        if ( !renderTarget->requiresTextureFlipping() && shaderSyntax == "glsl" )
        {
            viewportHeightOffset =
                    static_cast<float>( (1.0 - (viewport->getTop() + viewport->getHeight())) *
                                        renderTarget->getHeight() );
        }

        //vec4 f3dData;
        *passBufferPtr++ = mMinDistance;
        *passBufferPtr++ = mInvExponentK;
        *passBufferPtr++ = static_cast<float>( mNumSlices - 1 );
        *passBufferPtr++ = static_cast<float>( viewportHeight );

        //vec4 fwdScreenToGrid
        *passBufferPtr++ = static_cast<float>( mWidth ) / viewportWidth;
        *passBufferPtr++ = static_cast<float>( mHeight ) / viewportHeight;
        *passBufferPtr++ = viewportWidthOffset;
        *passBufferPtr++ = viewportHeightOffset;
    }
    //-----------------------------------------------------------------------------------
    void ForwardClustered::setHlmsPassProperties( Hlms *hlms )
    {
        ForwardPlusBase::setHlmsPassProperties( hlms );

        hlms->_setProperty( HlmsBaseProp::ForwardPlus, HlmsBaseProp::ForwardClustered.mHash );

        hlms->_setProperty( HlmsBaseProp::FwdClusteredWidthxHeight, mWidth * mHeight );
        hlms->_setProperty( HlmsBaseProp::FwdClusteredWidth,        mWidth );
        hlms->_setProperty( HlmsBaseProp::FwdClusteredLightsPerCell,mObjsPerCell );

        if( mDecalsEnabled )
        {
            const PrePassMode prePassMode = mSceneManager->getCurrentPrePassMode();
            int32 numDecalsTex = 0;
            if( mSceneManager->getDecalsDiffuse() && prePassMode != PrePassCreate )
            {
                hlms->_setProperty( HlmsBaseProp::DecalsDiffuse,    1 );
                ++numDecalsTex;
            }
            if( mSceneManager->getDecalsNormals() && prePassMode != PrePassUse )
            {
                hlms->_setProperty( HlmsBaseProp::DecalsNormals,    2 );
                ++numDecalsTex;
            }
            if( mSceneManager->getDecalsEmissive() && prePassMode != PrePassCreate )
            {
                bool mergedTex = mSceneManager->getDecalsDiffuse() == mSceneManager->getDecalsEmissive();
                hlms->_setProperty( HlmsBaseProp::DecalsEmissive,   mergedTex ? 1 : 3 );
                ++numDecalsTex;
            }

            const size_t decalOffsetStart = mLightsPerCell + c_reservedLightSlotsPerCell;
            hlms->_setProperty( HlmsBaseProp::FwdPlusDecalsSlotOffset,
                                static_cast<int32>( decalOffsetStart ) );

            hlms->_setProperty( HlmsBaseProp::EnableDecals, numDecalsTex );
        }
    }
    //-----------------------------------------------------------------------------------
    void ForwardClustered::setDebugFrustum( bool bEnableDebugFrustumWireAabb )
    {
        if( bEnableDebugFrustumWireAabb )
        {
            if( getDebugFrustum() )
                setDebugFrustum( false );

            const size_t numDebugWires = mWidth * mHeight * mNumSlices;
            mDebugWireAabb.reserve( numDebugWires );
            SceneNode *rootNode = mSceneManager->getRootSceneNode();
            for( size_t i=0; i<numDebugWires; ++i )
            {
                mDebugWireAabb.push_back( mSceneManager->createWireAabb() );
                rootNode->createChildSceneNode()->attachObject( mDebugWireAabb.back() );
            }
        }
        else
        {
            //LIFO order for optimum cleanup perfomance
            vector<WireAabb*>::type::const_reverse_iterator ritor = mDebugWireAabb.rbegin();
            vector<WireAabb*>::type::const_reverse_iterator rend  = mDebugWireAabb.rend();

            while( ritor != rend )
            {
                SceneNode *sceneNode = (*ritor)->getParentSceneNode();
                sceneNode->getParentSceneNode()->removeAndDestroyChild( sceneNode );
                mSceneManager->destroyWireAabb( *ritor );
                ++ritor;
            }

            mDebugWireAabb.clear();
        }
    }
    //-----------------------------------------------------------------------------------
    bool ForwardClustered::getDebugFrustum(void) const
    {
        return !mDebugWireAabb.empty();
    }
    //-----------------------------------------------------------------------------------
    void ForwardClustered::setFreezeDebugFrustum( bool freezeDebugFrustum )
    {
        mDebugWireAabbFrozen = freezeDebugFrustum;
    }
    //-----------------------------------------------------------------------------------
    bool ForwardClustered::getFreezeDebugFrustum(void) const
    {
        return mDebugWireAabbFrozen;
    }
}
