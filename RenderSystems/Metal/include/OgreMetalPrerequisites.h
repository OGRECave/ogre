/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2016 Torus Knot Software Ltd

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
#ifndef _OgreMetalPrerequisites_H_
#define _OgreMetalPrerequisites_H_

#include "OgrePrerequisites.h"
#include "OgreLogManager.h"

#include "OgreHardwareVertexBuffer.h"
#include "OgreRenderOperation.h"
#include "OgrePixelFormat.h"

#ifdef __OBJC__
    @protocol MTLBlitCommandEncoder;
    @protocol MTLBuffer;
    @protocol MTLCommandBuffer;
    @protocol MTLCommandQueue;
    @protocol MTLComputeCommandEncoder;
    @protocol MTLComputePipelineState;
    @protocol MTLDepthStencilState;
    @protocol MTLDevice;
    @protocol MTLRenderCommandEncoder;
    @protocol MTLTexture;

    @class MTLRenderPassColorAttachmentDescriptor;
    @class MTLRenderPassDepthAttachmentDescriptor;
    @class MTLRenderPassStencilAttachmentDescriptor;
#endif

#define RESTRICT_ALIAS __restrict__
#define RESTRICT_ALIAS_RETURN

namespace Ogre
{
    // Forward declarations
    class MetalDepthBuffer;
    struct MetalDevice;
    class MetalProgram;
    class MetalProgramFactory;
    class MetalRenderSystem;
    class MetalRenderTargetCommon;

    // forward compatibility defines
    class MetalHardwareBufferCommon;

    /// Aligns the input 'offset' to the next multiple of 'alignment'.
    /// Alignment can be any value except 0. Some examples:
    ///
    /// alignToNextMultiple( 0, 4 ) = 0;
    /// alignToNextMultiple( 1, 4 ) = 4;
    /// alignToNextMultiple( 2, 4 ) = 4;
    /// alignToNextMultiple( 3, 4 ) = 4;
    /// alignToNextMultiple( 4, 4 ) = 4;
    /// alignToNextMultiple( 5, 4 ) = 8;
    ///
    /// alignToNextMultiple( 0, 3 ) = 0;
    /// alignToNextMultiple( 1, 3 ) = 3;
    inline size_t alignToNextMultiple( size_t offset, size_t alignment )
    {
        return ( (offset + alignment - 1u) / alignment ) * alignment;
    }
}

#if defined ( OGRE_GCC_VISIBILITY )
#    define _OgreMetalExport  __attribute__ ((visibility("default")))
#else
#    define _OgreMetalExport
#endif

#endif //#ifndef _OgreMetalPrerequisites_H_
