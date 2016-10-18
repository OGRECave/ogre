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
	float3x4 worldScaledMatrix;
	float3 probeCameraPosScaled;
};

vertex PS_INPUT main_metal
(
	VS_INPUT input [[stage_in]],
	constant Params &p	[[buffer(PARAMETER_SLOT)]]
)
{
	PS_INPUT outVs;

	outVs.gl_Position	= ( p.worldViewProj * float4( input.position.xyz, 1.0 ) ).xyzw;
	outVs.posLS			= ( float4( input.position.xyz, 1.0 ) * p.worldScaledMatrix ).xyz;
	outVs.posLS			= outVs.posLS - p.probeCameraPosScaled;
	outVs.posLS.z		= -outVs.posLS.z; //Left handed

	return outVs;
}
