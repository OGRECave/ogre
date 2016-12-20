#include <metal_stdlib>
using namespace metal;

struct PS_INPUT
{
	float2 uv0;
};

constexpr constant float2 usedTexelED[8] =
{
	float2( -1, -1 ),
	float2(  0, -1 ),
	float2(  1, -1 ),
	float2( -1,  0 ),
	float2(  1,  0 ),
	float2( -1,  1 ),
	float2(  0,  1 ),
	float2(  1,  1 )
};

fragment float4 main_metal
(
	PS_INPUT inPs [[stage_in]],
	texture2d<float>	RT				[[texture(0)]],
	sampler				samplerState	[[sampler(0)]],

	constant float2 &vTexelSize			[[buffer(PARAMETER_SLOT)]]
)
{
	float4 cAvgColor = 9 * RT.sample( samplerState, inPs.uv0 );

	for(int t=0; t<8; t++)
		cAvgColor -= RT.sample( samplerState, inPs.uv0 + vTexelSize * usedTexelED[t] );

	return cAvgColor;
}
