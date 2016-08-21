#include <metal_stdlib>
using namespace metal;

struct PS_INPUT
{
	float2 uv0;
};

fragment float4 main_metal
(
	PS_INPUT inPs [[stage_in]],

	texture2d<float>	ssaoTexture		[[texture(0)]],
	texture2d<float>	sceneTexture	[[texture(1)]],

	sampler				samplerState	[[sampler(0)]],

	constant float &powerScale			[[buffer(PARAMETER_SLOT)]]
)
{
	float ssao = ssaoTexture.sample( samplerState, inPs.uv0 ).x;
	
	ssao = saturate( pow(ssao, powerScale) );

	float4 col = sceneTexture.sample( samplerState, inPs.uv0 ).xyzw;
	return float4( col.xyz * ssao, col.w );
}
