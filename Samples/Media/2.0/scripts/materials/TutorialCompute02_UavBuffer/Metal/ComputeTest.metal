#include <metal_stdlib>

using namespace metal;

kernel void main_metal
(
	device uint *pixelBuffer		[[buffer(UAV_SLOT_START)]],
	constant uint2 &texResolution	[[buffer(PARAMETER_SLOT)]],

	ushort3 gl_LocalInvocationID	[[thread_position_in_threadgroup]],
	ushort3 gl_GlobalInvocationID	[[thread_position_in_grid]]
)
{
	if( gl_GlobalInvocationID.x < texResolution.x && gl_GlobalInvocationID.y < texResolution.y )
	{
		uint idx = gl_GlobalInvocationID.y * texResolution.x + gl_GlobalInvocationID.x;
		pixelBuffer[idx] = pack_float_to_unorm4x8( float4( float2(gl_LocalInvocationID.xy) / 16.0f, 0.0f, 1.0f ) );
	}
}
