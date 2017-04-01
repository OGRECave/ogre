
#include "SMAA_Metal.metal"

struct PS_INPUT
{
	float2 uv0;
	float2 pixcoord0;
	float4 offset[3];
};

fragment float4 main_metal
(
	PS_INPUT inPs [[stage_in]],
	constant Params &p [[buffer(PARAMETER_SLOT)]],

	texture2d<float> edgeTex		[[texture(0)]],
	texture2d<float> areaTex		[[texture(1)]],
	texture2d<float> searchTex		[[texture(2)]]
)
{
	return SMAABlendingWeightCalculationPS( inPs.uv0, inPs.pixcoord0, inPs.offset,
											edgeTex, areaTex, searchTex,
											float4( 0, 0, 0, 0 ), p.viewportSize );
}
