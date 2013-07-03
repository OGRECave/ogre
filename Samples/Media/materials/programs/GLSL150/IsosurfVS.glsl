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

in vec4 vertex;

out VertexData {
    vec3 N;
    vec2 Field;
    // vec4 Color;
} VertexOut;

uniform float IsoValue = 1.0;
uniform mat4 WorldViewProj;
uniform mat4 origWorldViewIT;
uniform vec4 Metaballs[2] = vec4[](vec4( -0.5, 0, 0, 0.2 ), vec4( 0.6, 0, 0, 0.1 ));

// Size of the sampling grid
const ivec3 SizeMask = ivec3( 63, 63, 63 );
const ivec3 SizeShift = ivec3( 0, 6, 12 );

// Metaball function
// Returns metaball function value in .w and its gradient in .xyz
vec4 Metaball(vec3 Pos, vec3 Center, float RadiusSq)
{
    const float epsilon = 0.001;

    vec3 Dist = Pos - Center;
    float InvDistSq = 1 / (dot(Dist, Dist) + epsilon);

    vec4 o;
    o.xyz = -2 * RadiusSq * InvDistSq * InvDistSq * Dist;
    o.w = RadiusSq * InvDistSq;
    return o;
}

void main()
{
    vec4 Pos;

    // Generate sampling point position based on its index
    // Pos.x = float((gl_VertexID >> SizeShift.x) & SizeMask.x) / (SizeMask.x + 1);
    // Pos.y = float((gl_VertexID >> SizeShift.y) & SizeMask.y) / (SizeMask.y + 1);
    // Pos.z = float((gl_VertexID >> SizeShift.z) & SizeMask.z) / (SizeMask.z + 1);
    // Pos = Pos*2 - 1;
    Pos = vertex;
    
    // Sum up contributions from all metaballs
    vec4 Field = vec4(0,0,0,0);
    for (int i = 0; i < 2; i++)
        Field += Metaball(Pos.xyz, Metaballs[i].xyz, Metaballs[i].w);

    mat3 WorldViewIT = mat3(origWorldViewIT[0].xyz, origWorldViewIT[1].xyz, origWorldViewIT[2].xyz);
    
    // Transform position and normals
    gl_Position = WorldViewProj * vec4(Pos.xyz, 1);
    VertexOut.N = WorldViewIT * Field.xyz;        // we want normals in world space
    VertexOut.Field.x = Field.w;

    // Generate in-out flags
    VertexOut.Field.y = (Field.w < IsoValue) ? 1 : 0;

    // VertexOut.Color = (Field*0.5+0.5) * (Field.w / 10.0);
}
