#version 330

#ifdef GL_AMD_shader_trinary_minmax
	#extension GL_AMD_shader_trinary_minmax : enable
#endif

#ifndef GL_AMD_shader_trinary_minmax
	float max3( float x, float y, float z ) { return max(x, max(y, z)); }
#endif

#if !MSAA_INITIALIZED
	#define MSAA_SUBSAMPLE_WEIGHT	0.25
	#define MSAA_NUM_SUBSAMPLES		4
#endif

float rcp( float x )
{
	return 1.0 / x;
}

// Apply this to tonemap linear HDR color "c" after a sample is fetched in the resolve.
// Note "c" 1.0 maps to the expected limit of low-dynamic-range monitor output.
//vec3 tonemap( vec3 c ) { return c * rcp(max3(c.r, c.g, c.b) + 1.0); }

// When the filter kernel is a weighted sum of fetched colors,
// it is more optimal to fold the weighting into the tonemap operation.
vec3 tonemapWithWeight( float invLum, vec3 c, float w )
{
	return c * (invLum * w * rcp( max3(c.r, c.g, c.b) * invLum + 1.0 ));
}

// Apply this to restore the linear HDR color before writing out the result of the resolve.
vec3 tonemapInvert( float invLum, vec3 c )
{
	vec3 val = c * rcp( 1.0 - max3(c.r, c.g, c.b) );
	return val * rcp( invLum );
}

vec4 loadWithToneMapAndWeight( float invLum, sampler2DMS tex, ivec2 iCoord, int subsample )
{
	vec4 value = texelFetch( tex, iCoord, subsample ).xyzw;
	value.xyz = tonemapWithWeight( value.xyz, MSAA_SUBSAMPLE_WEIGHT );
	value.w *= MSAA_SUBSAMPLE_WEIGHT;

	return value;
}

uniform sampler2DMS rt0;

in block
{
	vec2 uv0;
} inPs;

out vec4 fragColour;

in vec4 gl_FragCoord;

void main()
{
	ivec2 iFragCoord = ivec2( gl_FragCoord.xy );

	float oldInvLum = texelFetch( oldLumRt, vec2( 0, 0 ), 0 );

	vec4 accumValue;

	accumValue  = loadWithToneMapAndWeight( oldInvLum, rt0, iFragCoord, 0 );
	accumValue += loadWithToneMapAndWeight( oldInvLum, rt0, iFragCoord, 1 );

#if MSAA_NUM_SUBSAMPLES > 2
	accumValue += loadWithToneMapAndWeight( oldInvLum, rt0, iFragCoord, 2 );
	accumValue += loadWithToneMapAndWeight( oldInvLum, rt0, iFragCoord, 3 );
#endif

#if MSAA_NUM_SUBSAMPLES > 4
	accumValue += loadWithToneMapAndWeight( oldInvLum, rt0, iFragCoord, 4 );
	accumValue += loadWithToneMapAndWeight( oldInvLum, rt0, iFragCoord, 5 );
	accumValue += loadWithToneMapAndWeight( oldInvLum, rt0, iFragCoord, 6 );
	accumValue += loadWithToneMapAndWeight( oldInvLum, rt0, iFragCoord, 7 );
#endif

#if MSAA_NUM_SUBSAMPLES > 8
	accumValue += loadWithToneMapAndWeight( oldInvLum, rt0, iFragCoord, 8 );
	accumValue += loadWithToneMapAndWeight( oldInvLum, rt0, iFragCoord, 9 );
	accumValue += loadWithToneMapAndWeight( oldInvLum, rt0, iFragCoord, 10 );
	accumValue += loadWithToneMapAndWeight( oldInvLum, rt0, iFragCoord, 11 );
	accumValue += loadWithToneMapAndWeight( oldInvLum, rt0, iFragCoord, 12 );
	accumValue += loadWithToneMapAndWeight( oldInvLum, rt0, iFragCoord, 13 );
	accumValue += loadWithToneMapAndWeight( oldInvLum, rt0, iFragCoord, 14 );
	accumValue += loadWithToneMapAndWeight( oldInvLum, rt0, iFragCoord, 15 );
#endif

	fragColour.xyz	= tonemapInvert( oldInvLum, accumValue.xyz );
	fragColour.w	= accumValue.w;
}
