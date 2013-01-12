#version 100

precision mediump int;
precision mediump float;

uniform vec4 lightPosition; // object space 
uniform vec4 lightPosition1; // object space 
uniform vec4 eyePosition;   // object space 
uniform vec4 spotDirection; // object space
uniform vec4 spotDirection1; // object space
uniform mat4 worldViewProj; // not actually used but here for compat with HLSL
uniform mat4 worldMatrix;
uniform mat4 texViewProj1;
uniform mat4 texViewProj2;

varying vec3 tangentEyeDir;
varying vec3 tangentLightDir[2];
varying vec3 tangentSpotDir[2];
varying vec4 shadowUV[2];
varying vec4 oUv0;

attribute vec3 tangent;
attribute vec4 position;
attribute vec3 normal;
attribute vec4 uv0;

void main()
{
	gl_Position = worldViewProj * position;

	vec4 worldPos = worldMatrix * position;

    oUv0 = uv0;

	shadowUV[0] = texViewProj1 * worldPos;
	shadowUV[1] = texViewProj2 * worldPos;

	// calculate tangent space light vector 
	// Get object space light direction 
    vec3 lightDir = normalize(lightPosition.xyz -  (position.xyz * lightPosition.w));
	vec3 lightDir1 = normalize(lightPosition1.xyz -  (position.xyz * lightPosition1.w));
	
	vec3 eyeDir = (eyePosition - position).xyz; 

	// Calculate the binormal (NB we assume both normal and tangent are 
	// already normalised) 
	vec3 binormal = cross(normal, tangent); 

	// Form a rotation matrix out of the vectors, column major for glsl es 
	mat3 rotation = mat3(vec3(tangent[0], binormal[0], normal[0]),
						vec3(tangent[1], binormal[1], normal[1]),
						vec3(tangent[2], binormal[2], normal[2]));
    
	// Transform the light vector according to this matrix 
	tangentLightDir[0] = normalize(rotation * lightDir); 
	tangentLightDir[1] = normalize(rotation * lightDir1); 
	// Invert the Y on the eye dir since we'll be using this to alter UVs and
	// GL has Y inverted
	tangentEyeDir = normalize(rotation * eyeDir) * vec3(1.0, -1.0, 1.0); 

	tangentSpotDir[0] = normalize(rotation * -spotDirection.xyz);
	tangentSpotDir[1] = normalize(rotation * -spotDirection1.xyz);	
}
