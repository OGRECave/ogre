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
#ifndef _OgreRenderQueue_H_
#define _OgreRenderQueue_H_

#include "OgrePrerequisites.h"
#include "OgreSharedPtr.h"
#include "OgreHeaderPrefix.h"
#include "OgreIteratorWrappers.h"

namespace Ogre {

    class Camera;
    class MovableObject;

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup RenderSystem
    *  @{
    */

    struct QueuedRenderable
    {
        uint64              hash;
        Renderable          *renderable;
        MovableObject const *movableObject;

        QueuedRenderable() : hash( 0 ), renderable( 0 ), movableObject( 0 ) {}
        QueuedRenderable( uint64 _hash, Renderable *_renderable,
                          const MovableObject *_movableObject ) :
            hash( _hash ), renderable( _renderable ), movableObject( _movableObject ) {}

        bool operator < ( const QueuedRenderable &_r ) const
        {
            return this->hash < _r.hash;
        }
    };

    /** Enumeration of queue groups, by which the application may group queued renderables
        so that they are rendered together with events in between
    @remarks
        When passed into methods these are actually passed as a uint8 to allow you
        to use values in between if you want to.
    */

    /** Class to manage the scene object rendering queue.
        @remarks
            Objects are grouped by material to minimise rendering state changes. The map from
            material to renderable object is wrapped in a class for ease of use.
        @par
            This class now includes the concept of 'queue groups' which allows the application
            adding the renderable to specifically schedule it so that it is included in 
            a discrete group. Good for separating renderables into the main scene,
            backgrounds and overlays, and also could be used in the future for more
            complex multipass routines like stenciling.
    */
    class _OgreExport RenderQueue : public RenderQueueAlloc
    {
        typedef FastArray<QueuedRenderable> QueuedRenderableArray;

        struct RenderQueueGroup
        {
            QueuedRenderableArray   mQueuedRenderables;
            bool                    mSorted;

            RenderQueueGroup() : mSorted( false ) {}
        };

        typedef vector<IndirectBufferPacked*>::type IndirectBufferPackedVec;

        RenderQueueGroup mRenderQueues[256];

        HlmsManager *mHlmsManager;
        SceneManager *mSceneManager;

        bool                    mLastWasCasterPass;
        HlmsMacroblock const    *mLastMacroblock;
        HlmsBlendblock const    *mLastBlendblock;
        uint32                  mLastVaoId;
        v1::VertexData const    *mLastVertexData;
        v1::IndexData const     *mLastIndexData;
        HlmsCache const         *mLastHlmsCache;
        uint32                  mLastTextureHash;

        CommandBuffer           *mCommandBuffer;
        IndirectBufferPackedVec mFreeIndirectBuffers;
        IndirectBufferPackedVec mUsedIndirectBuffers;

        /** Returns a new (or an existing) indirect buffer that can hold the requested number of draws.
        @param numDraws
            Number of draws the indirect buffer is expected to hold. It must be an upper limit.
            The caller may end up using less draws if he desires.
        @return
            Pointer to usable indirect buffer
        */
        IndirectBufferPacked* getIndirectBuffer( size_t numDraws );

    public:
        RenderQueue( HlmsManager *hlmsManager, SceneManager *sceneManager );
        ~RenderQueue();

        /// Empty the queue - should only be called by SceneManagers.
        void clear(void);

        /** The RenderQueue keeps track of API state to avoid redundant state change passes
            Calling this function forces the RenderQueue to re-set the Macro- & Blendblocks,
            shaders, and any other API dependendant calls on the next render.
        @remarks
            Calling this function inside render or renderES2 won't have any effect.
        */
        void clearState(void);

        /** Add a renderable object to the queue.
        @remarks
            This methods adds a Renderable to the queue, which will be rendered later by 
            the SceneManager. This is the advanced version of the call which allows the renderable
            to be added to any queue.
        @note
            Called by implementation of MovableObject::_updateRenderQueue.
        @param
            pRend Pointer to the Renderable to be added to the queue
        @param groupID
            The group the renderable is to be added to. This
            can be used to schedule renderable objects in separate groups such that the SceneManager
            respects the divisions between the groupings and does not reorder them outside these
            boundaries. This can be handy for overlays where no matter what you want the overlay to 
            be rendered last.
        */
        void addRenderable( Renderable* pRend, const MovableObject *pMovableObject,
                            bool casterPass );

        void renderES2( RenderSystem *rs, uint8 firstRq, uint8 lastRq,
                        bool casterPass, bool dualParaboloid );

        /// Renders in a compatible way with GL 3.3 and D3D11. Can only render V2 objects
        /// (i.e. Items, VertexArrayObject)
        void renderGL3( RenderSystem *rs, uint8 firstRq, uint8 lastRq,
                        bool casterPass, bool dualParaboloid );

        /// Don't call this too often.
        void renderSingleObject( Renderable* pRend, const MovableObject *pMovableObject,
                                 RenderSystem *rs, bool casterPass, bool dualParaboloid );
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
