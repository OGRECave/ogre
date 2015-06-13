#version 330

uniform sampler2D tex;

in block
{
	vec2 uv0;
} inPs;

out vec3 fragColour;

void main()
{
	fragColour = texture( tex, inPs.uv0 ).xyz;
}
