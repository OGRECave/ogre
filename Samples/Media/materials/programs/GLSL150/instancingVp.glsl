#version 150

in vec4 position;
in vec3 normal;
in vec4 uv0;
in float uv1;

out vec4 oColor_0;
#if SHADOW_CASTER
#else
out vec2 oTexcoord2_0;
#endif

uniform mat4x3 worldMatrix3x4Array[80];
uniform mat4 viewProjectionMatrix;
uniform vec4 lightPos;
uniform vec4 ambient;
uniform vec4 lightDiffuseColour;

void main()
{
#if SHADOW_CASTER
	// transform by indexed matrix
	vec4 transformedPos = vec4((worldMatrix3x4Array[int(uv1)] * position).xyz, 1.0);

	// view / projection
	gl_Position = viewProjectionMatrix * transformedPos;
	
	oColor_0 = ambient;
#else
	// transform by indexed matrix
	vec4 transformedPos = vec4((worldMatrix3x4Array[int(uv1)] * position).xyz, 1.0);
	
	// view / projection
	gl_Position = viewProjectionMatrix * transformedPos;
	oTexcoord2_0 = uv0.xy;

	vec3 norm = mat3(worldMatrix3x4Array[int(uv1)]) * normal;
	
	vec3 lightDir = 	normalize(
		lightPos.xyz -  (transformedPos.xyz * lightPos.w));

	oColor_0 = ambient + clamp(dot(lightDir, norm), 0.0, 1.0) * lightDiffuseColour;
#endif
}

