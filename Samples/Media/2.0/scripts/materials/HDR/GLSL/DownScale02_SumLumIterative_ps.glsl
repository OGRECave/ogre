#version 330

out float fragColour;

in block
{
	vec2 uv0;
} inPs;

const vec2 c_offsets[4] = vec2[4]
(
	vec2( -1.0, -1.0 ), vec2( 1.0, -1.0 ),
	vec2( -1.0,  1.0 ), vec2( 1.0,  1.0 )
);

uniform sampler2D lumRt;

uniform vec4 tex0Size;

void main()
{
	float fLumAvg = texture( lumRt, inPs.uv0 + c_offsets[0] * tex0Size.zw ).x;

	for( int i=1; i<4; ++i )
		fLumAvg += texture( lumRt, inPs.uv0 + c_offsets[i] * tex0Size.zw ).x;

	fLumAvg *= 0.25; // /= 4.0;
	
	fragColour = fLumAvg;
}
