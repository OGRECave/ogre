#version 330

uniform sampler2D Image;
uniform float scale;
uniform float pixelSize;

out vec4 fragColour;

in block
{
	vec2 uv0;
} inPs;

// The Laplace filter approximates the second order derivate,
// that is, the rate of change of slope in the image. It can be
// used for edge detection. The Laplace filter gives negative
// response on the higher side of the edge and positive response
// on the lower side.

// This is the filter kernel:
// 0  1  0
// 1 -4  1
// 0  1  0

void main()
{
	const vec2 samples[4] = vec2[4]
	(
		vec2(  0, -1 ),
		vec2( -1,  0 ),
		vec2(  1,  0 ),
		vec2(  0,  1 )
	);

	mediump vec4 tc = texture( Image, inPs.uv0 );
    vec4 laplace = -4.0 * tc;

    // Sample the neighbor pixels
	for( int i = 0; i < 4; ++i )
	   laplace += texture( Image, inPs.uv0 + pixelSize * samples[i] );

    fragColour = (0.5 + scale * laplace);
}
