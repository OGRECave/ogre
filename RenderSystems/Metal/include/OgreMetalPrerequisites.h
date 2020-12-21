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

#define OGRE_METAL_CONST_SLOT_START     16u
#define OGRE_METAL_TEX_SLOT_START       24u
#define OGRE_METAL_PARAMETER_SLOT       23u
#define OGRE_METAL_UAV_SLOT_START       28u

#define OGRE_METAL_CS_PARAMETER_SLOT    8u
#define OGRE_METAL_CS_CONST_SLOT_START  0u
#define OGRE_METAL_CS_UAV_SLOT_START    8u
#define OGRE_METAL_CS_TEX_SLOT_START    16u

#define RESTRICT_ALIAS __restrict__
#define RESTRICT_ALIAS_RETURN

namespace Ogre
{
    // Forward declarations
    class MetalDepthBuffer;
    struct MetalDevice;
    class MetalDiscardBuffer;
    class MetalDiscardBufferManager;
    class MetalDynamicBuffer;
    class MetalProgram;
    class MetalProgramFactory;
    class MetalStagingBuffer;
    class MetalRenderSystem;
    class MetalRenderTargetCommon;

    // forward compatibility defines
    class MetalHardwareBufferCommon;
    struct BufferPacked;

    typedef MetalStagingBuffer StagingBuffer;

    enum StagingStallType
    {
        /// Next map will not stall.
        STALL_NONE,

        /// Next map call will cause a stall. We can't predict how long, but
        /// on average should be small. You should consider doing something
        /// else then try again.
        STALL_PARTIAL,

        /// The whole pipeline is brought to a stop. We have to wait for the GPU
        /// to finish all operations issued so far. This can be very expensive.
        /// Grab a different StagingBuffer.
        STALL_FULL,

        NUM_STALL_TYPES
    };

    enum MappingState
    {
        MS_UNMAPPED,
        MS_MAPPED,
        NUM_MAPPING_STATE
    };

    enum UnmapOptions
    {
        /// Unmaps all types of mapping, including persistent buffers.
        UO_UNMAP_ALL,

        /// When unmapping, unmap() will keep persistent buffers mapped.
        /// Further calls to map will only do some error checking
        UO_KEEP_PERSISTENT
    };


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

    /** Used for efficient removal in std::vector and std::deque (like an std::list)
        However it assumes the order of elements in container is not important or
        something external to the container holds the index of an element in it
        (but still should be kept deterministically across machines)
        Basically it swaps the iterator with the last iterator, and pops back
        Returns the next iterator
    */
    template<typename T>
    typename T::iterator efficientVectorRemove( T& container, typename T::iterator& iterator )
    {
        const size_t idx = iterator - container.begin();
        *iterator = container.back();
        container.pop_back();

        return container.begin() + idx;
    }
}

#if defined ( OGRE_GCC_VISIBILITY )
#    define _OgreMetalExport  __attribute__ ((visibility("default")))
#else
#    define _OgreMetalExport
#endif

#endif //#ifndef _OgreMetalPrerequisites_H_
