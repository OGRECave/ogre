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
#include "OgreRenderQueueSortingGrouping.h"
#include "OgrePass.h"
#include "OgreMaterialManager.h"
#include "OgreSceneManager.h"
#include "OgreMovableObject.h"
#include "OgreSceneManagerEnumerator.h"
#include "OgreTechnique.h"


namespace Ogre
{
    const int SubRqIdBits           = 6;
    const int TransparencyBits      = 1;
    const int MaterialBits          = 30;
    const int MeshBits              = 10;
    const int DepthBits             = 17;

    #define OGRE_MAKE_MASK( x ) ( (1 << x) - 1 )

    const int SubRqIdShift          = 64                - SubRqIdBits;      //58
    const int TransparencyShift     = SubRqIdShift      - TransparencyBits; //57
    const int MaterialShift         = TransparencyShift - MaterialBits;     //27
    const int MeshShift             = MaterialShift     - MeshBits;         //17
    const int DepthShift            = MeshShift         - DepthBits;        //0

    const int DepthShiftTransp      = TransparencyShift - DepthBits;        //40
    const int MaterialShiftTransp   = DepthShiftTransp  - MaterialBits;     //10
    const int MeshShiftTransp       = MaterialShiftTransp- MeshBits;         //0
    //---------------------------------------------------------------------
    RenderQueue::RenderQueue()
    {
        mRenderQueues.resize( 255 );
    }
    //---------------------------------------------------------------------
    RenderQueue::~RenderQueue()
    {
    }
    //-----------------------------------------------------------------------
    void RenderQueue::clear(void)
    {
        RenderQueueIdVec::iterator itor = mRenderQueues.begin();
        RenderQueueIdVec::iterator end  = mRenderQueues.end();

        while( itor != end )
        {
            itor->mQueuedRenderables.clear();
            itor->mSorted = false;
            ++itor;
        }

    }
    //-----------------------------------------------------------------------
    void RenderQueue::addRenderable(Renderable* pRend, uint8 rqId, uint8 subId, RealAsUint depth )
    {
        assert( !mRenderQueues[rqId].mSorted &&
                "Called addRenderable after render and before clear" );
        assert( subId < OGRE_MAKE_MASK( SubRqIdBits ) );

        const MaterialPtr& material = pRend->getMaterial();

        Technique *technique = material->getBestTechnique( 0, pRend );
        uint32 techniqueHash; //TODO

        Pass *pass = technique->getPass( 0 );
        bool opaque = !pass->isTransparent();

        //Flip the float to deal with negative & positive numbers
#if OGRE_DOUBLE_PRECISION == 1
        RealAsUint mask = -int(depth >> 31) | 0x80000000;
        depth = (depth ^ mask);
#else
        RealAsUint mask = -int64(depth >> 63) | 0x8000000000000000;
        depth = (depth ^ mask) >> 32;
#endif
        uint32 quantizedDepth = static_cast<uint32>( depth );

        RenderOperation op;
        pRend->getRenderOperation( op ); //TODO
        //op.vertexData->
        uint32 meshHash; //TODO
        //TODO: Account for skeletal animation in any of the hashes (preferently on the material side)

        uint64 hash;
        if( opaque )
        {
            //Opaque objects are first sorted by material, then by mesh, then by depth front to back.
            hash =
                ( (subId            & OGRE_MAKE_MASK( SubRqIdBits ))        << SubRqIdShift )       |
                ( (opaque           & OGRE_MAKE_MASK( TransparencyBits ))   << TransparencyShift )  |
                ( (techniqueHash    & OGRE_MAKE_MASK( MaterialBits ))       << MaterialShift )      |
                ( (meshHash         & OGRE_MAKE_MASK( MeshBits ))           << MeshShift )          |
                ( (quantizedDepth   & OGRE_MAKE_MASK( DepthBits ))          << DepthShift );
        }
        else
        {
            //Transparent objects are sorted by depth back to front, then by material, then by mesh.
            quantizedDepth = quantizedDepth ^ 0xffffffff;
            hash =
                ( (subId            & OGRE_MAKE_MASK( SubRqIdBits ))        << SubRqIdShift )       |
                ( (transparent      & OGRE_MAKE_MASK( TransparencyBits ))   << TransparencyShift )  |
                ( (quantizedDepth   & OGRE_MAKE_MASK( DepthBits ))          << DepthShiftTransp )   |
                ( (techniqueHash    & OGRE_MAKE_MASK( MaterialBits ))       << MaterialShiftTransp )|
                ( (meshHash         & OGRE_MAKE_MASK( MeshBits ))           << MeshShiftTransp );
        }

        mRenderQueues[rqId].push_back( QueuedRenderable( hash, pRend ) );
    }
    //-----------------------------------------------------------------------
    void RenderQueue::render( uint8 firstRq, uint8 lastRq )
    {
        //mBatchesToRender.push_back( 0 ); TODO First batch is always dummy
        float *texBufferPtr; //TODO

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
                RenderOperation op;
                queuedRenderable.renderable->getRenderOperation( op );

                //TODO
                uint32 renderOpHash;
                size_t elementsPerInstance;

                //const Hlms *hlms = queuedRenderable.renderable->getHlms();
                //size_t hlmsElements = hlms->getNumElements( queuedRenderable.renderable );
                //size_t elementsPerInstance = numWorldTransforms + hlmsElements;

                Batch *batch = &mBatchesToRender.back();
                if( batch->renderOpHash != renderOpHash ||
                    batch->start + elementsPerInstance < batch->bufferBucket->numElements )
                {
                    mBatchesToRender.push_back( Batch() ); //TODO
                    batch = &mBatchesToRender.back();
                }

                unsigned short numWorldTransforms = queuedRenderable.renderable->getNumWorldTransforms();
                if( numWorldTransforms <= 1 )
                {
                    queuedRenderable.renderable->writeSingleTransform3x4( texBufferPtr );
                    texBufferPtr += 12;
                }
                else
                {
                    queuedRenderable.renderable->writeAnimatedTransform3x4( texBufferPtr );
                    texBufferPtr += 12 * numWorldTransforms;
                }

                hlms->writeElements( queuedRenderable.renderable, texBufferPtr );
                texBufferPtr += hlmsElements;

                ++batch->count;

                //itor->
                ++itor;
            }
        }
    }
}

