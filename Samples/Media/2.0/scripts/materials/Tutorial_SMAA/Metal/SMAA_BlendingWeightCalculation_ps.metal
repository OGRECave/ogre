
#include "SMAA_Metal.metal"

struct PS_INPUT
{
	float2 uv0;
	float2 pixcoord0;
	float4 offset0;
	float4 offset1;
	float4 offset2;
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
	return SMAABlendingWeightCalculationPS( inPs.uv0, inPs.pixcoord0, inPs.offset0,
											inPs.offset1, inPs.offset2,
											edgeTex, areaTex, searchTex,
											float4( 0, 0, 0, 0 ), p.viewportSize );
}
