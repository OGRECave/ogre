#include <metal_stdlib>
using namespace metal;

struct VS_INPUT
{
	float4 position [[attribute(VES_POSITION)]];
	float3 normal	[[attribute(VES_NORMAL)]];
	float2 uv0		[[attribute(VES_TEXTURE_COORDINATES0)]];
};

struct PS_INPUT
{
	float2 uv0;
	float3 cameraDir;

	float4 gl_Position [[position]];
};

vertex PS_INPUT main_metal
(
	VS_INPUT input [[stage_in]],
	constant float4x4 &worldViewProj [[buffer(PARAMETER_SLOT)]]
)
{
	PS_INPUT outVs;

	outVs.gl_Position	= ( worldViewProj * input.position ).xyzw;
	outVs.uv0			= input.uv0;
	outVs.cameraDir		= input.normal;

	return outVs;
}
