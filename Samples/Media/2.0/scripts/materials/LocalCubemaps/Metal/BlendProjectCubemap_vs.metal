#include <metal_stdlib>
using namespace metal;

struct VS_INPUT
{
	float4 position [[attribute(VES_POSITION)]];
};

struct PS_INPUT
{
	float3 posLS;
	float4 gl_Position [[position]];
};

struct Params
{
	float4x4 worldViewProj;
	float4x4 localToProbeLocal;
};

vertex PS_INPUT main_metal
(
	VS_INPUT input [[stage_in]],
	constant Params &p	[[buffer(PARAMETER_SLOT)]]
)
{
	PS_INPUT outVs;

	outVs.gl_Position	= ( p.worldViewProj * float4( input.position.xyz, 1.0 ) ).xyzw;
	outVs.posLS			= ( p.localToProbeLocal * float4( input.position.xyz, 1.0 ) ).xyz;
	outVs.posLS.z = -outVs.posLS.z; //Left handed

	return outVs;
}
