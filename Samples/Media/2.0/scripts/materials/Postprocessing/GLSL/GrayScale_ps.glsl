#version 330

uniform sampler2D RT;

out vec4 fragColour;

in block
{
	vec2 uv0;
} inPs;

void main()
{
	float greyscale = dot( texture(RT, inPs.uv0).xyz, vec3(0.3, 0.59, 0.11) );
	fragColour = vec4(greyscale, greyscale, greyscale, 1.0);
}
