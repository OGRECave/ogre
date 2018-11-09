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

#include "OgreRenderQueue.h"

#include "OgreMaterial.h"
#include "OgrePass.h"
#include "OgreMaterialManager.h"
#include "OgreSceneManager.h"
#include "OgreMovableObject.h"
#include "OgreSceneManagerEnumerator.h"
#include "OgreHardwareBufferManager.h"
#include "OgreTechnique.h"
#include "OgreHlmsDatablock.h"
#include "OgreHlmsManager.h"
#include "OgreHlms.h"

#include "Vao/OgreVaoManager.h"
#include "Vao/OgreVertexArrayObject.h"
#include "Vao/OgreIndexBufferPacked.h"
#include "Vao/OgreIndirectBufferPacked.h"

#include "CommandBuffer/OgreCommandBuffer.h"
#include "CommandBuffer/OgreCbPipelineStateObject.h"
#include "CommandBuffer/OgreCbDrawCall.h"
#include "CommandBuffer/OgreCbShaderBuffer.h"

#include "OgreProfiler.h"


namespace Ogre
{
    AtomicScalar<uint32> v1::RenderOperation::MeshIndexId( 0 );

    const HlmsCache c_dummyCache( 0, HLMS_MAX, HlmsPso() );

    const int RqBits::SubRqIdBits           = 3;
    const int RqBits::TransparencyBits      = 1;
    const int RqBits::MacroblockBits        = 10;
    const int RqBits::ShaderBits            = 10;    //The higher 3 bits contain HlmsTypes
    const int RqBits::MeshBits              = 14;
    const int RqBits::TextureBits           = 11;
    const int RqBits::DepthBits             = 15;

    const int RqBits::SubRqIdShift          = 64                - SubRqIdBits;      //61
    const int RqBits::TransparencyShift     = SubRqIdShift      - TransparencyBits; //60
    const int RqBits::MacroblockShift       = TransparencyShift - MacroblockBits;   //50
    const int RqBits::ShaderShift           = MacroblockShift   - ShaderBits;       //40
    const int RqBits::MeshShift             = ShaderShift       - MeshBits;         //26
    const int RqBits::TextureShift          = MeshShift         - TextureBits;      //15
    const int RqBits::DepthShift            = TextureShift      - DepthBits;        //0

