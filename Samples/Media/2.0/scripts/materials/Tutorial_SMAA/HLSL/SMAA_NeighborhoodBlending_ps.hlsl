
#include "SMAA_HLSL.hlsl"

struct PS_INPUT
{
	float2 uv0			: TEXCOORD0;
	float4 offset       : TEXCOORD1;
};

Texture2D<float4> rt_input			: register(t0); //Can be sRGB
Texture2D<float4> blendTex			: register(t1);
#if SMAA_REPROJECTION
	Texture2D<float4> velocityTex	: register(t2);
#endif

float4 main
(
	PS_INPUT inPs,
	uniform float4 viewportSize
) : SV_Target
{
#if SMAA_REPROJECTION
	return SMAANeighborhoodBlendingPS( inPs.uv0, inPs.offset,
									   rt_input, blendTex, velocityTex );
#else
	return SMAANeighborhoodBlendingPS( inPs.uv0, inPs.offset,
									   rt_input, blendTex SMAA_EXTRA_PARAM_ARG );
#endif
}
