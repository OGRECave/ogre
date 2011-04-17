#version 100

precision mediump int;
precision mediump float;

float shadowPCF(sampler2D shadowMap, vec4 shadowMapPos, vec2 offset)
{
	shadowMapPos = shadowMapPos / shadowMapPos.w;
	vec2 uv = shadowMapPos.xy;
	vec3 o = vec3(offset, -offset.x) * 0.3;

	// Note: We using 2x2 PCF. Good enough and is a lot faster.
	float c =	(shadowMapPos.z <= texture2D(shadowMap, uv.xy - o.xy).r) ? 1.0 : 0.0; // top left
	c +=		(shadowMapPos.z <= texture2D(shadowMap, uv.xy + o.xy).r) ? 1.0 : 0.0; // bottom right
	c +=		(shadowMapPos.z <= texture2D(shadowMap, uv.xy + o.zy).r) ? 1.0 : 0.0; // bottom left
	c +=		(shadowMapPos.z <= texture2D(shadowMap, uv.xy - o.zy).r) ? 1.0 : 0.0; // top right

	return c / 4.0;
}

uniform vec4 lightPosition;				// object space
uniform vec3 eyePosition;					// object space
uniform mat4 worldViewProjMatrix;
uniform mat4 texWorldViewProjMatrix0;
uniform mat4 texWorldViewProjMatrix1;
uniform mat4 texWorldViewProjMatrix2;

varying vec4 oUv0;
varying vec3 oLightDir;
varying vec3 oHalfAngle;
varying vec4 oLightPosition0;
varying vec4 oLightPosition1;
varying vec4 oLightPosition2;
varying vec3 oNormal;

attribute vec4 position;
attribute vec3 normal;
attribute vec4 uv0;

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
