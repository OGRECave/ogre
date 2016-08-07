#include <metal_stdlib>
using namespace metal;

#define NUM_SAMPLES 65

struct PS_INPUT
{
	float2 uv0;
};

inline float3 fromSRGB( float3 x )
{
	return x * x;
}
inline float3 toSRGB( float3 x )
{
	return sqrt( x );
}

fragment float4 main_metal
(
	PS_INPUT inPs [[stage_in]],
	texture2d<float>	Blur0			[[texture(0)]],
	sampler				samplerState	[[sampler(0)]],

	constant float4 &invTex0Size [[buffer(PARAMETER_SLOT)]]
)
{
	float2 uv = inPs.uv0.xy;

	uv.x -= invTex0Size.x * ((NUM_SAMPLES-1) * 0.5);
	float3 sum = fromSRGB( Blur0.sample( samplerState, uv ).xyz );

	for( int i=1; i<NUM_SAMPLES; ++i )
	{
		uv.x += invTex0Size.x;
		sum += fromSRGB( Blur0.sample( samplerState, uv ).xyz );
	}

	return float4( toSRGB( sum / NUM_SAMPLES ), 1.0 );
}
