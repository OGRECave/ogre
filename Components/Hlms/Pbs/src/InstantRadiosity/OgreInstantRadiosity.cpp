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
#include "OgreHlmsPbsDatablock.h"
#include "OgreHlmsPbs.h"
#include "OgreHlmsManager.h"

#include "OgreRay.h"
#include "OgreLight.h"
#include "OgreSceneManager.h"
#include "Vao/OgreAsyncTicket.h"

#include "Math/Array/OgreBooleanMask.h"

#include "OgreItem.h"

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
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
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
    };
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    InstantRadiosity::InstantRadiosity( SceneManager *sceneManager, HlmsManager *hlmsManager ) :
        mSceneManager( sceneManager ),
        mHlmsManager( hlmsManager ),
        mVisibilityMask( 0xffffffff ),
        mFirstRq( 0 ),
        mLastRq( 255 ),
        mLightMask( 0xffffffff ),
        //mNumRays( 10000 ),
        mNumRays( 32 ),
        mCellSize( 2 ),
        mVplMaxRange( 12 ),
        mVplConstAtten( 0.5 ),
        mVplLinearAtten( 0.5 ),
        mVplQuadAtten( 0 ),
        mVplThreshold( 0.0 ),
        mBias( 0.97f ),
        mNumSpreadIterations( 6 ),
        mSpreadThreshold( 0.0004 ),
        mVplPowerBoost( 2.0f ),
        mEnableDebugMarkers( false )
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
        Vector3 diffuseTerm = /*NdotL **/ hit.materialDiffuse * lightColour;

    #if 0
        if( hasUVs )
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
            const Real a0 = f1.crossProduct( f2 ).length() * hit.invTriArea;
            const Real a1 = f2.crossProduct( f0 ).length() * hit.invTriArea;
            const Real a2 = f0.crossProduct( f1 ).length() * hit.invTriArea;

            const Vector2 interpUV = uv[0] * a0 + uv[1] * a1 + uv[2] * a2;
        }
    #endif

        Vpl vpl;
        vpl.light = 0;
        vpl.diffuse = diffuseTerm;
        vpl.normal = hit.triNormal;
        vpl.position = pointOnTri;
        vpl.numMergedVpls = 1.0f;

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

            Real atten = Real(1.0f) /
                    (attenConst + (attenLinear + attenQuad * hit.distance) * hit.distance);
            atten = Ogre::min( Real(1.0f), atten );

            const Vector3 pointOnTri = hit.ray.getPoint( hit.distance * bias );

            const int32 blockX = static_cast<int32>( Math::Floor( pointOnTri.x * cellSize ) );
            const int32 blockY = static_cast<int32>( Math::Floor( pointOnTri.y * cellSize ) );
            const int32 blockZ = static_cast<int32>( Math::Floor( pointOnTri.z * cellSize ) );

            Vpl vpl = convertToVpl( lightColour, pointOnTri, hit );
            vpl.diffuse *= atten;

            Real numCollectedVpls = 1.0f;

            //Merge the lights (simple average) that lie in the same cluster.
            RayHitVec::iterator itor = mRayHits.begin() + 1u;
            RayHitVec::iterator end  = mRayHits.end();

            while( itor != end )
            {
                const RayHit &alikeHit = *itor;

                Real alikeAtten = Real(1.0f) /
                        (attenConst + (attenLinear + attenQuad * alikeHit.distance) * alikeHit.distance);
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
            vpl.diffuse /= mNumRays;
            vpl.position/= numCollectedVpls;
            vpl.normal.normalise();
            vpl.numMergedVpls = numCollectedVpls;

            mVpls.push_back( vpl );

            mTmpSparseClusters[0].insert( SparseCluster( blockX, blockY, blockZ,
                                                         vpl.diffuse, vpl.normal ) );

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

                const Vector3 diffuseCol = NdotL * itor->diffuse * invNumSpreadIterations;

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

                    //TODO: Consider attenuation.
                    gridCluster->diffuse    += diffuseCol;
                    gridCluster->direction  += itor->direction;
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
    void InstantRadiosity::autogenerateAreaOfInfluence(void)
    {
        AxisAlignedBox areaOfInfluence;
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
                areaOfInfluence.merge( tmpBox );
            }
        }

        mAoI.push_back( Aabb::newFromExtents( areaOfInfluence.getMinimum(),
                                              areaOfInfluence.getMaximum() ) );
    }
    //-----------------------------------------------------------------------------------
    void InstantRadiosity::processLight( Vector3 lightPos, const Quaternion &lightRot, uint8 lightType,
                                         Radian angle, Vector3 lightColour, Real lightRange,
                                         Real attenConst, Real attenLinear, Real attenQuad,
                                         const Aabb &areaOfInfluence )
    {
        Aabb rotatedAoI = areaOfInfluence;
        {
            Matrix4 rotMatrix;
            rotMatrix.makeTransform( Vector3::ZERO, Vector3::UNIT_SCALE, lightRot.Inverse() );
            rotatedAoI.transformAffine( rotMatrix );
        }

        //Same RNG/seed for every object & triangle
        RandomNumberGenerator rng;
        mRayHits.resize( mNumRays );

        ArrayRay * RESTRICT_ALIAS arrayRays = mArrayRays.get();

        for( size_t i=0; i<mNumRays; ++i )
        {
            mRayHits[i].distance = std::numeric_limits<Real>::max();

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
                randomPos.z = rotatedAoI.mHalfSize.z + 1.0f;
                randomPos = lightRot * randomPos + areaOfInfluence.mCenter;

                mRayHits[i].ray.setOrigin( randomPos );
                mRayHits[i].ray.setDirection( -lightRot.zAxis() );
            }

            arrayRays->mOrigin.setAll( mRayHits[i].ray.getOrigin() );
            arrayRays->mDirection.setAll( mRayHits[i].ray.getDirection() );
            ++arrayRays;
        }

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
                                       areaOfInfluence );
            }
        }

        generateAndClusterVpls( lightColour, attenConst, attenLinear, attenQuad );
    }
    //-----------------------------------------------------------------------------------
    const InstantRadiosity::MeshData* InstantRadiosity::downloadVao( VertexArrayObject *vao )
    {
        MeshDataMapV2::const_iterator itor = mMeshDataMapV2.find( vao );
        if( itor != mMeshDataMapV2.end() )
            return &itor->second;

        const VertexBufferPackedVec &vertexBuffers = vao->getVertexBuffers();
        IndexBufferPacked *indexBuffer = vao->getIndexBuffer();

        MeshData meshData;
        memset( &meshData, 0, sizeof(meshData) );

        size_t posIdx, posOffset;
        const VertexElement2 *posElement = vao->findBySemantic( VES_POSITION, posIdx, posOffset );

        //Issue all async requests now.
        AsyncTicketPtr posTicket;
        AsyncTicketPtr indexTicket;

        if( !vertexBuffers[posIdx]->getShadowCopy() )
        {
            if( !indexBuffer )
            {
                posTicket = vertexBuffers[posIdx]->readRequest( vao->getPrimitiveStart(),
                                                                vao->getPrimitiveCount() );
            }
            else
            {
                posTicket =
                        vertexBuffers[posIdx]->readRequest( 0, vertexBuffers[posIdx]->getNumElements() );
            }
        }

        if( indexBuffer && !indexBuffer->getShadowCopy() )
        {
            indexTicket = indexBuffer->readRequest( vao->getPrimitiveStart(),
                                                    vao->getPrimitiveCount() );
        }

        if( indexBuffer )
        {
            meshData.numVertices = vertexBuffers[posIdx]->getNumElements();
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
        meshData.vertexPos = reinterpret_cast<float*>(
                    OGRE_MALLOC_SIMD( meshData.numVertices * sizeof(float) * 3u,
                                      MEMCATEGORY_GEOMETRY ) );

        //Copy position
        bool isHalf = v1::VertexElement::getBaseType( posElement->mType ) == VET_HALF2;
        uint8 const *posBuffer = 0;

        if( !vertexBuffers[posIdx]->getShadowCopy() )
        {
            posBuffer = reinterpret_cast<uint8 const * RESTRICT_ALIAS>( posTicket->map() ) + posOffset;
        }
        else
        {
            posBuffer = reinterpret_cast<uint8 const * RESTRICT_ALIAS>(
                        vertexBuffers[posIdx]->getShadowCopy() ) + posOffset;
        }

        if( !indexBuffer )
            posBuffer += vao->getPrimitiveStart() * vertexBuffers[posIdx]->getBytesPerElement();

        for( size_t i=0; i<meshData.numVertices; ++i )
        {
            if( isHalf )
            {
                uint16 const * RESTRICT_ALIAS posBuffer16 =
                        reinterpret_cast<uint16 const * RESTRICT_ALIAS>( posBuffer );
                meshData.vertexPos[i*3u + 0u] = Bitwise::halfToFloat( posBuffer16[0] );
                meshData.vertexPos[i*3u + 1u] = Bitwise::halfToFloat( posBuffer16[1] );
                meshData.vertexPos[i*3u + 2u] = Bitwise::halfToFloat( posBuffer16[2] );
            }
            else
            {
                float const * RESTRICT_ALIAS posBufferF32 =
                        reinterpret_cast<float const * RESTRICT_ALIAS>( posBuffer );
                meshData.vertexPos[i*3u + 0u] = posBufferF32[0];
                meshData.vertexPos[i*3u + 1u] = posBufferF32[1];
                meshData.vertexPos[i*3u + 2u] = posBufferF32[2];
            }

            posBuffer += vertexBuffers[posIdx]->getBytesPerElement();
        }

        if( !posTicket.isNull() )
            posTicket->unmap();

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

        const v1::VertexElement *posElement = renderOp.vertexData->vertexDeclaration->
                findElementBySemantic( VES_POSITION );

        size_t posIdx, posOffset;
        posIdx = posElement->getSource();
        posOffset = posElement->getOffset();

        MeshData meshData;
        memset( &meshData, 0, sizeof(meshData) );

        meshData.numVertices = renderOp.vertexData->vertexCount;
        meshData.vertexPos = reinterpret_cast<float*>(
                    OGRE_MALLOC_SIMD( meshData.numVertices * sizeof(float) * 3u,
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

        //Copy position
        bool isHalf = v1::VertexElement::getBaseType( posElement->getType() ) == VET_HALF2;
        uint8 const *posBuffer = reinterpret_cast<uint8 const * RESTRICT_ALIAS>(
                    renderOp.vertexData->vertexBufferBinding->
                    getBuffer(posIdx)->lock( v1::HardwareBuffer::HBL_READ_ONLY ) );

        posBuffer += posOffset;
        if( !renderOp.useIndexes )
            posBuffer += renderOp.vertexData->vertexStart;

        for( size_t i=0; i<meshData.numVertices; ++i )
        {
            if( isHalf )
            {
                uint16 const * RESTRICT_ALIAS posBuffer16 =
                        reinterpret_cast<uint16 const * RESTRICT_ALIAS>( posBuffer );
                meshData.vertexPos[i*3u + 0u] = Bitwise::halfToFloat( posBuffer16[0] );
                meshData.vertexPos[i*3u + 1u] = Bitwise::halfToFloat( posBuffer16[1] );
                meshData.vertexPos[i*3u + 2u] = Bitwise::halfToFloat( posBuffer16[2] );
            }
            else
            {
                float const * RESTRICT_ALIAS posBufferF32 =
                        reinterpret_cast<float const * RESTRICT_ALIAS>( posBuffer );
                meshData.vertexPos[i*3u + 0u] = posBufferF32[0];
                meshData.vertexPos[i*3u + 1u] = posBufferF32[1];
                meshData.vertexPos[i*3u + 2u] = posBufferF32[2];
            }

            posBuffer += renderOp.vertexData->vertexDeclaration->getVertexSize(posIdx);
        }

        renderOp.vertexData->vertexBufferBinding->getBuffer(posIdx)->unlock();

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
    void InstantRadiosity::testLightVsAllObjects( uint8 lightType, Real lightRange,
                                                  ObjectData objData, size_t numNodes,
                                                  const Aabb &scalarAreaOfInfluence )
    {
        const size_t numRays = mNumRays;
        const ArrayInt sceneFlags = Mathlib::SetAll( mVisibilityMask &
                                                     VisibilityFlags::RESERVED_VISIBILITY_FLAGS );
        ArrayAabb areaOfInfluence( ArrayVector3::ZERO, ArrayVector3::ZERO );
        areaOfInfluence.setAll( scalarAreaOfInfluence );

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
                ArrayMaskI hitMask = CastRealToInt( areaOfInfluence.intersects( *objData.mWorldAabb ) );
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
            ArrayRay * RESTRICT_ALIAS arrayRays = mArrayRays.get();
            for( size_t j=0; j<numRays; ++j )
            {
                ArrayMaskR rayHits = arrayRays->intersects( *objData.mWorldAabb );
                uint32 scalarRayHits = BooleanMask4::getScalarMask( rayHits );
                for( size_t k=0; k<ARRAY_PACKED_REALS; ++k )
                {
                    if( IS_BIT_SET( k, scalarRayHits ) )
                        mTmpRaysThatHitObject[k].push_back( j );
                }

                ++arrayRays;
            }

            for( size_t j=0; j<ARRAY_PACKED_REALS; ++j )
            {
                //Convert isInAreaOfIterest into something smaller we can work with.
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
                            HlmsPbsDatablock *pbsDatablock = static_cast<HlmsPbsDatablock*>( datablock );
                            //TODO: Should we account fresnel here? What about metalness?
                            Vector3 materialDiffuse = pbsDatablock->getDiffuse();
                            if( pbsDatablock->getTexture( PBSM_DIFFUSE ).isNull() )
                            {
                                const ColourValue &bgDiffuse = pbsDatablock->getBackgroundDiffuse();
                                materialDiffuse.x *= bgDiffuse.r;
                                materialDiffuse.y *= bgDiffuse.g;
                                materialDiffuse.z *= bgDiffuse.b;
                            }
                            raycastLightRayVsMesh( lightRange, *meshData,
                                                   worldMatrix, materialDiffuse,
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
                                                  Matrix4 worldMatrix, Vector3 materialDiffuse,
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

            if( meshData.indexData )
            {
                if( meshData.useIndices16bit )
                {
                    triVerts[0].x = meshData.vertexPos[indexData16[i+0] * 3u + 0];
                    triVerts[0].y = meshData.vertexPos[indexData16[i+0] * 3u + 1];
                    triVerts[0].z = meshData.vertexPos[indexData16[i+0] * 3u + 2];
                    triVerts[1].x = meshData.vertexPos[indexData16[i+1] * 3u + 0];
                    triVerts[1].y = meshData.vertexPos[indexData16[i+1] * 3u + 1];
                    triVerts[1].z = meshData.vertexPos[indexData16[i+1] * 3u + 2];
                    triVerts[2].x = meshData.vertexPos[indexData16[i+2] * 3u + 0];
                    triVerts[2].y = meshData.vertexPos[indexData16[i+2] * 3u + 1];
                    triVerts[2].z = meshData.vertexPos[indexData16[i+2] * 3u + 2];
                }
                else
                {
                    triVerts[0].x = meshData.vertexPos[indexData32[i+0] * 3u + 0];
                    triVerts[0].y = meshData.vertexPos[indexData32[i+0] * 3u + 1];
                    triVerts[0].z = meshData.vertexPos[indexData32[i+0] * 3u + 2];
                    triVerts[1].x = meshData.vertexPos[indexData32[i+1] * 3u + 0];
                    triVerts[1].y = meshData.vertexPos[indexData32[i+1] * 3u + 1];
                    triVerts[1].z = meshData.vertexPos[indexData32[i+1] * 3u + 2];
                    triVerts[2].x = meshData.vertexPos[indexData32[i+2] * 3u + 0];
                    triVerts[2].y = meshData.vertexPos[indexData32[i+2] * 3u + 1];
                    triVerts[2].z = meshData.vertexPos[indexData32[i+2] * 3u + 2];
                }
            }
            else
            {
                triVerts[0].x = meshData.vertexPos[(i+0) * 3u + 0];
                triVerts[0].y = meshData.vertexPos[(i+0) * 3u + 1];
                triVerts[0].z = meshData.vertexPos[(i+0) * 3u + 2];
                triVerts[1].x = meshData.vertexPos[(i+1) * 3u + 0];
                triVerts[1].y = meshData.vertexPos[(i+1) * 3u + 1];
                triVerts[1].z = meshData.vertexPos[(i+1) * 3u + 2];
                triVerts[2].x = meshData.vertexPos[(i+2) * 3u + 0];
                triVerts[2].y = meshData.vertexPos[(i+2) * 3u + 1];
                triVerts[2].z = meshData.vertexPos[(i+2) * 3u + 2];
            }

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
                //TODO_fill_uv;
                Ray ray = mRayHits[*itRayIdx].ray;

                const std::pair<bool, Real> inters = Math::intersects(
                            ray, triVerts[0], triVerts[1], triVerts[2], triNormal, true, false );

                if( inters.first )
                {
                    if( inters.second < mRayHits[*itRayIdx].distance &&
                        inters.second <= lightRange )
                    {
                        mRayHits[*itRayIdx].distance = inters.second;
                        mRayHits[*itRayIdx].materialDiffuse = materialDiffuse;
                        mRayHits[*itRayIdx].triVerts[0] = triVerts[0];
                        mRayHits[*itRayIdx].triVerts[1] = triVerts[1];
                        mRayHits[*itRayIdx].triVerts[2] = triVerts[2];
                        mRayHits[*itRayIdx].triNormal = triNormal;
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
            if( diffuseCol.x >= mVplThreshold ||
                diffuseCol.y >= mVplThreshold ||
                diffuseCol.z >= mVplThreshold )
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
                vpl.light->setDiffuseColour( colour );
                vpl.light->setSpecularColour( ColourValue::Black );
                vpl.light->setAttenuation( mVplMaxRange, mVplConstAtten,
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

        mArrayRays = RawSimdUniquePtr<ArrayRay, MEMCATEGORY_GENERAL>( mNumRays );

        const uint32 lightMask = mLightMask & VisibilityFlags::RESERVED_VISIBILITY_FLAGS;

        ObjectMemoryManager &memoryManager = mSceneManager->_getLightMemoryManager();
        const size_t numRenderQueues = memoryManager.getNumRenderQueues();

        bool aoiAutogenerated = false;
        if( mAoI.empty() )
        {
            autogenerateAreaOfInfluence();
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
                                const Aabb &areaOfInfluence = mAoI[l];
                                processLight( lightNode->_getDerivedPosition(),
                                              lightNode->_getDerivedOrientation(),
                                              light->getType(),
                                              light->getSpotlightOuterAngle(),
                                              diffuseCol,
                                              lightRange,
                                              light->getAttenuationConstant(),
                                              light->getAttenuationLinear(),
                                              light->getAttenuationQuadric(),
                                              areaOfInfluence );
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
                OGRE_FREE_SIMD( meshData.vertexPos, MEMCATEGORY_GEOMETRY );
                meshData.vertexPos = 0;
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
                OGRE_FREE_SIMD( meshData.vertexPos, MEMCATEGORY_GEOMETRY );
                meshData.vertexPos = 0;
                if( meshData.indexData )
                {
                    OGRE_FREE_SIMD( meshData.indexData, MEMCATEGORY_GEOMETRY );
                    meshData.indexData = 0;
                }
                ++itor;
            }

            mMeshDataMapV1.clear();
        }
    }
    //-----------------------------------------------------------------------------------
    void InstantRadiosity::createDebugMarkers(void)
    {
        destroyDebugMarkers();

        SceneNode *rootNode = mSceneManager->getRootSceneNode( SCENE_STATIC );

        Hlms *hlms = mHlmsManager->getHlms( HLMS_UNLIT );

        VplVec::const_iterator itor = mVpls.begin();
        VplVec::const_iterator end  = mVpls.end();

        while( itor != end )
        {
            const Vpl &vpl = *itor;
            if( vpl.light )
            {
                SceneNode *sceneNode = rootNode->createChildSceneNode( SCENE_STATIC );
                sceneNode->setPosition( vpl.position );
                sceneNode->setScale( Vector3( mCellSize * 0.05f ) );

                Item *item = mSceneManager->createItem( "Sphere1000.mesh",
                                                        Ogre::ResourceGroupManager::
                                                        AUTODETECT_RESOURCE_GROUP_NAME,
                                                        Ogre::SCENE_STATIC );
                sceneNode->attachObject( item );
                item->setDatablock( hlms->getDefaultDatablock() );
                mDebugMarkers.push_back( item );
            }

            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void InstantRadiosity::destroyDebugMarkers(void)
    {
        vector<Item*>::type::const_iterator itor = mDebugMarkers.begin();
        vector<Item*>::type::const_iterator end  = mDebugMarkers.end();

        while( itor != end )
        {
            SceneNode *sceneNode = (*itor)->getParentSceneNode();
            sceneNode->getParentSceneNode()->removeAndDestroyChild( sceneNode );
            mSceneManager->destroyItem( *itor );
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
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    InstantRadiosity::SparseCluster::SparseCluster() {}
    //-----------------------------------------------------------------------------------
    InstantRadiosity::SparseCluster::SparseCluster( int32 blockX, int32 blockY, int32 blockZ,
                                                    const Vector3 _diffuse, const Vector3 dir ) :
        diffuse( _diffuse ), direction( dir )
    {
        blockHash[0] = blockX;
        blockHash[1] = blockY;
        blockHash[2] = blockZ;
    }
    //-----------------------------------------------------------------------------------
    InstantRadiosity::SparseCluster::SparseCluster( int32 _blockHash[3] ) :
        diffuse( Vector3::ZERO ), direction( Vector3::ZERO )
    {
        blockHash[0] = _blockHash[0];
        blockHash[1] = _blockHash[1];
        blockHash[2] = _blockHash[2];
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
}
