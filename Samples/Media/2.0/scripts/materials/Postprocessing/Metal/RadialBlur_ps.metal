#include <metal_stdlib>
using namespace metal;

struct PS_INPUT
{
	float2 uv0;
};

struct Params
{
	float4	centerUVPos; //z = min, w = 1 / (max - min)
	float	exponent;
};

#define NUM_SAMPLES 7

constexpr constant float c_multipliers[NUM_SAMPLES] =
{
	1.00f,
	0.99f,
	0.98f,
	0.97f,
	0.96f,
	0.94f,
	0.93f
};

fragment float4 main_metal
(
	PS_INPUT inPs [[stage_in]],
	texture2d<float>	rt0				[[texture(0)]],
	sampler				samplerState	[[sampler(0)]],

	constant Params &params [[buffer(PARAMETER_SLOT)]]
)
{
	float atten = ( distance( inPs.uv0, params.centerUVPos.xy ) -
					params.centerUVPos.z ) * params.centerUVPos.w;
	atten = 1.0f - saturate( atten );
	atten = pow( atten, params.exponent );

	float4 originalColour = rt0.sample( samplerState, (inPs.uv0 - params.centerUVPos.xy) *
													  c_multipliers[0] + params.centerUVPos.xy );

	float4 sumColours = float4( 0.0, 0.0, 0.0, 0.0 );
	for( int i=1; i<NUM_SAMPLES; ++i )
	{
		sumColours += rt0.sample( samplerState, (inPs.uv0 - params.centerUVPos.xy) *
												c_multipliers[i] + params.centerUVPos.xy );
	}

	//'originalColour' has always the same strength. 'sumColours' gets weaker over distance (past min distance).
	//The denominator is altered according to the distance, so that the average is always consistent.
	return ( originalColour + sumColours * atten ) / (1 + (NUM_SAMPLES-1) * atten );
}
