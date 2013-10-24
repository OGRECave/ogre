#version 120

uniform mat4 world;
uniform mat4 worldViewProj;
uniform mat4 texViewProj;
uniform vec4 lightPosition; // object space

attribute vec4 vertex;
attribute vec3 normal;
attribute vec3 tangent;
attribute vec4 uv0;

varying vec3 tangentLightDir;
varying	vec4 oUv;
varying	vec2 oUv2;

void main()
{
	vec4 worldPos = world * vertex;

	// Get object space light direction 
    vec3 lightDir = normalize(lightPosition.xyz -  (vertex.xyz * lightPosition.w));

	// calculate shadow map coords
	oUv = texViewProj * worldPos;

	// pass the main uvs straight through unchanged 
	oUv2 = uv0.xy;

	// Calculate the binormal (NB we assume both normal and tangent are 
	// already normalised) 
	vec3 binormal = cross(normal, tangent); 

	// Form a rotation matrix out of the vectors 
	mat3 rotation = mat3(tangent, binormal, normal); 
    
	// Transform the light vector according to this matrix 
	tangentLightDir = normalize(rotation * lightDir); 
	
	gl_Position = worldViewProj * vertex;
}

