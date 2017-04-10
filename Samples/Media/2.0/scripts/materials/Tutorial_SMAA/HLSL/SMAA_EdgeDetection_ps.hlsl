
#include "SMAA_HLSL.hlsl"

struct PS_INPUT
{
	float2 uv0			: TEXCOORD0;
	float4 offset[3]    : TEXCOORD1;
	float4 gl_Position	: SV_Position;
};

Texture2D<float4> rt_input		: register(t0); //Must not be sRGB
#if SMAA_PREDICATION
	Texture2D<float> depthTex	: register(t1);
#endif

float2 main
(
	PS_INPUT inPs,
	uniform float4 viewportSize
) : SV_Target
{
#if !SMAA_EDGE_DETECTION_MODE || SMAA_EDGE_DETECTION_MODE == 2
	#if SMAA_PREDICATION
		return SMAAColorEdgeDetectionPS( inPs.uv0, inPs.offset, rt_input, depthTex );
	#else
		return SMAAColorEdgeDetectionPS( inPs.uv0, inPs.offset, rt_input SMAA_EXTRA_PARAM_ARG );
	#endif
#else
	#if SMAA_PREDICATION
		return SMAALumaEdgeDetectionPS( inPs.uv0, inPs.offset, rt_input, depthTex );
	#else
		return SMAALumaEdgeDetectionPS( inPs.uv0, inPs.offset, rt_input SMAA_EXTRA_PARAM_ARG );
	#endif
#endif
}
