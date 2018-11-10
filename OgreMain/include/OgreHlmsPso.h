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
#ifndef _OgreHlmsPso_H_
#define _OgreHlmsPso_H_

#include "OgrePixelFormat.h"
#include "Vao/OgreVertexBufferPacked.h"
#include "OgreGpuProgram.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Resources
    *  @{
    */

    enum OperationType
    {
        /// A list of points, 1 vertex per point
        OT_POINT_LIST = 1,
        /// A list of lines, 2 vertices per line
        OT_LINE_LIST = 2,
        /// A strip of connected lines, 1 vertex per line plus 1 start vertex
        OT_LINE_STRIP = 3,
        /// A list of triangles, 3 vertices per triangle
        OT_TRIANGLE_LIST = 4,
        /// A strip of triangles, 3 vertices for the first triangle, and 1 per triangle after that
        OT_TRIANGLE_STRIP = 5,
        /// A fan of triangles, 3 vertices for the first triangle, and 1 per triangle after that
        OT_TRIANGLE_FAN = 6,
        /// Patch control point operations, used with tessellation stages
        OT_PATCH_1_CONTROL_POINT    = 7,
        OT_PATCH_2_CONTROL_POINT    = 8,
        OT_PATCH_3_CONTROL_POINT    = 9,
        OT_PATCH_4_CONTROL_POINT    = 10,
        OT_PATCH_5_CONTROL_POINT    = 11,
        OT_PATCH_6_CONTROL_POINT    = 12,
        OT_PATCH_7_CONTROL_POINT    = 13,
        OT_PATCH_8_CONTROL_POINT    = 14,
        OT_PATCH_9_CONTROL_POINT    = 15,
        OT_PATCH_10_CONTROL_POINT   = 16,
        OT_PATCH_11_CONTROL_POINT   = 17,
        OT_PATCH_12_CONTROL_POINT   = 18,
        OT_PATCH_13_CONTROL_POINT   = 19,
        OT_PATCH_14_CONTROL_POINT   = 20,
        OT_PATCH_15_CONTROL_POINT   = 21,
        OT_PATCH_16_CONTROL_POINT   = 22,
        OT_PATCH_17_CONTROL_POINT   = 23,
        OT_PATCH_18_CONTROL_POINT   = 24,
        OT_PATCH_19_CONTROL_POINT   = 25,
        OT_PATCH_20_CONTROL_POINT   = 26,
        OT_PATCH_21_CONTROL_POINT   = 27,
        OT_PATCH_22_CONTROL_POINT   = 28,
        OT_PATCH_23_CONTROL_POINT   = 29,
        OT_PATCH_24_CONTROL_POINT   = 30,
        OT_PATCH_25_CONTROL_POINT   = 31,
        OT_PATCH_26_CONTROL_POINT   = 32,
        OT_PATCH_27_CONTROL_POINT   = 33,
        OT_PATCH_28_CONTROL_POINT   = 34,
        OT_PATCH_29_CONTROL_POINT   = 35,
        OT_PATCH_30_CONTROL_POINT   = 36,
        OT_PATCH_31_CONTROL_POINT   = 37,
        OT_PATCH_32_CONTROL_POINT   = 38
    };

    /** IT'S MEMBERS MUST BE KEPT POD (Otherwise HlmsPso needs to be modified).
    @par
        Padding must be explicit! This way, copy/assignment operators will also copy the padded
        values which is needed for properly testing via memcmp.
        Use these Clang params to help you find alignment issues:
            -Xclang -fdump-record-layouts -Wpadded > dmp.txt 2>err.txt
    */
    struct HlmsPassPso
    {
        /// Stencil support
        StencilParams   stencilParams;

        bool        hwGamma[OGRE_MAX_MULTIPLE_RENDER_TARGETS];
        /// PF_NULL if no colour attachment is used.
        PixelFormat colourFormat[OGRE_MAX_MULTIPLE_RENDER_TARGETS];
        /// PF_NULL if no depth buffer is used.
        PixelFormat depthFormat;

        /// MSAA specific settings
        uint32      multisampleCount;
        uint32      multisampleQuality;

        /// For multi-GPU support
        uint32      adapterId;

        uint8 strongMacroblockBits;
        uint8 padding0[3];

        bool operator == ( const HlmsPassPso &_r ) const
        {
            //This will work correctly, because padded bytes are explicit.
            return !memcmp( this, &_r, sizeof(HlmsPassPso) );
        }
        bool operator != ( const HlmsPassPso &_r ) const
        {
            return !(*this == _r);
        }
        bool operator < ( const HlmsPassPso &_r ) const
        {
            return memcmp( this, &_r, sizeof(HlmsPassPso) ) < 0;
        }

        bool hasStrongMacroblock(void) const    { return strongMacroblockBits != 0u; }

        enum StrongMacroblockBits
        {
            ForceDisableDepthWrites     = 1u << 0u,
            InvertVertexWinding         = 1u << 1u,
        };
    };

    /** Defines a PipelineStateObject as required by Vulkan, Metal & DX12.
    @remarks
        Some RenderSystem-specific quirks:
            In OpenGL, vertex-input data (vertexElements, operationType, enablePrimitiveRestart)
            is ignored, and controlled via the VertexArrayObject pointer.

            In the other APIs, vertex-input data is use, and VertexArrayObject pointers
            only control which vertex and index buffers are bound to the device.
    @par
        Padding must be explicit! This way, copy/assignment operators will also copy the padded
        values which is needed for properly testing via memcmp.
        Use these Clang params to help you find alignment issues:
            -Xclang -fdump-record-layouts -Wpadded > dmp.txt 2>err.txt
    */
    struct HlmsPso
    {
        GpuProgramPtr   vertexShader;
        GpuProgramPtr   geometryShader;
        GpuProgramPtr   tesselationHullShader;
        GpuProgramPtr   tesselationDomainShader;
        GpuProgramPtr   pixelShader;

        //Computed from the VertexArrayObject (or v1 equivalent)
        VertexElement2VecVec    vertexElements;
        OperationType           operationType;
        bool                    enablePrimitiveRestart;
        uint8                   clipDistances; //Bitmask. Only needed by GL.
        uint8                   padding[2];

        HlmsMacroblock const    *macroblock;
        HlmsBlendblock const    *blendblock;
        //No independent blenblocks for now
//      HlmsBlendblock const    *blendblock[8];
//      bool                    independentBlend;

        //TODO: Stream Out.
        //-dark_sylinc update: Stream Out seems to be dying.
        //It was hard to setup, applications are limited,
        //UAVs are easier and more flexible/powerful.

        // --- Begin Pass data ---
        uint32          sampleMask; /// Fixed to 0xffffffff for now
        HlmsPassPso     pass;
        // --- End Pass data ---

        void        *rsData;        /// Render-System specific data

        //No constructor on purpose. Performance implications
        //(could get called every object when looking up!)
        //see Hlms::getShaderCache. Use initialize instead.
        //HlmsPso();

        void initialize()
        {
            macroblock = 0;
            blendblock = 0;
            sampleMask = 0;
            operationType = OT_POINT_LIST;
            enablePrimitiveRestart = false;
            clipDistances = 0;
            memset( padding, 0, sizeof(padding) );
            memset( &pass, 0, sizeof(HlmsPassPso) );
            rsData = 0;
        }

        bool equalNonPod( const HlmsPso &_r ) const
        {
            return this->vertexShader == _r.vertexShader &&
                   this->geometryShader == _r.geometryShader &&
                   this->tesselationHullShader == _r.tesselationHullShader &&
                   this->tesselationDomainShader == _r.tesselationDomainShader &&
                   this->pixelShader == _r.pixelShader &&
                   this->vertexElements == _r.vertexElements;
        }
        int lessNonPod( const HlmsPso &_r ) const
        {
            if( this->vertexShader < _r.vertexShader ) return -1;
            if( this->vertexShader != _r.vertexShader ) return 1;
            if( this->geometryShader < _r.geometryShader ) return -1;
            if( this->geometryShader != _r.geometryShader ) return 1;
            if( this->tesselationHullShader < _r.tesselationHullShader ) return -1;
            if( this->tesselationHullShader != _r.tesselationHullShader ) return 1;
            if( this->tesselationDomainShader < _r.tesselationDomainShader ) return -1;
            if( this->tesselationDomainShader != _r.tesselationDomainShader ) return 1;
            if( this->pixelShader < _r.pixelShader ) return -1;
            if( this->pixelShader != _r.pixelShader ) return 1;
            if( this->vertexElements < _r.vertexElements ) return -1;
            if( this->vertexElements != _r.vertexElements ) return 1;

            return 0;
        }

        /// Compares if this == _r but only accounting data that is independent of a pass
        /// (and is typically part of a renderable with a material already assigned).
        bool equalExcludePassData( const HlmsPso &_r ) const
        {
            //Non-POD datatypes.
            return
            equalNonPod( _r ) &&
            //POD datatypes
            memcmp( &this->operationType, &_r.operationType,
                    (const uint8*)&this->sampleMask -
                    (const uint8*)&this->operationType ) == 0;
        }
        /// Compares if this <= _r. See equalExcludePassData
        bool lessThanExcludePassData( const HlmsPso &_r ) const
        {
            //Non-POD datatypes.
            int nonPodResult = lessNonPod( _r );
            if( nonPodResult != 0 )
                return nonPodResult < 0;

            //POD datatypes
            return memcmp( &this->operationType, &_r.operationType,
                           (const uint8*)&this->sampleMask -
                           (const uint8*)&this->operationType ) < 0;
        }

        /* Disabled because it's not used.
        /// IMPORTANT: rsData is not considered.
        bool operator < ( const HlmsPso &_r ) const
        {
            //Non-POD datatypes.
            int nonPodResult = lessNonPod( _r );
            if( nonPodResult != 0 )
                return nonPodResult < 0;

            //POD datatypes
            return memcmp( &this->operationType, &_r.operationType,
                           (uint8*)&this->rsData -
                           (uint8*)&this->operationType ) < 0;
        }
        /// IMPORTANT: rsData is not considered.
        bool operator == ( const HlmsPso &_r ) const
        {
            //Non-POD datatypes.
            return
            equalNonPod( _r ) &&
            //POD datatypes
            memcmp( &this->operationType, &_r.operationType,
                    (uint8*)&this->rsData -
                    (uint8*)&this->operationType ) == 0;
        }
        /// IMPORTANT: rsData is not considered.
        bool operator != ( const HlmsPso &_r ) const
        {
            //Non-POD datatypes.
            return  !(*this == _r);
        }*/
    };

    struct HlmsComputePso
    {
        GpuProgramPtr   computeShader;
        GpuProgramParametersSharedPtr computeParams;

        /// XYZ. Metal needs the threads per group on C++ side. HLSL & GLSL want
        /// the thread count on shader side, thus we allow users to tell us
        /// the thread count to C++, and send it to the shaders via
        /// @value( threads_per_group_x ); OR let the shader tell C++ the threadcount
        /// via @pset( threads_per_group_x, 64 )
        /// (there's also threads_per_group_y & threads_per_group_z)
        /// @see HlmsComputeJob::setThreadsPerGroup
        uint32  mThreadsPerGroup[3];

        /// The number of thread groups to dispatch.
        /// eg. Typically:
        ///     mNumThreadGroups[0] = ceil( mThreadsPerGroup[0] / image.width );
        ///     mNumThreadGroups[1] = ceil( mThreadsPerGroup[1] / image.height );
        /// @see HlmsComputeJob::setNumThreadGroups
        uint32  mNumThreadGroups[3];

        void        *rsData;    /// Render-System specific data

        //No constructor on purpose. Performance implications
        //(could get called every object when looking up!)
        //see Hlms::getShaderCache
        //HlmsComputePso();

        void initialize()
        {
            memset( mThreadsPerGroup, 0, sizeof(mThreadsPerGroup) );
            memset( mNumThreadGroups, 0, sizeof(mNumThreadGroups) );
            rsData = 0;
        }
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
