#version 330

uniform sampler2D RT;
uniform sampler2D noise;

out vec4 fragColour;

in block
{
	vec2 uv0;
} inPs;

void main()
{
	float c = dot( texture(RT, inPs.uv0).xyz, vec3(0.30, 0.11, 0.59) );
	float n = texture(noise, inPs.uv0).x * 2.0 - 1.0;
	c += n;

	c = step(c, 0.5);

	fragColour = vec4( c, c, c, 1.0 );
}
