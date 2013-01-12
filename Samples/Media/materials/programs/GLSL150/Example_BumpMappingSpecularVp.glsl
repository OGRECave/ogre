#version 150

// General functions
// parameters
uniform vec4 lightPosition; // object space
uniform vec3 eyePosition;   // object space
uniform mat4 worldViewProj;

in vec4 vertex;
in vec3 normal;
in vec3 tangent;
in vec4 uv0;

out vec4 oUv0;
out vec3 oTSLightDir;
out vec3 oTSHalfAngle;

/* Vertex program which includes specular component */
void main()
{
	// calculate output position
	gl_Position = worldViewProj * vertex;

	// pass the main uvs straight through unchanged
	oUv0 = uv0;

	// calculate tangent space light vector
	// Get object space light direction
	vec3 lightDir = normalize(lightPosition.xyz - (vertex * lightPosition.w).xyz);

	// Calculate the binormal (NB we assume both normal and tangent are
	// already normalised)
	vec3 binormal = cross(normal, tangent);
	
	// Form a rotation matrix out of the vectors
	mat3 rotation = mat3(vec3(tangent[0], binormal[0], normal[0]),
						vec3(tangent[1], binormal[1], normal[1]),
						vec3(tangent[2], binormal[2], normal[2]));
	
	// Transform the light vector according to this matrix
	oTSLightDir = rotation * lightDir;

	// Calculate half-angle in tangent space
	vec3 eyeDir = normalize(eyePosition - vertex.xyz);
	vec3 halfAngle = normalize(eyeDir + lightDir);
	oTSHalfAngle = rotation * halfAngle;
}
