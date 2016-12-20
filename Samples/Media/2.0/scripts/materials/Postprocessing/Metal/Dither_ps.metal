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
	texture2d<float>	pattern			[[texture(1)]],
	sampler				samplerState0	[[sampler(0)]],
	sampler				samplerState1	[[sampler(1)]]
)
{	
	float c = dot( RT.sample( samplerState0, inPs.uv0 ).xyz, float3( 0.30, 0.11, 0.59 ) );
	float n = pattern.sample( samplerState1, inPs.uv0 ).x * 2.0 - 1.0;
	c += n;
	if (c > 0.5)
	{
		c = 0.0;
	}
	else
	{
		c = 1.0;
	}
	return float4( c, c, c, 1.0 );
}
