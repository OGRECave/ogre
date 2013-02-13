#version 150

in vec4 position;
in vec3 normal;
in vec4 uv0;
in vec4 uv1;

out vec4 colour;
out vec4 oUv0;
out vec4 oUv1;

uniform mat3x4 worldMatrix3x4Array[80];
uniform mat4 viewProjectionMatrix;
uniform vec4 lightPos;
uniform vec4 ambient;
uniform vec4 lightDiffuseColour;

void main()
{
	// transform by indexed matrix
	vec4 transformedPos = vec4((worldMatrix3x4Array[index] * position).xyz, 1.0);
	
	// view / projection
	gl_Position = viewProjectionMatrix * transformedPos;
	oUv = uv;

	vec3 norm = worldMatrix3x4Array[index] * normal;
	
	vec3 lightDir = 	normalize(
		lightPos.xyz -  (transformedPos.xyz * lightPos.w));

	colour = ambient + clamp(dot(lightDir, norm), 0.0, 1.0) * lightDiffuseColour
}

