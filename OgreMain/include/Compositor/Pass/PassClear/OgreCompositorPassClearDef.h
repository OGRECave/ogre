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

#ifndef __CompositorPassClearDef_H__
#define __CompositorPassClearDef_H__

#include "OgreHeaderPrefix.h"

#include "../OgreCompositorPassDef.h"
#include "OgreCommon.h"
#include "OgreColourValue.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Effects
    *  @{
    */

    class _OgreExport CompositorPassClearDef : public CompositorPassDef
    {
    public:
        uint32                  mClearBufferFlags;
        ColourValue             mColourValue;
        Real                    mDepthValue;
        uint32                  mStencilValue;

        /** This value is only supported by a few APIs (D3D11 at the time of writting)
            which instead of actually clearing the buffers, it informs the GPU that
            the __entire__ contents will be overwritten, which is what happens
            with fullscreen quad pass.
        @remarks
            When the API does not support discard-only clears, __the pass is ignored__
        @par
            The main purpose of this feature is to be friendly with multi-GPU systems
            (since we're telling there is no dependency with a previous frame) while
            avoid doing a full clear at the same time (wastes bandwidth)
        @par
            When performing PASS_QUAD that covers the entire render target and doesn't
            have alpha blending, a discard is automatically performed on the colour
            buffer.
        @par
            Warning: Incorrect usage of discard only clears can lead to visual artifacts
        */
        bool                    mDiscardOnly;

        /** By default clear all buffers. Stencil needs to be cleared because it hinders Fast Z Clear
            on GPU architectures that don't separate the stencil from depth and the program requested
            a Z Buffer with stencil (even if we never use it)
        */
        CompositorPassClearDef( CompositorTargetDef *parentTargetDef ) :
            CompositorPassDef( PASS_CLEAR, parentTargetDef ),
            mClearBufferFlags( FBT_COLOUR|FBT_DEPTH|FBT_STENCIL ),
            mColourValue( ColourValue::Black ),
            mDepthValue( 1.0f ),
            mStencilValue( 0 ),
            mDiscardOnly( false )
        {
            //Override so that it only gets executed on the first execution on the
            //whole screen (i.e. clear the whole viewport during the left eye pass)
            mExecutionMask          = 0x01;
            mViewportModifierMask   = 0x00;
        }
    };

    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif
