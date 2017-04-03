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

#ifndef _OgreCompositorPassDepthCopyDef_H_
#define _OgreCompositorPassDepthCopyDef_H_

#include "OgreHeaderPrefix.h"

#include "../OgreCompositorPassDef.h"
#include "OgreCommon.h"

namespace Ogre
{
    class CompositorNodeDef;

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Effects
    *  @{
    */

    class _OgreExport CompositorPassDepthCopyDef : public CompositorPassDef
    {
        friend class CompositorPassDepthCopy;
    public:
        /// DX10 HW (or the API) doesn't support copying depth buffers. Which means
        /// Whatever composition you're trying to do will fail to render correctly.
        /// However, most of the time this pass is used to prevent the depth buffer
        /// from decompressing; but you could still get away without the pass, i.e.
        /// as if it never existed (and use just one depth texture).
        /// If this is your case, when mAliasDepthBufferOnCopyFailure = true and the
        /// copy failed, the destination texture will have its own depth buffer
        /// detached, and will be attached to the same depth buffer as the source
        /// texture (effectively aliasing both textures to the same depth buffer).
        bool        mAliasDepthBufferOnCopyFailure;

    protected:
        /// Name of the src RT (can come from input channel, local textures, or global ones)
        IdString    mSrcDepthTextureName;
        /// Name of the dst RT (can come from input channel, local textures, or global ones)
        IdString    mDstDepthTextureName;
        CompositorNodeDef   *mParentNodeDef;

    public:

        CompositorPassDepthCopyDef( CompositorNodeDef *parentNodeDef,
                                    CompositorTargetDef *parentTargetDef ) :
            CompositorPassDef( PASS_DEPTHCOPY, parentTargetDef ),
            mAliasDepthBufferOnCopyFailure( false ),
            mParentNodeDef( parentNodeDef )
        {
        }

        /** Indicates the pass to change the texture units to use the specified texture sources.
            @See QuadTextureSource for params
        */
        void setDepthTextureCopy( const String &srcTextureName, const String &dstTextureName );
    };

    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif
