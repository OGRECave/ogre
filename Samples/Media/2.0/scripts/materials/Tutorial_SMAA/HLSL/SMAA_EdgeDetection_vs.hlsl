
#include "SMAA_HLSL.hlsl"

#include "SMAA.hlsl"

struct VS_INPUT
{
	float4 vertex	: POSITION;
	float2 uv0		: TEXCOORD0;
};

struct PS_INPUT
{
	float2 uv0			: TEXCOORD0;
	float4 offset[3]    : TEXCOORD1;
	float4 gl_Position	: SV_Position;
};

PS_INPUT main
(
	VS_INPUT input,
	uniform float4x4 worldViewProj
)
{
	PS_INPUT outVs;
	outVs.gl_Position	= mul( worldViewProj, input.vertex ).xyzw;
	outVs.uv0 = input.uv0.xy;
	SMAAEdgeDetectionVS( input.uv0.xy, outVs.offset );

	return outVs;
}
