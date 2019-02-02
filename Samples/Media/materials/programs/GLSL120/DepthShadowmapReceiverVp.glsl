#version 120

uniform mat4 world;
uniform mat4 worldIT;
uniform mat4 worldViewProj;
uniform mat4 texViewProj;
uniform vec4 lightPosition;
uniform vec4 lightColour;

attribute vec4 vertex;
attribute vec3 normal;

varying	vec4 oUv;
varying	vec4 outColor;

void main()
{
	gl_Position = worldViewProj * vertex;
	
	vec4 worldPos = world * vertex;

	vec3 worldNorm = (worldIT * vec4(normal, 1.0)).xyz;

	// calculate lighting (simple vertex lighting)
	vec3 lightDir = normalize(
		lightPosition.xyz - (worldPos.xyz * lightPosition.w));

	outColor = lightColour * max(dot(lightDir, worldNorm), 0.0);

	// calculate shadow map coords
	oUv = texViewProj * worldPos;
}

