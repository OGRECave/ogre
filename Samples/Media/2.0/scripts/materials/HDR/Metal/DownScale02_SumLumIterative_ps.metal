#include <metal_stdlib>
using namespace metal;

constexpr constant float2 c_offsets[4] =
{
	float2( -1.0, -1.0 ), float2( 1.0, -1.0 ),
	float2( -1.0,  1.0 ), float2( 1.0,  1.0 )
};

struct PS_INPUT
{
	float2 uv0;
};

fragment float4 main_metal
(
	PS_INPUT inPs [[stage_in]],
	texture2d<float>	lumRt			[[texture(0)]],
	sampler				samplerState	[[sampler(0)]],

	constant float4 &tex0Size [[buffer(PARAMETER_SLOT)]]
)
{
	float fLumAvg = lumRt.sample( samplerState, inPs.uv0 + c_offsets[0] * tex0Size.zw ).x;

	for( int i=1; i<4; ++i )
		fLumAvg += lumRt.sample( samplerState, inPs.uv0 + c_offsets[i] * tex0Size.zw ).x;
		
	//fLumAvg *= 0.0625f; // /= 16.0f;
	fLumAvg *= 0.25f; // /= 4.0f;
	
	return fLumAvg;
}
