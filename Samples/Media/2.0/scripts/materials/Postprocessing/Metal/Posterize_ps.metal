#include <metal_stdlib>
using namespace metal;

struct PS_INPUT
{
	float2 uv0;
};

fragment float4 main_metal
(
	PS_INPUT inPs [[stage_in]],
	texture2d<float>	RT				[[texture(0)]],
	sampler				samplerState	[[sampler(0)]]
)
{
	float nColors = 8;
	float gamma = 0.6;

	float4 texCol = RT.sample( samplerState, inPs.uv0 );
	float3 tc = texCol.xyz;
	tc = pow(tc, gamma);
	tc = tc * nColors;
	tc = floor(tc);
	tc = tc / nColors;
	tc = pow(tc,1.0/gamma);
	return float4(tc,texCol.w);
}
