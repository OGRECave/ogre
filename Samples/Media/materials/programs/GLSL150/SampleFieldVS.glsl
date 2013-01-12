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

uniform float IsoValue = 1.0;
uniform vec4 Metaballs[2];
uniform mat4 WorldViewProj;
uniform mat4 origWorldViewIT;

in vec4 vertex;

out VertexData {
    vec3 N;
    vec2 Field;
} VertexOut;

// Metaball function
// Returns metaball function value in .w and its gradient in .xyz

vec4 Metaball(vec3 Pos, vec3 Center, float RadiusSq)
{
	float epsilon = 0.001;

	vec3 Dist = Pos - Center;
	float InvDistSq = 1 / (dot(Dist, Dist) + epsilon);

	vec4 o;
	o.xyz = -2 * RadiusSq * InvDistSq * InvDistSq * Dist;
	o.w = RadiusSq * InvDistSq;
	return o;
}

void main()
{
	// Sum up contributions from all metaballs
	vec4 Field;
	for (int i = 0; i<2; i++) 
	{
		Field += Metaball(vertex.xyz, Metaballs[i].xyz, Metaballs[i].w);
	}
	
	mat3 WorldViewIT = mat3(origWorldViewIT[0].xyz, origWorldViewIT[1].xyz, origWorldViewIT[2].xyz);
	
	// Transform position and normals
	gl_Position = WorldViewProj * vertex;
	VertexOut.N = WorldViewIT * Field.xyz;	// we want normals in world space
	VertexOut.Field.x = Field.w;
	
	// Generate in-out flags
	VertexOut.Field.y = (Field.w < 1.0) ? 1 : 0;
}
