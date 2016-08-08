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
	float charBias;
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

	//sample RT
	localVal.xy = fmod(inPs.uv0, p.iNumTiles);
	float2 middle = inPs.uv0 - localVal.xy;
	localVal.xy = localVal.xy * p.numTiles;
	
	//iNumTiles2 = iNumTiles / 2
	middle = middle + p.iNumTiles2;
	float4 c = RT.sample( samplerState0, middle );
	
	//multiply luminance by charbias , beacause not all slices of the ascii
	//volume texture are used
	localVal.z = dot(c , p.lum)*p.charBias;
	
	//fix to brighten the dark pixels with small characters
	//c *= mix(2.0,1.0, localVal.z);
	
	c *= pattern.sample( samplerState1, localVal );
	return c;
}
