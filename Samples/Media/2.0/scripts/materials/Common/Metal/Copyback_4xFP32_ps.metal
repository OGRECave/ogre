#include <metal_stdlib>
using namespace metal;

struct PS_INPUT
{
	float2 uv0;
};

fragment float4 main_metal
(
	PS_INPUT inPs [[stage_in]],
	texture2d<float>	myTexture [[texture(0)]],
	sampler				mySampler [[sampler(0)]]
)
{
	return myTexture.sample( mySampler, inPs.uv0 ).xyzw;
}
