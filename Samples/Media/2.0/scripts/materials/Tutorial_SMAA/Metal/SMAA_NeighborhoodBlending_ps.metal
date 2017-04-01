
#include "SMAA_Metal.metal"

struct PS_INPUT
{
	float2 uv0;
	float4 offset;
};

fragment float4 main_metal
(
	PS_INPUT inPs [[stage_in]],
	constant Params &p [[buffer(PARAMETER_SLOT)]],

	texture2d<float> rt_input			[[texture(0)]], //Can be sRGB
	texture2d<float> blendTex			[[texture(1)]]
	#if SMAA_REPROJECTION
	,	texture2d<float> velocityTex	[[texture(2)]]
	#endif
)
{
#if SMAA_REPROJECTION
	return SMAANeighborhoodBlendingPS( inPs.uv0, inPs.offset,
									   rt_input, blendTex, velocityTex, p.viewportSize );
#else
	return SMAANeighborhoodBlendingPS( inPs.uv0, inPs.offset,
									   rt_input, blendTex, p.viewportSize );
#endif
}
