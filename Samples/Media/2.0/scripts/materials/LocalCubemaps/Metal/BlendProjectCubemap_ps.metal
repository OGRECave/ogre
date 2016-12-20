#include <metal_stdlib>
using namespace metal;

struct PS_INPUT
{
	float3 posLS;
};

struct Params
{
	float weight;
	float lodLevel;
};

fragment float4 main_metal
(
	PS_INPUT inPs [[stage_in]],

	texturecube<float>	cubeTexture		[[texture(0)]],
	sampler				cubeSampler		[[sampler(0)]],

	constant Params &p	[[buffer(PARAMETER_SLOT)]]
)
{
	return cubeTexture.sample( cubeSampler, inPs.posLS, level( p.lodLevel ) ).xyzw * p.weight;
}
