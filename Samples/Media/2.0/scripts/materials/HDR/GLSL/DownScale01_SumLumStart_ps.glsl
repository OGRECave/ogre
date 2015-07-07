#version 330

out float fragColour;

in block
{
	vec2 uv0;
} inPs;

//Morton Order. Table generated in Python:
//def CompactBy1( x ):
//	x &= 0x55555555
//	x = (x ^ (x >>  1)) & 0x33333333
//	x = (x ^ (x >>  2)) & 0x0f0f0f0f
//	x = (x ^ (x >>  4)) & 0x00ff00ff
//	x = (x ^ (x >>  8)) & 0x0000ffff
//	return x
//def printMorton( val ):
//	x = CompactBy1( val )
//	y = CompactBy1( val >> 1 )
//	print( "\tvec2( %s, %s )," % (x, y) )
//
//for x in range(0, 16):
//	printMorton(x)

const vec2 c_offsets[16] = vec2[16]
(
	vec2( 0, 0 ), vec2( 1, 0 ), vec2( 0, 1 ), vec2( 1, 1 ),
	vec2( 2, 0 ), vec2( 3, 0 ), vec2( 2, 1 ), vec2( 3, 1 ),
	vec2( 0, 2 ), vec2( 1, 2 ), vec2( 0, 3 ), vec2( 1, 3 ),
	vec2( 2, 2 ), vec2( 3, 2 ), vec2( 2, 3 ), vec2( 3, 3 )
);

//Luminance coefficient taken from the DX SDK Docs
const vec3 c_luminanceCoeffs = vec3(0.2125f, 0.7154f, 0.0721f);
//Luminance vector for RGB colour in linear space (the usual coeffs are for gamma space colours)
//const vec3 c_luminanceCoeffs = vec3( 0.3086f, 0.6094f, 0.0820f );

uniform sampler2D rt0;

uniform vec4 tex0Size;
uniform vec4 viewportSize;

void main()
{
	//Compute how many pixels we have to skip because we can't sample them all
	//e.g we have a 4096x4096 viewport (rt0), and we're rendering to a 64x64 surface
	//We would need 64x64 samples, but we only sample 4x4, therefore we sample one
	//pixel and skip 15, then repeat. We perform:
	//(ViewportResolution / TargetResolution) / 4
	vec2 ratio = tex0Size.xy * viewportSize.zw * 0.25;

	vec3 vSample	= texture( rt0, inPs.uv0 ).xyz;
	float sampleLum	= dot( vSample, c_luminanceCoeffs ) + 0.0001;
	//float fLogLuminance = log( clamp( sampleLum, c_minLuminance, c_maxLuminance ) );
	float fLogLuminance = log( sampleLum * 1024.0 );

	for( int i=1; i<16; ++i )
	{
		//TODO: Precompute c_offsets[i] * ratio in CPU and upload it as c_offset, probably using a listener
		vSample		= texture( rt0, inPs.uv0 + ((c_offsets[i] * ratio) * tex0Size.zw) ).xyz;
		sampleLum	= dot( vSample, c_luminanceCoeffs ) + 0.0001;
		//fLogLuminance += log( clamp( sampleLum, c_minLuminance, c_maxLuminance ) );
		fLogLuminance += log( sampleLum * 1024.0 );
	}

	fLogLuminance *= 0.0625; // /= 16.0;

	fragColour = fLogLuminance;
}
