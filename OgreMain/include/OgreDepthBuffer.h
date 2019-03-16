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
#ifndef __DepthBuffer_H__
#define __DepthBuffer_H__

#include "OgrePrerequisites.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup RenderSystem
    *  @{
    */

    /** An abstract class that contains a depth/stencil buffer.
        Depth Buffers can be attached to render targets. Note we handle Depth & Stencil together.
        DepthBuffer sharing is handled automatically for you. However, there are times where you want
        to specifically control depth buffers to achieve certain effects or increase performance.
        You can control this by hinting Ogre with POOL IDs. Created depth buffers can live in different
        pools, or all together in the same one.
        Usually, a depth buffer can only be attached to a RenderTarget if it's dimensions are bigger
        and have the same bit depth and same multisample settings. Depth Buffers are created automatically
        for new RTs when needed, and stored in the pool where the RenderTarget should have drawn from.
        By default, all RTs have the Id POOL_DEFAULT, which means all depth buffers are stored by default
        in that pool. By choosing a different Pool Id for a specific RenderTarget, that RT will only
        retrieve depth buffers from _that_ pool, therefore not conflicting with sharing depth buffers
        with other RTs (such as shadows maps).
        Setting an RT to POOL_MANUAL_USAGE means Ogre won't manage the DepthBuffer for you (not recommended)
        RTs with POOL_NO_DEPTH are very useful when you don't want to create a DepthBuffer for it. You can
        still manually attach a depth buffer though as internally POOL_NO_DEPTH & POOL_MANUAL_USAGE are
        handled in the same way.

        Behavior is consistent across all render systems, if, and only if, the same RSC flags are set
        RSC flags that affect this class are:
            * RSC_RTT_MAIN_DEPTHBUFFER_ATTACHABLE:
                some APIs (ie. OpenGL w/ FBO) don't allow using
                the main depth buffer for offscreen RTTs. When this flag is set, the depth buffer can be
                shared between the main window and an RTT.
            * RSC_RTT_DEPTHBUFFER_RESOLUTION_LESSEQUAL:
                When this flag isn't set, the depth buffer can only be shared across RTTs who have the EXACT
                same resolution. When it's set, it can be shared with RTTs as long as they have a
                resolution less or equal than the depth buffer's.

        @remarks
            Design discussion http://www.ogre3d.org/forums/viewtopic.php?f=4&t=53534&p=365582
        @author
            Matias N. Goldberg
        @version
            1.0
     */
    class _OgreExport DepthBuffer : public RenderSysAlloc
    {
    public:
        enum PoolId
        {
            POOL_NO_DEPTH       = 0,
            POOL_MANUAL_USAGE   = 0,
            POOL_DEFAULT        = 1
        };

        DepthBuffer( uint16 poolId, uint16 bitDepth, uint32 width, uint32 height,
                     uint32 fsaa, const String &fsaaHint, bool manual );
        virtual ~DepthBuffer();

        /** Sets the pool id in which this DepthBuffer lives.
            Note this will detach any render target from this depth buffer */
        void _setPoolId( uint16 poolId );

        /// Gets the pool id in which this DepthBuffer lives
        virtual uint16 getPoolId() const;
        virtual uint16 getBitDepth() const;
        virtual uint32 getWidth() const;
        virtual uint32 getHeight() const;
        uint32 getFSAA() const { return mFsaa; }
        const String& getFSAAHint() const { return mFsaaHint; }
        OGRE_DEPRECATED uint32 getFsaa() const { return getFSAA(); }
        OGRE_DEPRECATED const String& getFsaaHint() const { return getFSAAHint(); }

        /** Manual DepthBuffers are cleared in RenderSystem's destructor. Non-manual ones are released
            with it's render target (aka, a backbuffer or similar) */
        bool isManual() const;

        /** Returns whether the specified RenderTarget is compatible with this DepthBuffer
            That is, this DepthBuffer can be attached to that RenderTarget
            @remarks
                Most APIs impose the following restrictions:
                Width & height must be equal or higher than the render target's
                They must be of the same bit depth.
                They need to have the same FSAA setting
            @param renderTarget The render target to test against
        */
        virtual bool isCompatible( RenderTarget *renderTarget ) const;

        /** Called when a RenderTarget is attaches this DepthBuffer
            @remarks
                This function doesn't actually attach. It merely informs the DepthBuffer
                which RenderTarget did attach. The real attachment happens in
                RenderTarget::attachDepthBuffer()
            @param renderTarget The RenderTarget that has just been attached
        */
        virtual void _notifyRenderTargetAttached( RenderTarget *renderTarget );

        /** Called when a RenderTarget is detaches from this DepthBuffer
            @remarks
                Same as DepthBuffer::_notifyRenderTargetAttached()
            @param renderTarget The RenderTarget that has just been detached
        */
        virtual void _notifyRenderTargetDetached( RenderTarget *renderTarget );

    protected:
        typedef std::set<RenderTarget*> RenderTargetSet;

        uint16                      mPoolId;
        uint16                      mBitDepth;
        uint32                      mWidth;
        uint32                      mHeight;
        uint32                      mFsaa;
        String                      mFsaaHint;

        bool                        mManual; //We don't Release manual surfaces on destruction
        RenderTargetSet             mAttachedRenderTargets;

        void detachFromAllRenderTargets();
    };
}

#include "OgreHeaderSuffix.h"

#endif
