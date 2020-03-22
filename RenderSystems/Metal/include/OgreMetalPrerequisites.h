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
    struct MetalHlmsPso;
    class MetalProgram;
    class MetalProgramFactory;
    class MetalStagingBuffer;
    class MetalRenderSystem;
    class MetalRenderTargetCommon;
    class MetalVaoManager;

    // forward compatibility defines
    class MetalHardwareBufferCommon;
    class MetalHardwareIndexBuffer;
    class MetalHardwareVertexBuffer;
    class MetalTexBufferPacked;
    class MetalMultiSourceVertexBufferPool;
    class MetalConstBufferPacked;

    struct HlmsPso;
    struct BufferPacked;
    typedef int StencilParams;
    typedef void UavBufferPacked;

    typedef MetalVaoManager VaoManager;
    typedef MetalStagingBuffer StagingBuffer;
    typedef MetalTexBufferPacked TexBufferPacked;
    typedef MetalMultiSourceVertexBufferPool MultiSourceVertexBufferPool;
    typedef void IndexBufferPacked;
    typedef void VertexBufferPacked;
    typedef MetalConstBufferPacked ConstBufferPacked;

    class Sampler;
    typedef Sampler HlmsSamplerblock;

    typedef RenderOperation::OperationType OperationType;

    enum BufferType
    {
        /// Read access from GPU.
        /// i.e. Textures, most meshes.
        BT_IMMUTABLE,

        /** Read and write access from GPU. No access for CPU at all.
            i.e. RenderTextures, vertex buffers that will be used for R2VB
        @remarks
            Ogre will use staging buffers to upload and download contents,
            but don't do this frequently (except where needed,
            i.e. live video capture)
        */
        BT_DEFAULT,

        /// Read access from GPU. Write access for CPU.
        /// i.e. Particles, dynamic textures. Dynamic buffers don't put a
        /// hidden buffer behind the scenes. You get what you ask, therefore it's
        /// your responsability to ensure you don't lock a region that is currently
        /// in use by the GPU (or else stall).
        BT_DYNAMIC_DEFAULT,

        /// Same as BT_DYNAMIC, but mapping will be persistent.
        BT_DYNAMIC_PERSISTENT,

        /// Same as BT_DYNAMIC_PERSISTENT, but mapping will be persistent and cache coherent.
        BT_DYNAMIC_PERSISTENT_COHERENT,
    };

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

    struct VertexElement2
    {
        /// The type of element
        VertexElementType mType;
        /// The meaning of the element
        VertexElementSemantic mSemantic;

        /// The number of instances to draw using the same per-instance data before
        /// advancing in the buffer by one element. This value must be 0 for an
        /// element that contains per-vertex data
        uint32 mInstancingStepRate;

        VertexElement2( VertexElementType type, VertexElementSemantic semantic ) :
            mType( type ), mSemantic( semantic ), mInstancingStepRate( 0 ) {}

        bool operator == ( const VertexElement2 _r ) const
        {
            return mType == _r.mType && mSemantic == _r.mSemantic &&
                    mInstancingStepRate == _r.mInstancingStepRate;
        }

        bool operator == ( VertexElementSemantic semantic ) const
        {
            return mSemantic == semantic;
        }

        /// Warning: Beware a VertexElement2Vec shouldn't be sorted.
        /// The order in which they're pushed into the vector defines the offsets
        /// in the vertex buffer. Altering the order would cause the GPU to
        /// to read garbage when trying to interpret the vertex buffer
        /// This operator exists because it's useful when implementing
        /// a '<' operator in other structs that contain VertexElement2Vecs
        /// (see HlmsPso)
        bool operator < ( const VertexElement2 &_r ) const
        {
            if( this->mType < _r.mType ) return true;
            if( this->mType > _r.mType ) return false;

            if( this->mSemantic < _r.mSemantic ) return true;
            if( this->mSemantic > _r.mSemantic ) return false;

            return this->mInstancingStepRate < _r.mInstancingStepRate;
        }
    };

    typedef std::vector<VertexElement2> VertexElement2Vec;
    typedef std::vector<VertexElement2Vec> VertexElement2VecVec;
}

#if defined ( OGRE_GCC_VISIBILITY )
#    define _OgreMetalExport  __attribute__ ((visibility("default")))
#else
#    define _OgreMetalExport
#endif

#endif //#ifndef _OgreMetalPrerequisites_H_
