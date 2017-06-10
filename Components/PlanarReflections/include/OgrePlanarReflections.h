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
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Component
    *  @{
    */
    /** \addtogroup Material
    *  @{
    */

    class _OgrePlanarReflectionsExport PlanarReflections
    {
        struct TrackedRenderable
        {
            Renderable      *renderable;
            MovableObject   *movableObject;
            Vector3         reflNormal;
            Vector3         renderableCenter;
        };

        typedef FastArray<TrackedRenderable> TrackedRenderableArray;

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
        TrackedRenderableArray      mTrackedRenderables;

        uint8               mMaxActiveActors;
        Real                mInvMaxDistance;
        Real                mMaxSqDistance;
        SceneManager        *mSceneManager;
        CompositorManager2  *mCompositorManager;

        void pushActor( PlanarReflectionActor *actor );

    public:
        PlanarReflections( SceneManager *sceneManager, CompositorManager2 *compositorManager,
                           uint8 maxActiveActors, Real maxDistance, Camera *lockCamera );
        ~PlanarReflections();

        /** Adds an actor plane that other objects can use as source for reflections if they're
            close enough to it (and aligned enough to the normal).
        @param actor
            Actor to use, with some data prefilled (see PlanarReflectionActor constructor)
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
        @return
            ID of the actor.
        */
        PlanarReflectionActor* addActor( const PlanarReflectionActor &actor, bool useAccurateLighting,
                                         uint32 width, uint32 height, bool withMipmaps,
                                         PixelFormat pixelFormat, bool mipmapMethodCompute );
        void destroyActor( PlanarReflectionActor *actor );
        void destroyAllActors(void);

        void beginFrame(void);
        void update( Camera *camera, Real aspectRatio );

        /// Returns the amount of bytes that fillConstBufferData is going to fill.
        size_t getConstBufferSize(void) const;

        /** Fills 'passBufferPtr' with the necessary data for PlanarReflections.
            @see getConstBufferSize
        @remarks
            Assumes 'passBufferPtr' is aligned to a vec4/float4 boundary.
        */
        void fillConstBufferData( RenderTarget *renderTarget, const Matrix4 &projectionMatrix,
                                  float * RESTRICT_ALIAS passBufferPtr ) const;
        TexturePtr getTexture( uint8 actorIdx ) const;

        /// Returns true if the Camera settings (position, orientation, aspect ratio, etc)
        /// match with the reflection we have in cache.
        bool cameraMatches( const Camera *camera );
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
