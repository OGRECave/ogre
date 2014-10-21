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

#ifndef __CompositorPassSceneDef_H__
#define __CompositorPassSceneDef_H__

#include "OgreHeaderPrefix.h"

#include "../OgreCompositorPassDef.h"

#include "OgreVisibilityFlags.h"
#include "OgreMaterialManager.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Effects
    *  @{
    */

    enum ShadowNodeRecalculation
    {
        SHADOW_NODE_RECALCULATE,
        SHADOW_NODE_REUSE,
        SHADOW_NODE_FIRST_ONLY,
        SHADOW_NODE_CASTER_PASS     //Set automatically only when this pass is used by a ShadowNode
    };

    class _OgreExport CompositorPassSceneDef : public CompositorPassDef
    {
    public:
        /// Viewport's visibility mask while rendering our pass
        uint32                  mVisibilityMask;
        IdString                mShadowNode;
        ShadowNodeRecalculation mShadowNodeRecalculation; //Only valid if mShadowNode is not empty
        /// When empty, uses the default camera.
        IdString                mCameraName;
        /** When empty, it implies mCameraName == mLodCameraName; except for shadow nodes.
            For shadow nodes, when empty, it will use the receiver's lod camera.
        */
        IdString                mLodCameraName;

        /// First Render Queue ID to render. Inclusive
        uint8           mFirstRQ;
        /// Last Render Queue ID to render. Not inclusive
        uint8           mLastRQ;

        /** When true, the camera will be rotated 90°, -90° or 180° depending on the value of
            mRtIndex and then restored to its original rotation after we're done.
        */
        bool            mCameraCubemapReorient;

        /** When true, which Lod index is current will be updated. Reasons to set this to false:
             1. You don't use LOD (i.e. you're GPU bottleneck). Setting to false helps CPU.
             2. LODs have been calculated in a previous pass. This happens if previous pass(es)
                all used the same lod camera and all RenderQueue IDs this pass will use have
                been rendered already and updated their lod lists.
        @remarks
            Automatically set to false for shadow nodes that leave mLodCameraName empty
        */
        bool            mUpdateLodLists;

        /** Multiplier to the Lod value. What it means depends on the technique.
            You'll probably want to avoid setting it directly and rather use
            @LodStrategy::transformBias
        */
        Real            mLodBias;

        /** The material scheme used for this pass. If no material scheme is set then
            it will use the default scheme
        */
        String          mMaterialScheme;

        CompositorPassSceneDef( uint32 rtIndex ) :
            CompositorPassDef( PASS_SCENE, rtIndex ),
            mVisibilityMask( VisibilityFlags::RESERVED_VISIBILITY_FLAGS ),
            mShadowNodeRecalculation( SHADOW_NODE_FIRST_ONLY ),
            mFirstRQ( 0 ),
            mLastRQ( -1 ),
            mCameraCubemapReorient( false ),
            mUpdateLodLists( true ),
            mLodBias( 1.0f ),
            mMaterialScheme(MaterialManager::DEFAULT_SCHEME_NAME)
        {
            //Change base defaults
            mIncludeOverlays = true;
        }

        void setVisibilityMask( uint32 visibilityMask )
        {
            mVisibilityMask = visibilityMask & VisibilityFlags::RESERVED_VISIBILITY_FLAGS;
        }
    };

    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif
