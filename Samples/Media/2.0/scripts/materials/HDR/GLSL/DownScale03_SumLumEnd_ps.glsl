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
uniform sampler2D oldLumRt;

uniform vec3 exposure;
uniform float timeSinceLast;
uniform vec4 tex0Size;

void main()
{
	float fLumAvg = texture( lumRt, inPs.uv0 + c_offsets[0] * tex0Size.zw ).x;

	for( int i=1; i<4; ++i )
		fLumAvg += texture( lumRt, inPs.uv0 + c_offsets[i] * tex0Size.zw ).x;

	fLumAvg *= 0.25; // /= 4.0;

	float newLum = exposure.x / exp( clamp( fLumAvg, exposure.y, exposure.z ) );
	float oldLum = texture( oldLumRt, float( 0.0 ).xx ).x;

	//Adapt luminicense based 75% per second.
	fragColour = mix( newLum, oldLum, pow( 0.25, timeSinceLast ) );
}
