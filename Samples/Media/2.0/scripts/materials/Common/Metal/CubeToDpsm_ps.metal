#include <metal_stdlib>
using namespace metal;

struct PS_INPUT
{
	float2 uv0;
};

struct PS_OUTPUT
{
#if OUTPUT_TO_COLOUR
	float depth [[color(0)]];
#else
	float depth [[depth(any)]];
#endif
};

fragment PS_OUTPUT main_metal
(
	PS_INPUT inPs [[stage_in]],

	texturecube<float> depthTexture	[[texture(0)]],
	sampler samplerState			[[sampler(0)]]
)
{
	float3 cubeDir;

	cubeDir.x = fmod( inPs.uv0.x, 0.5 ) * 4.0 - 1.0;
	cubeDir.y = inPs.uv0.y * 2.0 - 1.0;
	cubeDir.z = 0.5 - 0.5 * (cubeDir.x * cubeDir.x + cubeDir.y * cubeDir.y);

	cubeDir.z = inPs.uv0.x < 0.5 ? cubeDir.z : -cubeDir.z;

	PS_OUTPUT outPs;
	outPs.depth = depthTexture.sample( samplerState, cubeDir.xyz, level(0) ).x;
	return outPs;
}
