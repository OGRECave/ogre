#version 330

uniform sampler2D RT;
uniform vec2 vTexelSize;

out vec4 fragColour;

in block
{
	vec2 uv0;
} inPs;

void main()
{
	const vec2 usedTexelED[8] = vec2[8]
	(
		vec2(-1, -1),
		vec2( 0, -1),
		vec2( 1, -1),
		vec2(-1,  0),
		vec2( 1,  0),
		vec2(-1,  1),
		vec2( 0,  1),
		vec2( 1,  1)
	);

	vec4 cAvgColor = 9.0 * texture( RT, inPs.uv0 );

	for(int t=0; t<8; t++)
		cAvgColor -= texture( RT, inPs.uv0 + vTexelSize * usedTexelED[t] );

	fragColour = cAvgColor;
}
