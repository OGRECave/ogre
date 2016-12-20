#include <metal_stdlib>
using namespace metal;

struct PS_INPUT
{
	float2 uv0;
};

struct Params
{
	float4 lum;
	float time;
};

fragment float4 main_metal
(
	PS_INPUT inPs [[stage_in]],
	texture2d<float>	RT				[[texture(0)]],
	texture3d<float>	noiseVol		[[texture(1)]],
	sampler				samplerState0	[[sampler(0)]],
	sampler				samplerState1	[[sampler(1)]],

	constant Params &p [[buffer(PARAMETER_SLOT)]]
)
{	
	float4 oC;
	oC = RT.sample( samplerState0, inPs.uv0 );
	
	//obtain luminence value
	oC = dot(oC,p.lum);
	
	//add some random noise
	oC += 0.2 *(noiseVol.sample( samplerState1, float3(inPs.uv0*5,p.time) ))- 0.05;
	
	//add lens circle effect
	//(could be optimised by using texture)
	float dist = distance(inPs.uv0, float2(0.5,0.5));
	oC *= smoothstep(0.5,0.45,dist);
	
	//add rb to the brightest pixels
	oC.rb = max (oC.r - 0.75, 0.0)*4.0;
	
	return oC ;
}
