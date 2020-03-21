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

#define OGRE_METAL_CONST_SLOT_START     16u
#define OGRE_METAL_TEX_SLOT_START       24u
#define OGRE_METAL_PARAMETER_SLOT       23u
#define OGRE_METAL_UAV_SLOT_START       28u

#define OGRE_METAL_CS_PARAMETER_SLOT    8u
#define OGRE_METAL_CS_CONST_SLOT_START  0u
#define OGRE_METAL_CS_UAV_SLOT_START    8u
#define OGRE_METAL_CS_TEX_SLOT_START    16u

namespace Ogre
{
    // Forward declarations
    class MetalDepthBuffer;
    struct MetalDevice;
    class MetalDiscardBuffer;
    class MetalDiscardBufferManager;
    class MetalDynamicBuffer;
    class MetalGpuProgramManager;
    struct MetalHlmsPso;
    class MetalProgram;
    class MetalProgramFactory;
    class MetalStagingBuffer;
    class MetalRenderSystem;
    class MetalRenderTargetCommon;
    class MetalVaoManager;

    namespace v1
    {
        class MetalHardwareBufferCommon;
        class MetalHardwareIndexBuffer;
        class MetalHardwareVertexBuffer;
    }
}

#if defined ( OGRE_GCC_VISIBILITY )
#    define _OgreMetalExport  __attribute__ ((visibility("default")))
#else
#    define _OgreMetalExport
#endif

#endif //#ifndef _OgreMetalPrerequisites_H_
