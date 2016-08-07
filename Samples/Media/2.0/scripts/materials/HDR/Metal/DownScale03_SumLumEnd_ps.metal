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

struct Params
{
	float3 exposure;
	float timeSinceLast;
	float4 tex0Size;
};

fragment float4 main_metal
(
	PS_INPUT inPs [[stage_in]],
	texture2d<float>				lumRt			[[texture(0)]],
	texture2d<float, access::read>	oldLumRt		[[texture(1)]],
	sampler							samplerBilinear	[[sampler(0)]],

	constant Params &p [[buffer(PARAMETER_SLOT)]]
)
{
	float fLumAvg = lumRt.sample( samplerBilinear, inPs.uv0 + c_offsets[0] * p.tex0Size.zw ).x;

	for( int i=1; i<4; ++i )
		fLumAvg += lumRt.sample( samplerBilinear, inPs.uv0 + c_offsets[i] * p.tex0Size.zw ).x;

	fLumAvg *= 0.25f; // /= 4.0f;

	float newLum = p.exposure.x / exp( clamp( fLumAvg, p.exposure.y, p.exposure.z ) );
	float oldLum = oldLumRt.read( uint2( 0, 0 ) ).x;

	//Adapt luminicense based 75% per second.
	return mix( newLum, oldLum, pow( 0.25f, p.timeSinceLast ) );
}
