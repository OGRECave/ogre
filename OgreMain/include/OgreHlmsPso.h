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

    struct HlmsPassPso
    {
        /// Stencil support
        StencilParams   stencilParams;

        /// PF_NULL if no colour attachment is used.
        PixelFormat colourFormat[OGRE_MAX_MULTIPLE_RENDER_TARGETS];
        /// PF_NULL if no depth buffer is used.
        PixelFormat depthFormat;

        /// MSAA specific settings
        uint32      multisampleCount;
        uint32      multisampleQuality;

        /// For multi-GPU support
        uint32      adapterId;

        bool operator == ( const HlmsPassPso &_r ) const
        {
            //Warning! For this to work correctly, the struct must have been previously
            //memset to zero (padded bytes would affect the results otherwise)
            return !memcmp( this, &_r, sizeof(HlmsPassPso) );
        }
    };

    /** Defines a PipelineStateObject as required by Vulkan, Metal & DX12.
    @remarks
        Some RenderSystem-specific quirks:
            In OpenGL, vertex-input data (vertexElements, operationType, enablePrimitiveRestart)
            is ignored, and controlled via the VertexArrayObject pointer.

            In the other APIs, vertex-input data is use, and VertexArrayObject pointers
            only control which vertex and index buffers are bound to the device.
    */
    struct HlmsPso
    {
        GpuProgramPtr   vertexShader;
        GpuProgramPtr   geometryShader;
        GpuProgramPtr   tesselationHullShader;
        GpuProgramPtr   tesselationDomainShader;
        GpuProgramPtr   pixelShader;

        HlmsMacroblock const    *macroblock;
        HlmsBlendblock const    *blendblock;
        //No independent blenblocks for now
//      HlmsBlendblock const    *blendblock[8];
//      bool                    independentBlend;
        uint32                  sampleMask; /// Fixed to 0xffffffff for now

        //Computed from the VertexArrayObject (or v1 equivalent)
        VertexElement2VecVec    vertexElements;
        OperationType           operationType;
        bool                    enablePrimitiveRestart;
        //TODO: Stream Out.

        HlmsPassPso     pass;

        void        *rsData;        /// Render-System specific data

        //No constructor on purpose. Performance implications
        //(could get called every object when looking up!)
        //see Hlms::getShaderCache
        //HlmsPso();
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
