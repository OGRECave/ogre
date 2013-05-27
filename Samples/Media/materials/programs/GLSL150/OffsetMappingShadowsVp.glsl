#version 150

uniform vec4 lightPosition; // object space 
uniform vec4 lightPosition1; // object space 
uniform vec4 eyePosition;   // object space 
uniform vec4 spotDirection; // object space
uniform vec4 spotDirection1; // object space
uniform mat4 worldViewProj; // not actually used but here for compat with HLSL
uniform mat4 worldMatrix;
uniform mat4 texViewProj1;
uniform mat4 texViewProj2;

out vec3 tangentEyeDir;
out vec3 tangentLightDir[2];
out vec3 tangentSpotDir[2];
out vec4 shadowUV[2];
out vec4 oUv0;

in vec3 tangent;
in vec4 position;
in vec3 normal;
in vec4 uv0;

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
	mat3 rotation = mat3(tangent, binormal, normal);
    
	// Transform the light vector according to this matrix 
	tangentLightDir[0] = normalize(rotation * lightDir); 
	tangentLightDir[1] = normalize(rotation * lightDir1); 
	// Invert the Y on the eye dir since we'll be using this to alter UVs and
	// GL has Y inverted
	tangentEyeDir = normalize(rotation * eyeDir) * vec3(1.0, -1.0, 1.0); 

	tangentSpotDir[0] = normalize(rotation * -spotDirection.xyz);
	tangentSpotDir[1] = normalize(rotation * -spotDirection1.xyz);	
}
