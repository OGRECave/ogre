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
	float greyscale = dot( RT.sample(samplerState, inPs.uv0).xyz, float3(0.3, 0.59, 0.11) );
	return float4( greyscale, greyscale, greyscale, 1.0 );
}
