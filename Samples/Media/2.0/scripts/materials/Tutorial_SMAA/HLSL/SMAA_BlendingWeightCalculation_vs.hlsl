
#include "SMAA_HLSL.hlsl"

struct VS_INPUT
{
	float4 vertex	: POSITION;
	float2 uv0		: TEXCOORD0;
};

struct PS_INPUT
{
	float2 uv0			: TEXCOORD0;
	float2 pixcoord0    : TEXCOORD1;
	float4 offset[3]    : TEXCOORD2;
	float4 gl_Position	: SV_Position;
};

PS_INPUT main
(
	VS_INPUT input,
	uniform float4x4 worldViewProj,
	uniform float4 viewportSize
)
{
	PS_INPUT outVs;
	outVs.gl_Position	= mul( worldViewProj, input.vertex ).xyzw;
	outVs.uv0 = input.uv0.xy;
	SMAABlendingWeightCalculationVS( input.uv0.xy, outVs.pixcoord0, outVs.offset SMAA_EXTRA_PARAM_ARG );

	return outVs;
}
