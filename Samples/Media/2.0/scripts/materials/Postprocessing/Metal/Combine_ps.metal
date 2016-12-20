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
	texture2d<float>	Sum				[[texture(1)]],
	sampler				samplerState	[[sampler(0)]],

	constant float &blur				[[buffer(PARAMETER_SLOT)]]
)
{
   float4 render = RT.sample( samplerState, inPs.uv0 );
   float4 sum = Sum.sample( samplerState, inPs.uv0 );

   return mix( render, sum, blur );
}
