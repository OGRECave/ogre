Texture2D<@insertpiece(texture0_pf_type)> srcRtt	: register(t0);
@property( !texture1_msaa )
	Texture2D<@insertpiece(texture1_pf_type)> srcDepth	: register(t1);
@end @property( texture1_msaa )
	Texture2DMS<@insertpiece(texture1_pf_type)> srcDepth: register(t1);
@end

RWTexture2D<@insertpiece(uav0_pf_type)> dstRtt		: register(u0);
RWTexture2D<@insertpiece(uav1_pf_type)> dstDepth	: register(u1);

/*Texture2D<unorm float4> srcRtt : register(t0);
Texture2D<float> srcDepth : register(t1);

RWTexture2D<unorm float4> dstRtt : register(u0);
RWTexture2D<float> dstDepth : register(u1);*/

[numthreads(@value( threads_per_group_x ), @value( threads_per_group_y ), @value( threads_per_group_z ))]
//[numthreads(8, 8, 1)]
void main
(
	uint3 gl_GlobalInvocationID : SV_DispatchThreadId
)
{
	uint2 xyPos0 = gl_GlobalInvocationID.xy << 1u;
	uint2 xyPos1 = int2( xyPos0.xy + uint2( 1, 0 ) );
	uint2 xyPos2 = int2( xyPos0.xy + uint2( 0, 1 ) );
	uint2 xyPos3 = int2( xyPos0.xy + uint2( 1, 1 ) );

	float4 srcRttValue0 = srcRtt.Load( int3( xyPos0.xy, 0 ) );
	float4 srcRttValue1 = srcRtt.Load( int3( xyPos1.xy, 0 ) );
	float4 srcRttValue2 = srcRtt.Load( int3( xyPos2.xy, 0 ) );
	float4 srcRttValue3 = srcRtt.Load( int3( xyPos3.xy, 0 ) );

	@property( !texture1_msaa )
		float srcDepthValue0 = srcDepth.Load( int3( xyPos0.xy, 0 ) ).x;
		float srcDepthValue1 = srcDepth.Load( int3( xyPos1.xy, 0 ) ).x;
		float srcDepthValue2 = srcDepth.Load( int3( xyPos2.xy, 0 ) ).x;
		float srcDepthValue3 = srcDepth.Load( int3( xyPos3.xy, 0 ) ).x;
	@end @property( texture1_msaa )
		float srcDepthValue0 = srcDepth.Load( xyPos0.xy, 0 ).x;
		float srcDepthValue1 = srcDepth.Load( xyPos1.xy, 0 ).x;
		float srcDepthValue2 = srcDepth.Load( xyPos2.xy, 0 ).x;
		float srcDepthValue3 = srcDepth.Load( xyPos3.xy, 0 ).x;
	@end

	dstRtt[ int2( xyPos0.xy ) ] = srcRttValue0;
	dstRtt[ int2( xyPos1.xy ) ] = srcRttValue1;
	dstRtt[ int2( xyPos2.xy ) ] = srcRttValue2;
	dstRtt[ int2( xyPos3.xy ) ] = srcRttValue3;

	dstDepth[ int2( xyPos0.xy ) ] = srcDepthValue0;
	dstDepth[ int2( xyPos1.xy ) ] = srcDepthValue1;
	dstDepth[ int2( xyPos2.xy ) ] = srcDepthValue2;
	dstDepth[ int2( xyPos3.xy ) ] = srcDepthValue3;
}
