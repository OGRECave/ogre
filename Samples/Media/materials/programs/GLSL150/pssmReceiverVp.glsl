#version 150

uniform vec4 lightPosition;				// object space
uniform vec3 eyePosition;					// object space
uniform mat4 worldViewProjMatrix;
uniform mat4 texWorldViewProjMatrix0;
uniform mat4 texWorldViewProjMatrix1;
uniform mat4 texWorldViewProjMatrix2;

out vec4 oUv0;
out vec3 oLightDir;
out vec3 oHalfAngle;
out vec4 oLightPosition0;
out vec4 oLightPosition1;
out vec4 oLightPosition2;
out vec3 oNormal;

in vec4 position;
in vec3 normal;
in vec4 uv0;

void main()
{
	// Calculate output position
	gl_Position = worldViewProjMatrix * position;

	// Pass the main uvs straight through unchanged
	oUv0.xy = uv0.xy;
	oUv0.z = gl_Position.z;

	// Calculate tangent space light vector
	// Get object space light direction
	oLightDir = normalize(lightPosition.xyz - (position * lightPosition.w).xyz);

	// Calculate half-angle in tangent space
	vec3 eyeDir = normalize(eyePosition - position.xyz);
	oHalfAngle = normalize(eyeDir + oLightDir);	

	// Calculate the position of vertex in light space
	oLightPosition0 = texWorldViewProjMatrix0 * position;
	oLightPosition1 = texWorldViewProjMatrix1 * position;
	oLightPosition2 = texWorldViewProjMatrix2 * position;

	oNormal = normal;
}
