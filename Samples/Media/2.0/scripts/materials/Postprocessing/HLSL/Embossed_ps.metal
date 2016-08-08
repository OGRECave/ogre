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
    float4 Color;
    Color.a = 1.0f;
    Color.xyz = 0.5f;
	Color.xyz -= RT.sample( samplerState, inPs.uv0 - 0.001).xyz*2.0f;
	Color.xyz += RT.sample( samplerState, inPs.uv0 + 0.001).xyz*2.0f;
    Color.xyz = (Color.r+Color.g+Color.b)/3.0f;
    return Color;
}
