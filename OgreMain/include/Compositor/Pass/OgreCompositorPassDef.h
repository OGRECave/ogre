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

#ifndef __CompositorPassDef_H__
#define __CompositorPassDef_H__

#include "OgreHeaderPrefix.h"

#include "OgrePrerequisites.h"
#include "OgreIdString.h"
#include "OgreResourceTransition.h"

namespace Ogre
{
    class CompositorNodeDef;

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Effects
    *  @{
    */
    enum CompositorPassType
    {
        PASS_INVALID = 0,
        PASS_SCENE,
        PASS_QUAD,
        PASS_CLEAR,
        PASS_STENCIL,
        PASS_RESOLVE,
        PASS_DEPTHCOPY,
        PASS_UAV,
        PASS_MIPMAP,
        PASS_COMPUTE,
        PASS_CUSTOM
    };

    extern const char *CompositorPassTypeEnumNames[PASS_CUSTOM+1u];

    class CompositorTargetDef;

    /** Interface to abstract all types of pass definitions (@see CompositorPassType):
            * PASS_SCENE (@See CompositorPassSceneDef)
            * PASS_QUAD (@See CompositorPassQuadDef)
            * PASS_CLEAR (@See CompositorPassClearDef)
            * PASS_STENCIL (@See CompositorPassStencilDef)
            * PASS_DEPTHCOPY (@See CompositorPassDepthCopy)
            * PASS_UAV (@See CompositorPassUavDef)
            * PASS_COMPUTE (@See CompositorPassComputeDef)
            * PASS_MIPMAP (@See CompositorPassMipmapDef)
        This class doesn't do much on its own. See the derived types for more information
        A definition is shared by all pass instantiations (i.e. Five CompositorPassScene can
        share the same CompositorPassSceneDef) and are assumed to remain const throughout
        their lifetime.
    @par
        Modifying a definition while there are active instantiations is undefined. Some
        implementations may see the change (eg. changing CompositorPassSceneDef::mFirstRQ)
        immediately while not see others (eg. changing CompositorPassSceneDef::mCameraName)
        Also crashes could happen depending on the changes being made.
    */
    class _OgreExport CompositorPassDef : public CompositorInstAlloc
    {
        CompositorPassType  mPassType;

        CompositorTargetDef *mParentTargetDef;

    public:
        /// Viewport's region to draw
        float               mVpLeft;
        float               mVpTop;
        float               mVpWidth;
        float               mVpHeight;
        float               mVpScissorLeft;
        float               mVpScissorTop;
        float               mVpScissorWidth;
        float               mVpScissorHeight;

        /// Shadow map index it belongs to (only filled in passes owned by Shadow Nodes)
        uint32              mShadowMapIdx;

        /// Number of times to perform the pass before stopping. -1 to never stop.
        uint32              mNumInitialPasses;

        /// Custom value in case there's a listener attached (to identify the pass)
        uint32              mIdentifier;

        /// True if a previous pass doesn't alter the contents of the same render target we do
        ///TODO: Fill this automatically.
        bool                mBeginRtUpdate;
        /// End if we're the last consecutive pass to alter the contents of the same render target
        bool                mEndRtUpdate;

        /// When false will not really bind the RenderTarget for rendering and
        /// use a null colour buffer instead. Useful for depth prepass, or if
        /// the RTT is actually an UAV.
        /// Some passes may ignore this setting (e.g. Clear passes)
        bool                mColourWrite;
        bool                mReadOnlyDepth;
        bool                mReadOnlyStencil;

        /** TODO: Refactor OgreOverlay to remove this design atrocity.
            A custom overlay pass is a better alternative (or just use their own RQ)
        */
        bool                mIncludeOverlays;

        uint8               mExecutionMask;
        uint8               mViewportModifierMask;

        /// Only used if mShadowMapIdx is valid (if pass is owned by Shadow Nodes). If true,
        /// we won't force the viewport to fit the region of the UV atlas on the texture,
        /// and respect mVp* settings instead.
        bool                mShadowMapFullViewport;

        IdStringVec         mExposedTextures;

        struct UavDependency
        {
            /// The slot must be in range [0; 64) and ignores the starting
            /// slot (@see CompositorPassUavDef::mStartingSlot)
            uint32                          uavSlot;

            /// The UAV pass already sets the texture access.
            /// However two passes in a row may only read from it,
            /// thus having this information is convenient (without
            /// needing to add another bind UAV pass)
            ResourceAccess::ResourceAccess  access;
            bool                            allowWriteAfterWrite;

            UavDependency( uint32 _uavSlot, ResourceAccess::ResourceAccess _access,
                           bool _allowWriteAfterWrite ) :
                uavSlot( _uavSlot ), access( _access ), allowWriteAfterWrite( _allowWriteAfterWrite ) {}
        };
        typedef vector<UavDependency>::type UavDependencyVec;
        UavDependencyVec    mUavDependencies;

        String              mProfilingId;

    public:
        CompositorPassDef( CompositorPassType passType, CompositorTargetDef *parentTargetDef ) :
            mPassType( passType ), mParentTargetDef( parentTargetDef ),
            mVpLeft( 0 ), mVpTop( 0 ),
            mVpWidth( 1 ), mVpHeight( 1 ),
            mVpScissorLeft( 0 ), mVpScissorTop( 0 ),
            mVpScissorWidth( 1 ), mVpScissorHeight( 1 ),
            mShadowMapIdx( ~0U ),
            mNumInitialPasses( ~0U ), mIdentifier( 0U ),
            mBeginRtUpdate( true ), mEndRtUpdate( true ),
            mColourWrite( true ),
            mReadOnlyDepth( false ),
            mReadOnlyStencil( false ),
            mIncludeOverlays( false ),
            mExecutionMask( 0xFF ),
            mViewportModifierMask( 0xFF ),
            mShadowMapFullViewport( false ) {}
        virtual ~CompositorPassDef() {}

        CompositorPassType getType() const              { return mPassType; }
        uint32 getRtIndex(void) const;
        const CompositorTargetDef* getParentTargetDef(void) const;
    };

    typedef vector<CompositorPassDef*>::type CompositorPassDefVec;

    class _OgreExport CompositorTargetDef : public CompositorInstAlloc
    {
        /// Name is local to Node! (unless using 'global_' prefix)
        IdString                mRenderTargetName;
        CompositorPassDefVec    mCompositorPasses;

        /// Used for cubemaps and 3D textures.
        uint32                  mRtIndex;

        /// Used by shadow map passes only. Determines which light types are supposed
        /// to be run with the current shadow casting light. i.e. usually point lights
        /// need to be treated differently, and only directional lights are compatible
        /// with PSSM. This bitmask contains:
        ///     mShadowMapSupportedLightTypes & 1u << Light::LT_DIRECTIONAL
        ///     mShadowMapSupportedLightTypes & 1u << Light::LT_POINT
        ///     mShadowMapSupportedLightTypes & 1u << Light::LT_SPOTLIGHT
        uint8                   mShadowMapSupportedLightTypes;

        CompositorNodeDef       *mParentNodeDef;

    public:
        CompositorTargetDef( IdString renderTargetName, uint32 rtIndex,
                             CompositorNodeDef *parentNodeDef ) :
                mRenderTargetName( renderTargetName ),
                mRtIndex( rtIndex ),
                mShadowMapSupportedLightTypes( 0 ),
                mParentNodeDef( parentNodeDef ) {}
        ~CompositorTargetDef();

        IdString getRenderTargetName() const            { return mRenderTargetName; }
        uint32 getRtIndex(void) const                   { return mRtIndex; }

        void setShadowMapSupportedLightTypes( uint8 types ) { mShadowMapSupportedLightTypes = types; }
        uint8 getShadowMapSupportedLightTypes(void) const   { return mShadowMapSupportedLightTypes; }

        /** Reserves enough memory for all passes (efficient allocation)
        @remarks
            Calling this function is not obligatory, but recommended
        @param numPasses
            The number of passes expected to contain.
        */
        void setNumPasses( size_t numPasses )           { mCompositorPasses.reserve( numPasses ); }

        CompositorPassDef* addPass( CompositorPassType passType, IdString customId = IdString() );

        const CompositorPassDefVec& getCompositorPasses() const { return mCompositorPasses; }

        /// @copydoc CompositorManager2::getNodeDefinitionNonConst
        CompositorPassDefVec& getCompositorPassesNonConst()     { return mCompositorPasses; }
    };

    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif
