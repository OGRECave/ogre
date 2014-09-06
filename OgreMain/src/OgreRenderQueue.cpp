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
#include "OgreTechnique.h"
#include "OgreHlmsDatablock.h"
#include "OgreHlmsManager.h"
#include "OgreHlms.h"

#include "Vao/OgreVertexArrayObject.h"


namespace Ogre
{
    AtomicScalar<uint32> v1::RenderOperation::MeshIndexId( 0 );

    const HlmsCache c_dummyCache( 0, HLMS_MAX );

    const int SubRqIdBits           = 3;
    const int TransparencyBits      = 1;
    const int MacroblockBits        = 10;
    const int ShaderBits            = 10;    //The higher 3 bits contain HlmsTypes
    const int MeshBits              = 14;
    const int TextureBits           = 11;
    const int DepthBits             = 15;

    #define OGRE_MAKE_MASK( x ) ( (1 << x) - 1 )

    const int SubRqIdShift          = 64                - SubRqIdBits;      //60
    const int TransparencyShift     = SubRqIdShift      - TransparencyBits; //59
    const int MacroblockShift       = TransparencyShift - MacroblockBits;   //49
    const int ShaderShift           = MacroblockShift   - ShaderBits;       //40
    const int MeshShift             = ShaderShift       - MeshBits;         //26
    const int TextureShift          = MeshShift         - TextureBits;      //15
    const int DepthShift            = TextureShift      - DepthBits;        //0

    const int DepthShiftTransp      = TransparencyShift - DepthBits;        //44
    const int MacroblockShiftTransp = DepthShiftTransp  - MacroblockBits;   //34
    const int ShaderShiftTransp     = MacroblockShiftTransp - ShaderBits;   //25
    const int MeshShiftTransp       = ShaderShiftTransp - MeshBits;         //11
    const int TextureShiftTransp    = MeshShiftTransp   - TextureBits;      //0
    //---------------------------------------------------------------------
    RenderQueue::RenderQueue( HlmsManager *hlmsManager, SceneManager *sceneManager ) :
        mHlmsManager( hlmsManager ),
        mSceneManager( sceneManager ),
        mLastWasCasterPass( false ),
        mLastMacroblock( 0 ),
        mLastBlendblock( 0 ),
        mLastVaoId( 0 ),
        mLastVertexData( 0 ),
        mLastIndexData( 0 ),
        mLastHlmsCache( &c_dummyCache ),
        mLastTextureHash( 0 )
    {
    }
    //---------------------------------------------------------------------
    RenderQueue::~RenderQueue()
    {
    }
    //-----------------------------------------------------------------------
    void RenderQueue::clear(void)
    {
        for( size_t i=0; i<256; ++i )
        {
            mRenderQueues[i].mQueuedRenderables.clear();
            mRenderQueues[i].mSorted = false;
        }
    }
    //-----------------------------------------------------------------------
    void RenderQueue::clearState(void)
    {
        mLastWasCasterPass = false;
        mLastMacroblock = 0;
        mLastBlendblock = 0;
        mLastVaoId      = 0;
        mLastVertexData = 0;
        mLastIndexData  = 0;
        mLastHlmsCache  = &c_dummyCache;
        mLastTextureHash= 0;
    }
    //-----------------------------------------------------------------------
    void RenderQueue::addRenderable( Renderable* pRend, const MovableObject *pMovableObject,
                                     bool casterPass )
    {
        uint8 rqId  = pMovableObject->getRenderQueueGroup();
        uint8 subId = pRend->getRenderQueueSubGroup();
        RealAsUint depth = pMovableObject->getCachedDistanceToCamera();

        assert( !mRenderQueues[rqId].mSorted &&
                "Called addRenderable after render and before clear" );
        assert( subId < OGRE_MAKE_MASK( SubRqIdBits ) );

        uint32 hlmsHash = casterPass ? pRend->getHlmsCasterHash() : pRend->getHlmsHash();
        const HlmsDatablock *datablock = pRend->getDatablock();

        bool transparent = datablock->mIsTransparent;

        uint16 macroblock = datablock->mMacroblockHash;
        uint16 texturehash= datablock->mTextureHash;

        //Flip the float to deal with negative & positive numbers
#if OGRE_DOUBLE_PRECISION == 0
        RealAsUint mask = -int(depth >> 31) | 0x80000000;
        depth = (depth ^ mask);
#else
        RealAsUint mask = -int64(depth >> 63) | 0x8000000000000000;
        depth = (depth ^ mask) >> 32;
#endif
        uint32 quantizedDepth = static_cast<uint32>( depth );

        v1::RenderOperation op;
        pRend->getRenderOperation( op ); //TODO
        uint32 meshHash = op.meshIndex;
        //TODO: Account for skeletal animation in any of the hashes (preferently on the material side)
        //TODO: Account for auto instancing animation in any of the hashes

        uint64 hash;
        if( transparent )
        {
            //Opaque objects are first sorted by material, then by mesh, then by depth front to back.
            hash =
                ( uint64(subId          & OGRE_MAKE_MASK( SubRqIdBits ))        << SubRqIdShift )       |
                ( uint64(transparent    & OGRE_MAKE_MASK( TransparencyBits ))   << TransparencyShift )  |
                ( uint64(macroblock     & OGRE_MAKE_MASK( MacroblockBits ))     << MacroblockShift )    |
                ( uint64(hlmsHash       & OGRE_MAKE_MASK( ShaderBits ))         << ShaderShift )        |
                ( uint64(meshHash       & OGRE_MAKE_MASK( MeshBits ))           << MeshShift )          |
                ( uint64(texturehash    & OGRE_MAKE_MASK( TextureBits ))        << TextureShift )       |
                ( uint64(quantizedDepth & OGRE_MAKE_MASK( DepthBits ))          << DepthShift );
        }
        else
        {
            //Transparent objects are sorted by depth back to front, then by material, then by mesh.
            quantizedDepth = quantizedDepth ^ 0xffffffff;
            hash =
                ( uint64(subId          & OGRE_MAKE_MASK( SubRqIdBits ))        << SubRqIdShift )       |
                ( uint64(transparent    & OGRE_MAKE_MASK( TransparencyBits ))   << TransparencyShift )  |
                ( uint64(quantizedDepth & OGRE_MAKE_MASK( DepthBits ))          << DepthShiftTransp )   |
                ( uint64(macroblock     & OGRE_MAKE_MASK( MacroblockBits ))    << MacroblockShiftTransp)|
                ( uint64(hlmsHash       & OGRE_MAKE_MASK( ShaderBits ))         << ShaderShiftTransp )  |
                ( uint64(meshHash       & OGRE_MAKE_MASK( MeshBits ))           << MeshShiftTransp );
        }

        mRenderQueues[rqId].mQueuedRenderables.push_back( QueuedRenderable( hash, pRend,
                                                                            pMovableObject ) );
    }
    //-----------------------------------------------------------------------
    void RenderQueue::renderES2( RenderSystem *rs, uint8 firstRq, uint8 lastRq,
                                 bool casterPass, bool dualParaboloid )
    {
        if( mLastWasCasterPass != casterPass )
        {
            clearState();
            mLastWasCasterPass = casterPass;
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

        HlmsMacroblock const *lastMacroblock = mLastMacroblock;
        HlmsBlendblock const *lastBlendblock = mLastBlendblock;
        v1::VertexData const *lastVertexData = mLastVertexData;
        v1::IndexData const *lastIndexData = mLastIndexData;
        HlmsCache const *lastHlmsCache = mLastHlmsCache;
        uint32 lastTextureHash = mLastTextureHash;
        //uint32 lastVertexDataId = ~0;

        for( size_t i=firstRq; i<lastRq; ++i )
        {
            QueuedRenderableArray &queuedRenderables = mRenderQueues[i].mQueuedRenderables;

            if( !mRenderQueues[i].mSorted )
            {
                std::sort( queuedRenderables.begin(), queuedRenderables.end() );
                mRenderQueues[i].mSorted = true;
            }

            QueuedRenderableArray::const_iterator itor = queuedRenderables.begin();
            QueuedRenderableArray::const_iterator end  = queuedRenderables.end();

            while( itor != end )
            {
                const QueuedRenderable &queuedRenderable = *itor;
                v1::RenderOperation op;
                queuedRenderable.renderable->getRenderOperation( op );
                /*uint32 hlmsHash = casterPass ? queuedRenderable.renderable->getHlmsCasterHash() :
                                               queuedRenderable.renderable->getHlmsHash();*/
                const HlmsDatablock *datablock = queuedRenderable.renderable->getDatablock();

                if( lastMacroblock != datablock->mMacroblock )
                {
                    rs->_setHlmsMacroblock( datablock->mMacroblock );
                    lastMacroblock = datablock->mMacroblock;
                }

                if( lastBlendblock != datablock->mBlendblock )
                {
                    rs->_setHlmsBlendblock( datablock->mBlendblock );
                    lastBlendblock = datablock->mBlendblock;
                }

                if( lastVertexData != op.vertexData )
                {
                    lastVertexData = op.vertexData;
                }
                if( lastIndexData != op.indexData )
                {
                    lastIndexData = op.indexData;
                }

                Hlms *hlms = mHlmsManager->getHlms( static_cast<HlmsTypes>( datablock->mType ) );

                const HlmsCache *hlmsCache = hlms->getMaterial( lastHlmsCache,
                                                                passCache[datablock->mType],
                                                                queuedRenderable,
                                                                casterPass );
                if( lastHlmsCache != hlmsCache )
                    rs->_setProgramsFromHlms( hlmsCache );

                lastTextureHash = hlms->fillBuffersFor( hlmsCache, queuedRenderable, casterPass,
                                                        lastHlmsCache, lastTextureHash );

                rs->_render( op );

                lastHlmsCache   = hlmsCache;

                ++itor;
            }
        }

        mLastMacroblock     = lastMacroblock;
        mLastBlendblock     = lastBlendblock;
        mLastVertexData     = lastVertexData;
        mLastIndexData      = lastIndexData;
        mLastHlmsCache      = lastHlmsCache;
        mLastTextureHash    = lastTextureHash;
    }
    //-----------------------------------------------------------------------
    void RenderQueue::renderGL3( RenderSystem *rs, uint8 firstRq, uint8 lastRq,
                                 bool casterPass, bool dualParaboloid )
    {
        if( mLastWasCasterPass != casterPass )
        {
            clearState();
            mLastWasCasterPass = casterPass;
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

        HlmsMacroblock const *lastMacroblock = mLastMacroblock;
        HlmsBlendblock const *lastBlendblock = mLastBlendblock;
        uint32 lastVaoId = mLastVaoId;
        HlmsCache const *lastHlmsCache = mLastHlmsCache;
        uint32 lastTextureHash = mLastTextureHash;

        for( size_t i=firstRq; i<lastRq; ++i )
        {
            QueuedRenderableArray &queuedRenderables = mRenderQueues[i].mQueuedRenderables;

            if( !mRenderQueues[i].mSorted )
            {
                std::sort( queuedRenderables.begin(), queuedRenderables.end() );
                mRenderQueues[i].mSorted = true;
            }

            QueuedRenderableArray::const_iterator itor = queuedRenderables.begin();
            QueuedRenderableArray::const_iterator end  = queuedRenderables.end();

            while( itor != end )
            {
                const QueuedRenderable &queuedRenderable = *itor;
                uint8 meshLod = 0; //TODO
                const VertexArrayObjectArray &vaos = queuedRenderable.renderable->getVaos();

                VertexArrayObject *vao = vaos[meshLod];
                const HlmsDatablock *datablock = queuedRenderable.renderable->getDatablock();

                if( lastMacroblock != datablock->mMacroblock )
                {
                    rs->_setHlmsMacroblock( datablock->mMacroblock );
                    lastMacroblock = datablock->mMacroblock;
                }

                if( lastBlendblock != datablock->mBlendblock )
                {
                    rs->_setHlmsBlendblock( datablock->mBlendblock );
                    lastBlendblock = datablock->mBlendblock;
                }

                if( lastVaoId != vao->getRenderQueueId() )
                {
                    rs->_setVertexArrayObject( vao );
                    lastVaoId = vao->getRenderQueueId();
                }

                Hlms *hlms = mHlmsManager->getHlms( static_cast<HlmsTypes>( datablock->mType ) );

                const HlmsCache *hlmsCache = hlms->getMaterial( lastHlmsCache,
                                                                passCache[datablock->mType],
                                                                queuedRenderable,
                                                                casterPass );
                if( lastHlmsCache != hlmsCache )
                    rs->_setProgramsFromHlms( hlmsCache );

                lastTextureHash = hlms->fillBuffersFor( hlmsCache, queuedRenderable, casterPass,
                                                        lastHlmsCache, lastTextureHash );

                rs->_render( vao );

                lastHlmsCache   = hlmsCache;

                ++itor;
            }
        }

        mLastMacroblock     = lastMacroblock;
        mLastBlendblock     = lastBlendblock;
        mLastVaoId          = lastVaoId;
        mLastVertexData     = 0;
        mLastIndexData      = 0;
        mLastHlmsCache      = lastHlmsCache;
        mLastTextureHash    = lastTextureHash;
    }
    //-----------------------------------------------------------------------
    void RenderQueue::renderSingleObject( Renderable* pRend, const MovableObject *pMovableObject,
                                          RenderSystem *rs, bool casterPass, bool dualParaboloid )
    {
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
        queuedRenderable.renderable->getRenderOperation( op );
        /*uint32 hlmsHash = casterPass ? queuedRenderable.renderable->getHlmsCasterHash() :
                                       queuedRenderable.renderable->getHlmsHash();*/

        if( mLastMacroblock != datablock->mMacroblock )
        {
            rs->_setHlmsMacroblock( datablock->mMacroblock );
            mLastMacroblock = datablock->mMacroblock;
        }

        if( mLastBlendblock != datablock->mBlendblock )
        {
            rs->_setHlmsBlendblock( datablock->mBlendblock );
            mLastBlendblock = datablock->mBlendblock;
        }

        if( mLastVertexData != op.vertexData )
        {
            mLastVertexData = op.vertexData;
        }
        if( mLastIndexData != op.indexData )
        {
            mLastIndexData = op.indexData;
        }

        const HlmsCache *hlmsCache = hlms->getMaterial( mLastHlmsCache, passCache,
                                                        queuedRenderable, casterPass );
        if( mLastHlmsCache != hlmsCache )
            rs->_setProgramsFromHlms( hlmsCache );

        mLastTextureHash = hlms->fillBuffersFor( hlmsCache, queuedRenderable, casterPass,
                                                 mLastHlmsCache, mLastTextureHash );

        rs->_render( op );

        mLastHlmsCache   = hlmsCache;
        mLastVaoId = 0;
    }
}

