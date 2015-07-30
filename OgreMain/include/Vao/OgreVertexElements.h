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
#ifndef _OgreVertexElements_H_
#define _OgreVertexElements_H_

// Precompiler options
#include "OgrePrerequisites.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /// Vertex element semantics, used to identify the meaning of vertex buffer contents
    enum VertexElementSemantic
    {
        /// Position, 3 reals per vertex
        VES_POSITION = 1,
        /// Blending weights
        VES_BLEND_WEIGHTS = 2,
        /// Blending indices
        VES_BLEND_INDICES = 3,
        /// Normal, 3 reals per vertex
        VES_NORMAL = 4,
        /// Diffuse colours
        VES_DIFFUSE = 5,
        /// Specular colours
        VES_SPECULAR = 6,
        /// Texture coordinates. You can have up to 8 of these. 6 if
        /// VES_BLEND_WEIGHTS2 or VES_BLEND_INDICES2 if present.
        VES_TEXTURE_COORDINATES = 7,
        /// Binormal (Y axis if normal is Z)
        VES_BINORMAL = 8,
        /// Tangent (X axis if normal is Z)
        VES_TANGENT = 9,
        /// Second pair of blending weights (i.e. more than 4, less or equal than 8)
        VES_BLEND_WEIGHTS2 = 10,
        /// Second pair of blending indices (i.e. more than 4, less or equal than 8)
        VES_BLEND_INDICES2 = 11,
        /// The  number of VertexElementSemantic elements (note - the first value VES_POSITION is 1)
        VES_COUNT = 11,
    };

    /// Vertex element type, used to identify the base types of the vertex contents
    /// Note that all attributes must be aligned to 4 bytes, that's why VET_SHORT1
    /// (among others) doesn't exist.
    enum VertexElementType
    {
        VET_FLOAT1 = 0,
        VET_FLOAT2 = 1,
        VET_FLOAT3 = 2,
        VET_FLOAT4 = 3,
        /// alias to more specific colour type - use the current rendersystem's colour packing
        VET_COLOUR = 4,
        //VET_SHORT1 = 5,   Deprecated for being invalid
        VET_SHORT2 = 6,
        //VET_SHORT3 = 7,   Deprecated for being invalid
        VET_SHORT4 = 8,
        VET_UBYTE4 = 9,
        /// D3D style compact colour
        VET_COLOUR_ARGB = 10,
        /// GL style compact colour
        VET_COLOUR_ABGR = 11,
        VET_DOUBLE1 = 12,
        VET_DOUBLE2 = 13,
        VET_DOUBLE3 = 14,
        VET_DOUBLE4 = 15,
        VET_USHORT1_DEPRECATED = 16, // Deprecated for being invalid
        VET_USHORT2 = 17,
        VET_USHORT3_DEPRECATED = 18, // Deprecated for being invalid
        VET_USHORT4 = 19,
        VET_INT1 = 20,
        VET_INT2 = 21,
        VET_INT3 = 22,
        VET_INT4 = 23,
        VET_UINT1 = 24,
        VET_UINT2 = 25,
        VET_UINT3 = 26,
        VET_UINT4 = 27,
        VET_BYTE4 = 28, // signed bytes
        VET_BYTE4_SNORM = 29,  // signed normalized bytes
        VET_UBYTE4_NORM = 30,  // unsigned normalized bytes
        VET_SHORT2_SNORM = 31, // signed normalized shorts
        VET_SHORT4_SNORM = 32,
        VET_USHORT2_NORM = 33, // unsigned normalized shorts
        VET_USHORT4_NORM = 34,
        VET_HALF2 = 35,
        VET_HALF4 = 36
    };
}

#include "OgreHeaderSuffix.h"

#endif
