#version 330

uniform sampler2D RT;

out vec4 fragColour;

in block
{
	vec2 uv0;
} inPs;

void main()
{
	fragColour = 1.0 - texture(RT, inPs.uv0);
}
