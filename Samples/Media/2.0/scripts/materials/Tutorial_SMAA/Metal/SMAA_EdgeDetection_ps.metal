
#include "SMAA_Metal.metal"

struct PS_INPUT
{
	float2 uv0;
	float4 offset0;
	float4 offset1;
	float4 offset2;
};

fragment float2 main_metal
(
	PS_INPUT inPs [[stage_in]],
	constant Params &p [[buffer(PARAMETER_SLOT)]],

	texture2d<float> rt_input		[[texture(0)]] //Must not be sRGB
	#if SMAA_PREDICATION
	,	texture2d<float> depthTex	[[texture(1)]]
	#endif
)
{
	float4 offset[3];
	offset[0] = inPs.offset0;
	offset[1] = inPs.offset1;
	offset[2] = inPs.offset2;

#if !SMAA_EDGE_DETECTION_MODE || SMAA_EDGE_DETECTION_MODE == 2
	#if SMAA_PREDICATION
		return SMAAColorEdgeDetectionPS( inPs.uv0, inPs.offset0, inPs.offset1,
										 inPs.offset2, rt_input, depthTex, p.viewportSize );
	#else
		return SMAAColorEdgeDetectionPS( inPs.uv0, inPs.offset0, inPs.offset1,
										 inPs.offset2, rt_input, p.viewportSize );
	#endif
#else
	#if SMAA_PREDICATION
		return SMAALumaEdgeDetectionPS( inPs.uv0, inPs.offset0, inPs.offset1,
										inPs.offset2, rt_input, depthTex, p.viewportSize );
	#else
		return SMAALumaEdgeDetectionPS( inPs.uv0, inPs.offset0, inPs.offset1,
										inPs.offset2, rt_input, p.viewportSize );
	#endif
#endif
}
