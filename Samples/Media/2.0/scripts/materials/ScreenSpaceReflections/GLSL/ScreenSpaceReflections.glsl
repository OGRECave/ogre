#version 330

uniform sampler2D prevFrame;
uniform sampler2D depthTexture;
uniform sampler2D gBuf_normals;
uniform sampler2D gBuf_shadowRoughness;

//uniform float scale;
//uniform float pixelSize;

out vec4 fragColour;

in block
{
	vec2 uv0;
} inPs;

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
