#include <metal_stdlib>

using namespace metal;

kernel void main_metal
(
	texture2d<@insertpiece(texture0_pf_type), access::read> srcRtt			[[texture(0)]],
	@property( !texture1_msaa )
		depth2d<@insertpiece(texture1_pf_type), access::read> srcDepth		[[texture(1)]],
	@end @property( texture1_msaa )
		depth2d_ms<@insertpiece(texture1_pf_type), access::read> srcDepth	[[texture(1)]],
	@end

	texture2d<@insertpiece(uav0_pf_type), access::write> dstRtt		[[texture(UAV_SLOT_START+0)]],
	texture2d<@insertpiece(uav1_pf_type), access::write> dstDepth	[[texture(UAV_SLOT_START+1)]],

	ushort3 gl_GlobalInvocationID	[[thread_position_in_grid]]
)
{
	ushort2 xyPos0 = gl_GlobalInvocationID.xy << 1u;
	ushort2 xyPos1 = xyPos0.xy + ushort2( 1, 0 );
	ushort2 xyPos2 = xyPos0.xy + ushort2( 0, 1 );
	ushort2 xyPos3 = xyPos0.xy + ushort2( 1, 1 );

	float4 srcRttValue0 = srcRtt.read( xyPos0.xy, 0 );
	float4 srcRttValue1 = srcRtt.read( xyPos1.xy, 0 );
	float4 srcRttValue2 = srcRtt.read( xyPos2.xy, 0 );
	float4 srcRttValue3 = srcRtt.read( xyPos3.xy, 0 );

	float srcDepthValue0 = srcDepth.read( xyPos0.xy, 0 );
	float srcDepthValue1 = srcDepth.read( xyPos1.xy, 0 );
	float srcDepthValue2 = srcDepth.read( xyPos2.xy, 0 );
	float srcDepthValue3 = srcDepth.read( xyPos3.xy, 0 );

	dstRtt.write( srcRttValue0, xyPos0.xy );
	dstRtt.write( srcRttValue1, xyPos1.xy );
	dstRtt.write( srcRttValue2, xyPos2.xy );
	dstRtt.write( srcRttValue3, xyPos3.xy );

	dstDepth.write( srcDepthValue0, xyPos0.xy );
	dstDepth.write( srcDepthValue1, xyPos1.xy );
	dstDepth.write( srcDepthValue2, xyPos2.xy );
	dstDepth.write( srcDepthValue3, xyPos3.xy );
}