    const int RqBits::DepthShiftTransp      = TransparencyShift - DepthBits;        //45
    const int RqBits::MacroblockShiftTransp = DepthShiftTransp  - MacroblockBits;   //35
    const int RqBits::ShaderShiftTransp     = MacroblockShiftTransp - ShaderBits;   //25
    const int RqBits::MeshShiftTransp       = ShaderShiftTransp - MeshBits;         //11
    const int RqBits::TextureShiftTransp    = MeshShiftTransp   - TextureBits;      //0
    //---------------------------------------------------------------------
    RenderQueue::RenderQueue( HlmsManager *hlmsManager, SceneManager *sceneManager,
                              VaoManager *vaoManager ) :
        mHlmsManager( hlmsManager ),
        mSceneManager( sceneManager ),
        mVaoManager( vaoManager ),
        mLastWasCasterPass( false ),
        mLastVaoName( 0 ),
        mLastVertexData( 0 ),
        mLastIndexData( 0 ),
        mLastTextureHash( 0 ),
        mCommandBuffer( 0 )
    {
        mCommandBuffer = new CommandBuffer();

        for( size_t i=0; i<256; ++i )
            mRenderQueues[i].mQueuedRenderablesPerThread.resize( sceneManager->getNumWorkerThreads() );

        // Set some defaults.
        setRenderQueueMode( 0, FAST );
        setRenderQueueMode( 1, V1_FAST );
    }
    //---------------------------------------------------------------------
    RenderQueue::~RenderQueue()
    {
        delete mCommandBuffer;

        assert( mUsedIndirectBuffers.empty() );

        IndirectBufferPackedVec::const_iterator itor = mFreeIndirectBuffers.begin();
        IndirectBufferPackedVec::const_iterator end  = mFreeIndirectBuffers.end();

        while( itor != end )
        {
            if( (*itor)->getMappingState() != MS_UNMAPPED )
                (*itor)->unmap( UO_UNMAP_ALL );
            mVaoManager->destroyIndirectBuffer( *itor );
            ++itor;
        }
    }
    //-----------------------------------------------------------------------
    IndirectBufferPacked* RenderQueue::getIndirectBuffer( size_t numDraws )
    {
        size_t requiredBytes = numDraws * sizeof( CbDrawIndexed );

        IndirectBufferPackedVec::iterator itor = mFreeIndirectBuffers.begin();
        IndirectBufferPackedVec::iterator end  = mFreeIndirectBuffers.end();

        size_t smallestBufferSize                           = std::numeric_limits<size_t>::max();
        IndirectBufferPackedVec::iterator smallestBuffer    = end;

        //Find the smallest buffer in the pool that can fit the request.
        while( itor != end )
        {
            size_t bufferSize = (*itor)->getTotalSizeBytes();
            if( requiredBytes <= bufferSize && smallestBufferSize > bufferSize )
            {
                smallestBuffer      = itor;
                smallestBufferSize  = bufferSize;
            }

            ++itor;
        }

        if( smallestBuffer == end )
        {
            //None found? Create a new one.
            mFreeIndirectBuffers.push_back( mVaoManager->createIndirectBuffer( requiredBytes,
                                                                               BT_DYNAMIC_PERSISTENT,
                                                                               0, false ) );
            smallestBuffer = mFreeIndirectBuffers.end() - 1;
        }

        IndirectBufferPacked *retVal = *smallestBuffer;

        mUsedIndirectBuffers.push_back( *smallestBuffer );
        efficientVectorRemove( mFreeIndirectBuffers, smallestBuffer );

        return retVal;
    }
    //-----------------------------------------------------------------------
    void RenderQueue::clear(void)
    {
        for( size_t i=0; i<256; ++i )
        {
            QueuedRenderableArrayPerThread::iterator itor =
                    mRenderQueues[i].mQueuedRenderablesPerThread.begin();
            QueuedRenderableArrayPerThread::iterator end  =
                    mRenderQueues[i].mQueuedRenderablesPerThread.end();

            while( itor != end )
            {
                itor->q.clear();
                ++itor;
            }

            mRenderQueues[i].mQueuedRenderables.clear();
            mRenderQueues[i].mSorted = false;
        }
    }
    //-----------------------------------------------------------------------
    void RenderQueue::clearState(void)
    {
        mLastWasCasterPass = false;
        mLastVaoName    = 0;
        mLastVertexData = 0;
        mLastIndexData  = 0;
        mLastTextureHash= 0;
    }
    //-----------------------------------------------------------------------
    void RenderQueue::addRenderableV1( uint8 renderQueueId, bool casterPass, Renderable* pRend,
                                       const MovableObject *pMovableObject )
    {
        addRenderable( 0, renderQueueId, casterPass, pRend, pMovableObject, true );
    }
    //-----------------------------------------------------------------------
    void RenderQueue::addRenderableV2( size_t threadIdx, uint8 renderQueueId, bool casterPass,
                                       Renderable* pRend, const MovableObject *pMovableObject )
    {
        addRenderable( threadIdx, renderQueueId, casterPass, pRend, pMovableObject, false );
    }
    //-----------------------------------------------------------------------
    void RenderQueue::addRenderable( size_t threadIdx, uint8 rqId, bool casterPass,
                                     Renderable* pRend, const MovableObject *pMovableObject,
                                     bool isV1 )
    {
        assert( rqId == pMovableObject->getRenderQueueGroup() );

        uint8 subId = pRend->getRenderQueueSubGroup();
        RealAsUint depth = pMovableObject->getCachedDistanceToCamera();

        assert( !mRenderQueues[rqId].mSorted &&
                "Called addRenderable after render and before clear" );
        assert( subId < OGRE_RQ_MAKE_MASK( RqBits::SubRqIdBits ) );

        uint32 hlmsHash = casterPass ? pRend->getHlmsCasterHash() : pRend->getHlmsHash();
        const HlmsDatablock *datablock = pRend->getDatablock();

        bool transparent = datablock->mBlendblock[casterPass]->mIsTransparent;

        uint16 macroblock = datablock->mMacroblockHash[casterPass];
        uint16 texturehash= datablock->mTextureHash;

        //Flip the float to deal with negative & positive numbers
#if OGRE_DOUBLE_PRECISION == 0
        RealAsUint mask = -int(depth >> 31) | 0x80000000;
        depth = (depth ^ mask);
#else
        RealAsUint mask = -int64(depth >> 63) | 0x8000000000000000;
        depth = (depth ^ mask) >> 32;
#endif
        uint32 quantizedDepth = static_cast<uint32>( depth ) >> (32 - RqBits::DepthBits);

        uint32 meshHash;

        if( isV1 )
        {
            v1::RenderOperation op;
            //v2 objects cache the alpha testing information. v1 is evaluated in realtime.
            pRend->getRenderOperation( op,
                                       casterPass & (datablock->getAlphaTest() == CMPF_ALWAYS_PASS) );
            meshHash = op.meshIndex;
        }
        else
        {
            uint8 meshLod = pMovableObject->getCurrentMeshLod();
            const VertexArrayObjectArray &vaos = pRend->getVaos( static_cast<VertexPass>(casterPass) );

            assert( meshLod < vaos.size() && "Vaos meshLod/shadowLod not set. "
                    "Note: If this is a v1 object, it is in the wrong RenderQueue ID "
                    "(or the queue incorrectly set)." );

            VertexArrayObject *vao = vaos[meshLod];
            meshHash = vao->getRenderQueueId();
        }
        //TODO: Account for skeletal animation in any of the hashes (preferently on the material side)
        //TODO: Account for auto instancing animation in any of the hashes

        #define OGRE_RQ_HASH( x, bits, shift ) ( uint64( (x) & OGRE_RQ_MAKE_MASK( (bits) ) ) << (shift) )

        uint64 hash;
        if( !transparent )
        {
            //Opaque objects are first sorted by material, then by mesh, then by depth front to back.
            hash =
            OGRE_RQ_HASH( subId,            RqBits::SubRqIdBits,        RqBits::SubRqIdShift )      |
            OGRE_RQ_HASH( transparent,      RqBits::TransparencyBits,   RqBits::TransparencyShift ) |
            OGRE_RQ_HASH( macroblock,       RqBits::MacroblockBits,     RqBits::MacroblockShift )   |
            OGRE_RQ_HASH( hlmsHash,         RqBits::ShaderBits,         RqBits::ShaderShift )       |
            OGRE_RQ_HASH( meshHash,         RqBits::MeshBits,           RqBits::MeshShift )         |
            OGRE_RQ_HASH( texturehash,      RqBits::TextureBits,        RqBits::TextureShift )      |
            OGRE_RQ_HASH( quantizedDepth,   RqBits::DepthBits,          RqBits::DepthShift );
        }
        else
        {
            //Transparent objects are sorted by depth back to front, then by material, then by mesh.
            quantizedDepth = quantizedDepth ^ 0xffffffff;
            hash =
            OGRE_RQ_HASH( subId,            RqBits::SubRqIdBits,        RqBits::SubRqIdShift )          |
            OGRE_RQ_HASH( transparent,      RqBits::TransparencyBits,   RqBits::TransparencyShift )     |
            OGRE_RQ_HASH( quantizedDepth,   RqBits::DepthBits,          RqBits::DepthShiftTransp )      |
            OGRE_RQ_HASH( macroblock,       RqBits::MacroblockBits,     RqBits::MacroblockShiftTransp ) |
            OGRE_RQ_HASH( hlmsHash,         RqBits::ShaderBits,         RqBits::ShaderShiftTransp )     |
            OGRE_RQ_HASH( meshHash,         RqBits::MeshBits,           RqBits::MeshShiftTransp );
        }

        #undef OGRE_RQ_HASH

        mRenderQueues[rqId].mQueuedRenderablesPerThread[threadIdx].q.push_back(
                    QueuedRenderable( hash, pRend, pMovableObject ) );
    }
    //-----------------------------------------------------------------------
    void RenderQueue::render( RenderSystem *rs, uint8 firstRq, uint8 lastRq,
                              bool casterPass, bool dualParaboloid )
    {
        if( mLastWasCasterPass != casterPass )
        {
            clearState();
            mLastWasCasterPass = casterPass;
        }

        OgreProfileBeginGroup( "Command Preparation", OGREPROF_RENDERING );

        rs->setCurrentPassIterationCount( 1 );

        size_t numNeededDraws = 0;
        for( size_t i=firstRq; i<lastRq; ++i )
        {
            if( mRenderQueues[i].mMode == FAST )
            {
                QueuedRenderableArrayPerThread::const_iterator itor =
                        mRenderQueues[i].mQueuedRenderablesPerThread.begin();
                QueuedRenderableArrayPerThread::const_iterator end  =
                        mRenderQueues[i].mQueuedRenderablesPerThread.end();

                while( itor != end )
                {
                    numNeededDraws += itor->q.size();
                    ++itor;
                }
            }
        }

        HlmsCache passCache[HLMS_MAX];

        for( size_t i=0; i<HLMS_MAX; ++i )
        {
            Hlms *hlms = mHlmsManager->getHlms( static_cast<HlmsTypes>( i ) );
            if( hlms )
            {
                passCache[i] = hlms->preparePassHash( mSceneManager->getCurrentShadowNode(), casterPass,
                                                      dualParaboloid, mSceneManager );
            }
        }

        mCommandBuffer->setCurrentRenderSystem( rs );

        bool supportsIndirectBuffers = mVaoManager->supportsIndirectBuffers();

        IndirectBufferPacked *indirectBuffer = 0;
        unsigned char *indirectDraw = 0;
        unsigned char *startIndirectDraw = 0;

        if( numNeededDraws > 0 )
        {
            indirectBuffer = getIndirectBuffer( numNeededDraws );

            if( supportsIndirectBuffers )
            {
                indirectDraw = static_cast<unsigned char*>(
                            indirectBuffer->map( 0, indirectBuffer->getNumElements() ) );
            }
            else
            {
                indirectDraw = indirectBuffer->getSwBufferPtr();
            }

            startIndirectDraw = indirectDraw;
        }

        for( size_t i=firstRq; i<lastRq; ++i )
        {
            QueuedRenderableArray &queuedRenderables = mRenderQueues[i].mQueuedRenderables;
            QueuedRenderableArrayPerThread &perThreadQueue = mRenderQueues[i].mQueuedRenderablesPerThread;

            if( !mRenderQueues[i].mSorted )
            {
                OgreProfileGroupAggregate( "Sorting", OGREPROF_RENDERING );

                size_t numRenderables = 0;
                QueuedRenderableArrayPerThread::const_iterator itor = perThreadQueue.begin();
                QueuedRenderableArrayPerThread::const_iterator end  = perThreadQueue.end();

                while( itor != end )
                {
                    numRenderables += itor->q.size();
                    ++itor;
                }

                queuedRenderables.reserve( numRenderables );

                itor = perThreadQueue.begin();
                while( itor != end )
                {
                    queuedRenderables.appendPOD( itor->q.begin(), itor->q.end() );
                    ++itor;
                }

                //TODO: Exploit temporal coherence across frames then use insertion sorts.
                //As explained by L. Spiro in
                //http://www.gamedev.net/topic/661114-temporal-coherence-and-render-queue-sorting/?view=findpost&p=5181408
                //Keep a list of sorted indices from the previous frame (one per camera).
                //If we have the sorted list "5, 1, 4, 3, 2, 0":
                //  * If it grew from last frame, append: 5, 1, 4, 3, 2, 0, 6, 7 and use insertion sort.
                //  * If it's the same, leave it as is, and use insertion sort just in case.
                //  * If it's shorter, reset the indices 0, 1, 2, 3, 4; probably use quicksort or other generic sort
                //
                //TODO2: Explore sorting first on multiple threads, then merge sort into one.
                if( mRenderQueues[i].mSortMode == NormalSort )
                {
                    std::sort( queuedRenderables.begin(), queuedRenderables.end() );
                    mRenderQueues[i].mSorted = true;
                }
                else if( mRenderQueues[i].mSortMode == StableSort )
                {
                    std::stable_sort( queuedRenderables.begin(), queuedRenderables.end() );
                    mRenderQueues[i].mSorted = true;
                }
            }

            if( mRenderQueues[i].mMode == V1_LEGACY )
            {
                if( mLastVaoName )
                {
                    rs->_startLegacyV1Rendering();
                    mLastVaoName = 0;
                }
                renderES2( rs, casterPass, dualParaboloid, passCache, mRenderQueues[i] );
            }
            else if( mRenderQueues[i].mMode == V1_FAST )
            {
                if( mLastVaoName )
                {
                    *mCommandBuffer->addCommand<v1::CbStartV1LegacyRendering>() =
                                                                    v1::CbStartV1LegacyRendering();
                    mLastVaoName = 0;
                }
                renderGL3V1( casterPass, dualParaboloid, passCache, mRenderQueues[i] );
            }
            else if( numNeededDraws > 0 /*&& mRenderQueues[i].mMode == FAST*/ )
            {
                indirectDraw = renderGL3( casterPass, dualParaboloid, passCache, mRenderQueues[i],
                                          indirectBuffer, indirectDraw, startIndirectDraw );
            }
        }

        if( supportsIndirectBuffers && indirectBuffer )
            indirectBuffer->unmap( UO_KEEP_PERSISTENT );

        OgreProfileEndGroup( "Command Preparation", OGREPROF_RENDERING );

        OgreProfileBeginGroup( "Command Execution", OGREPROF_RENDERING );
        OgreProfileGpuBegin( "Command Execution" );

        for( size_t i=0; i<HLMS_MAX; ++i )
        {
            Hlms *hlms = mHlmsManager->getHlms( static_cast<HlmsTypes>( i ) );
            if( hlms )
                hlms->preCommandBufferExecution( mCommandBuffer );
        }

        mCommandBuffer->execute();

        for( size_t i=0; i<HLMS_MAX; ++i )
        {
            Hlms *hlms = mHlmsManager->getHlms( static_cast<HlmsTypes>( i ) );
            if( hlms )
                hlms->postCommandBufferExecution( mCommandBuffer );
        }

        OgreProfileGpuEnd( "Command Execution" );
        OgreProfileEndGroup( "Command Execution", OGREPROF_RENDERING );
    }
    //-----------------------------------------------------------------------
    void RenderQueue::renderES2( RenderSystem *rs, bool casterPass, bool dualParaboloid,
                                 HlmsCache passCache[HLMS_MAX],
                                 const RenderQueueGroup &renderQueueGroup )
    {
        v1::VertexData const *lastVertexData = mLastVertexData;
        v1::IndexData const *lastIndexData = mLastIndexData;
        HlmsCache const *lastHlmsCache = &c_dummyCache;
        uint32 lastHlmsCacheHash = 0;
        uint32 lastTextureHash = mLastTextureHash;
        //uint32 lastVertexDataId = ~0;

        const QueuedRenderableArray &queuedRenderables = renderQueueGroup.mQueuedRenderables;

        QueuedRenderableArray::const_iterator itor = queuedRenderables.begin();
        QueuedRenderableArray::const_iterator end  = queuedRenderables.end();

        while( itor != end )
        {
            const QueuedRenderable &queuedRenderable = *itor;
            /*uint32 hlmsHash = casterPass ? queuedRenderable.renderable->getHlmsCasterHash() :
                                           queuedRenderable.renderable->getHlmsHash();*/
            const HlmsDatablock *datablock = queuedRenderable.renderable->getDatablock();
            v1::RenderOperation op;
            queuedRenderable.renderable->getRenderOperation(
                        op, casterPass & (datablock->getAlphaTest() == CMPF_ALWAYS_PASS) );

            if( lastVertexData != op.vertexData )
            {
                lastVertexData = op.vertexData;
            }
            if( lastIndexData != op.indexData )
            {
                lastIndexData = op.indexData;
            }

            Hlms *hlms = mHlmsManager->getHlms( static_cast<HlmsTypes>( datablock->mType ) );

            lastHlmsCacheHash = lastHlmsCache->hash;
            const HlmsCache *hlmsCache = hlms->getMaterial( lastHlmsCache,
                                                            passCache[datablock->mType],
                                                            queuedRenderable, casterPass );
            if( lastHlmsCacheHash != hlmsCache->hash )
            {
                rs->_setPipelineStateObject( &hlmsCache->pso );
                lastHlmsCache = hlmsCache;
            }

            lastTextureHash = hlms->fillBuffersFor( hlmsCache, queuedRenderable, casterPass,
                                                    lastHlmsCacheHash, lastTextureHash );

            const v1::CbRenderOp cmd( op );
            rs->_setRenderOperation( &cmd );

            rs->_render( op );

            ++itor;
        }

        mLastVertexData     = lastVertexData;
        mLastIndexData      = lastIndexData;
        mLastTextureHash    = lastTextureHash;
    }
    //-----------------------------------------------------------------------
    unsigned char* RenderQueue::renderGL3( bool casterPass, bool dualParaboloid, HlmsCache passCache[],
                                           const RenderQueueGroup &renderQueueGroup,
                                           IndirectBufferPacked *indirectBuffer,
                                           unsigned char *indirectDraw,
                                           unsigned char *startIndirectDraw )
    {
        VertexArrayObject *lastVao = 0;
        uint32 lastVaoName = mLastVaoName;
        HlmsCache const *lastHlmsCache = &c_dummyCache;
        uint32 lastHlmsCacheHash = 0;

        int baseInstanceAndIndirectBuffers = 0;
        if( mVaoManager->supportsIndirectBuffers() )
            baseInstanceAndIndirectBuffers = 2;
        else if( mVaoManager->supportsBaseInstance() )
            baseInstanceAndIndirectBuffers = 1;

        uint32 instanceCount = 1;

        CbDrawCall *drawCmd = 0;
        CbSharedDraw    *drawCountPtr = 0;

        const QueuedRenderableArray &queuedRenderables = renderQueueGroup.mQueuedRenderables;

        QueuedRenderableArray::const_iterator itor = queuedRenderables.begin();
        QueuedRenderableArray::const_iterator end  = queuedRenderables.end();

        while( itor != end )
        {
            const QueuedRenderable &queuedRenderable = *itor;
            uint8 meshLod = queuedRenderable.movableObject->getCurrentMeshLod();
            const VertexArrayObjectArray &vaos = queuedRenderable.renderable->getVaos(
                        static_cast<VertexPass>(casterPass) );

            VertexArrayObject *vao = vaos[meshLod];
            const HlmsDatablock *datablock = queuedRenderable.renderable->getDatablock();

            Hlms *hlms = mHlmsManager->getHlms( static_cast<HlmsTypes>( datablock->mType ) );

            lastHlmsCacheHash = lastHlmsCache->hash;
            const HlmsCache *hlmsCache = hlms->getMaterial( lastHlmsCache,
                                                            passCache[datablock->mType],
                                                            queuedRenderable,
                                                            casterPass );
            if( lastHlmsCacheHash != hlmsCache->hash )
            {
                CbPipelineStateObject *psoCmd = mCommandBuffer->addCommand<CbPipelineStateObject>();
                *psoCmd = CbPipelineStateObject( &hlmsCache->pso );
                lastHlmsCache = hlmsCache;

                //Flush the Vao when changing shaders. Needed by D3D11/12 & possibly Vulkan
                lastVaoName = 0;
            }

            uint32 baseInstance = hlms->fillBuffersForV2( hlmsCache, queuedRenderable, casterPass,
                                                          lastHlmsCacheHash, mCommandBuffer );

            if( drawCmd != mCommandBuffer->getLastCommand() ||
                lastVaoName != vao->getVaoName() )
            {
                //Different mesh, vertex buffers or layout. Make a new draw call.
                //(or also the the Hlms made a batch-breaking command)

                if( lastVaoName != vao->getVaoName() )
                {
                    *mCommandBuffer->addCommand<CbVao>() = CbVao( vao );
                    *mCommandBuffer->addCommand<CbIndirectBuffer>() =
                                                            CbIndirectBuffer( indirectBuffer );
                    lastVaoName = vao->getVaoName();
                }

                void *offset = reinterpret_cast<void*>( indirectBuffer->_getFinalBufferStart() +
                                                        (indirectDraw - startIndirectDraw) );

                if( vao->getIndexBuffer() )
                {
                    CbDrawCallIndexed *drawCall = mCommandBuffer->addCommand<CbDrawCallIndexed>();
                    *drawCall = CbDrawCallIndexed( baseInstanceAndIndirectBuffers, vao, offset );
                    drawCmd = drawCall;
                }
                else
                {
                    CbDrawCallStrip *drawCall = mCommandBuffer->addCommand<CbDrawCallStrip>();
                    *drawCall = CbDrawCallStrip( baseInstanceAndIndirectBuffers, vao, offset );
                    drawCmd = drawCall;
                }

                lastVao = 0;
            }

            if( lastVao != vao )
            {
                //Different mesh, but same vertex buffers & layouts. Advance indirection buffer.
                ++drawCmd->numDraws;

                if( vao->mIndexBuffer )
                {
                    CbDrawIndexed *drawIndexedPtr = reinterpret_cast<CbDrawIndexed*>( indirectDraw );
                    indirectDraw += sizeof( CbDrawIndexed );

                    drawCountPtr = drawIndexedPtr;
                    drawIndexedPtr->primCount       = vao->mPrimCount;
                    drawIndexedPtr->instanceCount   = 1;
                    drawIndexedPtr->firstVertexIndex= vao->mIndexBuffer->_getFinalBufferStart() +
                                                                                    vao->mPrimStart;
                    drawIndexedPtr->baseVertex      = vao->mBaseVertexBuffer->_getFinalBufferStart();
                    drawIndexedPtr->baseInstance    = baseInstance;

                    instanceCount = 1;
                }
                else
                {
                    CbDrawStrip *drawStripPtr = reinterpret_cast<CbDrawStrip*>( indirectDraw );
                    indirectDraw += sizeof( CbDrawStrip );

                    drawCountPtr = drawStripPtr;
                    drawStripPtr->primCount         = vao->mPrimCount;
                    drawStripPtr->instanceCount     = 1;
                    drawStripPtr->firstVertexIndex  = vao->mBaseVertexBuffer->_getFinalBufferStart() +
                                                                                        vao->mPrimStart;
                    drawStripPtr->baseInstance      = baseInstance;

                    instanceCount = 1;
                }

                lastVao = vao;
            }
            else
            {
                //Same mesh. Just go with instancing. Keep the counter in
                //an external variable, as the region can be write-combined
                drawCountPtr->instanceCount = ++instanceCount;
            }

            ++itor;
        }

        mLastVaoName        = lastVaoName;
        mLastVertexData     = 0;
        mLastIndexData      = 0;
        mLastTextureHash    = 0;

        return indirectDraw;
    }
    //-----------------------------------------------------------------------
    void RenderQueue::renderGL3V1( bool casterPass, bool dualParaboloid,
                                   HlmsCache passCache[],
                                   const RenderQueueGroup &renderQueueGroup )
    {
        v1::RenderOperation lastRenderOp;
        HlmsCache const *lastHlmsCache = &c_dummyCache;
        uint32 lastHlmsCacheHash = 0;

        const bool supportsBaseInstance = mVaoManager->supportsBaseInstance();

        uint32 instanceCount = 1;

        v1::CbDrawCall *drawCmd = 0;

        const QueuedRenderableArray &queuedRenderables = renderQueueGroup.mQueuedRenderables;

        QueuedRenderableArray::const_iterator itor = queuedRenderables.begin();
        QueuedRenderableArray::const_iterator end  = queuedRenderables.end();

        while( itor != end )
        {
            const QueuedRenderable &queuedRenderable = *itor;

            const HlmsDatablock *datablock = queuedRenderable.renderable->getDatablock();

            v1::RenderOperation renderOp;
            queuedRenderable.renderable->getRenderOperation(
                        renderOp, casterPass & (datablock->getAlphaTest() == CMPF_ALWAYS_PASS) );

            Hlms *hlms = mHlmsManager->getHlms( static_cast<HlmsTypes>( datablock->mType ) );

            lastHlmsCacheHash = lastHlmsCache->hash;
            const HlmsCache *hlmsCache = hlms->getMaterial( lastHlmsCache,
                                                            passCache[datablock->mType],
                                                            queuedRenderable, casterPass );
            if( lastHlmsCache != hlmsCache )
            {
                CbPipelineStateObject *psoCmd = mCommandBuffer->addCommand<CbPipelineStateObject>();
                *psoCmd = CbPipelineStateObject( &hlmsCache->pso );
                lastHlmsCache = hlmsCache;

                //Flush the RenderOp when changing shaders. Needed by D3D11/12 & possibly Vulkan
                lastRenderOp.vertexData = 0;
                lastRenderOp.indexData  = 0;
            }

            uint32 baseInstance = hlms->fillBuffersForV1( hlmsCache, queuedRenderable, casterPass,
                                                          lastHlmsCacheHash, mCommandBuffer );

            bool differentRenderOp = lastRenderOp.vertexData != renderOp.vertexData ||
                    lastRenderOp.indexData != renderOp.indexData ||
                    lastRenderOp.operationType != renderOp.operationType ||
                    lastRenderOp.useGlobalInstancingVertexBufferIsAvailable !=
                        renderOp.useGlobalInstancingVertexBufferIsAvailable;

            if( drawCmd != mCommandBuffer->getLastCommand() || differentRenderOp ||
                renderOp.numberOfInstances != 1 )
            {
                //Different mesh, vertex buffers or layout. If instanced, entities
                //likely use their own low level materials. Make a new draw call.
                //(or also the the Hlms made a batch-breaking command)

                if( differentRenderOp )
                {
                    *mCommandBuffer->addCommand<v1::CbRenderOp>() = v1::CbRenderOp( renderOp );
                    lastRenderOp = renderOp;
                }

                if( renderOp.useIndexes )
                {
                    v1::CbDrawCallIndexed *drawCall =
                            mCommandBuffer->addCommand<v1::CbDrawCallIndexed>();
                    *drawCall = v1::CbDrawCallIndexed( supportsBaseInstance );

                    /*drawCall->useGlobalInstancingVertexBufferIsAvailable =
                            renderOp.useGlobalInstancingVertexBufferIsAvailable;*/
                    drawCall->primCount         = renderOp.indexData->indexCount;
                    drawCall->instanceCount     = renderOp.numberOfInstances;
                    drawCall->firstVertexIndex  = renderOp.indexData->indexStart;
                    drawCall->baseInstance      = baseInstance;

                    instanceCount = renderOp.numberOfInstances;

                    drawCmd = drawCall;
                }
                else
                {
                    v1::CbDrawCallStrip *drawCall =
                            mCommandBuffer->addCommand<v1::CbDrawCallStrip>();
                    *drawCall = v1::CbDrawCallStrip( supportsBaseInstance );

                    /*drawCall->useGlobalInstancingVertexBufferIsAvailable =
                            renderOp.useGlobalInstancingVertexBufferIsAvailable;*/
                    drawCall->primCount         = renderOp.vertexData->vertexCount;
                    drawCall->instanceCount     = renderOp.numberOfInstances;
                    drawCall->firstVertexIndex  = renderOp.vertexData->vertexStart;
                    drawCall->baseInstance      = baseInstance;

                    instanceCount = renderOp.numberOfInstances;

                    drawCmd = drawCall;
                }
            }
            else
            {
                //Same mesh. Just go with instancing. Keep the counter in
                //an external variable, as the region can be write-combined
                drawCmd->instanceCount = ++instanceCount;
            }

            ++itor;
        }

        mLastVaoName        = 0;
        mLastVertexData     = 0;
        mLastIndexData      = 0;
        mLastTextureHash    = 0;
    }
    //-----------------------------------------------------------------------
    void RenderQueue::renderSingleObject( Renderable* pRend, const MovableObject *pMovableObject,
                                          RenderSystem *rs, bool casterPass, bool dualParaboloid )
    {
        if( mLastVaoName )
        {
            rs->_startLegacyV1Rendering();
            mLastVaoName = 0;
        }

        if( mLastWasCasterPass != casterPass )
        {
            clearState();
            mLastWasCasterPass = casterPass;
        }

        const HlmsDatablock *datablock = pRend->getDatablock();

        Hlms *hlms = datablock->getCreator();
        HlmsCache passCache = hlms->preparePassHash( mSceneManager->getCurrentShadowNode(), casterPass,
                                                     dualParaboloid, mSceneManager );

        const QueuedRenderable queuedRenderable( 0, pRend, pMovableObject );
        v1::RenderOperation op;
        queuedRenderable.renderable->getRenderOperation(
                    op, casterPass & (datablock->getAlphaTest() == CMPF_ALWAYS_PASS) );
        /*uint32 hlmsHash = casterPass ? queuedRenderable.renderable->getHlmsCasterHash() :
                                       queuedRenderable.renderable->getHlmsHash();*/

        if( mLastVertexData != op.vertexData )
        {
            mLastVertexData = op.vertexData;
        }
        if( mLastIndexData != op.indexData )
        {
            mLastIndexData = op.indexData;
        }

        const HlmsCache *hlmsCache = hlms->getMaterial( &c_dummyCache, passCache,
                                                        queuedRenderable, casterPass );
        rs->_setPipelineStateObject( &hlmsCache->pso );

        mLastTextureHash = hlms->fillBuffersFor( hlmsCache, queuedRenderable, casterPass,
                                                 0, mLastTextureHash );

        const v1::CbRenderOp cmd( op );
        rs->_setRenderOperation( &cmd );

        rs->_render( op );

        mLastVaoName        = 0;
    }
    //-----------------------------------------------------------------------
    void RenderQueue::frameEnded(void)
    {
        mFreeIndirectBuffers.insert( mFreeIndirectBuffers.end(),
                                     mUsedIndirectBuffers.begin(),
                                     mUsedIndirectBuffers.end() );
        mUsedIndirectBuffers.clear();        
    }
    //-----------------------------------------------------------------------
    void RenderQueue::setRenderQueueMode( uint8 rqId, Modes newMode )
    {
        mRenderQueues[rqId].mMode = newMode;
    }
    //-----------------------------------------------------------------------
    RenderQueue::Modes RenderQueue::getRenderQueueMode( uint8 rqId ) const
    {
        return mRenderQueues[rqId].mMode;
    }
    //-----------------------------------------------------------------------
    void RenderQueue::setSortRenderQueue( uint8 rqId, RqSortMode sortMode )
    {
        mRenderQueues[rqId].mSortMode = sortMode;
    }
    //-----------------------------------------------------------------------
    RenderQueue::RqSortMode RenderQueue::getSortRenderQueue( uint8 rqId ) const
    {
        return mRenderQueues[rqId].mSortMode;
    }
}

