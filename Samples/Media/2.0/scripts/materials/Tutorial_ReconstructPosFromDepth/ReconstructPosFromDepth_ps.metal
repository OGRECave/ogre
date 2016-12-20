#include <metal_stdlib>
using namespace metal;

struct PS_INPUT
{
	float2 uv0;
	float3 cameraDir;
};

fragment float4 main_metal
(
	PS_INPUT inPs [[stage_in]],
	texture2d<float>	depthTexture	[[texture(0)]],
	sampler				samplerState	[[sampler(0)]],

	constant float2 &projectionParams	[[buffer(PARAMETER_SLOT)]]
)
{
	float fDepth = depthTexture.sample( samplerState, inPs.uv0 ).x;
	
	float linearDepth = projectionParams.y / (fDepth - projectionParams.x);
	
	float3 viewSpacePosition = inPs.cameraDir * linearDepth;
	
	//For this next line to work cameraPos would have to be an uniform in world space, and cameraDir
	//would have to be sent using the compositor setting "quad_normals camera_far_corners_world_space_centered"
	//float3 worldSpacePosition = inPs.cameraDir * linearDepth + cameraPos;
	
	//Scale the values down to get some useful colour debugging in the valid range
	return float4( viewSpacePosition.xy / 5, -viewSpacePosition.z / 80.0, 1.0 );
	//return linearDepth.xxx;
}
