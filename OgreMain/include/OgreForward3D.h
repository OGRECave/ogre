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
#ifndef _OgreForward3D_H_
#define _OgreForward3D_H_

#include "OgrePrerequisites.h"
#include "OgreCommon.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Resources
    *  @{
    */

    class CompositorShadowNode;

    /** Forward3D */
    class _OgreExport Forward3D : public PassAlloc
    {
        struct Resolution
        {
            uint32 width;
            uint32 height;
            Real   zEnd; /// Depth at which this slice ends, in view space.
            Resolution() : width( 0 ), height( 0 ), zEnd( 0 ) {}
            Resolution( uint32 w, uint32 h, Real _zEnd ) :
                width( w ), height( h ), zEnd( _zEnd ) {}
        };

        struct CachedGrid
        {
            Camera                  *camera;
            /// Cameras used for reflection have a different view proj matrix
            bool                    reflection;
            /// Cameras can change their AR depending on the RTT they're rendering to.
            Real                    aspectRatio;
            /// Cameras w/out shadows have a different light list from cameras that do.
            CompositorShadowNode const *shadowNode;
            /// Last frame this cache was updated.
            uint32                  lastFrame;

            TexBufferPacked         *gridBuffer;
            TexBufferPacked         *globalLightListBuffer;
        };

        typedef vector<CachedGrid>::type CachedGridVec;
        CachedGridVec   mCachedGrid;
        LightArray      mCurrentLightList;

        uint32  mWidth;
        uint32  mHeight;
        uint32  mNumSlices;
        uint32  mLightsPerCell;
        uint32  mTableSize; /// Automatically calculated, size of the first table, elements.

        FastArray<Resolution>   mResolutionAtSlice;
        FastArray<uint32>       mLightCountInCell;

        float   mMinDistance;
        float   mMaxDistance;
        float   mInvMaxDistance;

        VaoManager      *mVaoManager;
        SceneManager    *mSceneManager;

        bool    mDebugMode;

        /// Performs the reverse of getSliceAtDepth. @see getSliceAtDepth.
        inline Real getDepthAtSlice( uint32 slice ) const;

        /** Returns the slice index at the given depth
        @param depth
            Depth, in view space.
        @return
            Slice index, in range [0; mNumSlices)
        */
        inline uint32 getSliceAtDepth( Real depth ) const;

        /** Converts a vector in projection space to grid space from the specified slice index.
        @param projSpace
            projSpace.x & projSpace.y should be in range [0; 1]
        @param slice
            Slice index.
        @param outX [out]
            The column of table in the given slice.
            Will be in range [0; mResolutionAtSlice[slice].width)
        @param outY [out]
            The row of the table in the given slice.
            Will be in range [0; mResolutionAtSlice[slice].height)
        */
        inline void projectionSpaceToGridSpace( const Vector2 &projSpace, uint32 slice,
                                                uint32 &outX, uint32 &outY ) const;

        void fillGlobalLightListBuffer( Camera *camera, TexBufferPacked *globalLightListBuffer );

        /** Finds a grid already cached in mCachedGrid that can be used for the given camera.
            If the cache does not exist, we create a new entry.
        @param camera
            The camera for which we'll find a cached entry.
        @param outCachedGrid
            The CachedGrid being retrieved. May be new or an existing one.
            This pointer may be invalidated on the next call to getCachedGridFor
        @return
            True if the cache is up to date. False if the cache needs to be updated.
        */
        bool getCachedGridFor( Camera *camera, CachedGrid **outCachedGrid );

        /// The const version will not create a new cache if not found, and
        /// output a null pointer instead (also returns false in that case).
        bool getCachedGridFor( Camera *camera, const CachedGrid **outCachedGrid ) const;

    public:
        Forward3D( uint32 width, uint32 height, uint32 numSlices, uint32 lightsPerCell,
                   float minDistance, float maxDistance, SceneManager *sceneManager );
        ~Forward3D();

        void _changeRenderSystem( RenderSystem *newRs );

        void collectLights( Camera *camera );

        uint32 getNumSlices(void) const                                 { return mNumSlices; }

        /// Cache the return value as internally we perform an O(N) search
        TexBufferPacked* getGridBuffer( Camera *camera ) const;
        /// Cache the return value as internally we perform an O(N) search
        TexBufferPacked* getGlobalLightListBuffer( Camera *camera ) const;

        /// Returns the amount of bytes that fillConstBufferData is going to fill.
        size_t getConstBufferSize(void) const;

        /** Fills 'passBufferPtr' with the necessary data for Forward3D rendering.
            @see getConstBufferSize
        @remarks
            Assumes 'passBufferPtr' is aligned to a vec4/float4 boundary.
        */
        void fillConstBufferData( RenderTarget *renderTarget,
                                  float * RESTRICT_ALIAS passBufferPtr ) const;

        /// Turns on visualization of light cell occupancy
        void setDebugMode( bool debugMode )                             { mDebugMode = debugMode; }
        bool getDebugMode(void) const                                   { return mDebugMode; }
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
