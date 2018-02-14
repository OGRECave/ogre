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

#ifndef __CompositorShadowNode_H__
#define __CompositorShadowNode_H__

#include "OgreHeaderPrefix.h"

#include "Compositor/OgreCompositorNode.h"
#include "Compositor/OgreCompositorShadowNodeDef.h"
#include "OgreShadowCameraSetup.h"
#include "OgreLight.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Effects
    *  @{
    */

    typedef vector<TexturePtr>::type TextureVec;

    /** Shadow Nodes are special nodes (not to be confused with @see CompositorNode)
        that are only used for rendering shadow maps.
        Normal Compositor Nodes can share or own a ShadowNode. The ShadowNode will
        render the scene enough times to fill all shadow maps so the main scene pass
        can use them.
    @par
        ShadowNode are very flexible compared to Ogre 1.x; as they allow mixing multiple
        shadow camera setups for different lights.
    @par
        Shadow Nodes derive from nodes so that they can be used as regular nodes
    @par
        During a render with shadow mapping enabled, we render first the Shadow Node's pass,
        then render the regular scene.
        In the past there used to be an AABB enclosing all visible objects that receive shadows
        that was used for calculating the shadow maps. This forced Ogre 2.x to split
        rendering into two stages: _cullPhase01 & _renderPhase02.
        This is not needed anymore.

        To summarize: a normal rendering flow with shadow map looks like this:
            shadowNode->setupShadowCamera( normal->getVisibleBoundsInfo() );
            shadowNode->_cullPhase01();
            shadowNode->_renderPhase02();
            normal->_cullPhase01();
            normal->_renderPhase02();
    @par
    @par
        On forward lighting passes, shadow mapping is handled in the following way:
            1) Build a list of all lights visible by all cameras (SceneManager does this)
            2) Traverse the list to get the closest lights to the current camera.
               These lights will cast shadows.
            3) Build a list of the closest lights for each object (SceneManager does this)
            4) Traverse this list and find those that are actually casting a shadow
            5) Send to the GPU & engine the list in step 4, but shadow casting lights are
               put first, then sorted by proximity.
        See the comments inside the function setShadowMapsToPass for more information.
    @author
        Matias N. Goldberg
    @version
        1.0
    */
    class _OgreExport CompositorShadowNode : public CompositorNode
    {
    public:
        typedef vector<bool>::type LightsBitSet;

    private:
        CompositorShadowNodeDef const *mDefinition;

        struct ShadowMapCamera
        {
            ShadowCameraSetupPtr    shadowCameraSetup;
            Camera                  *camera;
            /// TexturePtr is at mLocalTextures[idxToLocalTextures]
            uint32                  idxToLocalTextures;
            /// Index to mContiguousShadowMapTex[idxToContiguousTex]
            /// mContiguousShadowMapTex keeps them together for binding quickly
            /// during render.
            /// Several shadow maps may reference the same texture (i.e. UV atlas)
            /// Hence the need for this idx variable.
            uint32                  idxToContiguousTex;
            /// @See ShadowCameraSetup mMinDistance
            Real                    minDistance;
            Real                    maxDistance;
            Vector2                 scenePassesViewportSize[Light::NUM_LIGHT_TYPES];
        };

        typedef vector<ShadowMapCamera>::type ShadowMapCameraVec;
        /// One per shadow map (whether texture or atlas)
        ShadowMapCameraVec      mShadowMapCameras;

        /// If all shadowmaps share the same texture (i.e. UV atlas), then
        /// mContiguousShadowMapTex.size() == 1. We can't use mLocalTextures
        /// directly because it could have textures unrelated to shadow mapping
        /// (or indirectly related)
        TextureVec              mContiguousShadowMapTex;

        Camera const *          mLastCamera;
        size_t                  mLastFrame;
        size_t                  mNumActiveShadowMapCastingLights;
        /// mShadowMapCastingLights may have gaps (can happen if no light of
        /// the types the shadow map supports could be assigned at this slot)
        LightClosestArray       mShadowMapCastingLights;
        vector<size_t>::type    mTmpSortedIndexes;

        /** Cached value. Contains the aabb of all caster-only objects (filtered by
            camera's visibility flags) from the minimum RQ used by our shadow render
            passes, to the maximum RQ used. The tighter the box, the higher the
            shadow quality.
        */
        AxisAlignedBox          mCastersBox;

        LightsBitSet            mAffectedLights;

        /// Changes with each call to setShadowMapsToPass
        LightList               mCurrentLightList;

        /** Called by update to find out which lights are the ones closest to the given
            camera. Early outs if we've already calculated our stuff for that camera in
            a previous call.
            Also updates internals lists for easy look up of lights <-> shadow maps
        @param newCamera
            User camera to base our shadow map cameras from.
        */
        void buildClosestLightList(Camera *newCamera , const Camera *lodCamera);

        /** Finds the first index to mShadowMapCastingLights[*startIdx] where
            mShadowMapCastingLights[i].light == 0; starting from startIdx (inclusive).
            and the first index to mShadowMapCastingLights[*entryToUse] where
            lightTypeMask & mDefinition->mLightTypesMask[*entryToUse] != 0
        @param startIdx [in/out]
            [in] Where to start searching from.
            [out] Where to start searching from the next time you call this function.
                  Outputs mShadowMapCastingLights.size() if there are no more empty
                  entries.
            For example if our light types are like this:
                dir point point dir point
            And you're looking for all the directional lights, then sucessively calling
            this function will return:
                startIdx = 0, entryToUse = 0
                startIdx = 1, entryToUse = 3
                startIdx = 1, entryToUse = 5 --> 5 == mShadowMapCastingLights.size()
            startIdx will always be 1 until that point light entry is filled.
        @param entryToUse [out]
            What entry to use. Outputs mShadowMapCastingLights.size() if there are no more
            empty that supports the requested light types.
        */
        void findNextEmptyShadowCastingLightEntry( uint8 lightTypeMask,
                                                   size_t * RESTRICT_ALIAS inOutStartIdx,
                                                   size_t * RESTRICT_ALIAS outEntryToUse ) const;

        void clearShadowCastingLights( const LightListInfo &globalLightList );
        void restoreStaticShadowCastingLights( const LightListInfo &globalLightList );

    public:
        CompositorShadowNode( IdType id, const CompositorShadowNodeDef *definition,
                              CompositorWorkspace *workspace, RenderSystem *renderSys,
                              const RenderTarget *finalTarget );
        virtual ~CompositorShadowNode();

        const CompositorShadowNodeDef* getDefinition() const            { return mDefinition; }

        /** Renders into the shadow map, executes passes
        @param camera
            Camera used to calculate our shadow camera (in case of directional lights).
        */
        void _update(Camera* camera, const Camera *lodCamera, SceneManager *sceneManager);

        /// We derive so we can override the camera with ours
        virtual void postInitializePass( CompositorPass *pass );

        const LightList* setShadowMapsToPass( Renderable* rend, const Pass* pass,
                                              AutoParamDataSource *autoParamDataSource,
                                              size_t startLight );

        /// @See mCastersBox
        const AxisAlignedBox& getCastersBox(void) const     { return mCastersBox; }

        bool isShadowMapIdxInValidRange( uint32 shadowMapIdx ) const;

        /// Returns true if the shadow map index is not active. For example:
        ///     * There are 3 shadow maps, but only 2 shadow casting lights
        ///     * There are 3 directional maps for directional PSSM, but no directional light.
        bool isShadowMapIdxActive( uint32 shadowMapIdx ) const;

        bool _shouldUpdateShadowMapIdx( uint32 shadowMapIdx ) const;

        /// Do not call this if isShadowMapIdxActive == false or isShadowMapIdxInValidRange == false
        uint8 getShadowMapLightTypeMask( uint32 shadowMapIdx ) const;

        /// Note: May return null if there is no such shadowMapIdx, or if there
        /// is no light that could be linked with that shadow map index.
        /// i.e. if isShadowMapIdxActive( shadowMapIdx ) is true, then we'll
        /// return a valid pointer.
        const Light* getLightAssociatedWith( uint32 shadowMapIdx ) const;

        /** Outputs the min & max depth range for the given camera. 0 & 100000 if camera not found
        @remarks
            Performs linear search O(N), except the overload that provides a shadowMapIdx
        */
        void getMinMaxDepthRange( const Frustum *shadowMapCamera, Real &outMin, Real &outMax ) const;
        void getMinMaxDepthRange( size_t shadowMapIdx, Real &outMin, Real &outMax ) const;

        /// Returns the texture view projection matrix for the given shadow map index
        Matrix4 getViewProjectionMatrix( size_t shadowMapIdx ) const;
        /// Returns the texture view matrix for the given shadow map index
        const Matrix4& getViewMatrix( size_t shadowMapIdx ) const;

        /** Returns a list of points with the limits of each PSSM split in projection space
            for the given shadow map index.
        @remarks
            If shadow map 0, 1 & 2 use light 0 with different splits, the return value should
            be the same for all of them.
        @return
            An array with the split points. The number of elements is N+1 where N is the number
            of splits for that shadow map.
            Returns null if shadowMapIdx is out of bounds, or is not a PSSM technique.
        */
        const vector<Real>::type* getPssmSplits( size_t shadowMapIdx ) const;

        /** Returns a list of points with the blend band boundaries of the closest N-1 PSSM split
            in projection space for the given shadow map index.
        @remarks
            @see getPssmSplits
        @return
            An array with the blend points. The number of elements is N-1 where N is the number
            of splits for that shadow map.
            Returns null if shadowMapIdx is out of bounds, or is not a PSSM technique.
        */
        const vector<Real>::type* getPssmBlends( size_t shadowMapIdx ) const;

        /** Returns the fade point of the last PSSM split in projection space
            for the given shadow map index.
        @remarks
            @see getPssmSplits
        @return
            The fade point.
            Returns null if shadowMapIdx is out of bounds, or is not a PSSM technique.
        */
        const Real* getPssmFade( size_t shadowMapIdx ) const;

        /** The return value may change in the future, which happens when the number of lights
            changes to or from a value lower than the supported shadow casting lights by the
            definition.
        */
        size_t getNumActiveShadowCastingLights(void) const
                                                            { return mNumActiveShadowMapCastingLights; }
        const LightClosestArray& getShadowCastingLights(void) const { return mShadowMapCastingLights; }

        const LightsBitSet& getAffectedLightsBitSet(void) const     { return mAffectedLights; }

        const TextureVec& getContiguousShadowMapTex(void) const     { return mContiguousShadowMapTex; }
        uint32 getIndexToContiguousShadowMapTex( size_t shadowMapIdx ) const;

        /** Marks a shadow map as statically updated, and ties the given light to always use
            that shadow map.
        @remarks
            By default Ogre recalculates the shadow maps every single frame (even if nothing
            has changed). However if you know that whatever a light is illuminating is not
            changing at all (or barely changing), with static shadow maps you are the one who
            tells Ogre when to update it (e.g. you may only need to update it three times during
            the whole level); hence the framerate goes up.
            Perceived quality may also go up because by default Ogre applies shadow mapping on the
            closest lights; so shadows flip on and off as you move the camera (because lights that
            had no shadows get closer while lights that were using shadows get farther away).
            While often this is desirable, there are cases where the artist may want a particular
            light to always have shadows (regardless of distance); with static shadow maps you can
            force that; hence the perceived quality may go up (but that's up to the talent of the
            artist and the scene in particular).
        @par
            Note that for point & spot lights, you have to consider if the light changed
            (e.g. moved, rotated) or if anything that is or could be lit by the light has moved
            Directional lights are harder because they depend on the camera placement as well.
        @par
            Use setStaticShadowMapDirty to tell Ogre to update the shadow map in the next render.
        @par
            Ogre may call light->setCastShadows( true ); on the light.
        @par
            IMPORTANT: Do not put static and dynamic shadow maps in the same UV atlas.
            It's asking for trouble and will probably not work. Keep the atlas separate.
        @par
            VERY IMPORTANT: You *must* respect lights are set in the following order:
                1. Directional
                2. Point
                3. Spot
            If you have shadow maps defined that support both point & spotlight, and you want
            to mix both static lights with dynamic ones; set fixed point lights in the first
            shadow map indices and spotlight in the last indices, to avoid Ogre automatically
            (e.g.) placing a point light after your static spot light.
        @param shadowMapIdx
            Shadow map index to tie this light to. If this shadow map index is part of a PSSM
            split, all PSSM splits will be affected (thus you only need to call it once for
            any of the split that belong to the same set)
        @param light
            Light to tie to the given shadow map. Null pointer disables it.
        */
        void setLightFixedToShadowMap( size_t shadowMapIdx, Light *light );

        /// Tags a static shadow map as dirty, causing Ogre to update it on the next time this
        /// Shadow node gets executed.
        /// If drawing to a texture atlas, multiple shadow maps may be sharing the same texture,
        /// thus if you're doing a clear on the whole atlas, you will need to update all of
        /// the shadow maps, not just this one. Use includeLinked=true to mark as dirty all
        /// static shadow maps that share the same atlas.
        /// Set it to false if that's explicitly what you want, or if you're already going
        /// to call it for every shadow map (otherwise you will trigger a O(N^2) behavior).
        void setStaticShadowMapDirty( size_t shadowMapIdx, bool includeLinked=true );

        /// @copydoc CompositorNode::finalTargetResized
        virtual void finalTargetResized( const RenderTarget *finalTarget );
    };

    class _OgreExport ShadowNodeHelper
    {
    public:
        struct _OgreExport Resolution
        {
            uint32 x;
            uint32 y;

            Resolution();
            Resolution( uint32 x, uint32 y );

            uint64 asUint64(void) const;
        };

        struct _OgreExport ShadowParam
        {
            /// Bitmask OR of e.g.
            /// (1u << LT_DIRECTIONAL) | (1u << LT_POINT) | (1u << LT_SPOTLIGHT)
            uint32 supportedLightTypes;
            /// Technique to use
            ShadowMapTechniques technique;
            /// Number of PSSM splits. In range [2; 4]. Ignored for non-PSSM techniques
            uint8 numPssmSplits;
            /// What texture atlas to use. Should be in range [0; numShadowParam)
            /// You can have all shadow maps share the same shadow map if you want.
            /// Don't leave gaps (e.g. use atlasId = 0 and atlasId = 2, but not atlasId = 1)
            uint8 atlasId;
            /// In pixels, start of the texture (XY). One for each PSSM split.
            /// When not using PSSM, entries in range [1; 4) are ignored.
            /// Be careful not to overlap within the same atlasId.
            Resolution atlasStart[4];
            Resolution resolution[4];

            /// See Light::LightTypes
            void addLightType( Light::LightTypes lightType );
        };
        typedef vector<ShadowParam>::type ShadowParamVec;

        /** Utility to programmatically create a shadow node, since doing it yourself
            can be confusing.
        @param compositorManager
        @param capabilities
        @param shadowNodeName
            Name to give to the shadow node definition. Must be unique and not exist already.
        @param shadowParams
            Array of params, one per shadow map.
            PSSM techniques must come first, and there can only be one shadow map using
            that technique.
        @param useEsm
            True if the shadow node should be set for ESM (Exponential Shadow Maps)
        @param pointLightCubemapResolution
            The resolution to use for the temporary cubemap used in case of point lights.
            If you don't set point lights in any of the ShadowParam::supportedLightTypes in
            the shadowParams array, this value is ignored.
        @param pssmLambda
            PSSM lambda. Ignored if not using PSSM.
        @param splitPadding
            PSSM split padding. Ignored if not using PSSM.
        @param splitBlend
            PSSM blend. Ignored if not using PSSM.
        @param splitFade
            PSSM split fade. Ignored if not using PSSM.
        */
        static void createShadowNodeWithSettings( CompositorManager2 *compositorManager,
                                                  const RenderSystemCapabilities *capabilities,
                                                  const String &shadowNodeName,
                                                  const ShadowNodeHelper::
                                                  ShadowParamVec &shadowParams,
                                                  bool useEsm,
                                                  uint32 pointLightCubemapResolution=1024u,
                                                  Real pssmLambda=0.95f, Real splitPadding=1.0f,
                                                  Real splitBlend=0.125f, Real splitFade=0.313f );
    };

    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif
