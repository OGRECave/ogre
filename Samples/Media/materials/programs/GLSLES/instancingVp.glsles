#version 100

precision highp int;
precision highp float;

uniform mat4 worldMatrixArray[80];
uniform mat4 viewProjectionMatrix;
uniform vec4 lightPos;
uniform vec4 ambient;
uniform vec4 lightDiffuseColour;

attribute vec4 position;
attribute vec3 normal;
attribute vec4 uv0;
attribute float uv1;

varying vec4 oColor_0;
#if SHADOW_CASTER
#else
varying vec2 oTexcoord2_0;
#endif

void main()
{
	// transform by indexed matrix
	vec4 transformedPos = vec4((worldMatrixArray[int(uv1)] * position).xyz, 1.0);

	// view / projection
	gl_Position = viewProjectionMatrix * transformedPos;

#if SHADOW_CASTER
	
	oColor_0 = ambient;
#else
	oTexcoord2_0 = uv0.xy;

	vec3 norm = mat3(worldMatrixArray[int(uv1)]) * normal;
	
	vec3 lightDir = 	normalize(
		lightPos.xyz -  (transformedPos.xyz * lightPos.w));

	oColor_0 = ambient + clamp(dot(lightDir, norm), 0.0, 1.0) * lightDiffuseColour;
#endif
}
