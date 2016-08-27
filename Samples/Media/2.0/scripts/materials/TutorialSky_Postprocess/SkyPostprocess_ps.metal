#include <metal_stdlib>
using namespace metal;

struct PS_INPUT
{
	float3 cameraDir;
};

fragment float4 main_metal
(
	PS_INPUT inPs [[stage_in]],
	texturecube<float> skyCubemap [[texture(0)]],
	sampler samplerState [[sampler(0)]]
)
{
	return float4( skyCubemap.sample( samplerState, inPs.cameraDir ).xyz, 1.0f );
}
