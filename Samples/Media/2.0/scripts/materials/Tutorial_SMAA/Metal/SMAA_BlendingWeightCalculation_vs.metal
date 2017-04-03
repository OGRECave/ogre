#define VERTEX_SHADER 1
#include "SMAA_Metal.metal"

struct PS_INPUT
{
	float2 uv0;
	float2 pixcoord0;
	float4 offset[3];
	float4 gl_Position [[position]];
};

vertex PS_INPUT main_metal
(
	VS_INPUT input [[stage_in]],
	constant Params &p [[buffer(PARAMETER_SLOT)]]
)
{
	PS_INPUT outVs;
	outVs.gl_Position	= ( p.worldViewProj * input.position ).xyzw;
	outVs.uv0 = input.uv0.xy;
	SMAABlendingWeightCalculationVS( input.uv0.xy, outVs.pixcoord0, outVs.offset, p.viewportSize );

	return outVs;
}
