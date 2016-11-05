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

#if OGRE_COMPILER == OGRE_COMPILER_MSVC
    #include <random>
#else
    #include <tr1/random>
#endif

namespace Ogre
{
    class RandomNumberGenerator
    {
        std::tr1::mt19937   mRng;

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
        mVplThreshold( 0.05 ),
        mBias( 0.97f ),
        mVplPowerBoost( 2.0f )
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

        return vpl;
    }
    //-----------------------------------------------------------------------------------
    void InstantRadiosity::clusterLights( Vector3 lightPos, Vector3 lightColour,
                                          Real attenConst, Real attenLinear, Real attenQuad )
    {
        assert( mCellSize > 0 );

        int32 blockX, blockY, blockZ;
        const Real cellSize = Real(1.0) / mCellSize;

        const Real bias = mBias;

        while( !mRayHits.empty() )
        {
            const RayHit &hit = mRayHits.front();

            if( hit.distance >= std::numeric_limits<Real>::max() )
            {
                efficientVectorRemove( mRayHits, mRayHits.begin() );
                continue;
            }

            Real atten = Real(1.0f) /
                    (attenConst + (attenLinear + attenQuad * hit.distance) * hit.distance);
            atten = Ogre::min( Real(1.0f), atten );

            const Vector3 pointOnTri = lightPos + hit.rayDir * hit.distance * bias;

            blockX = static_cast<int32>( Math::Floor( pointOnTri.x * cellSize ) );
            blockY = static_cast<int32>( Math::Floor( pointOnTri.y * cellSize ) );
            blockZ = static_cast<int32>( Math::Floor( pointOnTri.z * cellSize ) );

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

                const Vector3 pointOnTri02 = lightPos + alikeHit.rayDir * alikeHit.distance * bias;

                const int32 alikeBlockX = static_cast<int32>( Math::Floor( pointOnTri02.x * cellSize ) );
                const int32 alikeBlockY = static_cast<int32>( Math::Floor( pointOnTri02.y * cellSize ) );
                const int32 alikeBlockZ = static_cast<int32>( Math::Floor( pointOnTri02.z * cellSize ) );

                if( blockX == alikeBlockX && blockY == alikeBlockY && blockZ == alikeBlockZ )
                {
                    Vpl alikeVpl = convertToVpl( lightColour, pointOnTri02, alikeHit );
                    vpl.diffuse += alikeVpl.diffuse * alikeAtten;
                    vpl.normal  += alikeVpl.normal;
                    vpl.radius  += alikeVpl.radius;
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
            vpl.radius  /= numCollectedVpls;
            vpl.position/= numCollectedVpls;
            vpl.normal.normalise();

            vpl.diffuse /= mNumRays;

            vpl.radius = 1.0f;

            mVpls.push_back( vpl );

            efficientVectorRemove( mRayHits, mRayHits.begin() );
        }
    }
    //-----------------------------------------------------------------------------------
    void InstantRadiosity::processLight( Vector3 lightPos, const Vector3 &lightDir,
                                         Radian angle, Vector3 lightColour,
                                         Real attenConst, Real attenLinear, Real attenQuad )
    {
        //Same RNG/seed for every object & triangle
        RandomNumberGenerator rng;
        mRayHits.resize( mNumRays );

        for( size_t i=0; i<mNumRays; ++i )
        {
            mRayHits[i].distance = std::numeric_limits<Real>::max();
            //TODO: Respect lightDir & angle
            mRayHits[i].rayDir = rng.getRandomDir();
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
                processLight( lightPos, objData, totalObjs );
            }
        }

        clusterLights( lightPos, lightColour, attenConst, attenLinear, attenQuad );
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
            if( indexBuffer )
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

        //TODO: Free these structs
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

        //TODO: Free these structs
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
    void InstantRadiosity::processLight( const Vector3 &lightPos, ObjectData objData, size_t numNodes )
    {
        const uint32 visibilityMask = mVisibilityMask & VisibilityFlags::RESERVED_VISIBILITY_FLAGS;
        for( size_t i=0; i<numNodes; i += ARRAY_PACKED_REALS )
        {
            for( size_t j=0; j<ARRAY_PACKED_REALS; ++j )
            {
                uint32 * RESTRICT_ALIAS visibilityFlags = objData.mVisibilityFlags;

                if( visibilityFlags[j] & VisibilityFlags::LAYER_VISIBILITY &&
                    visibilityFlags[j] & visibilityMask )
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
                            processLight( lightPos, *meshData, worldMatrix, materialDiffuse );
                        }

                        ++itor;
                    }
                }
            }

