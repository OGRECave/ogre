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

#ifndef _OgreResourceTransition_H_
#define _OgreResourceTransition_H_

#include "OgrePrerequisites.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Resources
    *  @{
    */

    namespace ResourceLayout
    {
    enum Layout
    {
        Undefined,
        Texture,
        TextureDepth,
        RenderTarget,
        RenderDepth,
        Clear,
        Uav,
        CopySrc,
        CopyDst,

        NumResourceLayouts
    };
    }

    namespace WriteBarrier
    {
    enum WriteBarrier
    {
        /// Notifies we've been writing from CPU to resource.
        CpuWrite        = 0x00000001,
        /// Waits until writes from shaders to resource are finished.
        Uav             = 0x00000002,
        /// Waits until rendering to colour buffers are finished.
        RenderTarget    = 0x00000004,
        /// Waits until rendering to depth/stencil buffers are finished.
        DepthStencil    = 0x00000008,

        /// Full write barrier
        All             = 0xffffffff
    };
    }

    namespace ReadBarrier
    {
    enum ReadBarrier
    {
        /// Finishes writing to & flushes all caches to VRAM so CPU can read it
        CpuRead         = 0x00000001,
        /// After the barrier, data can be used as an indirect buffer
        Indirect        = 0x00000002,
        /// After the barrier, data can be used as a vertex buffer
        VertexBuffer    = 0x00000004,
        /// After the barrier, data can be used as an index buffer
        IndexBuffer     = 0x00000008,
        /// After the barrier, data can be used as a const buffer
        ConstBuffer     = 0x00000010,
        /// After the barrier, data can be used as a texture or as a tex. buffer
        Texture         = 0x00000020,
        /// After the barrier, data can be used as an UAV
        Uav             = 0x00000040,
        /// After the barrier, data can be used as a RenderTarget
        RenderTarget    = 0x00000080,
        /// After the barrier, data can be used as a Depth/Stencil buffer
        DepthStencil    = 0x00000100,

        All             = 0xffffffff
    };
    }

    namespace ResourceAccess
    {
    /// Enum identifying the texture access privilege
    enum ResourceAccess
    {
        Undefined = 0x00,
        Read = 0x01,
        Write = 0x10,
        ReadWrite = Read | Write
    };

    const char* toString( ResourceAccess value );
    }

    struct ResourceTransition
    {
        //Resource *resource; //TODO: We'll get here when D3D12/Vulkan is finished
        ResourceLayout::Layout      oldLayout;
        ResourceLayout::Layout      newLayout;

        uint32 writeBarrierBits;    /// @see WriteBarrier::WriteBarrier
        uint32 readBarrierBits;     /// @see ReadBarrier::ReadBarrier

        void    *mRsData;       /// Render-System specific data
    };

    struct GpuResource
    {
    };

    typedef map<GpuResource*, ResourceLayout::Layout>::type ResourceLayoutMap;
    typedef map<GpuResource*, ResourceAccess::ResourceAccess>::type ResourceAccessMap;

    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif
