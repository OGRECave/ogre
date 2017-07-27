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
#ifndef _OgrePlanarReflections_H_
#define _OgrePlanarReflections_H_

#include "OgrePlanarReflectionActor.h"
#include "OgreTexture.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Component
    *  @{
    */
    /** \addtogroup Material
    *  @{
    */

    struct ActiveActorData
    {
        Camera              *reflectionCamera;
        CompositorWorkspace *workspace;
        TexturePtr          reflectionTexture;
        bool                isReserved;
    };

    typedef FastArray<Renderable*> RenderableArray;
    /** Planar Reflections can be used with both Unlit and PBS, but they're setup
        differently. Unlit is very fast, but also very basic. It's mostly useful for
        perfect mirrors.

        An unlit datablock is tied to a particular Actor (reflection plane).
        PBS on the other hand, can dynamically assign Renderables to the closest actor that
        aligns with the Renderable's (predominant) normal, regardless of the pbs datablock
        it uses. If the actor is close enough but not an exact match, PBS will attempt to
        project the reflection in an attempt to correct this, however it's an approximation.

        If you want perfect reflections, the reflection plane of the actor must match and align
        exactly the surface being drawn; or rely on a fallback (Local Cubemaps, SSR, etc).

        Furthermore, PBS will automatically disable reflections while rendering reflections
        (but Unlit won't do this, so it's your job to leave it out of rendering i.e.
        via visibility masks).

        Actors are culled against the camera, thus if they're no longer visible Ogre will
        stop updating those actors, improving performance.
    */
    class _OgrePlanarReflectionsExport PlanarReflections
    {
    public:
        struct TrackedRenderable
        {
            Renderable      *renderable;
            MovableObject   *movableObject;
            Vector3         reflNormal;
            Vector3         renderableCenter;
            uint32          hlmsHashes[2];

            /**
            @param _renderable
                SubItem/SubEntity/etc that will have planar reflections
            @param _movableObject
                Parent of the Renderable (usually Item/Entity/etc, but in some cases the
                Renderable & MovableObject can be the same class)
            @param _reflNormal
                The predominant normal of the reflection of this Renderable, in local space.
                We'll use the MovableObject's node transform to convert it to object space.
                Actors that are close enough and have a direction that resembles enough
                this reflNormal will be considered.
            @param _renderableCenter
                The center of the Renderable, in local space. We'll use this center to determine
                how close this Renderable is to each Actor.
            */
            TrackedRenderable( Renderable *_renderable, MovableObject *_movableObject,
                               const Vector3 &_reflNormal, const Vector3 &_renderableCenter ) :
                renderable( _renderable ), movableObject( _movableObject ),
                reflNormal( _reflNormal ), renderableCenter( _renderableCenter )
            {
                memset( hlmsHashes, 0, sizeof( hlmsHashes ) );
            }
        };

    protected:
        typedef FastArray<TrackedRenderable> TrackedRenderableArray;

        typedef vector<ActiveActorData>::type ActiveActorDataVec;

        typedef vector<PlanarReflectionActor*>::type PlanarReflectionActorVec;

        PlanarReflectionActorVec    mActors;
        ArrayActorPlane             *mActorsSoA;
        size_t                      mCapacityActorsSoA;

        Real                        mLastAspectRatio;
        Vector3                     mLastCameraPos;
        Quaternion                  mLastCameraRot;
        Camera                      *mLastCamera;
        //Camera                      *mLockCamera;
        PlanarReflectionActorVec    mActiveActors;
        ActiveActorDataVec          mActiveActorData;
        TrackedRenderableArray      mTrackedRenderables;
        bool                        mUpdatingRenderablesHlms;
        bool                        mAnyPendingFlushRenderable;

        uint8               mMaxActiveActors;
        Real                mInvMaxDistance;
        Real                mMaxSqDistance;
        SceneManager        *mSceneManager;
        CompositorManager2  *mCompositorManager;

        PlanarReflectionActor   mDummyActor;

        void updateFlushedRenderables(void);

    public:
        /**
        @param sceneManager
        @param compositorManager
        @param maxActiveActors
        @param maxDistance
        @param lockCamera
        */
        PlanarReflections( SceneManager *sceneManager, CompositorManager2 *compositorManager,
                           Real maxDistance, Camera *lockCamera );
        ~PlanarReflections();

        void setMaxDistance( Real maxDistance );

        /** Setups how many actors can be active at the same time.
            You may have many actors (i.e. 1000 actors), but for performance and memory
            reasons you may only want 1-5 actors at the same time. Actors are dynamically
            activated based on distance to camera.
        @remarks
            Note HlmsPbs hardcodes the max number of actors, so changing
            this value after having rendered a few frames could cause many
            shaders to be recompiled.
        @param maxActiveActors
            New max active actors. Can be lower or higher than its previous value.
            When this value is less or equal than current value, all the next parameters
            are ignored.
            When this value is higher than its previous, value, the new active actors
            will use these parameters; which don't have to necessarily match previous
            calls.
        @param workspaceName
            Workspace to use for rendering.
        @param useAccurateLighting
            When true, overall scene CPU usage may be higher, but lighting information
            in the reflections will be accurate.
            Turning it to false is faster. This is a performance optimization that rarely
            has a noticeable impact on quality.
        @param width
            Width resolution of the RTT assigned to this reflection plane
        @param height
            Height resolution of the RTT assigned to this reflection plane
        @param withMipmaps
            When true, mipmaps will be created; which are useful for glossy reflections
            (i.e. roughness close to 1). Set this to false if you only plan on using
            it for a mirror (which are always a perfect reflection... unless you want
            to use a roughness map to add scratches & stains to it).
        @param pixelFormat
            Pixel Format of the RTT assigned to this reflection plane
        @param mipmapMethodCompute
            Set to true if the workspace assigned via PlanarReflectionActor::workspaceName
            will filter the RTT with a compute filter (usually for higher quality).
        */
        void setMaxActiveActors( uint8 maxActiveActors, IdString workspaceName, bool useAccurateLighting,
                                 uint32 width, uint32 height, bool withMipmaps, PixelFormat pixelFormat,
                                 bool mipmapMethodCompute );

        /** Adds an actor plane that other objects can use as source for reflections if they're
            close enough to it (and aligned enough to the normal).
        @param actor
            Actor to use, with some data prefilled (see
            PlanarReflectionActor::PlanarReflectionActor constructor)
        @return
            Pointer to the created actor for further manipulation.
        */
        PlanarReflectionActor* addActor( const PlanarReflectionActor &actor );
        void destroyActor( PlanarReflectionActor *actor );
        void destroyAllActors(void);

        /** Reserves a particular slot (i.e. texture) to be used only with a specifc actor.
            A slot can only be reserved by one actor at a time.
            If this actor doesn't activate (i.e. it's culled by the camera), the slot won't
            be used and thus the contents of the texture will remain unchanged.
        @remarks
            Reservation is useful to have a texture be linked always with the same actor,
            which can be used to avoid updating it while still showing a (partially wrong)
            reflection (as a performance improvement) or for using actors with HlmsUnlit,
            which can only bind reflection textures with materials; rather than binding
            actors with materials.
        @param activeActorSlot
            Slot to reserve. Must be in range [0; mMaxActiveActors)
        @param actor
            Actor to link this reservation with.
        */
        void reserve( uint8 activeActorSlot, PlanarReflectionActor *actor );
        /// Releases a reservation made with reserve().
        /// This is done automatically if you call destroyActor
        void releaseReservation( PlanarReflectionActor *actor );

        /** Once you add a Renderable, we will automatically update its PBS material to use
            reflection if it's close to any active actor. If no active actor is close, we'll
            disable the reflections for that frame.
        @remarks
            You must call removeRenderable before destroying the Renderable.
            You can use PlanarReflections::hasPlanarReflections( renderable ) to tell
            whether this Renderable has already been added.
        @param trackedRenderable
            See TrackedRenderable::TrackedRenderable.
        */
        void addRenderable( const TrackedRenderable &trackedRenderable );
        void removeRenderable( Renderable *renderable );
        void _notifyRenderableFlushedHlmsDatablock( Renderable *renderable );

        void beginFrame(void);
        void update( Camera *camera, Real aspectRatio );

        uint8 getMaxActiveActors(void) const            { return mMaxActiveActors; }

        /// Returns the amount of bytes that fillConstBufferData is going to fill.
        size_t getConstBufferSize(void) const;

        /** Fills 'passBufferPtr' with the necessary data for PlanarReflections.
            @see getConstBufferSize
        @remarks
            Assumes 'passBufferPtr' is aligned to a vec4/float4 boundary.
        */
        void fillConstBufferData( RenderTarget *renderTarget, Camera *camera,
                                  const Matrix4 &projectionMatrix,
                                  float * RESTRICT_ALIAS passBufferPtr ) const;
        TexturePtr getTexture( uint8 actorIdx ) const;

        /// Returns true if the Camera settings (position, orientation, aspect ratio, etc)
        /// match with the reflection we have in cache.
        bool cameraMatches( const Camera *camera );

        bool _isUpdatingRenderablesHlms(void) const;

        bool hasPlanarReflections( const Renderable *renderable ) const;
        bool hasFlushPending( const Renderable *renderable ) const;
        bool hasActiveActor( const Renderable *renderable ) const;

        enum CustomParameterBits
        {
            UseActiveActor      = 0x80,
            FlushPending        = 0x60,
            InactiveActor       = 0x40
        };
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
