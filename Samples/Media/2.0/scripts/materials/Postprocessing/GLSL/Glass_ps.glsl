#version 330

uniform sampler2D RT;
uniform sampler2D NormalMap;

out vec4 fragColour;

in block
{
	vec2 uv0;
} inPs;

void main()
{
	vec2 normal = 2.0 * (texture( NormalMap, inPs.uv0 * 2.5 ).xy - 0.5);

	fragColour = texture( RT, inPs.uv0 + normal.xy * 0.05 );
}