            objData.advancePack();
        }
    }
    //-----------------------------------------------------------------------------------
    void InstantRadiosity::processLight( const Vector3 &lightPos, const MeshData meshData,
                                         Matrix4 worldMatrix, Vector3 materialDiffuse )
    {
        Ray ray;
        ray.setOrigin( lightPos );

        const size_t numRays = mNumRays;
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

            for( size_t j=0; j<numRays; ++j )
            {
                //TODO: create an array of rays that hit the AABB of the object.
                //TODO_fill_uv;
                ray.setDirection( mRayHits[j].rayDir );

                const std::pair<bool, Real> inters = Math::intersects(
                            ray, triVerts[0], triVerts[1], triVerts[2], triNormal, true, false );

                if( inters.first )
                {
                    if( inters.second < mRayHits[j].distance )
                    {
                        mRayHits[j].distance = inters.second;
                        mRayHits[j].materialDiffuse = materialDiffuse;
                        mRayHits[j].triVerts[0] = triVerts[0];
                        mRayHits[j].triVerts[1] = triVerts[1];
                        mRayHits[j].triVerts[2] = triVerts[2];
                        mRayHits[j].triNormal = triNormal;
                    }
                }
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
            if( vpl.diffuse.x >= mVplThreshold ||
                vpl.diffuse.y >= mVplThreshold ||
                vpl.diffuse.z >= mVplThreshold )
            {
                if( !vpl.light )
                {
                    SceneNode *lightNode = rootNode->createChildSceneNode( SCENE_DYNAMIC );
                    vpl.light = mSceneManager->createLight();
                    vpl.light->setType( Light::LT_VPL );
                    lightNode->attachObject( vpl.light );
                    lightNode->setPosition( vpl.position );
#if 0
                    Item *item = mSceneManager->createItem( "Sphere1000.mesh",
                                                            Ogre::ResourceGroupManager::
                                                            AUTODETECT_RESOURCE_GROUP_NAME,
                                                            Ogre::SCENE_DYNAMIC);
                    lightNode->scale( mVplMaxRange );
                    lightNode->attachObject( item );
#endif
                }

                ColourValue colour;
                colour.r = vpl.diffuse.x * mVplPowerBoost;
                colour.g = vpl.diffuse.y * mVplPowerBoost;
                colour.b = vpl.diffuse.z * mVplPowerBoost;
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
#if 0
                mSceneManager->destroyItem(  );
#endif
            }

            ++itor;
        }
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
#if 0
                mSceneManager->destroyItem(  );
#endif
            }

            ++itor;
        }

        mVpls.clear();
    }
    //-----------------------------------------------------------------------------------
    void InstantRadiosity::build(void)
    {
        clear();

        Hlms *hlms = mHlmsManager->getHlms( HLMS_PBS );
        if( !dynamic_cast<HlmsPbs*>( hlms ) )
        {
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                         "This InstantRadiosity is designed to downcast HlmsDatablock into "
                         "HlmsPbsDatablock, and it cannot understand datablocks made by other Hlms "
                         "implementations.",
                         "InstantRadiosity::build" );
        }

        const uint32 lightMask = mLightMask & VisibilityFlags::RESERVED_VISIBILITY_FLAGS;

        ObjectMemoryManager &memoryManager = mSceneManager->_getLightMemoryManager();
        const size_t numRenderQueues = memoryManager.getNumRenderQueues();

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

                            processLight( lightNode->_getDerivedPosition(),
                                          light->getDirection(),
                                          light->getSpotlightOuterAngle(),
                                          diffuseCol,
                                          light->getAttenuationConstant(),
                                          light->getAttenuationLinear(),
                                          light->getAttenuationQuadric() );

                            //light->setPowerScale( Math::PI * 4 );
                            //light->setPowerScale( 0 );
                        }
                    }
                }

                objData.advancePack();
            }
        }

        updateExistingVpls();
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
}
