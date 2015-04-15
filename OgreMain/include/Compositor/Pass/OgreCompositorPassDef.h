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
        PASS_CUSTOM
    };

    /** Interface to abstract all types of pass definitions (@see CompositorPassType):
            * PASS_SCENE (@See CompositorPassSceneDef)
            * PASS_QUAD (@See CompositorPassQuadDef)
            * PASS_CLEAR (@See CompositorPassClearDef)
            * PASS_STENCIL (@See CompositorPassStencilDef)
        This class doesn't do much on its own. See the derived types for more information
        A definition is shared by all pass instantiations (i.e. Five CompositorPassScene can
        share the same CompositorPassSceneDef) and are asumed to remain const throughout
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

        /// Used for cubemaps and 3D textures.
        uint32              mRtIndex;

    public:
        /// Viewport's region to draw
        float               mVpLeft;
        float               mVpTop;
        float               mVpWidth;
        float               mVpHeight;

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

        /** TODO: Refactor OgreOverlay to remove this design atrocity.
            A custom overlay pass is a better alternative (or just use their own RQ)
        */
        bool                mIncludeOverlays;

    public:
        CompositorPassDef( CompositorPassType passType, uint32 rtIndex ) :
            mPassType( passType ), mRtIndex( rtIndex ),
            mVpLeft( 0 ), mVpTop( 0 ),
            mVpWidth( 1 ), mVpHeight( 1 ), mShadowMapIdx( 0 ),
            mNumInitialPasses( -1 ), mIdentifier( 0 ),
            mBeginRtUpdate( true ), mEndRtUpdate( true ),
            mIncludeOverlays( false ) {}
        virtual ~CompositorPassDef() {}

        CompositorPassType getType() const              { return mPassType; }
        uint32 getRtIndex(void) const                   { return mRtIndex; }
    };

    typedef vector<CompositorPassDef*>::type CompositorPassDefVec;

    class _OgreExport CompositorTargetDef : public CompositorInstAlloc
    {
        /// Name is local to Node! (unless using 'global_' prefix)
        IdString                mRenderTargetName;
        CompositorPassDefVec    mCompositorPasses;

        /// @copydoc CompositorPass::mRtIndex
        uint32                  mRtIndex;

        CompositorNodeDef       *mParentNodeDef;

    public:
        CompositorTargetDef( IdString renderTargetName, uint32 rtIndex,
                             CompositorNodeDef *parentNodeDef ) :
                mRenderTargetName( renderTargetName ),
                mRtIndex( rtIndex ),
                mParentNodeDef( parentNodeDef ) {}
        ~CompositorTargetDef();

        IdString getRenderTargetName() const            { return mRenderTargetName; }

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
