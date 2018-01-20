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

#include "InstantRadiosity/OgreInstantRadiosity.h"
#include "OgreIrradianceVolume.h"
#include "OgreHlmsPbsDatablock.h"
#include "OgreHlmsPbs.h"
#include "OgreHlmsManager.h"

#include "OgreRay.h"
#include "OgreLight.h"
#include "OgreSceneManager.h"
#include "Vao/OgreAsyncTicket.h"

#include "Math/Array/OgreBooleanMask.h"

#include "OgreItem.h"
#include "OgreLwString.h"

#include "OgreBitwise.h"
#include "OgreTextureManager.h"
#include "OgreHardwarePixelBuffer.h"

#if (OGRE_COMPILER == OGRE_COMPILER_MSVC ||\
    OGRE_PLATFORM == OGRE_PLATFORM_APPLE ||\
    OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS)
    #include <random>
#else
    #include <tr1/random>
#endif

namespace Ogre
{
    class RandomNumberGenerator
    {
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS || ( OGRE_COMPILER == OGRE_COMPILER_MSVC && OGRE_COMP_VER >= 1910 )
        std::mt19937        mRng;
#else
        std::tr1::mt19937   mRng;
#endif

    public:
        uint32 rand()       { return mRng(); }

        /// Returns value in range [0; 1]
        Real saturatedRand()
        {
            return rand() / (Real)mRng.max();
        }

        /// Returns value in range [-1; 1]
        Real boxRand()
        {
            return saturatedRand() * Real(2.0) - Real(1.0);
        }

        Real rangeRand( Real min, Real max )
        {
            return saturatedRand() * (max - min) + min;
        }

        Vector3 getRandomDir(void)
        {
            const Real theta= Real(2.0) * Math::PI * saturatedRand();
            const Real z    = boxRand();

            const Real sharedTerm = Math::Sqrt( Real(1.0) - z * z );

            Vector3 retVal;
            retVal.x = sharedTerm * Math::Cos( theta );
            retVal.y = sharedTerm * Math::Sin( theta );
            retVal.z = z;
//            Vector3 retVal;
//            retVal.x = boxRand();
//            retVal.y = boxRand();
//            retVal.z = boxRand();
//            retVal.normalise();

            return retVal;
        }

        /// Returns values in range [-1; 1] both XY, inside a circle of radius 1.
        Vector2 getRandomPointInCircle(void)
        {
            const Real theta= Real(2.0) * Math::PI * saturatedRand();
            const Real r    = saturatedRand();

            const Real sqrtR = Math::Sqrt( r );

            Vector2 retVal;
            retVal.x = sqrtR * Math::Cos( theta );
            retVal.y = sqrtR * Math::Sin( theta );

            return retVal;
        }

        Vector3 getRandomDirInRange( Radian angle )
        {
            const Real theta= Real(2.0) * Math::PI * saturatedRand();
            const Real z    = rangeRand( Math::Cos( angle ), 1.0f );

            const Real sharedTerm = Math::Sqrt( Real(1.0) - z * z );

            Vector3 retVal;
            retVal.x = sharedTerm * Math::Cos( theta );
            retVal.y = sharedTerm * Math::Sin( theta );
            retVal.z = z;

            return retVal;
        }

        Vector3 randomizeDirAroundCone( const Vector3 vDir, Radian angle )
        {
            Vector3 vUp = vDir.perpendicular();
            Vector3 vRight = vUp.crossProduct( vDir );
            Quaternion qRot;
            qRot.FromAxes( vRight, vUp, vDir );
            Vector3 retVal = qRot * getRandomDirInRange( angle );
            return retVal;
        }
    };
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    InstantRadiosity::InstantRadiosity( SceneManager *sceneManager, HlmsManager *hlmsManager ) :
        mSceneManager( sceneManager ),
        mHlmsManager( hlmsManager ),
        mFirstRq( 0 ),
        mLastRq( 255 ),
        mVisibilityMask( 0xffffffff ),
        mLightMask( 0xffffffff ),
        //mNumRays( 10000 ),
        mNumRays( 128 ),
        mNumRayBounces( 1 ),
        mSurvivingRayFraction( 0.5f ),
        mCellSize( 3 ),
        mBias( 0.982f ),
        mNumSpreadIterations( 1 ),
        mSpreadThreshold( 0.0004 ),
        mVplMaxRange( 8 ),
        mVplConstAtten( 0.5 ),
        mVplLinearAtten( 0.5 ),
        mVplQuadAtten( 0 ),
        mVplThreshold( 0.0005f ),
        mVplPowerBoost( 1.4f ),
        mVplUseIntensityForMaxRange( true ),
        mVplIntensityRangeMultiplier( 100.0 ),
        mMipmapBias( 0 ),
        mTotalNumRays( 0 ),
        mEnableDebugMarkers( false ),
        mUseTextures( true ),
        mUseIrradianceVolume( false )
    {
    }
    //-----------------------------------------------------------------------------------
    InstantRadiosity::~InstantRadiosity()
    {
        freeMemory();
        clear();
    }
    //-----------------------------------------------------------------------------------
    bool InstantRadiosity::OrderRenderOperation::operator () ( const v1::RenderOperation &_l,
                                                               const v1::RenderOperation &_r ) const
    {
        return  _l.vertexData < _r.vertexData &&
                _l.operationType < _r.operationType &&
                _l.useIndexes < _r.useIndexes &&
                _l.indexData < _r.indexData;
    }
    //-----------------------------------------------------------------------------------
    InstantRadiosity::Vpl InstantRadiosity::convertToVpl( Vector3 lightColour,
                                                          Vector3 pointOnTri,
                                                          const RayHit &hit )
    {
        //const Real NdotL = hit.triNormal.dotProduct( -hit.rayDir );

        //materialDiffuse is already divided by PI
        Vector3 diffuseTerm = /*NdotL **/ hit.material.diffuse * lightColour;

        if( hit.material.needsUv )
        {
            const Real invTriArea = Real(1.0) / ( (hit.triVerts[0] - hit.triVerts[1]).
                    crossProduct( hit.triVerts[0] - hit.triVerts[2] ).length() );

            //Calculate barycentric coordinates (only works if point is inside tri)
            //Calculate vectors from point to vertices p0, p1 and p2:
            const Vector3 f0 = hit.triVerts[0] - pointOnTri;
            const Vector3 f1 = hit.triVerts[1] - pointOnTri;
            const Vector3 f2 = hit.triVerts[2] - pointOnTri;

            //Calculate the areas and factors (order of parameters doesn't matter):
            //a0 = p0's triangle area / tri_area
            const Real a0 = f1.crossProduct( f2 ).length() * invTriArea;
            const Real a1 = f2.crossProduct( f0 ).length() * invTriArea;
            const Real a2 = f0.crossProduct( f1 ).length() * invTriArea;

            for( int i=0; i<5 && hit.material.image[i]; ++i )
            {
                const uint8 uvSet = hit.material.uvSet[i];
                const Vector2 * RESTRICT_ALIAS uv = hit.triUVs[uvSet];
                Vector2 interpUV = uv[0] * a0 + uv[1] * a1 + uv[2] * a2;

                const Real texWidth  = hit.material.image[i]->getWidth();
                const Real texHeight = hit.material.image[i]->getHeight();

                //The texel centers are in the middle of the pixel, so we need to subtract
                //0.5; but later we need to add 0.5 to do correct rounding. So they negate
                interpUV.x = interpUV.x * texWidth/* - 0.5f*/;
                interpUV.y = interpUV.y * texHeight/* - 0.5f*/;

                interpUV.x = fmod( interpUV.x, texWidth );
                interpUV.y = fmod( interpUV.y, texHeight );
                if( interpUV.x < 0 )
                    interpUV.x += texWidth;
                if( interpUV.y < 0 )
                    interpUV.y += texHeight;

                //TODO: Do blending modes
                ColourValue colourVal =
                        hit.material.image[i]->getColourAt( static_cast<size_t>(interpUV.x),
                                                            static_cast<size_t>(interpUV.y),
                                                            0u );
                diffuseTerm.x *= colourVal.r;
                diffuseTerm.y *= colourVal.g;
                diffuseTerm.z *= colourVal.b;
            }
        }

        Vpl vpl;
        vpl.light = 0;
        vpl.diffuse = diffuseTerm;
        vpl.normal = hit.triNormal;
        vpl.position = pointOnTri;
        vpl.numMergedVpls = 1.0f;

        memset( vpl.dirDiffuse, 0, sizeof(vpl.dirDiffuse) );
        mergeDirectionalDiffuse( diffuseTerm, hit.triNormal, vpl.dirDiffuse );

        return vpl;
    }
    //-----------------------------------------------------------------------------------
    void InstantRadiosity::generateAndClusterVpls( Vector3 lightColour, Real attenConst,
                                                   Real attenLinear, Real attenQuad )
    {
        assert( mCellSize > 0 );

        const Real cellSize = Real(1.0) / mCellSize;
        const Real bias = mBias;

        mTmpSparseClusters[0].clear();
        mTmpSparseClusters[1].clear();
        mTmpSparseClusters[2].clear();

        while( !mRayHits.empty() )
        {
            const RayHit &hit = mRayHits.front();

            if( hit.distance >= std::numeric_limits<Real>::max() )
            {
                RayHitVec::iterator itRay = mRayHits.begin();
                efficientVectorRemove( mRayHits, itRay );
                continue;
            }

            const Real accumDistance = hit.accumDistance + hit.distance;

            Real atten = Real(1.0f) /
                    (attenConst + (attenLinear + attenQuad * accumDistance) * accumDistance);
            atten = Ogre::min( Real(1.0f), atten );

            const Vector3 pointOnTri = hit.ray.getPoint( hit.distance * bias );

            const int32 blockX = static_cast<int32>( Math::Floor( pointOnTri.x * cellSize ) );
            const int32 blockY = static_cast<int32>( Math::Floor( pointOnTri.y * cellSize ) );
            const int32 blockZ = static_cast<int32>( Math::Floor( pointOnTri.z * cellSize ) );

            Vpl vpl = convertToVpl( lightColour, pointOnTri, hit );
            vpl.diffuse *= atten;
            for( int i=0; i<6; ++i )
                vpl.dirDiffuse[i] *= atten;

            Real numCollectedVpls = 1.0f;

            //Merge the lights (simple average) that lie in the same cluster.
            RayHitVec::iterator itor = mRayHits.begin() + 1u;
            RayHitVec::iterator end  = mRayHits.end();

            while( itor != end )
            {
                const RayHit &alikeHit = *itor;

                if( alikeHit.distance >= std::numeric_limits<Real>::max() )
                {
                    itor = efficientVectorRemove( mRayHits, itor );
                    end  = mRayHits.end();
                    continue;
                }

                const Real alikeAccumDistance = alikeHit.accumDistance + alikeHit.distance;
                Real alikeAtten = Real(1.0f) /
                        (attenConst + (attenLinear +
                                       attenQuad * alikeAccumDistance) * alikeAccumDistance);
                alikeAtten = Ogre::min( Real(1.0f), alikeAtten );

                const Vector3 pointOnTri02 = alikeHit.ray.getPoint( alikeHit.distance * bias );

                const int32 alikeBlockX = static_cast<int32>( Math::Floor( pointOnTri02.x * cellSize ) );
                const int32 alikeBlockY = static_cast<int32>( Math::Floor( pointOnTri02.y * cellSize ) );
                const int32 alikeBlockZ = static_cast<int32>( Math::Floor( pointOnTri02.z * cellSize ) );

                if( blockX == alikeBlockX && blockY == alikeBlockY && blockZ == alikeBlockZ )
                {
                    Vpl alikeVpl = convertToVpl( lightColour, pointOnTri02, alikeHit );
                    vpl.diffuse += alikeVpl.diffuse * alikeAtten;
                    vpl.normal  += alikeVpl.normal;
                    vpl.position+= alikeVpl.position;

                    for( int i=0; i<6; ++i )
                        vpl.dirDiffuse[i] += alikeVpl.dirDiffuse[i] * alikeAtten;

                    ++numCollectedVpls;

                    itor = efficientVectorRemove( mRayHits, itor );
                    end  = mRayHits.end();
                }
                else
                {
                    ++itor;
                }
            }

            //vpl.diffuse /= numCollectedVpls;
            vpl.diffuse /= mTotalNumRays;
            vpl.position/= numCollectedVpls;
            vpl.normal.normalise();
            vpl.numMergedVpls = numCollectedVpls;

            for( int i=0; i<6; ++i )
                vpl.dirDiffuse[i] /= mTotalNumRays;

            mVpls.push_back( vpl );

            mTmpSparseClusters[0].insert( SparseCluster( blockX, blockY, blockZ,
                                                         vpl.diffuse, vpl.normal, vpl.dirDiffuse ) );

            RayHitVec::iterator itRay = mRayHits.begin();
            efficientVectorRemove( mRayHits, itRay );
        }

        if( mNumSpreadIterations > 0 )
        {
            mTmpSparseClusters[1] = mTmpSparseClusters[0];

            for( int i=mNumSpreadIterations; --i; )
            {
                spreadSparseClusters( mTmpSparseClusters[0], mTmpSparseClusters[1] );
                mTmpSparseClusters[0] = mTmpSparseClusters[1];
            }

            createVplsFromSpreadClusters( mTmpSparseClusters[0] );
        }
    }
    //-----------------------------------------------------------------------------------
    void InstantRadiosity::spreadSparseClusters( const SparseClusterSet &grid0,
                                                 SparseClusterSet &inOutGrid1 )
    {
        SparseClusterSet grid1;
        grid1.swap( inOutGrid1 );

        const int32 c_directions[6][3] =
        {
            {  0,  0, -1 },
            { -1,  0,  0 },
            {  1,  0,  0 },
            {  0, -1,  0 },
            {  0,  1,  0 },
            {  0,  0,  1 }
        };

        Vector3 vDirs[6][9];
        for( int i=0; i<6; ++i )
        {
            Vector3 clusterCorner = Vector3( c_directions[i][0],
                                             c_directions[i][1],
                                             c_directions[i][2] );
            vDirs[i][0] = clusterCorner;
            vDirs[i][1] = clusterCorner + Vector3( -0.5f, -0.5f, -0.5f );
            vDirs[i][2] = clusterCorner + Vector3(  0.5f, -0.5f, -0.5f );
            vDirs[i][3] = clusterCorner + Vector3( -0.5f,  0.5f, -0.5f );
            vDirs[i][4] = clusterCorner + Vector3(  0.5f,  0.5f, -0.5f );
            vDirs[i][5] = clusterCorner + Vector3( -0.5f, -0.5f,  0.5f );
            vDirs[i][6] = clusterCorner + Vector3(  0.5f, -0.5f,  0.5f );
            vDirs[i][7] = clusterCorner + Vector3( -0.5f,  0.5f,  0.5f );
            vDirs[i][8] = clusterCorner + Vector3(  0.5f,  0.5f,  0.5f );

            for( int j=0; j<9; ++j )
                vDirs[i][j].normalise();
        }

        const Real invNumSpreadIterations = Real(1.0f) / mNumSpreadIterations;

        SparseClusterSet::const_iterator itor = grid0.begin();
        SparseClusterSet::const_iterator end  = grid0.end();

        while( itor != end )
        {
            const Vector3 lightDir = itor->direction.normalisedCopy();

            //Spread into all 6 directions. We don't do diagonals because it
            //would be prohibitively expensive (26 directions). We hope doing
            //this multiple times ends up spreading into all directions.
            for( int i=0; i<6; ++i )
            {
                Real NdotL = 0;
                for( int j=0; j<9; ++j )
                    NdotL += Ogre::max( lightDir.dotProduct( vDirs[i][j] ), 0 );
                NdotL /= 9.0f;

                const Vector3 diffuseCol = NdotL * itor->diffuse;

                if( diffuseCol.x >= mSpreadThreshold ||
                    diffuseCol.y >= mSpreadThreshold ||
                    diffuseCol.z >= mSpreadThreshold )
                {
                    int32 newBlockHash[3];
                    newBlockHash[0] = itor->blockHash[0] + c_directions[i][0];
                    newBlockHash[1] = itor->blockHash[1] + c_directions[i][1];
                    newBlockHash[2] = itor->blockHash[2] + c_directions[i][2];

                    SparseClusterSet::iterator gridCluster = grid1.find( (int32*)newBlockHash );

                    if( gridCluster == grid1.end() )
                    {
                        grid1.insert( SparseCluster( newBlockHash ) );
                        gridCluster = grid1.find( (int32*)newBlockHash );
                    }

                    //We guarantee we won't change the order of the iterators.
                    SparseCluster *gridClusterNonConst = const_cast<SparseCluster*>( &(*gridCluster) );

                    //TODO: Consider attenuation.
                    gridClusterNonConst->diffuse    += diffuseCol * invNumSpreadIterations;
                    gridClusterNonConst->direction  += itor->direction;
                }
            }

            ++itor;
        }

        grid1.swap( inOutGrid1 );
    }
    //-----------------------------------------------------------------------------------
    void InstantRadiosity::createVplsFromSpreadClusters( const SparseClusterSet &spreadCluster )
    {
        const Real cellSize = mCellSize;

        SparseClusterSet::const_iterator itor = spreadCluster.begin();
        SparseClusterSet::const_iterator end  = spreadCluster.end();

        while( itor != end )
        {
            Vector3 vClusterCenter( itor->blockHash[0], itor->blockHash[1], itor->blockHash[2] );
            vClusterCenter = (vClusterCenter + 0.5f) * cellSize;

            Vpl vpl;
            vpl.light = 0;
            vpl.diffuse = itor->diffuse;
            vpl.normal = itor->direction;
            vpl.position = vClusterCenter;
            vpl.numMergedVpls = 1.0f;

            memcpy( vpl.dirDiffuse, itor->dirDiffuse, sizeof(vpl.dirDiffuse) );

            mVpls.push_back( vpl );

            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void InstantRadiosity::clusterAllVpls(void)
    {
        assert( mCellSize > 0 );

        const Real cellSize = Real(1.0) / mCellSize;

        VplVec::iterator itor = mVpls.begin();
        VplVec::iterator end  = mVpls.end();

        while( itor != end )
        {
            Vpl vpl = *itor; //Hard copy!

            const size_t idx = itor - mVpls.begin();

            const int32 blockX = static_cast<int32>( Math::Floor( vpl.position.x * cellSize ) );
            const int32 blockY = static_cast<int32>( Math::Floor( vpl.position.y * cellSize ) );
            const int32 blockZ = static_cast<int32>( Math::Floor( vpl.position.z * cellSize ) );

            vpl.normal  *= vpl.numMergedVpls;
            vpl.position*= vpl.numMergedVpls;

            Real numCollectedVpls = vpl.numMergedVpls;

            //Merge the lights (simple average) that lie in the same cluster.
            VplVec::iterator itAlike = itor + 1;

            while( itAlike != end )
            {
                const Vpl &alikeVpl = *itAlike;

                const Vector3 pointOnTri02 = itAlike->position;

                const int32 alikeBlockX = static_cast<int32>( Math::Floor( pointOnTri02.x * cellSize ) );
                const int32 alikeBlockY = static_cast<int32>( Math::Floor( pointOnTri02.y * cellSize ) );
                const int32 alikeBlockZ = static_cast<int32>( Math::Floor( pointOnTri02.z * cellSize ) );

                if( blockX == alikeBlockX && blockY == alikeBlockY && blockZ == alikeBlockZ )
                {
                    vpl.diffuse += alikeVpl.diffuse;
                    vpl.normal  += alikeVpl.normal * alikeVpl.numMergedVpls;
                    vpl.position+= alikeVpl.position * alikeVpl.numMergedVpls;

                    for( int i=0; i<6; ++i )
                        vpl.dirDiffuse[i] += alikeVpl.dirDiffuse[i];

                    numCollectedVpls += alikeVpl.numMergedVpls;

                    //Iterators get invalidated!
                    itAlike = efficientVectorRemove( mVpls, itAlike );
                    itor    = mVpls.begin() + idx;
                    end     = mVpls.end();
                }
                else
                {
                    ++itAlike;
                }
            }

            if( numCollectedVpls > vpl.numMergedVpls )
            {
                vpl.position/= numCollectedVpls;
                vpl.normal.normalise();
                vpl.numMergedVpls = numCollectedVpls;
                *itor = vpl;
            }

            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void InstantRadiosity::autogenerateAreaOfInterest(void)
    {
        AxisAlignedBox areaOfInterest;
        for( size_t i=0; i<NUM_SCENE_MEMORY_MANAGER_TYPES; ++i )
        {
            ObjectMemoryManager &memoryManager = mSceneManager->_getEntityMemoryManager(
                        static_cast<SceneMemoryMgrTypes>(i) );

            const size_t numRenderQueues = memoryManager.getNumRenderQueues();

            size_t firstRq = std::min<size_t>( mFirstRq, numRenderQueues );
            size_t lastRq  = std::min<size_t>( mLastRq,  numRenderQueues );

            for( size_t j=firstRq; j<lastRq; ++j )
            {
                AxisAlignedBox tmpBox;
                ObjectData objData;
                const size_t totalObjs = memoryManager.getFirstObjectData( objData, j );
                MovableObject::calculateCastersBox( totalObjs, objData,
                                                    mVisibilityMask &
                                                    VisibilityFlags::RESERVED_VISIBILITY_FLAGS,
                                                    &tmpBox );
                areaOfInterest.merge( tmpBox );
            }
        }

        Aabb aabb = Aabb::newFromExtents( areaOfInterest.getMinimum(), areaOfInterest.getMaximum() );
        mAoI.push_back( AreaOfInterest( aabb, 0.0f ) );
    }
    //-----------------------------------------------------------------------------------
    void InstantRadiosity::processLight( Vector3 lightPos, const Quaternion &lightRot, uint8 lightType,
                                         Radian angle, Vector3 lightColour, Real lightRange,
                                         Real attenConst, Real attenLinear, Real attenQuad,
                                         const AreaOfInterest &areaOfInterest )
    {
        Aabb rotatedAoI = areaOfInterest.aabb;
        {
            Matrix4 rotMatrix;
            rotMatrix.makeTransform( Vector3::ZERO, Vector3::UNIT_SCALE, lightRot.Inverse() );
            rotatedAoI.transformAffine( rotMatrix );
        }

        //Same RNG/seed for every object & triangle
        RandomNumberGenerator rng;
        mRayHits.resize( mTotalNumRays );

        ArrayRay * RESTRICT_ALIAS arrayRays = mArrayRays.get();

        for( size_t i=0; i<mNumRays; ++i )
        {
            mRayHits[i].distance = std::numeric_limits<Real>::max();
            mRayHits[i].accumDistance = 0;

            if( lightType == Light::LT_POINT )
            {
                mRayHits[i].ray.setOrigin( lightPos );
                mRayHits[i].ray.setDirection( rng.getRandomDir() );
            }
            else if( lightType == Light::LT_SPOTLIGHT )
            {
                assert( angle < Degree(180) );
                Vector2 pointInCircle = rng.getRandomPointInCircle();
                pointInCircle *= Math::Tan( angle * 0.5f );
                Vector3 rayDir = Vector3( pointInCircle.x, pointInCircle.y, -1.0f );
                rayDir.normalise();
                rayDir = lightRot * rayDir;
                mRayHits[i].ray.setOrigin( lightPos );
                mRayHits[i].ray.setDirection( rayDir );
            }
            else
            {
                Vector3 randomPos;
                randomPos.x = rng.boxRand() * rotatedAoI.mHalfSize.x;
                randomPos.y = rng.boxRand() * rotatedAoI.mHalfSize.y;
                randomPos.z = Ogre::max( rotatedAoI.mHalfSize.z, areaOfInterest.sphereRadius ) + 1.0f;
                randomPos = lightRot * randomPos + areaOfInterest.aabb.mCenter;

                mRayHits[i].ray.setOrigin( randomPos );
                mRayHits[i].ray.setDirection( -lightRot.zAxis() );
            }

            arrayRays->mOrigin.setAll( mRayHits[i].ray.getOrigin() );
            arrayRays->mDirection.setAll( mRayHits[i].ray.getDirection() );
            ++arrayRays;
        }

        //Initialize all other rays (some rays may not be initialized
        //at all when not all bounced rays end up hitting something)
        for( size_t i=mNumRays; i<mTotalNumRays; ++i )
            mRayHits[i].distance = std::numeric_limits<Real>::max();

        size_t rayStart = 0;
        size_t numRays = mNumRays;

        for( size_t k=0; k<mNumRayBounces + 1u; ++k )
        {
            for( size_t i=0; i<NUM_SCENE_MEMORY_MANAGER_TYPES; ++i )
            {
                ObjectMemoryManager &memoryManager = mSceneManager->_getEntityMemoryManager(
                            static_cast<SceneMemoryMgrTypes>(i) );

                const size_t numRenderQueues = memoryManager.getNumRenderQueues();

                size_t firstRq = std::min<size_t>( mFirstRq, numRenderQueues );
                size_t lastRq  = std::min<size_t>( mLastRq,  numRenderQueues );

                for( size_t j=firstRq; j<lastRq; ++j )
                {
                    ObjectData objData;
                    const size_t totalObjs = memoryManager.getFirstObjectData( objData, j );
                    testLightVsAllObjects( lightType, lightRange, objData, totalObjs,
                                           areaOfInterest, rayStart, numRays );
                }
            }

            const size_t oldRayStart    = rayStart;
            const size_t oldNumRays     = numRays;

            rayStart += numRays;
            numRays = static_cast<size_t>( mNumRays * powf( mSurvivingRayFraction, k + 1 ) );
            numRays = std::min<size_t>( numRays, std::max<int>( 0, mTotalNumRays - rayStart ) );

            numRays = generateRayBounces( oldRayStart, oldNumRays, numRays, rng );
        }

        generateAndClusterVpls( lightColour, attenConst, attenLinear, attenQuad );
    }
    //-----------------------------------------------------------------------------------
    size_t InstantRadiosity::generateRayBounces( size_t raySrcStart, size_t raySrcCount,
                                                 size_t raysToGenerate,
                                                 RandomNumberGenerator &rng )
    {
        size_t rayIdx = raySrcStart;
        size_t raysRemaining = raysToGenerate;
        size_t rayDstStart = raySrcStart + raySrcCount;
        const size_t raySrcLimit = rayDstStart;

        const Real bias = mBias;

        ArrayRay * RESTRICT_ALIAS arrayRays = mArrayRays.get();

        while( rayIdx < raySrcLimit && raysRemaining > 0 )
        {
            while( rayIdx < raySrcLimit &&
                   mRayHits[rayIdx].distance >= std::numeric_limits<Real>::max() )
            {
                ++rayIdx;
            }

            if( rayIdx < raySrcLimit )
            {
                const RayHit &hit = mRayHits[rayIdx];
                const Vector3 pointOnTri = hit.ray.getPoint( hit.distance * bias );

                size_t i = rayDstStart++;
                mRayHits[i].distance = std::numeric_limits<Real>::max();
                mRayHits[i].accumDistance = hit.accumDistance + hit.distance;
                mRayHits[i].ray.setOrigin( pointOnTri );
                mRayHits[i].ray.setDirection( rng.randomizeDirAroundCone( hit.triNormal, Degree( 90.0f ) ) );
                arrayRays[i].mOrigin.setAll( mRayHits[i].ray.getOrigin() );
                arrayRays[i].mDirection.setAll( mRayHits[i].ray.getDirection() );

                ++rayIdx;
                --raysRemaining;
            }
        }

        return raysToGenerate - raysRemaining;
    }
    //-----------------------------------------------------------------------------------
    const InstantRadiosity::MeshData* InstantRadiosity::downloadVao( VertexArrayObject *vao )
    {
        MeshDataMapV2::const_iterator itor = mMeshDataMapV2.find( vao );
        if( itor != mMeshDataMapV2.end() )
            return &itor->second;

        IndexBufferPacked *indexBuffer = vao->getIndexBuffer();

        MeshData meshData;
        memset( &meshData, 0, sizeof(meshData) );

        //Issue all async requests now.
        VertexArrayObject::ReadRequestsArray readRequests;
        AsyncTicketPtr indexTicket;

        {
            //Request to read VES_POSITION (must be present) and all of its UVs
            readRequests.push_back( VES_POSITION );

            //Avoid downloading UVs if not needed
            if( mUseTextures )
            {
                VertexElement2VecVec vertexDeclaration = vao->getVertexDeclaration();
                VertexElement2VecVec::const_iterator it0 = vertexDeclaration.begin();
                VertexElement2VecVec::const_iterator en0 = vertexDeclaration.end();

                while( it0 != en0 )
                {
                    VertexElement2Vec::const_iterator it1 = it0->begin();
                    VertexElement2Vec::const_iterator en1 = it0->end();

                    while( it1 != en1 )
                    {
                        if( *it1 == VES_TEXTURE_COORDINATES )
                            readRequests.push_back( VES_TEXTURE_COORDINATES );
                        ++it1;
                    }

                    ++it0;
                }
            }

            if( !indexBuffer )
                vao->readRequests( readRequests, vao->getPrimitiveStart(), vao->getPrimitiveCount() );
            else
                vao->readRequests( readRequests );
        }

        if( indexBuffer && !indexBuffer->getShadowCopy() )
        {
            indexTicket = indexBuffer->readRequest( vao->getPrimitiveStart(),
                                                    vao->getPrimitiveCount() );
        }

        if( indexBuffer )
        {
            meshData.numVertices = readRequests[0].vertexBuffer->getNumElements();
            meshData.useIndices16bit = indexBuffer->getIndexType() == IndexBufferPacked::IT_16BIT;
            meshData.numIndices = vao->getPrimitiveCount();
            if( !indexBuffer->getShadowCopy() )
            {
                meshData.indexData = reinterpret_cast<uint8*>(
                            OGRE_MALLOC_SIMD( vao->getPrimitiveCount() *
                                              indexBuffer->getBytesPerElement(),
                                              MEMCATEGORY_GEOMETRY ) );
            }
        }
        else
        {
            meshData.numVertices = vao->getPrimitiveCount();
        }

        const size_t numVertexElements = readRequests.size();

        meshData.vertexData = reinterpret_cast<float*>(
                    OGRE_MALLOC_SIMD( meshData.numVertices * (sizeof(float) * 3u +
                                      sizeof(float) * 2u * (numVertexElements - 1u)),
                                      MEMCATEGORY_GEOMETRY ) );

        //Copy position + UVs
        bool isHalf[9];
        for( size_t j=0; j<numVertexElements; ++j )
            isHalf[j] = v1::VertexElement::getBaseType( readRequests[j].type ) == VET_HALF2;

        vao->mapAsyncTickets( readRequests );

        for( size_t i=0; i<meshData.numVertices; ++i )
        {
            //Copy position
            if( isHalf[0] )
            {
                uint16 const * RESTRICT_ALIAS bufferF16 =
                        reinterpret_cast<uint16 const * RESTRICT_ALIAS>( readRequests[0].data );
                meshData.vertexData[i*3u + 0u] = Bitwise::halfToFloat( bufferF16[0] );
                meshData.vertexData[i*3u + 1u] = Bitwise::halfToFloat( bufferF16[1] );
                meshData.vertexData[i*3u + 2u] = Bitwise::halfToFloat( bufferF16[2] );
            }
            else
            {
                float const * RESTRICT_ALIAS bufferF32 =
                        reinterpret_cast<float const * RESTRICT_ALIAS>( readRequests[0].data );
                meshData.vertexData[i*3u + 0u] = bufferF32[0];
                meshData.vertexData[i*3u + 1u] = bufferF32[1];
                meshData.vertexData[i*3u + 2u] = bufferF32[2];
            }

            readRequests[0].data += readRequests[0].vertexBuffer->getBytesPerElement();

            //Copy UVs
            for( size_t j=1; j<numVertexElements; ++j )
            {
                float * RESTRICT_ALIAS uvDst = meshData.getUvStart( j - 1u );
                if( isHalf[j] )
                {
                    uint16 const * RESTRICT_ALIAS bufferF16 =
                            reinterpret_cast<uint16 const * RESTRICT_ALIAS>( readRequests[j].data );
                    uvDst[i*2u + 0u] = Bitwise::halfToFloat( bufferF16[0] );
                    uvDst[i*2u + 1u] = Bitwise::halfToFloat( bufferF16[1] );
                }
                else
                {
                    float const * RESTRICT_ALIAS bufferF32 =
                            reinterpret_cast<float const * RESTRICT_ALIAS>( readRequests[j].data );
                    uvDst[i*2u + 0u] = bufferF32[0];
                    uvDst[i*2u + 1u] = bufferF32[1];
                }

                readRequests[j].data += readRequests[j].vertexBuffer->getBytesPerElement();
            }
        }

        vao->unmapAsyncTickets( readRequests );

        //Copy index buffer
        if( indexBuffer )
        {
            if( !indexBuffer->getShadowCopy() )
            {
                const void *indexData = indexTicket->map();
                memcpy( meshData.indexData, indexData,
                        meshData.numIndices * indexBuffer->getBytesPerElement() );
                indexTicket->unmap();
            }
            else
            {
                meshData.indexDataConst =
                        reinterpret_cast<const uint8*>( indexBuffer->getShadowCopy() ) +
                        vao->getPrimitiveStart();
            }
        }

        mMeshDataMapV2[vao] = meshData;

        return &mMeshDataMapV2[vao];
    }
    //-----------------------------------------------------------------------------------
    const InstantRadiosity::MeshData* InstantRadiosity::downloadRenderOp(
            const v1::RenderOperation &renderOp )
    {
        MeshDataMapV1::const_iterator itor = mMeshDataMapV1.find( renderOp );
        if( itor != mMeshDataMapV1.end() )
            return &itor->second;

        v1::VertexData::ReadRequestsArray readRequests;

        {
            //Request to read VES_POSITION (must be present) and all of its UVs
            readRequests.push_back( VES_POSITION );

            //Avoid downloading UVs if not needed
            if( mUseTextures )
            {
                const v1::VertexDeclaration::VertexElementList &vertexElements =
                        renderOp.vertexData->vertexDeclaration->getElements();
                v1::VertexDeclaration::VertexElementList::const_iterator it0 = vertexElements.begin();
                v1::VertexDeclaration::VertexElementList::const_iterator en0 = vertexElements.end();

                while( it0 != en0 )
                {
                    if( it0->getSemantic() == VES_TEXTURE_COORDINATES )
                        readRequests.push_back( VES_TEXTURE_COORDINATES );
                    ++it0;
                }
            }
        }

        const size_t numVertexElements = readRequests.size();
        renderOp.vertexData->lockMultipleElements( readRequests, v1::HardwareBuffer::HBL_READ_ONLY );

        MeshData meshData;
        memset( &meshData, 0, sizeof(meshData) );

        meshData.numVertices = renderOp.vertexData->vertexCount;
        meshData.vertexData = reinterpret_cast<float*>(
                    OGRE_MALLOC_SIMD( meshData.numVertices * (sizeof(float) * 3u +
                                      sizeof(float) * 2u * (numVertexElements - 1u)),
                                      MEMCATEGORY_GEOMETRY ) );
        if( renderOp.useIndexes )
        {
            meshData.useIndices16bit = renderOp.indexData->indexBuffer->getType() ==
                    v1::HardwareIndexBuffer::IT_16BIT;
            meshData.numIndices = renderOp.indexData->indexCount;
            meshData.indexData = reinterpret_cast<uint8*>(
                        OGRE_MALLOC_SIMD( meshData.numIndices *
                                          renderOp.indexData->indexBuffer->getIndexSize(),
                                          MEMCATEGORY_GEOMETRY ) );
        }

        //Copy position + UVs
        bool isHalf[9];
        for( size_t j=0; j<numVertexElements; ++j )
        {
            isHalf[j] = v1::VertexElement::getBaseType( readRequests[j].type ) == VET_HALF2;

            if( !renderOp.useIndexes )
            {
                readRequests[j].data += renderOp.vertexData->vertexStart *
                        readRequests[j].vertexBuffer->getVertexSize();
            }
        }

        for( size_t i=0; i<meshData.numVertices; ++i )
        {
            //Copy position
            if( isHalf[0] )
            {
                uint16 const * RESTRICT_ALIAS bufferF16 =
                        reinterpret_cast<uint16 const * RESTRICT_ALIAS>( readRequests[0].data );
                meshData.vertexData[i*3u + 0u] = Bitwise::halfToFloat( bufferF16[0] );
                meshData.vertexData[i*3u + 1u] = Bitwise::halfToFloat( bufferF16[1] );
                meshData.vertexData[i*3u + 2u] = Bitwise::halfToFloat( bufferF16[2] );
            }
            else
            {
                float const * RESTRICT_ALIAS bufferF32 =
                        reinterpret_cast<float const * RESTRICT_ALIAS>( readRequests[0].data );
                meshData.vertexData[i*3u + 0u] = bufferF32[0];
                meshData.vertexData[i*3u + 1u] = bufferF32[1];
                meshData.vertexData[i*3u + 2u] = bufferF32[2];
            }

            readRequests[0].data += readRequests[0].vertexBuffer->getVertexSize();

            //Copy UVs
            for( size_t j=1; j<numVertexElements; ++j )
            {
                float * RESTRICT_ALIAS uvDst = meshData.getUvStart( j - 1u );
                if( isHalf[j] )
                {
                    uint16 const * RESTRICT_ALIAS bufferF16 =
                            reinterpret_cast<uint16 const * RESTRICT_ALIAS>( readRequests[j].data );
                    uvDst[i*2u + 0u] = Bitwise::halfToFloat( bufferF16[0] );
                    uvDst[i*2u + 1u] = Bitwise::halfToFloat( bufferF16[1] );
                }
                else
                {
                    float const * RESTRICT_ALIAS bufferF32 =
                            reinterpret_cast<float const * RESTRICT_ALIAS>( readRequests[j].data );
                    uvDst[i*2u + 0u] = bufferF32[0];
                    uvDst[i*2u + 1u] = bufferF32[1];
                }

                readRequests[j].data += readRequests[j].vertexBuffer->getVertexSize();
            }
        }

        renderOp.vertexData->unlockMultipleElements( readRequests );

        //Copy index buffer
        if( renderOp.useIndexes )
        {
            const void *indexData = renderOp.indexData->indexBuffer->lock(
                        renderOp.indexData->indexStart,
                        renderOp.indexData->indexCount,
                        v1::HardwareBuffer::HBL_READ_ONLY );
            memcpy( meshData.indexData, indexData,
                    renderOp.indexData->indexCount *
                    renderOp.indexData->indexBuffer->getIndexSize() );
            renderOp.indexData->indexBuffer->unlock();
        }

        mMeshDataMapV1[renderOp] = meshData;

        return &mMeshDataMapV1[renderOp];
    }
    //-----------------------------------------------------------------------------------
    const Image& InstantRadiosity::downloadTexture( const TexturePtr &texture )
    {
        ImageMap::iterator itor = mImageMap.find( texture.get() );
        if( itor != mImageMap.end() )
            return itor->second;

        mImageMap[texture.get()] = Image();
        itor = mImageMap.find( texture.get() );
        texture->convertToImage( itor->second, false, mMipmapBias );

        return itor->second;
    }
    //-----------------------------------------------------------------------------------
    void InstantRadiosity::testLightVsAllObjects( uint8 lightType, Real lightRange,
                                                  ObjectData objData, size_t numNodes,
                                                  const AreaOfInterest &scalarAreaOfInterest,
                                                  size_t rayStart, size_t numRays )
    {
        Aabb biggestAoI = scalarAreaOfInterest.aabb;
        biggestAoI.merge( Aabb( biggestAoI.mCenter, Vector3( scalarAreaOfInterest.sphereRadius ) ) );

        const ArrayInt sceneFlags = Mathlib::SetAll( mVisibilityMask &
                                                     VisibilityFlags::RESERVED_VISIBILITY_FLAGS );
        ArrayAabb areaOfInterest( ArrayVector3::ZERO, ArrayVector3::ZERO );
        areaOfInterest.setAll( biggestAoI );

        for( size_t i=0; i<numNodes; i += ARRAY_PACKED_REALS )
        {
            ArrayInt * RESTRICT_ALIAS visibilityFlags = reinterpret_cast<ArrayInt*RESTRICT_ALIAS>
                                                                        (objData.mVisibilityFlags);

            //isObjectHitByRays = isVisble;
            ArrayMaskI isObjectHitByRays = Mathlib::TestFlags4( *visibilityFlags,
                                               Mathlib::SetAll( VisibilityFlags::LAYER_VISIBILITY ) );
            //isObjectHitByRays = isVisble & (sceneFlags & visibilityFlags);
            isObjectHitByRays = Mathlib::And( isObjectHitByRays,
                                              Mathlib::TestFlags4( sceneFlags, *visibilityFlags ) );

            if( lightType == Light::LT_DIRECTIONAL )
            {
                //Check if obj is in area of interest for directional lights
                ArrayMaskI hitMask = CastRealToInt( areaOfInterest.intersects( *objData.mWorldAabb ) );
                isObjectHitByRays = Mathlib::And( isObjectHitByRays, hitMask );
            }

            if( BooleanMask4::getScalarMask( isObjectHitByRays ) == 0 )
            {
                //None of these objects are visible. Early out.
                objData.advancePack();
                continue;
            }

            for( size_t k=0; k<ARRAY_PACKED_REALS; ++k )
                mTmpRaysThatHitObject[k].clear();

            //Make a list of rays that hit these objects (i.e. broadphase)
            ArrayRay * RESTRICT_ALIAS arrayRays = mArrayRays.get() + rayStart;
            for( size_t j=0; j<numRays; ++j )
            {
                ArrayMaskR rayHits = arrayRays->intersects( *objData.mWorldAabb );
                uint32 scalarRayHits = BooleanMask4::getScalarMask( rayHits );
                for( size_t k=0; k<ARRAY_PACKED_REALS; ++k )
                {
                    if( IS_BIT_SET( k, scalarRayHits ) )
                        mTmpRaysThatHitObject[k].push_back( j + rayStart );
                }

                ++arrayRays;
            }

            for( size_t j=0; j<ARRAY_PACKED_REALS; ++j )
            {
                //Convert isObjectHitByRays into something smaller we can work with.
                uint32 scalarIsObjectHitByRays = BooleanMask4::getScalarMask( isObjectHitByRays );

                if( !mTmpRaysThatHitObject[j].empty() &&
                    IS_BIT_SET( j, scalarIsObjectHitByRays ) )
                {
                    MovableObject *movableObject = objData.mOwner[j];

                    const Matrix4 &worldMatrix = movableObject->_getParentNodeFullTransform();
                    RenderableArray::const_iterator itor = movableObject->mRenderables.begin();
                    RenderableArray::const_iterator end  = movableObject->mRenderables.end();

                    while( itor != end )
                    {
                        const VertexArrayObjectArray &vaos = (*itor)->getVaos( VpNormal );
                        MeshData const *meshData = 0;
                        if( !vaos.empty() )
                        {
                            //v2 object
                            VertexArrayObject *vao = vaos[0]; //TODO Allow picking a LOD.
                            meshData = downloadVao( vao );
                        }
                        else
                        {
                            //v1 object
                            v1::RenderOperation renderOp;
                            (*itor)->getRenderOperation( renderOp, false );
                            meshData = downloadRenderOp( renderOp );
                        }

                        HlmsDatablock *datablock = (*itor)->getDatablock();

                        if( datablock->mType == HLMS_PBS )
                        {
                            MaterialData material;
                            memset( &material, 0, sizeof(material) );
                            int imageIdx = 0;

                            HlmsPbsDatablock *pbsDatablock = static_cast<HlmsPbsDatablock*>( datablock );
                            //TODO: Should we account fresnel here? What about metalness?
                            material.diffuse = pbsDatablock->getDiffuse();
                            TexturePtr diffuseTex = pbsDatablock->getTexture( PBSM_DIFFUSE );
                            if( diffuseTex.isNull() ||
                                PixelUtil::isCompressed( diffuseTex->getFormat() ) )
                            {
                                const ColourValue &bgDiffuse = pbsDatablock->getBackgroundDiffuse();
                                material.diffuse.x *= bgDiffuse.r;
                                material.diffuse.y *= bgDiffuse.g;
                                material.diffuse.z *= bgDiffuse.b;
                            }
                            else if( mUseTextures )
                            {
                                material.image[imageIdx] = &downloadTexture( diffuseTex );
                                material.uvSet[imageIdx] =
                                        pbsDatablock->getTextureUvSource( PBSM_DIFFUSE );
                                material.needsUv = true;
                                ++imageIdx;
                            }

                            if( mUseTextures )
                            {
                                for( int k=0; k<4; ++k )
                                {
                                    const PbsTextureTypes texType = static_cast<PbsTextureTypes>(
                                                                                PBSM_DETAIL0 + k );
                                    TexturePtr detailTex = pbsDatablock->getTexture( texType );
                                    if( !detailTex.isNull() &&
                                        !PixelUtil::isCompressed( detailTex->getFormat() ) )
                                    {
                                        material.image[imageIdx] = &downloadTexture( detailTex );
                                        material.uvSet[imageIdx] =
                                                pbsDatablock->getTextureUvSource( texType );
                                        material.needsUv = true;
                                        ++imageIdx;
                                    }
                                }
                            }

                            raycastLightRayVsMesh( lightRange, *meshData,
                                                   worldMatrix, material,
                                                   mTmpRaysThatHitObject[j] );
                        }

                        ++itor;
                    }
                }
            }

            objData.advancePack();
        }
    }
    //-----------------------------------------------------------------------------------
    void InstantRadiosity::raycastLightRayVsMesh( Real lightRange, const MeshData meshData,
                                                  Matrix4 worldMatrix, const MaterialData &material,
                                                  const FastArray<size_t> &raysThatHitObj )
    {
        const size_t numElements = meshData.indexData ? meshData.numIndices : meshData.numVertices;

        const uint16 * RESTRICT_ALIAS indexData16 = reinterpret_cast<const uint16 * RESTRICT_ALIAS>(
                    meshData.indexData );
        const uint32 * RESTRICT_ALIAS indexData32 = reinterpret_cast<const uint32 * RESTRICT_ALIAS>(
                    meshData.indexData );

        for( size_t i=0; i<numElements; i += 3 )
        {
            Vector3 triVerts[3];

            uint32 vertexIdx[3];

            if( meshData.indexData )
            {
                if( meshData.useIndices16bit )
                {
                    vertexIdx[0] = indexData16[i+0];
                    vertexIdx[1] = indexData16[i+1];
                    vertexIdx[2] = indexData16[i+2];
                }
                else
                {
                    vertexIdx[0] = indexData32[i+0];
                    vertexIdx[1] = indexData32[i+1];
                    vertexIdx[2] = indexData32[i+2];
                }
            }
            else
            {
                vertexIdx[0] = i+0;
                vertexIdx[1] = i+1;
                vertexIdx[2] = i+2;
            }

            triVerts[0].x = meshData.vertexData[vertexIdx[0] * 3u + 0];
            triVerts[0].y = meshData.vertexData[vertexIdx[0] * 3u + 1];
            triVerts[0].z = meshData.vertexData[vertexIdx[0] * 3u + 2];
            triVerts[1].x = meshData.vertexData[vertexIdx[1] * 3u + 0];
            triVerts[1].y = meshData.vertexData[vertexIdx[1] * 3u + 1];
            triVerts[1].z = meshData.vertexData[vertexIdx[1] * 3u + 2];
            triVerts[2].x = meshData.vertexData[vertexIdx[2] * 3u + 0];
            triVerts[2].y = meshData.vertexData[vertexIdx[2] * 3u + 1];
            triVerts[2].z = meshData.vertexData[vertexIdx[2] * 3u + 2];

            triVerts[0] = worldMatrix * triVerts[0];
            triVerts[1] = worldMatrix * triVerts[1];
            triVerts[2] = worldMatrix * triVerts[2];

            Vector3 triNormal = Math::calculateBasicFaceNormalWithoutNormalize(
                        triVerts[0], triVerts[1], triVerts[2] );
            triNormal.normalise();

            FastArray<size_t>::const_iterator itRayIdx = raysThatHitObj.begin();
            FastArray<size_t>::const_iterator enRayIdx = raysThatHitObj.end();
            while( itRayIdx != enRayIdx )
            {
                Ray ray = mRayHits[*itRayIdx].ray;

                const std::pair<bool, Real> inters = Math::intersects(
                            ray, triVerts[0], triVerts[1], triVerts[2], triNormal, true, false );

                if( inters.first )
                {
                    RayHit &rayHit = mRayHits[*itRayIdx];
                    if( inters.second < rayHit.distance &&
                        inters.second <= lightRange )
                    {
                        rayHit.distance = inters.second;
                        rayHit.material = material;
                        rayHit.triVerts[0] = triVerts[0];
                        rayHit.triVerts[1] = triVerts[1];
                        rayHit.triVerts[2] = triVerts[2];
                        rayHit.triNormal = triNormal;

                        for( int j=0; j<5 && material.image[j]; ++j )
                        {
                            const uint8 uvSet = material.uvSet[j];
                            const float * RESTRICT_ALIAS uvPtr = meshData.getUvStart( uvSet );
                            rayHit.triUVs[j][0].x = uvPtr[vertexIdx[0] * 2u + 0];
                            rayHit.triUVs[j][0].y = uvPtr[vertexIdx[0] * 2u + 1];

                            rayHit.triUVs[j][1].x = uvPtr[vertexIdx[1] * 2u + 0];
                            rayHit.triUVs[j][1].y = uvPtr[vertexIdx[1] * 2u + 1];

                            rayHit.triUVs[j][2].x = uvPtr[vertexIdx[2] * 2u + 0];
                            rayHit.triUVs[j][2].y = uvPtr[vertexIdx[2] * 2u + 1];
                        }
                    }
                }

                ++itRayIdx;
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void InstantRadiosity::updateExistingVpls(void)
    {
        SceneNode *rootNode = mSceneManager->getRootSceneNode( SCENE_DYNAMIC );

        VplVec::iterator itor = mVpls.begin();
        VplVec::iterator end  = mVpls.end();

        while( itor != end )
        {
            Vpl &vpl = *itor;
            Vector3 diffuseCol = vpl.diffuse * mVplPowerBoost;
            if( (diffuseCol.x > mVplThreshold ||
                 diffuseCol.y > mVplThreshold ||
                 diffuseCol.z > mVplThreshold) &&
                 !mUseIrradianceVolume)
            {
                if( !vpl.light )
                {
                    SceneNode *lightNode = rootNode->createChildSceneNode( SCENE_DYNAMIC );
                    vpl.light = mSceneManager->createLight();
                    vpl.light->setType( Light::LT_VPL );
                    lightNode->attachObject( vpl.light );
                    lightNode->setPosition( vpl.position );
                }

                ColourValue colour;
                colour.r = diffuseCol.x;
                colour.g = diffuseCol.y;
                colour.b = diffuseCol.z;
                colour.a = 1.0f;

                Real range = mVplMaxRange;
                if( mVplUseIntensityForMaxRange )
                {
                    double intensity;
                    intensity = Ogre::max( colour.r, colour.g );
                    intensity = Ogre::max( intensity, (double)colour.b );
                    if( mVplQuadAtten != 0 )
                        intensity *= 1e-6 / mVplQuadAtten;
                    double rangeInMeters = sqrt( intensity );
                    range = (float)(rangeInMeters * mVplIntensityRangeMultiplier);
                }

                vpl.light->setDiffuseColour( colour );
                vpl.light->setSpecularColour( ColourValue::Black );
                vpl.light->setAttenuation( Ogre::min( range, mVplMaxRange ), mVplConstAtten,
                                           mVplLinearAtten, mVplQuadAtten );
            }
            else if( vpl.light )
            {
                SceneNode *lightNode = vpl.light->getParentSceneNode();
                lightNode->getParentSceneNode()->removeAndDestroyChild( lightNode );
                mSceneManager->destroyLight( vpl.light );
                vpl.light = 0;
            }

            ++itor;
        }

        if( mEnableDebugMarkers )
            createDebugMarkers();
    }
    //-----------------------------------------------------------------------------------
    void InstantRadiosity::clear(void)
    {
        VplVec::const_iterator itor = mVpls.begin();
        VplVec::const_iterator end  = mVpls.end();

        while( itor != end )
        {
            const Vpl &vpl = *itor;
            if( vpl.light )
            {
                SceneNode *lightNode = vpl.light->getParentSceneNode();
                lightNode->getParentSceneNode()->removeAndDestroyChild( lightNode );
                mSceneManager->destroyLight( vpl.light );
            }

            ++itor;
        }

        mVpls.clear();

        destroyDebugMarkers();
    }
    //-----------------------------------------------------------------------------------
    void InstantRadiosity::build(void)
    {
        clear();

        if( mNumRayBounces > 0 && (mSurvivingRayFraction <= 0 || mSurvivingRayFraction > 1.0f) )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "For multiple bounces, mSurvivingRayFraction must be in range (0; 1]",
                         "InstantRadiosity::build" );
        }

        //Sum of the first n terms of a geometric series
        //mNumRays + mNumRays * mSurvivingRayFraction + mNumRays * mSurvivingRayFraction^2 + ...
        if( mSurvivingRayFraction != 1.0f )
        {
            mTotalNumRays = static_cast<size_t>( mNumRays *
                        (1.0f - powf( mSurvivingRayFraction, mNumRayBounces + 1u )) /
                        (1.0f - mSurvivingRayFraction) );
        }
        else
        {
            mTotalNumRays = mNumRays * (mNumRayBounces + 1u);
        }

        //Ensure position & AABB data is up to date.
        mSceneManager->updateSceneGraph();
        mSceneManager->clearFrameData();

        Hlms *hlms = mHlmsManager->getHlms( HLMS_PBS );
        if( !dynamic_cast<HlmsPbs*>( hlms ) )
        {
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                         "This InstantRadiosity is designed to downcast HlmsDatablock into "
                         "HlmsPbsDatablock, and it cannot understand datablocks made by other Hlms "
                         "implementations.",
                         "InstantRadiosity::build" );
        }

        mArrayRays = RawSimdUniquePtr<ArrayRay, MEMCATEGORY_GENERAL>( mTotalNumRays );

        const uint32 lightMask = mLightMask & VisibilityFlags::RESERVED_VISIBILITY_FLAGS;

        ObjectMemoryManager &memoryManager = mSceneManager->_getLightMemoryManager();
        const size_t numRenderQueues = memoryManager.getNumRenderQueues();

        bool aoiAutogenerated = false;
        if( mAoI.empty() )
        {
            autogenerateAreaOfInterest();
            aoiAutogenerated = true;
        }

        for( size_t i=0; i<numRenderQueues; ++i )
        {
            ObjectData objData;
            const size_t totalObjs = memoryManager.getFirstObjectData( objData, i );

            for( size_t j=0; j<totalObjs; j += ARRAY_PACKED_REALS )
            {
                for( size_t k=0; k<ARRAY_PACKED_REALS; ++k )
                {
                    uint32 * RESTRICT_ALIAS visibilityFlags = objData.mVisibilityFlags;

                    if( visibilityFlags[k] & VisibilityFlags::LAYER_VISIBILITY &&
                        visibilityFlags[k] & lightMask )
                    {
                        Light *light = static_cast<Light*>( objData.mOwner[k] );
                        if( light->getType() != Light::LT_VPL )
                        {
                            Node *lightNode = light->getParentNode();
                            Vector3 diffuseCol;
                            ColourValue lightColour = light->getDiffuseColour() * light->getPowerScale();
                            diffuseCol.x = lightColour.r;
                            diffuseCol.y = lightColour.g;
                            diffuseCol.z = lightColour.b;

                            Real lightRange = light->getAttenuationRange();
                            if( light->getType() == Light::LT_DIRECTIONAL )
                                lightRange = std::numeric_limits<Real>::max();

                            size_t numAoI = mAoI.size();

                            if( light->getType() != Light::LT_DIRECTIONAL )
                                numAoI = 1;

                            for( size_t l=0; l<numAoI; ++l )
                            {
                                const AreaOfInterest &areaOfInterest = mAoI[l];
                                processLight( lightNode->_getDerivedPosition(),
                                              lightNode->_getDerivedOrientation(),
                                              light->getType(),
                                              light->getSpotlightOuterAngle(),
                                              diffuseCol,
                                              lightRange,
                                              light->getAttenuationConstant(),
                                              light->getAttenuationLinear(),
                                              light->getAttenuationQuadric(),
                                              areaOfInterest );
                            }

                            //light->setPowerScale( Math::PI * 4 );
                            //light->setPowerScale( 0 );
                        }
                    }
                }

                objData.advancePack();
            }
        }

        clusterAllVpls();

        updateExistingVpls();

        //Free memory
        mArrayRays = RawSimdUniquePtr<ArrayRay, MEMCATEGORY_GENERAL>();

        if( aoiAutogenerated )
            mAoI.clear();
    }
    //-----------------------------------------------------------------------------------
    void InstantRadiosity::freeMemory(void)
    {
        {
            MeshDataMapV2::iterator itor = mMeshDataMapV2.begin();
            MeshDataMapV2::iterator end  = mMeshDataMapV2.end();

            while( itor != end )
            {
                MeshData &meshData = itor->second;
                OGRE_FREE_SIMD( meshData.vertexData, MEMCATEGORY_GEOMETRY );
                meshData.vertexData = 0;
                if( meshData.indexData && !itor->first->getIndexBuffer()->getShadowCopy() )
                {
                    OGRE_FREE_SIMD( meshData.indexData, MEMCATEGORY_GEOMETRY );
                    meshData.indexData = 0;
                }
                ++itor;
            }

            mMeshDataMapV2.clear();
        }
        {
            MeshDataMapV1::iterator itor = mMeshDataMapV1.begin();
            MeshDataMapV1::iterator end  = mMeshDataMapV1.end();

            while( itor != end )
            {
                MeshData &meshData = itor->second;
                OGRE_FREE_SIMD( meshData.vertexData, MEMCATEGORY_GEOMETRY );
                meshData.vertexData = 0;
                if( meshData.indexData )
                {
                    OGRE_FREE_SIMD( meshData.indexData, MEMCATEGORY_GEOMETRY );
                    meshData.indexData = 0;
                }
                ++itor;
            }

            mMeshDataMapV1.clear();
        }

        mImageMap.clear();
    }
    //-----------------------------------------------------------------------------------
    void InstantRadiosity::mergeDirectionalDiffuse( const Vector3 &diffuse, const Vector3 &lightDir,
                                                    Vector3 *inOutDirDiffuse )
    {
        const Vector3 directions[6] =
        {
            Vector3(  1,  0,  0 ),
            Vector3( -1,  0,  0 ),
            Vector3(  0,  1,  0 ),
            Vector3(  0, -1,  0 ),
            Vector3(  0,  0,  1 ),
            Vector3(  0,  0, -1 )
        };

        for( int i=0; i<6; ++i )
            inOutDirDiffuse[i] += Ogre::max( lightDir.dotProduct( directions[i] ), 0 ) * diffuse;
    }
    //-----------------------------------------------------------------------------------
    void InstantRadiosity::createDebugMarkers(void)
    {
        destroyDebugMarkers();

        SceneNode *rootNode = mSceneManager->getRootSceneNode( SCENE_STATIC );

        Hlms *hlms = mHlmsManager->getHlms( HLMS_UNLIT );

        VplVec::const_iterator itor = mVpls.begin();
        VplVec::const_iterator end  = mVpls.end();

        char tmpBuffer[128];
        while( itor != end )
        {
            const Vpl &vpl = *itor;
            if( vpl.light )
            {
                SceneNode *sceneNode = rootNode->createChildSceneNode( SCENE_STATIC );
                sceneNode->setPosition( vpl.position );
                sceneNode->setScale( Vector3( mCellSize * 0.05f ) );

                ColourValue colour = vpl.light->getDiffuseColour();
                //Prevent very dark VPLs from being almost invisible
                colour = colour * ColourValue( 0.95f, 0.95f, 0.95f ) +
                        ColourValue( 0.05f, 0.05f, 0.05f );

                LwString texName( LwString::FromEmptyPointer( tmpBuffer, sizeof(tmpBuffer) ) );
                texName.a( colour.r, " ", colour.g, " ", colour.b );
                HlmsParamVec params;
                params.push_back( std::pair<IdString, String>( "diffuse", texName.c_str() ) );

                String datablockName = "InstantRadiosity_DebugMarker_" +
                        StringConverter::toString( Id::generateNewId<InstantRadiosity>() );
                HlmsDatablock *datablock = hlms->createDatablock( datablockName, datablockName,
                                                                  HlmsMacroblock(), HlmsBlendblock(),
                                                                  params, false );

                Item *item = mSceneManager->createItem( "Sphere1000.mesh",
                                                        Ogre::ResourceGroupManager::
                                                        AUTODETECT_RESOURCE_GROUP_NAME,
                                                        Ogre::SCENE_STATIC );
                item->setCastShadows( false );
                sceneNode->attachObject( item );
                item->setDatablock( datablock );
                mDebugMarkers.push_back( item );
            }

            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void InstantRadiosity::destroyDebugMarkers(void)
    {
        Hlms *hlms = mHlmsManager->getHlms( HLMS_UNLIT );

        vector<Item*>::type::const_iterator itor = mDebugMarkers.begin();
        vector<Item*>::type::const_iterator end  = mDebugMarkers.end();

        while( itor != end )
        {
            SceneNode *sceneNode = (*itor)->getParentSceneNode();
            sceneNode->getParentSceneNode()->removeAndDestroyChild( sceneNode );
            HlmsDatablock *datablock = (*itor)->getSubItem(0)->getDatablock();
            mSceneManager->destroyItem( *itor );

            hlms->destroyDatablock( datablock->getName() );
            ++itor;
        }

        mDebugMarkers.clear();
    }
    //-----------------------------------------------------------------------------------
    void InstantRadiosity::setEnableDebugMarkers( bool bEnable )
    {
        mEnableDebugMarkers = bEnable;

        if( bEnable )
            createDebugMarkers();
        else
            destroyDebugMarkers();
    }
    //-----------------------------------------------------------------------------------
    void InstantRadiosity::setUseTextures( bool bUseTextures )
    {
        if( mUseTextures != bUseTextures )
        {
            freeMemory();
            mUseTextures = bUseTextures;
        }
    }
    //-----------------------------------------------------------------------------------
    void InstantRadiosity::setUseIrradianceVolume(bool bUseIrradianceVolume)
    {
        if (bUseIrradianceVolume != mUseIrradianceVolume)
        {
            mUseIrradianceVolume = bUseIrradianceVolume;
            updateExistingVpls();
        }
    }
    //-----------------------------------------------------------------------------------
    void InstantRadiosity::suggestIrradianceVolumeParameters( const Vector3 &cellSize,
                                                              Vector3 &outVolumeOrigin,
                                                              Real &outLightMaxPower,
                                                              uint32 &outNumBlocksX,
                                                              uint32 &outNumBlocksY,
                                                              uint32 &outNumBlocksZ)
    {
        const Vector3 invCellSize = Real(1.0) / cellSize;

        int32 minBlockX = std::numeric_limits<int32>::max();
        int32 minBlockY = std::numeric_limits<int32>::max();
        int32 minBlockZ = std::numeric_limits<int32>::max();

        int32 maxBlockX = std::numeric_limits<int32>::min();
        int32 maxBlockY = std::numeric_limits<int32>::min();
        int32 maxBlockZ = std::numeric_limits<int32>::min();

        Real lightMaxPower = 0.0f;

        VplVec::const_iterator itor = mVpls.begin();
        VplVec::const_iterator end  = mVpls.end();

        while( itor != end )
        {
            const Vpl &vpl = *itor;

            Real range = mVplMaxRange;
            const Vector3 diffuseColForRange = vpl.diffuse;
            if( mVplUseIntensityForMaxRange )
            {
                double intensity;
                intensity = Ogre::max( diffuseColForRange.x, diffuseColForRange.y );
                intensity = Ogre::max( intensity, (double)diffuseColForRange.z );
                /*if( mVplQuadAtten != 0 )
                    intensity *= 1e-6 / mVplQuadAtten;*/
                double rangeInMeters = sqrt( intensity );
                range = (float)(rangeInMeters * mVplIntensityRangeMultiplier);
            }

            range = Ogre::min( range, mVplMaxRange );
            const int32 xRange = static_cast<int32>( Math::Floor( range * invCellSize.x ) );
            const int32 yRange = static_cast<int32>( Math::Floor( range * invCellSize.y ) );
            const int32 zRange = static_cast<int32>( Math::Floor( range * invCellSize.z ) );

            int32 blockX = static_cast<int32>( Math::Floor( vpl.position.x * invCellSize.x ) );
            int32 blockY = static_cast<int32>( Math::Floor( vpl.position.y * invCellSize.y ) );
            int32 blockZ = static_cast<int32>( Math::Floor( vpl.position.z * invCellSize.z ) );

            minBlockX = std::min( minBlockX, blockX - xRange );
            minBlockY = std::min( minBlockY, blockY - yRange );
            minBlockZ = std::min( minBlockZ, blockZ - zRange );

            maxBlockX = std::max( maxBlockX, blockX + xRange );
            maxBlockY = std::max( maxBlockY, blockY + yRange );
            maxBlockZ = std::max( maxBlockZ, blockZ + zRange );

            for( int i=0; i<6; ++i )
            {
                lightMaxPower = Ogre::max( lightMaxPower, vpl.dirDiffuse[i].x );
                lightMaxPower = Ogre::max( lightMaxPower, vpl.dirDiffuse[i].y );
                lightMaxPower = Ogre::max( lightMaxPower, vpl.dirDiffuse[i].z );
            }

            ++itor;
        }

        outNumBlocksX = maxBlockX - minBlockX + 1;
        outNumBlocksY = maxBlockY - minBlockY + 1;
        outNumBlocksZ = maxBlockZ - minBlockZ + 1;

        outVolumeOrigin.x = static_cast<Real>( minBlockX ) * cellSize.x;
        outVolumeOrigin.y = static_cast<Real>( minBlockY ) * cellSize.y;
        outVolumeOrigin.z = static_cast<Real>( minBlockZ ) * cellSize.z;

        outLightMaxPower = lightMaxPower;
    }
    //-----------------------------------------------------------------------------------
    void InstantRadiosity::fillIrradianceVolume( IrradianceVolume *volume,
                                                 Vector3 cellSize, Vector3 volumeOrigin,
                                                 Real lightMaxPower, bool fadeAttenuationOverDistance )
    {
        if (!volume) return;

        const Vector3 invCellSize  = Real(1.0) / cellSize;

        //Quantize volumeCenter.
        volumeOrigin.x = static_cast<int32>( Math::Floor( volumeOrigin.x * invCellSize.x ) );
        volumeOrigin.y = static_cast<int32>( Math::Floor( volumeOrigin.y * invCellSize.y ) );
        volumeOrigin.z = static_cast<int32>( Math::Floor( volumeOrigin.z * invCellSize.z ) );

        volume->setIrradianceOrigin( volumeOrigin * cellSize );
        volume->setIrradianceCellSize( cellSize );
        volume->setIrradianceMaxPower( lightMaxPower );
        volume->setPowerScale( mVplPowerBoost );
        volume->setFadeAttenuationOverDistace( fadeAttenuationOverDistance );

        const int32 volumeOriginX = static_cast<int32>( volumeOrigin.x );
        const int32 volumeOriginY = static_cast<int32>( volumeOrigin.y );
        const int32 volumeOriginZ = static_cast<int32>( volumeOrigin.z );

        const Real invMaxPower = 1.0f / lightMaxPower;

        const int32 numBlocksX = volume->getNumBlocksX();
        const int32 numBlocksY = volume->getNumBlocksY();
        const int32 numBlocksZ = volume->getNumBlocksZ();

        VplVec::const_iterator itor = mVpls.begin();
        VplVec::const_iterator end  = mVpls.end();

        volume->clearVolumeData();

        const Vector3 c_directions[6] =
        {
            Vector3(  1,  0,  0 ),
            Vector3( -1,  0,  0 ),
            Vector3(  0,  1,  0 ),
            Vector3(  0, -1,  0 ),
            Vector3(  0,  0,  1 ),
            Vector3(  0,  0, -1 )
        };

        while( itor != end )
        {
            const Vpl &vpl = *itor;

            Real range = mVplMaxRange;
            const Vector3 diffuseColForRange = vpl.diffuse * mVplPowerBoost;
            if( mVplUseIntensityForMaxRange )
            {
                double intensity;
                intensity = Ogre::max( diffuseColForRange.x, diffuseColForRange.y );
                intensity = Ogre::max( intensity, (double)diffuseColForRange.z );
                /*if( mVplQuadAtten != 0 )
                    intensity *= 1e-6 / mVplQuadAtten;*/
                double rangeInMeters = sqrt( intensity );
                range = (float)(rangeInMeters * mVplIntensityRangeMultiplier);
            }

            range = Ogre::min( range, mVplMaxRange );

            const int32 xRange = static_cast<int32>( Math::Floor( range * invCellSize.x ) );
            const int32 yRange = static_cast<int32>( Math::Floor( range * invCellSize.y ) );
            const int32 zRange = static_cast<int32>( Math::Floor( range * invCellSize.z ) );

            int32 blockX = static_cast<int32>( Math::Floor( vpl.position.x * invCellSize.x ) );
            int32 blockY = static_cast<int32>( Math::Floor( vpl.position.y * invCellSize.y ) );
            int32 blockZ = static_cast<int32>( Math::Floor( vpl.position.z * invCellSize.z ) );

            blockX -= volumeOriginX;
            blockY -= volumeOriginY;
            blockZ -= volumeOriginZ;

            const int32 minBlockX = std::max( 0, blockX - xRange );
            const int32 minBlockY = std::max( 0, blockY - yRange );
            const int32 minBlockZ = std::max( 0, blockZ - zRange );

            const int32 maxBlockX = std::min( numBlocksX - 1, blockX + xRange );
            const int32 maxBlockY = std::min( numBlocksY - 1, blockY + yRange);
            const int32 maxBlockZ = std::min( numBlocksZ - 1, blockZ + zRange);

            if (maxBlockX >= 0 && minBlockX < numBlocksX &&
                maxBlockY >= 0 && minBlockY < numBlocksY &&
                maxBlockZ >= 0 && minBlockZ < numBlocksZ)
            {
                for( int32 z=minBlockZ; z<=maxBlockZ; ++z )
                {
                    for( int32 y=minBlockY; y<=maxBlockY; ++y )
                    {
                        for( int32 x=minBlockX; x<=maxBlockX; ++x )
                        {
                            Vector3 vplToCell = Vector3( x - blockX, y - blockY, z - blockZ );
                            vplToCell *= cellSize;
                            Real distance = vplToCell.normalise();
                            if( vplToCell.dotProduct( vpl.normal ) < 0 )
                                continue;

                            Real atten = Real(1.0f) /
                                    (mVplConstAtten + (mVplLinearAtten +
                                                       mVplQuadAtten * distance) * distance);
                            atten = Ogre::min( Real(1.0f), atten );
                            if( fadeAttenuationOverDistance )
                                atten *= Ogre::max( (range - distance) / range, Ogre::Real( 0.0f ) );

                            const Vector3 diffuseCol = vpl.diffuse * invMaxPower * atten;
                            for( int i=0; i<6; ++i )
                            {
                                if( x != blockX || y != blockY || z != blockZ )
                                {
                                    Vector3 finalCol = Ogre::max(
                                                -vplToCell.dotProduct( c_directions[i] ),
                                                0 ) * diffuseCol;
                                    volume->changeVolumeData( x, y, z, i, finalCol );
                                }
                                else
                                {
                                    volume->changeVolumeData( x, y, z, i, vpl.dirDiffuse[i] * invMaxPower );
                                }
                            }
                        }
                    }
                }
            }

            ++itor;
        }

        volume->updateIrradianceVolumeTexture();
    }
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    InstantRadiosity::SparseCluster::SparseCluster() {}
    //-----------------------------------------------------------------------------------
    InstantRadiosity::SparseCluster::SparseCluster( int32 blockX, int32 blockY, int32 blockZ,
                                                    const Vector3 &_diffuse, const Vector3 &dir,
                                                    const Vector3 _dirDiffuse[6] ) :
        diffuse( _diffuse ), direction( dir )
    {
        blockHash[0] = blockX;
        blockHash[1] = blockY;
        blockHash[2] = blockZ;

        memcpy( dirDiffuse, _dirDiffuse, sizeof(dirDiffuse) );
    }
    //-----------------------------------------------------------------------------------
    InstantRadiosity::SparseCluster::SparseCluster( int32 _blockHash[3] ) :
        diffuse( Vector3::ZERO ), direction( Vector3::ZERO )
    {
        blockHash[0] = _blockHash[0];
        blockHash[1] = _blockHash[1];
        blockHash[2] = _blockHash[2];

        memset( dirDiffuse, 0, sizeof(dirDiffuse) );
    }
    //-----------------------------------------------------------------------------------
    bool InstantRadiosity::SparseCluster::operator () ( const SparseCluster &_l, int32 _r[3] ) const
    {
        return memcmp( _l.blockHash, _r, sizeof( blockHash ) ) < 0;
    }
    bool InstantRadiosity::SparseCluster::operator () ( int32 _l[3], const SparseCluster &_r ) const
    {
        return memcmp( _l, _r.blockHash, sizeof( blockHash ) ) < 0;
    }
    bool InstantRadiosity::SparseCluster::operator () ( const SparseCluster &_l,
                                                         const SparseCluster &_r ) const
    {
        return memcmp( _l.blockHash, _r.blockHash, sizeof( blockHash ) ) < 0;
    }
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    float* InstantRadiosity::MeshData::getUvStart( uint8_t uvSet ) const
    {
        return vertexData + numVertices * 3u + uvSet * 2u;
    }
}
