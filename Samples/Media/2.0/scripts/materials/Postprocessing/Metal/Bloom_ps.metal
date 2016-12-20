//---------------------------------------------
// Bloom

#include <metal_stdlib>
using namespace metal;

struct PS_INPUT
{
	float2 uv0;
};

struct Params
{
	float OriginalImageWeight;
	float BlurWeight;
};

fragment float4 main_metal
(
	PS_INPUT inPs [[stage_in]],
	texture2d<float>	RT				[[texture(0)]],
	texture2d<float>	Blur1			[[texture(1)]],
	sampler				samplerState	[[sampler(0)]],

	constant Params &params [[buffer(PARAMETER_SLOT)]]
)
{
	float4 sharp = RT.sample( samplerState,		inPs.uv0 );
	float4 blur  = Blur1.sample( samplerState,	inPs.uv0 );

	return blur*params.BlurWeight + sharp*params.OriginalImageWeight;
}
