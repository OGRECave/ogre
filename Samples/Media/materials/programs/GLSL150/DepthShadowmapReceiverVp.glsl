#version 150

uniform mat4 world;
uniform mat4 worldIT;
uniform mat4 worldViewProj;
uniform mat4 texViewProj;
uniform vec4 lightPosition;
uniform vec4 lightColour;
uniform vec4 shadowDepthRange;

in vec4 position;
in vec3 normal;
in vec4 ambient;

out vec4 shadowUV;
out vec4 oColour;

void main()
{
	gl_Position = worldViewProj * position;
	vec4 worldPos = world * position;
	vec3 worldNorm = (worldIT * vec4(normal, 1)).xyz;

	// calculate lighting (simple vertex lighting)
	vec3 lightDir = normalize(
		lightPosition.xyz - (worldPos.xyz * lightPosition.w));

	oColour = lightColour * max(dot(lightDir, worldNorm), 0.0);

	// calculate shadow map coords
	shadowUV = texViewProj * worldPos;
#if LINEAR_RANGE
	// adjust by fixed depth bias, rescale into range
//	shadowUV.z = (shadowUV.z - shadowDepthRange.x) * shadowDepthRange.w;
	shadowUV.xy = shadowUV.xy / shadowUV.w;
#else
	shadowUV = shadowUV / shadowUV.w;
#endif
}
