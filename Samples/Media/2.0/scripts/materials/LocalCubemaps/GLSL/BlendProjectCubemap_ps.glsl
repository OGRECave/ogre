#version 330

out vec4 fragColour;

in block
{
	vec3 posLS;
} inPs;

uniform samplerCube cubeTexture;

uniform float weight;
uniform float lodLevel;

void main()
{
	fragColour = textureLod( cubeTexture, inPs.posLS, lodLevel ).xyzw * weight;
}
