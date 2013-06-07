#version 100
precision highp int;
precision highp float;

uniform mat4 world;
uniform mat4 worldIT;
uniform mat4 worldViewProj;
uniform mat4 texViewProj;
uniform vec4 lightPosition;
uniform vec4 lightColour;
#if LINEAR_RANGE
uniform vec4 shadowDepthRange;
#endif

attribute vec4 position;
attribute vec3 normal;

varying vec4 shadowUV;
varying vec4 vColour;

void main()
{
	gl_Position = worldViewProj * position;
	vec4 worldPos = world * position;
	vec3 worldNorm = (worldIT * vec4(normal, 1.0)).xyz;

vec4 lightPos = vec4(300.0, 750.0, -700.0, 1.0);
	// Calculate lighting (simple vertex lighting)
	vec3 lightDir = normalize(lightPos.xyz - (worldPos.xyz * lightPos.w));

	vColour = vec4(0.590839, 0.36056, 0.13028, 1.0) * max(dot(lightDir, worldNorm), 0.0);

	// Calculate shadow map coords
	shadowUV = texViewProj * worldPos;
#if LINEAR_RANGE
	// Adjust by fixed depth bias, rescale into range
	shadowUV.z = (shadowUV.z - shadowDepthRange.x) * shadowDepthRange.w;
	shadowUV.xy = shadowUV.xy / shadowUV.w;
#else
	shadowUV = shadowUV / shadowUV.w;
#endif
}
