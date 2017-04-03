
#include "SMAA_HLSL.hlsl"

struct PS_INPUT
{
	float2 uv0			: TEXCOORD0;
	float2 pixcoord0    : TEXCOORD1;
	float4 offset[3]    : TEXCOORD2;
	float4 gl_Position	: SV_Position;
};

Texture2D edgeTex		: register(t0);
Texture2D areaTex		: register(t1);
Texture2D searchTex		: register(t2);

float4 main
(
	PS_INPUT inPs,
	uniform float4 viewportSize
) : SV_Target
{
	return SMAABlendingWeightCalculationPS( inPs.uv0, inPs.pixcoord0, inPs.offset,
											edgeTex, areaTex, searchTex,
											float4( 0, 0, 0, 0 ) SMAA_EXTRA_PARAM_ARG );
}
