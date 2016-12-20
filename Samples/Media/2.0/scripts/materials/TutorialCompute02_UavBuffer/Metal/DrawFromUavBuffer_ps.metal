#include <metal_stdlib>

using namespace metal;

struct PS_INPUT
{
	float2 uv0;
	float4 gl_FragCoord [[position]];
};

fragment float4 main_metal
(
	PS_INPUT inPs [[stage_in]],
	device const uint *pixelBuffer [[buffer(UAV_SLOT_START)]],
	constant uint2 &texResolution [[buffer(PARAMETER_SLOT)]]
)
{
	uint idx = uint(inPs.gl_FragCoord.y) * texResolution.x + uint(inPs.gl_FragCoord.x);
	float4 fragColour = unpack_unorm4x8_to_float( pixelBuffer[idx] );
	
	return fragColour;
}
