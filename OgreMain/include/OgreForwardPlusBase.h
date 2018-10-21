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
#ifndef _OgreForwardPlusBase_H_
#define _OgreForwardPlusBase_H_

#include "OgrePrerequisites.h"
#include "OgreCommon.h"
#include "OgreLight.h"
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

    static const size_t c_ForwardPlusNumFloat4PerLight = 6u;
    static const size_t c_ForwardPlusNumFloat4PerDecal = 4u;
    static const size_t c_ForwardPlusNumFloat4PerCubemapProbe = 7u;

    /** ForwardPlusBase */
    class _OgreExport ForwardPlusBase : public HlmsAlloc
    {
    public:
        enum ForwardPlusMethods
        {
            MethodForward3D,
            MethodForwardClustered,
            NumForwardPlusMethods
        };

        struct CachedGridBuffer
        {
            TexBufferPacked *gridBuffer;
            TexBufferPacked *globalLightListBuffer;
            CachedGridBuffer() : gridBuffer( 0 ), globalLightListBuffer( 0 ) {}
        };

        typedef vector<CachedGridBuffer>::type CachedGridBufferVec;

        static const size_t MinDecalRq;     // Inclusive
        static const size_t MaxDecalRq;     // Inclusive

    protected:
        static const size_t NumBytesPerLight;
        static const size_t NumBytesPerDecal;

        struct CachedGrid
        {
            Camera                  *camera;
            Vector3                 lastPos;
            Quaternion              lastRot;
            /// Cameras used for reflection have a different view proj matrix
            bool                    reflection;
            /// Cameras can change their AR depending on the RTT they're rendering to.
            Real                    aspectRatio;
            uint32                  visibilityMask;
            /// Cameras w/out shadows have a different light list from cameras that do.
            CompositorShadowNode const *shadowNode;
            /// Last frame this cache was updated.
            uint32                  lastFrame;

            uint32                  currentBufIdx;
            CachedGridBufferVec     gridBuffers;
        };

        enum ObjTypes
        {
            ObjType_Decal = 0,
            NumObjTypes
        };

        struct LightCount
        {
            //We use LT_DIRECTIONAL (index = 0) to contain the total light count.
            uint32  lightCount[Light::MAX_FORWARD_PLUS_LIGHTS];
            uint32  objCount[NumObjTypes];
            LightCount()
            {
                memset( lightCount, 0, sizeof(lightCount) );
                memset( objCount, 0, sizeof(objCount) );
            }
        };

        typedef vector<CachedGrid>::type CachedGridVec;
        CachedGridVec   mCachedGrid;
        LightArray      mCurrentLightList;

        FastArray<LightCount>   mLightCountInCell;

        VaoManager      *mVaoManager;
        SceneManager    *mSceneManager;

        bool    mDebugMode;
        bool    mFadeAttenuationRange;
        /// VPLs = Virtual Point Lights. Used by InstantRadiosity.
        bool    mEnableVpls;
        bool    mDecalsEnabled;
#if !OGRE_NO_FINE_LIGHT_MASK_GRANULARITY
        bool    mFineLightMaskGranularity;
#endif
        //How many float4 to skip before Decals start in globalLightListBuffer
        uint16 mDecalFloat4Offset;

        static size_t calculateBytesNeeded( size_t numLights, size_t numDecals );

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
        bool getCachedGridFor( const Camera *camera, const CachedGrid **outCachedGrid ) const;

        /// Check if some of the caches are really old and delete them
        void deleteOldGridBuffers(void);

    public:
        ForwardPlusBase( SceneManager *sceneManager, bool decalsEnabled );
        virtual ~ForwardPlusBase();

        virtual ForwardPlusMethods getForwardPlusMethod(void) const = 0;

        void _changeRenderSystem( RenderSystem *newRs );

        virtual void collectLights( Camera *camera ) = 0;

        bool isCacheDirty( const Camera *camera ) const;

        /// Cache the return value as internally we perform an O(N) search
        TexBufferPacked* getGridBuffer( Camera *camera ) const;
        /// Cache the return value as internally we perform an O(N) search
        TexBufferPacked* getGlobalLightListBuffer( Camera *camera ) const;

        /// Returns the amount of bytes that fillConstBufferData is going to fill.
        virtual size_t getConstBufferSize(void) const = 0;

        /** Fills 'passBufferPtr' with the necessary data for ForwardPlusBase rendering.
            @see getConstBufferSize
        @remarks
            Assumes 'passBufferPtr' is aligned to a vec4/float4 boundary.
        */
        virtual void fillConstBufferData( Viewport *viewport, RenderTarget* renderTarget,
                                          IdString shaderSyntax,
                                          float * RESTRICT_ALIAS passBufferPtr ) const = 0;

        virtual void setHlmsPassProperties( Hlms *hlms );

        /// Turns on visualization of light cell occupancy
        void setDebugMode( bool debugMode )                             { mDebugMode = debugMode; }
        bool getDebugMode(void) const                                   { return mDebugMode; }

        /// Attenuates the light by the attenuation range, causing smooth endings when
        /// at the end of the light range instead of a sudden sharp termination. This
        /// isn't physically based (light's range is infinite), but looks very well,
        /// and makes more intuitive to manipulate a light by controlling its range
        /// instead of controlling its radius. @see Light::setAttenuationBasedOnRadius
        /// and @see setAttenuation.
        /// And even when controlling the light by its radius, you don't have to worry
        /// so much about the threshold's value being accurate.
        /// It has a tendency to make lights dimmer though. That's the price to pay
        /// for this optimization and having more intuitive controls.
        /// Enabled by default.
        ///
        /// In math:
        ///     atten *= max( (attenRange - fDistance) / attenRange, 0.0f );
        void setFadeAttenuationRange( bool fade )                       { mFadeAttenuationRange = fade; }
        bool getFadeAttenuationRange(void) const                        { return mFadeAttenuationRange; }

        void setEnableVpls( bool enable )                               { mEnableVpls = enable; }
        bool getEnableVpls(void) const                                  { return mEnableVpls; }

        bool getDecalsEnabled(void) const                               { return mDecalsEnabled; }

#if !OGRE_NO_FINE_LIGHT_MASK_GRANULARITY
        /// Toggles whether light masks will be obeyed per object & per light by doing:
        /// if( movableObject->getLightMask() & light->getLightMask() )
        ///     doLighting( movableObject light );
        /// Note this toggle only affects Forward+ lights.
        /// You may want to see HlmsPbs::setFineLightMaskGranularity
        /// for control over Forward lights.
        /// If you only need coarse granularity control over Forward+ lights, you
        /// may get higher performance via CompositorPassSceneDef::mLightVisibilityMask
        /// (light_visibility_mask keyword in scripts).
        void setFineLightMaskGranularity( bool useFineGranularity )
                                                    { mFineLightMaskGranularity = useFineGranularity; }
        bool getFineLightMaskGranularity(void) const{ return mFineLightMaskGranularity; }
#endif
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
