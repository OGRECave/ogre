#version 100

precision mediump int;
precision mediump float;

uniform vec4 lightPosition; // object space 
uniform vec3 eyePosition;   // object space 
uniform mat4 worldViewProj; // not actually used but here for compat with HLSL

varying vec3 oEyeDir;
varying vec3 oLightDir;
varying vec3 oHalfAngle;
varying vec4 oUv0;

attribute vec3 normal;
attribute vec3 tangent;
attribute vec4 uv0;
attribute vec4 position;

/* Vertex program that moves light and eye vectors into texture tangent space at vertex */ 

void main()
{
    // Calculate output position 
	gl_Position = worldViewProj * position;

    // Pass the main uvs straight through unchanged 
    oUv0 = uv0;

    vec3 lightDir = lightPosition.xyz - (position.xyz * lightPosition.w);

	vec3 eyeDir = eyePosition - position.xyz; 
    
	// Calculate the binormal (NB we assume both normal and tangent are
	// already normalised)
	// NB looks like nvidia cross params are BACKWARDS to what you'd expect
	// this equates to NxT, not TxN
	vec3 localbinormal = cross(tangent, normal);

	// Form a rotation matrix out of the vectors, column major for glsl es 
	mat3 TBN = mat3(vec3(tangent[0], localbinormal[0], normal[0]),
						vec3(tangent[1], localbinormal[1], normal[1]),
						vec3(tangent[2], localbinormal[2], normal[2]));
    
    // Transform the light vector according to this matrix 
	oLightDir = normalize(TBN * lightDir); 
	oEyeDir = normalize(TBN * eyeDir); 
    oHalfAngle = normalize(oEyeDir + oLightDir);
}

