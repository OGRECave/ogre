#include <metal_stdlib>
using namespace metal;

struct PS_INPUT
{
	float2 uv0;
};

struct Params
{
	float2 numTiles;
	float2 iNumTiles;
	float2 iNumTiles2;
	float4 lum;
};

fragment float4 main_metal
(
	PS_INPUT inPs [[stage_in]],
	texture2d<float>	RT				[[texture(0)]],
	texture3d<float>	pattern			[[texture(1)]],
	sampler				samplerState0	[[sampler(0)]],
	sampler				samplerState1	[[sampler(1)]],

	constant Params &p [[buffer(PARAMETER_SLOT)]]
)
{	
	float3 localVal;
	localVal.xy = fmod(inPs.uv0, p.iNumTiles);
	float2 middle = inPs.uv0 - localVal.xy;
	localVal.xy = localVal.xy * p.numTiles;
	middle +=  p.iNumTiles2;
	localVal.z = dot( RT.sample(samplerState0, middle ), p.lum );
	float4 c = pattern.sample(samplerState1,localVal).x;
	return c;
}
