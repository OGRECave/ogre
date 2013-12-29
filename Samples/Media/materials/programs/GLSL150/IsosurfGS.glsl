#version 150

// Ogre port of Nvidia's IsoSurf.cg file
// Modified code follows. See http://developer.download.nvidia.com/SDK/10/opengl/samples.html for original
//
// Cg port of Yury Uralsky's metaball FX shader
//
// Authors: Simon Green and Yury Urlasky
// Email: sdkfeedback@nvidia.com
//
// Copyright (c) NVIDIA Corporation. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

// Size of the sampling grid
in VertexData 
{
    vec3 N;
    vec2 Field;
} VertexIn[];

out vec3 oNormal;

uniform float IsoValue;

layout(lines_adjacency) in;
layout(triangle_strip, max_vertices = 4) out;

// Estimate where isosurface intersects grid edge with endpoints v0, v1.
void CalcIntersection(vec4 Pos0,
                      vec3 N0,
                      vec2 Field0,
                      vec4 Pos1,
                      vec3 N1,
                      vec2 Field1)
{
    float t = (IsoValue - Field0.x) / (Field1.x - Field0.x);
    if ((Field0.x < IsoValue) && (Field1.x > Field0.x))
    {
        if (t > 0 && t < 1)
        {
            gl_Position = mix(Pos0, Pos1, t);
            oNormal = mix(N0, N1, t);
            EmitVertex();
        }
    }
}

// Geometry shader
// input: line with adjacency (tetrahedron)
// outputs: zero, one or two triangles depending if isosurface intersects tetrahedron
void main()
{

    // Construct index for this tetrahedron.
    uint index = uint((uint(VertexIn[0].Field.y) << 3) |
                      (uint(VertexIn[1].Field.y) << 2) |
                      (uint(VertexIn[2].Field.y) << 1) |
                      uint(VertexIn[3].Field.y));

    // Don't bother if all vertices out or all vertices inside isosurface.
    if (index > uint(0) && index < uint(15))
    {
        // Uber-compressed version of the edge table.
        uint edgeListHex[8] = 
            uint[8](uint(0x0001cde0), uint(0x98b08c9d), uint(0x674046ce), uint(0x487bc480), 
                    uint(0x21301d2e), uint(0x139bd910), uint(0x26376e20), uint(0x3b700000));

        uint edgeValFull = edgeListHex[index/uint(2)];
        uint three = uint(0x3);
        uint edgeVal = (index % uint(2) == uint(1)) ? (edgeValFull & uint(0xFFFF)) : ((edgeValFull >> 16) & uint(0xFFFF));
        ivec4 e0 = ivec4((edgeVal >> 14) & three, (edgeVal >> 12) & three, (edgeVal >> 10) & three, (edgeVal >> 8) & three);
        ivec4 e1 = ivec4((edgeVal >> 6) & three, (edgeVal >> 4) & three, (edgeVal >> 2) & three, (edgeVal >> 0) & three);

        CalcIntersection(gl_in[e0.x].gl_Position, VertexIn[e0.x].N, VertexIn[e0.x].Field,
                         gl_in[e0.y].gl_Position, VertexIn[e0.y].N, VertexIn[e0.y].Field);
        CalcIntersection(gl_in[e0.z].gl_Position, VertexIn[e0.z].N, VertexIn[e0.z].Field,
                         gl_in[e0.w].gl_Position, VertexIn[e0.w].N, VertexIn[e0.w].Field);
        CalcIntersection(gl_in[e1.x].gl_Position, VertexIn[e1.x].N, VertexIn[e1.x].Field,
                         gl_in[e1.y].gl_Position, VertexIn[e1.y].N, VertexIn[e1.y].Field);

        // Emit additional triangle, if necessary.
        if (e1.z != -1) {
             CalcIntersection(gl_in[e1.z].gl_Position, VertexIn[e1.z].N, VertexIn[e1.z].Field,
                              gl_in[e1.w].gl_Position, VertexIn[e1.w].N, VertexIn[e1.w].Field);
         }

         EndPrimitive();
     }
}
