#version 100
precision highp int;
precision highp float;

attribute vec3 tangent;
attribute vec4 position;
attribute vec3 normal;

uniform mat4 world;
uniform mat4 worldViewProj;
uniform mat4 texViewProj;
uniform vec4 lightPosition; // object space
uniform vec4 shadowDepthRange;

varying vec3 tangentLightDir;
varying vec4 vUv0;
varying vec4 vUv1;

attribute vec4 uv0;
attribute vec4 uv1;

void main()
{
	gl_Position = worldViewProj * position;
	
	vec4 worldPos = world * position;
vec4 lightPos = vec4(300.0, 750.0, -700.0, 1.0);

	// Get object space light direction 
    vec3 lightDir = normalize(lightPos.xyz -  (position.xyz * lightPos.w));

	// calculate shadow map coords
	vUv0 = texViewProj * worldPos;
#if LINEAR_RANGE
	// adjust by fixed depth bias, rescale into range
	vUv0.z = (uv0.z - shadowDepthRange.x) * shadowDepthRange.w;
#endif

	vUv1 = uv1;

	// Calculate the binormal (NB we assume both normal and tangent are 
	// already normalised) 
	vec3 binormal = cross(normal, tangent); 

	// Form a rotation matrix out of the vectors, column major for glsl es 
	mat3 rotation = mat3(vec3(tangent[0], binormal[0], normal[0]),
						vec3(tangent[1], binormal[1], normal[1]),
						vec3(tangent[2], binormal[2], normal[2]));
    
	// Transform the light vector according to this matrix 
	tangentLightDir = normalize(rotation * lightDir); 
}
