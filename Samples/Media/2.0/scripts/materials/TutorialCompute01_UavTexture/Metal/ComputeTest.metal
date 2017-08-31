#include <metal_stdlib>

using namespace metal;

kernel void main_metal
(
	texture2d<float, access::write> testTexture [[texture(UAV_SLOT_START)]],

	ushort3 gl_LocalInvocationID	[[thread_position_in_threadgroup]],
	ushort3 gl_GlobalInvocationID	[[thread_position_in_grid]]
)
{
	testTexture.write( float4( float2(gl_LocalInvocationID.xy) / 16.0f, 0.0f, 1.0f ),
					   gl_GlobalInvocationID.xy );
}
